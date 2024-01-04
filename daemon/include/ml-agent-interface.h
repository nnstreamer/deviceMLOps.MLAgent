/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file    ml-agent-interface.h
 * @date    5 April 2023
 * @brief   A set of exported ml-agent interfaces for managing pipelines, models, and other service.
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Wook Song <wook16.song@samsung.com>
 * @bug     No known bugs except for NYI items
 */

#ifndef __ML_AGENT_INTERFACE_H__
#define __ML_AGENT_INTERFACE_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <glib.h>

/**
 * @brief An interface exported for setting the description of a pipeline.
 * @param[in] name A name indicating the pipeline whose description would be set.
 * @param[in] pipeline_desc A stringified description of the pipeline.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_set_description (const gchar *name, const gchar *pipeline_desc, GError **err);

/**
 * @brief An interface exported for getting the pipeline's description corresponding to the given @a name.
 * @remarks If the function succeeds, @a pipeline_desc should be released using g_free().
 * @param[in] name A given name of the pipeline to get the description.
 * @param[out] pipeline_desc A stringified description of the pipeline.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_get_description (const gchar *name, gchar **pipeline_desc, GError **err);

/**
 * @brief An interface exported for deletion of the pipeline's description corresponding to the given @a name.
 * @param[in] name A given name of the pipeline to remove the description.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_delete (const gchar *name, GError **err);

/**
 * @brief An interface exported for launching the pipeline's description corresponding to the given @a name.
 * @param[in] name A given name of the pipeline to launch.
 * @param[out] id A pointer of integer identifier for the launched pipeline.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_launch (const gchar *name, gint64 *id, GError **err);

/**
 * @brief An interface exported for changing the pipeline's state of the given @a id to start.
 * @param[in] id An identifier of the launched pipeline whose state would be changed to start.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_start (const gint64 id, GError **err);

/**
 * @brief An interface exported for changing the pipeline's state of the given @a id to stop.
 * @param[in] id An identifier of the launched pipeline whose state would be changed to stop.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_stop (const gint64 id, GError **err);

/**
 * @brief An interface exported for destroying a launched pipeline corresponding to the given @a id.
 * @param[in] id An identifier of the launched pipeline that would be destroyed.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_destroy (const gint64 id, GError **err);

/**
 * @brief An interface exported for getting the pipeline's state of the given @a id.
 * @param[in] id An identifier of the launched pipeline that would be destroyed.
 * @param[out] state A pointer for the pipeline's state.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_pipeline_get_state (const gint64 id, gint *state, GError **err);

/**
 * @brief An interface exported for registering a model.
 * @param[in] name A name indicating the model that would be registered.
 * @param[in] path A path that specifies the location of the model file.
 * @param[in] activate An initial activation state.
 * @param[in] description A stringified description of the given model.
 * @param[in] app_info Application-specific information from Tizen's RPK.
 * @param[out] version A pointer for the version of the given model registered.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_model_register(const gchar *name, const gchar *path, const gboolean activate, const gchar *description, const gchar *app_info, guint *version, GError **err);

/**
 * @brief An interface exported for updating the description of the model with @a name and @a version.
 * @param[in] name A name indicating the model whose description would be updated.
 * @param[in] version A version for identifying the model whose description would be updated.
 * @param[in] description A new description to update the existing one.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_model_update_description (const gchar *name, const guint version, const gchar *description, GError **err);

/**
 * @brief An interface exported for activating the model with @a name and @a version.
 * @param[in] name A name indicating a registered model.
 * @param[in] version A version of the given model, @a name.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_model_activate(const gchar *name, const guint version, GError **err);

/**
 * @brief An interface exported for getting the information of the model with @a name and @a version.
 * @remarks If the function succeeds, @a model_info should be released using g_free().
 * @param[in] name A name indicating the model whose description would be get.
 * @param[in] version A version of the given model.
 * @param[out] model_info A pointer for the information of the given model of @a name and @a version.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_model_get(const gchar *name, const guint version, gchar **model_info, GError **err);

/**
 * @brief An interface exported for getting the information of the activated model with @a name.
 * @remarks If the function succeeds, @a model_info should be released using g_free().
 * @param[in] name A name indicating the model whose description would be get.
 * @param[out] model_info A pointer for the information of an activated model of the given @a name.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_model_get_activated(const gchar *name, gchar **model_info, GError **err);

/**
 * @brief An interface exported for getting the information of all the models corresponding to the given @a name.
 * @remarks If the function succeeds, @a model_info should be released using g_free().
 * @param[in] name A name indicating the models whose description would be get.
 * @param[out] model_info A pointer for the information of all the models corresponding to the given @a name.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_model_get_all(const gchar *name, gchar **model_info, GError **err);

/**
 * @brief An interface exported for removing the model of @a name and @a version.
 * @details If version is 0, this function removes all registered model of @a name.
 * @param[in] name A name indicating the model that would be removed.
 * @param[in] version A version for identifying a specific model.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_model_delete(const gchar *name, const guint version, GError **err);

/**
 * @brief An interface exported for adding the resource.
 * @param[in] name A name indicating the resource.
 * @param[in] path A path that specifies the location of the resource.
 * @param[in] description A stringified description of the resource.
 * @param[in] app_info Application-specific information from Tizen's RPK.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_resource_add (const gchar *name, const gchar *path, const gchar *description, const gchar *app_info, GError **err);

/**
 * @brief An interface exported for removing the resource with @a name.
 * @param[in] name A name indicating the resource.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_resource_delete (const gchar *name, GError **err);

/**
 * @brief An interface exported for getting the description of the resource with @a name.
 * @remarks If the function succeeds, @a res_info should be released using g_free().
 * @param[in] name A name indicating the resource.
 * @param[out] res_info A pointer for the information of the resource.
 * @param[out] err A pointer for error, or NULL.
 * @return 0 on success, a negative error value if failed.
 */
gint ml_agent_resource_get (const gchar *name, gchar **res_info, GError **err);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __ML_AGENT_INTERFACE_H__ */
