/* SPDX-License-Identifier: Apache-2.0 */
/**
 * NNStreamer API / Machine Learning Agent Daemon
 * Copyright (C) 2024 Samsung Electronics Co., Ltd. All Rights Reserved.
 */
/**
 * @file        unittest_mlops_plugin_parser.cc
 * @date        27 Mar 2024
 * @brief       Unit tests for the MLOps plugin parser.
 * @see         https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author      Yongjoo Ahn <yongjoo1.ahn@samsung.com>
 * @bug         No known bugs except for NYI items
 *
 * This file contains the unit tests for the MLOps plugin parser. It defines the mock class
 * and test fixtures, and implements the unit tests using gtest and gmock.The mock class are
 * used to simulate the behavior of the pkgmgr-info class, which is used by the plugin parser.
 */
#include <glib.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mlops-agent-interface.h>
#include <pkgmgr-info.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

/**
 * @brief Interface for pkgmgr-info
 */
class IPkgMgrInfo
{
    public:
    virtual ~IPkgMgrInfo() {}
    virtual int pkgmgrinfo_pkginfo_get_pkginfo (const char *pkgid, pkgmgrinfo_pkginfo_h *pkginfo) = 0;
    virtual int pkgmgrinfo_pkginfo_destroy_pkginfo (pkgmgrinfo_pkginfo_h handle) = 0;
    virtual int pkgmgrinfo_pkginfo_get_type (pkgmgrinfo_pkginfo_h handle, char **type) = 0;
    virtual int pkgmgrinfo_pkginfo_get_root_path (pkgmgrinfo_pkginfo_h handle, char **root_path) = 0;
    virtual int pkgmgrinfo_pkginfo_get_res_type (pkgmgrinfo_pkginfo_h handle, char **type) = 0;
    virtual int pkgmgrinfo_pkginfo_get_res_version (pkgmgrinfo_pkginfo_h handle, char **res_version) = 0;
};

/**
 * @brief pkgmgr-info Mock class
 */
class PkgMgrInfoMock : public IPkgMgrInfo
{
  public:
  MOCK_METHOD (int, pkgmgrinfo_pkginfo_get_pkginfo, (const char *pkgid, pkgmgrinfo_pkginfo_h *pkginfo));
  MOCK_METHOD (int, pkgmgrinfo_pkginfo_destroy_pkginfo, (pkgmgrinfo_pkginfo_h handle));
  MOCK_METHOD (int, pkgmgrinfo_pkginfo_get_type, (pkgmgrinfo_pkginfo_h handle, char **type));
  MOCK_METHOD (int, pkgmgrinfo_pkginfo_get_root_path, (pkgmgrinfo_pkginfo_h handle, char **root_path));
  MOCK_METHOD (int, pkgmgrinfo_pkginfo_get_res_type, (pkgmgrinfo_pkginfo_h handle, char **res_type));
  MOCK_METHOD (int, pkgmgrinfo_pkginfo_get_res_version, (pkgmgrinfo_pkginfo_h handle, char **res_version));
};

PkgMgrInfoMock *pkgMgrInfoMockInstance = nullptr;

/**
 * @brief Mock for pkgmgrinfo_pkginfo_get_pkgid
 */
extern "C" int
pkgmgrinfo_pkginfo_get_pkginfo (const char *pkgid, pkgmgrinfo_pkginfo_h *return_handle)
{
  return pkgMgrInfoMockInstance->pkgmgrinfo_pkginfo_get_pkginfo (pkgid, return_handle);
}

/**
 * @brief Mock for pkgmgrinfo_pkginfo_destroy_pkginfo
 */
extern "C" int
pkgmgrinfo_pkginfo_destroy_pkginfo (pkgmgrinfo_pkginfo_h handle)
{
  return pkgMgrInfoMockInstance->pkgmgrinfo_pkginfo_destroy_pkginfo (handle);
}

/**
 * @brief Mock for pkgmgrinfo_pkginfo_get_type
 */
extern "C" int
pkgmgrinfo_pkginfo_get_type (pkgmgrinfo_pkginfo_h handle, char **return_type)
{
  return pkgMgrInfoMockInstance->pkgmgrinfo_pkginfo_get_type (handle, return_type);
}

/**
 * @brief Mock for pkgmgrinfo_pkginfo_get_root_path
 */
extern "C" int
pkgmgrinfo_pkginfo_get_root_path (pkgmgrinfo_pkginfo_h handle, char **return_path)
{
  return pkgMgrInfoMockInstance->pkgmgrinfo_pkginfo_get_root_path (handle, return_path);
}

/**
 * @brief Mock for pkgmgrinfo_pkginfo_get_res_type
 */
extern "C" int
pkgmgrinfo_pkginfo_get_res_type (pkgmgrinfo_pkginfo_h handle, char **return_res_type)
{
  return pkgMgrInfoMockInstance->pkgmgrinfo_pkginfo_get_res_type (handle, return_res_type);
}

