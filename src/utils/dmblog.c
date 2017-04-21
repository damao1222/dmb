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

#include "dmblog.h"
#include "core/dmballoc.h"
#include "dmbfilesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include "dmbtime.h"
#include "utils/dmbioutil.h"

#define MSG_BUFFER_SIZE 4096
#define DMB_LOG_FILENAME "dmb.log"
static const char LEVEL_NAMES[][8] =
{"DEBUG", "INFO", "RUNTIME", "WARNING", "ERROR", "FATAL", "NONE"};
static const char* PREFIX_FORMAT = "[dmb %s %s]: ";

static int g_log_level = LOGDEBUG;
static pthread_mutex_t g_log_mutex;
static int g_log_fd = -1;
static dmbBOOL g_MutexInited = FALSE;

static dmbBOOL Output2File(const char *pcMsg)
{
    if (g_log_fd != -1)
    {
        return dmbSafeWrite(g_log_fd, (void*)pcMsg, strlen(pcMsg)) != -1;
    }
    return FALSE;
}

void OutputMsg(const char *pcMsg)
{
    Output2File(pcMsg);
    fprintf(stderr, "%s", pcMsg);
    fflush(stderr);
}

dmbBOOL dmbLogInit(const char *pcLogFileDir, dmbINT iLogLevel)
{
	dmbBOOL ret=FALSE;
    do
    {
        dmbInitAppClock();

        g_MutexInited= (pthread_mutex_init(&g_log_mutex, NULL) == 0);
        if(!g_MutexInited)
        {
        	DMB_LOGR("init log mutex failed");
        	break;
        }

        if(!dmbLogConfig(pcLogFileDir,iLogLevel))
        	break;

        ret=TRUE;
    }while(FALSE);

    if(!ret)
    	dmbLogPurge();

    return ret;
}

void dmbLogPurge()
{
    if (g_log_fd != -1)
    {
        //SafeClose(g_log_fd);
        g_log_fd=-1;
    }

    if (g_MutexInited == TRUE)
    {
        pthread_mutex_destroy(&g_log_mutex);
        g_MutexInited=FALSE;
    }
}

void dmbLog(int level, const char *format, ...)
{
    char MSGBUF[MSG_BUFFER_SIZE];
    char TIMEBUF[20];

    if (g_log_level > level)
        return ;

    dmbMemSet(MSGBUF, 0 ,MSG_BUFFER_SIZE);
    dmbGetFormatTime(TIMEBUF, 20);
    snprintf(MSGBUF, 36, PREFIX_FORMAT, TIMEBUF, LEVEL_NAMES[level]);
    va_list ArgPtr;
    va_start(ArgPtr, format);
    int len = strlen(MSGBUF);
    vsnprintf(MSGBUF+len, MSG_BUFFER_SIZE-len, format, ArgPtr);
    va_end(ArgPtr);

    pthread_mutex_lock(&g_log_mutex);
    OutputMsg(MSGBUF);
    pthread_mutex_unlock(&g_log_mutex);
    DMB_ASSERT(level <= LOGERROR);
}

void dmbLogSystemInfo()
{
    DMB_LOGR("<System>: Used memory size is %d\n", dmbGetUsedMemSize());
}

dmbBOOL dmbLogConfig(const char *pcLogFileDir, dmbINT iLogLevel)
{
	dmbBOOL ret=FALSE;
	do
	{
	    g_log_level = iLogLevel;
	    if(pcLogFileDir==NULL)
	    {
	    	DMB_LOGR("log file config error");
	    	break;
	    }

        if(!dmbIsDirExists(pcLogFileDir) && !dmbMkdir(pcLogFileDir))
	    {
	    	DMB_LOGR("make log file  dir error");
	    	break;
	    }

        char pcLogFilePath[256] = {0};
        strcpy(pcLogFilePath, pcLogFileDir);
        strcat(pcLogFilePath, DMB_LOG_FILENAME);

        //g_log_fd = SafeOpen(pcLogFilePath, O_CREAT | O_APPEND | O_RDWR, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
        if(g_log_fd==-1)
        {
        	 DMB_LOGE("Open log file failed");
        	 break;
        }

        char szBuf[256] = {0};
        dmbGetFormatTime(szBuf, sizeof(szBuf));
        DMB_LOGR("System : Log start up > %s\n", szBuf);


	    ret=TRUE;
	}while(FALSE);

	return ret;
}
