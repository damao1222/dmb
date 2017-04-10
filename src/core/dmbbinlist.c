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

/*
 ********************************************************************************************
    binlist:
     ------------------  -----------------------  ----------------  -------  -------     -------  -------------------
    |total length 4byte||last entry offset 4byte||entry size 2byte||entry 1||entry 2|...|entry n||end code 0x00 1byte|
     ------------------  -----------------------  ----------------  -------  -------     -------  -------------------

    tiny string entry:
     --------   ----------
    |00bbbbbb| |content<63|
     --------   ----------
    normal string entry:
     -------- --------   -------------
    |01bbbbbb bbbbbbbb| |content<16383|
     -------- --------   -------------
    large string entry:
     -------- -------- -------- -------- --------   ------------------
    |10?????? bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb| |content<4294967295|
     -------- -------- -------- -------- --------   ------------------
    int16 entry:
     --------   -------------
    |11000000| |content 2byte|
     --------   -------------
    int32 entry:
     --------   -------------
    |11010000| |content 4byte|
     --------   -------------
    int64 entry:
     --------   -------------
    |11100000| |content 8byte|
     --------   -------------
 ********************************************************************************************
**/

#include "dmbbinlist.h"
#include "core/dmballoc.h"
#include <limits.h>

#define BINLIST_SIZE(LIST_PTR) (*(dmbUINT*)&(LIST_PTR)[0])
#define BINLIST_LAST(LIST_PTR) (*(dmbUINT*)&(LIST_PTR)[sizeof(dmbUINT)])
#define BINLIST_LEN(LIST_PTR) (*(dmbUINT16*)&(LIST_PTR)[(sizeof(dmbUINT)*2)])
#define BINLIST_UPDATE_SIZE(LIST_PTR, SIZE) dmbInt32ToByte((LIST_PTR), (SIZE))
#define BINLIST_UPDATE_LAST(LIST_PTR, OFFSET) dmbInt32ToByte((LIST_PTR)+sizeof(dmbUINT), (OFFSET))
#define BINLIST_UPDATE_LEN(LIST_PTR, NUM) dmbInt16ToByte((LIST_PTR)+sizeof(dmbUINT)+sizeof(dmbUINT), (NUM))
#define BINLIST_UPDATE_ENDCODE(LIST_PTR) do { \
                        (LIST_PTR)[BINLIST_SIZE(LIST_PTR)-DMB_BINLIST_TAIL_SIZE] = DMB_BINLIST_ENDCODE; \
                    } while (0)

const dmbUINT DMB_BINLIST_HEAD_SIZE = sizeof(dmbUINT)*2 + sizeof(dmbUINT16);
const dmbUINT DMB_BINLIST_TAIL_SIZE = sizeof(dmbBYTE);

static inline void dmbBinEntryLen(dmbBinEntry *pEntry, dmbUINT *pLen, dmbUINT *pAllLen);
static inline void setStrLen(dmbBinEntry *pEntry, dmbUINT uLen);

dmbBinlist* dmbBinlistCreate(dmbBinAllocator *pAllocator)
{
    dmbUINT uSize = DMB_BINLIST_HEAD_SIZE + DMB_BINLIST_TAIL_SIZE;
    dmbBinlist *pList = NULL;
    if (DMB_ERRCODE_OK != pAllocator->malloc(pAllocator, (void**)&pList, &uSize))
        return NULL;

    BINLIST_UPDATE_SIZE(pList, uSize);
    BINLIST_UPDATE_LAST(pList, DMB_BINLIST_HEAD_SIZE);
    BINLIST_UPDATE_LEN(pList, 0);
    BINLIST_UPDATE_ENDCODE(pList);

    return pList;
}

