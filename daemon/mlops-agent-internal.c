/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file    mlops-agent-internal.c
 * @date    20 January 2025
 * @brief   Internal function for ml-agent interface.
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Jaeyun Jung <jy1210.jung@samsung.com>
 * @bug     No known bugs except for NYI items
 */

#include "log.h"
#include "mlops-agent-internal.h"
#include "mlops-agent-node.h"
#include "service-db-util.h"

/**
 * @brief Internal function to initialize mlops-agent interface.
 */
int
ml_agent_initialize (const char *db_path)
{
  int ret;

  if (!STR_IS_VALID (db_path) ||
      !g_file_test (db_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
    ml_loge ("Failed to initialize mlops-agent, database path (%s) is invalid.", db_path);
    return -EINVAL;
  }

  ret = svcdb_initialize (db_path);
  if (ret < 0)
    goto error;

  ret = mlops_node_initialize ();

error:
  if (ret < 0)
    ml_agent_finalize ();
  return ret;
}

/**
 * @brief Internal function to finalize mlops-agent interface.
 */
void
ml_agent_finalize (void)
{
  mlops_node_finalize ();
  svcdb_finalize ();
}
