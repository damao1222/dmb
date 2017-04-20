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

#ifndef DMBCOMMAND_H
#define DMBCOMMAND_H

#include "dmbdefines.h"
#include "core/dmbbinlist.h"
#include "dmbobject.h"

#define DMB_CMD_READ 1
#define DMB_CMD_WRITE 2

typedef struct {
    dmbCode (*exec) (dmbBinlist *pParam, dmbObject **pObject);
    dmbCode (*undo) (dmbBinlist *pParam);
    dmbBinlist* (*reverse) (dmbBinlist *pParam);
    dmbBOOL (*canUndo) ();
    dmbUINT (*getFlag)();
} dmbCommand;

dmbUINT dmbCmdCanRead();
dmbUINT dmbCmdCanWrite();
dmbBOOL dmbCmdCanUndo();
dmbBOOL dmbCmdCannotUndo();

#endif // DMBCOMMAND_H