dmbCode dmbBinlistClear(dmbBinAllocator *pAllocator, dmbBinlist *pList)
{
    dmbUINT uSize = DMB_BINLIST_HEAD_SIZE + DMB_BINLIST_TAIL_SIZE;
    if (DMB_ERRCODE_OK != pAllocator->realloc(pAllocator, (void**)&pList, BINLIST_SIZE(pList), &uSize))
        return DMB_ERRCODE_ALLOC_FAILED;

    BINLIST_UPDATE_SIZE(pList, uSize);
    BINLIST_UPDATE_LAST(pList, DMB_BINLIST_HEAD_SIZE);
    BINLIST_UPDATE_LEN(pList, 0);
    BINLIST_UPDATE_ENDCODE(pList);

    return DMB_ERRCODE_OK;
}

inline dmbUINT16 dmbBinlistLen(dmbBinlist *pList)
{
    return BINLIST_LEN(pList);
}

dmbCode dmbBinlistPushBack(dmbBinAllocator *pAllocator, dmbBinlist *pList, dmbBinItem *pItem)
{
    dmbUINT uLen, uAllLen, uCurrent, uLenNew, uAllLenNew, uAllocLen;
    if (dmbBinlistLen(pList) == USHRT_MAX)
        return DMB_ERRCODE_BINLIST_ENTRY_OOR;

    dmbBinEntryLen(pItem->entryhead, &uLenNew, &uAllLenNew);

    uCurrent = BINLIST_SIZE(pList);
    if (uCurrent + uAllLenNew > UINT_MAX)
        return DMB_ERRCODE_BINLIST_FULL;

    uAllocLen = uCurrent + uAllLenNew;
    dmbCode code = pAllocator->realloc(pAllocator, (void**)&pList, uCurrent, &uAllocLen);
    if (code != DMB_ERRCODE_OK)
        return code;

    dmbBinEntry *entry = dmbBinlistLast(pList);
    dmbBinEntryLen(entry, &uLen, &uAllLen);
    if (DMB_BINENTRY_IS_STR(pItem->entryhead))
    {
        pAllocator->memcpy(pAllocator, entry + uAllLen, pItem->entryhead, uAllLenNew - uLenNew);
        pAllocator->memcpy(pAllocator, entry + uAllLen + (uAllLenNew - uLenNew), pItem->data, uLenNew);
    }
    else
    {
        pAllocator->memcpy(pAllocator, entry + uAllLen, pItem->entryhead, uAllLenNew);
    }
    BINLIST_UPDATE_SIZE(pList, uAllocLen);
    BINLIST_UPDATE_LAST(pList, BINLIST_LAST(pList) + uAllLen);
    BINLIST_UPDATE_LEN(pList, BINLIST_LEN(pList) + 1);
    BINLIST_UPDATE_ENDCODE(pList);

    return DMB_ERRCODE_OK;
}

dmbBinEntry* dmbBinlistFirst(dmbBinlist *pList)
{
    return pList + DMB_BINLIST_HEAD_SIZE;
}

dmbBinEntry* dmbBinlistLast(dmbBinlist *pList)
{
    return pList + BINLIST_LAST(pList);
}

dmbBinEntry* dmbBinlistNext(dmbBinEntry *pEntry)
{
    dmbUINT uLen, uAllLen;
    dmbBinEntryLen(pEntry, &uLen, &uAllLen);
    pEntry += uAllLen;
    if (pEntry[0] == DMB_BINLIST_ENDCODE)
        return NULL;

    return pEntry;
}

void dmbBinlistDestroy(dmbBinAllocator *pAllocator, dmbBinlist *pList)
{
    pAllocator->free(pAllocator, pList);
}

