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

#include "dmbdllist_test.h"
#include "base/dmbdllist.h"
#include "utils/dmblog.h"

void print(const char* tag, dmbDLList *pList);

void dmbdllist_test()
{
    dmbDLList *pList = dmbDLListCreate(), *pTmpList = dmbDLListCreate();
    dmbUINT uIndex = 0;

    dmbObject *o = dmbCreateIntObject(1);
    dmbDLListPushBack(pList, o);
    dmbObjectRelease(o);
    o = dmbCreateIntObject(2);
    dmbDLListPushBack(pList, o);
    dmbObjectRelease(o);

    o = dmbCreateIntObject(3);
    dmbDLListPushFront(pList, o);
    dmbObjectRelease(o);

    o = dmbCreateIntObject(4);
    dmbDLListPushFront(pList, o);
    dmbObjectRelease(o);

    print("plist", pList);

    do {
        dmbDLListScan(pList, &uIndex, 2, &pTmpList);

        print("tmplist scan", pTmpList);
        DMB_LOGD("----------scan index %d-----------\n", uIndex);
    } while (uIndex != 0);

    dmbDLListClear(pTmpList);

    dmbDLListGetRange(pList, 1, 5, pTmpList);

    print("tmplist range", pTmpList);

    o = dmbDLListPopFront(pList);
    DMB_LOGD("dmbDLListPopFront : %d\n", o->num);
    dmbObjectRelease(o);

    o = dmbDLListPopBack(pList);
    DMB_LOGD("dmbDLListPopBack : %d\n", o->num);
    dmbObjectRelease(o);

    print("plist", pList);

    dmbDLListDestroy(pList);
    dmbDLListDestroy(pTmpList);
}

void print(const char* tag, dmbDLList *pList)
{
    dmbDLEntry *pEntry;
    dmbDLListForeach(pEntry, pList)
    {
        DMB_LOGD("%s: list size %d, entry is %d\n", tag, dmbDLListSize(pList), pEntry->obj->num);
    }
}
