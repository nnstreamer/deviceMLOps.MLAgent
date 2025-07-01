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
void
ml_agent_initialize (const char *db_path)
{
  g_assert (STR_IS_VALID (db_path));
  g_assert (g_file_test (db_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR));

  svcdb_initialize (db_path);
  mlops_node_initialize ();
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
