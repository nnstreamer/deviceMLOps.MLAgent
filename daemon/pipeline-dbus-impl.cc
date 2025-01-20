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
#include "mlops-agent-node.h"
#include "modules.h"
#include "pipeline-dbus.h"
#include "service-db-util.h"

static MachinelearningServicePipeline *g_gdbus_instance = NULL;

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

  result = mlops_node_create (service_name, MLOPS_NODE_TYPE_PIPELINE, &id);
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

  result = mlops_node_start (id);
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

  result = mlops_node_stop (id);
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

  result = mlops_node_destroy (id);
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
  GstState state = GST_STATE_NULL;

  result = mlops_node_get_state (id, &state);
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
}

/**
 * @brief Finalize this module.
 */
static void
exit_pipeline_module (void *data)
{
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
