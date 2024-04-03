/* SPDX-License-Identifier: Apache-2.0 */
/**
 * Copyright (c) 2024 Samsung Electronics Co., Ltd. All Rights Reserved.
 *
 * @file    mlops-plugin-parser.cc
 * @date    21 Mar 2024
 * @brief   This file contains the implementation of a plugin parser for Tizen RPK packages.
 * The plugin parser is responsible for parsing JSON files in RPK and updating the database using daemon.
 * @see	    https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Yongjoo Ahn <yongjoo1.ahn@samsung.com>
 * @bug     No known bugs except for NYI items
 * @todo    Add more unit tests using mocks.
 * @todo    Support UNINSTALL and UPGRADE.
 * @todo    Support allowed resource other than global resource.
 */

#include <dlog.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <mlops-agent-interface.h>
#include <pkgmgr-info.h>

#define __TAG_ "ml-agent-plugin-parser"
#define LOG_V(prio, tag, fmt, arg...)                                                     \
  ({                                                                                      \
    do {                                                                                  \
      dlog_print (prio, tag, "%s: %s(%d) > " fmt, __MODULE__, __func__, __LINE__, ##arg); \
    } while (0);                                                                          \
  })

#define _D(fmt, arg...) LOG_V (DLOG_DEBUG, __TAG_, fmt, ##arg)
#define _I(fmt, arg...) LOG_V (DLOG_INFO, __TAG_, fmt, ##arg)
#define _W(fmt, arg...) LOG_V (DLOG_WARN, __TAG_, fmt, ##arg)
#define _E(fmt, arg...) LOG_V (DLOG_ERROR, __TAG_, fmt, ##arg)
#define _F(fmt, arg...) LOG_V (DLOG_FATAL, __TAG_, fmt, ##arg)
#define STR_IS_VALID(s) ((s) && (s)[0] != '\0')

/**
 * @brief Struct used to parse metadata tag from xml file.
 */
typedef struct _metadata_s {
  const char *key;
  const char *value;
} _metadata_s;

/**
 * @brief Make pkg-info json string.
 */
static gchar *
_make_pkg_info (const gchar *pkgid, const gchar *appid, const gchar *res_type,
    const gchar *res_version)
{
  g_autoptr (JsonBuilder) builder = json_builder_new ();
  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "is_rpk");
  json_builder_add_string_value (builder, "T");

  json_builder_set_member_name (builder, "pkg_id");
  json_builder_add_string_value (builder, pkgid);

  json_builder_set_member_name (builder, "app_id");
  json_builder_add_string_value (builder, appid ? appid : "");

  json_builder_set_member_name (builder, "res_type");
  json_builder_add_string_value (builder, res_type);

  json_builder_set_member_name (builder, "res_version");
  json_builder_add_string_value (builder, res_version);

  json_builder_end_object (builder);

  g_autoptr (JsonNode) root = json_builder_get_root (builder);
  g_autoptr (JsonGenerator) gen = json_generator_new ();
  json_generator_set_root (gen, root);
  json_generator_set_pretty (gen, TRUE);

  return json_generator_to_data (gen, NULL);
}

/**
 * @brief Internal enumeration for data types of json.
 */
typedef enum {
  MLSVC_JSON_MODEL = 0,
  MLSVC_JSON_PIPELINE = 1,
  MLSVC_JSON_RESOURCE = 2,
  MLSVC_JSON_MAX
} mlsvc_json_type_e;

/**
 * @brief Internal enumeration for package manager event types.
 */
typedef enum {
  MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_INSTALL = 0,
  MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_UNINSTALL = 1,
  MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_UPGRADE = 2,
  MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_MAX
} mlsvc_package_manager_event_type_e;

/**
 * @brief Internal function for uninstall models and pipelines.
 */
