/********************  COPYRIGHT(C) ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: localcom.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: ����ͨѶ���Ƴ��������뱾��OMC���ʹ��
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
#include "../../common/commonfun.h"
#include "../../common/druheader.h"
#include "../../protocol/approtocol.h"
#include "../../protocol/apbprotocol.h"
#include "../../protocol/apcprotocol.h"
#include "uartdev.h"
#include "localcom.h"

ComBuf_t g_LocalCom;
SelfThread_t g_LocalComThread;

extern DevicePara_t g_DevicePara;

/*******************************************************************************
*�������� : int LocalComInit(void)
*��    �� : ����ͨѶUART�����ú���
*������� : None
*������� : Fd
*******************************************************************************/
int LocalComInit(void)
{
  UartInit(&g_LocalCom, LOCALCOM);
  LocalComThreadInit();
  return g_LocalCom.Fd;
}

/*******************************************************************************
*�������� : void LocalComThreadInit(void)
*��    �� : ����ͨѶ�ӿ��̳߳�ʼ��
*������� : none
*������� : none
*******************************************************************************/ 
void LocalComThreadInit(void)
{
  g_LocalComThread.Tid = 0;
  g_LocalComThread.ThreadStatus = THREAD_STATUS_EXIT;
}

/*******************************************************************************
*�������� : void *LocalCom_Thread(void *pvoid)
*��    �� : ����ͨѶ�ӿ��߳�
*������� : none
*������� : none
*******************************************************************************/ 
void *LocalCom_Thread(void *pvoid)
{
	int rcsum, res;
	APPack_t PackBuf, *p_PackBuf;//����Э�黺�����ݽṹ
	DevInfo_t DevInfo, *p_DevInfo;
	DevicePara_t *p_DevicePara;

	pthread_detach(pthread_self());
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);//�߳�����Ϊ����ȡ��

	printf("LocalCom_Thread Run!\r\n");
	while(1)
	{
		//���ؼ�ؽ���
		rcsum = UartReceiveData(&g_LocalCom, 500);
		if (rcsum > 0)
		{
			DEBUGOUT("Fd:%d.UartReceiveData:%d.\r\n", g_LocalCom.Fd, g_LocalCom.RecvLen);
			ComDataHexDis(g_LocalCom.Buf, g_LocalCom.RecvLen);
			//һ�����������ݰ�,����APCЭ����,ʹ��06D2�а��յ�ַ->ģ������˳����зֱ���
			p_PackBuf = &PackBuf;
			p_DevInfo = &DevInfo;
			p_DevicePara = &g_DevicePara;

			//res = APCUnpack(g_LocalCom.Buf, g_LocalCom.RecvLen, p_PackBuf);
			res = APUnpack(g_LocalCom.Buf, g_LocalCom.RecvLen, p_PackBuf);
			if(res > 0)
			{
				DEBUGOUT("LocalCom:ReceiveData WriteLogBook.\r\n");
				ComDataWriteLog(g_LocalCom.Buf, g_LocalCom.RecvLen);//����־
				GetDevInfo(p_DevInfo, p_PackBuf);

				if (APProcess(p_PackBuf, p_DevicePara) > 0)
				{
					if (p_PackBuf->PackLen > 0)
						UartSendPack(&g_LocalCom, p_PackBuf);//������ݷ���
				}
				ClearComBuf(&g_LocalCom);
			}
			else
			{
				DEBUGOUT("LocalCom APC Unpacked Error!\r\n");//��������ݰ���ʶ
				ClearComBuf(&g_LocalCom);
			}
		}
		usleep(1000);
	}
	g_LocalComThread.ThreadStatus = THREAD_STATUS_EXIT;
	g_LocalComThread.Tid = 0;
	pthread_exit(NULL);
}

/*******************************************************************************
*�������� : void LocalComThreadStart(void)
*��    �� : ��������ͨѶ�ӿ��߳�
*������� : none
*������� : none
*******************************************************************************/ 
void LocalComThreadStart(void)
{
  if (g_LocalComThread.ThreadStatus != THREAD_STATUS_RUNNING)
  {
    pthread_create(&g_LocalComThread.Tid, NULL, LocalCom_Thread, NULL);
    g_LocalComThread.ThreadStatus = THREAD_STATUS_RUNNING;
    printf("LocalCom_Thread ID: %lu.\n", g_LocalComThread.Tid);
  }
}

/*******************************************************************************
*�������� : void LocalComThreadStop(void)
*��    �� : ֹͣ����ͨѶ�ӿ��߳�
*������� : none
*������� : none
*******************************************************************************/ 
void LocalComThreadStop(void)
{
  if (g_LocalComThread.ThreadStatus != THREAD_STATUS_EXIT)
  {
    pthread_cancel(g_LocalComThread.Tid);
    g_LocalComThread.Tid = 0;
    g_LocalComThread.ThreadStatus = THREAD_STATUS_EXIT;
    printf("LocalCom_Thread Stop!\r\n");
  }
}

/*********************************End Of File*************************************/
