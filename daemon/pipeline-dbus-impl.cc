/* SPDX-License-Identifier: Apache-2.0 */
/**
 * NNStreamer API / Machine Learning Agent Daemon
 * Copyright (C) 2022 Samsung Electronics Co., Ltd. All Rights Reserved.
 */

/**
 * @file      pipeline-dbus-impl.cc
 * @date      20 Jul 2022
 * @brief     Implementation of pipeline dbus interface.
 * @see       https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author    Yongjoo Ahn <yongjoo1.ahn@samsung.com>
 * @bug       No known bugs except for NYI items
 * @details   This implements the pipeline dbus interface.
 */

#include <glib.h>
#include <gst/gst.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "dbus-interface.h"
#include "gdbus-util.h"
#include "log.h"
#include "modules.h"
#include "pipeline-dbus.h"
#include "service-db-util.h"

static MachinelearningServicePipeline *g_gdbus_instance = NULL;
static GHashTable *pipeline_table = NULL;
G_LOCK_DEFINE_STATIC (pipeline_table_lock);

/**
 * @brief Structure for pipeline.
 */
typedef struct _pipeline {
  GstElement *element;
  gint64 id;
  GMutex lock;
  gchar *service_name;
  gchar *description;
} pipeline_s;

/**
 * @brief Internal function to destroy pipeline instances.
 */
static void
_pipeline_free (gpointer data)
{
  pipeline_s *p;

  if (!data) {
    ml_loge ("internal error, the data should not be NULL");
    return;
  }

  p = (pipeline_s *) data;

  if (p->element)
    gst_object_unref (p->element);

  g_free (p->service_name);
  g_free (p->description);
  g_mutex_clear (&p->lock);

  g_free (p);
}

/**
 * @brief Get the skeleton object of the DBus interface.
 */
static MachinelearningServicePipeline *
gdbus_get_pipeline_instance (void)
{
  return machinelearning_service_pipeline_skeleton_new ();
}

/**
 * @brief Put the obtained skeleton object and release the resource.
 */
static void
gdbus_put_pipeline_instance (MachinelearningServicePipeline **instance)
{
  g_clear_object (instance);
}

/**
 * @brief Set the service with given description. Return the call result.
 */
static gboolean
dbus_cb_core_set_pipeline (MachinelearningServicePipeline *obj, GDBusMethodInvocation *invoc,
    const gchar *service_name, const gchar *pipeline_desc, gpointer user_data)
{
  gint result = 0;

  result = svcdb_pipeline_set (service_name, pipeline_desc);
  machinelearning_service_pipeline_complete_set_pipeline (obj, invoc, result);

  return TRUE;
}

/**
 * @brief Get the pipeline description of the given service. Return the call result and the pipeline description.
 */
static gboolean
dbus_cb_core_get_pipeline (MachinelearningServicePipeline *obj,
    GDBusMethodInvocation *invoc, const gchar *service_name, gpointer user_data)
{
  gint result = 0;
  g_autofree gchar *desc = NULL;

  result = svcdb_pipeline_get (service_name, &desc);
  machinelearning_service_pipeline_complete_get_pipeline (obj, invoc, result, desc);

  return TRUE;
}

/**
 * @brief Delete the pipeline description of the given service. Return the call result.
 */
static gboolean
dbus_cb_core_delete_pipeline (MachinelearningServicePipeline *obj,
    GDBusMethodInvocation *invoc, const gchar *service_name, gpointer user_data)
{
  gint result = 0;

  result = svcdb_pipeline_delete (service_name);
  machinelearning_service_pipeline_complete_delete_pipeline (obj, invoc, result);

  return TRUE;
}

/**
 * @brief Launch the pipeline with given description. Return the call result and its id.
 */
static gboolean
dbus_cb_core_launch_pipeline (MachinelearningServicePipeline *obj,
    GDBusMethodInvocation *invoc, const gchar *service_name, gpointer user_data)
{
  gint result = 0;
  gint64 id = -1;
  GError *err = NULL;
  GstStateChangeReturn sc_ret;
  GstElement *pipeline = NULL;
  pipeline_s *p;
  g_autofree gchar *desc = NULL;

  result = svcdb_pipeline_get (service_name, &desc);
  if (result != 0) {
    ml_loge ("Failed to launch pipeline of '%s'.", service_name);
    goto error;
  }

  pipeline = gst_parse_launch (desc, &err);
  if (!pipeline || err) {
    ml_loge ("Failed to launch pipeline '%s' (error msg: %s).",
        desc, (err) ? err->message : "unknown reason");
    g_clear_error (&err);

    if (pipeline)
      gst_object_unref (pipeline);

    result = -ESTRPIPE;
    goto error;
  }

  /** now set pipeline as paused state */
  sc_ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
  if (sc_ret == GST_STATE_CHANGE_FAILURE) {
    ml_loge ("Failed to set the state of the pipeline to PAUSED. For the detail, please check the GStreamer log message. The input pipeline was '%s'.",
        desc);

    gst_object_unref (pipeline);
    result = -ESTRPIPE;
    goto error;
  }

  /** now fill the struct and store into hash table */
  p = g_new0 (pipeline_s, 1);
  p->element = pipeline;
  p->description = g_strdup (desc);
  p->service_name = g_strdup (service_name);
  g_mutex_init (&p->lock);

  G_LOCK (pipeline_table_lock);
  id = p->id = g_get_monotonic_time ();
  g_hash_table_insert (pipeline_table, GINT_TO_POINTER (p->id), p);
  G_UNLOCK (pipeline_table_lock);

error:
  machinelearning_service_pipeline_complete_launch_pipeline (obj, invoc, result, id);

  return TRUE;
}

