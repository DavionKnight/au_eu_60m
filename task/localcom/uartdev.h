/********************  COPYRIGHT(C) ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: uartdev.h
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: uartͷ�ļ�
**--------------��ʷ�汾��Ϣ----------------------------------------------------
** ������: �ں�ͼ
** ��  ��: v1.0
** �ա���: 
** �衡��: ԭʼ�汾
**--------------��ǰ�汾�޶�----------------------------------------------------
** �޸���:
** ��  ��:
** �ա���:
** �衡��:
**------------------------------------------------------------------------------
**
*******************************************************************************/
#ifndef _UARTDEV_H
#define _UARTDEV_H

#include "../../common/druheader.h"

int OpenCommPortISR(int fd, int comport, struct sigaction saio);
int OpenCommPort(int fd, int comport);
int SetCommState(int fd, int nSpeed, int nBits, char nEvent, int nStop);
int UartInit(ComBuf_t *p_combuf, int ttyid);
int UartReceiveData(ComBuf_t *pcombuf, int waittime);
int UartSendData(int fd, char *sbuf, int len);
int UartSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf);

#endif//_UARTDEV_H

/*********************************End Of File*************************************/