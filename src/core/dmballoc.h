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

#ifndef DMBALLOC_H
#define DMBALLOC_H

#include <stdlib.h>
#include "dmbdefines.h"

/**
 * @brief 分配内存
 * @param size 内存大小
 * @return 分配后的内存指针，失败返回NULL
 */
void *dmbMalloc(size_t nSize);

/**
 * @brief 释放内存
 * @param ptr 需要释放内存的指针
 */
void dmbFree(void *p);

/**
 * @brief 重新分配size大小的内存
 * @param ptr  原内存块的指针
 * @param size  新内存块的大小
 * @return 新分配内存块的指针
 */
void *dmbRealloc(void *p, size_t nSize);

/**
 * @brief 分配一块指定对齐的内存
 *
 * @param size 内存大小
 * @param alignment 对齐值，必须是 2 的整数次幂
 */
void *dmbMallocAligned(size_t size, size_t alignment);

/**
 * @brief 重新分配一块指定对齐的内存
 *
 * @param oldptr 原内存块的指针
 * @param newsize 新内存块的大小
 * @param alignment 对齐值，必须是 2 的整数次幂
 */
void *dmbReallocAligned(void *oldptr, size_t newsize, size_t alignment);

/**
 * @brief 释放对齐的内存，必须是CBMallocAligned或者CBReallocAligned申请的内存
 *
 * @param p 对齐内存地址
 */
void dmbFreeAligned(void *p);

/**
 * @brief 从原地址拷贝指定大小的内存数据到目标地址.
 * @param dest  目标内存地址.
 * @param src  源内存地址.
 * @param n  拷贝内存大小.
 * @return 目标地址.
 */
void *dmbMemCopy(void *dest, const void *src, size_t n);

/**
 * @brief 将目标地址指定大小的内存设置为指定的值
 * @param dest  目标内存地址
 * @param v  需要设置的值
 * @param n  设置内存大小
 * @return 目标地址。
 */
void *dmbMemSet(void *dest, int v, size_t n);

/**
 * @brief 将原地址移动指定大小的内存数据到目标地址.
 * @param dest  目标内存地址.
 * @param src  源内存地址.
 * @param n  移动内存大小.
 * @return 目标地址.
 */
void *dmbMemMove(void *dest, const void *src, size_t n);

/**
 * @brief dmbMemCmp 比较内存
 * @param dest 目标地址
 * @param src 源地址
 * @param n 比较长度
 * @return 返回大于、等于、小于0 （0表示完全相等）
 */
dmbINT dmbMemCmp(const void *dest, const void *src, size_t n);

/**
 * @brief 获得已分配内存的大小
 *
 * @param p 堆指针
 * @return size_t 已分配内存大小
 */
size_t dmbAllocSize(void *p);

/**
 * @brief 获得已使用的内存总量
 *
 * @return size_t 内存总量
 */
size_t dmbGetUsedMemSize();

#define DMB_SAFE_FREE(PTR) do { if (PTR != NULL) { dmbFree(PTR); PTR = NULL; } } while (0)

#define DMB_CHECK_PTR(p) DMB_ASSERT(p)

#endif // DMBALLOC_H
