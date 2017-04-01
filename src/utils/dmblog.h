/*
    Copyright (C) 2016-2017 Xiongfa Li, <damao1222@live.com>
    All rights reserved.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef DMBLOG_H
#define DMBLOG_H

#include "dmbdefines.h"

#define LOGDEBUG    0
#define LOGINFO     1
#define LOGRUNTIME  2
#define LOGWARNING  3
#define LOGERROR    4
#define LOGFATAL    5

/**
 * @brief 初始化日志系统
 * @return 成功返回TRUE，否则返回FALSE
 */
dmbBOOL dmbLogInit(const char *pcLogFileDir, dmbINT iLogLevel);

/**
 * @brief 回收log资源
 *
 */
void dmbLogPurge();

/**
 * @brief 配置log
 *
 * @param pcLogFilePath 日志文件目录
 * @param iLogLevel log级别
 */
dmbBOOL dmbLogConfig(const char *pcLogFileDir, dmbINT iLogLevel);

/**
 * @brief 输出日志
 * @param level  日志级别
 * @param format  日志内容
 * @param ... 参数
 */
void dmbLog(int level, const char *format, ...);

/**
 * @brief 输出当前系统信息
 *
 */
void dmbLogSystemInfo();

//DMB_LOGI   info级别日志
//DMB_LOGD   debug级别日志
//DMB_LOGW   warning级别日志
//DMB_LOGE   error级别日志
//DMB_FATAL  fatal日志，直接退出程序
//DMB_ASSERT_X assert封装














//internal macros

#ifdef DMB_DEBUG
    //runtime log
    #define DMB_LOGR(FORMAT, ARG...) dmbLog(LOGRUNTIME, FORMAT, ##ARG)
    //common log
    #define DMB_LOG(LEVEL, FORMAT, ARG...) dmbLog(LEVEL, "<%s:%d>(%s) \n"FORMAT, __FILE__, __LINE__, __FUNCTION__, ##ARG)
    //debug
    #define DMB_LOGD(FORMAT, ARG...) dmbLog(LOGDEBUG, FORMAT, ##ARG)
    //info
    #define DMB_LOGI(FORMAT, ARG...) dmbLog(LOGINFO, FORMAT, ##ARG)
    //warning
    #define DMB_LOGW(FORMAT, ARG...) dmbLog(LOGWARNING, "<%s:%d>(%s) \n"FORMAT, __FILE__, __LINE__, __FUNCTION__, ##ARG)
    //error
    #define DMB_LOGE(FORMAT, ARG...) dmbLog(LOGERROR, "<%s:%d>(%s) \n"FORMAT, __FILE__, __LINE__, __FUNCTION__, ##ARG)
    //fatal
    #define DMB_FATAL(FORMAT, ARG...) dmbLog(LOGFATAL, "<%s:%d>(%s) \n"FORMAT, __FILE__, __LINE__, __FUNCTION__, ##ARG)
    //assert
    #define DMB_ASSERT_X(cond, msg) ((!(cond)) ? dmbLog(LOGFATAL, "<%s:%d>(%s) \n%s", __FILE__, __LINE__, __FUNCTION__, msg) : cb_noop())

#else //!DMB_DEBUG

    #define DMB_LOGR(FORMAT, ARG...) dmbLog(LOGRUNTIME, FORMAT, ##ARG)
    #define DMB_LOG(LEVEL, FORMAT, ARG...)
    #define DMB_LOGD(FORMAT, ARG...)
    #define DMB_LOGI(FORMAT, ARG...)
    #define DMB_LOGW(FORMAT, ARG...) dmbLog(LOGWARNING, "<%s:%d>(%s) \n"FORMAT, __FILE__, __LINE__, __FUNCTION__, ##ARG)
    #define DMB_LOGE(FORMAT, ARG...) dmbLog(LOGERROR, "<%s:%d>(%s) \n"FORMAT, __FILE__, __LINE__, __FUNCTION__, ##ARG)
    #define DMB_FATAL(FORMAT, ARG...) dmbLog(LOGERROR, "<%s:%d>(%s) \n"FORMAT, __FILE__, __LINE__, __FUNCTION__, ##ARG)
    #define DMB_ASSERT_X(cond, msg)
#endif //DMB_DEBUG

#endif // DMBLOG_H
