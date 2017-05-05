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

#ifndef DMBPROTOCOL_H
#define DMBPROTOCOL_H

#include "dmbnetwork.h"

#define DMB_MAGIC_NUMBER 0x1222

#ifndef DMB_VERSION
#define DMB_VERSION 1
#endif

typedef struct dmbRequest {
    dmbINT16 magicNum;
    dmbINT16 version;
    dmbINT16 multiPkg:1;
    dmbINT16 multiEnd:1;
    dmbINT16 reserve:14;
    dmbUINT16 cmd;
    dmbUINT32 length;
    dmbBYTE data[0];
} dmbRequest;

typedef struct dmbResponse {
    dmbINT16 magicNum;
    dmbINT16 version;
    dmbINT16 multiPkg:1;
    dmbINT16 reserve:15;
    dmbINT16 status;
    dmbUINT32 length;
    dmbBYTE data[0];
} dmbResponse;

enum {
    dmbRequestHeaderSize = sizeof(dmbRequest),
    dmbResponseHeaderSize = sizeof(dmbResponse)
};

#define DMB_INVALID_CMD 0xFFFF

void dmbProcessEvent(dmbNetworkContext *pCtx, dmbConnect *pConn);

#endif // DMBPROTOCOL_H
