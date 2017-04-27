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

#include "dmbnetwork_test.h"
#include "dmbtest.h"
#include "network/dmbservercore.h"
#include "utils/dmbtime.h"

static void testServer();

void dmbnetwork_test()
{
    testServer();
}

static void testServer()
{
    dmbServerContext ctx;
    dmbEndTime time;
    dmbCode code;
    DMB_TEST_P1(code, dmbInitServerContext, &ctx);
    DMB_TEST_P1(code, dmbInitWorkThreads, &ctx);
    DMB_TEST_P1(code, dmbInitAcceptThread, &ctx);

    dmbEndTimeInit(&time, 1*1000);
    while (!dmbEndTimeIsExpired(&time))
    {

    }

    dmbStopApp();

    DMB_TEST_P1(code, dmbQuitAcceptThread, &ctx);
    DMB_TEST_P1(code, dmbQuitWorkThreads, &ctx);
    dmbPurgeServerContext(&ctx);
}
