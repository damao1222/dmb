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

#include "dmbstring.h"
#include "dmballoc.h"
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#if defined(DMB_USE_FAST_STRSTR)
#include "fast_strstr.h"
#define dmb_strstr(h,n)	fast_strstr((h),(n))
#elif defined(DMB_USE_SUNDAY_STRSTR)
#define dmb_strstr(h,n)	sunday_strstr((h),(n))
#else
#define dmb_strstr(h,n)	strstr((h),(n))
#endif

static dmbCode insert(dmbString *pDestStr, const dmbCHAR *pStr, dmbUINT uPos, dmbUINT uLen) ;
static dmbINT indexOfFrom(const dmbCHAR *pSrcStr, dmbUINT uSrcLen, const dmbCHAR *pStr, dmbUINT uLen, dmbUINT uFrom)
{
    DMB_UNUSED(uLen);
    if (uSrcLen <= uFrom)
        return -1;

    dmbCHAR *p = dmb_strstr(pSrcStr + uFrom, pStr);
    if (p == NULL)
        return -1;

    return (dmbINT)(p - pSrcStr);
}

static dmbINT lastIndexOfFrom(const dmbCHAR *pSrcStr, dmbUINT uSrcLen, const dmbCHAR *pStr, dmbUINT uLen, dmbUINT uFrom)
{
    if (uSrcLen <= uFrom)
        return -1;

    dmbCHAR* pPos = (dmbCHAR*)pSrcStr + uFrom, *p;
    dmbINT iIndex = -1;

    do {
        p = dmb_strstr(pPos, pStr);
        if (p == NULL)
            break;

        iIndex = p - pSrcStr;
        pPos = p + uLen;
    } while(pPos < (pSrcStr + uSrcLen - 1));

    return iIndex;
}

dmbString* dmbStringCreate(dmbUINT uLen)
{
    dmbString* p = (dmbString*)dmbMalloc(sizeof(dmbString) + (size_t)uLen);
    if (p == NULL)
        return p;

    p->capacity = uLen;
    p->len = 0;
    dmbMemSet(p->data, 0, p->capacity);

    return p;
}

dmbString* dmbStringCreateWithBuffer(const dmbCHAR* pcBuf, dmbUINT uLen)
{
    dmbString *pStr = dmbStringCreate(uLen);
    if (pStr != NULL)
    {
        dmbMemCopy(pStr->data, pcBuf, uLen);
    }
    return pStr;
}

void dmbStringDestroy(dmbString *pStr)
{
    dmbFree(pStr);
}

void dmbStringClear(dmbString *pStr)
{
    dmbStringErase(pStr, 0, dmbStringLength(pStr));
}

dmbUINT dmbStringErase(dmbString *pStr, dmbUINT uPos, dmbUINT uLen)
{
    dmbUINT uOldLen = dmbStringLength(pStr);
    if (pStr == NULL || uLen == 0 || uPos >= uOldLen)
        return 0;

    dmbMemMove(pStr->data + uPos, pStr->data + uPos + uLen, (uOldLen - uLen - uPos) * sizeof(dmbCHAR));
    dmbMemSet(pStr->data + uOldLen - uLen, 0, uLen * sizeof(dmbCHAR));
    pStr->len -= uLen;

    return uLen;
}

dmbUINT dmbStringLength(dmbString *pStr)
{
    return pStr->len;
}

dmbBOOL dmbStringIsEmpty(dmbString *pStr)
{
    return pStr->len == 0;
}

dmbBOOL dmbStringUpper(dmbString *pStr)
{
    dmbUINT length = dmbStringLength(pStr), i = 0;

    for (i=0; i<length; i++)
    {
        pStr->data[i] = toupper(pStr->data[i]);
    }

    return TRUE;
}

dmbBOOL dmbStringLower(dmbString *pStr)
{
    dmbUINT length = dmbStringLength(pStr), i = 0;

    for (i=0; i<length; i++)
    {
        pStr->data[i] = tolower(pStr->data[i]);
    }

    return TRUE;
}

const dmbCHAR* dmbStringGet(const dmbString *pStr)
{
    return pStr->data;
}

void dmbStringGetData(const dmbString *pStr, const dmbCHAR **pData, dmbUINT *pLen)
{
    if (NULL != pData)
        *pData = pStr->data;
    if (NULL != pLen)
        *pLen = pStr->len;
}

dmbINT dmbStringIndexOf(const dmbString *pSrcStr, const dmbCHAR *pStr, dmbUINT uStrLen)
{
    const dmbCHAR *p;
    dmbUINT u;
    dmbStringGetData(pSrcStr, &p, &u);
    return indexOfFrom(p, u, pStr, uStrLen, 0);
}

dmbINT dmbStringLastIndexOf(const dmbString *pSrcStr, const dmbCHAR *pStr, dmbUINT uStrLen)
{
    const dmbCHAR *p;
    dmbUINT u;
    dmbStringGetData(pSrcStr, &p, &u);
    return lastIndexOfFrom(p, u, pStr, uStrLen, 0);
}

static inline dmbINT compareUint(dmbUINT u1, dmbUINT u2)
{
    return (dmbINT)((dmbSIZE)u1 - (dmbSIZE)u2);
}

dmbBOOL dmbStringCompare(const dmbString *pStr, const dmbCHAR *pcStr, dmbUINT uLen)
{
    const dmbCHAR *p;
    dmbUINT u;
    dmbStringGetData(pStr, &p, &u);

    dmbINT iRet = dmbMemCmp(p, pcStr, u);
    if (iRet == 0)
        return compareUint(u, uLen);
    return iRet;
}

dmbCode dmbStringAppend(dmbString *pDestStr, const dmbCHAR *pStr, dmbUINT uLen)
{
    return insert(pDestStr, pStr, dmbStringLength(pDestStr), uLen);
}

