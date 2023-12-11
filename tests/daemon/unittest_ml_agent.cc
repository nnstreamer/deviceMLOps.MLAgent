/**
 * @file        unittest_ml_agent.cc
 * @date        16 Jul 2022
 * @brief       Unit test for ML-Agent and its interface
 * @see         https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author      Sangjung Woo <sangjung.woo@samsung.com>
 * @bug         No known bugs
 */

#include <gtest/gtest.h>
#include <gio/gio.h>

#include "ml-agent-interface.h"

/**
 * @brief Test base class for ML-Agent.
 */
class MLAgentTest : public ::testing::Test
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
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline)
{
  const gchar pipeline_desc[] = "fakesrc ! fakesink";
  gint ret;
  gint state;
  gint64 id;
  gchar *desc = NULL;

  ret = ml_agent_pipeline_set_description ("test-pipeline", pipeline_desc, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_pipeline_get_description ("test-pipeline", &desc, NULL);
  EXPECT_EQ (ret, 0);
  EXPECT_STREQ (desc, pipeline_desc);
  g_free (desc);

  ret = ml_agent_pipeline_launch ("test-pipeline", &id, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_pipeline_start (id, NULL);
  EXPECT_EQ (ret, 0);
  g_usleep (200000);

  ret = ml_agent_pipeline_stop (id, NULL);
  EXPECT_EQ (ret, 0);
  g_usleep (200000);

  ret = ml_agent_pipeline_get_state (id, &state, NULL);
  EXPECT_EQ (ret, 0);
  g_usleep (200000);

  ret = ml_agent_pipeline_destroy (id, NULL);
  EXPECT_EQ (ret, 0);
  g_usleep (200000);

  ret = ml_agent_pipeline_delete ("test-pipeline", NULL);
  EXPECT_EQ (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_set_description_01_n)
{
  gint ret;

  ret = ml_agent_pipeline_set_description (NULL, "fakesrc ! fakesink", NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_set_description ("", "fakesrc ! fakesink", NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_set_description ("test-pipeline", NULL, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_set_description ("test-pipeline", "", NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_get_description_01_n)
{
  gint ret;
  gchar *desc = NULL;

  ret = ml_agent_pipeline_get_description (NULL, &desc, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_get_description ("", &desc, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_get_description ("test-pipeline", NULL, NULL);
  EXPECT_NE (ret, 0);

  /* no registered pipeline */
  ret = ml_agent_pipeline_get_description ("test-pipeline", &desc, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_delete_01_n)
{
  gint ret;

  ret = ml_agent_pipeline_delete (NULL, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_delete ("", NULL);
  EXPECT_NE (ret, 0);

  /* no registered pipeline */
  ret = ml_agent_pipeline_delete ("test-pipeline", NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_launch_01_n)
{
  gint ret;
  gint64 id;

  ret = ml_agent_pipeline_launch (NULL, &id, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_launch ("", &id, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_pipeline_launch ("test-pipeline", NULL, NULL);
  EXPECT_NE (ret, 0);

  /* no registered pipeline */
  ret = ml_agent_pipeline_launch ("test-pipeline", &id, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_start_01_n)
{
  gint ret;

  /* invalid id */
  ret = ml_agent_pipeline_start (-1, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_stop_01_n)
{
  gint ret;

  /* invalid id */
  ret = ml_agent_pipeline_stop (-1, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_destroy_01_n)
{
  gint ret;

  /* invalid id */
  ret = ml_agent_pipeline_destroy (-1, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - pipeline.
 */
TEST_F (MLAgentTest, pipeline_get_state_01_n)
{
  gint ret;
  gint state;
  gint64 id;

  ret = ml_agent_pipeline_set_description ("test-pipeline", "fakesrc ! fakesink", NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_pipeline_launch ("test-pipeline", &id, NULL);
  EXPECT_EQ (ret, 0);
  g_usleep (200000);

  ret = ml_agent_pipeline_get_state (id, NULL, NULL);
  EXPECT_NE (ret, 0);

  /* invalid id */
  ret = ml_agent_pipeline_get_state (-1, &state, NULL);
  EXPECT_NE (ret, 0);

  ret = ml_agent_pipeline_destroy (id, NULL);
  EXPECT_EQ (ret, 0);
  g_usleep (200000);

  ret = ml_agent_pipeline_delete ("test-pipeline", NULL);
  EXPECT_EQ (ret, 0);
  g_usleep (200000);

  /* no registered pipeline */
  ret = ml_agent_pipeline_get_state (id, &state, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model)
{
  gint ret;
  guint ver1, ver2;
  gchar *model_info = NULL;
  gchar *str;

  ret = ml_agent_model_register ("test-model", "/path/model1.tflite", TRUE, NULL, NULL, &ver1, NULL);
  EXPECT_EQ (ret, 0);
  ret = ml_agent_model_register ("test-model", "/path/model2.tflite", FALSE, NULL, NULL, &ver2, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_model_update_description ("test-model", ver1, "model1desc", NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_model_get ("test-model", ver1, &model_info, NULL);
  EXPECT_EQ (ret, 0);

  str = (model_info != NULL) ? strstr (model_info, "/path/model1.tflite") : NULL;
  EXPECT_TRUE (str != NULL);
  str = (model_info != NULL) ? strstr (model_info, "model1desc") : NULL;
  EXPECT_TRUE (str != NULL);
  g_free (model_info);
  model_info = NULL;

  ret = ml_agent_model_activate ("test-model", ver2, NULL);
  EXPECT_EQ (ret, 0);
  ret = ml_agent_model_get_activated ("test-model", &model_info, NULL);
  EXPECT_EQ (ret, 0);

  str = (model_info != NULL) ? strstr (model_info, "/path/model2.tflite") : NULL;
  EXPECT_TRUE (str != NULL);
  g_free (model_info);
  model_info = NULL;

  ret = ml_agent_model_get_all ("test-model", &model_info, NULL);
  EXPECT_EQ (ret, 0);

  str = (model_info != NULL) ? strstr (model_info, "/path/model1.tflite") : NULL;
  EXPECT_TRUE (str != NULL);
  str = (model_info != NULL) ? strstr (model_info, "/path/model2.tflite") : NULL;
  EXPECT_TRUE (str != NULL);
  g_free (model_info);
  model_info = NULL;

  ret = ml_agent_model_delete ("test-model", 0U, NULL);
  EXPECT_EQ (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model_register_01_n)
{
  gint ret;
  guint ver;

  ret = ml_agent_model_register (NULL, "/path/model.tflite", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_register ("", "/path/model.tflite", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_register ("test-model", NULL, FALSE, NULL, NULL, &ver, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_register ("test-model", "", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_register ("test-model", "/path/model.tflite", FALSE, NULL, NULL, NULL, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model_update_description_01_n)
{
  gint ret;
  guint ver;

  ret = ml_agent_model_register ("test-model", "/path/model.tflite", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_model_update_description (NULL, ver, "desc", NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_update_description ("", ver, "desc", NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_update_description ("test-model", 0U, "desc", NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_update_description ("test-model", ver, NULL, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_update_description ("test-model", ver, "", NULL);
  EXPECT_NE (ret, 0);

  /* invalid version */
  ret = ml_agent_model_update_description ("test-model", (ver + 5U), "desc", NULL);
  EXPECT_NE (ret, 0);

  ret = ml_agent_model_delete ("test-model", 0U, NULL);
  EXPECT_EQ (ret, 0);

  /* no registered model */
  ret = ml_agent_model_update_description ("test-model", ver, "desc", NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model_activate_01_n)
{
  gint ret;
  guint ver;

  ret = ml_agent_model_register ("test-model", "/path/model.tflite", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_model_activate (NULL, ver, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_activate ("", ver, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_activate ("test-model", 0U, NULL);
  EXPECT_NE (ret, 0);

  /* invalid version */
  ret = ml_agent_model_activate ("test-model", (ver + 5U), NULL);
  EXPECT_NE (ret, 0);

  ret = ml_agent_model_delete ("test-model", 0U, NULL);
  EXPECT_EQ (ret, 0);

  /* no registered model */
  ret = ml_agent_model_activate ("test-model", ver, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model_get_01_n)
{
  gint ret;
  guint ver;
  gchar *model_info = NULL;

  ret = ml_agent_model_register ("test-model", "/path/model.tflite", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_model_get (NULL, ver, &model_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_get ("", ver, &model_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_get ("test-model", 0U, &model_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_get ("test-model", ver, NULL, NULL);
  EXPECT_NE (ret, 0);

  /* invalid version */
  ret = ml_agent_model_get ("test-model", (ver + 5U), &model_info, NULL);
  EXPECT_NE (ret, 0);

  ret = ml_agent_model_delete ("test-model", 0U, NULL);
  EXPECT_EQ (ret, 0);

  /* no registered model */
  ret = ml_agent_model_get ("test-model", ver, &model_info, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model_get_activated_01_n)
{
  gint ret;
  guint ver;
  gchar *model_info = NULL;

  ret = ml_agent_model_register ("test-model", "/path/model.tflite", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_model_get_activated (NULL, &model_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_get_activated ("", &model_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_get_activated ("test-model", NULL, NULL);
  EXPECT_NE (ret, 0);

  /* no activated model */
  ret = ml_agent_model_get_activated ("test-model", &model_info, NULL);
  EXPECT_NE (ret, 0);

  ret = ml_agent_model_delete ("test-model", 0U, NULL);
  EXPECT_EQ (ret, 0);

  /* no registered model */
  ret = ml_agent_model_get_activated ("test-model", &model_info, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model_get_all_01_n)
{
  gint ret;
  gchar *model_info = NULL;

  ret = ml_agent_model_get_all (NULL, &model_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_get_all ("", &model_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_get_all ("test-model", NULL, NULL);
  EXPECT_NE (ret, 0);

  /* no registered model */
  ret = ml_agent_model_get_all ("test-model", &model_info, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - model.
 */
TEST_F (MLAgentTest, model_delete_01_n)
{
  gint ret;
  guint ver;

  ret = ml_agent_model_register ("test-model", "/path/model.tflite", FALSE, NULL, NULL, &ver, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_model_delete (NULL, ver, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_model_delete ("", ver, NULL);
  EXPECT_NE (ret, 0);

  /* invalid version */
  ret = ml_agent_model_delete ("test-model", (ver + 5U), NULL);
  EXPECT_NE (ret, 0);

  ret = ml_agent_model_delete ("test-model", 0U, NULL);
  EXPECT_EQ (ret, 0);

  /* no registered model */
  ret = ml_agent_model_delete ("test-model", 0U, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - resource.
 */
TEST_F (MLAgentTest, resource)
{
  gint ret;
  gchar *res_info = NULL;
  gchar *str;

  ret = ml_agent_resource_add ("test-res", "/path/res1.dat", NULL, NULL, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_resource_get ("test-res", &res_info, NULL);
  EXPECT_EQ (ret, 0);

  str = (res_info != NULL) ? strstr (res_info, "/path/res1.dat") : NULL;
  EXPECT_TRUE (str != NULL);
  str = (res_info != NULL) ? strstr (res_info, "/path/res2.dat") : NULL;
  EXPECT_FALSE (str != NULL);
  g_free (res_info);
  res_info = NULL;

  ret = ml_agent_resource_add ("test-res", "/path/res2.dat", "res2desc", NULL, NULL);
  EXPECT_EQ (ret, 0);

  ret = ml_agent_resource_get ("test-res", &res_info, NULL);
  EXPECT_EQ (ret, 0);

  str = (res_info != NULL) ? strstr (res_info, "/path/res1.dat") : NULL;
  EXPECT_TRUE (str != NULL);
  str = (res_info != NULL) ? strstr (res_info, "/path/res2.dat") : NULL;
  EXPECT_TRUE (str != NULL);
  str = (res_info != NULL) ? strstr (res_info, "res2desc") : NULL;
  EXPECT_TRUE (str != NULL);
  g_free (res_info);
  res_info = NULL;

  ret = ml_agent_resource_delete ("test-res", NULL);
  EXPECT_EQ (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - resource.
 */
TEST_F (MLAgentTest, resource_add_01_n)
{
  gint ret;

  ret = ml_agent_resource_add (NULL, "/path/res.dat", NULL, NULL, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_resource_add ("", "/path/res.dat", NULL, NULL, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_resource_add ("test-res", NULL, NULL, NULL, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_resource_add ("test-res", "", NULL, NULL, NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - resource.
 */
TEST_F (MLAgentTest, resource_delete_01_n)
{
  gint ret;

  ret = ml_agent_resource_delete (NULL, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_resource_delete ("", NULL);
  EXPECT_NE (ret, 0);

  /* no registered resource */
  ret = ml_agent_resource_delete ("test-res", NULL);
  EXPECT_NE (ret, 0);
}

/**
 * @brief Testcase for ML-Agent interface - resource.
 */
TEST_F (MLAgentTest, resource_get_01_n)
{
  gint ret;
  gchar *res_info = NULL;

  ret = ml_agent_resource_get (NULL, &res_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_resource_get ("", &res_info, NULL);
  EXPECT_NE (ret, 0);
  ret = ml_agent_resource_get ("test-res", NULL, NULL);
  EXPECT_NE (ret, 0);

  /* no registered resource */
  ret = ml_agent_resource_get ("test-res", &res_info, NULL);
  EXPECT_NE (ret, 0);
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
