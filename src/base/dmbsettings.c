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

#include "dmbsettings.h"
#include "utils/dmbsysutil.h"
#include "core/dmbstring.h"
#include "utils/dmbproperty.h"
#include "utils/dmblog.h"
#include "core/dmballoc.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define DEFAULT_CONF_PATH "dmb.conf"
#define UNIT_KB 1024
#define UNIT_MB UNIT_KB*1024
#define UNIT_GB UNIT_MB*1024

dmbLONG parseIntString(const dmbCHAR* pcStr);

#define PARSE_INT(root, obj, key)  \
    do { \
        dmbLONG value; \
        if (dmbPropertyGetLong(root, key, &value)==DMB_ERRCODE_OK) { \
            (obj) = value; \
        } \
    } while(0)

#define PARSE_STRING(root, obj, key) \
    do{ \
        dmbString *value; \
        if (dmbPropertyGetString(root, key, &value)==DMB_ERRCODE_OK) { \
            strcpy((obj), dmbStringGet(value)); \
        } \
    }while(0)

#define PARSE_INTSTRING(root, obj, key)  \
    do{ \
        dmbString *value; \
        if (dmbPropertyGetString(root, key, &value)==DMB_ERRCODE_OK) { \
            (obj) = parseIntString(dmbStringGet(value)); \
        } \
    }while(0);

dmbSettings g_settings;

void dmbResetDefaultSettings()
{
    g_settings.max_mem_size = 512*1024*1024;
    strcpy(g_settings.host, "0.0.0.0");
    g_settings.port = 12345;
    g_settings.listen_backlog = 3000;
    g_settings.net_rw_timeout = 15; //second
    g_settings.connect_size_per_thread = 3000;
    g_settings.net_read_bufsize = 4194304; //4MB
    g_settings.net_write_bufsize = 4194304; //4MB
    g_settings.thread_size = 10;
    g_settings.open_files = 1024;
}

dmbCode CheckConfig()
{
//    if (g_settings.key_max_size > 32767)
//        return DMB_ERROR;

    return DMB_OK;
}

dmbLONG parseIntString(const dmbCHAR* pcStr)
{
    dmbINT strLen = dmb_strlen(pcStr);
    if(!pcStr || !strLen)
        return 0;

    dmbLONG unit = 1;
    dmbCHAR ch;
    dmbINT i;

    for( i = strLen-1; i >= 0 ; i--)
    {
        ch = pcStr[i];
        //found number
        if (ch >= '0' && ch <= '9')
            break;

        if(ch == 'k' || ch == 'K')
        {
            unit = UNIT_KB;
            break;
        }
        else if(ch == 'm' || ch == 'M')
        {
            unit = UNIT_MB;
            break;
        }
        else if(ch == 'g' || ch == 'G')
        {
            unit = UNIT_GB;
            break;
        }
    }

    return strtol(pcStr, NULL, 10) * unit;
}

dmbCode dmbLoadSettings(const dmbCHAR *confPath)
{
    dmbCode code = DMB_ERRCODE_OK;
    if (confPath == NULL)
    {
        dmbCHAR pcPath[256];
        size_t pathSize = sizeof(pcPath);
        code = dmbGetAppPathByPid(getpid(), pcPath, pathSize);
        if (code != DMB_ERRCODE_OK)
            return code;

        if ((dmb_strlen(pcPath) + dmb_strlen(DEFAULT_CONF_PATH)) >= pathSize)
            return DMB_ERRCODE_OUT_OF_BUFF_BOUNDS;

        return dmbLoadSettings(strcat(pcPath, DEFAULT_CONF_PATH));
    }

    dmbResetDefaultSettings();

    dmbProperty *property = dmbPropertyCreate();
    if (property == NULL)
        return DMB_ERRCODE_ALLOC_FAILED;

    code = dmbPropertyLoad(property, confPath);
    if(code != DMB_ERRCODE_OK)
    {
        DMB_LOGR("Load config file error\n");
        return code;
    }

    PARSE_INTSTRING(property, g_settings.max_mem_size, "max_mem_size");
    PARSE_STRING(property, g_settings.host, "host");
    PARSE_INT(property, g_settings.port, "port");
    PARSE_INT(property, g_settings.listen_backlog, "listen_backlog");
    PARSE_INT(property, g_settings.open_files, "open_files");
    PARSE_INT(property, g_settings.thread_size, "thread_size");
    PARSE_INT(property, g_settings.connect_size_per_thread, "connect_size_per_thread");
    PARSE_INT(property, g_settings.net_rw_timeout, "net_rw_timeout");
    PARSE_INTSTRING(property, g_settings.net_read_bufsize, "net_read_bufsize");
    PARSE_INTSTRING(property, g_settings.net_write_bufsize, "net_write_bufsize");

    dmbSetMaxMemSize((size_t) g_settings.max_mem_size);

    if (property != NULL)
        dmbPropertyDestroy(property);

    return CheckConfig();
}
