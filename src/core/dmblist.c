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

#include "dmblist.h"

static inline void insert(dmbNode * pNew, dmbNode *pPrev, dmbNode *pNext)
{
	pPrev->pNext = pNew;
	pNext->pPrev = pNew;
	pNew->pPrev = pPrev;
	pNew->pNext = pNext;
}

/**
 * @brief 创建一个双向链表，使用者负责调用CBDLListDestroy销毁
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return 成功返回链表指针，失败返回NULL
 */
void dmbListInit(dmbList *pHead)
{
	if (pHead != NULL)
	{
		pHead->pNext = pHead->pPrev = pHead;
	}
}

/**
 * @brief 判断双向链表是否为空
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return 为空返回TRUE，否则返回FALSE
 */
dmbBOOL dmbListIsEmpty(dmbList *pHead)
{
	return pHead == NULL ? TRUE : pHead->pNext == pHead;
}

/**
 * @brief 在链表末尾追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的指针
 * @param pNode 插入的数据
 */
void dmbListAppend(dmbList *pHead, dmbNode *pNode)
{
    insert(pNode, pHead->pPrev, pHead);
}

/**
 * @brief 在链表头追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 */
void dmbListPrepend(dmbList *pHead, dmbNode *pNode)
{
    insert(pNode, pHead, pHead->pNext);
}

/**
 * @brief 在链表头追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 */
void dmbListPushFront(dmbList *pHead, dmbNode *pNode)
{
    insert(pNode, pHead, pHead->pNext);
}

/**
 * @brief 在链表末尾追加一个元素
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 * @return 成功返回TRUE，否则返回FALSE
 */
void dmbListPushBack(dmbList *pHead, dmbNode *pNode)
{
    insert(pNode, pHead->pPrev, pHead);
}

/**
 * @brief 从链表头取出第一个节点，该节点将从链表中移除
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return dmbNode * 成功返回首个节点，失败返回NULL
 */
dmbNode* dmbListPopFront(dmbList *pHead)
{
    if (!dmbListIsEmpty(pHead))
	{
        dmbNode *p = pHead->pNext;
        dmbListRemove(p);
		return p;
	}
	return NULL;
}

/**
 * @brief 从链表尾取出最后一个节点，该节点将从链表中移除
 * 时间复杂度：O(1)
 * @param pHead 双向链表的头指针
 * @return dmbNode * 成功返回最后一个节点，失败返回NULL
 */
dmbNode* dmbListPopBack(dmbList *pHead)
{
    if (!dmbListIsEmpty(pHead))
	{
        dmbNode *p = pHead->pPrev;
        dmbListRemove(p);
		return p;
	}
	return NULL;
}

/**
 * @brief 在制定位置插入一个元素
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 * @param pNode 插入的数据
 * @param iIndex 插入的索引位置
 * @return 成功返回TRUE，否则返回FALSE
 */
dmbBOOL dmbListInsert(dmbList *pHead, dmbNode *pNode, dmbINT iIndex)
{
	if (pHead != NULL && pNode != NULL)
	{
		if (iIndex < 0)
		{
			iIndex = -iIndex - 1;
            dmbINT i = 0;
            dmbNode *pCurrent;
            dmbListForeachReverse(pCurrent, pHead)
			{
				if (i == iIndex)
				{
					insert(pNode, pCurrent, pCurrent->pNext);
					return TRUE;
				}
				++i;
			}
		}
		else
		{
            dmbINT i = 0;
            dmbNode *pCurrent;
            dmbListForeach(pCurrent, pHead)
			{
				if (i == iIndex)
				{
					insert(pNode, pCurrent->pPrev, pCurrent);
					return TRUE;
				}
				++i;
			}
		}
	}
	return FALSE;
}

/**
 * @brief 删除一个索引区域内的所有元素
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 * @param iStart 起始索引，负值表示从末尾开始计算的索引
 * @param iEnd 结尾索引，负值表示从末尾开始计算的索引
 * @return 成功删除的元素的数量
 */
dmbINT dmbListRemoveRange(dmbList *pHead, dmbINT iStart, dmbINT iEnd)
{
	if (pHead != NULL)
	{
        if (iStart < 0 || iEnd < 0 || iStart > iEnd || dmbListIsEmpty(pHead))
			return 0;

        dmbINT i = 0;
        dmbNode *pStart = NULL, *pEnd = NULL;
        dmbListForeach(pEnd, pHead)
		{
			if (i == iStart)
				pStart = pEnd->pPrev;

			if (i == iEnd)
			{
				pEnd = pEnd->pNext;
				break;
			}

			++i;
		}

		pStart->pNext = pEnd;
		pEnd->pPrev = pStart;

		return iEnd - iStart + 1;
	}
	return 0;
}

/**
 * @brief 将结点从它所在的链表中移除
 * 时间复杂度：O(1)
 * @param pNode 需要从链表中移除的节点指针
 */
void dmbListRemove(dmbNode *pNode)
{
	if (pNode != NULL)
	{
		pNode->pNext->pPrev = pNode->pPrev;
		pNode->pPrev->pNext = pNode->pNext;
		pNode->pNext = NULL;
		pNode->pPrev = NULL;
	}
}

/**dmbListRemoveRange
 * @brief 删除制定索引位置的元素
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 * @param iIndex 索引位置，负值表示从末尾开始计算的索引
 * @return 成功返回TRUE，否则返回FALSE
 */
dmbBOOL dmbListRemoveAt(dmbList *pHead, dmbINT iIndex)
{
    return dmbListRemoveRange(pHead, iIndex, iIndex) > 0;
}

/**
 * @brief 删除链表中的所有元素
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 */
void dmbListRemoveAll(dmbList *pHead)
{
	if (pHead)
	{
		pHead->pNext = pHead->pPrev = pHead;
	}
}

/**
 * @brief 将pSrc合并到pDest中，并清空pSrc
 * 时间复杂度：O(1)
 * @param pDest 目标链表
 * @param pSrc 被合并的源链表
 */
void dmbListMerge(dmbList *pDest, dmbList *pSrc)
{
    if (pDest && !dmbListIsEmpty(pSrc))
	{
		pDest->pPrev->pNext = pSrc->pNext;
		pSrc->pNext->pPrev = pDest->pPrev;
		pDest->pPrev = pSrc->pPrev;
		pSrc->pPrev->pNext = pDest;

		//clean pSrc
		pSrc->pNext = pSrc->pPrev = pSrc;
	}
}

/**
 * @brief 依次遍历到指定索引然后返回索引位置的数据，如果需要依次取数据，则使用迭代器CBListCreateIterator
 * 时间复杂度：O(n)
 * @param pHead 双向链表的头指针
 * @param iIndex 索引位置，负值表示从末尾开始计算的索引
 * @return 成功返回索引位置的结点，失败返回NULL
 */
dmbNode* CBListGet(dmbList *pHead, dmbINT iIndex)
{
	if (pHead != NULL)
	{
		if (iIndex < 0)
		{
			iIndex = -iIndex - 1;
            dmbINT i = 0;
            dmbNode *pCurrent;
            dmbListForeachReverse(pCurrent, pHead)
			{
				if (i == iIndex)
					return pCurrent;
				++i;
			}
		}
		else
		{
            dmbINT i = 0;
            dmbNode *pCurrent;
            dmbListForeach(pCurrent, pHead)
			{
				if (i == iIndex)
					return pCurrent;
				++i;
			}
		}
	}

	return NULL;
}