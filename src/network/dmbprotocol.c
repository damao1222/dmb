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

static dmbCode readData(dmbNetworkContext *pCtx, dmbConnect *pConn);
static dmbCode processData(dmbConnect *pConn);
static dmbCode writeData(dmbNetworkContext *pCtx, dmbConnect *pConn);
dmbCode dmbProcessPackage(dmbConnect *pConn, dmbINT16 iCmd, dmbBYTE *pData, dmbUINT uSize);
static inline dmbBOOL needDisconnect(dmbCode code)
{
    return code > DMB_ERRCODE_NETWORK_ERRBEGIN && code < DMB_ERRCODE_NETWORK_ERREND;
}

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
    dmbResponse *pResp = (dmbResponse*) pConn->writeBuf;
    pResp->magicNum = DMB_MAGIC_NUMBER;
    pResp->version = DMB_VERSION;
    pResp->status = code;
    pConn->needClose = needDisconnect(code);

    pResp->length = 0;
    pConn->writeIndex = 0;
    pConn->writeLength += dmbResponseHeaderSize;
}

void dmbMakeErrorResponseWithData(dmbConnect *pConn, dmbCode code, dmbBYTE *pData, dmbUINT uSize)
{
    dmbResponse *pResp = (dmbResponse*) pConn->writeBuf;
    pResp->magicNum = DMB_MAGIC_NUMBER;
    pResp->version = DMB_VERSION;
    pResp->status = code;

    pConn->writeIndex = 0;
    if (dmbResponseHeaderSize + uSize > g_settings.net_write_bufsize)
    {
        pResp->status = DMB_ERRCODE_OUT_OF_WRITEBUF;
        pResp->length = 0;
        pConn->writeLength += dmbResponseHeaderSize;
        pConn->needClose = TRUE;
        return ;
    }
    pResp->length = uSize;
    pConn->writeLength += dmbResponseHeaderSize + uSize;
    dmbMemCopy(pConn->writeBuf+pConn->writeIndex, pData, uSize);
}

void dmbProcessEvent(dmbNetworkContext *pCtx, dmbConnect *pConn)
{
    dmbCode readCode, dataCode, writeCode;

    readCode = readData(pCtx, pConn);

    if (readCode == DMB_ERRCODE_NETWORK_ERROR || readCode == DMB_ERRCODE_NERWORK_CLOSED)
        return ;

    if (!dmbConnectIsBlocked(pConn))
        dataCode = processData(pConn);

    writeCode = writeData(pCtx, pConn);

    if (pConn->needClose)
        dmbNetworkCloseConnect(pCtx, pConn);

    if (readCode == DMB_ERRCODE_NETWORK_AGAIN &&
        (writeCode == DMB_ERRCODE_NETWORK_AGAIN ||
         dataCode == DMB_ERRCODE_NETWORK_AGAIN))
    {
        //ALL DONE
    }
    else
    {
        dmbListPushBack(&pCtx->roundRobinList, &pConn->roundNode);
    }
}

dmbCode dmbProcessPackage(dmbConnect *pConn, dmbINT16 iCmd, dmbBYTE *pData, dmbUINT uSize)
{
    DMB_UNUSED(pConn);
    DMB_UNUSED(iCmd);
    DMB_UNUSED(pData);
    DMB_UNUSED(uSize);

    return DMB_ERRCODE_OK;
}

static dmbCode readData(dmbNetworkContext *pCtx, dmbConnect *pConn)
{
    ssize_t ret = 0;
    dmbCode code = DMB_ERRCODE_OK;
    if (dmbConnectCanRead(pConn))
    {
        do {
            //Need add to roundrobinlist and continue read.
            if (pConn->readBufSize <= pConn->readIndex)
            {
                dmbNetworkWatchTimeout(pCtx, pConn, g_settings.net_rw_timeout);
                return code;
            }

            ret = dmbReadAvailable(pConn->cliFd, pConn->readBuf + pConn->readIndex, pConn->readBufSize - pConn->readIndex);
            if (ret == DMB_IO_AGAIN)
            {
                DMB_LOGD("Read again\n");
                pConn->canRead = FALSE;
                code = DMB_ERRCODE_NETWORK_AGAIN;
                dmbNetworkWatchTimeout(pCtx, pConn, g_settings.net_rw_timeout);
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
                code = DMB_ERRCODE_NERWORK_CLOSED;
                break;
            }
            else
            {
                pConn->readIndex += ret;
                pConn->readLength += ret;

                if (pConn->readLength < dmbRequestHeaderSize)
                {
                    continue;
                }
            }
        } while (0);
    }
    else
    {
        return DMB_ERRCODE_NETWORK_AGAIN;
    }

    return code;
}

static dmbCode processData(dmbConnect *pConn)
{
    dmbCode code = DMB_ERRCODE_OK;
    dmbRequest *pRequest;

    if (pConn->readLength < dmbRequestHeaderSize)
    {
        return DMB_ERRCODE_NETWORK_AGAIN;
    }

    code = parsePkgHeader(pConn->readBuf + pConn->requestIndex, &pRequest);
    if (code != DMB_ERRCODE_OK)
    {
        if (pRequest->length + dmbResponseHeaderSize > g_settings.net_read_bufsize)
        {
            code = DMB_ERRCODE_OUT_OF_READBUF;
            dmbMakeErrorResponseWithData(pConn, code, (dmbBYTE*)&g_settings.net_read_bufsize, sizeof(g_settings.net_read_bufsize));
        }
        else
        {
            dmbMakeErrorResponse(pConn, code);
        }
        return code;
    }

    if (pRequest->length < pConn->readLength - dmbResponseHeaderSize)
    {
        do {
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
        } while (0);

        pConn->readLength -= (dmbResponseHeaderSize + pRequest->length);
        pConn->readIndex -= pConn->readLength;
        if (pConn->readLength > 0)
        {
            dmbMemMove(pConn->readBuf + pConn->requestIndex,
                       pConn->readBuf + pConn->requestIndex + dmbResponseHeaderSize + pRequest->length,
                       pConn->readLength);
        }
    }
    else
    {
        return DMB_ERRCODE_NETWORK_AGAIN;
    }

    return code;
}

static dmbCode writeData(dmbNetworkContext *pCtx, dmbConnect *pConn)
{
    dmbCode code = DMB_ERRCODE_OK;
    ssize_t ret = 0;
    if (dmbConnectCanWrite(pConn))
    {
        do {
            ret = dmbWriteAvailable(pConn->cliFd, pConn->writeBuf + pConn->writeIndex, pConn->writeLength);
            if (ret == DMB_IO_AGAIN)
            {
                DMB_LOGD("Write again\n");
                pConn->canWrite = FALSE;
                pConn->isblocked = TRUE;
                code = DMB_ERRCODE_NETWORK_AGAIN;
                dmbNetworkWatchTimeout(pCtx, pConn, g_settings.net_rw_timeout);
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
            }
            else
            {
                //write response
                pConn->writeIndex += ret;
                pConn->writeLength -= ret;
                if (pConn->writeLength > 0)
                {
                    continue;
                }
                else
                {
                    pConn->isblocked = FALSE;
                    pConn->writeIndex = 0;
                    return DMB_ERRCODE_OK;
                }
            }
        } while (0);
    }
    else
    {
        return DMB_ERRCODE_NETWORK_AGAIN;
    }

    return code;
}