dmbCode dmbStringPrepend(dmbString *pDestStr, const dmbCHAR *pStr, dmbUINT uLen)
{
    return insert(pDestStr, pStr, 0, uLen);
}

dmbCode dmbString2Long(const dmbCHAR *pStr, dmbLONG *pValue)
{
    char *eptr;
    errno = 0;
    dmbLONG value = strtol(pStr, &eptr, 10);
    if (isspace(pStr[0]) || eptr[0] != '\0' ||
        errno == ERANGE)
        return DMB_ERRCODE_CONVERT_TYPE_ERROR;
    *pValue = value;
    return DMB_ERRCODE_OK;
}

static const dmbCHAR DIGIT_TENS[] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1',
    '1', '1', '1', '1', '1', '1', '1', '1', '1', '2', '2', '2', '2', '2', '2', '2', '2', '2',
    '2', '3', '3', '3', '3', '3', '3', '3', '3', '3', '3', '4', '4', '4', '4', '4', '4', '4',
    '4', '4', '4', '5', '5', '5', '5', '5', '5', '5', '5', '5', '5', '6', '6', '6', '6', '6',
    '6', '6', '6', '6', '6', '7', '7', '7', '7', '7', '7', '7', '7', '7', '7', '8', '8', '8',
    '8', '8', '8', '8', '8', '8', '8', '9', '9', '9', '9', '9', '9', '9', '9', '9', '9', };

static const dmbCHAR DIGIT_ONES[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', };

static const dmbCHAR DIGIT[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a',
    'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z' };

static void Long2RevStr(dmbLONG value, dmbCHAR *pcBuf, dmbINT *count)
{
    dmbCHAR sign = 0;
    dmbLONG q, r;
    dmbINT pos = 0;

    if (value < 0)
    {
        sign = '-';
        value = -value;
    }

    while (value >= 65536)
    {
        q = value / 100;
        r = value - ((q << 6) + (q << 5) + (q << 2));
        value = q;
        pcBuf[pos++] = DIGIT_ONES[r];
        pcBuf[pos++] = DIGIT_TENS[r];
    }

    do
    {
        q = (value * 52429) >> (19);
        r = value - ((q << 3) + (q << 1));
        pcBuf[pos++] = DIGIT[r];
        value = q;
    } while (value != 0);

    if (sign != 0)
    {
        pcBuf[pos++] = sign;
    }

    *count = pos;
}

dmbCode dmbLong2Str(dmbLONG value, dmbCHAR *pcBuf, dmbUINT *uSize)
{
    dmbCHAR buf[256] = {0};
    dmbINT cur = 0, pos, count;

    Long2RevStr(value, buf, &count);

    pos = count;

    // exclude '\0'
    if (*uSize < (size_t)count)
        return DMB_ERRCODE_OUT_OF_BUFF_BOUNDS;

    while (cur < count)
    {
        pcBuf[cur++] = buf[--pos];
    }

    *uSize = count;

    return DMB_ERRCODE_OK;
}

dmbLONG dmbGetDigit10(dmbLONG value)
{
    dmbLONG lDigit = 0;

    if (value == 0)
        return 1;

    for (; value != 0; ++lDigit)
        value /= 10;

    return lDigit;
}

dmbLONG dmbGetStrLenWithLong(dmbLONG value)
{
    return dmbGetDigit10(value) + (value < 0 ? 1 : 0);
}

static dmbUINT allocMore(dmbUINT alloc)
{
    const dmbUINT page = 1 << 12;
    dmbUINT nalloc;
    if (alloc < 1<<8) {
        nalloc = (1<<4) + ((alloc >>4) << 4);
    } else  {
        // don't do anything if the loop will overflow signed int.
        if (alloc >= UINT_MAX/2)
            return UINT_MAX;
        nalloc = (alloc < page) ? 1 << 4 : page;
        while (nalloc < alloc) {
            if (nalloc <= 0)
                return UINT_MAX;
            nalloc *= 2;
        }
    }
    return nalloc;
}

static dmbUINT calcCapacity(dmbUINT size)
{
    volatile dmbUINT x = allocMore(size);
    return x;
}

dmbString* reallocString(dmbString *pDestStr, dmbUINT uSize)
{
    pDestStr = (dmbString*)dmbRealloc(pDestStr, sizeof(dmbString) + uSize);
    if (pDestStr == NULL)
        return pDestStr;

    if (uSize > pDestStr->capacity)
    {
        dmbMemSet(pDestStr->data + pDestStr->capacity, 0, sizeof(dmbCHAR) * (uSize - pDestStr->capacity));
    }
    pDestStr->capacity = uSize;


    return pDestStr;
}

static dmbCode insert(dmbString *pDestStr, const dmbCHAR *pStr, dmbUINT uPos, dmbUINT uLen) {
    dmbUINT uOldLen = dmbStringLength(pDestStr);
    if (uPos > uOldLen || uLen == 0)
        return DMB_ERRCODE_WRONG_ARGUMENT_VALUE;

    dmbUINT uReserveSize = uOldLen + uLen;

    if (pDestStr->capacity < uReserveSize)
    {
        dmbString *pData = reallocString(pDestStr, calcCapacity(uReserveSize));
        if (pData == NULL)
            return DMB_ERRCODE_ALLOC_FAILED;
    }

    if (uPos != uOldLen)
        dmbMemMove(pDestStr->data + uPos + uLen, pDestStr->data + uPos, (uOldLen - uPos) * sizeof(dmbCHAR));
    dmbMemCopy(pDestStr->data + uPos, pStr, uLen * sizeof(dmbCHAR));

    pDestStr->len = uOldLen + uLen;

    return DMB_ERRCODE_OK;
}
