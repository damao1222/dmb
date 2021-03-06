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

#include "dmbdefines.h"
#include "utils/dmbsysutil.h"
#include "base/dmbsettings.h"
#include "base/dmbserver.h"
#include <unistd.h>
#include "utils/dmblog.h"

#include "tests/dmbbinlist_test.h"
#include "tests/dmbstring_test.h"
#include "tests/dmbdllist_test.h"
#include "tests/dmbutils_test.h"
#include "tests/dmbnetwork_test.h"

static volatile dmbBOOL g_app_run = TRUE;

dmbBOOL dmbIsAppQuit()
{
    return g_app_run;
}

void dmbStopApp()
{
    g_app_run = FALSE;
}

int main(int argc, char** argv)
{
    DMB_UNUSED(argc);
    DMB_UNUSED(argv);

    dmbSystemInit();

    dmbLoadSettings(NULL);
    dmbSetrLimit(g_settings.open_files);
//    dmbbinlist_test();
//    dmbbinlist_merge_test();
//    dmbstring_test();
//    dmbdllist_test();
//    dmbutils_test();
    dmbnetwork_test();

    sync();

    dmbLogSystemInfo();

    return DMB_OK;
}

inline void dmb_noop()
{}
