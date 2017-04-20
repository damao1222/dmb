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

#ifndef DMBOBJECT_H
#define DMBOBJECT_H

#include "dmbdefines.h"

#define DMB_OBJ_TYPE_BEGIN              0//起始

#define DMB_OBJ_TYPE_INT                1//整形
#define DMB_OBJ_TYPE_STRING             2//字串
#define DMB_OBJ_TYPE_SET                3//集合
#define DMB_OBJ_TYPE_ZSET               4//有序集
#define DMB_OBJ_TYPE_LIST               5//链表
#define DMB_OBJ_TYPE_MAP                6//字典

#define DMB_OBJ_TYPE_END                 7//终止

typedef struct {
    dmbRef ref;
    dmbUINT32 type : 4;
    dmbUINT32 encode : 28;
    union {
        volatile void *ptr;
        volatile dmbLONG num;
    };
} dmbObject;

dmbBOOL dmbObjectRetain(dmbObject *o);
dmbBOOL dmbObjectRelease(dmbObject *o);

void dmbDestroyIntObject(dmbObject *o);
void dmbDestroyStringObject(dmbObject *o);
void dmbDestroyListObject(dmbObject *o);
void dmbDestroySetObject(dmbObject *o);
void dmbDestroyZsetObject(dmbObject *o);
void dmbDestroyMapObject(dmbObject *o);

#endif // DMBOBJECT_H
