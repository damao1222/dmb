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

#define DMB_BINLIST_ENDCODE 0x00
#define DMB_BINCODE_TINYSTR 0x00    //长度小于等于 63 字节的字符数组
#define DMB_BINCODE_STR 0x40        //长度小于等于 16383 字节的字符数组
#define DMB_BINCODE_LARGESTR 0x80     //长度小于等于 4294967295 的字符数组
#define DMB_BINCODE_I16 0xC0
#define DMB_BINCODE_I32 0xD0
#define DMB_BINCODE_I64 0xE0

#define DMB_BINCODE(ENTRY) (0xF0 & ENTRY[0])

typedef dmbBYTE dmbBinEntry, dmbBinlist;
typedef struct dmbBinItem {
    dmbBYTE encode[5];
    union {
        dmbINT16 i16;
        dmbINT32 i32;
        dmbINT64 i64;
        void *ptr;
    } v;
} dmbBinItem;

typedef struct dmbBinAllocator {
    dmbCode (*malloc)(void *pParams, void **ptr, dmbSIZE *pLen);
    dmbCode (*realloc)(void *pParams, void **ptr, dmbSIZE *pLen);
    dmbCode (*free)(void *pParams, void *ptr);
    void* (*getParams)(struct dmbBinAllocator *ptr);
} dmbBinAllocator;

dmbBinlist* dmbBinlistCreate(dmbBinAllocator *pAllocator);
dmbUINT dmbBinlistLen(dmbBinlist *pList);
dmbCode dmbBinlistPushBack(dmbBinlist *pList, dmbBinItem *pItem);
dmbBinEntry* dmbBinlistFirst(dmbBinlist *pList);
dmbBinEntry* dmbBinlistNext(dmbBinEntry *pList);
void dmbBinlistDestroy(dmbBinlist *pList);
dmbUINT dmbBinContentLen(dmbBinEntry *pEntry);
dmbBYTE* dmbBinEntryGet(dmbBinEntry *pEntry);
dmbCode dmbBinItemStr(dmbBinItem *pItem, dmbBYTE *pData, dmbUINT uLen);

#define DMB_BINITEM_I16(ITEM_PTR, v) do { \
                    ITEM_PTR->encode[0] = DMB_BINCODE_I16; \
                    ITEM_PTR->v.i16 = v; \
                } while(0)

#define DMB_BINITEM_I32(ITEM_PTR, v) do { \
                    ITEM_PTR->encode[0] = DMB_BINCODE_I32; \
                    ITEM_PTR->v.i32 = v; \
                } while(0)

#define DMB_BINITEM_I64(ITEM_PTR, v) do { \
                    ITEM_PTR->encode[0] = DMB_BINCODE_I64; \
                    ITEM_PTR->v.i64 = v; \
                } while(0)

#define DMB_BINITEM_STR(ITEM_PTR, v, LEN) dmbBinItemStr(ITEM_PTR, v, LEN)

#endif // DMBBINLIST_H
