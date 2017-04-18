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

#include "dmbstring_test.h"
#include "core/dmbstring.h"
#include "utils/dmblog.h"
#include <string.h>

void dmbstring_test()
{
    dmbString *pStr = dmbStringCreate(9);

    dmbCHAR *testStr = "123";
    dmbStringAppend(pStr, testStr, strlen(testStr));
    DMB_LOGD("str is %s\n", dmbStringGet(pStr));

    testStr = "abc";
    dmbStringPrepend(pStr, testStr, strlen(testStr));
    DMB_LOGD("str is %s\n", dmbStringGet(pStr));

    testStr = "456";
    dmbStringPrepend(pStr, testStr, strlen(testStr));
    DMB_LOGD("str is %s\n", dmbStringGet(pStr));

    testStr = "123";
    dmbStringPrepend(pStr, testStr, strlen(testStr));
    DMB_LOGD("str is %s\n", dmbStringGet(pStr));

    dmbINT index= dmbStringIndexOf(pStr, "6ab", strlen("6ab"));
    DMB_LOGD("Index of \"6ab\" is %d\n", index);

    index= dmbStringLastIndexOf(pStr, "12", strlen("12"));
    DMB_LOGD("Last Index of \"12\" is %d\n", index);

    dmbStringDestroy(pStr);

    dmbLONG lValue;
    dmbString2Long("-8317687561", &lValue);
    DMB_LOGD("dmbString2Long %ld\n", lValue);

    dmbCHAR strValue[256]= {0};
    dmbUINT uStrLen = 256;
    dmbLong2Str(lValue, strValue, &uStrLen);
    DMB_LOGD("dmbLong2Str %s %d\n", strValue, uStrLen);

    pStr = dmbStringCreateWithBuffer(strValue, uStrLen);
    DMB_LOGD("dmbStringCreateWithBuffer str is %s\n", dmbStringGet(pStr));
    dmbStringDestroy(pStr);

    pStr = dmbStringCreateWithFormat(256, "this is a test %x %s\n", 0xabcd1234, "yes!");
    DMB_LOGD("dmbStringCreateWithFormat str is %s\n", dmbStringGet(pStr));

    dmbStringDestroy(pStr);
}

