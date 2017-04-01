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

#ifndef DMBDICTMETAS_H
#define DMBDICTMETAS_H

#include "cbstring.h"
#include "cbobject.h"
#include "cbdict.h"
#include "cballoc.h"

static inline __attribute__((always_inline)) dmbINT CBCompareLong(dmbLONG l1, dmbLONG l2)
{
    if (l1 > l2)
        return 1;
    else if (l1 == l2)
        return 0;
    else
        return -1;
}

static inline dmbINT CBStringDumpKeyCompare (const void *pKey1Data, dmbSIZE key1Len, const void *pKey2Data, dmbSIZE key2Len)
{
//    dmbINT ret = CBCompareLong(key1Len, key2Len);
//    if (ret != 0)
//        return ret;

//    return dmbMemCmp(pKey1Data, pKey2Data, key1Len);
    dmbINT ret = dmbMemCmp(pKey1Data, pKey2Data, key1Len);
    if (ret == 0)
        ret = CBCompareLong(key1Len, key2Len);
    return ret;
}

static inline dmbUINT CBStringDumpHashFunc (const void *pKey, dmbSIZE len)
{
    dmbUINT hash = 5381;
    dmbSIZE i;
    dmbCHAR *key = (dmbCHAR*)pKey;

    for (i = 0; i < len; i++)
    {
        hash += (hash << 5) + key[i];
    }
    return hash;
}

static inline void CBStringDumpKey (const void *pKey, void **pKeyData, dmbSIZE *pKeyLen)
{
    CBStringGetData((CBString*)pKey, (dmbCHAR **)pKeyData, (dmbLONG*)pKeyLen);
}

static inline dmbINT CBStringKeyCompare (const void *pKey1, const void *pKey2)
{
    dmbCHAR *v1, *v2;
    dmbLONG n1, n2;
    CBStringGetData((CBString*)pKey1, &v1, &n1);
    CBStringGetData((CBString*)pKey2, &v2, &n2);

    return CBStringDumpKeyCompare(v1, n1, v2, n2);
}

static inline dmbUINT CBStringHashFunc (const void *pKey)
{
    dmbSIZE len;
    void *key;

    CBStringDumpKey(pKey, &key, &len);
    return CBStringDumpHashFunc(key, len);
}

static inline dmbUINT CBStringObjHashFunc (const void *pKey)
{
    return CBStringHashFunc((void*)((CBObject*)pKey)->ptr);
}

static inline dmbINT CBStringObjKeyCompare (const void *pKey1, const void *pKey2)
{
    return CBStringKeyCompare((void*)((CBObject*)pKey1)->ptr, (void*)((CBObject*)pKey2)->ptr);
}

static inline void CBStringObjDumpKey (const void *pKey, void **pKeyData, dmbSIZE *pKeyLen)
{
    CBStringGetData((CBString*)((CBObject*)pKey)->ptr, (dmbCHAR **)pKeyData, (dmbLONG*)pKeyLen);
}

extern struct CBDictMeta CBDictMetaStrObj;
extern struct CBDictMeta CBDictMetaStr;
#endif // DMBDICTMETAS_H
