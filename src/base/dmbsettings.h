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

#ifndef DMBSETTINGS_H
#define DMBSETTINGS_H

#include "dmbdefines.h"

typedef struct dmbSettings{
    dmbCHAR host[16];
    dmbINT port;
    dmbINT listen_backlog;
    dmbINT open_files;
    dmbINT thread_size;
    dmbUINT net_read_bufsize;
    dmbUINT net_write_bufsize;
} dmbSettings;

void dmbResetDefaultSettings();

dmbCode dmbLoadSettings(const dmbCHAR *confPath);

extern dmbSettings g_settings;

#endif // DMBSETTINGS_H