static inline void dmbBinEntryLen(dmbBinEntry *pEntry, dmbUINT *pLen, dmbUINT *pAllLen)
{
    if (pEntry[0] == DMB_BINLIST_ENDCODE)
    {
        *pLen = 0;
        *pAllLen = 0;
        return ;
    }

    dmbBYTE encode = DMB_BINCODE(pEntry);
    switch (encode) {
    case DMB_BINCODE_I16:
    {
        *pLen = sizeof(dmbINT16);
        *pAllLen = *pLen + 1;
        break;
    }
    case DMB_BINCODE_I32:
    {
        *pLen = sizeof(dmbINT32);
        *pAllLen = *pLen + 1;
        break;
    }
    case DMB_BINCODE_I64:
    {
        *pLen = sizeof(dmbINT64);
        *pAllLen = *pLen + 1;
        break;
    }
    case DMB_BINCODE_TINYSTR:
    {
        *pLen = (*(dmbBYTE*)&pEntry[0]) & 0x3F;
        *pAllLen = *pLen + 1;
        break;
    }
    case DMB_BINCODE_STR:
    {
        *pLen = ((pEntry[0] & 0x3F) << 8) | pEntry[1];
        *pAllLen = *pLen + 2;
        break;
    }
    case DMB_BINCODE_LARGESTR:
    {
        *pLen = (pEntry[1] << 24) | (pEntry[2] << 16) | (pEntry[3] << 8) | pEntry[4];
        *pAllLen = *pLen + 5;
        break;
    }
    default:
        *pLen = 0;
        *pAllLen = 0;
    }
}

inline dmbUINT dmbBinContentLen(dmbBinEntry *pEntry)
{
    dmbUINT uLen, uAllLen;
    dmbBinEntryLen(pEntry, &uLen, &uAllLen);
    return uLen;
}

dmbCode dmbBinEntryGet(dmbBinEntry *pEntry, dmbBinVar *var)
{
    dmbUINT uLen, uAllLen;
    dmbBinEntryLen(pEntry, &uLen, &uAllLen);
    if (DMB_BINENTRY_IS_STR(pEntry))
    {
        var->v.data = pEntry + (uAllLen - uLen);
    }
    else
    {
        dmbINT64 i64 = 0;
        dmbMemCopy(&i64, pEntry + (uAllLen - uLen), uLen);
        dmbInt64ToByte((dmbBYTE*)&var->v.i64, i64);
    }
    var->len = uLen;

    return DMB_ERRCODE_OK;
}

dmbCode dmbBinEntryMerge(dmbBinAllocator *pAllocator, dmbBinlist *pList, dmbBinEntry *pDest, dmbBinEntry *pSrc, dmbBOOL bPart)
{
    dmbUINT uCurrent, uSrcLen, uSrcAllLen, uAllocLen, uDestLen, uDestAllLen;

    if (!DMB_BINENTRY_IS_STR(pSrc))
        return DMB_ERRCODE_BINLIST_MERGE_ERROR_TYPE;

    uCurrent = BINLIST_SIZE(pList);
    dmbBinEntryLen(pSrc, &uSrcLen, &uSrcAllLen);
    dmbBinEntryLen(pDest, &uDestLen, &uDestAllLen);

    if (uCurrent + uSrcLen > UINT_MAX)
        return DMB_ERRCODE_BINLIST_FULL;

    uAllocLen = uCurrent + uSrcLen;
    dmbCode code = pAllocator->realloc(pAllocator, (void**)&pList, uCurrent, &uAllocLen);
    if (code == DMB_ERRCODE_BINLIST_ALLOC_FAILED)
        return code;
    else if (code == DMB_ERRCODE_BINLIST_ALLOC_PART && !bPart)
        return code;

    if (uAllocLen == uCurrent)
        return DMB_ERRCODE_BINLIST_NO_ENOUGH_SPACE;

    setStrLen(pDest, uAllocLen);
    pAllocator->memcpy(pAllocator, pDest + uDestAllLen, pSrc + (uSrcAllLen - uSrcLen), uAllocLen - uCurrent);

    BINLIST_UPDATE_SIZE(pList, uAllocLen);
    if (dmbBinlistLast(pList) != pDest)
        BINLIST_UPDATE_LAST(pList, BINLIST_LAST(pList) + (uAllocLen - uCurrent));
//    BINLIST_UPDATE_LEN(pList, BINLIST_LEN(pList) + 1);
    BINLIST_UPDATE_ENDCODE(pList);

    return code;
}