static void
_uninstall_rpk (const gchar *name, const gchar *_info, mlsvc_json_type_e json_type)
{
  g_autoptr (JsonParser) parser = json_parser_new ();
  g_autoptr (GError) err = NULL;
  JsonNode *root;
  JsonObject *object;
  JsonArray *array = NULL;
  guint _info_len = 1U;

  if (!json_parser_load_from_data (parser, _info, -1, &err)) {
    _E ("Failed to load '%s' info data (%s).", name, err ? err->message : "Unknown error");
    return;
  }

  root = json_parser_get_root (parser);
  if (!root) {
    _E ("Failed to load '%s' info, cannot get the top node from info.", name);
    return;
  }

  if (JSON_NODE_HOLDS_ARRAY (root)) {
    array = json_node_get_array (root);
    if (!array) {
      _E ("Failed to get root array from '%s' info", name);
      return;
    }

    _info_len = json_array_get_length (array);
  }

  /* Update ML service database. */
  for (guint i = 0; i < _info_len; ++i) {
    int ret = 0;

    if (array)
      object = json_array_get_object_element (array, i);
    else
      object = json_node_get_object (root);

    const gchar *app_info = json_object_get_string_member (object, "app_info");

    /* If app info is empty string, it is not installed from rpk. */
    if (!STR_IS_VALID (app_info))
      continue;

    g_autoptr (JsonParser) app_info_parser = json_parser_new ();

    if (!json_parser_load_from_data (app_info_parser, app_info, -1, &err)) {
      _E ("Failed to load '%s' app_info data.", name);
      return;
    }

    JsonNode *app_info_root = json_parser_get_root (app_info_parser);
    if (!app_info_root) {
      _E ("Failed to load '%s'app_info, cannot get the top node from app_info.", name);
      return;
    }

    JsonObject *app_info_object = json_node_get_object (app_info_root);
    const gchar *is_rpk = json_object_get_string_member (app_info_object, "is_rpk");

    if (!is_rpk) {
      _E ("Failed to get 'is_rpk' from '%s' app_info.", name);
      return;
    }
    if (g_ascii_strcasecmp (is_rpk, "F") == 0)
      continue;

    switch (json_type) {
      case MLSVC_JSON_MODEL:
        {
          const gchar *version = json_object_get_string_member (object, "version");

          ret = ml_agent_model_delete (
              name, (const uint32_t) g_ascii_strtoull (version, NULL, 10), TRUE);

          if (ret == 0) {
            _I ("The model is deleted. - name: %s, version %s", name, version);
          } else {
            _E ("Failed to delete model return %d. - name: %s, version: %s",
                ret, name, version);
          }
        }
        break;
      case MLSVC_JSON_RESOURCE:
        {
          /** @todo Support delete resource installed by rpk */
        }
        break;
      default:
        _E ("Unknown json type '%d', internal error?", json_type);
        return;
    }
  }

  _I ("All deleted. - name: %s", name);
  return;
}

/**
 * @brief Parse json and update ml-service database via invoking daemon.
 */
