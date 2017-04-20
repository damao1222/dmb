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

#ifndef DMBLIST_H
#define DMBLIST_H

#include "dmbdefines.h"

typedef struct dmbNode
{
    struct dmbNode *pNext, *pPrev;
} dmbList, dmbNode;

typedef struct dmbListIter
{
    dmbBOOL reverse;
    dmbNode *pNode;
    dmbList *pList;
} dmbListIter;

/**
 * @brief 创建一个双向链表，使用者负责调用CBDLListDestroy销毁
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return 成功返回链表指针，失败返回NULL
 */
void dmbListInit(dmbList *pHead);

/**
 * @brief 判断双向链表是否为空
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return 为空返回TRUE，否则返回FALSE
 */
dmbBOOL dmbListIsEmpty(dmbList *pHead);

/**
 * @brief 在链表末尾追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 */
void dmbListAppend(dmbList *pHead, dmbNode *pNode);

/**
 * @brief 在链表头追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 */
void dmbListPrepend(dmbList *pHead, dmbNode *pNode);

/**
 * @brief 在链表头追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 */
void dmbListPushFront(dmbList *pHead, dmbNode *pNode);

/**
 * @brief 在链表末尾追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 */
void dmbListPushBack(dmbList *pHead, dmbNode *pNode);

/**
 * @brief 从链表头取出第一个节点，该节点将从链表中移除
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return dmbNode * 成功返回首个节点，失败返回NULL
 */
dmbNode* dmbListPopFront(dmbList *pHead);

/**
 * @brief 从链表尾取出最后一个节点，该节点将从链表中移除
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return dmbNode * 成功返回最后一个节点，失败返回NULL
 */
dmbNode* dmbListPopBack(dmbList *pHead);

/**
 * @brief 在制定位置插入一个元素
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 * @param iIndex 插入的索引位置
 * @return 成功返回TRUE，否则返回FALSE
 */
dmbBOOL dmbListInsert(dmbList *pHead, dmbNode *pNode, dmbINT iIndex);

/**
 * @brief 删除一个索引区域内的所有元素
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 * @param iStart 起始索引，负值表示从末尾开始计算的索引
 * @param iEnd 结尾索引，负值表示从末尾开始计算的索引
 * @return 成功删除的元素的数量
 */
dmbINT dmbListRemoveRange(dmbList *pHead, dmbINT iStart, dmbINT iEnd);

/**
 * @brief 删除制定索引位置的元素
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 * @param iIndex 索引位置，负值表示从末尾开始计算的索引
 * @return 成功返回TRUE，否则返回FALSE
 */
dmbBOOL dmbListRemoveAt(dmbList *pHead, dmbINT iIndex);

/**
 * @brief 将结点从它所在的链表中移除
 * 时间复杂度：O(1)
 * @param pNode 需要从链表中移除的节点指针
 */
void dmbListRemove(dmbNode *pNode);

/**
 * @brief 删除链表中的所有元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 */
void dmbListRemoveAll(dmbList *pHead);

/**
 * @brief 将pSrc所有元素合并到pDest中，并清空pSrc
 * 时间复杂度：O(1)
 * @param pDest 目标链表
 * @param pSrc 被合并的源链表
 */
void dmbListMerge(dmbList *pDest, dmbList *pSrc);

/**
 * @brief dmbListInitIter 初始化迭代器
 * @param pList 链表指针
 * @param pIter 迭代器
 * @param reverse 是否反向迭代
 */
void dmbListInitIter(dmbList *pList, dmbListIter *pIter, dmbBOOL reverse);

/**
 * @brief dmbListNext 移动到下个节点
 * @param pIter 迭代器
 * @return 成功返回TRUE，遍历完毕返回FALSE
 */
dmbBOOL dmbListNext(dmbListIter *pIter);

/**
 * @brief dmbListGet 从迭代器中获得节点
 * @param pIter 迭代器
 * @return 节点指针
 */
dmbNode* dmbListGet(dmbListIter *pIter);

/**
 * @brief 根据节点获得实际数据结构指针
 * @param ptr 当前节点指针
 * @param type 实际数据结构类型
 * @param member 节点在实际数据结构中的名称
 */
#define dmbListEntry(ptr, type, member) \
        DMB_CONTAINER_OF(ptr, type, member)

/**
 * @brief 从头到尾遍历一次链表
 * @param pos 存储当前结点的指针
 * @param head 链表头节点
 */
#define dmbListForeach(pos, head) \
        for (pos = (head)->pNext; pos != (head); pos = pos->pNext)

/**
 * @brief 从尾到头遍历一次链表
 * @param pos 存储当前结点的指针
 * @param head 链表头节点
 */
#define dmbListForeachReverse(pos, head) \
        for (pos = (head)->pPrev; pos != (head); pos = pos->pPrev)

/**
 * @brief 从头到尾遍历一次链表，取出实际的数据结构指针
 * @param entry 存储实际数据结构的指针
 * @param head 链表头节点
 * @param member 节点在实际数据结构中的名称
 */
#define dmbListForeachEntry(entry, head, member) \
        for (entry = dmbListEntry((head)->pNext, typeof(*entry), member); \
             &entry->member != (head); \
             entry = dmbListEntry(entry->member.pNext, typeof(*entry), member))

/**
 * @brief 从尾到头遍历一次链表，取出实际的数据结构指针
 * @param entry 存储实际数据结构的指针
 * @param head 链表头节点
 * @param member 节点在实际数据结构中的名称
 */
#define dmbListForeachEntryReverse(entry, head, member) \
        for (entry = dmbListEntry((head)->pPrev, typeof(*entry), member); \
             &entry->member != (head); \
             entry = dmbListEntry(entry->member.pPrev, typeof(*entry), member))

/**
 * @brief 从头到尾遍历一次链表，清除所有数据
 * @param entry 存储实际数据结构的指针
 * @param pos 存储当前结点的指针
 * @param head 链表头节点
 * @param member 节点在实际数据结构中的名称
 * @param func 清除entry的函数
 */
#define dmbListClear(entry, pos, head, member, func) \
        while (!dmbListIsEmpty(head)) { \
            pos = (head)->pNext; \
            entry = dmbListEntry(pos, typeof(*entry), member); \
            dmbListRemove(pos); \
            func(pEntry); \
        }
#endif // DMBLIST_H
