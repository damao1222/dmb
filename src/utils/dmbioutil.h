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

#ifndef DMBIOUTIL_H
#define DMBIOUTIL_H

#include "dmbdefines.h"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define DMB_IO_END 0
#define DMB_IO_AGAIN -1
#define DMB_IO_ERROR -2

ssize_t dmbSafeRead(int fd, dmbBYTE *pBuf, ssize_t count);

ssize_t dmbSafeWrite(int fd, dmbBYTE *pBuf, ssize_t count);

ssize_t dmbReadAvailable(int fd, dmbBYTE *pBuf, ssize_t count);

ssize_t dmbWriteAvailable(int fd, dmbBYTE *pBuf, ssize_t count);

#define EINTR_LOOP(var, cmd)                    \
    do {                                        \
        var = cmd;                              \
    } while (var == -1 && errno == EINTR)

static inline int dmbSafeOpen(const char *path, int flags, mode_t mode)
{
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
    register int fd;
    EINTR_LOOP(fd, open(path, flags, mode));

    if (fd != -1)
        fcntl(fd, F_SETFD, FD_CLOEXEC);
    return fd;
}

static inline int dmbSafeClose(int fd)
{
    register int ret;
    EINTR_LOOP(ret, close(fd));
    return ret;
}

#endif // DMBIOUTIL_H
