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
 * @brief Parse json and update ml-service database via invoking daemon.
 */
static void
_parse_json (const gchar *json_path, mlsvc_json_type_e json_type, const gchar *app_info)
{
  g_autofree gchar *json_file = NULL;
  gint ret;

  switch (json_type) {
    case MLSVC_JSON_MODEL:
      json_file = g_build_filename (json_path, "model_description.json", NULL);
      break;
    case MLSVC_JSON_PIPELINE:
      json_file = g_build_filename (json_path, "pipeline_description.json", NULL);
      break;
    case MLSVC_JSON_RESOURCE:
      json_file = g_build_filename (json_path, "resource_description.json", NULL);
      break;
    default:
      _E ("Unknown data type '%d', internal error?", json_type);
      return;
  }

  if (!g_file_test (json_file, (GFileTest) (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))) {
    _W ("Failed to find json file '%s'. RPK using ML Service API should provide this json file.",
        json_file);
    return;
  }

  g_autoptr (JsonParser) parser = json_parser_new ();
  g_autoptr (GError) err = NULL;

  json_parser_load_from_file (parser, json_file, &err);
  if (err) {
    _E ("Failed to parse json file '%s': %s", json_file, err->message);
    return;
  }

  JsonNode *root = json_parser_get_root (parser);
  JsonArray *array = NULL;
  JsonObject *object = NULL;
  guint json_len = 1U;

  if (JSON_NODE_HOLDS_ARRAY (root)) {
    array = json_node_get_array (root);
    if (!array) {
      _E ("Failed to get root array from json file '%s'", json_file);
      return;
    }

    json_len = json_array_get_length (array);
  }

  /* Update ML service database. */
  for (guint i = 0; i < json_len; ++i) {
    if (array)
      object = json_array_get_object_element (array, i);
    else
      object = json_node_get_object (root);

    switch (json_type) {
      case MLSVC_JSON_MODEL:
        {
          const gchar *name = json_object_get_string_member (object, "name");
          const gchar *model = json_object_get_string_member (object, "model");
          const gchar *desc = json_object_get_string_member (object, "description");
          const gchar *activate = json_object_get_string_member (object, "activate");
          const gchar *clear = json_object_get_string_member (object, "clear");

          if (!name || !model) {
            _E ("Failed to get name or model from json file '%s'.", json_file);
            continue;
          }

          guint version;
          bool active = (activate && g_ascii_strcasecmp (activate, "true") == 0);
          bool clear_old = (clear && g_ascii_strcasecmp (clear, "true") == 0);

          /* Remove old model from database. */
          if (clear_old) {
            /* Ignore error case. */
            ml_agent_model_delete (name, 0U);
          }

          ret = ml_agent_model_register (name, model, active, desc ? desc : "",
              app_info ? app_info : "", &version);

          if (ret == 0)
            _I ("The model with name '%s' is registered as version '%u'.", name, version);
          else
            _E ("Failed to register the model with name '%s'.", name);
        }
        break;
      case MLSVC_JSON_PIPELINE:
        {
          const gchar *name = json_object_get_string_member (object, "name");
          const gchar *desc = json_object_get_string_member (object, "description");

          if (!name || !desc) {
            _E ("Failed to get name or description from json file '%s'.", json_file);
            continue;
          }

          ret = ml_agent_pipeline_set_description (name, desc);

          if (ret == 0)
            _I ("The pipeline description with name '%s' is registered.", name);
          else
            _E ("Failed to register pipeline with name '%s'.", name);
        }
        break;
      case MLSVC_JSON_RESOURCE:
        {
          const gchar *name = json_object_get_string_member (object, "name");
          const gchar *path = json_object_get_string_member (object, "path");
          const gchar *desc = json_object_get_string_member (object, "description");
          const gchar *clear = json_object_get_string_member (object, "clear");

          if (!name || !path) {
            _E ("Failed to get name or path from json file '%s'.", json_file);
            continue;
          }

          bool clear_old = (clear && g_ascii_strcasecmp (clear, "true") == 0);

          /* Remove old resource from database. */
          if (clear_old) {
            /* Ignore error case. */
            ml_agent_resource_delete (name);
          }

          ret = ml_agent_resource_add (
              name, path, desc ? desc : "", app_info ? app_info : "");

          if (ret == 0)
            _I ("The resource with name '%s' is registered.", name);
          else
            _E ("Failed to register the resource with name '%s'.", name);
        }
        break;
      default:
        _E ("Unknown data type '%d', internal error?", json_type);
        break;
    }
  }
}

/**
 * @brief Handle INSTALL process. Tizen app-installer invoke this function.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_INSTALL (const char *pkgid, const char *appid, GList *metadata)
{
  GList *list = NULL;
  _metadata_s *detail = NULL;
  pkgmgrinfo_pkginfo_h handle;
  int ret = 0;

  _I ("PKGMGR_MDPARSER_PLUGIN_INSTALL called");
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
  _I ("res_type = %s\n", res_type);

  char *res_version = NULL;
  ret = pkgmgrinfo_pkginfo_get_res_version (handle, &res_version);
  _I ("res_version = %s\n", res_version);

  g_autofree gchar *app_info = _make_pkg_info (pkgid, appid, res_type, res_version);
  _I ("app_info = %s\n", app_info);

  /* check description.json file */
  g_autofree gchar *json_dir_path = g_build_filename (root_path, "res", "global", res_type, NULL);

  _parse_json (json_dir_path, MLSVC_JSON_MODEL, app_info);

  _I ("PKGMGR_MDPARSER_PLUGIN_INSTALL finished");
  pkgmgrinfo_pkginfo_destroy_pkginfo (handle);

  return 0;
}

/**
 * @brief Handle UNINSTALL process. Tizen app-installer invoke this function.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_UNINSTALL (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_UNINSTALL called");
  return 0;
}

/**
 * @brief Handle UPGRADE process. Tizen app-installer invoke this function.
 */
extern "C" int
PKGMGR_MDPARSER_PLUGIN_UPGRADE (const char *pkgid, const char *appid, GList *metadata)
{
  _I ("PKGMGR_MDPARSER_PLUGIN_UPGRADE called");
  PKGMGR_MDPARSER_PLUGIN_UNINSTALL (pkgid, appid, metadata);
  PKGMGR_MDPARSER_PLUGIN_INSTALL (pkgid, appid, metadata);

  return 0;
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