static gboolean
_parse_json (JsonNode *node, const gchar *app_info, mlsvc_json_type_e json_type,
    mlsvc_package_manager_event_type_e event)
{
  JsonArray *array = NULL;
  JsonObject *object = NULL;
  guint json_len = 1U;
  gint ret;

  if (JSON_NODE_HOLDS_ARRAY (node)) {
    array = json_node_get_array (node);
    if (!array) {
      _E ("Failed to get array from json");
      return FALSE;
    }

    json_len = json_array_get_length (array);
  }

  /* Update ML service database. */
  for (guint i = 0; i < json_len; ++i) {
    if (array)
      object = json_array_get_object_element (array, i);
    else
      object = json_node_get_object (node);

    switch (json_type) {
      case MLSVC_JSON_MODEL:
        {
          const gchar *name = json_object_get_string_member (object, "name");
          const gchar *model = json_object_get_string_member (object, "model");
          const gchar *desc = json_object_get_string_member (object, "description");
          const gchar *activate = json_object_get_string_member (object, "activate");

          if (!name || !model) {
            _E ("Failed to get name or model from MLSVC_JSON_MODEL.");
            return FALSE;
          }

          if (event == MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_INSTALL) {
            guint version;
            bool active = (activate && g_ascii_strcasecmp (activate, "true") == 0);

            ret = ml_agent_model_register (name, model, active,
                desc ? desc : "", app_info ? app_info : "", &version);

            if (ret == 0) {
              _I ("The model with name '%s' is registered as version '%u'.", name, version);
            } else {
              _E ("Failed to register the model with name '%s'.", name);
              return FALSE;
            }
          } else if (event == MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_UNINSTALL) {
            g_autofree gchar *model_info = NULL;

            ret = ml_agent_model_get_all (name, &model_info);

            if (ret == 0) {
              _uninstall_rpk (name, model_info, json_type);
            } else {
              _I ("The model with name '%s' is already deleted or not installed.", name);
            }
          } else {
            _E ("Unknown event type '%d', internal error?", event);
            return FALSE;
          }
        }
        break;
      case MLSVC_JSON_PIPELINE:
        {
          const gchar *name = json_object_get_string_member (object, "name");
          const gchar *pipe = json_object_get_string_member (object, "pipeline");

          if (!name || !pipe) {
            _E ("Failed to get name or pipeline from MLSVC_JSON_PIPELINE.");
            return FALSE;
          }

          if (event == MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_INSTALL) {
            ret = ml_agent_pipeline_set_description (name, pipe);

            if (ret == 0) {
              _I ("The pipeline description with name '%s' is registered.", name);
            } else {
              _E ("Failed to register pipeline with name '%s'.", name);
              return FALSE;
            }
          } else if (event == MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_UNINSTALL) {
            ret = ml_agent_pipeline_delete (name);

            if (ret == 0) {
              _I ("The pipeline description with name '%s' is deleted.", name);
            } else {
              _E ("Failed to delete pipeline with name '%s'.", name);
              return FALSE;
            }
          } else {
            _E ("Unknown event type '%d', internal error?", event);
            return FALSE;
          }
        }
        break;
      case MLSVC_JSON_RESOURCE:
        {
          const gchar *name = json_object_get_string_member (object, "name");
          const gchar *desc = json_object_get_string_member (object, "description");
          JsonNode *path_node = json_object_get_member (object, "path");
          JsonArray *path_array = NULL;
          guint path_len;

          if (!name) {
            _E ("Failed to get name from MLSVC_JSON_RESOURCE.");
            return FALSE;
          }

          path_len = 1U;
          if (JSON_NODE_HOLDS_ARRAY (path_node)) {
            path_array = json_node_get_array (path_node);
            path_len = (path_array) ? json_array_get_length (path_array) : 0U;
          }

          if (path_len == 0U) {
            _E ("Failed to get path from MLSVC_JSON_RESOURCE.");
            return FALSE;
          }

          if (event == MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_INSTALL) {
            for (guint pidx = 0; pidx < path_len; pidx++) {
              const gchar *path = NULL;

              if (path_array)
                path = json_array_get_string_element (path_array, pidx);
              else
                path = json_node_get_string (path_node);

              if (!path) {
                _E ("Failed to get path at '%d'th of '%s' from MLSVC_JSON_RESOURCE.",
                    pidx, name);
                return FALSE;
              }

              ret = ml_agent_resource_add (
                  name, path, desc ? desc : "", app_info ? app_info : "");

              if (ret == 0) {
                _I ("The resource at '%d'th of name '%s' is registered.", pidx, name);
              } else {
                _E ("Failed to register the resource with name '%s'.", name);
                return FALSE;
              }
            }
          } else if (event == MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_UNINSTALL) {
            ret = ml_agent_resource_delete (name);

            if (ret == 0) {
              _I ("The resource is deleted. - name: %s", name);
            } else {
              _I ("The model with name '%s' is already deleted or not installed", name);
            }
          } else {
            _E ("Unknown event type '%d', internal error?", event);
            return FALSE;
          }
        }
        break;
      default:
        _E ("Unknown data type '%d', internal error?", json_type);
        return FALSE;
    }
  }
  return TRUE;
}

