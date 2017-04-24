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

#ifndef DMBPROPERTY_H
#define DMBPROPERTY_H

#include "core/dmbstring.h"
#include "core/dmblist.h"
#include "core/dmbdict.h"

typedef struct dmbPropertyEntry {
    dmbNode node;
    dmbDictEntry entry;
}dmbPropertyEntry;

typedef struct dmbProperty {
    dmbList list;
    dmbDict *dict;
} dmbProperty;

dmbCode dmbPropertyLoad(dmbProperty *pProperty, const dmbCHAR *pcPath);

dmbCode dmbPropertyGetString(dmbProperty *pProperty, const dmbCHAR *pcProp, dmbString **value);

dmbCode dmbPropertyGetLong(dmbProperty *pProperty, const dmbCHAR *pcProp, dmbLONG *value);

dmbProperty* dmbPropertyCreate();

void dmbPropertyDestroy(dmbProperty *property);

void dmbPropertyClean(dmbProperty *property);

dmbCode dmbPropertyInsert(dmbProperty *property, dmbString *pKey, dmbString *pValue);

#endif // DMBPROPERTY_H
