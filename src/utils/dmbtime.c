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
#include "dmbtime.h"
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <limits.h>

volatile dmbLONG g_virtual_system_time = 0;
static dmbLONG g_start_time = 0;

void dmbSetVirualSystemTime(dmbLONG lTime)
{
    g_virtual_system_time = lTime;
}

dmbBOOL dmbGetFormatTime(char *pcBuf, dmbUINT uSize)
{
    time_t timer = time(NULL);
    struct tm result;
    return strftime(pcBuf, uSize, "%Y-%m-%d %H:%M:%S", localtime_r(&timer, &result)) > 0;
}

dmbBOOL dmbGetSpecialFormatTime(const char *pcFormat, char *pcBuf, dmbUINT uSize)
{
    time_t timer = time(NULL);
    struct tm result;
    return strftime(pcBuf, uSize, pcFormat, localtime_r(&timer, &result)) > 0;
}

dmbLONG dmbLocalCurrentMillis()
{
    struct timeval time;
    gettimeofday(&time, NULL );

    return (1000L *  time.tv_sec + time.tv_usec / 1000L);
}

inline dmbLONG dmbLocalCurrentSec()
{
    return time(NULL);
}

dmbLONG dmbSystemCurrentMillis()
{
    return g_virtual_system_time == 0 ? dmbLocalCurrentMillis() : g_virtual_system_time;
}

void dmbInitAppClock()
{
    struct timeval time;
    gettimeofday(&time, NULL );
	g_start_time= 1000 *  time.tv_sec + time.tv_usec / 1000;
}

dmbLONG dmbGetAppClockMillis()
{
    struct timeval time;
    gettimeofday(&time, NULL );

    dmbLONG now_time = 1000 *  time.tv_sec + time.tv_usec / 1000;
    return (now_time - g_start_time);
}

void dmbEndTimeInit(dmbEndTime *pTime, dmbLONG uTotleTime)
{
    pTime->totle = uTotleTime;
    pTime->begin = dmbGetAppClockMillis();
}

void dmbEndTimeSetInfinite(dmbEndTime *pTime)
{
    pTime->totle = LONG_MAX;
}

dmbBOOL dmbEndTimeIsExpired(dmbEndTime *pTime)
{
    if (pTime->totle == 0)
            return TRUE;
    else if (dmbEndTimeIsInfinite(pTime))
        return FALSE;
    else if (dmbEndTimePastTime(pTime) >= pTime->totle)
        return TRUE;

    return FALSE;
}

dmbLONG dmbEndTimeLeftTime(dmbEndTime *pTime)
{
    if (dmbEndTimeIsInfinite(pTime))
        return LONG_MAX;

    dmbLONG past = dmbEndTimePastTime(pTime);
    return past >= pTime->totle ? 0 : pTime->totle - past;
}

dmbBOOL dmbEndTimeIsInfinite(dmbEndTime *pTime)
{
    return pTime->totle == LONG_MAX;
}

dmbLONG dmbEndTimePastTime(dmbEndTime *pTime)
{
    return dmbGetAppClockMillis() - pTime->begin;
}

