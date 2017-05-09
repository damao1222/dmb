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
#include "thread/dmbatomic.h"
#include <sys/socket.h>
#include "dmbprotocol.h"

#define DEFAULT_SELECT_TIMEOUT 5 //second
#define DEFAULT_SELECT_EPOLL_TIMEOUT 5000 //millisecond
#define DEFAULT_EPOLL_EVENTNUM 10240

void * acceptThreadImpl (dmbThreadData data);
void * workThreadImpl (dmbThreadData data);

static dmbCode OnConnect(void *p)
{
    DMB_UNUSED(p);
    return DMB_ERRCODE_OK;
}

static void OnClosed(void *p)
{
    dmbWorkThreadData *pData = (dmbWorkThreadData*)p;
    dmbAtomicDecr(&pData->connCount);
}

dmbCode dmbInitServerContext(dmbServerContext *pCtx)
{
    dmbCode code = DMB_ERRCODE_OK;
    pCtx->acceptSocket = DMB_INVALID_FD;

    pCtx->workThreadArr = (dmbWorkThreadData*) dmbMalloc(sizeof(dmbWorkThreadData) * g_settings.thread_size);
    dmbINT i;

    if (pCtx->workThreadArr == NULL)
    {
        return DMB_ERRCODE_ALLOC_FAILED;
    }

    for (i=0; i<g_settings.thread_size; ++i)
    {
        pCtx->workThreadArr[i].pipeArr[0] = DMB_INVALID_FD;
        pCtx->workThreadArr[i].pipeArr[1] = DMB_INVALID_FD;

        dmbMemSet(pCtx->workThreadArr[i].cliSoDataArr, 0, DMB_CLISO_ARR_SIZE);
        pCtx->workThreadArr[i].cliSoDataIndex = 0;
        pCtx->workThreadArr[i].connCount = 0;
    }

    return code;
}

void dmbPurgeServerContext(dmbServerContext *pCtx)
{
    pCtx->acceptSocket = DMB_INVALID_FD;

    DMB_SAFE_FREE(pCtx->workThreadArr);
}

dmbCode dmbInitAcceptThread(dmbServerContext *pCtx)
{
    dmbCode code = dmbNetworkListen(&pCtx->acceptSocket, g_settings.host, g_settings.port, g_settings.listen_backlog);
    if (code != DMB_ERRCODE_OK)
        return code;

    dmbThreadInit(&pCtx->acceptThread, acceptThreadImpl, dmbIsAppQuit, pCtx);
    return dmbThreadStart(&pCtx->acceptThread);
}

dmbCode dmbQuitAcceptThread(dmbServerContext *pCtx)
{
    dmbCode code = DMB_ERRCODE_OK;
    if (pCtx->acceptThread.id != 0)
    {
        code = dmbThreadJoin(&pCtx->acceptThread);
    }

    if (pCtx->acceptSocket != DMB_INVALID_FD)
    {
        code = dmbSafeClose(pCtx->acceptSocket);
        pCtx->acceptSocket = DMB_INVALID_FD;
    }

    return code;
}

dmbCode dmbInitWorkThreads(dmbServerContext *pCtx)
{
    dmbCode code = DMB_ERROR;
    dmbINT i;
    for (i=0; i<g_settings.thread_size; ++i)
    {
        dmbNetworkListener *l = &pCtx->workThreadArr[i].listener;
        l->onConnect = OnConnect;
        l->onClosed = OnClosed;
        l->data = &pCtx->workThreadArr[i];
        code = dmbNetworkInit(&pCtx->workThreadArr[i].ctx, DEFAULT_EPOLL_EVENTNUM, l);
        if (code != DMB_ERRCODE_OK)
            return code;

        code = dmbNetworkInitConnectPool(&pCtx->workThreadArr[i].ctx, g_settings.connect_size_per_thread, g_settings.net_read_bufsize, g_settings.net_write_bufsize);
        if (code != DMB_ERRCODE_OK)
            return code;

        if (pipe(pCtx->workThreadArr[i].pipeArr) != 0)
            return DMB_ERRCODE_NETWORK_ERROR;

        code = dmbNetworkAddEvent(&pCtx->workThreadArr[i].ctx, pCtx->workThreadArr[i].pipeArr[0], DMB_NW_READ, NULL);
        if (code != DMB_ERRCODE_OK)
            return code;

        dmbThreadInit(&pCtx->workThreadArr[i].thread, workThreadImpl, dmbIsAppQuit, &pCtx->workThreadArr[i]);

        code = dmbThreadStart(&pCtx->workThreadArr[i].thread);
        if (code != DMB_ERRCODE_OK)
            return code;
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
            dmbThreadJoin(&pCtx->workThreadArr[i].thread);
            if (pCtx->workThreadArr[i].pipeArr[0] != DMB_INVALID_FD)
            {
                dmbSafeClose(pCtx->workThreadArr[i].pipeArr[0]);
                pCtx->workThreadArr[i].pipeArr[0] = DMB_INVALID_FD;
            }

            if (pCtx->workThreadArr[i].pipeArr[1] != DMB_INVALID_FD)
            {
                dmbSafeClose(pCtx->workThreadArr[i].pipeArr[1]);
                pCtx->workThreadArr[i].pipeArr[1] = DMB_INVALID_FD;
            }

            dmbNetworkPurgeConnectPool(&pCtx->workThreadArr[i].ctx);
            dmbNetworkPurge(&pCtx->workThreadArr[i].ctx);
        }
    }
    return DMB_ERRCODE_OK;
}

