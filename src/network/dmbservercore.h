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

#ifndef DMBSERVERCORE_H
#define DMBSERVERCORE_H

#include "dmbdefines.h"
#include "dmbnetwork.h"
#include "thread/dmbthread.h"

#define DMB_CLISO_ARR_SIZE 131072

typedef struct dmbWorkThreadData {
    dmbNetworkContext ctx;
    dmbNetworkListener listener;
    dmbThread thread;
    dmbPIPE  pipeArr[2];//0-read,1-write
    dmbBYTE cliSoDataArr[DMB_CLISO_ARR_SIZE]; //accept socket data
    dmbINT cliSoDataIndex;
    volatile dmbINT64 connCount;
} dmbWorkThreadData;

typedef struct dmbServerContext {
    dmbWorkThreadData *workThreadArr;
    dmbSOCKET acceptSocket;
    dmbThread acceptThread;
} dmbServerContext;

#define DMB_CLISO_ARR_LEN(DATA) (DMB_CLISO_ARR_SIZE - (DATA)->cliSoDataIndex)

extern void dmbStopApp();
extern dmbBOOL dmbIsAppQuit();

dmbCode dmbInitServerContext(dmbServerContext *pCtx);
void dmbPurgeServerContext(dmbServerContext *pCtx);

dmbCode dmbInitAcceptThread(dmbServerContext *pCtx);
dmbCode dmbQuitAcceptThread(dmbServerContext *pCtx);

dmbCode dmbInitWorkThreads(dmbServerContext *pCtx);
dmbCode dmbQuitWorkThreads(dmbServerContext *pCtx);

#endif // DMBSERVERCORE_H
