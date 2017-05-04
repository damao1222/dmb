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

#include "dmbprotocol.h"
#include "utils/dmbioutil.h"
#include "base/dmbsettings.h"
#include "utils/dmblog.h"
#include "core/dmballoc.h"

static dmbCode writeData(dmbNetworkContext *pCtx, dmbNetworkEvent *pEvent, dmbConnect *pConn);
dmbCode dmbProcessPackage(dmbConnect *pConn, dmbINT16 iCmd, dmbBYTE *pData, dmbUINT uSize);

static dmbCode parsePkgHeader(dmbBYTE *pBuf, dmbRequest **pDestRequest)
{
    dmbRequest *pReq = (dmbRequest*)pBuf;

    if (pReq->magicNum != DMB_MAGIC_NUMBER)
        return DMB_ERRCODE_PROTOCOL_ERROR;

    if (pReq->version != DMB_VERSION)
        return DMB_ERRCODE_VERSION_ERROR;

    *pDestRequest = pReq;

    return DMB_ERRCODE_OK;
}

void dmbMakeErrorResponse(dmbConnect *pConn, dmbCode code)
{
    dmbResponse *pResp = (dmbResponse*) pConn->writeBuf + pConn->writeIndex;
    pResp->magicNum = DMB_MAGIC_NUMBER;
    pResp->version = DMB_VERSION;
    pResp->status = code;

    pResp->length = 0;
    pConn->writeLength += dmbResponseHeaderSize;
}

void dmbMakeErrorResponseWithData(dmbConnect *pConn, dmbCode code, dmbBYTE *pData, dmbUINT uSize)
{
    dmbResponse *pResp = (dmbResponse*) pConn->writeBuf + pConn->writeIndex;
    pResp->magicNum = DMB_MAGIC_NUMBER;
    pResp->version = DMB_VERSION;
    pResp->status = code;

    if (dmbResponseHeaderSize + uSize > g_settings.net_write_bufsize)
    {
        pResp->status = DMB_ERRCODE_OUT_OF_WRITEBUF;
        pResp->length = 0;
        pConn->writeLength += dmbResponseHeaderSize;
        return ;
    }
    pResp->length = uSize;
    pConn->writeLength += dmbResponseHeaderSize + uSize;
    dmbMemCopy(pConn->writeBuf+pConn->writeIndex, pData, uSize);
}

