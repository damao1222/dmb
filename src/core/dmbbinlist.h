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

#ifndef DMBBINLIST_H
#define DMBBINLIST_H

#include "dmbdefines.h"
#include "utils/dmbsysutil.h"

#define DMB_BINLIST_ENDCODE 0x00
#define DMB_TINYSTR_LENMAX 63
#define DMB_NORMALSTR_LENMAX 16383
#define DMB_LARGESTR_LENMAX 4294967295
#define DMB_BINCODE_TINYSTR 0x00    //长度小于等于 63 字节的字符数组
#define DMB_BINCODE_STR 0x40        //长度小于等于 16383 字节的字符数组
#define DMB_BINCODE_LARGESTR 0x80     //长度小于等于 4294967295 的字符数组
#define DMB_BINCODE_I16 0xC0
#define DMB_BINCODE_I32 0xD0
#define DMB_BINCODE_I64 0xE0

#define DMB_BINENTRY_IS_STR(ENTRY_PTR) (((ENTRY_PTR)[0] & 0xC0) != 0xC0)

#define DMB_BINCODE(ENTRY) (0xF0 & (ENTRY)[0])

typedef dmbBYTE dmbBinEntry, dmbBinlist;
typedef struct dmbBinItem {
    dmbBYTE entryhead[9];
    dmbBYTE *data;
} dmbBinItem;

typedef struct dmbBinVar {
    union {
        dmbBYTE *data;
        dmbINT16 i16;
        dmbINT32 i32;
        dmbINT64 i64;
    };
    dmbUINT len;
} dmbBinVar;

typedef struct dmbBinAllocator {
    dmbCode (*malloc)(struct dmbBinAllocator *pAllocator, dmbBinlist **pList, dmbUINT *pLen);
    dmbCode (*realloc)(struct dmbBinAllocator *pAllocator, dmbBinlist **pList, dmbUINT *pLen);
    dmbCode (*free)(struct dmbBinAllocator *pAllocator, dmbBinlist *pList);
    dmbCode (*memcpy)(struct dmbBinAllocator *pAllocator, void *pDest, void *pSrc, dmbUINT uSize);
    dmbCode (*reset)(struct dmbBinAllocator *pAllocator);
    void* (*getData)(struct dmbBinAllocator *pAllocator);
} dmbBinAllocator;

dmbBinlist* dmbBinlistCreate(dmbBinAllocator *pAllocator);
dmbCode dmbBinlistClear(dmbBinAllocator *pAllocator, dmbBinlist **pList);
dmbUINT16 dmbBinlistLen(dmbBinlist *pList);
//dmbCode dmbBinlistPushBack(dmbBinAllocator *pAllocator, dmbBinlist **pList, dmbBinItem *pItem);
dmbCode dmbBinlistPushBack(dmbBinAllocator *pAllocator, dmbBinlist **pList, dmbBinItem *pItem, dmbBOOL bPart);
dmbBinEntry* dmbBinlistFirst(dmbBinlist *pList);
dmbBinEntry* dmbBinlistLast(dmbBinlist *pList);
dmbBinEntry* dmbBinlistNext(dmbBinEntry *pEntry);
void dmbBinlistDestroy(dmbBinAllocator *pAllocator, dmbBinlist *pList);
dmbUINT dmbBinContentLen(dmbBinEntry *pEntry);
dmbBOOL dmbBinEntryIsEmpty(dmbBinEntry *pEntry);
dmbCode dmbBinEntryGet(dmbBinEntry *pEntry, dmbBinVar *var);
dmbCode dmbBinEntryMerge(dmbBinAllocator *pAllocator, dmbBinlist **pList, dmbBinEntry *pDest, dmbBinEntry *pSrc, dmbBOOL bPart);
dmbCode dmbBinListMerge(dmbBinAllocator *pAllocator, dmbBinlist **pDestList, dmbBinlist *pSrcList, dmbBOOL mergeLast);
dmbCode dmbBinItemStr(dmbBinItem *pItem, dmbBYTE *pData, dmbUINT uLen);

#define DMB_BINITEM_I16(ITEM_PTR, v) do { \
                    (ITEM_PTR)->entryhead[0] = DMB_BINCODE_I16; \
                    dmbInt16ToByte((ITEM_PTR)->entryhead+1, (v)); \
                } while(0)

#define DMB_BINITEM_I32(ITEM_PTR, v) do { \
                    (ITEM_PTR)->entryhead[0] = DMB_BINCODE_I32; \
                    dmbInt32ToByte((ITEM_PTR)->entryhead+1, (v)); \
                } while(0)

#define DMB_BINITEM_I64(ITEM_PTR, v) do { \
                    (ITEM_PTR)->entryhead[0] = DMB_BINCODE_I64; \
                    dmbInt64ToByte((ITEM_PTR)->entryhead+1, (v)); \
                } while(0)

#define DMB_BINITEM_STR(ITEM_PTR, v, LEN) dmbBinItemStr(ITEM_PTR, v, (LEN))

//malloc binlist
extern const dmbBinAllocator g_default_binlist_allcator;
#define DMB_DEFAULT_BINALLOCATOR ((dmbBinAllocator*)&g_default_binlist_allcator)
//void binlist
extern const dmbBinAllocator g_void_binlist_allcator;
#define DMB_VOID_BINALLOCATOR ((dmbBinAllocator*)&g_void_binlist_allcator)

typedef struct dmbFixmemAllocator{
    dmbBinAllocator allocator;
    struct {
        dmbBYTE *ptr;
        dmbSIZE offset;
        dmbSIZE len;
    } data;
} dmbFixmemAllocator;

dmbBinAllocator* dmbInitFixmemAllocator(dmbFixmemAllocator *pAllocator, dmbBYTE *ptr, dmbUINT size);

#endif // DMBBINLIST_H