static inline void setStrLen(dmbBinEntry *pEntry, dmbUINT uLen)
{
    dmbBYTE encode = DMB_BINCODE(pEntry);
    switch (encode) {
    case DMB_BINCODE_TINYSTR:
    {
        pEntry[0] = DMB_BINCODE_TINYSTR | uLen;
        break;
    }
    case DMB_BINCODE_STR:
    {
        pEntry[0] = DMB_BINCODE_STR | ((uLen >> 8) & 0x3F);
        pEntry[1] = uLen & 0xFF;
        break;
    }
    case DMB_BINCODE_LARGESTR:
    {
        pEntry[0] = DMB_BINCODE_LARGESTR;
        pEntry[1] = (uLen >> 24) & 0xFF;
        pEntry[2] = (uLen >> 16) & 0xFF;
        pEntry[3] = (uLen >> 8) & 0xFF;
        pEntry[4] = uLen & 0xFF;
        break;
    }
    default:
        break;
    }
}

inline dmbCode dmbBinItemStr(dmbBinItem *pItem, dmbBYTE *pData, dmbUINT uLen)
{
    pItem->offset = 0;
    if (uLen < 63)
    {
        pItem->entryhead[0] = DMB_BINCODE_TINYSTR | uLen;
    }
    else if (uLen < 16383)
    {
        pItem->entryhead[0] = DMB_BINCODE_STR | ((uLen >> 8) & 0x3F);
        pItem->entryhead[1] = uLen & 0xFF;
    }
    else if (uLen < 4294967295)
    {
        pItem->entryhead[0] = DMB_BINCODE_LARGESTR;
        pItem->entryhead[1] = (uLen >> 24) & 0xFF;
        pItem->entryhead[2] = (uLen >> 16) & 0xFF;
        pItem->entryhead[3] = (uLen >> 8) & 0xFF;
        pItem->entryhead[4] = uLen & 0xFF;
    }
    else
    {
        return DMB_ERRCODE_BINLIST_STR_OOR;
    }
    pItem->data = pData;

    return DMB_ERRCODE_OK;
}

dmbCode default_malloc(dmbBinAllocator *pAllocator, void **ptr, dmbUINT *pLen)
{
    DMB_UNUSED(pAllocator);
    *ptr = dmbMalloc((dmbSIZE)*pLen);
    if (*ptr == NULL)
    {
        *pLen = 0;
        return DMB_ERRCODE_BINLIST_ALLOC_FAILED;
    }

    return DMB_ERRCODE_OK;
}

dmbCode default_realloc(dmbBinAllocator *pAllocator, void **ptr, dmbUINT oldSize, dmbUINT *pLen)
{
    DMB_UNUSED(pAllocator);
    DMB_UNUSED(oldSize);
    *ptr = dmbRealloc(*ptr, (dmbSIZE)*pLen);
    if (*ptr == NULL)
    {
        *pLen = 0;
        return DMB_ERRCODE_BINLIST_ALLOC_FAILED;
    }

    return DMB_ERRCODE_OK;
}

dmbCode default_free(dmbBinAllocator *pAllocator, void *ptr)
{
    DMB_UNUSED(pAllocator);
    dmbFree(ptr);

    return DMB_ERRCODE_OK;
}

dmbCode default_memcpy(struct dmbBinAllocator *pAllocator, void *pDest, void *pSrc, dmbUINT uSize)
{
    DMB_UNUSED(pAllocator);
    dmbMemCopy(pDest, pSrc, uSize);

    return DMB_ERRCODE_OK;
}

dmbCode default_reset(dmbBinAllocator *pAllocator)
{
    DMB_UNUSED(pAllocator);
    return DMB_ERRCODE_OK;
}

void* default_getMember(dmbBinAllocator *pAllocator)
{
    DMB_UNUSED(pAllocator);
    return NULL;
}

