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

#define CB_NW_NONE  0 //none, accept
#define CB_NW_READ  1 //read
#define CB_NW_WRITE 2 //write

typedef int dmbSocket;

typedef struct dmbEpollData {
    dmbINT epfd;
    dmbUINT eventSize;
    struct epoll_event events[];
} dmbEpollData;


typedef struct dmbConnect {
    dmbINT cliFd;
    dmbBYTE *readBuf;
    dmbBYTE *writeBuf;
    dmbUINT writeIndex;
    dmbUINT writeSize;
    dmbNode node;
} dmbConnect;

typedef struct dmbNetworkContext {
    dmbEpollData *netData;
    dmbUINT connectSize;
    dmbList idleConnList;
    dmbConnect *connects;
} dmbNetworkContext;


dmbCode dmbNetworkInit(dmbNetworkContext *pCtx, dmbUINT uConnectSize, dmbUINT uFdSize, dmbUINT uEventNum);

dmbCode dmbNetworkPurge(dmbNetworkContext *pCtx);

dmbCode dmbNetworkAddEvent(dmbNetworkContext *pCtx, dmbINT iFd, dmbINT iMask, dmbConnect *pConn);

dmbCode dmbNetworkChangeEvent(dmbNetworkContext *pCtx, dmbINT iFd, dmbINT iMask, dmbConnect *pConn);

dmbCode dmbNetworkDelEvent(dmbNetworkContext *pCtx, dmbINT iFd, dmbINT iMask);

dmbCode dmbNetworkPoll(dmbNetworkContext *pCtx, dmbINT *iEventNum, dmbINT iTimeout);

dmbCode dmbNetworkOnLoop(dmbNetworkContext *pCtx, dmbSocket listener);

dmbCode dmbNetworkInitBuffer(dmbNetworkContext *pCtx, dmbUINT readBufSize, dmbUINT writeBufSize);

dmbCode dmbNetworkPurgeBuffer(dmbNetworkContext *pCtx);

dmbCode dmbNetworkAccept(dmbSocket listener, dmbSocket *pSocket);

dmbCode dmbNetworkListen(dmbSocket *pFd, const char *addr, int port, int backlog);

dmbCode dmbNetworkProcessNewConnect(dmbNetworkContext *pCtx, dmbSocket fd);


#endif // DMBNETWORK_H
