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

#include "dmbdict.h"
#include "dmballoc.h"

dmbDict* dmbDictCreate(dmbDictMeta *pMeta, dmbUINT uSize)
{
    dmbDict* dict = (dmbDict*)dmbMalloc(sizeof(dmbDict) + (sizeof(dmbDictEntry*)*uSize));
    if (dict != NULL)
    {
        dmbMemSet(dict, 0, sizeof(dmbDict) + (sizeof(dmbDictEntry*)*uSize));
        dict->meta = pMeta;
        dict->size = uSize;
        dict->count = 0;
    }
    return dict;
}

void dmbDictDestroy(dmbDict *pDict)
{
    dmbFree(pDict);
}

void dmbDictRemoveAll(dmbDict *pDict)
{
    dmbMemSet(((dmbBYTE*)pDict) + sizeof(dmbDict), 0, (sizeof(dmbDictEntry*)*pDict->size));
    pDict->count = 0;
}

dmbDictEntry* dmbDictGet(dmbDict *pDict, const void *pKey)
{
    dmbUINT index = pDict->meta->hashFunc(pKey) % pDict->size;
    dmbDictEntry *pEntry = pDict->entry[index];

    if (pEntry == NULL)
        return NULL;

    do
    {
        if (pDict->meta->keyCompare(pKey, pEntry->k.val) == 0)
            return pEntry;

        pEntry = pEntry->next;
    } while (pEntry != NULL);

    return NULL;
}

void dmbDictPut(dmbDict *pDict, dmbDictEntry *pEntry)
{
    dmbUINT index = pDict->meta->hashFunc(pEntry->k.val) % pDict->size;
    dmbDictEntry *pDest = pDict->entry[index];
    //Ensure pEntry->next is NULL.
    pEntry->next = NULL;

    if (pDest == NULL)
    {
        pDict->entry[index] = pEntry;
    }
    else
    {
        while (pDest->next != NULL)
            pDest = pDest->next;

        pDest->next = pEntry;
    }

    pDict->count++;
}

dmbDictEntry* dmbDictPop(dmbDict *pDict, const void *pKey)
{
    dmbUINT index = pDict->meta->hashFunc(pKey) % pDict->size;
    dmbDictEntry *pEntry = pDict->entry[index], *pPrev = NULL;

    if (pEntry == NULL)
        return NULL;

    do
    {
        if (pDict->meta->keyCompare(pKey, pEntry->k.val) == 0)
        {
            if (pDict->entry[index] == pEntry)
            {
                pDict->entry[index] = pEntry->next;
            }
            else
            {
                pPrev->next = pEntry->next;
            }
            pDict->count--;
            return pEntry;
        }

        pPrev = pEntry;
        pEntry = pEntry->next;
    } while (pEntry != NULL);

    return NULL;
}

dmbDictEntry* dmbDictGetByData(dmbDict *pDict, const void *pKeyData, dmbSIZE size)
{
    dmbUINT index = pDict->meta->dumpHashFunc(pKeyData, size) % pDict->size;
    dmbDictEntry *pEntry = pDict->entry[index];
    void *data; dmbSIZE len;

    if (pEntry == NULL)
        return NULL;

    do
    {
        pDict->meta->dumpKey(pEntry->k.val, &data, &len);
        if (pDict->meta->dumpKeyCompare(pKeyData, size, data, len) == 0)
            return pEntry;

        pEntry = pEntry->next;
    } while (pEntry != NULL);

    return NULL;
}

dmbDictEntry* dmbDictPopByData(dmbDict *pDict, const void *pKeyData, dmbSIZE size)
{
    dmbUINT index = pDict->meta->dumpHashFunc(pKeyData, size) % pDict->size;
    dmbDictEntry *pEntry = pDict->entry[index], *pPrev = NULL;
    void *data; dmbSIZE len;

    if (pEntry == NULL)
        return NULL;

    do
    {
        pDict->meta->dumpKey(pEntry->k.val, &data, &len);
        if (pDict->meta->dumpKeyCompare(pKeyData, size, data, len) == 0)
        {
            if (pDict->entry[index] == pEntry)
            {
                pDict->entry[index] = pEntry->next;
            }
            else
            {
                pPrev->next = pEntry->next;
            }
            pDict->count--;
            return pEntry;
        }

        pPrev = pEntry;
        pEntry = pEntry->next;
    } while (pEntry != NULL);

    return NULL;
}

void dmbDictInitIter(dmbDict *pDict, dmbDictIter *pIter)
{
    pIter->entry = NULL;
    //dmbDictNext will increase to to 0
    pIter->index = -1;
    pIter->dict = pDict;
}

dmbDictEntry* dmbDictNext(dmbDictIter *pIter)
{
    //move to next entry
    if (pIter->entry != NULL)
        pIter->entry = pIter->entry->next;

    //find next bucket
    while (pIter->entry == NULL)
    {
        //end
        if ((dmbUINT)(pIter->index + 1) >= pIter->dict->size)
            break;
        else
            ++(pIter->index);

        pIter->entry = pIter->dict->entry[pIter->index];
    };

    return pIter->entry;
}
