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

#ifndef DMBDICT_H
#define DMBDICT_H

#include "dmbdefines.h"

typedef struct dmbDictEntry {
    union {
        void *val;
        dmbUINT64 u64;
        dmbINT64 i64;
        dmbLONG l;
    } k;

    union {
        void *val;
        dmbUINT64 u64;
        dmbINT64 i64;
        dmbLONG l;
    } v;
    struct dmbDictEntry *next;
} dmbDictEntry;

#define DMB_DICT_GETLONG(ENTRY) ((ENTRY)->v.l)

typedef struct dmbDictMeta {
    dmbUINT (*hashFunc) (const void *pKey);
    dmbINT (*keyCompare) (const void *pKey1, const void *pKey2);
    void (*dumpKey) (const void *pKey, void **pKeyData, dmbSIZE *pKeyLen);
    dmbUINT (*dumpHashFunc) (const void *pKeyData, dmbSIZE dataSize);
    dmbINT (*dumpKeyCompare)(const void *pKey1Data, dmbSIZE key1Len, const void *pKey2Data, dmbSIZE key2Len);
} dmbDictMeta;

typedef struct dmbDict {
    dmbDictMeta *meta;
    dmbUINT count;
    dmbUINT size;
    dmbDictEntry *entry[];
} dmbDict;

typedef struct dmbDictIter {
    dmbDict *dict;
    dmbDictEntry *entry;
    dmbINT index;
} dmbDictIter;

dmbDict* dmbDictCreate(dmbDictMeta *pMeta, dmbUINT uSize);

void dmbDictDestroy(dmbDict *pDict);

void dmbDictRemoveAll(dmbDict *pDict);

dmbDictEntry* dmbDictGet(dmbDict *pDict, const void *pKey);

dmbDictEntry* dmbDictPop(dmbDict *pDict, const void *pKey);

dmbDictEntry* dmbDictGetByData(dmbDict *pDict, const void *pKeyData, dmbSIZE size);

dmbDictEntry* dmbDictPopByData(dmbDict *pDict, const void *pKeyData, dmbSIZE size);

void dmbDictPut(dmbDict *pDict, dmbDictEntry *pEntry);

void dmbDictInitIter(dmbDict *pDict, dmbDictIter *pIter);

dmbDictEntry* dmbDictNext(dmbDictIter *pIter);

#endif // DMBDICT_H
