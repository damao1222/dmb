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

#ifndef DMBSYSUTIL_H
#define DMBSYSUTIL_H

#include "dmbdefines.h"

extern dmbBOOL DMB_BIGENDIAN;

dmbBOOL dmbSystemInit();

void dmbRev16(void *p);
void dmbRev32(void *p);
void dmbRev64(void *p);
dmbINT16 dmbRevInt16(dmbINT16 i);
dmbINT32 dmbRevint32(dmbINT32 i);
dmbINT64 dmbRevint64(dmbINT64 i);

void dmbInt16ToByte(dmbBYTE *p, dmbINT16 i);
void dmbInt32ToByte(dmbBYTE *p, dmbINT32 i);
void dmbInt64ToByte(dmbBYTE *p, dmbINT64 i);

dmbCode dmbGetAppPathByPid(pid_t pid, dmbCHAR *pcBuf, dmbUINT uSize);

#endif // DMBSYSUTIL_H
