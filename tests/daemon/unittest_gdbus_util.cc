/**
 * @file        unittest_gdbus_util.cc
 * @date        2 May 2023
 * @brief       Unit test for GDBus utility functions
 * @see         https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author      Wook Song <wook16.song@samsung.com>
 * @bug         No known bugs
 */

#include <gtest/gtest.h>

#include "../dbus/test-dbus-interface.h"
#include "dbus-interface.h"
#include "gdbus-util.h"
#include "test-dbus.h"

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
 * @brief Call the 'get_state' DBus method and check the result.
 */
TEST_F (GDbusTest, call_method)
{
  MachinelearningServiceTest *proxy = NULL;
  GError *error = NULL;
  int status = 0;
  int result = 0;

  /* Test : Connect to the DBus interface */
  proxy = machinelearning_service_test_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
      G_DBUS_PROXY_FLAGS_NONE, DBUS_ML_BUS_NAME, DBUS_TEST_PATH, NULL, &error);
  if (error != NULL) {
    g_error_free (error);
    FAIL ();
  }

  /* Test: Call the DBus method */
  machinelearning_service_test_call_get_state_sync (proxy, &status, &result, NULL, &error);
  if (error != NULL) {
    g_critical ("Error : %s", error->message);
    g_error_free (error);
    FAIL ();
  }

  /* Check the return value */
  EXPECT_EQ (result, 0);
  EXPECT_EQ (status, 1);

  g_object_unref (proxy);
}

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
    g_warning ("catch 'testing::internal::<unnamed>::ClassUniqueToAlwaysTrue'");
  }

  try {
    result = RUN_ALL_TESTS ();
  } catch (...) {
    g_warning ("catch `testing::internal::GoogleTestFailureException`");
  }

  return result;
}
