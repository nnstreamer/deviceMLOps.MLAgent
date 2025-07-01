/* SPDX-License-Identifier: Apache-2.0 */
/**
 * NNStreamer API / Machine Learning Agent Daemon
 * Copyright (C) 2025 Samsung Electronics Co., Ltd. All Rights Reserved.
 */

/**
 * @file      mlops-agent-node.h
 * @date      20 january 2025
 * @brief     Implementation of mlops node.
 * @see       https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author    Jaeyun Jung <jy1210.jung@samsung.com>
 * @bug       No known bugs except for NYI items
 * @details   This implements the node information to run a pipeline.
 */

#ifndef _MLOPS_AGENT_NODE_H_
#define _MLOPS_AGENT_NODE_H_

#include <glib.h>
#include <gst/gst.h>

G_BEGIN_DECLS

/**
 * @brief Enumeration for mlops node type.
 */
typedef enum
{
  MLOPS_NODE_TYPE_NONE = 0,
  MLOPS_NODE_TYPE_PIPELINE,
  MLOPS_NODE_TYPE_MAX
} mlops_node_type_e;

/**
 * @brief Initialize mlops node info.
 */
void mlops_node_initialize (void);

/**
 * @brief Finalize mlops node info.
 */
void mlops_node_finalize (void);

/**
 * @brief Check service name and launch the pipeline.
 */
int mlops_node_create (const gchar *name, const mlops_node_type_e type, int64_t *id);

/**
 * @brief Start the pipeline with given id.
 */
int mlops_node_start (const int64_t id);

/**
 * @brief Stop the pipeline with given id.
 */
int mlops_node_stop (const int64_t id);

/**
 * @brief Destroy the pipeline with given id.
 */
int mlops_node_destroy (const int64_t id);

/*
 * @brief Get the state of pipeline with given id.
 */
int mlops_node_get_state (const int64_t id, GstState *state);

G_END_DECLS
#endif /* _MLOPS_AGENT_NODE_H_ */
