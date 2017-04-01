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

#include "dmbfilesystem.h"
#include <dirent.h>
#include <sys/stat.h>

dmbBOOL dmbIsDirExists(const dmbCHAR *pcDir)
{
    if (pcDir == NULL)
        return FALSE;

    DIR *tmpDir = opendir(pcDir);
    if (tmpDir)
    {
        closedir(tmpDir);
        return TRUE;
    }
    return FALSE;
}

dmbBOOL dmbIsFileExists(const dmbCHAR *pcFile)
{
    struct stat buffer;
    return (stat(pcFile, &buffer)==0);
}

dmbBOOL dmbMkdir(const dmbCHAR *pcDir)
{
    return mkdir(pcDir, 0777) == 0;
}