/**
 * @brief Start the pipeline with given id. Return the call result.
 */
static gboolean
dbus_cb_core_start_pipeline (MachinelearningServicePipeline *obj,
    GDBusMethodInvocation *invoc, gint64 id, gpointer user_data)
{
  gint result = 0;
  GstStateChangeReturn sc_ret;
  pipeline_s *p = NULL;

  G_LOCK (pipeline_table_lock);
  p = (pipeline_s *) g_hash_table_lookup (pipeline_table, GINT_TO_POINTER (id));

  if (!p) {
    ml_loge ("The callback start_pipeline is called, but there is no pipeline matched with ID.");
    G_UNLOCK (pipeline_table_lock);
    result = -EINVAL;
  } else {
    g_mutex_lock (&p->lock);
    G_UNLOCK (pipeline_table_lock);
    sc_ret = gst_element_set_state (p->element, GST_STATE_PLAYING);
    g_mutex_unlock (&p->lock);

    if (sc_ret == GST_STATE_CHANGE_FAILURE) {
      ml_loge ("Failed to set the state of the pipeline to PLAYING whose service name is %s.",
          p->service_name);
      result = -ESTRPIPE;
    }
  }

  machinelearning_service_pipeline_complete_start_pipeline (obj, invoc, result);

  return TRUE;
}

/**
 * @brief Stop the pipeline with given id. Return the call result.
 */
static gboolean
dbus_cb_core_stop_pipeline (MachinelearningServicePipeline *obj,
    GDBusMethodInvocation *invoc, gint64 id, gpointer user_data)
{
  gint result = 0;
  GstStateChangeReturn sc_ret;
  pipeline_s *p = NULL;

  G_LOCK (pipeline_table_lock);
  p = (pipeline_s *) g_hash_table_lookup (pipeline_table, GINT_TO_POINTER (id));

  if (!p) {
    ml_loge ("The callback stop_pipeline is called, but there is no pipeline matched with ID.");
    G_UNLOCK (pipeline_table_lock);
    result = -EINVAL;
  } else {
    g_mutex_lock (&p->lock);
    G_UNLOCK (pipeline_table_lock);
    sc_ret = gst_element_set_state (p->element, GST_STATE_PAUSED);
    g_mutex_unlock (&p->lock);

    if (sc_ret == GST_STATE_CHANGE_FAILURE) {
      ml_loge ("Failed to set the state of the pipeline to PAUSED whose service name is %s.",
          p->service_name);
      result = -ESTRPIPE;
    }
  }

  machinelearning_service_pipeline_complete_stop_pipeline (obj, invoc, result);

  return TRUE;
}

/**
 * @brief Destroy the pipeline with given id. Return the call result.
 */
static gboolean
dbus_cb_core_destroy_pipeline (MachinelearningServicePipeline *obj,
    GDBusMethodInvocation *invoc, gint64 id, gpointer user_data)
{
  gint result = 0;
  pipeline_s *p = NULL;

  G_LOCK (pipeline_table_lock);
  p = (pipeline_s *) g_hash_table_lookup (pipeline_table, GINT_TO_POINTER (id));

  if (!p) {
    ml_loge ("The callback destroy_pipeline is called, but there is no pipeline matched with ID.");
    result = -EINVAL;
  } else {
    /**
     * @todo Fix hanging issue when trying to set GST_STATE_NULL state for pipelines
     * containing tensor_query_*. As a workaround, just unref the pipeline instance.
     * To fix this issue, tensor_query elements and nnstreamer-edge should well-behavior
     * to the state change. And it should properly free socket resources. Revive following code after then.
     *
     *   GstStateChangeReturn sc_ret;
     *   g_mutex_lock (&p->lock);
     *   sc_ret = gst_element_set_state (p->element, GST_STATE_NULL);
     *   g_mutex_unlock (&p->lock);
     *   if (sc_ret == GST_STATE_CHANGE_FAILURE) {
     *     ml_loge ("Failed to set the state of the pipeline to NULL whose service name is %s. Destroy it anyway.", p->service_name);
     *     result = -ESTRPIPE;
     *   }
     */
    g_hash_table_remove (pipeline_table, GINT_TO_POINTER (id));
  }

  G_UNLOCK (pipeline_table_lock);
  machinelearning_service_pipeline_complete_destroy_pipeline (obj, invoc, result);

  return TRUE;
}

