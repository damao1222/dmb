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

#include "dmbdllist.h"
#include "core/dmballoc.h"
#include "utils/dmblog.h"

static void destroyEntry(dmbDLEntry *pEntry)
{
    dmbObjectRelease(pEntry->obj);
    dmbFree(pEntry);
}

static dmbBOOL removeNode(dmbDLList *pList, dmbNode *pNode, dmbBOOL release)
{
    dmbListRemove(pNode);
    dmbDLEntry *pEntry = dmbDLListEntry(pNode);
    if (release)
    {
        destroyEntry(pEntry);
    }
    pList->len--;

    return TRUE;
}

void dmbDLListInit(dmbDLList *pList)
{
    dmbListInit(&pList->head);
    pList->len = 0;
}

void dmbDLListClear(dmbDLList *pList)
{
    dmbDLEntry *pEntry;
    dmbNode *pIter;

    dmbListClear(pEntry, pIter, &pList->head, iter, destroyEntry);
    pList->len = 0;
}

dmbDLList* dmbDLListCreate()
{
    dmbDLList *pList = (dmbDLList*)dmbMalloc(sizeof(dmbDLList));
    if (pList != NULL)
        dmbDLListInit(pList);
    return pList;
}

void dmbDLListDestroy(dmbDLList *pList)
{
    dmbDLListClear(pList);
    dmbFree(pList);
}

static inline dmbDLEntry* createEntryWithObject(dmbObject *pObject)
{
    dmbDLEntry *pEntry = (dmbDLEntry*)dmbMalloc(sizeof(dmbDLEntry));
    if (pEntry == NULL)
        return NULL;

    dmbObjectRetain(pObject);
    pEntry->obj = pObject;

    return pEntry;
}

dmbCode dmbDLListPushBack(dmbDLList *pList, dmbObject *pObject)
{
    dmbDLEntry *pEntry = createEntryWithObject(pObject);
    if (pEntry == NULL)
        return DMB_ERRCODE_ALLOC_FAILED;

    dmbListPushBack(&pList->head, &pEntry->iter);
    pList->len++;

    return DMB_ERRCODE_OK;
}

dmbCode dmbDLListPushFront(dmbDLList *pList, dmbObject *pObject)
{
    dmbDLEntry *pEntry = createEntryWithObject(pObject);
    if (pEntry == NULL)
        return DMB_ERRCODE_ALLOC_FAILED;

    dmbListPushFront(&pList->head, &pEntry->iter);
    pList->len++;

    return DMB_ERRCODE_OK;
}

dmbObject* dmbDLListPopBack(dmbDLList *pList)
{
    dmbNode *iter = dmbListPopBack(&pList->head);
    if (iter != NULL)
    {
        dmbDLEntry *pEntry = dmbDLListEntry(iter);
        dmbObject *pObj = pEntry->obj;
        dmbFree(pEntry);
        pList->len--;

        return pObj;
    }
    return NULL;
}

dmbObject* dmbDLListPopFront(dmbDLList *pList)
{
    dmbNode *iter = dmbListPopFront(&pList->head);
    if (iter != NULL)
    {
        dmbDLEntry *pEntry = dmbDLListEntry(iter);
        dmbObject *pObj = pEntry->obj;
        dmbFree(pEntry);
        pList->len--;

        return pObj;
    }
    return NULL;
}

dmbUINT dmbDLListSize(dmbDLList *pList)
{
    return pList->len;
}

dmbBOOL dmbDLListIsEmpty(dmbDLList *pList)
{
    return pList->len == 0;
}

static dmbBOOL fasterFromEnd(dmbDLList *pList, dmbUINT uIndex)
{
    return (uIndex > (pList->len >> 1));
}

