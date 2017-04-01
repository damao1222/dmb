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

#include "dmbskiplist.h"
#include "dmballoc.h"

#define CB_SKIPLIST_MAX_LEVEL 32
#define CB_SKIPLIST_P 0.25

static inline __attribute__((always_inline)) dmbINT CompareScore(dmbSkipList *pList, void *cmpScore, void *listScore)
{
    return pList->meta->CompareScore(cmpScore, listScore);
}

static inline __attribute__((always_inline)) dmbINT CompareValue(dmbSkipList *pList, void *cmpValue, void *listValue)
{
    return pList->meta->CompareValue(cmpValue, listValue);
}

static dmbCode CreateNode(dmbSkipListNode **pNode, dmbINT iLevel)
{
    *pNode = (dmbSkipListNode*)dmbMalloc(sizeof(dmbSkipListNode) + (sizeof(dmbSkipListEntry) * iLevel));
    if (*pNode == NULL)
        return DMB_ERRCODE_ALLOC_FAILED;

    dmbMemSet(*pNode, 0, sizeof(dmbSkipListNode));
    return DMB_ERRCODE_OK;
}

dmbSkipList* dmbSkipListCreate(dmbSkipListMeta *pMeta)
{
    dmbSkipList *pList = (dmbSkipList*)dmbMalloc(sizeof(dmbSkipList) + (sizeof(dmbSkipListEntry) * CB_SKIPLIST_MAX_LEVEL));

    if (pList == NULL)
        return NULL;

    dmbMemSet(pList, 0, sizeof(dmbSkipList) + (sizeof(dmbSkipListEntry) * CB_SKIPLIST_MAX_LEVEL));

    pList->meta = pMeta;

    return pList;
}

static inline dmbINT randomLevel()
{
    dmbINT level = 1;
    while (((random()&0xFFFF) < (CB_SKIPLIST_P * 0xFFFF)) && (CB_SKIPLIST_MAX_LEVEL > level))
        level += 1;
    return level;
}

dmbCode dmbSkipListInsert(dmbSkipList *pList, void *score, void *pValue, dmbSkipListNode **pNew)
{
    if (pValue == NULL)
        return DMB_ERRCODE_NULL_POINTER;

    dmbSkipListNode *pNode = NULL, *pNewNode;
    dmbSkipListNode *updateArr[CB_SKIPLIST_MAX_LEVEL];
    dmbUINT rankArr[CB_SKIPLIST_MAX_LEVEL];
    dmbINT iLevel = pList->level, index;
    dmbCode code = DMB_ERRCODE_OK;

    pNode = &pList->header;
    while (iLevel--)
    {
        //叠加rank
        rankArr[iLevel] = iLevel + 1 == pList->level ? 0 : rankArr[iLevel+1];
        while ((pNode->entry[iLevel].next != NULL) &&
                (CompareScore(pList, score, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) > 0 ||
                (CompareScore(pList, score, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) == 0 &&
                 CompareValue(pList, pValue, DMB_SL_GETVALUE(pNode->entry[iLevel].next)) > 0)
               ))

        {
            //取每层离插入节点最近的节点rank
            rankArr[iLevel] += pNode->entry[iLevel].step;
            pNode = pNode->entry[iLevel].next;
        }
        updateArr[iLevel] = pNode;
    }

    iLevel = randomLevel(pList);

    if (iLevel > pList->level)
    {
        for (index = pList->level; index < iLevel; ++index)
        {
            //提层，该层离插入节点最近的节点rank应该为0
            rankArr[index] = 0;
            updateArr[index] = &pList->header;
            //新加入的节点提曾，所以步长为list的当前长度
            updateArr[index]->entry[index].step = pList->len;
        }
        pList->level = iLevel;
    }

    code = CreateNode(&pNewNode, iLevel);
    if (code != DMB_ERRCODE_OK)
        return code;

    code = pList->meta->InitScore(&score);
    if (code != DMB_ERRCODE_OK)
    {
        dmbFree(pNewNode);
        return code;
    }

    code = pList->meta->InitValue(&pValue);
    if (code != DMB_ERRCODE_OK)
    {
        dmbFree(pNewNode);
        pList->meta->CleanScore(score);
        return code;
    }

    DMB_SL_SETSCORE(pNewNode, score);
    DMB_SL_SETVALUE(pNewNode, pValue);

    for (index=0; index<iLevel; ++index)
    {
        pNewNode->entry[index].next = updateArr[index]->entry[index].next;
        updateArr[index]->entry[index].next = pNewNode;
        //新节点步长为：当前最近节点步长- （实际rank - 当前曾最近节点rank）
        pNewNode->entry[index].step = updateArr[index]->entry[index].step - (rankArr[0] - rankArr[index]);
        //最近节点步长更新为：实际rank - 当前曾最近节点rank + 新节点1
        updateArr[index]->entry[index].step = rankArr[0] - rankArr[index] + 1;
    }

    for (index=iLevel; index<pList->level; ++index)
    {
        //未更新节点步长增加新节点1
        updateArr[index]->entry[index].step++;
    }

    pNewNode->prev = (updateArr[0] == (&pList->header)) ? NULL : updateArr[0];
    if (pNewNode->entry[0].next != NULL)
        pNewNode->entry[0].next->prev = pNewNode;

    pList->len++;
    if (pNew != NULL)
        *pNew = pNewNode;

    return code;
}