const dmbBinAllocator DMB_DEFAULT_BINALLOCATOR = {
    default_malloc,
    default_realloc,
    default_free,
    default_memcpy,
    default_reset,
    default_getMember
};

dmbCode void_realloc(dmbBinAllocator *pAllocator, void **ptr, dmbUINT oldSize, dmbUINT *pLen)
{
    DMB_UNUSED(pAllocator);
    DMB_UNUSED(ptr);
    DMB_UNUSED(oldSize);
    DMB_UNUSED(pLen);

    return DMB_ERRCODE_OK;
}

dmbCode void_memcpy(struct dmbBinAllocator *pAllocator, void *pDest, void *pSrc, dmbUINT uSize)
{
    DMB_UNUSED(pAllocator);
    DMB_UNUSED(pDest);
    DMB_UNUSED(pSrc);
    DMB_UNUSED(uSize);

    return DMB_ERRCODE_OK;
}

const dmbBinAllocator DMB_VOID_BINALLOCATOR = {
    default_malloc,
    void_realloc,
    default_free,
    void_memcpy,
    default_reset,
    default_getMember
};

dmbCode fixmem_malloc(dmbBinAllocator *pAllocator, void **ptr, dmbUINT *pLen)
{
    dmbFixmemAllocator* fixmem = (dmbFixmemAllocator*)pAllocator->getData(pAllocator);
    dmbUINT left = fixmem->data.len - fixmem->data.offset;
    dmbCode code = DMB_ERRCODE_OK;
    if (*pLen > left)
    {
        *pLen = left;
        code = DMB_ERRCODE_BINLIST_ALLOC_PART;
    }

    *ptr = fixmem->data.ptr + fixmem->data.offset;
    fixmem->data.offset += *pLen;

    return code;
}

dmbCode fixmem_realloc(dmbBinAllocator *pAllocator, void **ptr, dmbUINT oldSize, dmbUINT *pLen)
{
    DMB_UNUSED(ptr);
    dmbFixmemAllocator* fixmem = (dmbFixmemAllocator*)pAllocator->getData(pAllocator);
    dmbUINT left = fixmem->data.len - fixmem->data.offset;
    dmbUINT need = *pLen - oldSize;
    dmbCode code = DMB_ERRCODE_OK;
    if (need > left)
    {
        need = left;
        code = DMB_ERRCODE_BINLIST_ALLOC_PART;
    }
    *pLen = oldSize + need;
    fixmem->data.offset += need;

    return code;
}

dmbCode fixmem_free(dmbBinAllocator *pAllocator, void *ptr)
{
    DMB_UNUSED(ptr);
    pAllocator->reset(pAllocator);

    return DMB_ERRCODE_OK;
}

dmbCode fixmem_reset(dmbBinAllocator *pAllocator)
{
    dmbFixmemAllocator* fixmem = (dmbFixmemAllocator*)pAllocator->getData(pAllocator);
    fixmem->data.offset = 0;
    return DMB_ERRCODE_OK;
}

void* fixmem_getData(dmbBinAllocator *pAllocator)
{
    dmbFixmemAllocator* fixmem = DMB_ENTRY(pAllocator, dmbFixmemAllocator, allocator);
    return fixmem;
}

dmbBinAllocator* dmbInitFixmemAllocator(dmbFixmemAllocator *pAllocator, dmbBYTE *ptr, dmbUINT size)
{
    pAllocator->allocator.malloc = fixmem_malloc;
    pAllocator->allocator.realloc = fixmem_realloc;
    pAllocator->allocator.free = fixmem_free;
    pAllocator->allocator.memcpy = default_memcpy;
    pAllocator->allocator.reset = fixmem_reset;
    pAllocator->allocator.getData = fixmem_getData;

    pAllocator->data.ptr = ptr;
    pAllocator->data.offset = 0;
    pAllocator->data.len = size;

    return &pAllocator->allocator;
}
