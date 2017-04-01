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

#include "dmbbinlist.h"
#include "core/dmballoc.h"
#include "utils/dmbsysutil.h"

#define BINLIST_LEN(LIST_PTR) (*(dmbUINT*)&LIST_PTR[4])

const dmbUINT DMB_BINLIST_HEAD_SIZE = sizeof(dmbUINT)*2;
const dmbUINT DMB_BINLIST_TAIL_SIZE = sizeof(dmbBYTE);

static inline void dmbBinEntryLen(dmbBinEntry *pEntry, dmbUINT *pLen, dmbUINT *pAllLen);

dmbBinlist* dmbBinlistCreate(dmbBinAllocator *pAllocator)
{
    dmbUINT uSize = DMB_BINLIST_HEAD_SIZE + DMB_BINLIST_TAIL_SIZE;
    dmbBinlist *pList = dmbMalloc(uSize);
    if (NULL == pList)
        return NULL;

    dmbInt32ToByte(pList, uSize);
    dmbInt32ToByte(pList+4, 0);
    pList[uSize-DMB_BINLIST_TAIL_SIZE] = DMB_BINLIST_ENDCODE;

    return pList;
}

inline dmbUINT dmbBinlistLen(dmbBinlist *pList)
{
    return BINLIST_LEN(pList);
}

dmbCode dmbBinlistPushBack(dmbBinlist *pList, dmbBinItem *pItem)
{

}

dmbBinEntry* dmbBinlistFirst(dmbBinlist *pList)
{
    return pList + DMB_BINLIST_HEAD_SIZE;
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

void dmbBinlistDestroy(dmbBinlist *pList)
{
    dmbFree(pList);
}

static inline void dmbBinEntryLen(dmbBinEntry *pEntry, dmbUINT *pLen, dmbUINT *pAllLen)
{
    dmbBYTE encode = DMB_BINCODE(pEntry);
    switch (encode) {
    case DMB_BINCODE_I16:
        *pLen = sizeof(dmbINT16);
        *pAllLen = *pLen + 1;
        break;
    case DMB_BINCODE_I32:
        *pLen = sizeof(dmbINT32);
        *pAllLen = *pLen + 1;
        break;
    case DMB_BINCODE_I64:
        *pLen = sizeof(dmbINT64);
        *pAllLen = *pLen + 1;
        break;
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

dmbBYTE* dmbBinEntryGet(dmbBinEntry *pEntry)
{
    return pEntry + dmbBinContentLen(pEntry);
}

inline dmbCode dmbBinItemStr(dmbBinItem *pItem, dmbBYTE *pData, dmbUINT uLen)
{
    if (uLen < 63)
    {
        pItem->encode[0] = DMB_BINCODE_TINYSTR | uLen;
    }
    else if (uLen < 16383)
    {
        pItem->encode[0] = DMB_BINCODE_STR | ((uLen >> 8) & 0x3F);
        pItem->encode[1] = uLen & 0xFF;
    }
    else if (uLen < 4294967295)
    {
        pItem->encode[0] = DMB_BINCODE_LARGESTR;
        pItem->encode[1] = (uLen >> 24) & 0xFF;
        pItem->encode[2] = (uLen >> 16) & 0xFF;
        pItem->encode[3] = (uLen >> 8) & 0xFF;
        pItem->encode[4] = uLen & 0xFF;
    }
    else
    {
        return DMB_ERRCODE_BINLIST_STR_OOR;
    }
    pItem->v.ptr = pData;

    return DMB_ERRCODE_OK;
}


/*
 ********************************************************************************************
    binlist:
     ------------------  ----------------  -------  -------     -------  -------------------
    |total length 4byte||entry size 4byte||entry 1||entry 2|...|entry n||end code 0x00 1byte|
     ------------------  ----------------  -------  -------     -------  -------------------

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
