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
#include <unistd.h>
#include "utils/dmblog.h"

#include "tests/dmbbinlist_test.h"

int main(int argc, char** argv)
{
    dmbSystemInit();

    dmbbinlist_test();
    dmbbinlist_merge_test();

    sync();

    dmbLogSystemInfo();

    return DMB_OK;
}

inline void dmb_noop()
{}
