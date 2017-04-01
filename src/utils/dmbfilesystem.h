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

#ifndef DMBFILESYSTEM_H
#define DMBFILESYSTEM_H

#include "dmbdefines.h"

typedef enum {
    dmbTypeUnknown = 0,
    dmbTypeFile  = 1,//file
    dmbTypeDir = 2//dir
} dmbFileType;

/**
 * @brief 判断目录是否存在
 * @param 目录路径
 * @return 存在返回TRUE，不存在返回FALSE
 */
dmbBOOL dmbIsDirExists(const dmbCHAR *pcDir);

/**
 * @brief dmbIsFileExists 判断文件是否存在
 * @param pcFile 文件路径
 * @return 存在返回TRUE，不存在返回FALSE
 */
dmbBOOL dmbIsFileExists(const dmbCHAR *pcFile);

/**
 * @brief 创建一个目录（不逐级创建）
 * @return 成功返回TRUE，否则返回FALSE
 */
dmbBOOL dmbMkdir(const dmbCHAR *pcDir);

#endif // DMBFILESYSTEM_H
