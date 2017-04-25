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

#include "dmbnetwork.h"
#include "core/dmballoc.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils/dmbioutil.h"

#define LINGER_TIMEOUT 60

static dmbConnect *GetIdleConn(dmbNetworkContext *pCtx);

dmbCode dmbNetworkInit(dmbNetworkContext *pCtx, dmbUINT uConnectSize, dmbUINT uEventNum)
{
    dmbCode code = DMB_ERRCODE_OK;
    dmbUINT i;
    if (pCtx == NULL)
    {
        return DMB_ERRCODE_NULL_POINTER;
    }

    pCtx->connects = (dmbConnect*)dmbMalloc(sizeof(dmbConnect) * uConnectSize);
    if (pCtx->connects == NULL)
    {
        return DMB_ERRCODE_ALLOC_FAILED;
    }

    do {
        pCtx->netData = (dmbEpollData*)dmbMalloc(sizeof(dmbEpollData) + sizeof(struct epoll_event) * uEventNum);

        if (pCtx->netData == NULL)
        {
            code = DMB_ERRCODE_ALLOC_FAILED;
            break;
        }

        do {
            pCtx->netData->epfd = epoll_create1(0);
            if (pCtx->netData->epfd == -1)
            {
                code = DMB_ERRCODE_NETWORK_ERROR;
                break;
            }

            dmbMemSet(pCtx->connects, 0, sizeof(dmbConnect) * uConnectSize);
            pCtx->netData->eventSize = uEventNum;
            pCtx->connectSize = uConnectSize;

            dmbListInit(&pCtx->idleConnList);

            for (i=0; i<uConnectSize; ++i)
            {
                dmbListPushBack(&pCtx->idleConnList, &pCtx->connects[i].node);
            }

            return code;
        } while (0);
        DMB_SAFE_FREE(pCtx->netData);
    } while (0);
    DMB_SAFE_FREE(pCtx->connects);

    return code;
}

dmbCode dmbNetworkPurge(dmbNetworkContext *pCtx)
{
    if (pCtx != NULL)
    {
        if (pCtx->netData != NULL)
        {
            if (pCtx->netData->epfd != -1)
            {
                dmbSafeClose(pCtx->netData->epfd);
                pCtx->netData->epfd = -1;
            }

            DMB_SAFE_FREE(pCtx->netData);
        }
        DMB_SAFE_FREE(pCtx->connects);
    }

    return DMB_ERRCODE_NULL_POINTER;
}