/**
 * @brief Internal function to get json configuration file.
 */
static gboolean
_get_json_config (const gchar *json_path, const gchar *app_info,
    mlsvc_package_manager_event_type_e event)
{
  g_autofree gchar *json_string = NULL;
  g_autoptr (JsonParser) parser = NULL;
  g_autoptr (GError) err = NULL;
  g_autoptr (GList) members = NULL;
  GList *iter;
  JsonNode *root;
  JsonObject *object;

  if (!g_file_test (json_path, (GFileTest) (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))) {
    _E ("The parameter, json_path, is invalid. It should be a valid string.");
    return FALSE;
  }

  if (!g_file_get_contents (json_path, &json_string, NULL, NULL)) {
    _E ("Failed to read configuration file '%s'.", json_path);
    return FALSE;
  }

  parser = json_parser_new ();
  if (!parser) {
    _E ("Failed to parse configuration file, cannot allocate memory for JsonParser. Out of memory?");
    return FALSE;
  }

  if (!json_parser_load_from_data (parser, json_string, -1, &err)) {
    _E ("Failed to parse configuration file, cannot load json string (%s).",
        err ? err->message : "Unknown error");
    return FALSE;
  }

  root = json_parser_get_root (parser);
  if (!root) {
    _E ("Failed to parse configuration file, cannot get the top node from json string.");
    return FALSE;
  }

  object = json_node_get_object (root);
  members = json_object_get_members (object);

  for (iter = members; iter != NULL; iter = iter->next) {
    const gchar *name = (const gchar *) iter->data;
    JsonNode *node = json_object_get_member (object, name);
    mlsvc_json_type_e json_type;

    if (g_ascii_strcasecmp (name, "model") == 0 || g_ascii_strcasecmp (name, "models") == 0) {
      json_type = MLSVC_JSON_MODEL;
    } else if (g_ascii_strcasecmp (name, "pipeline") == 0
               || g_ascii_strcasecmp (name, "pipelines") == 0) {
      json_type = MLSVC_JSON_PIPELINE;
    } else if (g_ascii_strcasecmp (name, "resource") == 0
               || g_ascii_strcasecmp (name, "resources") == 0) {
      json_type = MLSVC_JSON_RESOURCE;
    } else {
      _E ("Failed to parse '%s' from configuration file, unsupported type.", name);
      return FALSE;
    }

    if (!_parse_json (node, app_info, json_type, event)) {
      _E ("Failed to parse '%s' from configuration file.", name);
      return FALSE;
    }
  }

  return TRUE;
}

/**
 * @brief Internal function to set pkg-mgr parser
 */
