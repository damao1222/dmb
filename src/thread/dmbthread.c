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

#include "dmbthread.h"
#include <unistd.h>

void* static_run(void *data);

void dmbThreadInit(dmbThread *pThread, dmbThreadFunc func, dmbCheckFunc check, void *pParam)
{
    pThread->func = func;
    pThread->checkFunc = check;
    pThread->id = 0;
    pThread->param = pParam;
}

dmbCode dmbThreadStart(dmbThread *pThread)
{
    if (pthread_create(&pThread->id, NULL, static_run, pThread) != 0)
        return DMB_ERRCODE_THREAD_ERROR;
    return DMB_ERRCODE_OK;
}

dmbCode dmbThreadJoin(dmbThread *pThread)
{
    if (pThread->id == 0)
        return DMB_ERRCODE_OK;

    return pthread_join(pThread->id, NULL) == 0 ? DMB_ERRCODE_OK : DMB_ERRCODE_THREAD_ERROR;
}

void dmbSleep(dmbUINT uMilliSec)
{
    usleep(uMilliSec * 1000);
}

void* static_run(void *data)
{
    dmbThread* pThread = (dmbThread*)(data);

    void * p = pThread->func(pThread);

    pthread_exit(p);

    return p;
}