/**
 * @brief Mock for pkgmgrinfo_pkginfo_get_res_version
 */
extern "C" int
pkgmgrinfo_pkginfo_get_res_version (pkgmgrinfo_pkginfo_h handle, char **return_res_version)
{
  return pkgMgrInfoMockInstance->pkgmgrinfo_pkginfo_get_res_version (handle, return_res_version);
}

/**
 * @brief pluging-parser test fixture
 */
class PkgMgrInfoMockTestFixture : public ::testing::Test
{
  protected:
  static GModule *module;
  static GTestDBus *dbus;

  /**
   * @brief Set up test suite.
   */
  static void SetUpTestSuite ()
  {
    g_autofree gchar *current_dir = g_get_current_dir ();
    g_autofree gchar *services_dir
        = g_build_filename (current_dir, "tests", "services", NULL);

    dbus = g_test_dbus_new (G_TEST_DBUS_NONE);
    ASSERT_TRUE (dbus != nullptr);

    g_test_dbus_add_service_dir (dbus, services_dir);
    g_test_dbus_up (dbus);

    module = g_module_open ("plugin-parser/libmlops-plugin-parser.so",
        (GModuleFlags) G_MODULE_BIND_LAZY);
    ASSERT_TRUE (module != nullptr);

    pkgMgrInfoMockInstance = new PkgMgrInfoMock ();
  }

  /**
   * @brief Tear down test suite.
   */
  static void TearDownTestSuite ()
  {
    delete pkgMgrInfoMockInstance;
    g_module_close (module);
    g_test_dbus_down (dbus);
    g_object_unref (dbus);
  }

  /**
   * @brief Executing the function from the loaded module, plugin-parser library.
   */
  int exec_plugin_parser_func (const std::string func, const std::string pkgid,
      const std::string appid, GList *metadata)
  {
    int (*func_ptr) (const char *, const char *, GList *);
    if (!g_module_symbol (module, func.c_str (), (gpointer *) &func_ptr)) {
      g_error ("Failed to get symbol %s", func.c_str ());
      return -1;
    }

    return func_ptr (pkgid.c_str (), appid.c_str (), metadata);
  }
};

GTestDBus *PkgMgrInfoMockTestFixture::dbus = nullptr;
GModule *PkgMgrInfoMockTestFixture::module = nullptr;

/**
 * @brief Negative test case of pkgmgrinfo_pkginfo_get_pkginfo() failed.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_n1)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of pkgmgrinfo_pkginfo_get_type() failed.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_n2)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of pkgmgrinfo_pkginfo_get_root_path() failed.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_n3)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillOnce (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of pkgmgrinfo_pkginfo_get_res_type() failed.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_n4)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_type (_, _))
      .WillOnce (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of pkgmgrinfo_pkginfo_get_res_version() failed.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_n5)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_type (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_version (_, _))
      .WillOnce (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of invalid config file.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_n6)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  char root_path[] = "some/invalid/root/path";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (root_path),
                        Return (PMINFO_R_OK)));

  char res_type[] = "some-res-type";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (res_type),
                        Return (PMINFO_R_OK)));

  char res_version[] = "1.5.0";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_version (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (res_version),
                        Return (PMINFO_R_OK)));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of invalid config file.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_n7)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  char root_path[] = "../tests/plugin-parser/test_rpk_samples/invalid_rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (root_path),
                        Return (PMINFO_R_OK)));

  char res_type[] = "sample-res-type";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (res_type),
                        Return (PMINFO_R_OK)));

  char res_version[] = "1.5.0";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_version (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (res_version),
                        Return (PMINFO_R_OK)));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Positive test case of sample rpk package install.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_p1)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  char root_path[] = "../tests/plugin-parser/test_rpk_samples/working_rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (root_path),
                        Return (PMINFO_R_OK)));

  char res_type[] = "sample-res-type";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (res_type),
                        Return (PMINFO_R_OK)));

  char res_version[] = "1.5.0";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_version (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (res_version),
                        Return (PMINFO_R_OK)));

  EXPECT_EQ (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Positive test case of tpk package.
 */
TEST_F (PkgMgrInfoMockTestFixture, test_tpk)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_OK));

  char pkg_type_tpk[] = "tpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillOnce (DoAll (SetArgPointee<1> (pkg_type_tpk),
                        Return (PMINFO_R_OK)));

  EXPECT_EQ (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief GTest main function.
 */
int
main (int argc, char **argv)
{
  int result = -1;

  try {
    testing::InitGoogleTest (&argc, argv);
  } catch (...) {
    g_warning ("catch 'testing::internal::<unnamed>::ClassUniqueToAlwaysTrue'");
  }

  try {
    result = RUN_ALL_TESTS ();
  } catch (...) {
    g_warning ("catch `testing::internal::GoogleTestFailureException`");
  }

  return result;
}
