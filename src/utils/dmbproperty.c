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

#include "dmbproperty.h"
#include "dmbioutil.h"
#include "../core/dmballoc.h"
#include "../core/dmbdictmetas.h"
#include "dmblog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static dmbCode Parse(dmbProperty *pProperty, dmbBYTE* lpBuf, size_t size);

dmbPropertyEntry* createEntry(dmbString *key, dmbString *value)
{
    dmbPropertyEntry* pEntry = (dmbPropertyEntry*)dmbMalloc(sizeof(dmbPropertyEntry));

    if (pEntry == NULL)
        return NULL;

    pEntry->entry.next = NULL;
    pEntry->entry.k.val = key;
    pEntry->entry.v.val = value;

    return pEntry;
}

dmbProperty* dmbPropertyCreate()
{
    dmbProperty *property = (dmbProperty*)dmbMalloc(sizeof(dmbProperty));
    if (property == NULL)
        return NULL;

    property->dict = dmbDictCreate(&dmbDictMetaStr, 100);
    if (property->dict == NULL)
    {
        dmbFree(property);
        return NULL;
    }
    dmbListInit(&property->list);

    return property;
}

void dmbPropertyDestroy(dmbProperty *pProperty)
{
    dmbPropertyClean(pProperty);
    dmbDictDestroy(pProperty->dict);
    dmbFree(pProperty);
}

static void destroyEntry(dmbPropertyEntry *pEntry)
{
    dmbStringDestroy((dmbString*)pEntry->entry.k.val);
    dmbStringDestroy((dmbString*)pEntry->entry.v.val);
    dmbFree(pEntry);
}

void dmbPropertyClean(dmbProperty *property)
{
    dmbNode *pos;
    dmbPropertyEntry *pEntry;
    dmbListClear(pEntry, pos, &property->list, node, destroyEntry);
    dmbListRemoveAll(&property->list);
    dmbDictRemoveAll(property->dict);
}

dmbCode dmbPropertyInsert(dmbProperty *pProperty, dmbString *pKey, dmbString *pValue)
{
    dmbPropertyEntry *pEntry = createEntry(pKey, pValue);
    if (pEntry == NULL)
        return DMB_ERRCODE_ALLOC_FAILED;

    dmbListPushBack(&pProperty->list, &pEntry->node);
    dmbDictPut(pProperty->dict, &pEntry->entry);

    return DMB_ERRCODE_OK;
}

dmbCode dmbPropertyLoad(dmbProperty *pProperty, const dmbCHAR *pcPath)
{
    if (!pProperty || !pcPath)
    {
        return DMB_ERRCODE_NULL_POINTER;
    }

    dmbCode iRet = DMB_ERRCODE_OK;
    struct stat fileStat;
    dmbBYTE* lpBuf;
    int fd = dmbSafeOpen(pcPath, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd == -1)
    {
        DMB_LOGD("open file %s failed\n", pcPath);
        return DMB_ERRCODE_FILE_OPEN_FAIL;
    }

    do {
        if (fstat(fd, &fileStat) != 0 || fileStat.st_size <= 0)
        {
            DMB_LOGD("read file %s size failed\n", pcPath);
            iRet = DMB_ERRCODE_FILE_READ_ERROR;
            break;
        }

        lpBuf = dmbMalloc(fileStat.st_size);
        if (lpBuf == NULL)
        {
            iRet = DMB_ERRCODE_ALLOC_FAILED;
            break;
        }

        do {
            if (dmbSafeRead(fd, lpBuf, fileStat.st_size) == -1)
            {
                DMB_LOGD("read file %s data failed\n", pcPath);
                iRet = DMB_ERRCODE_FILE_READ_ERROR;
                break;
            }

            iRet = Parse(pProperty, lpBuf, fileStat.st_size);
        } while (0);
        dmbFree(lpBuf);
    } while (0);

    dmbSafeClose(fd);
    return iRet;
}

dmbCode dmbPropertyGetString(dmbProperty *pProperty, const dmbCHAR *pcProp, dmbString **value)
{
    dmbDictEntry *pEntry = dmbDictGetByData(pProperty->dict, pcProp, dmb_strlen(pcProp));
    if (pEntry == NULL)
    {
        DMB_LOGR("Property %s not defined\n", pcProp);
        *value = NULL;
        return DMB_ERRCODE_NULL_POINTER;
    }

    *value = (dmbString*)pEntry->v.val;
    return DMB_ERRCODE_OK;
}

