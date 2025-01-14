/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file    mlops-agent-internal.h
 * @date    14 January 2025
 * @brief   Internal header for ml-agent interface.
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Jaeyun Jung <jy1210.jung@samsung.com>
 * @bug     No known bugs except for NYI items
 */

#ifndef __MLOPS_AGENT_INTERNAL_H__
#define __MLOPS_AGENT_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#define STR_IS_VALID(s) ((s) && (s)[0] != '\0')

/**
 * @brief Internal enumeration for service type.
 */
typedef enum
{
  ML_AGENT_SERVICE_PIPELINE = 0,
  ML_AGENT_SERVICE_MODEL,
  ML_AGENT_SERVICE_RESOURCE,
  ML_AGENT_SERVICE_END
} ml_agent_service_type_e;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLOPS_AGENT_INTERNAL_H__ */
