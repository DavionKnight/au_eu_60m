/********************  COPYRIGHT(C) ***************************************
**--------------文件信息--------------------------------------------------------
**文   件   名: apbprotocol.h
**创   建   人: 于宏图
**创 建  日 期: 
**程序开发环境：
**描        述: 中移动APB协议处理程序头文件
**--------------历史版本信息----------------------------------------------------
** 创建人: 于宏图
** 版  本: v1.0
** 日　期: 
** 描　述: 原始版本
**--------------当前版本修订----------------------------------------------------
** 修改人:
** 版  本:
** 日　期:
** 描　述:
**------------------------------------------------------------------------------
**
*******************************************************************************/
#ifndef _APBPROTOCOL_H
#define _APBPROTOCOL_H

#include "../common/druheader.h"

int APBAsciiToByte(char *pbuf, int sum);
int APBByteToAscii(char *pbuf, int sum);
int APBPack(APPack_t *p_packbuf, char *sdbuf);
int	APBUnpack(char *rcbuf, int rcnum, APPack_t *p_packbuf);
char AsciiToByte(char *pbuf);
void ByteToAscii(char hex_data, char *pbuf);

#endif// _APBPROTOCOL_H
/*********************************************************************************
**                            End Of File
*********************************************************************************/
