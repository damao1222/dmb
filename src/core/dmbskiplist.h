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

#ifndef DMBSKIPLIST_H
#define DMBSKIPLIST_H

#include "dmbdict.h"

#define DMB_SL_SETSCORE(NODE, SCORE) ((NODE)->data.v.val = (SCORE))
#define DMB_SL_SETVALUE(NODE, VALUE) ((NODE)->data.k.val = (VALUE))
#define DMB_SL_GETSCORE(NODE) ((NODE)->data.v.val)
#define DMB_SL_GETVALUE(NODE) ((NODE)->data.k.val)

typedef struct dmbSkipListMeta {
    dmbCode (*InitScore) (void**pScorePtr);
    dmbBOOL (*CleanScore) (void*pScore);
    dmbCode (*InitValue) (void**pValuePtr);
    dmbBOOL (*CleanValue) (void*pValue);
    dmbINT (*CompareScore) (void*pDestScore, void*pListScore);
    dmbINT (*CompareValue) (void*pDestValue, void*pListValue);
    dmbCode (*OnScan) (void*pData, void*pScore, void*pValue);
} dmbSkipListMeta;

struct dmbSkipListNode;
typedef struct dmbSkipListEntry {
    dmbUINT step;
    struct dmbSkipListNode *next;
} dmbSkipListEntry;

typedef struct dmbSkipListNode {
    dmbDictEntry data;
    struct dmbSkipListNode *prev;
    struct dmbSkipListEntry entry[];
} dmbSkipListNode;

typedef struct {
    volatile dmbINT level;
    volatile dmbLONG len;
    dmbSkipListMeta *meta;
    dmbSkipListNode header;
} dmbSkipList;

typedef struct dmbSkipListRemoveOpt {
    void (*fnBeforeRemove) (dmbSkipListNode *, void*);
    void *data;
} dmbSkipListRemoveOpt;

dmbSkipList* dmbSkipListCreate(dmbSkipListMeta *pMeta);
void dmbSkipListDestroy(dmbSkipList *pList);
dmbLONG dmbSkipListSize(dmbSkipList *pList);
void dmbSkipListRemoveAll(dmbSkipList *pList);
dmbBOOL dmbSkipListRemoveOne(dmbSkipList *pList, void *score, void *pValue);
dmbCode dmbSkipListInsert(dmbSkipList *pList, void *score, void *pValue, dmbSkipListNode **pNew);
dmbCode dmbSkipListRemoveByScore(dmbSkipList *pList, void *startScore, void *endScore, dmbLONG *pCount, dmbSkipListRemoveOpt *pOpt);
dmbCode dmbSkipListGetRangByScore(dmbSkipList *pList, void *startScore, void *endScore, void *pData, dmbBOOL reverse);
dmbUINT dmbSkipListGetRank(dmbSkipList *pList, void *score, void *value);
dmbLONG dmbSkipListGetRangeCount(dmbSkipList *pList, void *startScore, void *endScore);
dmbCode dmbSkipListScan(dmbSkipList *pList, dmbLONG startIndex, dmbLONG endIndex, void *pData, dmbLONG *nextIndex);
dmbCode dmbSkipListScanByRank(dmbSkipList *pList, dmbLONG startRank, dmbLONG endRank, void *pData, dmbLONG *nextRank, dmbBOOL reverse);

void dmbSkipListTest();

#endif // DMBSKIPLIST_H
