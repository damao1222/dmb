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

#include "dmbioutil.h"

inline ssize_t dmbSafeRead(int fd, dmbBYTE *pBuf, ssize_t count)
{
    int err;
    ssize_t ret = -1, cur = 0;

    while (cur != count)
    {
        ret = read(fd, pBuf + cur, count - cur);
        if (ret == -1)
        {
            err = errno;
            if (err == EINTR)
                continue;
            else if (err == EAGAIN || err == EWOULDBLOCK)
                return DMB_IO_AGAIN;
            else
                return DMB_IO_ERROR;
        }
        else if (ret == 0)
            break;

        cur += ret;
    }

    return cur;
}

inline ssize_t dmbSafeWrite(int fd, dmbBYTE *pBuf, ssize_t count)
{
    int err;
    ssize_t ret = -1, cur = 0;

    while (cur != count)
    {
        ret = write(fd, pBuf + cur, count - cur);
        if (ret == -1)
        {
            err = errno;
            if (err == EINTR)
                continue;
            else if (err == EAGAIN || err == EWOULDBLOCK)
                return DMB_IO_AGAIN;
            else
                return DMB_IO_ERROR;
        }
        else if (ret == 0)
            break;

        cur += ret;
    }

    return cur;
}

inline ssize_t dmbReadAvailable(int fd, dmbBYTE *pBuf, ssize_t count)
{
    int err;
    ssize_t ret = -1, cur = 0;

    while (cur == 0)
    {
        ret = read(fd, pBuf + cur, count - cur);
        if (ret == -1)
        {
            err = errno;
            if (err == EINTR)
                continue;
            else if (err == EAGAIN || err == EWOULDBLOCK)
                return DMB_IO_AGAIN;
            else
                return DMB_IO_ERROR;
        }
        else if (ret == 0)
        {
            return DMB_IO_END;
        }

        cur += ret;
    }

    return cur;
}

inline ssize_t dmbWriteAvailable(int fd, dmbBYTE *pBuf, ssize_t count)
{
    int err;
    ssize_t ret = -1, cur = 0;

    while (cur == 0)
    {
        ret = write(fd, pBuf + cur, count - cur);
        if (ret == -1)
        {
            err = errno;
            if (err == EINTR)
                continue;
            else if (err == EAGAIN || err == EWOULDBLOCK)
                return DMB_IO_AGAIN;
            else
                return DMB_IO_ERROR;
        }

        cur += ret;
    }

    return cur;
}
