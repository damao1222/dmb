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

#ifndef DMBNETWORK_H
#define DMBNETWORK_H

#include "dmbdefines.h"
#include <sys/epoll.h>
#include "core/dmblist.h"

#define DMB_NW_NONE  0 //none, accept
#define DMB_NW_READ  1 //read
#define DMB_NW_WRITE 2 //write

typedef int dmbSOCKET;
typedef int dmbPIPE;

typedef struct epoll_event dmbNetworkEvent;

typedef struct dmbEpollData {
    dmbINT epfd;
    dmbUINT eventSize;
    dmbNetworkEvent events[];
} dmbEpollData;

typedef struct dmbConnReq {
    dmbBYTE *data;
    dmbUINT len;
    dmbCode (*merge) (struct dmbConnReq*, dmbBYTE*, dmbUINT);
    void (*release) (struct dmbConnReq*);
} dmbConnReq;

typedef struct dmbConnect {
    dmbINT cliFd;
    dmbBYTE *readBuf;
    dmbBOOL canRead;
    dmbBOOL canWrite;
    dmbBOOL isblocked;
    dmbUINT readIndex;
    dmbUINT readBufSize;
    dmbUINT readLength;
    dmbUINT requestIndex;
    dmbConnReq request;
    dmbBYTE *writeBuf;
    dmbUINT writeIndex;
    dmbUINT writeBufSize;
    dmbUINT writeLength;
    dmbBOOL needClose;
    dmbUINT timeout;
    dmbNode idleNode;
    dmbNode timeoutNode;
    dmbNode roundNode;
} dmbConnect;

typedef struct dmbNetworkListener{
    dmbCode (*onConnect)(void *);
    void (*onClosed)(void*);
    void *data;
} dmbNetworkListener;

typedef struct dmbNetworkContext {
    dmbEpollData *netData;
    dmbUINT connectSize;
    dmbList idleConnList;
    dmbList timeoutConnList;
    dmbList roundRobinList;
    dmbConnect *connects;
    dmbNetworkListener *listener;
} dmbNetworkContext;


dmbCode dmbNetworkInit(dmbNetworkContext *pCtx, dmbUINT uEventNum, dmbNetworkListener *pListener);

dmbCode dmbNetworkPurge(dmbNetworkContext *pCtx);

dmbCode dmbNetworkAddEvent(dmbNetworkContext *pCtx, dmbSOCKET iFd, dmbINT iMask, dmbConnect *pConn);

dmbCode dmbNetworkChangeEvent(dmbNetworkContext *pCtx, dmbINT iFd, dmbINT iMask, dmbConnect *pConn);

dmbCode dmbNetworkDelEvent(dmbNetworkContext *pCtx, dmbINT iFd, dmbINT iMask);

dmbCode dmbNetworkPoll(dmbNetworkContext *pCtx, dmbINT *iEventNum, dmbINT iTimeout);

dmbCode dmbNetworkOnLoop(dmbNetworkContext *pCtx, dmbSOCKET listener);

dmbCode dmbNetworkInitConnectPool(dmbNetworkContext *pCtx, dmbUINT uConnectSize, dmbUINT readBufSize, dmbUINT writeBufSize);

dmbCode dmbNetworkPurgeConnectPool(dmbNetworkContext *pCtx);

dmbCode dmbNetworkAccept(dmbSOCKET listener, dmbSOCKET *pSocket);

dmbCode dmbNetworkListen(dmbSOCKET *pFd, const char *addr, int port, int backlog);

dmbCode dmbNetworkInitNewConnect(dmbSOCKET client);

dmbCode dmbNetworkProcessNewConnect(dmbNetworkContext *pCtx, dmbSOCKET fd, dmbLONG timeout);

void dmbNetworkWatchTimeout(dmbNetworkContext *pCtx, dmbConnect *pConn, dmbLONG timeout);

dmbINT dmbNetworkCloseTimeoutConnect(dmbNetworkContext *pCtx);

dmbCode dmbNetworkCloseConnect(dmbNetworkContext *pCtx, dmbConnect *pConn);

#define dmbNetworkGetConnect(EVENT_PTR) ((dmbConnect*)(EVENT_PTR)->data.ptr)

#define dmbNetworkBadConnect(EVENT_PTR) (((EVENT_PTR)->events) & (EPOLLRDHUP | EPOLLHUP | EPOLLPRI | EPOLLERR))

#define dmbNetworkCanRead(EVENT_PTR) (((EVENT_PTR)->events) & EPOLLIN)

#define dmbNetworkCanWrite(EVENT_PTR) (((EVENT_PTR)->events) & EPOLLOUT)

#define dmbConnectCanRead(CONN_PTR) ((CONN_PTR)->canRead)

#define dmbConnectCanWrite(CONN_PTR) ((CONN_PTR)->canWrite)

#define dmbConnectIsBlocked(CONN_PTR) ((CONN_PTR)->isblocked)

#define dmbNetworkEventForeach(CTX, EVENT_PTR, INDEX, NUM) \
    for (INDEX=0, EVENT_PTR=&CTX->netData->events[INDEX]; \
            INDEX<NUM; \
        ({EVENT_PTR = &CTX->netData->events[INDEX]; INDEX++;}))

#endif // DMBNETWORK_H
