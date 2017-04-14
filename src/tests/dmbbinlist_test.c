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

//#define TEST_DEFAULT_ALLCATOR

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

    dmbBinEntry *pEntry = dmbBinlistFirst(pList);
    dmbBinVar var;
    while (pEntry != NULL)
    {
        dmbBinEntryGet(pEntry, &var);
        if (DMB_BINENTRY_IS_STR(pEntry))
            DMB_LOGD("str: code: %x value: %s\n", DMB_BINCODE(pEntry), var.v.data);
        else
            DMB_LOGD("int: code: %x value: %lld\n", DMB_BINCODE(pEntry), var.v.i64);
        pEntry = dmbBinlistNext(pEntry);
    }

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

    dmbBinEntry *pEntry = dmbBinlistFirst(pSrcList);
    dmbBinVar var;
    while (pEntry != NULL)
    {
        dmbBinEntryGet(pEntry, &var);
        if (DMB_BINENTRY_IS_STR(pEntry))
            DMB_LOGD("Srclist str: code: %x value: %s\n", DMB_BINCODE(pEntry), var.v.data);
        else
            DMB_LOGD("Srclist int: code: %x value: %lld\n", DMB_BINCODE(pEntry), var.v.i64);
        pEntry = dmbBinlistNext(pEntry);
    }

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

    pEntry = dmbBinlistFirst(pDestList);
    while (pEntry != NULL)
    {
        dmbBinEntryGet(pEntry, &var);
        if (DMB_BINENTRY_IS_STR(pEntry))
        {
            DMB_LOGD("str: code: %x value: %s\n", DMB_BINCODE(pEntry), var.v.data);
        }
        else
            DMB_LOGD("int: code: %x value: %lld\n", DMB_BINCODE(pEntry), var.v.i64);
        pEntry = dmbBinlistNext(pEntry);
    }

    dmbBinlistDestroy(allocator, pSrcList);
    dmbBinlistDestroy(allocator, pDestList);

#else
    dmbBYTE fixMem[100] = {0};
    dmbFixmemAllocator fixallocator;
    dmbBinAllocator *allocator = dmbInitFixmemAllocator(&fixallocator, fixMem, sizeof(fixMem));

    dmbBinlist *pList = dmbBinlistCreate(allocator);
    dmbBYTE test_buf[100] = {0};
    dmbBinItem item;
    dmbINT i=0;
    for (; i<64; ++i)
    {
        test_buf[i] = 'a';
    }
    DMB_BINITEM_STR(&item, test_buf, 64);
    dmbCode code = dmbBinlistPushBack(allocator, &pList, &item, TRUE);
    DMB_LOGD("push back code: %d\n", code);

    for (i=0; i<64; ++i)
    {
        test_buf[i] = 'b';
    }
    DMB_BINITEM_STR(&item, test_buf, 64);
    code = dmbBinlistPushBack(allocator, &pList, &item, TRUE);
    DMB_LOGD("push back code: %d\n", code);

    dmbBinEntry *pEntry = dmbBinlistFirst(pList);
    dmbBinVar var;
    while (pEntry != NULL)
    {
        dmbBinEntryGet(pEntry, &var);
        if (DMB_BINENTRY_IS_STR(pEntry))
            DMB_LOGD("Srclist str: code: %x value: %s\n", DMB_BINCODE(pEntry), var.v.data);
        else
            DMB_LOGD("Srclist int: code: %x value: %lld\n", DMB_BINCODE(pEntry), var.v.i64);
        pEntry = dmbBinlistNext(pEntry);
    }

    dmbBinlistDestroy(allocator, pList);
#endif
}