static dmbCode NetworkAddEvent(dmbNetworkContext *pCtx, int iFd, int iMask, dmbConnect *pConn, dmbBOOL add)
{
    struct epoll_event epEvent;
    epEvent.events = EPOLLET | EPOLLRDHUP;

    dmbINT opt = add ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    if (pConn)
        pConn->cliFd = iFd;
    epEvent.data.ptr = pConn;

    if (iMask & DMB_NW_READ) epEvent.events |= EPOLLIN;
    if (iMask & DMB_NW_WRITE) epEvent.events |= EPOLLOUT;

    if (epoll_ctl(pCtx->netData->epfd, opt, iFd, &epEvent) == -1)
        return DMB_ERRCODE_NETWORK_ERROR;

    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkAddEvent(dmbNetworkContext *pCtx, dmbSOCKET iFd, int iMask, dmbConnect *pConn)
{
    return NetworkAddEvent(pCtx, iFd, iMask, pConn, TRUE);
}

dmbCode dmbNetworkChangeEvent(dmbNetworkContext *pCtx, dmbINT iFd, dmbINT iMask, dmbConnect *pConn)
{
    return NetworkAddEvent(pCtx, iFd, iMask, pConn, FALSE);
}

dmbCode dmbNetworkDelEvent(dmbNetworkContext *pCtx, int iFd, int iMask)
{
    DMB_UNUSED(iMask);
    if (epoll_ctl(pCtx->netData->epfd, EPOLL_CTL_DEL, iFd, NULL) == -1)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }
    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkPoll(dmbNetworkContext *pCtx, dmbINT *iEventNum, dmbINT iTimeout)
{
    dmbINT iNum = epoll_wait(pCtx->netData->epfd, pCtx->netData->events, pCtx->netData->eventSize, iTimeout);
    if (iNum == -1)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }

    *iEventNum = iNum;
    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkCloseConnect(dmbNetworkContext *pCtx, dmbConnect *pConn)
{
    dmbINT ret = dmbSafeClose(pConn->cliFd);
    dmbListPushBack(&pCtx->idleConnList, &pConn->node);

    return ret == 0 ? DMB_ERRCODE_OK : DMB_ERRCODE_NERWORK_CLOSE_FAILED;
}

static void read_test(dmbConnect *pConn)
{
    dmbSafeRead(pConn->cliFd, pConn->readBuf, 10);
}

static void write_test(dmbConnect *pConn)
{
    dmbMemCopy(pConn->writeBuf, pConn->readBuf, 10);
    dmbSafeWrite(pConn->cliFd, pConn->writeBuf, 10);
}

dmbCode dmbNetworkOnLoop(dmbNetworkContext *pCtx, dmbSOCKET listener)
{
    dmbINT num, i;
    dmbSOCKET client;
    dmbConnect *pConn = NULL;
    dmbCode code = dmbNetworkPoll(pCtx, &num, 10000);
    if (code != DMB_ERRCODE_OK)
        return code;

    for (i=0; i<num; ++i)
    {
        if (pCtx->netData->events[i].data.ptr == NULL)
        {
            code = dmbNetworkAccept(listener, &client);
            if (code != DMB_ERRCODE_OK)
                return code;

            code = dmbNetworkProcessNewConnect(pCtx, client);
            if (code != DMB_ERRCODE_OK)
                return code;

            continue;
        }

        pConn = pCtx->netData->events[i].data.ptr;

        if (pCtx->netData->events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLPRI | EPOLLERR))
        {
            dmbNetworkCloseConnect(pCtx, pConn);
            continue;
        }

        if (pCtx->netData->events[i].events & EPOLLIN)
            read_test(pConn);
        if (pCtx->netData->events[i].events & EPOLLOUT)
            write_test(pConn);
    }

    return code;
}

dmbCode dmbNetworkInitBuffer(dmbNetworkContext *pCtx, dmbUINT readBufSize, dmbUINT writeBufSize)
{
    dmbUINT i = 0;
    dmbBYTE *whole = dmbMalloc(pCtx->connectSize * readBufSize + pCtx->connectSize * writeBufSize);\
    if (whole == NULL)
        return DMB_ERRCODE_ALLOC_FAILED;

    for (; i<pCtx->connectSize; ++i)
    {
        pCtx->connects[i].readBuf = whole;
        whole += readBufSize;
        pCtx->connects[i].writeBuf = whole;
        whole += writeBufSize;
    }

    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkPurgeBuffer(dmbNetworkContext *pCtx)
{
    DMB_SAFE_FREE(pCtx->connects[0].readBuf);
    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkAccept(dmbSOCKET listener, dmbSOCKET *pSocket)
{
    int fd;
    int err;
    struct sockaddr_in sa;
    dmbMemSet((void *) &sa, 0, sizeof(struct sockaddr_in));
    socklen_t salen = sizeof(sa);
    while(1)
    {
        fd = accept(listener, (struct sockaddr*)&sa, &salen);
        if (fd == -1)
        {
            err = errno;
            if (err == EINTR)
                continue;
            else
            {
                //continue on next epoll loop
                if (err == EAGAIN || err == EWOULDBLOCK)
                {
                    return DMB_ERRCODE_NETWORK_AGAIN;
                }
                return DMB_ERRCODE_NETWORK_ERROR;
            }
        }
        break;
    }
    *pSocket = fd;
    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkNonblocking(int fd, int nonblock)
{
    //设置非阻塞IO
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return DMB_ERRCODE_NETWORK_ERROR;

    if (nonblock)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) < 0)
        return DMB_ERRCODE_NETWORK_ERROR;

    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkReuse(int fd)
{
    int reuseOpt = 1;
    //SO_REUSEADDR
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &reuseOpt, sizeof(reuseOpt)) < 0)
        return DMB_ERRCODE_NETWORK_ERROR;
    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkLinger(int fd, int on, int timeout)
{
    struct linger lingerOpt;
    lingerOpt.l_onoff = on;
    lingerOpt.l_linger = timeout;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &lingerOpt, sizeof(struct linger)) < 0)
        return DMB_ERRCODE_NETWORK_ERROR;

    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkKeepAlive(int fd, int interval)
{
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }

#ifdef __linux__
    val = interval;
    //对一个连接进行有效性探测之前运行的最大非活跃时间间隔，空闲时每隔val秒发送一次心跳包，系统默认值为14400（即 2 个小时）
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }

    //两个探测的时间间隔，单位秒，系统默认值为150即75秒
    val = interval/3;
    if (val == 0)
        val = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }

    val = 3;
    //关闭一个非活跃连接之前进行探测的最大次数，系统默认为8次
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }
#else
    DMB_UNUSED(interval);
#endif

    return DMB_ERRCODE_OK;
}

//1 enable 0 disable
dmbCode dmbNetworkSetTcpNoDelay(int fd, int val)
{
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }
    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkTcpKeepAlive(int fd)
{
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
    {
        return DMB_ERRCODE_NETWORK_ERROR;
    }

    return DMB_ERRCODE_OK;
}

dmbCode dmbNetworkListen(dmbSOCKET *pFd, const char *addr, int port, int backlog)
{
    dmbSOCKET listenfd = -1;
    dmbCode err = DMB_ERRCODE_OK;
    struct sockaddr_in serveraddr;

    *pFd = -1;
    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd == -1)
        return DMB_ERRCODE_NETWORK_ERROR;

    //非阻塞
    err = dmbNetworkNonblocking(listenfd, TRUE);
    if (err != DMB_ERRCODE_OK)
        return DMB_ERRCODE_NETWORK_ERROR;

    //地址重用
    err = dmbNetworkReuse(listenfd);
    if (err != DMB_ERRCODE_OK)
        return DMB_ERRCODE_NETWORK_ERROR;

    //SO_LINGER 若设置了SO_LINGER并确定了非零的超时间隔，则closesocket()调用阻塞进程，直到所剩数据发送完毕或超时
    err = dmbNetworkLinger(listenfd, TRUE, LINGER_TIMEOUT);
    if (err != DMB_ERRCODE_OK)
        return DMB_ERRCODE_NETWORK_ERROR;

    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;

    inet_aton(addr, &(serveraddr.sin_addr));

    serveraddr.sin_port = htons(port);

    if (bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return DMB_ERRCODE_NETWORK_ERROR;

    if (listen(listenfd, backlog) < 0)
        return DMB_ERRCODE_NETWORK_ERROR;

    *pFd = listenfd;

    return err;
}

dmbCode dmbNetworkInitNewConnect(dmbSOCKET client)
{
    dmbCode code = dmbNetworkSetTcpNoDelay(client, TRUE);
    if (code != DMB_ERRCODE_OK)
        return code;

    code = dmbNetworkKeepAlive(client, 5);
    if (code != DMB_ERRCODE_OK)
        return code;

    code = dmbNetworkNonblocking(client, TRUE);
    if (code != DMB_ERRCODE_OK)
        return code;

    return code;
}

dmbCode dmbNetworkProcessNewConnect(dmbNetworkContext *pCtx, dmbSOCKET fd)
{
    dmbCode code = dmbNetworkInitNewConnect(fd);
    if (code != DMB_ERRCODE_OK)
        return code;

    dmbConnect *pConn = GetIdleConn(pCtx);
    if (pConn == NULL)
        return DMB_ERRCODE_NETWORK_ERROR;

    return dmbNetworkAddEvent(pCtx, fd, DMB_NW_READ | DMB_NW_WRITE, pConn);
}

static dmbConnect *GetIdleConn(dmbNetworkContext *pCtx)
{
    dmbNode *node = dmbListPopFront(&pCtx->idleConnList);
    if (node == NULL)
        return NULL;

    return dmbListEntry(node, dmbConnect, node);
}
