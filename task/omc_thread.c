/*******************************************************************************
*�������� : int main(void)
*��    �� : 
*������� : 
*������� : 
*******************************************************************************/
#include "./localcom/localcom.h"
#include "./omccom/omccom.h"
//#include "./tcpclient/tcpclient.h"
//#include "./tcpserver/tcpserver.h"
#include "./udpcom/udpcom.h"
#include "../common/druheader.h"
#include "../net/netcom.h"
#include "../protocol/approtocol.h"
#include "../sqlite/drudatabase.h"

int settimeflag;
int g_DevType;
DevicePara_t g_DevicePara;
extern unsigned char get_device_type(void);
/******i*************************************************************************
*�������� : int _Omc_Thread_Init(void)
*��    �� : ����OMC��ؽ��̳�ʼ��
*������� : None
*������� : Fd
*******************************************************************************/
int _Omc_Thread_Init(void)
{

	DevicePara_t *pdev;
	DevInfo_t *pdevinfo, deviceinfo;

	//------2015_03_09----------------
	Pdu_t pdu, *p_pdu;
  	p_pdu = &pdu;
	//--------------------------------
	
	//ͨ���豸��ȡ
	g_DevType = get_device_type();
	//g_DevType = EXPAND_UNIT;
	DEBUGOUT("OMCcom Test:%d.\r\n", g_DevType);

	DataBaseInit();
	pdevinfo = &deviceinfo;
	pdev = &g_DevicePara;

	pdevinfo->DeviceNo = 0;
	pdevinfo->ModuleAddr = 0;
	pdevinfo->ModuleType = 0;
	LoadDevicePara(pdevinfo, pdev);
  
	//------2015_03_09----------------
	settimeflag = 1;
	DbGetParaValue_MCP_C(pdevinfo, DEVICETIME_ID, p_pdu);
	SetDevTime(pdu.var);
	//--------------------------------
  return 1;
}

/*******************************************************************************
*�������� : int _Omc_Thread(void)
*��    �� : ����OMC��ؽ���
*������� : None
*������� : Fd
*******************************************************************************/
int _Omc_Thread(void)
{
  //LocalComInit();//����ͨѶ��ʼ��
  //LocalComThreadStart();
  if (g_DevType == MAIN_UNIT)
  {
  	ModemInit();//modemͨѶ��ʼ��
  }

  OMCComInit();//OMCͨѶ��ʼ��
  OMCComThreadStart();
  if (g_DevType == MAIN_UNIT)
  {
    //��OMCͨѶ�߳�

    //server�߳�
    //TCPServerComInit();
    //TCPServerComThreadStart();
    //UDP�߳�
    UDPComThreadInit();
    UDPComThreadStart();
  }
  else
  {
    //��serverͨѶclient���߳�
    UDPComThreadInit();
    UDPComThreadStart();
    //TCPClientComInit();
    //TCPClientComThreadStart();
  }
  return 1;
}
