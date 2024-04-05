/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file    mlops-agent-interface.h
 * @date    5 April 2023
 * @brief   A set of exported ml-agent interfaces for managing pipelines, models, and other service.
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Wook Song <wook16.song@samsung.com>
 * @bug     No known bugs except for NYI items
 */

#ifndef __MLOPS_AGENT_INTERFACE_H__
#define __MLOPS_AGENT_INTERFACE_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

/**
 * @brief An interface exported for setting the description of a pipeline.
 * @param[in] name A name indicating the pipeline whose description would be set.
 * @param[in] pipeline_desc A stringified description of the pipeline.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_set_description (const char *name, const char *pipeline_desc);

/**
 * @brief An interface exported for getting the pipeline's description corresponding to the given @a name.
 * @remarks If the function succeeds, @a pipeline_desc should be released using free().
 * @param[in] name A given name of the pipeline to get the description.
 * @param[out] pipeline_desc A stringified description of the pipeline.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_get_description (const char *name, char **pipeline_desc);

/**
 * @brief An interface exported for deletion of the pipeline's description corresponding to the given @a name.
 * @param[in] name A given name of the pipeline to remove the description.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_delete (const char *name);

/**
 * @brief An interface exported for launching the pipeline's description corresponding to the given @a name.
 * @param[in] name A given name of the pipeline to launch.
 * @param[out] id A pointer of integer identifier for the launched pipeline.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_launch (const char *name, int64_t *id);

/**
 * @brief An interface exported for changing the pipeline's state of the given @a id to start.
 * @param[in] id An identifier of the launched pipeline whose state would be changed to start.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_start (const int64_t id);

/**
 * @brief An interface exported for changing the pipeline's state of the given @a id to stop.
 * @param[in] id An identifier of the launched pipeline whose state would be changed to stop.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_stop (const int64_t id);

/**
 * @brief An interface exported for destroying a launched pipeline corresponding to the given @a id.
 * @param[in] id An identifier of the launched pipeline that would be destroyed.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_destroy (const int64_t id);

/**
 * @brief An interface exported for getting the pipeline's state of the given @a id.
 * @param[in] id An identifier of the launched pipeline that would be destroyed.
 * @param[out] state A pointer for the pipeline's state.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_pipeline_get_state (const int64_t id, int *state);

/**
 * @brief An interface exported for registering a model.
 * @param[in] name A name indicating the model that would be registered.
 * @param[in] path A path that specifies the location of the model file.
 * @param[in] activate An initial activation state.
 * @param[in] description A stringified description of the given model.
 * @param[in] app_info Application-specific information from Tizen's RPK.
 * @param[out] version A pointer for the version of the given model registered.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_model_register (const char *name, const char *path, const int activate,
    const char *description, const char *app_info, uint32_t *version);

/**
 * @brief An interface exported for updating the description of the model with @a name and @a version.
 * @param[in] name A name indicating the model whose description would be updated.
 * @param[in] version A version for identifying the model whose description would be updated.
 * @param[in] description A new description to update the existing one.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_model_update_description (
    const char *name, const uint32_t version, const char *description);

/**
 * @brief An interface exported for activating the model with @a name and @a version.
 * @param[in] name A name indicating a registered model.
 * @param[in] version A version of the given model, @a name.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_model_activate (const char *name, const uint32_t version);

/**
 * @brief An interface exported for getting the information of the model with @a name and @a version.
 * @remarks If the function succeeds, @a model_info should be released using free().
 * @param[in] name A name indicating the model whose description would be get.
 * @param[in] version A version of the given model.
 * @param[out] model_info A pointer for the information of the given model of @a name and @a version.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_model_get (const char *name, const uint32_t version, char **model_info);

/**
 * @brief An interface exported for getting the information of the activated model with @a name.
 * @remarks If the function succeeds, @a model_info should be released using free().
 * @param[in] name A name indicating the model whose description would be get.
 * @param[out] model_info A pointer for the information of an activated model of the given @a name.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_model_get_activated (const char *name, char **model_info);

/**
 * @brief An interface exported for getting the information of all the models corresponding to the given @a name.
 * @remarks If the function succeeds, @a model_info should be released using free().
 * @param[in] name A name indicating the models whose description would be get.
 * @param[out] model_info A pointer for the information of all the models corresponding to the given @a name.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_model_get_all (const char *name, char **model_info);

/**
 * @brief An interface exported for removing the model of @a name and @a version.
 * @details If version is 0, this function removes all registered model of @a name.
 * @details If @a force is true, this will delete the model even if it is activated.
 * @param[in] name A name indicating the model that would be removed.
 * @param[in] version A version for identifying a specific model.
 * @param[in] force A force to forcibly delete a specific model.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_model_delete (const char *name, const uint32_t version, const int force);

/**
 * @brief An interface exported for adding the resource.
 * @param[in] name A name indicating the resource.
 * @param[in] path A path that specifies the location of the resource.
 * @param[in] description A stringified description of the resource.
 * @param[in] app_info Application-specific information from Tizen's RPK.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_resource_add (const char *name, const char *path,
    const char *description, const char *app_info);

/**
 * @brief An interface exported for removing the resource with @a name.
 * @param[in] name A name indicating the resource.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_resource_delete (const char *name);

/**
 * @brief An interface exported for getting the description of the resource with @a name.
 * @remarks If the function succeeds, @a res_info should be released using free().
 * @param[in] name A name indicating the resource.
 * @param[out] res_info A pointer for the information of the resource.
 * @return 0 on success, a negative error value if failed.
 */
int ml_agent_resource_get (const char *name, char **res_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLOPS_AGENT_INTERFACE_H__ */