dmbWorkThreadData* findIdleThread(dmbServerContext *pCtx, dmbINT *pCur)
{
    dmbINT i;
    dmbWorkThreadData* pData = NULL;
    dmbINT64 iCount64;

    for (i=0; i<g_settings.thread_size; ++(*pCur), ++i)
    {
        *pCur %= g_settings.thread_size;
        pData = &pCtx->workThreadArr[*pCur];
        iCount64 = dmbAtomicIncr(&pData->connCount);
        if (iCount64 > g_settings.connect_size_per_thread)
        {
            dmbAtomicDecr(&pData->connCount);
            pData = NULL;
            continue;
        }

        return pData;
    }
    return NULL;
}

void * acceptThreadImpl (dmbThreadData data)
{
    fd_set fdacc;
    int iRet, iCur = 0;
    dmbSOCKET client;
    struct timeval tiAcceptTimeOut = { DEFAULT_SELECT_TIMEOUT, 0 };
    dmbServerContext *pCtx = (dmbServerContext*)dmbThreadGetParam(data);
    dmbWorkThreadData *pWorkData;

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
            pWorkData = findIdleThread(pCtx, &iCur);
            if (pWorkData == NULL)
            {
                dmbSafeClose(client);
                continue;
            }
            iRet = dmbSafeWrite(pWorkData->pipeArr[1], (dmbBYTE*) &client, sizeof(dmbSOCKET));
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

static dmbCode processNewConnect(dmbWorkThreadData *pData)
{
    dmbINT i, iCount, iRemain;
    dmbINT iReadLen = dmbReadAvailable(pData->pipeArr[0], pData->cliSoDataArr+pData->cliSoDataIndex, DMB_CLISO_ARR_LEN(pData));
    DMB_ASSERT(iReadLen > 0);
    pData->cliSoDataIndex += iReadLen;
    iCount = pData->cliSoDataIndex / sizeof(dmbSOCKET);
    iRemain = pData->cliSoDataIndex % sizeof(dmbSOCKET);
    dmbSOCKET *pSocket = (dmbSOCKET*)pData->cliSoDataArr;
    if (iCount > 0)
    {
        for (i = 0; i<iCount; pSocket++, i++)
        {
            if (dmbNetworkProcessNewConnect(&pData->ctx, *pSocket, g_settings.net_rw_timeout) == DMB_ERRCODE_OK)
            {
                //Dont care about this.
            }
        }

        iCount *= sizeof(dmbSOCKET);
        dmbMemMove(pData->cliSoDataArr, pData->cliSoDataArr+iCount, iRemain);
        pData->cliSoDataIndex -= iCount;
    }

    return DMB_ERRCODE_OK;
}

static void processRoundRobin(dmbNetworkContext *pCtx)
{
    dmbNode *pNode;
    while (!dmbListIsEmpty(&pCtx->roundRobinList))
    {
        pNode = dmbListPopFront(&pCtx->roundRobinList);
        dmbProcessEvent(pCtx, dmbListEntry(pNode, dmbConnect, roundNode));
    }
}

void * workThreadImpl (dmbThreadData data)
{
    dmbCode code = DMB_ERRCODE_OK;
    dmbWorkThreadData *pThreadData = (dmbWorkThreadData*)dmbThreadGetParam(data);
    dmbNetworkContext *pCtx = &pThreadData->ctx;
    dmbINT iNum = 0, i;
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
                code = processNewConnect(pThreadData);
                continue;
            }

            if (dmbNetworkBadConnect(pEvent))
            {
                dmbNetworkCloseConnect(pCtx, pConn);
                continue;
            }

            if (dmbNetworkCanRead(pEvent))
                pConn->canRead = TRUE;

            if (dmbNetworkCanWrite(pEvent))
                pConn->canWrite = TRUE;

            dmbProcessEvent(pCtx, pConn);
        }

        processRoundRobin(pCtx);

        dmbNetworkCloseTimeoutConnect(pCtx);
    }

    return NULL;
}