static void removeNode(dmbSkipList *pList, dmbSkipListNode *pNode, dmbSkipListNode **pUpdateArr)
{
    dmbINT index;

    for (index=0; index<pList->level; ++index)
    {
        if (pUpdateArr[index]->entry[index].next == pNode)
        {
            pUpdateArr[index]->entry[index].step += pNode->entry[index].step - 1;
            pUpdateArr[index]->entry[index].next = pNode->entry[index].next;
        }
        else
        {
            pUpdateArr[index]->entry[index].step--;
        }
    }

    if (pNode->entry[0].next != NULL)
        pNode->entry[0].next->prev = pNode->prev;

    while (pList->level > 1 && pList->header.entry[pList->level-1].next == NULL)
        pList->level--;

    pList->len--;
}

static dmbSkipListNode* CBSkipListGetFirstByScore(dmbSkipList *pList, void *pScroe)
{
    dmbSkipListNode *pNode = NULL;
    dmbINT iLevel = pList->level;

    pNode = &pList->header;
    while (iLevel--)
    {
        while ((pNode->entry[iLevel].next != NULL) &&
               (CompareScore(pList, pScroe, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) > 0))
        {
            pNode = pNode->entry[iLevel].next;
        }
    }

    return pNode->entry[0].next;
}

static dmbSkipListNode* CBSkipListGetLastByScore(dmbSkipList *pList, void *pScroe)
{
    dmbSkipListNode *pNode = NULL;
    dmbINT iLevel = pList->level;

    pNode = &pList->header;
    while (iLevel--)
    {
        while ((pNode->entry[iLevel].next != NULL) &&
               (CompareScore(pList, pScroe, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) >= 0))
        {
            pNode = pNode->entry[iLevel].next;
        }
    }

    return (pNode == &pList->header) ? NULL : pNode;
}

dmbCode dmbSkipListGetRangByScore(dmbSkipList *pList, void *lStartScore, void *lEndScore, void *pData, dmbBOOL reverse)
{
    dmbSkipListNode *pNode = NULL;
    dmbCode code = DMB_ERRCODE_OK;

    if (reverse)
    {
        pNode = CBSkipListGetLastByScore(pList, lEndScore);

        while (pNode != NULL && CompareScore(pList, lStartScore, DMB_SL_GETSCORE(pNode)) <= 0)
        {
            code = pList->meta->OnScan(pData, DMB_SL_GETSCORE(pNode), DMB_SL_GETVALUE(pNode));
            if (code != DMB_ERRCODE_OK)
            {
                return code == DMB_ERRCODE_SKIPLIST_SCAN_BREAK ? DMB_ERRCODE_OK : code;
            }
            pNode = pNode->prev;
        }
    }
    else
    {
        pNode = CBSkipListGetFirstByScore(pList, lStartScore);

        while (pNode != NULL && CompareScore(pList, lEndScore, DMB_SL_GETSCORE(pNode)) >= 0)
        {
            code = pList->meta->OnScan(pData, DMB_SL_GETSCORE(pNode), DMB_SL_GETVALUE(pNode));
            if (code != DMB_ERRCODE_OK)
            {
                return code == DMB_ERRCODE_SKIPLIST_SCAN_BREAK ? DMB_ERRCODE_OK : code;
            }
            pNode = pNode->entry[0].next;
        }
    }

    return code;
}

dmbLONG dmbSkipListGetRangeCount(dmbSkipList *pList, void *startScore, void *endScore)
{
    dmbSkipListNode *pNode = NULL;
    dmbINT iLevel = pList->level;
    dmbLONG lCount = 0;

    pNode = &pList->header;
    while (iLevel--)
    {
        while ((pNode->entry[iLevel].next != NULL) &&
               CompareScore(pList, startScore, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) > 0)
        {
            pNode = pNode->entry[iLevel].next;
        }
    }

    pNode = pNode->entry[0].next;

    while (pNode != NULL && CompareScore(pList, endScore, DMB_SL_GETSCORE(pNode)) >= 0)
    {
        ++lCount;
        pNode = pNode->entry[0].next;
    }

    return lCount;
}