dmbCode dmbPropertyGetLong(dmbProperty *pProperty, const dmbCHAR *pcProp, dmbLONG *value)
{
    dmbString *str;
    dmbCode iRet = dmbPropertyGetString(pProperty, pcProp, &str);
    if (iRet != DMB_ERRCODE_OK)
        return iRet;

    iRet = dmbString2Long(dmbStringGet(str), value);
    if (iRet != DMB_ERRCODE_OK)
    {
        DMB_LOGR("String %s cannot tobe a number\n", dmbStringGet(str));
    }
    return iRet;
}

static dmbCode parseAnnotation(dmbBYTE* *lpBuf, dmbBYTE* lpEnd)
{
    dmbBYTE* cur = *lpBuf;

    do {
        if (*cur == '\n')
        {
            *lpBuf = cur;
            return DMB_ERRCODE_OK;
        }
    } while (cur++ != lpEnd);
    return DMB_ERROR;
}

static inline void trim(dmbBYTE* *lpBuf, dmbBYTE* lpEnd, const dmbCHAR *pcTrim)
{
    dmbINT i, trimSize = dmb_strlen(pcTrim);
    while (*lpBuf < lpEnd)
    {
        for (i=0; i<trimSize; ++i)
        {
            if (**lpBuf == pcTrim[i])
            {
                ++(*lpBuf);
                break;
            }
        }

        if (i == trimSize)
            break;
    }
}

static dmbCode parseString(dmbBYTE* *lpBuf, dmbBYTE* lpEnd, dmbString **str)
{
    dmbLONG len = 0;
    dmbBYTE* buf = *lpBuf;
    dmbString *tmp = NULL;
    while (buf < lpEnd)
    {
        if (*buf == '=' || *buf == '\t' || *buf == ' ' || *buf == '\n' || *buf == '\r')
        {
            if (len > 0)
            {
                tmp = dmbStringCreateWithBuffer((dmbCHAR*)*lpBuf, len);
                if (tmp == NULL)
                {
                    *str = tmp;
                    return DMB_ERRCODE_ALLOC_FAILED;
                }
                *str = tmp;
                *lpBuf = buf;
                return DMB_ERRCODE_OK;
            }
            DMB_LOGR("Parse error: property have an empty string.\n");
            return DMB_ERROR;
        }
        ++len;
        ++buf;
    }
    return DMB_ERROR;
}

static dmbCode parsePerperty(dmbProperty *pProperty, dmbBYTE* *lpBuf, dmbBYTE* lpEnd)
{
    dmbBYTE* cur = *lpBuf;
    dmbString *key = NULL, *value = NULL;
    dmbCode iRet = DMB_ERRCODE_OK;
    while (cur < lpEnd)
    {
        trim(&cur, lpEnd, " \t");

        iRet = parseString(&cur, lpEnd, &key);
        if (iRet != DMB_ERRCODE_OK)
            return iRet;

        trim(&cur, lpEnd, " \t");

        //check '='
        if (*cur != '=')
        {
            DMB_LOGD("property parse error! \'=\' not found!\n");
            dmbStringDestroy(key);
            return DMB_ERROR;
        }

        trim(&cur, lpEnd, " \t=");

        iRet = parseString(&cur, lpEnd, &value);
        if (iRet != DMB_ERRCODE_OK)
        {
            dmbStringDestroy(key);
            return iRet;
        }

        iRet = dmbPropertyInsert(pProperty, key, value);
        if (iRet != DMB_ERRCODE_OK)
        {
            dmbStringDestroy(key);
            dmbStringDestroy(value);
            return iRet;
        }
        *lpBuf = cur;
        return iRet;
    }
    return iRet;
}

static dmbCode Parse(dmbProperty *pProperty, dmbBYTE* lpBuf, size_t size)
{
    dmbBYTE* curBuf = lpBuf;
    dmbBYTE* endBuf = lpBuf + size;
    dmbCode iRet = DMB_ERRCODE_OK;
    while (curBuf < endBuf)
    {
        trim(&curBuf, endBuf, " \t\r\n");

        if (curBuf < endBuf && *curBuf == '#')
        {
            iRet = parseAnnotation(&curBuf, endBuf);
            if (iRet != DMB_ERRCODE_OK)
                return iRet;
            continue;
        }

        iRet = parsePerperty(pProperty, &curBuf, endBuf);
        if (iRet != DMB_ERRCODE_OK)
            return iRet;
    }
    return iRet;
}
