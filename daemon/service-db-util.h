/* SPDX-License-Identifier: Apache-2.0 */
/**
 * Copyright (c) 2022 Samsung Electronics Co., Ltd. All Rights Reserved.
 *
 * @file    service-db-util.h
 * @date    28 Mar 2022
 * @brief   NNStreamer/Service Database Interface
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Sangjung Woo <sangjung.woo@samsung.com>
 * @bug     No known bugs except for NYI items
 */

#ifndef __SERVICE_DB_UTIL_H__
#define __SERVICE_DB_UTIL_H__

#include <glib.h>

G_BEGIN_DECLS

void svcdb_initialize (const gchar *path);
void svcdb_finalize (void);
gint svcdb_pipeline_set (const gchar *name, const gchar *description);
gint svcdb_pipeline_get (const gchar *name, gchar **description);
gint svcdb_pipeline_delete (const gchar *name);
gint svcdb_model_add (const gchar *name, const gchar *path, const bool is_active, const gchar *description, const gchar *app_info, guint *version);
gint svcdb_model_update_description (const gchar *name, const guint version, const gchar *description);
gint svcdb_model_activate (const gchar *name, const guint version);
gint svcdb_model_get (const gchar *name, const guint version, gchar **model_info);
gint svcdb_model_get_activated (const gchar *name, gchar **model_info);
gint svcdb_model_get_all (const gchar *name, gchar **model_info);
gint svcdb_model_delete (const gchar *name, const guint version, const gboolean force);
gint svcdb_resource_add (const gchar *name, const gchar *path, const gchar *description, const gchar *app_info);
gint svcdb_resource_get (const gchar *name, gchar **res_info);
gint svcdb_resource_delete (const gchar *name);

G_END_DECLS
#endif /* __SERVICE_DB_UTIL_H__ */
