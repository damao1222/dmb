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

#ifndef DMBTIME_H
#define DMBTIME_H

#include "dmbdefines.h"

typedef struct dmbEndTime {
#ifdef DMB_NO_THREAD
    dmbLONG totle;
    dmbLONG begin;
#else
    volatile dmbLONG totle;
    volatile dmbLONG begin;
#endif
} dmbEndTime;


/**
 * @brief 获得格式化的系统时间
 *
 * @param pcBuf 保存时间的缓存指针
 * @param uSize 缓存大小
 * @return dmbBOOL 成功返回TRUE，失败返回FALSE
 */
dmbBOOL dmbGetFormatTime(char *pcBuf, dmbUINT uSize);

/**
 * @brief 获得格式化的系统时间
 *
 * @param pcFormat 指定的格式
 * @param pcBuf 保存时间的缓存指针
 * @param uSize 缓存大小
 * @return dmbBOOL 成功返回TRUE，失败返回FALSE
 */
dmbBOOL dmbGetSpecialFormatTime(const char *pcFormat, char *pcBuf, dmbUINT uSize);

/**
 * @brief 获得本地系统毫秒数
 *
 * @return dmbLONG 毫秒数
 */
dmbLONG dmbLocalCurrentMillis();

/**
 * @brief 获得本地系统秒数
 *
 * @return dmbLONG 秒数
 */
dmbLONG dmbLocalCurrentSec();

/**
 * @brief 获得系统毫秒数
 *
 * @return dmbLONG 毫秒数
 */
dmbLONG dmbSystemCurrentMillis();

/**
 * @brief 初始化程序运行时间
 */
void dmbInitAppClock();

/**
 * @brief 获得程序从运行到现在的毫秒数
 *
 * @return dmbLONG 毫秒数
 */
dmbLONG dmbGetAppClockMillis();

/**
 * @brief 初始化一个计时器
 *
 * @param pTime 计时器指针
 * @param uTotleTime 设置过期时间，单位毫秒
 */
void dmbEndTimeInit(dmbEndTime *pTime, dmbLONG uTotleTime);

/**
 * @brief 检测计时器是否过期
 *
 * @param pTime 计时器指针
 * @return dmbBOOL 过期返回TRUE，否则返回FALSE
 */
dmbBOOL dmbEndTimeIsExpired(dmbEndTime *pTime);

/**
 * @brief 将计时器设置为永不过期
 *
 * @param pTime 计时器指针
 */
void dmbEndTimeSetInfinite(dmbEndTime *pTime);

/**
 * @brief 检测计时器是否永不过期
 *
 * @param pTime 计时器指针
 * @return dmbBOOL 永不过期返回TRUE，否则返回FALSE
 */
dmbBOOL dmbEndTimeIsInfinite(dmbEndTime *pTime);

/**
 * @brief 获得计时器剩余过期时间
 *
 * @param pTime 计时器指针
 * @return dmbLONG 剩余时间，单位毫秒
 */
dmbLONG dmbEndTimeLeftTime(dmbEndTime *pTime);

/**
 * @brief 获得计时器从初始化开始过去的时间
 *
 * @param pTime 计时器指针
 * @return dmbLONG 过去的时间，单位毫秒
 */
dmbLONG dmbEndTimePastTime(dmbEndTime *pTime);

/**
 * @brief dmbSetVirualSystemTime 设置虚拟系统时间
 * @param lTime 虚拟系统时间
 */
void dmbSetVirualSystemTime(dmbLONG lTime);
#endif // DMBTIME_H
