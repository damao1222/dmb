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

#ifndef DMBTEST_H
#define DMBTEST_H

#include "utils/dmblog.h"

#define DMB_TEST_TAG "[dmb TEST]: "

#define DMB_TEST_CODE(CODE, FUNC) do { \
        if (CODE != DMB_ERRCODE_OK) \
            DMB_LOGD("%s [%s] FAILED, Code :%d\n", DMB_TEST_TAG, #FUNC, CODE); \
        else \
            DMB_LOGD("%s [%s] SUCCESS\n", DMB_TEST_TAG, #FUNC); \
    } while (0)


#define DMB_TEST(CODE, FUNC) do { \
        CODE = FUNC; \
        if (CODE != DMB_ERRCODE_OK) \
            DMB_LOGD("%s [%s] FAILED, Code :%d\n", DMB_TEST_TAG, #FUNC, CODE); \
        else \
            DMB_LOGD("%s [%s] SUCCESS\n", DMB_TEST_TAG, #FUNC); \
    } while (0)

#define DMB_TEST_P1(CODE, FUNC, P1) do { \
        CODE = FUNC((P1)); \
        DMB_TEST_CODE(CODE, FUNC); \
    } while (0)

#define DMB_TEST_P2(CODE, FUNC, P1, P2) do { \
        CODE = FUNC((P1), (P2)); \
        DMB_TEST_CODE(CODE, FUNC); \
    } while (0)

#define DMB_TEST_P3(CODE, FUNC, P1, P2, P3) do { \
        CODE = FUNC((P1), (P2), (P3)); \
        DMB_TEST_CODE(CODE, FUNC); \
    } while (0)

#define DMB_TEST_P4(CODE, FUNC, P1, P2, P3, P4) do { \
        CODE = FUNC((P1), (P2), (P3), (P4)); \
        DMB_TEST_CODE(CODE, FUNC); \
    } while (0)

#define DMB_TEST_P5(CODE, FUNC, P1, P2, P3, P4, P5) do { \
        CODE = FUNC((P1), (P2), (P3), (P4), (P5)); \
        DMB_TEST_CODE(CODE, FUNC); \
    } while (0)


#endif // DMBTEST_H