dmbCode dmbSkipListScan(dmbSkipList *pList, dmbLONG startIndex, dmbLONG endIndex, void *pData, dmbLONG *nextIndex)
{
    dmbSkipListNode *pNode = &pList->header;
    dmbLONG curIndex = 0;
    dmbCode code = DMB_ERRCODE_OK;
    startIndex = startIndex < 0 ? pList->len + startIndex : startIndex;
    endIndex = endIndex < 0 ? pList->len + endIndex : endIndex;

    if (startIndex >= pList->len || startIndex < 0)
    {
        if (*nextIndex) *nextIndex = 0;
        return DMB_ERRCODE_OK;
    }

    if (endIndex >= pList->len || endIndex < 0)
        endIndex = pList->len - 1;

    pNode = pNode->entry[0].next;

    while (pNode != NULL && curIndex <= endIndex)
    {
        if (curIndex >= startIndex)
        {
            code = pList->meta->OnScan(pData, DMB_SL_GETSCORE(pNode), DMB_SL_GETVALUE(pNode));
            if (code != DMB_ERRCODE_OK)
            {
                return code == DMB_ERRCODE_SKIPLIST_SCAN_BREAK ? DMB_ERRCODE_OK : code;
            }
        }

        ++curIndex;
        pNode = pNode->entry[0].next;
    }

    if (nextIndex != NULL)
    {
        *nextIndex = code == DMB_ERRCODE_OK ? curIndex : 0;
    }

    return code;
}

dmbCode dmbSkipListScanByRank(dmbSkipList *pList, dmbLONG startRank, dmbLONG endRank, void *pData, dmbLONG *nextRank, dmbBOOL reverse)
{
    dmbSkipListNode *pNode = NULL;
    dmbUINT cur = 0, start, end;
    dmbINT iLevel = pList->level;
    dmbCode code = DMB_ERRCODE_ZSETMEMBER_NOT_EXIST;
    startRank = startRank < 0 ? pList->len + startRank : startRank;
    endRank = endRank < 0 ? pList->len + endRank : endRank;

    if (startRank >= pList->len || startRank < 0)
    {
        if (nextRank != NULL) *nextRank = 0;
        return DMB_ERRCODE_OK;
    }

    if (endRank >= pList->len || endRank < 0)
        endRank = (dmbUINT)pList->len - 1;

    if (reverse)
    {
        start = (dmbUINT)endRank + 1;
        end = (dmbUINT)startRank + 1;
    }
    else
    {
        start = (dmbUINT)startRank + 1;
        end = (dmbUINT)endRank + 1;
    }

    pNode = &pList->header;
    while (iLevel--)
    {
        while ((pNode->entry[iLevel].next != NULL) && (cur + pNode->entry[iLevel].step <= start))
        {
            cur += pNode->entry[iLevel].step;
            pNode = pNode->entry[iLevel].next;
        }

        if (cur == start)
            break;
    }

    if (reverse)
    {
        while (pNode != NULL && cur >= end)
        {
            code = pList->meta->OnScan(pData, DMB_SL_GETSCORE(pNode), DMB_SL_GETVALUE(pNode));
            if (code != DMB_ERRCODE_OK)
            {
                return code == DMB_ERRCODE_SKIPLIST_SCAN_BREAK ? DMB_ERRCODE_OK : code;
            }

            --cur;
            pNode = pNode->prev;
        }
    }
    else
    {
        while (pNode != NULL && cur <= end)
        {
            code = pList->meta->OnScan(pData, DMB_SL_GETSCORE(pNode), DMB_SL_GETVALUE(pNode));
            if (code != DMB_ERRCODE_OK)
            {
                return code == DMB_ERRCODE_SKIPLIST_SCAN_BREAK ? DMB_ERRCODE_OK : code;
            }

            ++cur;
            pNode = pNode->entry[0].next;
        }
    }

    if (nextRank != NULL)
    {
        *nextRank = (code == DMB_ERRCODE_OK) && cur <= pList->len ? cur - 1 : 0;
    }

    return code;
}

