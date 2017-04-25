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

#ifndef DMBTHREAD_H
#define DMBTHREAD_H

#include "dmbdefines.h"
#include <pthread.h>

typedef void* dmbThreadData;
typedef dmbBOOL (*dmbCheckFunc)();
typedef void * (*dmbThreadFunc) (dmbThreadData);

typedef struct dmbThread{
    pthread_t id;
    dmbThreadFunc func;
    dmbCheckFunc checkFunc;
    void *param;
} dmbThread;

void dmbThreadInit(dmbThread *pThread, dmbThreadFunc func, dmbCheckFunc check, void *pParam);
dmbCode dmbThreadStart(dmbThread *pThread);
dmbCode dmbThreadJoin(dmbThread *pThread);
dmbBOOL dmbThreadRunning(dmbThreadData data);
void* dmbThreadGetParam(dmbThreadData data);

#define dmbThreadRunning(DATA) \
    (((dmbThread*)(DATA))->checkFunc ? ((dmbThread*)(DATA))->checkFunc() : TRUE)

#define dmbThreadGetParam(DATA) (((dmbThread*)(data))->param)

#endif // DMBTHREAD_H
