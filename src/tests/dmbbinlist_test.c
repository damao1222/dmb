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

#include "dmbbinlist_test.h"
#include "core/dmbbinlist.h"
#include "utils/dmblog.h"
#include "core/dmballoc.h"

//#define TEST_DEFAULT_ALLCATOR

static void printEntrys(const char *pcTag, dmbBinlist *pList);

void dmbbinlist_test()
{
#ifdef TEST_DEFAULT_ALLCATOR
    dmbBinAllocator *allocator = DMB_DEFAULT_BINALLOCATOR;
#else
//    dmbBYTE fixMem[20480] = {0};
    dmbBYTE fixMem[16384] = {0};
    dmbFixmemAllocator fixallocator;
    dmbBinAllocator *allocator = dmbInitFixmemAllocator(&fixallocator, fixMem, sizeof(fixMem));
#endif
    dmbBinlist *pList = dmbBinlistCreate(allocator);
    dmbBYTE test_buf[16384] = {0};
    dmbBinItem item;

    DMB_BINITEM_I16(&item, 1);
    dmbBinlistPushBack(allocator, &pList, &item, FALSE);

    DMB_BINITEM_I32(&item, 2);
    dmbBinlistPushBack(allocator, &pList, &item, FALSE);

    DMB_BINITEM_I64(&item, 3);
    dmbBinlistPushBack(allocator, &pList, &item, FALSE);

    test_buf[0] = 'a';
    DMB_BINITEM_STR(&item, test_buf, 2);
    dmbBinlistPushBack(allocator, &pList, &item, FALSE);

    test_buf[0] = 'b';
    DMB_BINITEM_STR(&item, test_buf, 64);
    dmbBinlistPushBack(allocator, &pList, &item, FALSE);

    test_buf[0] = 'c';
    DMB_BINITEM_STR(&item, test_buf, 16384);
    dmbBinlistPushBack(allocator, &pList, &item, TRUE);

    DMB_LOGD("Binlist len is %d\n", dmbBinlistLen(pList));

    printEntrys("list", pList);

    dmbBinlistDestroy(allocator, pList);
}

