/********************  COPYRIGHT(C)***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: udpcom.h
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: IRЭ��udp�㲥ͨѶ��������ͷ�ļ�
**--------------��ʷ�汾��Ϣ----------------------------------------------------
** ������: �ں�ͼ
** ��  ��:
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

#ifndef _UDPCOM_H
#define _UDPCOM_H

#include "../../common/druheader.h"

int MACCompare(char *mac1, char *mac2);
int MACToStr(char *p_mac, char *p_macstr);
int GetClientIP(char devno);
INT32U UDPBroadcastMsnDeal(char *mac);
void *UDPCom_Thread(void *pvoid);
void UDPComInit(void);
void UDPComThreadInit(void);
void UDPComThreadStart(void);
void UDPComThreadStop(void);
int ClientDbTableInit(char *tblname);

extern int creat_udp_relay(void);
#endif //_UDPCOM_H

/**************END OF FILE*********************************/