static int
_set_pkgmgr_plugin (const gchar *pkgid, const gchar *appid, GList *metadata,
    mlsvc_package_manager_event_type_e event)
{
  GList *list = NULL;
  _metadata_s *detail = NULL;
  pkgmgrinfo_pkginfo_h handle;
  int ret = 0;

  _I ("pkgid = %s, appid = %s\n", pkgid, appid);

  /* check metadata key/value */
  list = g_list_first (metadata);
  while (list) {
    detail = (_metadata_s *) list->data;
    _I ("key = %s, value = %s\n", detail->key, detail->value);
    list = g_list_next (list);
  }

  ret = pkgmgrinfo_pkginfo_get_pkginfo (pkgid, &handle);
  if (ret != PMINFO_R_OK) {
    _E ("Failed to get handle.");
    return -1;
  }

  /* Check whether the package is rpk */
  char *pkg_type = NULL;
  ret = pkgmgrinfo_pkginfo_get_type (handle, &pkg_type);
  if (ret != PMINFO_R_OK) {
    _E ("Failed to get package type.");
    pkgmgrinfo_pkginfo_destroy_pkginfo (handle);
    return -1;
  }
  _I ("pkg_type : %s", pkg_type);

  if (g_strcmp0 (pkg_type, "rpk") != 0) {
    _I ("pkg_type is not rpk. Skip parsing.");
    pkgmgrinfo_pkginfo_destroy_pkginfo (handle);
    return 0;
  }

  char *root_path = NULL;
  ret = pkgmgrinfo_pkginfo_get_root_path (handle, &root_path);
  if (ret != PMINFO_R_OK) {
    _E ("Failed to get root path.");
    pkgmgrinfo_pkginfo_destroy_pkginfo (handle);
    return -1;
  }
  _I ("root path: %s", root_path);

  char *res_type = NULL;
  ret = pkgmgrinfo_pkginfo_get_res_type (handle, &res_type);
  if (ret != PMINFO_R_OK) {
    _E ("Failed to get res type.");
    pkgmgrinfo_pkginfo_destroy_pkginfo (handle);
    return -1;
  }
  _I ("res_type = %s\n", res_type);

  char *res_version = NULL;
  ret = pkgmgrinfo_pkginfo_get_res_version (handle, &res_version);
  if (ret != PMINFO_R_OK) {
    _E ("Failed to get res version.");
    pkgmgrinfo_pkginfo_destroy_pkginfo (handle);
    return -1;
  }
  _I ("res_version = %s\n", res_version);

  g_autofree gchar *app_info = _make_pkg_info (pkgid, appid, res_type, res_version);
  _I ("app_info = %s\n", app_info);

  /* check rpk_config.json file */
  g_autofree gchar *json_file = g_build_filename (
      root_path, "res", "global", res_type, "rpk_config.json", NULL);

  if (!_get_json_config (json_file, app_info, event)) {
    _E ("Failed to parse the config file %s", json_file);
    pkgmgrinfo_pkginfo_destroy_pkginfo (handle);
    return -1;
  }

  _I ("PKGMGR_MDPARSER_PLUGIN finished");
  pkgmgrinfo_pkginfo_destroy_pkginfo (handle);

  return 0;
}

/**
 * @brief Handle INSTALL process. Tizen app-installer invoke this function.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_INSTALL (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_INSTALL called");
  return _set_pkgmgr_plugin (
      pkgid, appid, metadata, MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_INSTALL);
}

/**
 * @brief Handle UNINSTALL process. Tizen app-installer invoke this function.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_UNINSTALL (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_UNINSTALL called");
  return _set_pkgmgr_plugin (
      pkgid, appid, metadata, MLSVC_PKGMGR_MDPARSER_PLUGIN_EVENT_TYPE_UNINSTALL);
}

/**
 * @brief Handle UPGRADE process. Tizen app-installer invoke this function.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_UPGRADE (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_UPGRADE called");

  PKGMGR_MDPARSER_PLUGIN_UNINSTALL (pkgid, appid, metadata);
  return PKGMGR_MDPARSER_PLUGIN_INSTALL (pkgid, appid, metadata);
}

/**
 * @brief RECOVERINSTALL is invoked after the INSTALL process failed.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_RECOVERINSTALL (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_RECOVERINSTALL called");
  return PKGMGR_MDPARSER_PLUGIN_UNINSTALL (pkgid, appid, metadata);
}

/**
 * @brief RECOVERUPGRADE is invoked after the UPGRADE process failed.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_RECOVERUPGRADE (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_RECOVERUPGRADE called");
  return PKGMGR_MDPARSER_PLUGIN_UPGRADE (pkgid, appid, metadata);
}

/**
 * @brief RECOVERUNINSTALL is invoked after the UNINSTALL process failed.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_RECOVERUNINSTALL (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_RECOVERUNINSTALL called");
  return PKGMGR_MDPARSER_PLUGIN_INSTALL (pkgid, appid, metadata);
}

/**
 * @brief CLEAN is invoked after the installation process completed.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_CLEAN (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_CLEAN called");
  return 0;
}

/**
 * @brief UNDO is invoked after the installation process failed.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_UNDO (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_UNDO called");
  return 0;
}