void dmbbinlist_merge_test()
{
#ifdef TEST_DEFAULT_ALLCATOR
    dmbBinAllocator *allocator = DMB_DEFAULT_BINALLOCATOR;
    dmbBinlist *pSrcList = dmbBinlistCreate(allocator);
    dmbBYTE test_buf[16384] = {0};
    dmbBinItem item;

//    DMB_BINITEM_I16(&item, 1);
//    dmbBinlistPushBack(allocator, &pSrcList, &item, FALSE);

//    DMB_BINITEM_I32(&item, 2);
//    dmbBinlistPushBack(allocator, &pSrcList, &item, FALSE);

//    DMB_BINITEM_I64(&item, 3);
//    dmbBinlistPushBack(allocator, &pSrcList, &item, FALSE);

    test_buf[0] = 'a';
    DMB_BINITEM_STR(&item, test_buf, 2);
    dmbBinlistPushBack(allocator, &pSrcList, &item, FALSE);

    test_buf[0] = 'b';
    DMB_BINITEM_STR(&item, test_buf, 64);
    dmbBinlistPushBack(allocator, &pSrcList, &item, FALSE);

    test_buf[0] = 'c';
    DMB_BINITEM_STR(&item, test_buf, 16384);
    dmbBinlistPushBack(allocator, &pSrcList, &item, TRUE);

    DMB_LOGD("Srclist len is %d\n", dmbBinlistLen(pSrcList));

    printEntrys(pSrcList);

    dmbBinlist *pDestList = dmbBinlistCreate(allocator);

    DMB_BINITEM_I16(&item, 1);
    dmbBinlistPushBack(allocator, &pDestList, &item, FALSE);

    DMB_BINITEM_I32(&item, 2);
    dmbBinlistPushBack(allocator, &pDestList, &item, FALSE);

    DMB_BINITEM_I64(&item, 3);
    dmbBinlistPushBack(allocator, &pDestList, &item, FALSE);

    test_buf[0] = 'a';
    DMB_BINITEM_STR(&item, test_buf, 2);
    dmbBinlistPushBack(allocator, &pDestList, &item, FALSE);

    test_buf[0] = 'b';
    DMB_BINITEM_STR(&item, test_buf, 64);
    dmbBinlistPushBack(allocator, &pDestList, &item, FALSE);

    dmbInt i=0;
    for (; i<1; ++i)
    {
        test_buf[i] = 'c';
    }
    DMB_BINITEM_STR(&item, test_buf, 1);
    dmbBinlistPushBack(allocator, &pDestList, &item, TRUE);

    DMB_LOGD("dest list len is %d\n", dmbBinlistLen(pDestList));

    dmbCode code = dmbBinListMerge(allocator, &pDestList, pSrcList, TRUE);

    DMB_LOGD("dest merge error : %d, dest list len is %d\n", code, dmbBinlistLen(pDestList));

    printEntrys(pDestList);

    dmbBinlistDestroy(allocator, pSrcList);
    dmbBinlistDestroy(allocator, pDestList);

#else
    dmbBYTE fixMem[48] = {0};
    dmbFixmemAllocator fixallocator;
    dmbBinAllocator *allocator = dmbInitFixmemAllocator(&fixallocator, fixMem, sizeof(fixMem));
    const dmbUINT TEST_SIZE = 64;
    const dmbUINT TEST_ARR_SIZE = 3;

    dmbBinAllocator *mergeAllocator = DMB_DEFAULT_BINALLOCATOR;
    dmbBinlist *pMergeList = dmbBinlistCreate(mergeAllocator);

    dmbBinlist *pList = dmbBinlistCreate(allocator);
    dmbBYTE test_datas[TEST_ARR_SIZE][TEST_SIZE];
    int i = 0;
    for (i=0; i<TEST_ARR_SIZE; ++i)
    {
        dmbMemSet(test_datas[i], 'a' + i, TEST_SIZE);
    }

    dmbBinItem item;
    dmbCode code = DMB_ERRCODE_OK, mergeCode;

    for (i=0; i<TEST_ARR_SIZE; )
    {
        if (code == DMB_ERRCODE_OK)
            DMB_BINITEM_STR(&item, test_datas[i], TEST_SIZE);

        code = dmbBinlistPushBack(allocator, &pList, &item, TRUE);
        DMB_LOGD("push back code: %d\n", code);
        if (code == DMB_ERRCODE_OK)
        {
            ++i;
            if (i < TEST_ARR_SIZE)
                continue;
        }

        if (code == DMB_ERRCODE_BINLIST_INSERT_PART || code == DMB_ERRCODE_OK)
        {
            mergeCode = dmbBinListMerge(mergeAllocator, &pMergeList, pList, TRUE);
            DMB_LOGD("==============================================\n");
            printEntrys("src list", pList);
            DMB_LOGD("----------------------------------------------\n");
            printEntrys("merge list", pMergeList);
            DMB_LOGD("merge code: %d\n", mergeCode);
            dmbBinlistClear(allocator, &pList);
            if (mergeCode != DMB_ERRCODE_OK)
                return -1;
        }
        else
        {
//            return -1;
        }
    }

    dmbBinlistDestroy(allocator, pList);
    dmbBinlistDestroy(mergeAllocator, pMergeList);
#endif
}

static void  printEntrys(const char *pcTag, dmbBinlist *pList)
{
    dmbBYTE debugBuf[40960];
    dmbBinVar var;
    dmbBinEntry *pEntry = dmbBinlistFirst(pList);
    while (pEntry != NULL)
    {
        dmbBinEntryGet(pEntry, &var);
        if (DMB_BINENTRY_IS_STR(pEntry))
        {
            dmbMemCopy(debugBuf, var.v.data, var.len);
            debugBuf[var.len] = 0;
            DMB_LOGD("%s: code: %x value: %s\n", pcTag, DMB_BINCODE(pEntry), debugBuf);
        }
        else
            DMB_LOGD("%s: code: %x value: %lld\n", pcTag, DMB_BINCODE(pEntry), var.v.i64);
        pEntry = dmbBinlistNext(pEntry);
    }
}
