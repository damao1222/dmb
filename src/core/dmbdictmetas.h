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

#include "dmbstring.h"
#include "base/dmbobject.h"
#include "dmbdict.h"
#include "dmballoc.h"

static inline __attribute__((always_inline)) dmbINT dmbCompareLong(dmbLONG l1, dmbLONG l2)
{
    return l1 - l2;
}

static inline dmbINT dmbStringDumpKeyCompare (const void *pKey1Data, dmbSIZE key1Len, const void *pKey2Data, dmbSIZE key2Len)
{
//    dmbINT ret = dmbCompareLong(key1Len, key2Len);
//    if (ret != 0)
//        return ret;

//    return dmbMemCmp(pKey1Data, pKey2Data, key1Len);
    dmbINT ret = dmbMemCmp(pKey1Data, pKey2Data, key1Len);
    if (ret == 0)
        ret = dmbCompareLong(key1Len, key2Len);
    return ret;
}

static inline dmbUINT dmbStringDumpHashFunc (const void *pKey, dmbSIZE len)
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

static inline void dmbStringDumpKey (const void *pKey, void **pKeyData, dmbSIZE *pKeyLen)
{
    dmbStringGetData((const dmbString*)pKey, (const dmbCHAR **)pKeyData, (dmbUINT*)pKeyLen);
}

static inline dmbINT dmbStringKeyCompare (const void *pKey1, const void *pKey2)
{
    dmbCHAR *v1, *v2;
    dmbUINT n1, n2;
    dmbStringGetData((const dmbString*)pKey1, (const dmbCHAR**)&v1, &n1);
    dmbStringGetData((const dmbString*)pKey2, (const dmbCHAR**)&v2, &n2);

    return dmbStringDumpKeyCompare(v1, n1, v2, n2);
}

static inline dmbUINT dmbStringHashFunc (const void *pKey)
{
    dmbSIZE len;
    void *key;

    dmbStringDumpKey(pKey, &key, &len);
    return dmbStringDumpHashFunc(key, len);
}

static inline dmbUINT dmbStringObjHashFunc (const void *pKey)
{
    return dmbStringHashFunc((void*)((dmbObject*)pKey)->ptr);
}

static inline dmbINT dmbStringObjKeyCompare (const void *pKey1, const void *pKey2)
{
    return dmbStringKeyCompare((void*)((dmbObject*)pKey1)->ptr, (void*)((dmbObject*)pKey2)->ptr);
}

static inline void dmbStringObjDumpKey (const void *pKey, void **pKeyData, dmbSIZE *pKeyLen)
{
    dmbStringGetData((const dmbString*)((dmbObject*)pKey)->ptr, (const dmbCHAR **)pKeyData, (dmbUINT*)pKeyLen);
}

extern struct dmbDictMeta dmbDictMetaStrObj;
extern struct dmbDictMeta dmbDictMetaStr;
#endif // DMBDICTMETAS_H
