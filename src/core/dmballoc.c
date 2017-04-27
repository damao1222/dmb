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

#include "dmballoc.h"
#include "utils/dmblog.h"
#include "thread/dmbatomic.h"
#include <string.h>

#define __xstr(s) __str(s)
#define __str(s) #s

#if defined(DMB_USE_TCMALLOC)
#define DMB_MALLOC_LIB ("tcmalloc-" __xstr(TC_VERSION_MAJOR) "." __xstr(TC_VERSION_MINOR))
#include <google/tcmalloc.h>
#define malloc(size) tc_malloc(size)
#define calloc(count,size) tc_calloc(count,size)
#define realloc(ptr,size) tc_realloc(ptr,size)
#define free(ptr) tc_free(ptr)
#if (TC_VERSION_MAJOR == 1 && TC_VERSION_MINOR >= 6) || (TC_VERSION_MAJOR > 1)
#define DMB_MALLOC_SIZE(p) tc_malloc_size(p)
#else
#error "Newer version of tcmalloc required"
#endif

#elif defined(CB_USE_JEMALLOC)
#define DMB_MALLOC_LIB ("jemalloc-" __xstr(JEMALLOC_VERSION_MAJOR) "." __xstr(JEMALLOC_VERSION_MINOR) "." __xstr(JEMALLOC_VERSION_BUGFIX))
#include "../3rdparty/jemalloc/jemalloc.h"
#define malloc(size) je_malloc(size)
#define calloc(count,size) je_calloc(count,size)
#define realloc(ptr,size) je_realloc(ptr,size)
#define free(ptr) je_free(ptr)
#if (JEMALLOC_VERSION_MAJOR == 2 && JEMALLOC_VERSION_MINOR >= 1) || (JEMALLOC_VERSION_MAJOR > 2)
#define DMB_MALLOC_SIZE(p) je_malloc_usable_size(p)
#else
#error "Newer version of jemalloc required"
#endif

#elif defined(__APPLE__)
#include <malloc/malloc.h>
#define DMB_MALLOC_SIZE(p) malloc_size(p)
#endif

#ifndef DMB_MALLOC_LIB
#define DMB_MALLOC_LIB "libc"
#endif

static volatile size_t g_used_memory = 0;
static size_t g_max_memory = 1024*1024*512;

#ifdef DMB_MALLOC_SIZE
static const char PREFIX_SIZE = 0;
#else
static const char PREFIX_SIZE = sizeof(size_t);
#endif

static inline void UpdateAlignmentSize(size_t size, dmbBOOL increase)
{
    if (size&(sizeof(long)-1))
        size += sizeof(long)-(size&(sizeof(long)-1));

    if (increase)
    	InterlockedAddU64(&g_used_memory, size);
    else
    	InterlockedAddU64(&g_used_memory, -size);
}

static void DefaultOOMHandle(size_t size)
{
    DMB_LOGE("Out of Memory tring alloc %zu bytes\n", size);
//    abort();
}

static inline __attribute__((always_inline)) dmbBOOL CheckCurrentMemoryUsage()
{
    if (InterlockedAddU64(&g_used_memory,0) >= g_max_memory)
    {
        DMB_LOGW("The current memory usage is bigger than the maxmemory value set via CONFIG SET.\n");
        return FALSE;
    }
    return TRUE;
}

static void (*oom_handle) (size_t) = DefaultOOMHandle;

void *dmbMalloc(size_t size)
{
    if (CheckCurrentMemoryUsage() == FALSE)
        return NULL;

    void *pRealPtr = malloc(size + PREFIX_SIZE);

    if (!pRealPtr)
    {
        oom_handle(size);
        return NULL;
    }

#ifdef DMB_MALLOC_SIZE
   UpdateAlignmentSize(DMB_MALLOC_SIZE(pRealPtr), TRUE);
#else
    *((size_t*)pRealPtr) = size;
    UpdateAlignmentSize(size, TRUE);
#endif
    return (char*)pRealPtr + PREFIX_SIZE;
}

