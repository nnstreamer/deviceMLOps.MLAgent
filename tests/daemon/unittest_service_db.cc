/**
 * @file        unittest_service_db.cc
 * @date        21 Mar 2023
 * @brief       Unit test for service DB used by ML Agent
 * @see         https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author      Yongjoo Ahn <yongjoo1.ahn@samsung.com>
 * @bug         No known bugs
 */

#include <gtest/gtest.h>
#include <gio/gio.h>

#include "log.h"
#include "service-db.hh"
#include "service-db-util.h"

#define TEST_DB_PATH "."

/**
 * @brief Negative test for set_pipeline. Invalid param case (empty name or description).
 */
TEST (serviceDB, set_pipeline_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    db.set_pipeline ("", "videotestsrc ! fakesink");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.set_pipeline ("test_key", "");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for get_pipeline. Invalid param case (empty name).
 */
TEST (serviceDB, get_pipeline_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    gchar *pipeline_description;
    db.get_pipeline ("", &pipeline_description);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.get_pipeline ("test", NULL);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for delete_pipeline. Invalid param case (empty name).
 */
TEST (serviceDB, delete_pipeline_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    db.delete_pipeline ("");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for set_model. Invalid param case (empty name, model or version).
 */
TEST (serviceDB, set_model_n)
{
  MLServiceDB db (TEST_DB_PATH);
  guint version;

  db.connectDB ();

  try {
    db.set_model ("", "model", true, "description", "", &version);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.set_model ("test", "", true, "description", "", &version);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.set_model ("test", "model", true, "", "", NULL);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Check model update.
 */
TEST (serviceDB, update_model_scenario)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  /* No exception to add, get, and delete model with name 'test'. */
  try {
    gchar *model_info;
    gchar *pos;
    guint version, version_active;

    db.set_model ("test", "test_model1", true, "model1_description", "", &version_active);
    db.set_model ("test", "test_model2", false, "model2_description", "", &version);

    /* Check model info contains added string. */
    db.get_model ("test", 0, &model_info);
    pos = g_strstr_len (model_info, -1, "test_model1");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (model_info, -1, "test_model2");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (model_info, -1, "model1_description");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (model_info, -1, "model2_description");
    EXPECT_TRUE (pos != NULL);
    g_free (model_info);

    db.get_model ("test", version, &model_info);
    pos = g_strstr_len (model_info, -1, "test_model2");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (model_info, -1, "model2_description");
    EXPECT_TRUE (pos != NULL);
    g_free (model_info);

    db.get_model ("test", -1, &model_info);
    pos = g_strstr_len (model_info, -1, "test_model1");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (model_info, -1, "model1_description");
    EXPECT_TRUE (pos != NULL);
    g_free (model_info);

    db.activate_model ("test", version);
    db.update_model_description ("test", version, "updated_desc_model2");
    db.get_model ("test", -1, &model_info);
    pos = g_strstr_len (model_info, -1, "test_model2");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (model_info, -1, "updated_desc_model2");
    EXPECT_TRUE (pos != NULL);
    g_free (model_info);

    db.delete_model ("test", 0);
  } catch (const std::exception &e) {
    FAIL ();
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for get_model. Invalid param case (empty name or invalid version).
 */
TEST (serviceDB, get_model_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    gchar *model_description;
    db.get_model ("", 0, &model_description);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    gchar *model_description;
    db.get_model ("test", -54321, &model_description);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.get_model ("test", 0, NULL);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for update_model_description. Invalid param case (empty name or description, invalid version).
 */
TEST (serviceDB, update_model_description_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    db.update_model_description ("", 1, "description");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.update_model_description ("test", 1, "");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.update_model_description ("test", 0, "description");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for activate_model. Invalid param case (empty name or invalid version).
 */
TEST (serviceDB, activate_model_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    db.activate_model ("", 1);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.activate_model ("test", 0);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for delete_model. Invalid param case (rmpty name).
 */
TEST (serviceDB, delete_model_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    db.delete_model ("", 0);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for delete_model. Model is not registered.
 */
TEST (serviceDB, delete_model_unregistered_n)
{
  MLServiceDB db (TEST_DB_PATH);
  guint version;

  db.connectDB ();

  /* Test condition, remove all model with name 'test'. */
  db.set_model ("test", "test_model", true, "", "", &version);
  db.delete_model ("test", 0U);

  try {
    db.delete_model ("test", version);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for delete_model. Model is activated.
 */
TEST (serviceDB, delete_model_activated_n)
{
  MLServiceDB db (TEST_DB_PATH);
  guint version;

  db.connectDB ();

  /* Test condition, add new model as activated. */
  db.set_model ("test", "test_model", true, "", "", &version);

  try {
    db.delete_model ("test", version);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.delete_model ("test", 0U);
  db.disconnectDB ();
}

/**
 * @brief Negative test for set_pipline. DB is not initialized.
 */
TEST (serviceDBNotInitalized, set_pipeline_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    db.set_pipeline ("test", "videotestsrc ! fakesink");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for get_pipline. DB is not initialized.
 */
TEST (serviceDBNotInitalized, get_pipeline_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    gchar *pd;
    db.get_pipeline ("test", &pd);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for delete_pipeline. DB is not initialized.
 */
TEST (serviceDBNotInitalized, delete_pipeline_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    db.delete_pipeline ("test");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for set_model. DB is not initialized.
 */
TEST (serviceDBNotInitalized, set_model_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    guint version;
    db.set_model ("test", "model", true, "description", "", &version);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for update_model_description. DB is not initialized.
 */
TEST (serviceDBNotInitalized, update_model_description_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    db.update_model_description ("test", 0, "description");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for activate_model. DB is not initialized.
 */
TEST (serviceDBNotInitalized, activate_model_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    db.activate_model ("test", 0);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for get_model. DB is not initialized.
 */
TEST (serviceDBNotInitalized, get_model_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    gchar *model_path;
    db.get_model ("test", 0, &model_path);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for delete_model. DB is not initialized.
 */
TEST (serviceDBNotInitalized, delete_model_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    db.delete_model ("test", 0U);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for set_resource. Invalid param case (empty name or path).
 */
TEST (serviceDB, set_resource_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    db.set_resource ("", "resource", "description", "");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.set_resource ("test", "", "description", "");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Check resources.
 */
TEST (serviceDB, update_resource_scenario)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  /* No exception to add, get, and delete resources with name 'test'. */
  try {
    gchar *res_info;
    gchar *pos;

    db.set_resource ("test", "test_resource1", "res1_description", "");
    db.set_resource ("test", "test_resource2", "res2_description", "");

    /* Check res info contains added string. */
    db.get_resource ("test", &res_info);
    pos = g_strstr_len (res_info, -1, "test_resource1");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (res_info, -1, "test_resource2");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (res_info, -1, "res1_description");
    EXPECT_TRUE (pos != NULL);
    pos = g_strstr_len (res_info, -1, "res2_description");
    EXPECT_TRUE (pos != NULL);
    g_free (res_info);

    db.set_resource ("test", "test_resource2", "updated_desc_res2", "");
    db.get_resource ("test", &res_info);
    pos = g_strstr_len (res_info, -1, "updated_desc_res2");
    EXPECT_TRUE (pos != NULL);
    g_free (res_info);

    db.delete_resource ("test");
  } catch (const std::exception &e) {
    FAIL ();
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for get_resource. Invalid param case (empty name).
 */
TEST (serviceDB, get_resource_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    gchar *res_description;
    db.get_resource ("", &res_description);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  try {
    db.get_resource ("test", NULL);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for get_resource. Invalid param case (empty name or unregistered name).
 */
TEST (serviceDB, get_resource_unregistered_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  /* Test condition, remove all resource with name 'test'. */
  db.set_resource ("test", "test_resource", "", "");
  db.delete_resource ("test");

  try {
    gchar *res_description;
    db.get_resource ("test", &res_description);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for delete_resource. Invalid param case (empty name).
 */
TEST (serviceDB, delete_resource_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  try {
    db.delete_resource ("");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for delete_resource. Resource is not registered.
 */
TEST (serviceDB, delete_resource_unregistered_n)
{
  MLServiceDB db (TEST_DB_PATH);

  db.connectDB ();

  /* Test condition, remove all resource with name 'test'. */
  db.set_resource ("test", "test_resource", "", "");
  db.delete_resource ("test");

  try {
    db.delete_resource ("test");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }

  db.disconnectDB ();
}

/**
 * @brief Negative test for set_resource. DB is not initialized.
 */
TEST (serviceDBNotInitalized, set_resource_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    db.set_resource ("test", "resource", "description", "");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for get_resource. DB is not initialized.
 */
TEST (serviceDBNotInitalized, get_resource_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    gchar *res_description;
    db.get_resource ("test", &res_description);
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for delete_resource. DB is not initialized.
 */
TEST (serviceDBNotInitalized, delete_resource_n)
{
  MLServiceDB db (TEST_DB_PATH);

  try {
    db.delete_resource ("test");
    FAIL ();
  } catch (const std::exception &e) {
    /* expected */
  }
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, pipeline_set_n)
{
  gint ret;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_pipeline_set ("", "videotestsrc ! fakesink");
  EXPECT_NE (ret, 0);
  ret = svcdb_pipeline_set (NULL, "videotestsrc ! fakesink");
  EXPECT_NE (ret, 0);
  ret = svcdb_pipeline_set ("test", "");
  EXPECT_NE (ret, 0);
  ret = svcdb_pipeline_set ("test", NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, pipeline_get_n)
{
  gint ret;
  gchar *desc;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_pipeline_get ("", &desc);
  EXPECT_NE (ret, 0);
  ret = svcdb_pipeline_get (NULL, &desc);
  EXPECT_NE (ret, 0);
  ret = svcdb_pipeline_get ("test", NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, pipeline_delete_n)
{
  gint ret;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_pipeline_delete ("");
  EXPECT_NE (ret, 0);
  ret = svcdb_pipeline_delete (NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, model_add_n)
{
  gint ret;
  guint version;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_model_add ("", "model", true, "description", "", &version);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_add (NULL, "model", true, "description", "", &version);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_add ("test", "", true, "description", "", &version);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_add ("test", NULL, true, "description", "", &version);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_add ("test", "model", true, "", "", NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, model_update_description_n)
{
  gint ret;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_model_update_description ("", 1, "description");
  EXPECT_NE (ret, 0);
  ret = svcdb_model_update_description (NULL, 1, "description");
  EXPECT_NE (ret, 0);
  ret = svcdb_model_update_description ("test", 1, "");
  EXPECT_NE (ret, 0);
  ret = svcdb_model_update_description ("test", 1, NULL);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_update_description ("test", 0, "description");
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, model_activate_n)
{
  gint ret;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_model_activate ("", 1);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_activate (NULL, 1);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_activate ("test", 0);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, model_get_n)
{
  gint ret;
  gchar *model_info;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_model_get ("", 1, &model_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_get (NULL, 1, &model_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_get ("test", 1, NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, model_get_activated_n)
{
  gint ret;
  gchar *model_info;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_model_get_activated ("", &model_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_get_activated (NULL, &model_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_get_activated ("test", NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, model_get_all_n)
{
  gint ret;
  gchar *model_info;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_model_get_all ("", &model_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_get_all (NULL, &model_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_get_all ("test", NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, model_delete_n)
{
  gint ret;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_model_delete ("", 0U);
  EXPECT_NE (ret, 0);
  ret = svcdb_model_delete (NULL, 0U);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, resource_add_n)
{
  gint ret;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_resource_add ("", "resource", "description", "");
  EXPECT_NE (ret, 0);
  ret = svcdb_resource_add (NULL, "resource", "description", "");
  EXPECT_NE (ret, 0);
  ret = svcdb_resource_add ("test", "", "description", "");
  EXPECT_NE (ret, 0);
  ret = svcdb_resource_add ("test", NULL, "description", "");
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, resource_get_n)
{
  gint ret;
  gchar *res_info;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_resource_get ("", &res_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_resource_get (NULL, &res_info);
  EXPECT_NE (ret, 0);
  ret = svcdb_resource_get ("test", NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
}

/**
 * @brief Negative test for service-db util. Invalid param case.
 */
TEST (serviceDBUtil, resource_delete_n)
{
  gint ret;

  svcdb_initialize (TEST_DB_PATH);

  ret = svcdb_resource_delete ("");
  EXPECT_NE (ret, 0);
  ret = svcdb_resource_delete (NULL);
  EXPECT_NE (ret, 0);

  svcdb_finalize ();
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
