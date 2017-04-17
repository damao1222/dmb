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

#ifndef DMBSTRING_H
#define DMBSTRING_H

#include "dmbdefines.h"

typedef struct {
    dmbUINT len;
    dmbUINT capacity;
    dmbCHAR data[0];
} dmbString;

dmbString* dmbStringCreate(dmbUINT uLen);

dmbString* dmbStringCreateWithBuffer(const dmbCHAR* pcBuf, dmbUINT uLen);

void dmbStringDestroy(dmbString *pStr);

void dmbStringClear(dmbString *pStr);

dmbUINT dmbStringErase(dmbString *pStr, dmbUINT uPos, dmbUINT uLen);

dmbUINT dmbStringLength(dmbString *pStr);

dmbBOOL dmbStringIsEmpty(dmbString *pStr);

dmbBOOL dmbStringUpper(dmbString *pStr);

dmbBOOL dmbStringLower(dmbString *pStr);

const dmbCHAR* dmbStringGet(const dmbString *pStr);

void dmbStringGetData(const dmbString *pStr, const dmbCHAR **pData, dmbUINT *pLen);

dmbINT dmbStringIndexOf(const dmbString *pSrcStr, const dmbCHAR *pStr, dmbUINT uStrLen);

dmbINT dmbStringLastIndexOf(const dmbString *pSrcStr, const dmbCHAR *pStr, dmbUINT uStrLen);

dmbBOOL dmbStringCompare(const dmbString *pStr, const dmbCHAR *pcStr, dmbUINT uLen);

dmbCode dmbStringAppend(dmbString *pDestStr, const dmbCHAR *pStr, dmbUINT uLen);

dmbCode dmbStringPrepend(dmbString *pDestStr, const dmbCHAR *pStr, dmbUINT uLen);

dmbCode dmbString2Long(const dmbCHAR *pStr, dmbLONG *pValue);

//不包含'\0'
dmbCode dmbLong2Str(dmbLONG value, dmbCHAR *pcBuf, dmbUINT *uSize);

dmbLONG dmbGetDigit10(dmbLONG value);

dmbLONG dmbGetStrLenWithLong(dmbLONG value);
#endif // DMBSTRING_H