void dmbFree(void *p)
{
    if (p == NULL)
        return ;

#ifdef DMB_MALLOC_SIZE
    UpdateAlignmentSize(DMB_MALLOC_SIZE(p), FALSE);
    free(p);
#else
    void *pRealPtr = (char*)p - PREFIX_SIZE;
    size_t useSize = *((size_t*)pRealPtr);
    UpdateAlignmentSize(useSize, FALSE);
    free(pRealPtr);
#endif
}

void *dmbRealloc(void *p, size_t size)
{
    if (p == NULL)
        return dmbMalloc(size);

    if (CheckCurrentMemoryUsage() == FALSE)
        return NULL;

#ifdef DMB_MALLOC_SIZE
    size_t oldSize = DMB_MALLOC_SIZE(p);
    void *pNewPtr = realloc(p, size);
    if (pNewPtr == NULL)
    {
        oom_handle(size);
        return NULL;
    }

    UpdateAlignmentSize(oldSize, FALSE);
    UpdateAlignmentSize(DMB_MALLOC_SIZE(pNewPtr), TRUE);
#else
    void *pRealPtr = (char*)p - PREFIX_SIZE;
    size_t oldSize = *((size_t*)pRealPtr);
    void *pNewPtr = realloc(pRealPtr, size + PREFIX_SIZE);
    if (pNewPtr == NULL)
    {
        oom_handle(size);
        return NULL;
    }

    *((size_t*)pNewPtr) = size;
    UpdateAlignmentSize(oldSize, FALSE);
    UpdateAlignmentSize(size, TRUE);
#endif
    return (char*)pNewPtr + PREFIX_SIZE;
}

void *dmbMallocAligned(size_t size, size_t alignment)
{
    return dmbReallocAligned(0, size, alignment);
}

void *dmbReallocAligned(void *oldptr, size_t newsize, size_t alignment)
{
    void *pReal = oldptr ? ((void**)oldptr)[-1] : NULL;
    if (alignment <= sizeof(void*))
    {
        void **newptr = (void **)(dmbRealloc(pReal, newsize + sizeof(void*)));
        if (!newptr)
            return NULL;
        if (newptr == pReal)
            return oldptr;

        *newptr = newptr;
        return newptr + 1;
    }

    void *newptr = dmbRealloc(pReal, newsize + alignment);
    if (!newptr)
        return NULL;

    dmbULONG faked = ((dmbULONG)newptr) + alignment;
    faked &= ~(alignment - 1);

    void **faked_ptr = (void **)(faked);
    faked_ptr[-1] = newptr;
    return faked_ptr;
}

void dmbFreeAligned(void *p)
{
    if (!p)
        return ;
    void **ptr = (void **)(p);
    dmbFree(ptr[-1]);
}

void *dmbMemCopy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

void *dmbMemSet(void *dest, int c, size_t n)
{
    return memset(dest, c, n);
}

void *dmbMemMove(void *dest, const void *src, size_t n)
{
    return memmove(dest, src, n);
}

dmbINT dmbMemCmp(const void *dest, const void *src, size_t n)
{
    return memcmp(dest, src, n);
}

size_t dmbAllocSize(void *p)
{
#ifdef DMB_MALLOC_SIZE
    return DMB_MALLOC_SIZE(p);
#else
    void *pRealPtr = (char*)p - PREFIX_SIZE;
    size_t size = *((size_t*)pRealPtr);

    if (size&(sizeof(long)-1))
        size += sizeof(long)-(size&(sizeof(long)-1));

    return size + PREFIX_SIZE;
#endif
}

size_t dmbGetUsedMemSize()
{
    return InterlockedAddU64(&g_used_memory,0);
}

void dmbSetMaxMemSize(size_t size)
{
    g_max_memory = size;
}
