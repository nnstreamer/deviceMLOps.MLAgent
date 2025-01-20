/* SPDX-License-Identifier: Apache-2.0 */
/**
 * NNStreamer API / Machine Learning Agent Daemon
 * Copyright (C) 2025 Samsung Electronics Co., Ltd. All Rights Reserved.
 */

/**
 * @file      mlops-agent-node.c
 * @date      20 january 2025
 * @brief     Implementation of mlops node.
 * @see       https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author    Jaeyun Jung <jy1210.jung@samsung.com>
 * @bug       No known bugs except for NYI items
 * @details   This implements the node information to run a pipeline.
 */

#include "log.h"
#include "mlops-agent-node.h"
#include "service-db-util.h"

static GHashTable *g_mlops_node_table = NULL;
G_LOCK_DEFINE_STATIC (mlops_node_table);

/**
 * @brief Structure for mlops node.
 */
typedef struct
{
  mlops_node_type_e type;
  gint64 id;
  GMutex lock;
  GstElement *element;
  gchar *service_name;
  gchar *description;
} mlops_node_s;

/**
 * @brief Internal function to get node info.
 */
static mlops_node_s *
_mlops_node_get (const int64_t id)
{
  mlops_node_s *node = NULL;

  G_LOCK (mlops_node_table);
  node = (mlops_node_s *) g_hash_table_lookup (g_mlops_node_table,
      GINT_TO_POINTER (id));
  G_UNLOCK (mlops_node_table);

  if (!node) {
    ml_loge ("There is no pipeline matched with ID %" G_GINT64_FORMAT, id);
  }

  return node;
}

/**
 * @brief Internal function to change pipeline state.
 */
static int
_mlops_node_set_pipeline_state (mlops_node_s * node, GstState state)
{
  GstStateChangeReturn ret;

  g_return_val_if_fail (node != NULL, -EINVAL);

  g_mutex_lock (&node->lock);
  ret = gst_element_set_state (node->element, state);
  g_mutex_unlock (&node->lock);

  if (ret == GST_STATE_CHANGE_FAILURE) {
    ml_loge ("Failed to set the state of the pipeline to %s with ID %"
        G_GINT64_FORMAT, gst_element_state_get_name (state), node->id);
    return -ESTRPIPE;
  }

  return 0;
}

/**
 * @brief Internal function to release mlops node.
 */
static void
_mlops_node_free (gpointer data)
{
  mlops_node_s *node = (mlops_node_s *) data;

  if (!node) {
    ml_logw ("The data pointer is null, internal error?");
    return;
  }

  _mlops_node_set_pipeline_state (node, GST_STATE_NULL);

  g_mutex_lock (&node->lock);

  node->type = MLOPS_NODE_TYPE_NONE;
  node->id = 0;
  if (node->element) {
    gst_object_unref (node->element);
    node->element = NULL;
  }
  g_free (node->service_name);
  node->service_name = NULL;
  g_free (node->description);
  node->description = NULL;

  g_mutex_unlock (&node->lock);

  g_mutex_clear (&node->lock);
  g_free (node);
}

/**
 * @brief Initialize mlops node info.
 */
void
mlops_node_initialize (void)
{
  G_LOCK (mlops_node_table);
  if (!g_mlops_node_table) {
    g_mlops_node_table = g_hash_table_new_full (g_direct_hash, g_direct_equal,
        NULL, _mlops_node_free);
  }
  g_assert (g_mlops_node_table != NULL);
  G_UNLOCK (mlops_node_table);
}

/**
 * @brief Finalize mlops node info.
 */
void
mlops_node_finalize (void)
{
  G_LOCK (mlops_node_table);
  g_assert (g_mlops_node_table != NULL);
  g_hash_table_destroy (g_mlops_node_table);
  g_mlops_node_table = NULL;
  G_UNLOCK (mlops_node_table);
}

/**
 * @brief Check service name and launch the pipeline.
 */
int
mlops_node_create (const gchar * name, const mlops_node_type_e type,
    int64_t * id)
{
  mlops_node_s *node = NULL;
  gint result = -EIO;
  gchar *desc = NULL;
  GstElement *pipeline = NULL;
  GError *err = NULL;
  GstStateChangeReturn ret;

  switch (type) {
    case MLOPS_NODE_TYPE_PIPELINE:
    {
      result = svcdb_pipeline_get (name, &desc);
      if (result != 0) {
        ml_loge ("Failed to launch pipeline of '%s'.", name);
        goto error;
      }
      break;
    }
    default:
      return -EINVAL;
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

  /* Set pipeline as paused state. */
  ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    ml_loge
        ("Failed to set the state of the pipeline to PAUSED. For the detail, please check the GStreamer log message.");

    gst_object_unref (pipeline);
    result = -ESTRPIPE;
    goto error;
  }

  /* Add node info into hash table. */
  node = g_new0 (mlops_node_s, 1);
  node->type = type;
  node->id = g_get_monotonic_time ();
  node->element = pipeline;
  node->service_name = g_strdup (name);
  node->description = g_strdup (desc);
  g_mutex_init (&node->lock);

  G_LOCK (mlops_node_table);
  g_hash_table_insert (g_mlops_node_table, GINT_TO_POINTER (node->id), node);
  G_UNLOCK (mlops_node_table);

error:
  if (result == 0) {
    *id = node->id;
  } else {
    if (node)
      _mlops_node_free (node);
  }

  g_free (desc);
  return result;
}

/**
 * @brief Start the pipeline with given id.
 */
int
mlops_node_start (const int64_t id)
{
  mlops_node_s *node = NULL;

  node = _mlops_node_get (id);
  return _mlops_node_set_pipeline_state (node, GST_STATE_PLAYING);
}

/**
 * @brief Stop the pipeline with given id.
 */
int
mlops_node_stop (const int64_t id)
{
  mlops_node_s *node = NULL;

  node = _mlops_node_get (id);
  return _mlops_node_set_pipeline_state (node, GST_STATE_PAUSED);
}

/**
 * @brief Destroy the pipeline with given id.
 */
int
mlops_node_destroy (const int64_t id)
{
  mlops_node_s *node = NULL;

  node = _mlops_node_get (id);
  if (node) {
    G_LOCK (mlops_node_table);
    g_hash_table_remove (g_mlops_node_table, GINT_TO_POINTER (id));
    G_UNLOCK (mlops_node_table);
  }

  return node ? 0 : -EINVAL;
}

/**
 * @brief Get the state of pipeline with given id.
 */
int
mlops_node_get_state (const int64_t id, GstState * state)
{
  mlops_node_s *node = NULL;
  GstStateChangeReturn ret;

  node = _mlops_node_get (id);
  g_return_val_if_fail (node != NULL, -EINVAL);

  g_mutex_lock (&node->lock);
  ret = gst_element_get_state (node->element, state, NULL, GST_MSECOND);
  g_mutex_unlock (&node->lock);

  if (ret == GST_STATE_CHANGE_FAILURE) {
    ml_loge ("Failed to get the state of the pipeline with ID %"
        G_GINT64_FORMAT, id);
    return -ESTRPIPE;
  }

  return 0;
}