dmbUINT dmbDLListRemoveRange(dmbDLList *pList, dmbUINT uStart, dmbUINT uEnd)
{
    if (uStart >= pList->len || uEnd >= pList->len || uStart > uEnd)
    {
        DMB_LOGE("param error : start %d, end %d, list size is %d", uStart, uEnd, pList->len );
        return 0;
    }

    dmbListIter iter;
    dmbUINT uCount = 0;

    //from end is faster
    if (fasterFromEnd(pList, uStart))
    {
        dmbListInitIter(&pList->head, &iter, TRUE);
        dmbUINT i = pList->len - 1;
        while (uStart <= i)
        {
            if (!dmbListNext(&iter))
                break;

            if (i <= uEnd)
            {
                if (removeNode(pList, dmbListGet(&iter), TRUE))
                {
                    ++uCount;
                }
            }

            --i;
        }
    }
    else
    {
        dmbListInitIter(&pList->head, &iter, FALSE);
        dmbUINT i = 0;
        while (uEnd >= i)
        {
            if (!dmbListNext(&iter))
                break;

            if (i >= uStart)
            {
                if (removeNode(pList, dmbListGet(&iter), TRUE))
                {
                    ++uCount;
                }
            }

            ++i;
        }
    }
    return uCount;
}

dmbBOOL dmbDLListRemoveAt(dmbDLList *pList, dmbUINT uIndex)
{
    return dmbDLListRemoveRange(pList, uIndex, uIndex) > 0;
}

dmbCode dmbDLListGetRange(dmbDLList *pList, dmbUINT uStart, dmbUINT uEnd, dmbDLList* pDestList)
{
    if (uStart > uEnd)
    {
        DMB_LOGE("param error : start %d, end %d, list size is %d", uStart, uEnd, pList->len );
        return DMB_ERRCODE_WRONG_ARGUMENT_VALUE;
    }

    if (uStart >= pList->len) uStart = pList->len - 1;
    if (uEnd >= pList->len) uEnd = pList->len - 1;

    dmbListIter iter;
    dmbCode code = DMB_ERRCODE_OK;
    //from end is faster
    if (fasterFromEnd(pList, uStart))
    {
        dmbListInitIter(&pList->head, &iter, TRUE);
        dmbUINT i = pList->len - 1;
        while (uStart <= i)
        {
            if (!dmbListNext(&iter))
                break;

            if (i <= uEnd)
            {
                code = dmbDLListPushBack(pDestList, dmbDLListEntry(dmbListGet(&iter))->obj);
                if (code != DMB_ERRCODE_OK)
                {
                    return code;
                }
            }
            --i;
        }
    }
    else
    {
        dmbListInitIter(&pList->head, &iter, FALSE);
        dmbUINT i = 0;
        while (uEnd >= i)
        {
            if (!dmbListNext(&iter))
                break;

            if (i >= uStart)
            {
                code = dmbDLListPushBack(pDestList, dmbDLListEntry(dmbListGet(&iter))->obj);
                if (code != DMB_ERRCODE_OK)
                {
                    return code;
                }
            }

            ++i;
        }
    }

    return code;
}

dmbCode dmbDLListScan(dmbDLList *pList, dmbUINT* uIndex, dmbUINT uCount, dmbDLList** pDestList)
{
    if (*uIndex >= pList->len)
    {
        *uIndex = 0;
        return DMB_ERRCODE_OK;
    }

    dmbCode code = DMB_ERRCODE_OK;
    dmbUINT i = 0, done = *uIndex + uCount;
    dmbListIter iter;
    dmbListInitIter(&pList->head, &iter, FALSE);
    while (done > i)
    {
        if (!dmbListNext(&iter))
        {
            *uIndex = 0;
            return code;
        }

        if (i >= *uIndex)
        {
            code = dmbDLListPushBack(*pDestList, dmbDLListEntry(dmbListGet(&iter))->obj);
            if (code != DMB_ERRCODE_OK)
            {
                return code;
            }
        }

        ++i;
    }

    *uIndex = i >= pList->len - 1 ? 0 : i;

    return code;
}

void dmbDLListInitIter(dmbDLList *pList, dmbDLListIter *pIter, dmbBOOL reverse)
{
    dmbListInitIter(&pList->head, pIter, reverse);
}

dmbBOOL dmbDLListNext(dmbDLListIter *pIter)
{
    return dmbListNext(pIter);
}

dmbObject* dmbDLListGetRef(dmbDLListIter *pIter)
{
    dmbObject *p = dmbDLListEntry(dmbListGet(pIter))->obj;
    dmbObjectRetain(p);
    return p;
}
