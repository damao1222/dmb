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

#include "dmbservercore.h"
#include "base/dmbsettings.h"
#include "core/dmballoc.h"
#include "utils/dmbioutil.h"
#include <sys/socket.h>

#define DEFAULT_SELECT_TIMEOUT 5 //second
#define DEFAULT_SELECT_EPOLL_TIMEOUT 5000 //millisecond
#define DEFAULT_EPOLL_EVENTNUM 10240
#define INVALID_FD -1

void * acceptThreadImpl (dmbThreadData data);
void * workThreadImpl (dmbThreadData data);

static volatile dmbBOOL g_app_run = TRUE;

dmbBOOL IsAppQuit()
{
    return g_app_run;
}

void dmbStopApp()
{
    g_app_run = FALSE;
}

dmbCode dmbInitServerContext(dmbServerContext *pCtx)
{
    dmbCode code = DMB_ERRCODE_OK;
    pCtx->acceptSocket = INVALID_FD;

    pCtx->workThreadArr = (dmbWorkThreadData*) dmbMalloc(sizeof(dmbWorkThreadData) * g_settings.thread_size);
    dmbINT i;

    if (pCtx->workThreadArr == NULL)
    {
        return DMB_ERRCODE_ALLOC_FAILED;
    }

    for (i=0; i<g_settings.thread_size; ++i)
    {
        pCtx->workThreadArr[i].pipeArr[0] = INVALID_FD;
        pCtx->workThreadArr[i].pipeArr[1] = INVALID_FD;
    }

    return code;
}

void dmbPurgeServerContext(dmbServerContext *pCtx)
{
    pCtx->acceptSocket = INVALID_FD;

    DMB_SAFE_FREE(pCtx->workThreadArr);
}

dmbCode dmbInitAcceptThread(dmbServerContext *pCtx)
{
    dmbCode code = dmbNetworkListen(&pCtx->acceptSocket, g_settings.host, g_settings.port, g_settings.listen_backlog);
    if (code != DMB_ERRCODE_OK)
        return code;

    dmbThreadInit(&pCtx->acceptThread, acceptThreadImpl, IsAppQuit, pCtx);
    return dmbThreadStart(&pCtx->acceptThread);
}

dmbCode dmbQuitAcceptThread(dmbServerContext *pCtx)
{
    dmbCode code = DMB_ERRCODE_OK;
    if (pCtx->acceptThread.id != 0)
    {
        code = dmbThreadJoin(&pCtx->acceptThread);
    }

    if (pCtx->acceptSocket != INVALID_FD)
    {
        code = dmbSafeClose(pCtx->acceptSocket);
        pCtx->acceptSocket = INVALID_FD;
    }

    return code;
}

dmbCode dmbInitWorkThreads(dmbServerContext *pCtx)
{
    dmbCode code = DMB_ERROR;
    dmbINT i;
    for (i=0; i<g_settings.thread_size; ++i)
    {
        code = dmbNetworkInit(&pCtx->workThreadArr[i].ctx, g_settings.connect_size_per_thread, DEFAULT_EPOLL_EVENTNUM);
        if (code != DMB_ERRCODE_OK)
            return code;

        code = dmbNetworkInitBuffer(&pCtx->workThreadArr[i].ctx, g_settings.net_read_bufsize, g_settings.net_write_bufsize);
        if (code != DMB_ERRCODE_OK)
            return code;

        if (pipe(pCtx->workThreadArr[i].pipeArr) != 0)
            return DMB_ERRCODE_NETWORK_ERROR;

        code = dmbNetworkAddEvent(&pCtx->workThreadArr[i].ctx, pCtx->workThreadArr[i].pipeArr[0], DMB_NW_READ, NULL);
        if (code != DMB_ERRCODE_OK)
            return code;

        dmbThreadInit(&pCtx->workThreadArr[i].thread, workThreadImpl, IsAppQuit, &pCtx->workThreadArr[i].ctx);

        return dmbThreadStart(&pCtx->workThreadArr[i].thread);
    }
    return code;
}

dmbCode dmbQuitWorkThreads(dmbServerContext *pCtx)
{
    dmbINT i;
    if (pCtx->workThreadArr != NULL)
    {
        for (i=0;i<g_settings.thread_size; ++i)
        {
            dmbThreadJoin(&pCtx->workThreadArr->thread);
            if (pCtx->workThreadArr[i].pipeArr[0] != INVALID_FD)
            {
                dmbSafeClose(pCtx->workThreadArr[i].pipeArr[0]);
                pCtx->workThreadArr[i].pipeArr[0] = INVALID_FD;
            }

            if (pCtx->workThreadArr[i].pipeArr[1] != INVALID_FD)
            {
                dmbSafeClose(pCtx->workThreadArr[i].pipeArr[1]);
                pCtx->workThreadArr[i].pipeArr[1] = INVALID_FD;
            }

            dmbNetworkPurgeBuffer(&pCtx->workThreadArr[i].ctx);
            dmbNetworkPurge(&pCtx->workThreadArr[i].ctx);
        }
    }
    return DMB_ERROR;
}

void * acceptThreadImpl (dmbThreadData data)
{
    fd_set fdacc;
    int iRet;
    dmbSOCKET client;
    struct timeval tiAcceptTimeOut = { DEFAULT_SELECT_TIMEOUT, 0 };
    dmbServerContext *pCtx = (dmbServerContext*)dmbThreadGetParam(data);
    while (dmbThreadRunning(data))
    {
        FD_ZERO(&fdacc);
        FD_SET(pCtx->acceptSocket, &fdacc);

        iRet = select(pCtx->acceptSocket + 1, &fdacc, 0, 0, &tiAcceptTimeOut);
        if (iRet > 0)
        {
            if (dmbNetworkAccept(pCtx->acceptSocket, &client) != DMB_ERRCODE_OK)
                continue ;

            if (dmbNetworkInitNewConnect(client) != DMB_ERRCODE_OK)
            {
                dmbSafeClose(client);
                continue;
            }

            //add to work thread
        }
        //time out
        else if (iRet == 0)
        {
            continue;
        }
        else
        {
            //error, do nothing
        }
    }

    return NULL;
}

void * workThreadImpl (dmbThreadData data)
{
    dmbCode code = DMB_ERRCODE_OK;
    dmbNetworkContext *pCtx = (dmbNetworkContext*)dmbThreadGetParam(data);
    dmbINT iNum = 0, i;
    dmbSOCKET client = INVALID_FD;
    dmbConnect *pConn;
    dmbNetworkEvent *pEvent;

    while (dmbThreadRunning(data))
    {
        code = dmbNetworkPoll(pCtx, &iNum, DEFAULT_SELECT_EPOLL_TIMEOUT);
        if (code != DMB_ERRCODE_OK)
            continue ;

        dmbNetworkEventForeach(pCtx, pEvent, i, iNum)
        {
            pConn = dmbNetworkGetConnect(pEvent);

            if (pConn == NULL)
            {
                code = dmbNetworkProcessNewConnect(pCtx, client);
                continue;
            }

            if (dmbNetworkBadConnect(pEvent))
            {
                dmbNetworkCloseConnect(pCtx, pConn);
                continue;
            }

            if (dmbNetworkCanRead(pEvent))
            {
//                read_test(pConn);
            }

            if (dmbNetworkCanWrite(pEvent))
            {
//                write_test(pConn);
            }
        }
    }

    return NULL;
}
