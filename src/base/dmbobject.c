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

#include "dmbobject.h"
#include "utils/dmblog.h"
#include "thread/dmbatomic.h"
#include "core/dmballoc.h"
#include "core/dmbstring.h"

static dmbBOOL checkType(dmbObject *pObj)
{
    if (pObj->type > DMB_OBJ_TYPE_BEGIN && pObj->type < DMB_OBJ_TYPE_END)
        return TRUE;
    return FALSE;
}

dmbBOOL dmbObjectRetain(dmbObject *o)
{
    dmbLONG ret;
    DMB_ASSERT_X(checkType(o),"Unsupport Type, it maybe not a Object\n");

    ret = dmbAtomicIncr(&o->ref);
    DMB_ASSERT_X(ret>1, "dmbObject Reference count is < 1, it must have been freed at another place");

    return TRUE;
}

dmbBOOL dmbObjectRelease(dmbObject *pObj)
{
    DMB_ASSERT_X(checkType(pObj),"Unsupport Type, it maybe not a Object\n");

    DMB_ASSERT_X(dmbAtomicAdd(&pObj->ref ,0)>=1,"dmbObject Reference count is < 1, it must have been freed at another place\n");

    if (!dmbAtomicDecr(&pObj->ref))
    {
        switch (pObj->type)
        {
        case DMB_OBJ_TYPE_INT:
            dmbDestroyIntObject(pObj);
            break;
        case DMB_OBJ_TYPE_STRING:
            dmbDestroyStringObject(pObj);
            break;
        case DMB_OBJ_TYPE_LIST:
            dmbDestroyListObject(pObj);
            break;
        case DMB_OBJ_TYPE_SET:
            dmbDestroySetObject(pObj);
            break;
        case DMB_OBJ_TYPE_ZSET:
            dmbDestroyZsetObject(pObj);
            break;
        case DMB_OBJ_TYPE_MAP:
            dmbDestroyMapObject(pObj);
            break;
        default:
            DMB_LOGE("Unsupported Type %d\n", pObj->type);
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

dmbObject* dmbCreateIntObject(dmbLONG lValue)
{
    dmbObject *o = (dmbObject*)dmbMalloc(sizeof(dmbObject));
    if (o != NULL)
    {
        o->type = DMB_OBJ_TYPE_INT;
        o->encode = DMB_OBJ_ENCODE_INT;
        o->ref = 1;
        o->num = lValue;
    }
    return o;
}

dmbObject* dmbCreateStringObject(dmbCHAR *pcStr, dmbUINT uLen)
{
    dmbObject *o = (dmbObject*)dmbMalloc(sizeof(dmbObject));
    if (o != NULL)
    {
        o->ptr = dmbStringCreateWithBuffer(pcStr, uLen);
        if (o->ptr == NULL)
        {
            dmbFree(o);
            return NULL;
        }

        o->type = DMB_OBJ_TYPE_STRING;
        o->encode = DMB_OBJ_ENCODE_STRING;
        o->ref = 1;
    }
    return o;
}

void dmbDestroyIntObject(dmbObject *o)
{
    dmbFree(o);
}

void dmbDestroyStringObject(dmbObject *o)
{
    if (o->encode == DMB_OBJ_ENCODE_STRING)
        dmbStringDestroy((dmbString*)o->ptr);
    dmbFree(o);
}

void dmbDestroyListObject(dmbObject *o)
{
    DMB_UNUSED(o);
}

void dmbDestroySetObject(dmbObject *o)
{
    DMB_UNUSED(o);
}

void dmbDestroyZsetObject(dmbObject *o)
{
    DMB_UNUSED(o);
}

void dmbDestroyMapObject(dmbObject *o)
{
    DMB_UNUSED(o);
}
