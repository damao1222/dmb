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

#ifndef DMBDLLIST_H
#define DMBDLLIST_H

#include "dmbobject.h"
#include "core/dmblist.h"

typedef struct {
    dmbNode iter;
    dmbObject *obj;
} dmbDLEntry;

typedef struct {
    dmbNode head;
    dmbUINT len;
} dmbDLList;

typedef dmbListIter dmbDLListIter;

void dmbDLListInit(dmbDLList *pList);
void dmbDLListClear(dmbDLList *pList);
dmbDLList* dmbDLListCreate();
void dmbDLListDestroy(dmbDLList *pList);

dmbCode dmbDLListPushBack(dmbDLList *pList, dmbObject *pObject);
dmbCode dmbDLListPushFront(dmbDLList *pList, dmbObject *pObject);

/**
 * @brief dmbDLListPopBack 需要手动release
 * @param pList
 * @return
 */
dmbObject* dmbDLListPopBack(dmbDLList *pList);

/**
 * @brief dmbDLListPopFront 需要手动release
 * @param pList
 * @return
 */
dmbObject* dmbDLListPopFront(dmbDLList *pList);

dmbUINT dmbDLListSize(dmbDLList *pList);
dmbBOOL dmbDLListIsEmpty(dmbDLList *pList);

dmbUINT dmbDLListRemoveRange(dmbDLList *pList, dmbUINT uStart, dmbUINT uEnd);
dmbBOOL dmbDLListRemoveAt(dmbDLList *pList, dmbUINT uIndex);

dmbCode dmbDLListGetRange(dmbDLList *pList, dmbUINT uStart, dmbUINT uEnd, dmbDLList* pDestList);
dmbCode dmbDLListScan(dmbDLList *pList, dmbUINT* uIndex, dmbUINT uCount, dmbDLList** pDestList);

void dmbDLListInitIter(dmbDLList *pList, dmbDLListIter *pIter, dmbBOOL reverse);
dmbBOOL dmbDLListNext(dmbDLListIter *pIter);

/**
 * @brief dmbDLListGetRef 需要手动release
 * @param pIter
 * @return
 */
dmbObject* dmbDLListGetRef(dmbDLListIter *pIter);

#define dmbDLListEntry(pos) \
            dmbListEntry(pos, dmbDLEntry, iter)

#define dmbDLListForeach(pos, list) \
            dmbListForeachEntry(pos, &list->head, iter)
#endif // DMBDLLIST_H
