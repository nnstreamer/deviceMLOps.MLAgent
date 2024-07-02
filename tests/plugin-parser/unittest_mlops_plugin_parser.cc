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
#include <glib/gstdio.h>
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
 * @brief plugin-parser test fixture
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

  /**
   * @brief Make a new file with given path and set its content with given value.
   */
  bool create_and_set_file (const gchar *path, const gchar *value) {
    bool ret = true;
    GFile *new_file = g_file_new_for_path (path);
    GError *error = NULL;

    if (!g_file_set_contents (g_file_get_path (new_file), value, -1, &error)) {
      g_warning ("Failed to create/write file '%s': %s\n", path, error->message);
      g_clear_error (&error);
      ret = false;
    }
    g_object_unref (new_file);

    return ret;
  }
};

GTestDBus *PkgMgrInfoMockTestFixture::dbus = nullptr;
GModule *PkgMgrInfoMockTestFixture::module = nullptr;

/**
 * @brief Negative test case of pkgmgrinfo_pkginfo_get_pkginfo() failed.
 */
TEST_F (PkgMgrInfoMockTestFixture, rpk_install1_n)
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
TEST_F (PkgMgrInfoMockTestFixture, rpk_install2_n)
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
TEST_F (PkgMgrInfoMockTestFixture, rpk_install3_n)
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
TEST_F (PkgMgrInfoMockTestFixture, rpk_install4_n)
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
TEST_F (PkgMgrInfoMockTestFixture, rpk_install5_n)
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
TEST_F (PkgMgrInfoMockTestFixture, rpk_install6_n)
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
 * @brief Negative test case of various invalid config files.
 */
TEST_F (PkgMgrInfoMockTestFixture, rpk_install7_n)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillRepeatedly (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  char root_path[] = "../tests/plugin-parser/test_rpk_samples/invalid_rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (root_path),
                        Return (PMINFO_R_OK)));

  char res_type[] = "sample-res-type";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_type (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (res_type),
                        Return (PMINFO_R_OK)));

  char res_version[] = "1.5.0";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_version (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (res_version),
                        Return (PMINFO_R_OK)));

  /* create and set json file with invalid value */
  g_autofree gchar *rpk_config_dir_path = g_strdup_printf ("%s/res/global/%s/", root_path, res_type);
  g_autofree gchar *config_file_path = g_strdup_printf ("%s/rpk_config.json", rpk_config_dir_path);

  /* test 1 : invalid json format */
  const gchar *json_with_invalid_format = R""""(
{
  some invalid json
}
)"""";
  ASSERT_TRUE (create_and_set_file (config_file_path, json_with_invalid_format));
  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);

  if (g_remove (config_file_path) != 0) {
    g_printerr ("Error removing file: %s\n", g_strerror (errno));
    ASSERT_TRUE (false);
  }

  /* test 2 : model has no 'name' field */
  const gchar *json_with_no_name = R""""(
{
  "models" : {
    "model" : "dummy-global.tflite",
    "description" : "No name field, invalid rpk",
    "activate" : "true"
  }
}
)"""";
  ASSERT_TRUE (create_and_set_file (config_file_path, json_with_no_name));
  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);

  if (g_remove (config_file_path) != 0) {
    g_printerr ("Error removing file: %s\n", g_strerror (errno));
    ASSERT_TRUE (false);
  }

  /* test 3 : pipeline has no 'name' field */
  const gchar *json_with_no_name_pipeline = R""""(
{
  "pipelines" : {
    "pipeline" : "pipe ! line",
    "description" : "No name field, invalid rpk"
  }
}
)"""";
  ASSERT_TRUE (create_and_set_file (config_file_path, json_with_no_name_pipeline));
  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);

  if (g_remove (config_file_path) != 0) {
    g_printerr ("Error removing file: %s\n", g_strerror (errno));
    ASSERT_TRUE (false);
  }

  /* test 4 : resource has no 'name' field */
  const gchar *json_with_no_name_resource = R""""(
{
  "resources" : {
    "description" : "No name filed, invalid rpk",
    "path" : [
      "resource_00.dat"
    ]
  }
}
)"""";
  ASSERT_TRUE (create_and_set_file (config_file_path, json_with_no_name_resource));
  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);

  if (g_remove (config_file_path) != 0) {
    g_printerr ("Error removing file: %s\n", g_strerror (errno));
    ASSERT_TRUE (false);
  }
}

/**
 * @brief Positive test case of tpk package.
 */
TEST_F (PkgMgrInfoMockTestFixture, tpk_install_p1)
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
 * @brief Positive test case of sample rpk package install/upgrade/uninstall.
 */
TEST_F (PkgMgrInfoMockTestFixture, rpk_scenario_p1)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL (*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_destroy_pkginfo (_))
      .WillRepeatedly (Return (PMINFO_R_OK));

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillRepeatedly (Return (PMINFO_R_OK));

  char pkg_type_rpk[] = "rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_type (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (pkg_type_rpk),
                        Return (PMINFO_R_OK)));

  char root_path[] = "../tests/plugin-parser/test_rpk_samples/working_rpk";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_root_path (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (root_path),
                        Return (PMINFO_R_OK)));

  char res_type[] = "sample-res-type";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_type (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (res_type),
                        Return (PMINFO_R_OK)));

  char res_version[] = "1.5.0";
  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_res_version (_, _))
      .WillRepeatedly (DoAll (SetArgPointee<1> (res_version),
                        Return (PMINFO_R_OK)));

  /* install rpk */
  EXPECT_EQ (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_INSTALL", pkgid, appid, NULL), 0);

  /* upgrade rpk */
  EXPECT_EQ (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_UPGRADE", pkgid, appid, NULL), 0);

  /* uninstall the rpk */
  EXPECT_EQ (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_UNINSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of RECOVERINSTALL.
 */
TEST_F (PkgMgrInfoMockTestFixture, RECOVERINSTALL_n)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_RECOVERINSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of RECOVERUPGRADE.
 */
TEST_F (PkgMgrInfoMockTestFixture, RECOVERUPGRADE_n)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillRepeatedly (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_RECOVERUPGRADE", pkgid, appid, NULL), 0);
}

/**
 * @brief Negative test case of RECOVERUNINSTALL.
 */
TEST_F (PkgMgrInfoMockTestFixture, RECOVERUNINSTALL_n)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_CALL(*pkgMgrInfoMockInstance, pkgmgrinfo_pkginfo_get_pkginfo (_, _))
      .WillOnce (Return (PMINFO_R_ERROR));

  EXPECT_NE (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_RECOVERUNINSTALL", pkgid, appid, NULL), 0);
}

/**
 * @brief Positive test case of CLEAN.
 */
TEST_F (PkgMgrInfoMockTestFixture, clean_p1)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_EQ (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_CLEAN", pkgid, appid, NULL), 0);
}

/**
 * @brief Positive test case of UNDO.
 */
TEST_F (PkgMgrInfoMockTestFixture, undo_p1)
{
  std::string pkgid = "pkgid";
  std::string appid = "appid";

  EXPECT_EQ (exec_plugin_parser_func ("PKGMGR_MDPARSER_PLUGIN_UNDO", pkgid, appid, NULL), 0);
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
