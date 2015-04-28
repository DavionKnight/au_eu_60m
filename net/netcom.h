/********************  COPYRIGHT(C) ***************************************
**--------------文件信息--------------------------------------------------------
**文   件   名: netcom.h
**创   建   人: 于宏图
**创 建  日 期: 
**程序开发环境：
**描        述: 网络处理程序头文件
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
#ifndef _NETCOM_H
#define _NETCOM_H

#include "../common/druheader.h"

int GetNetlinkStatus(const char *if_name);
int GetSelfMac(char *ethname, char *pbuf);
INT32U GetSelfIp(char *ethname);
int SetDevIPpara(char *ethname, INT32U ip, char settype);
int SocketServerInit(int serverport, char iptype);
int SocketClientConnet(ComBuf_t *pcombuf, INT32U serverip, int serverport, INT32U devip, int devport, char iptype);
void SocketClientDisconnet(ComBuf_t *pcombuf);
int	TCPSocketReceiveData(ComBuf_t *pcombuf, int waittime);
int	TCPSocketSendData(int fd, char *sbuf, int len);
int TCPSocketSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf);
int	UDPSocketReceiveData(ComBuf_t *pcombuf, INT32U serverip, int serverport, int waittime);
int	UDPSocketSendData(int fd, char *sbuf, int len, INT32U serverip, int serverport);
int UDPSocketSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf, INT32U serverip, int serverport);
INT32U GetReceiveIP(void);

#endif  //_NETCOM_H

/*********************************End Of File*************************************/
