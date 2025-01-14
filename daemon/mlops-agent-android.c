/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file    mlops-agent-android.c
 * @date    5 April 2023
 * @brief   A set of exported ml-agent interfaces for managing pipelines, models, and other service.
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Wook Song <wook16.song@samsung.com>
 * @bug     No known bugs except for NYI items
 */

#include <errno.h>
#include <glib.h>
#include <stdint.h>

#include "log.h"
#include "mlops-agent-interface.h"
#include "mlops-agent-internal.h"

/**
 * @brief An interface exported for setting the description of a pipeline.
 */
int
ml_agent_pipeline_set_description (const char *name, const char *pipeline_desc)
{
  if (!STR_IS_VALID (name) || !STR_IS_VALID (pipeline_desc)) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for getting the pipeline's description corresponding to the given @a name.
 */
int
ml_agent_pipeline_get_description (const char *name, char **pipeline_desc)
{
  if (!STR_IS_VALID (name) || !pipeline_desc) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for deletion of the pipeline's description corresponding to the given @a name.
 */
int
ml_agent_pipeline_delete (const char *name)
{
  if (!STR_IS_VALID (name)) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for launching the pipeline's description corresponding to the given @a name.
 */
int
ml_agent_pipeline_launch (const char *name, int64_t * id)
{
  if (!STR_IS_VALID (name) || !id) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for changing the pipeline's state of the given @a id to start.
 */
int
ml_agent_pipeline_start (const int64_t id)
{
  return 0;
}

/**
 * @brief An interface exported for changing the pipeline's state of the given @a id to stop.
 */
int
ml_agent_pipeline_stop (const int64_t id)
{
  return 0;
}

/**
 * @brief An interface exported for destroying a launched pipeline corresponding to the given @a id.
 */
int
ml_agent_pipeline_destroy (const int64_t id)
{
  return 0;
}

/**
 * @brief An interface exported for getting the pipeline's state of the given @a id.
 */
int
ml_agent_pipeline_get_state (const int64_t id, int *state)
{
  return 0;
}

/**
 * @brief An interface exported for registering a model.
 */
int
ml_agent_model_register (const char *name, const char *path,
    const int activate, const char *description, const char *app_info,
    uint32_t * version)
{
  if (!STR_IS_VALID (name) || !STR_IS_VALID (path) || !version) {
    g_return_val_if_reached (-EINVAL);
  }
  return 0;
}

/**
 * @brief An interface exported for updating the description of the model with @a name and @a version.
 */
int
ml_agent_model_update_description (const char *name,
    const uint32_t version, const char *description)
{
  if (!STR_IS_VALID (name) || !STR_IS_VALID (description) || version == 0U) {
    g_return_val_if_reached (-EINVAL);
  }
  return 0;
}

/**
 * @brief An interface exported for activating the model with @a name and @a version.
 */
int
ml_agent_model_activate (const char *name, const uint32_t version)
{
  if (!STR_IS_VALID (name) || version == 0U) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for getting the information of the model with @a name and @a version.
 */
int
ml_agent_model_get (const char *name, const uint32_t version, char **model_info)
{
  if (!STR_IS_VALID (name) || !model_info || version == 0U) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for getting the information of the activated model with @a name.
 */
int
ml_agent_model_get_activated (const char *name, char **model_info)
{
  if (!STR_IS_VALID (name) || !model_info) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for getting the information of all the models corresponding to the given @a name.
 */
int
ml_agent_model_get_all (const char *name, char **model_info)
{
  if (!STR_IS_VALID (name) || !model_info) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for removing the model of @a name and @a version.
 * @details If @a force is true, this will delete the model even if it is activated.
 */
int
ml_agent_model_delete (const char *name, const uint32_t version,
    const int force)
{
  if (!STR_IS_VALID (name)) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for adding the resource.
 */
int
ml_agent_resource_add (const char *name, const char *path,
    const char *description, const char *app_info)
{
  if (!STR_IS_VALID (name) || !STR_IS_VALID (path)) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for removing the resource with @a name.
 */
int
ml_agent_resource_delete (const char *name)
{
  if (!STR_IS_VALID (name)) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}

/**
 * @brief An interface exported for getting the description of the resource with @a name.
 */
int
ml_agent_resource_get (const char *name, char **res_info)
{
  if (!STR_IS_VALID (name) || !res_info) {
    g_return_val_if_reached (-EINVAL);
  }

  return 0;
}
