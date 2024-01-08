/**
 * @file        unittest_gdbus_util.cc
 * @date        2 May 2023
 * @brief       Unit test for GDBus utility functions
 * @see         https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author      Wook Song <wook16.song@samsung.com>
 * @bug         No known bugs
 */

#include <gtest/gtest.h>

#include "dbus-interface.h"
#include "gdbus-util.h"
#include "log.h"

/**
 * @brief Test base class for GDbus.
 */
class GDbusTest : public ::testing::Test
{
  protected:
  GTestDBus *dbus;

  public:
  /**
   * @brief Setup method for each test case.
   */
  void SetUp () override
  {
    g_autofree gchar *current_dir = g_get_current_dir ();
    g_autofree gchar *services_dir = g_build_filename (current_dir, "tests", "services", NULL);

    dbus = g_test_dbus_new (G_TEST_DBUS_NONE);
    ASSERT_TRUE (dbus != nullptr);

    g_test_dbus_add_service_dir (dbus, services_dir);
    g_test_dbus_up (dbus);
  }

  /**
   * @brief Teardown method for each test case.
   */
  void TearDown () override
  {
    if (dbus) {
      g_test_dbus_down (dbus);
      g_object_unref (dbus);
      dbus = nullptr;
    }
  }
};

/**
 * @brief Negative test for the gdbus helper function, gdbus_export_interface().
 */
TEST (GDbusInstanceNotInitialized, export_interface_n)
{
  int ret;

  ret = gdbus_export_interface (nullptr, DBUS_PIPELINE_PATH);
  EXPECT_EQ (-ENOSYS, ret);

  ret = gdbus_export_interface (nullptr, DBUS_MODEL_PATH);
  EXPECT_EQ (-ENOSYS, ret);

  ret = gdbus_get_name (DBUS_ML_BUS_NAME);
  EXPECT_EQ (-ENOSYS, ret);
}

/**
 * @brief Negative test for the gdbus helper function, get_system_connection().
 */
TEST (GDbusInstanceNotInitialized, get_system_connection_n)
{
  int ret;

  ret = gdbus_get_system_connection (true);
  EXPECT_EQ (-ENOSYS, ret);

  ret = gdbus_get_system_connection (false);
  EXPECT_EQ (-ENOSYS, ret);
}

/**
 * @brief Main gtest
 */
int
main (int argc, char **argv)
{
  int result = -1;

  try {
    testing::InitGoogleTest (&argc, argv);
  } catch (...) {
    ml_logw ("catch 'testing::internal::<unnamed>::ClassUniqueToAlwaysTrue'");
  }

  try {
    result = RUN_ALL_TESTS ();
  } catch (...) {
    ml_logw ("catch `testing::internal::GoogleTestFailureException`");
  }

  return result;
}