/**
 * @brief Get the state of pipeline with given id. Return the call result and its state.
 */
static gboolean
dbus_cb_core_get_state (MachinelearningServicePipeline *obj,
    GDBusMethodInvocation *invoc, gint64 id, gpointer user_data)
{
  gint result = 0;
  GstStateChangeReturn sc_ret;
  GstState state = GST_STATE_NULL;
  pipeline_s *p = NULL;

  G_LOCK (pipeline_table_lock);
  p = (pipeline_s *) g_hash_table_lookup (pipeline_table, GINT_TO_POINTER (id));

  if (!p) {
    ml_loge ("The callback get_state is called, but there is no pipeline matched with ID.");
    result = -EINVAL;
    machinelearning_service_pipeline_complete_get_state (obj, invoc, result, (gint) state);
    G_UNLOCK (pipeline_table_lock);
    return TRUE;
  }

  g_mutex_lock (&p->lock);
  G_UNLOCK (pipeline_table_lock);
  sc_ret = gst_element_get_state (p->element, &state, NULL, GST_MSECOND);
  g_mutex_unlock (&p->lock);

  if (sc_ret == GST_STATE_CHANGE_FAILURE) {
    ml_loge ("Failed to get the state of the pipeline whose service name is %s.",
        p->service_name);
    result = -ESTRPIPE;
  }

  machinelearning_service_pipeline_complete_get_state (obj, invoc, result, (gint) state);

  return TRUE;
}

static struct gdbus_signal_info handler_infos[] = {
  {
      .signal_name = DBUS_PIPELINE_I_SET_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_set_pipeline),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_PIPELINE_I_GET_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_get_pipeline),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_PIPELINE_I_DELETE_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_delete_pipeline),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_PIPELINE_I_LAUNCH_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_launch_pipeline),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_PIPELINE_I_START_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_start_pipeline),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_PIPELINE_I_STOP_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_stop_pipeline),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_PIPELINE_I_DESTROY_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_destroy_pipeline),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_PIPELINE_I_GET_STATE_HANDLER,
      .cb = G_CALLBACK (dbus_cb_core_get_state),
      .cb_data = NULL,
      .handler_id = 0,
  },
};

/**
 * @brief Probe the D-BUS and connect this module.
 */
static int
probe_pipeline_module (void *data)
{
  int ret = 0;

  g_gdbus_instance = gdbus_get_pipeline_instance ();
  if (g_gdbus_instance == NULL) {
    ml_loge ("cannot get a dbus instance for the %s interface\n", DBUS_PIPELINE_INTERFACE);
    return -ENOSYS;
  }

  ret = gdbus_connect_signal (g_gdbus_instance, ARRAY_SIZE (handler_infos), handler_infos);
  if (ret < 0) {
    ml_loge ("cannot register callbacks as the dbus method invocation handlers\n ret: %d", ret);
    ret = -ENOSYS;
    goto out;
  }

  ret = gdbus_export_interface (g_gdbus_instance, DBUS_PIPELINE_PATH);
  if (ret < 0) {
    ml_loge ("cannot export the dbus interface '%s' at the object path '%s'\n",
        DBUS_PIPELINE_INTERFACE, DBUS_PIPELINE_PATH);
    ret = -ENOSYS;
    goto out_disconnect;
  }

  return 0;

out_disconnect:
  gdbus_disconnect_signal (g_gdbus_instance, ARRAY_SIZE (handler_infos), handler_infos);
out:
  gdbus_put_pipeline_instance (&g_gdbus_instance);

  return ret;
}

/**
 * @brief Initialize this module.
 */
static void
init_pipeline_module (void *data)
{
  gdbus_initialize ();

  G_LOCK (pipeline_table_lock);
  g_assert (NULL == pipeline_table); /** Internal error */
  pipeline_table = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, _pipeline_free);
  G_UNLOCK (pipeline_table_lock);
}

/**
 * @brief Finalize this module.
 */
static void
exit_pipeline_module (void *data)
{
  G_LOCK (pipeline_table_lock);
  g_assert (pipeline_table); /** Internal error */
  g_hash_table_destroy (pipeline_table);
  pipeline_table = NULL;
  G_UNLOCK (pipeline_table_lock);

  gdbus_disconnect_signal (g_gdbus_instance, ARRAY_SIZE (handler_infos), handler_infos);
  gdbus_put_pipeline_instance (&g_gdbus_instance);
}

static const struct module_ops pipeline_ops = {
  .name = "pipeline",
  .probe = probe_pipeline_module,
  .init = init_pipeline_module,
  .exit = exit_pipeline_module,
};

MODULE_OPS_REGISTER (&pipeline_ops)
