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

void dmbDestroyIntObject(dmbObject *o)
{

}

void dmbDestroyStringObject(dmbObject *o)
{

}

void dmbDestroyListObject(dmbObject *o)
{

}

void dmbDestroySetObject(dmbObject *o)
{

}

void dmbDestroyZsetObject(dmbObject *o)
{

}

void dmbDestroyMapObject(dmbObject *o)
{

}
