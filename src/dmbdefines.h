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

#define DMB_ASSERT(expr) assert(expr)

typedef u_int8_t dmbUINT8;
typedef u_int16_t dmbUINT16;
typedef u_int32_t dmbUINT32;
typedef u_int64_t dmbUINT64;
typedef int8_t dmbINT8;
typedef int16_t dmbINT16;
typedef int32_t dmbINT32;
typedef int64_t dmbINT64;
typedef unsigned long dmbULONG;
typedef long dmbLONG;
typedef signed long long dmbLONGLONG;
typedef unsigned long long dmbULONGLONG;
typedef dmbINT32 dmbINT;
typedef dmbUINT32 dmbUINT;
typedef dmbUINT8 dmbBOOL;
typedef size_t dmbSIZE;
typedef char dmbCHAR;
typedef dmbUINT8 dmbBYTE;

#ifndef NULL
#define NULL 0
#endif

void dmb_noop();

#ifdef __compiler_offsetof
#define DMB_OFFSETOF(TYPE, MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define DMB_OFFSETOF(TYPE, MEMBER) ((size_t)&(((TYPE *)0)->MEMBER))
#endif //__compiler_offsetof

#define DMB_CONTAINER_OF(ptr, type, member) ({                   \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - DMB_OFFSETOF(type,member) );})

#define DMB_ENTRY(ptr, type, member) \
    DMB_CONTAINER_OF(ptr, type, member)

//ERROR CODE DEFINES
typedef dmbINT16 dmbCode;

//########1-10通用错误码#######
//一切正常
#define DMB_ERRCODE_OK DMB_OK

//########11-50网络错误码#######

//########501-600系统操作错误码####
//内存分配失败
#define DMB_ERRCODE_ALLOC_FAILED 501
//########601-700程序逻辑错误码####
#define DMB_ERRCODE_NULL_POINTER 601

//########2001-3000数据结构错误码########
//########3101-3200 binlist相关错误码#####
//存储到binlist中的字符串大小超出支持的最大值
#define DMB_ERRCODE_BINLIST_STR_OOR 3101

//########3201-3300 string相关错误码######

//########3301-3400 list相关错误码######

//########3401-3500 map相关错误码#######

//########3501-3600 zset相关错误码#######
//条表遍历退出
#define DMB_ERRCODE_SKIPLIST_SCAN_BREAK 3501
//zset成员不存在
#define DMB_ERRCODE_ZSETMEMBER_NOT_EXIST 3502

//ERROR CODE DEFINES END

#endif // DMBDEFINES_H
