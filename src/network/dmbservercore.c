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
#include "dmbnetwork.h"
#include "thread/dmbthread.h"

void * acceptThreadImpl (dmbCheckFunc func);
static dmbThread g_accept_thread;
static volatile dmbBOOL g_app_run = TRUE;

dmbBOOL IsAppQuit()
{
    return g_app_run;
}

void dmbStopApp()
{
    g_app_run = FALSE;
}

dmbCode dmbInitAcceptThread()
{
    dmbThreadInit(&g_accept_thread, acceptThreadImpl, IsAppQuit);
    return dmbThreadStart(&g_accept_thread);
}

dmbCode dmbQuitAcceptThread()
{
    if (g_accept_thread.id == 0)
        return DMB_ERRCODE_OK;
    dmbCode code = dmbThreadJoin(&g_accept_thread);

    return code;
}

dmbCode dmbInitWorkThreads()
{
    return DMB_ERROR;
}

dmbCode dmbQuitWorkThreads()
{
    return DMB_ERROR;
}

void * acceptThreadImpl (dmbCheckFunc func)
{
//    dmbCode code = dmbNetworkListen();
    while (func())
    {
//        code = dmbNetworkAccept();
    }

    return NULL;
}
