/* SPDX-License-Identifier: Apache-2.0 */
/**
 * NNStreamer API / Machine Learning Agent Daemon
 * Copyright (C) 2022 Samsung Electronics Co., Ltd. All Rights Reserved.
 */

/**
 * @file    log.h
 * @date    25 June 2022
 * @brief   Internal log header of Machine Learning agent daemon
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Sangjung Woo <sangjung.woo@samsung.com>
 * @bug     No known bugs except for NYI items
 *
 * @details
 *    This provides the log macro for Machine Learning agent daemon.
 */
#ifndef __LOG_H__
#define __LOG_H__

#define AGENT_LOG_TAG "ml-agent"

#if defined(__TIZEN__)
#include <dlog.h>

#define LOG_V(prio, tag, fmt, arg...) \
  ({ do { \
    dlog_print(prio, tag, "%s: %s(%d) > " fmt, __MODULE__, __func__, __LINE__, ##arg);\
  } while (0); })

#define ml_logd(fmt, arg...) LOG_V(DLOG_DEBUG, AGENT_LOG_TAG, fmt, ##arg)
#define ml_logi(fmt, arg...) LOG_V(DLOG_INFO, AGENT_LOG_TAG, fmt, ##arg)
#define ml_logw(fmt, arg...) LOG_V(DLOG_WARN, AGENT_LOG_TAG, fmt, ##arg)
#define ml_loge(fmt, arg...) LOG_V(DLOG_ERROR, AGENT_LOG_TAG, fmt, ##arg)
#define ml_logf(fmt, arg...) LOG_V(DLOG_FATAL, AGENT_LOG_TAG, fmt, ##arg)
#elif defined(__ANDROID__)
#include <android/log.h>

#define ml_logd(...) \
    __android_log_print (ANDROID_LOG_DEBUG, AGENT_LOG_TAG, __VA_ARGS__)

#define ml_logi(...) \
    __android_log_print (ANDROID_LOG_INFO, AGENT_LOG_TAG, __VA_ARGS__)

#define ml_logw(...) \
    __android_log_print (ANDROID_LOG_WARN, AGENT_LOG_TAG, __VA_ARGS__)

#define ml_loge(...) \
    __android_log_print (ANDROID_LOG_ERROR, AGENT_LOG_TAG, __VA_ARGS__)

#define ml_logf(...) \
    __android_log_print (ANDROID_LOG_FATAL, AGENT_LOG_TAG, __VA_ARGS__)
#else
#include <glib.h>

#define ml_logd g_debug
#define ml_logi g_info
#define ml_logw g_warning
#define ml_loge g_critical
#define ml_logf g_error
#endif

#endif /* __LOG_H__ */