dmbUINT dmbSkipListGetRank(dmbSkipList *pList, void *score, void *value)
{
    dmbSkipListNode *pNode = NULL;
    dmbUINT rank = 0;
    dmbINT iLevel = pList->level;

    pNode = &pList->header;
    while (iLevel--)
    {
        while ((pNode->entry[iLevel].next != NULL) &&
                (CompareScore(pList, score, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) > 0 ||
                (CompareScore(pList, score, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) == 0 &&
                 CompareValue(pList, value, DMB_SL_GETVALUE(pNode->entry[iLevel].next)) >= 0)
               ))
        {
            rank += pNode->entry[iLevel].step;
            pNode = pNode->entry[iLevel].next;
        }

        if (DMB_SL_GETVALUE(pNode) && CompareValue(pList, value, DMB_SL_GETVALUE(pNode)) == 0)
            return rank;
    }
    return 0;
}

dmbBOOL dmbSkipListRemoveOne(dmbSkipList *pList, void *score, void *pValue)
{
    dmbSkipListNode *pNode = NULL;
    dmbSkipListNode *updateArr[CB_SKIPLIST_MAX_LEVEL];
    dmbINT iLevel = pList->level;

    pNode = &pList->header;
    while (iLevel--)
    {
        while ((pNode->entry[iLevel].next != NULL) &&
                (CompareScore(pList, score, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) > 0 ||
                (CompareScore(pList, score, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) == 0 &&
                 CompareValue(pList, pValue, DMB_SL_GETVALUE(pNode->entry[iLevel].next)) > 0)
               ))
        {
            pNode = pNode->entry[iLevel].next;
        }
        updateArr[iLevel] = pNode;
    }

    pNode = pNode->entry[0].next;
    if (pNode != NULL && CompareScore(pList, score, DMB_SL_GETSCORE(pNode)) == 0 &&
            CompareValue(pList, pValue, DMB_SL_GETVALUE(pNode)) == 0)
    {
        removeNode(pList, pNode, updateArr);
        pList->meta->CleanScore(DMB_SL_GETSCORE(pNode));
        pList->meta->CleanValue(DMB_SL_GETVALUE(pNode));
        dmbFree(pNode);
        return TRUE;
    }

    return FALSE;
}

dmbCode dmbSkipListRemoveByScore(dmbSkipList *pList, void *lStartScore, void *lEndScore, dmbLONG *pCount, dmbSkipListRemoveOpt *pOpt)
{
    dmbSkipListNode *pNode = NULL, *pNext;
    dmbSkipListNode *updateArr[CB_SKIPLIST_MAX_LEVEL];
    dmbINT iLevel = pList->level;
    dmbLONG lCount = 0;
    dmbCode code = DMB_ERRCODE_OK;

    pNode = &pList->header;
    while (iLevel--)
    {
        while ((pNode->entry[iLevel].next != NULL) &&
               (CompareScore(pList, lStartScore, DMB_SL_GETSCORE(pNode->entry[iLevel].next)) > 0))
        {
            pNode = pNode->entry[iLevel].next;
        }
        updateArr[iLevel] = pNode;
    }

    pNode = pNode->entry[0].next;

    while (pNode != NULL && (CompareScore(pList, lEndScore, DMB_SL_GETSCORE(pNode)) >= 0))
    {
        pNext = pNode->entry[0].next;
        if (pOpt != NULL)
            pOpt->fnBeforeRemove(pNode, pOpt->data);
        removeNode(pList, pNode, updateArr);
        pList->meta->CleanScore(DMB_SL_GETSCORE(pNode));
        pList->meta->CleanValue(DMB_SL_GETVALUE(pNode));
        dmbFree(pNode);
        ++lCount;
        pNode = pNext;
    }

    if (pCount != NULL) *pCount = lCount;

    return code;
}

void dmbSkipListRemoveAll(dmbSkipList *pList)
{
    dmbSkipListNode *pNode = &pList->header, *pNext;
    pNode = pNode->entry[0].next;
    while (pNode != NULL)
    {
        pNext = pNode->entry[0].next;
        pList->meta->CleanScore(DMB_SL_GETSCORE(pNode));
        pList->meta->CleanValue(DMB_SL_GETVALUE(pNode));
        dmbFree(pNode);
        pNode = pNext;
    }
    dmbMemSet(((dmbBYTE*)pList) + sizeof(dmbSkipList), 0, (sizeof(dmbSkipListEntry) * CB_SKIPLIST_MAX_LEVEL));
    pList->len = 0;
}

void dmbSkipListDestroy(dmbSkipList *pList)
{
    dmbSkipListRemoveAll(pList);
    dmbFree(pList);
}

dmbLONG dmbSkipListSize(dmbSkipList *pList)
{
    return pList->len;
}