dmbCode dmbProcessEvent(dmbNetworkContext *pCtx, dmbNetworkEvent *pEvent, dmbConnect *pConn)
{
    dmbCode code = DMB_ERRCODE_OK;
    ssize_t ret = 0;
    dmbBOOL bResetTimeout = FALSE;
    while (TRUE)
    {
        if (dmbNetworkCanRead(pEvent))
        {
            do {
                ret = dmbReadAvailable(pConn->cliFd, pConn->readBuf + pConn->readIndex, pConn->readBufSize - pConn->readIndex);
                if (ret == DMB_IO_AGAIN)
                {
                    DMB_LOGD("Read again\n");
                    bResetTimeout = TRUE;
                    code = DMB_ERRCODE_NETWORK_AGAIN;
                    break;
                }
                else if (ret == DMB_IO_ERROR)
                {
                    DMB_LOGD("Read error\n");
                    dmbNetworkCloseConnect(pCtx, pConn);
                    code = DMB_ERRCODE_NETWORK_ERROR;
                    break;
                }
                else if (ret == DMB_IO_END)
                {
                    DMB_LOGD("Read end\n");
                    dmbNetworkCloseConnect(pCtx, pConn);
                    break;
                }
                else
                {
                    dmbRequest *pRequest;
                    pConn->readIndex += ret;
                    pConn->readLength += ret;

                    if (pConn->readLength < dmbRequestHeaderSize)
                    {
                        continue;
                    }
ProcessHeader:
                    code = parsePkgHeader(pConn->readBuf + pConn->requestIndex, &pRequest);
                    if (code != DMB_ERRCODE_OK)
                    {
                        if (pRequest->length + dmbResponseHeaderSize > g_settings.net_read_bufsize)
                        {
                            code = DMB_ERRCODE_OUT_OF_READBUF;
                            dmbMakeErrorResponseWithData(pConn, code, g_settings.net_read_bufsize, sizeof(g_settings.net_read_bufsize));
                        }
                        else
                        {
                            dmbMakeErrorResponse(pConn, code);
                        }
                        break;
                    }

                    if (pRequest->length > pConn->readLength - dmbResponseHeaderSize)
                    {
                        continue;
                    }
                    else
                    {
                        if (pRequest->multiPkg)
                        {
                            code = pConn->request.merge(&pConn->request, pConn->readBuf + pConn->requestIndex, pRequest->length);
                            if (code != DMB_ERRCODE_OK)
                            {
                                code = DMB_ERRCODE_MERGE_PKG_FAILED;
                                dmbMakeErrorResponse(pConn, code);
                                pConn->request.release(&pConn->request);
                                break;
                            }

                            if (pRequest->multiEnd)
                            {
                                code = dmbProcessPackage(pConn, pRequest->cmd, pConn->request.data, pConn->request.len);
                            }
                        }
                        else
                        {
                            code = dmbProcessPackage(pConn, pRequest->cmd, pConn->readBuf + pConn->requestIndex + dmbResponseHeaderSize, pRequest->length);
                        }

                        pConn->readLength -= (dmbResponseHeaderSize + pRequest->length);
                        pConn->readIndex -= pConn->readLength;
                        if (pConn->readLength > 0)
                        {
                            dmbMemMove(pConn->readBuf + pConn->requestIndex,
                                       pConn->readBuf + pConn->requestIndex + dmbResponseHeaderSize + pRequest->length,
                                       pConn->readLength);
                        }
                    }
                }
            } while (0);
        }

        if (dmbNetworkCanWrite(pEvent))
        {
            while (TRUE)
            {
                ret = dmbWriteAvailable(pConn->cliFd, pConn->writeBuf + pConn->writeIndex, pConn->writeLength);
                if (ret == DMB_IO_AGAIN)
                {
                    DMB_LOGD("Write again\n");
                    code = DMB_ERRCODE_NETWORK_AGAIN;
                    break;
                }
                else if (ret == DMB_IO_ERROR)
                {
                    DMB_LOGD("Write error\n");
                    dmbNetworkCloseConnect(pCtx, pConn);
                    code = DMB_ERRCODE_NETWORK_ERROR;
                    break;
                }
                else if (ret == DMB_IO_END)
                {
                    DMB_LOGD("Write end\n");
                    dmbNetworkCloseConnect(pCtx, pConn);
                    break;
                }
                else
                {
                    pConn->writeIndex += ret;
                    pConn->writeLength -= ret;
                    if (pConn->writeLength > 0)
                    {
                        continue;
                    }
                    else
                    {
                        pConn->writeIndex = 0;
                        return DMB_ERRCODE_OK;
                    }
                }
            }
        }
    }

    if (bResetTimeout)
    {
        dmbNetworkWatchTimeout(pCtx, pConn, g_settings.net_rw_timeout);
    }

    return code;
}

dmbCode dmbProcessPackage(dmbConnect *pConn, dmbINT16 iCmd, dmbBYTE *pData, dmbUINT uSize)
{

}

static dmbCode writeData(dmbNetworkContext *pCtx, dmbNetworkEvent *pEvent, dmbConnect *pConn)
{
    dmbCode code = DMB_ERRCODE_OK;
    ssize_t ret = 0;
    if (dmbNetworkCanWrite(pEvent))
    {
        while (TRUE)
        {
            ret = dmbWriteAvailable(pConn->cliFd, pConn->writeBuf + pConn->writeIndex, pConn->writeLength);
            if (ret == DMB_IO_AGAIN)
            {
                DMB_LOGD("Write again\n");
                code = DMB_ERRCODE_NETWORK_AGAIN;
                break;
            }
            else if (ret == DMB_IO_ERROR)
            {
                DMB_LOGD("Write error\n");
                dmbNetworkCloseConnect(pCtx, pConn);
                code = DMB_ERRCODE_NETWORK_ERROR;
                break;
            }
            else if (ret == DMB_IO_END)
            {
                DMB_LOGD("Write end\n");
                dmbNetworkCloseConnect(pCtx, pConn);
                break;
            }
            else
            {
                pConn->writeIndex += ret;
                pConn->writeLength -= ret;
                if (pConn->writeLength > 0)
                {
                    continue;
                }
                else
                {
                    pConn->writeIndex = 0;
                    return DMB_ERRCODE_OK;
                }
            }
        }
    }
    else
    {
        return DMB_ERRCODE_NETWORK_AGAIN;
    }

    return code;
}
