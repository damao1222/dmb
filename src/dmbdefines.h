/*
    Copyright (C) 2012-2014 Xiongfa Li, <damao1222@live.com>
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

#ifndef DMBDEFINES_H
#define DMBDEFINES_H

#include <assert.h>
#include <sys/types.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef DMB_OK
#define DMB_OK          0
#endif

#ifndef DMB_ERROR
#define DMB_ERROR      -1
#endif

typedef u_int8_t DMBUINT8;
typedef u_int16_t DMBUINT16;
typedef u_int32_t DMBUINT32;
typedef u_int64_t DMBUINT64;
typedef int8_t DMBINT8;
typedef int16_t DMBINT16;
typedef int32_t DMBINT32;
typedef int64_t DMBINT64;
typedef unsigned long DMBULONG;
typedef long DMBLONG;
typedef signed long long DMBLONGLONG;
typedef unsigned long long DMBULONGLONG;
typedef DMBINT32 DMBINT;
typedef DMBUINT32 DMBUINT;
typedef DMBUINT8 DMBBOOL;
typedef size_t DMBSIZE;
typedef char DMBCHAR;

void dmb_noop();

//ERROR CODE DEFINES


//ERROR CODE DEFINES END

#endif // DMBDEFINES_H
