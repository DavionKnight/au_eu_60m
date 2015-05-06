/********************  COPYRIGHT(C) ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: udpcom.c
**��   ��   ��: 
**�� ��  �� ��:  
**���򿪷�������
**��        ��: IRЭ��udp�㲥ͨѶ�������
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
#include "../localcom/localcom.h"
//#include "../tcpserver/tcpserver.h"
#include "../../common/commonfun.h"
#include "../../net/netcom.h"
#include "../../protocol/approtocol.h"
#include "../../sqlite/sqliteops.h"
#include "../../sqlite/drudatabase.h"
#include "udpcom.h"

ComBuf_t g_UDPCom;
SelfThread_t g_UDPComThread;

extern int g_DevType;
extern unsigned long serverip;
extern DevicePara_t g_DevicePara;
ClientBuf_t g_TCPClient[CONNECT_TOTALNUMBER];
//extern ClientBuf_t g_TCPClient[CONNECT_TOTALNUMBER];

/*******************************************************************************
*�������� : int MACCompare(char *mac1, char *mac2)
*��    �� : MAC��ַ�ȶ�,
*������� : int newfd:��������fd;struct sockaddr_in addr:������IP��ַ
*������� : ��ͬ����1,���򷵻�-1
*******************************************************************************/ 
int MACCompare(char *mac1, char *mac2)
{
int i;

  for (i = 0; i < MACSIZE; i++)
  {
    if (*mac1++ != *mac2++)
    {
      return -1;
    }
  }
  return 1;
}

/*******************************************************************************
*�������� : int MACToStr(char *p_mac, char *p_macstr)
*��    �� : MAC��ַת��Ϊ�ַ���
*������� : char *p_mac:HEX��MAC��ַ;char *p_macstr:�ַ���MAC��ַ
*������� : ��ͬ����1,���򷵻�-1
*******************************************************************************/ 
int MACToStr(char *p_mac, char *p_macstr)
{
int i;

	for(i = 0; i < (MACSIZE-1); i++)
	{
		sprintf(p_macstr, "%02X:", *p_mac++);
		p_macstr = p_macstr + 3;
	}
	sprintf(p_macstr, "%02X", *p_mac);
	*(p_macstr+2) = 0;
	return 1;
}

/*******************************************************************************
*�������� : int GetClientIP(char devno)
*��    �� : ����devno��ȡclient��Ӧ��IP��ַ(���һλ)
*������� : char devno: �豸���
*������� : ��ȷ���ض�Ӧ��IP��ַ(���һλ),���򷵻�0
*******************************************************************************/ 
int GetClientIP(char devno)
{
int res, devip[4];
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE];

	memset(devip, 0, sizeof(devip));
	sprintf(sql, "SELECT * FROM %s WHERE DevNo=%d;", CLIENT_TBL, devno);
	res = SqliteSelect(sql, &sqlres);

	if (res == SQLITE_OK)
	{
		//SqlResultDis(&sqlres);
		if(sqlres.nRow > 0)//��DevNo����,���·���DevNo
		{
			sscanf(sqlres.dbResult[sqlres.nColumn + CLIENTTBL_IPADDR], "%d.%d.%d.%d",
             &devip[0], &devip[1], &devip[2], &devip[3]);
		}
		sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
	}
	return devip[3];
}

/*******************************************************************************
*�������� : int UDPMainMsnDeal(UDPRequestMsg_t *p_reqmsg, UDPAckMsg_t *p_ackmsg)
*��    �� : UDPMainMsnDeal��Ϣ������
*������� : p_reqmsg:������Ϣָ��;p_ackmsg:Ӧ����Ϣָ��
*������� : ��ȷ����1,���򷵻�0
*******************************************************************************/ 
int UDPMainMsnDeal(UDPRequestMsg_t *p_reqmsg, UDPAckMsg_t *p_ackmsg)
{
int res;
Pdu_t pdu;
SqlResult_t sqlres;
INT32U selfip, devip;
DevInfo_t DevInfo, *pdevinfo;
char macbuf[20], ipbuf[20], sql[SQL_CMD_SIZE], newip4;
struct sockaddr_in saddr;

	//�ж��Ƿ�����������Ԫ(server)IP��Ϣ�仯��������
	GetSelfMac("eth0", ipbuf);
	MACToStr(ipbuf, macbuf);
	selfip = GetSelfIp("eth0");
	memcpy(&saddr.sin_addr, &selfip, 4);
	inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
	//����Ԫ��Ϣ����
	sprintf(sql, "UPDATE %s SET IP=\'%s\' WHERE Mac=\'%s\';", CLIENT_TBL, ipbuf, macbuf);
	SqliteUpdate(sql);//����sqtbl���ݱ��ж�Ӧ����

	MACToStr(p_reqmsg->RRU_Mac, macbuf);
	sprintf(sql, "SELECT * FROM %s WHERE Mac=\"%s\";", CLIENT_TBL, macbuf);
	res = SqliteSelect(sql, &sqlres);

	if (res == SQLITE_OK)
	{
		//SqlResultDis(&sqlres);
		if(sqlres.nRow > 0)//��MAC����
		{
			//������ԪMAC��ͬ
			if (atoi(sqlres.dbResult[sqlres.nColumn + CLIENTTBL_DEVTYPE]) == MAIN_UNIT)
			{
				DEBUGOUT("%s������ԪMAC��ͬ,ϵͳ����!\r\n", macbuf);
				memset(p_ackmsg, 0, sizeof(UDPAckMsg_t));
				sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
				return -1;
			}
			else
			{
				goto _CONFIGDEVIP;
			}
		}
		else//��MAC������
		{
			DEBUGOUT("��MAC������!\r\n");
			MACToStr(p_reqmsg->RRU_Mac, macbuf);
			//���ݱ������Ӵӻ���Ϣ,(Fd,DevNo,Mac,IP,DevType,LastComTime)
			sprintf(sql, "INSERT INTO %s(Mac,DevType) VALUES (\'%s\',%d);", CLIENT_TBL, macbuf, p_reqmsg->DevType);
			SqliteInsert(sql);//��¼�������ݿ�
		}
	}
	else
	{
		DEBUGOUT("���ݿ�client_info�������!\r\n");
		memset(p_ackmsg, 0, sizeof(UDPAckMsg_t));
		sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
		return -1;
	}
//IP��ַ����
_CONFIGDEVIP:
	devip = p_reqmsg->IPAddr;
	MACToStr(p_reqmsg->RRU_Mac, macbuf);
	memcpy((char *)&devip, (char *)&selfip, 3);//������ͬһ����IP
	newip4 = DEFAULT_STARTIP;//��2��ѯIP�Ƿ�����ͬIP,������ͬ��,IP���·���
	if ((devip == selfip) || (((char *)&devip+3) == DEFAULT_STARTIP))//������ԪIP��ͬor IP=X.X.X.0/1
	{
		memcpy(((char *)&devip+3), &newip4, 1);
	}
	
_RECONFIGDEVIP:
	memcpy(&saddr.sin_addr, &devip, 4);
	inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
	sprintf(sql, "SELECT * FROM %s WHERE IP=\'%s\';", CLIENT_TBL, ipbuf);
	res = SqliteSelect(sql, &sqlres);
	
	if (res == SQLITE_OK)
	{
		if(sqlres.nRow > 0)//��IP�Ѿ�����
		{
			//SqlResultDis(&sqlres);
			//MAC��ַ��ͬ
			printf("dbResultMAC[%d]=%s\r\n", sqlres.nColumn, sqlres.dbResult[sqlres.nColumn + CLIENTTBL_MAC]);
			if (strcmp(sqlres.dbResult[sqlres.nColumn + CLIENTTBL_MAC], macbuf) != 0)
			{
				//���·���IP
				printf("���·���IP\r\n");
				memcpy(((char *)&devip+3), &newip4, 1);
				newip4++;
				if (newip4 == CONNECT_TOTALNUMBER)
				{
					DEBUGOUT("UDP Broadcast IP Record Full!\r\n");
				}
				else
				{
					goto _RECONFIGDEVIP;
				}
			}
		}
	}
	p_reqmsg->IPAddr = devip;
	memcpy(&saddr.sin_addr, &devip, 4);
	inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
	//�����豸IP��Ϣ����
	sprintf(sql, "UPDATE %s SET IP=\'%s\' WHERE Mac=\'%s\';", CLIENT_TBL, ipbuf, macbuf);
	SqliteUpdate(sql);//����sqtbl���ݱ��ж�Ӧ����

	//DevNo����
	devip = p_reqmsg->DevNo;
	newip4 = 1;//��1��ѯDevNo�Ƿ�����ͬDevNo,������ͬ��,DevNo���·���
	if (devip == 0)//������ԪDevNo��ͬ
	{
		devip = newip4;
	}

_RECONFIGDEVNO:
	sprintf(sql, "SELECT * FROM %s WHERE DevNo=%d;", CLIENT_TBL, devip);
	res = SqliteSelect(sql, &sqlres);

	if (res == SQLITE_OK)
	{
		if(sqlres.nRow > 0)//��DevNo����,���·���DevNo
		{
			printf("dbResultMAC[%d]=%s\r\n", sqlres.nColumn, sqlres.dbResult[sqlres.nColumn + CLIENTTBL_MAC]);
			if (strcmp(sqlres.dbResult[sqlres.nColumn + CLIENTTBL_MAC], macbuf) != 0)
			{
				printf("���·���DevNo\r\n");
				devip = newip4;
				newip4++;
				if (newip4 == CONNECT_TOTALNUMBER)
				{
					DEBUGOUT("DevNo Record Full!\r\n");
				}
				else
				{
					goto _RECONFIGDEVNO;
				}
			}
		}
	}
	p_reqmsg->DevNo = devip;
	sprintf(sql, "UPDATE %s SET DevNo=%d WHERE Mac=\'%s\';", CLIENT_TBL, devip, macbuf);
	SqliteUpdate(sql);//����sqtbl���ݱ��ж�Ӧ����
					
	p_ackmsg->BBU_OptInterface = p_reqmsg->BBU_OptInterface;
  	p_ackmsg->RRU_ID = p_reqmsg->RRU_ID;
  	p_ackmsg->BBU_ID = g_DevType;
	memcpy(p_ackmsg->RRU_Mac, p_reqmsg->RRU_Mac, sizeof(p_reqmsg->RRU_Mac));
	p_ackmsg->RRU_IP = p_reqmsg->IPAddr;
	p_ackmsg->BBU_IP = GetSelfIp("eth0");

 	pdevinfo = &DevInfo;
	memset(pdevinfo, 0, sizeof(DevInfo_t));
	DbGetParaValue(pdevinfo, DEVNETMASK_ID, &pdu);
	memcpy(&selfip, &pdu.var, pdu.len);
	p_ackmsg->NetMask = selfip;

	DbGetParaValue(pdevinfo, STATIONNO_ID, &pdu);
	memcpy(&selfip, &pdu.var, pdu.len);
	p_ackmsg->StationNo = selfip;
	p_ackmsg->DevNo = p_reqmsg->DevNo;
	return 1;
}

/*******************************************************************************
*�������� : INT32U UDPBroadcastMsnDeal(char *pmac)
*��    �� : UDPBroadcastMsnDeal������
*������� : char *pmac:������MAC��ַ
*������� : IP��ַ
*******************************************************************************/ 
INT32U UDPBroadcastMsnDeal(char *pmac)
{
int i, j, k, flag;
ClientBuf_t *pcbuf;
INT32U selfip;
char macbuf[MACSIZE], newip4, ip4;

  //�������Ӽ�¼,���Ƿ��Ѿ����ڸü�¼
printf("\r\nMAC:");
for(i=0;i<6;i++)
printf(":%02X",*pmac++);
printf("\r\n\r\n");
char ipbuf[20];
struct sockaddr_in saddr;

  flag = 0;
  for (i = 0; i < CONNECT_TOTALNUMBER; i++)
  {
    pcbuf = &g_TCPClient[i];
    //����Ѿ��������Ӽ�¼��Ϣ,�ж��Ƿ�������ͬһMAC��ַ
    if (MACCompare(pcbuf->Mac, pmac) == 1)//mac��ַ��ͬ,����ԭ��IP��ַ����,���豸����Ϊͬһ�����豸
    {
      flag = 1;
      memcpy(&ip4, ((char *)&pcbuf->IPAddr+3), 1);
      
memcpy(&saddr.sin_addr, &pcbuf->IPAddr, 4);
 inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
printf("Saved ip:%s\r\n",ipbuf);

      if (ip4 == 0)//��Ч��IP
      {
        pcbuf->IPAddr = 0;
        goto _CONFIGNEWIP;
      }
      else
      {
        selfip = GetSelfIp("eth0");
        memcpy((char *)&pcbuf->IPAddr, (char *)&selfip, 3);//������������Ԫͬһ����IP
        if (pcbuf->IPAddr == selfip)//������ԪIP��ͬ
        {

memcpy(&saddr.sin_addr, &pcbuf->IPAddr, 4);
  inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
printf("pcbuf->IPAddr == selfip:%s\r\n",ipbuf);

          pcbuf->IPAddr = 0;
          goto _CONFIGNEWIP;
        }
        else
        {

memcpy(&saddr.sin_addr, &pcbuf->IPAddr, 4);
 inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
printf("pcbuf->IPAddr:%s\r\n",ipbuf);

          return pcbuf->IPAddr;
        }
      }
    }
  }

  //��������ڸü�¼,���¿ͻ��˵ļ�������
  if (flag == 0)
  {
    memset(macbuf, 0, sizeof(macbuf));
    for (i = 0; i < CONNECT_TOTALNUMBER; i++)
    {
      pcbuf = &g_TCPClient[i];
      if (MACCompare(pcbuf->Mac, macbuf) == 1)//ȫ0,��ʾ�޼�¼,����MAC��ַ,�����µ�IP
      {
        memcpy(pcbuf->Mac, pmac, MACSIZE);//
_CONFIGNEWIP:
        //MAC��ַ,������IP
        selfip = GetSelfIp("eth0");
        memcpy((char *)&pcbuf->IPAddr, (char *)&selfip, 3);//������ͬһ����IP

memcpy(&saddr.sin_addr, &pcbuf->IPAddr, 4);
  inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
printf("������ͬһ����IP:%s\r\n",ipbuf);

        newip4 = DEFAULT_STARTIP;//��2��ѯIP�Ƿ�����ͬIP,������ͬ��,IP����1
        for (j = 0; j < CONNECT_TOTALNUMBER; j++)
        {
          for (k = 0; k < CONNECT_TOTALNUMBER; k++)//IP��˳������
          {
            memcpy(&ip4, ((char *)&selfip+3), 1);
            if (newip4 == ip4)//������ԪIP��ͬ
            {
printf("������ԪIP��ͬ\r\n");
              break;
            }

            memcpy(&ip4, ((char *)&g_TCPClient[k].IPAddr+3), 1);
            if (newip4 == ip4)//IP��ַ����
            {
            	memcpy(((char *)&pcbuf->IPAddr+3), &newip4, 1);
            	//memset(((char *)&pcbuf->IPAddr+3), newip4, 1);
printf("IP��ַ����,IP:%d\r\n",ip4);
printf("IPAddr:%d", pcbuf->IPAddr);
memcpy(&saddr.sin_addr, &pcbuf->IPAddr, 4);
  inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
printf("IP��ַ:%s\r\n",ipbuf);
							return pcbuf->IPAddr;
            }
          }
          if (k == CONNECT_TOTALNUMBER)
          {
printf("newip4:%d", newip4);
memcpy(&pcbuf->IPAddr, &selfip, 3);//������ͬһ����IP
printf("selfip:%d\r\n", pcbuf->IPAddr);
            memcpy(((char *)&pcbuf->IPAddr+3), &newip4, 1);
printf("IPAddr:%d\r\n", pcbuf->IPAddr);
//printf("pcbuf:%d\r\n", pcbuf);
pcbuf=&g_TCPClient[i];
//printf("pcbuf:%d\r\n", pcbuf);
memcpy(&saddr.sin_addr, &pcbuf->IPAddr, 4);
  inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
printf("newip4:%d;�·����IP:%s\r\n", newip4, ipbuf);

printf("i=:%d;\r\n", i);
memcpy(&saddr.sin_addr, &g_TCPClient[i].IPAddr, 4);
  inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
printf("�·����IP:%s\r\n",ipbuf);
            return pcbuf->IPAddr;
          }
          newip4++;
        }
        //��¼����
        if(j == CONNECT_TOTALNUMBER)
        {
          DEBUGOUT("UDP Broadcast IP Record Full!\r\n");
        }
      }
    }
    //��¼����
    if(i == CONNECT_TOTALNUMBER)
    {
      DEBUGOUT("Not find Matching MAC!\r\n");
    }
  }
  return 0;
}

struct st
{
	char r;
	int a,b;
};

/*******************************************************************************
*�������� : void *UDPCom_Thread(void *pvoid)
*��    �� : UDPͨѶ�߳�
*������� : none
*������� : none
*******************************************************************************/ 
void *UDPCom_Thread(void *pvoid)
{
int resum;
char ipstr[20];
struct in_addr inaddr;
INT32U broadcast_ip;
ComBuf_t *p_combuf;
Pdu_t pdu;
DevicePara_t *pdevpara;
DevInfo_t DevInfo, *pdevinfo;
time_t starttime;
UDPRequestMsg_t reqmsg;
UDPAckMsg_t ackmsg;

  pthread_detach(pthread_self());
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);//�߳�����Ϊ����ȡ��

  UDPComInit();
  
  starttime = time(NULL)-NETCONNETINTERVAL+3;
  printf("UDPCom_Thread Run!\r\n");
  sprintf(ipstr, "192.168.10.255");
  inet_pton(AF_INET, ipstr, (void *)&inaddr);
  broadcast_ip = inaddr.s_addr;
  while(1)
  {
    sleep(1);
    p_combuf = &g_UDPCom;	//modified by kevin 2015-5-5
    p_combuf->Timer = (int)(time(NULL) - starttime);
    pdevinfo = &DevInfo;

    if (p_combuf->Status == NET_DISCONNET)
    {
      if (g_DevType == MAIN_UNIT)
        SocketClientConnet(p_combuf, 0, 0, broadcast_ip, IRUDPRMUPORT, UDP_BROADCAST_MODE);
      else
        SocketClientConnet(p_combuf, 0, 0, broadcast_ip, IRUDPRAUPORT, UDP_BROADCAST_MODE);
    }
    if (p_combuf->Status == NET_CONNET)
    {
      if (g_DevType == MAIN_UNIT)
      {
        resum = UDPSocketReceiveData(p_combuf, broadcast_ip, IRUDPRAUPORT, COMWAITTIME);
        if (resum > 0)
        {
          DEBUGOUT("Fd:%d.UDPCom_Thread ReceiveData:%d.\r\n", p_combuf->Fd, p_combuf->RecvLen);
          ComDataHexDis(p_combuf->Buf, p_combuf->RecvLen);
          
          memcpy(&reqmsg, p_combuf->Buf, sizeof(reqmsg));
          if (UDPMainMsnDeal(&reqmsg, &ackmsg) == 1)
         	{
          	UDPSocketSendData(p_combuf->Fd, (char *)&ackmsg, sizeof(ackmsg), broadcast_ip, IRUDPRAUPORT);
          }
          ClearComBuf(p_combuf);
        }
      }
      else
      {
        resum = UDPSocketReceiveData(p_combuf, broadcast_ip, IRUDPRMUPORT, COMWAITTIME);
        if (resum > 0)
        {
          DEBUGOUT("Fd:%d.UDPCom_Thread ReceiveData:%d.\r\n", p_combuf->Fd, p_combuf->RecvLen);
          ComDataHexDis(p_combuf->Buf, p_combuf->RecvLen);
          
          memcpy(&ackmsg, p_combuf->Buf, sizeof(ackmsg));
          GetSelfMac("eth0", reqmsg.RRU_Mac);
          if (MACCompare(ackmsg.RRU_Mac, reqmsg.RRU_Mac) == 1)//�ж��Ƿ��Ǳ���MAC,����IP��ַ
          {
            pdevpara = &g_DevicePara;
            pdevinfo = &DevInfo;

            memset(pdevinfo, 0, sizeof(DevInfo_t));
            memset(pdu.var, 0, sizeof(pdu.var));
						//�豸IP
            memcpy(&pdevpara->DeviceIP, &ackmsg.RRU_IP, 4);
            SetDevIPpara("eth0", pdevpara->DeviceIP, SET_IP);
	          pdu.len = 4 ;
            memcpy(pdu.var, &ackmsg.RRU_IP, 4);
            DbSaveParaValue(pdevinfo, DEVICEIP_ID, &pdu);
            //�豸Ĭ������:Ĭ��xx.xx.xx.1
            memset(((char *)&ackmsg.RRU_IP+3), 1, 1);
            memcpy(&pdevpara->DeviceGateway, &ackmsg.RRU_IP, 4);
            SetDevIPpara("eth0", pdevpara->DeviceGateway, SET_GATEWAY);
	          pdu.len = 4 ;
            memcpy(pdu.var, &ackmsg.RRU_IP, 4);
            DbSaveParaValue(pdevinfo, DEVDEFAULTGW_ID, &pdu);
						//�豸��������
			/*
            memcpy(&pdevpara->DeviceNetmask, &ackmsg.NetMask, 4);
            SetDevIPpara("eth0", pdevpara->DeviceNetmask, SET_NETMASK);
	          pdu.len = 4 ;
            memcpy(pdu.var, &ackmsg.NetMask, 4);
            DbSaveParaValue(pdevinfo, DEVNETMASK_ID, &pdu);//�洢��������
            */
            //վ����StationNo
            pdevpara->StationNo = ackmsg.StationNo;
	          pdu.len = 4 ;
	          memcpy(pdu.var, &ackmsg.StationNo, 4);
            DbSaveParaValue(pdevinfo, STATIONNO_ID, &pdu);
						//�豸DeviceNo
						memset(pdu.var, 0, sizeof(pdu.var));
            pdevpara->DeviceNo = ackmsg.DevNo;
	          pdu.len = 1 ;
	          pdu.var[0] = pdevpara->DeviceNo;
            DbSaveParaValue(pdevinfo, DEVICENO_ID, &pdu);
            
			memcpy(&serverip, &ackmsg.BBU_IP, 4);//����ԪIP
						
            memset((char *)&ackmsg, 0, sizeof(UDPAckMsg_t));
            SocketClientDisconnet(p_combuf);
            goto _UDPCONFIGNEWIP_EXIT;
          }
          ClearComBuf(p_combuf);
        }
        else if (p_combuf->Timer > IRCONNETINTERVAL)
        {
        	pdevpara = &g_DevicePara;
          starttime = time(NULL);
          reqmsg.BBU_OptInterface = 1;
          reqmsg.RRU_ID = g_DevType;
          GetSelfMac("eth0", reqmsg.RRU_Mac);//RRU MAC��ַ,6BYTE
          //�Զ�������
          reqmsg.DevNo = pdevpara->DeviceNo;
          reqmsg.IPAddr = GetSelfIp("eth0");
          reqmsg.DevType = g_DevType;
          UDPSocketSendData(p_combuf->Fd, (char *)&reqmsg, sizeof(reqmsg), broadcast_ip, IRUDPRMUPORT);
        }
      }
    }
    else
    {
      ClearComBuf(p_combuf);
      p_combuf->Status = NET_DISCONNET;
    }
  }
_UDPCONFIGNEWIP_EXIT:
  g_UDPComThread.ThreadStatus = THREAD_STATUS_EXIT;
  g_UDPComThread.Tid = 0;
  DEBUGOUT("UDPCom_Thread Exit!\r\n");
  pthread_exit(NULL);
}

/*******************************************************************************
*�������� : int UDPComInit(void)
*��    �� : UDPͨѶ���ú���
*������� : None
*������� : Fd
*******************************************************************************/
void UDPComInit(void)
{
  g_UDPCom.Fd = -1;
  g_UDPCom.RecvLen = 0;
  g_UDPCom.Status = NET_DISCONNET;

  memset(g_UDPCom.Buf, 0, COMBUF_SIZE);
  memset(&g_TCPClient, 0 , sizeof(g_TCPClient));
  if (g_DevType == MAIN_UNIT)
  {
  	ClientDbTableInit(CLIENT_TBL);
  }
  UDPComThreadInit();
}

/*******************************************************************************
*�������� : void UDPComThreadInit(void)
*��    �� : UDPͨѶ�̳߳�ʼ��
*������� : none
*������� : none
*******************************************************************************/ 
void UDPComThreadInit(void)
{
  g_UDPComThread.Tid = 0;
  g_UDPComThread.ThreadStatus = THREAD_STATUS_EXIT;
}

/*******************************************************************************
*�������� : void UDPComThreadStart(void)
*��    �� : ��ʼUDPͨѶ�߳�
*������� : none
*������� : none
*******************************************************************************/ 
void UDPComThreadStart(void)
{
  if (g_UDPComThread.ThreadStatus != THREAD_STATUS_RUNNING)
  {
    pthread_create(&g_UDPComThread.Tid, NULL, UDPCom_Thread, NULL);
    g_UDPComThread.ThreadStatus = THREAD_STATUS_RUNNING;
    printf("UDPCom_Thread ID: %lu.\n", g_UDPComThread.Tid);
  }
}

/*******************************************************************************
*�������� : void UDPComThreadStop(void)
*��    �� : ֹͣUDPͨѶ�߳�
*������� : none
*������� : none
*******************************************************************************/ 
void UDPComThreadStop(void)
{
  if (g_UDPComThread.ThreadStatus != THREAD_STATUS_EXIT)
  {
    pthread_cancel(g_UDPComThread.Tid);
    g_UDPComThread.Tid = 0;
    g_UDPComThread.ThreadStatus = THREAD_STATUS_EXIT;
    printf("UDPCom_Thread Stop!\r\n");
  }
}

/*******************************************************************************
*�������� : int DbTableInit(char *tblname)
*��    �� : Client���ݱ��ʼ��
*������� : char *tblname:���ݱ�����
*������� : �ɹ�����1,������<0
*******************************************************************************/
int ClientDbTableInit(char *tblname)
{
int res, result;
INT32U selfip;
SqlResult_t sqlres;
char macbuf[20], ipbuf[20], sql[SQL_CMD_SIZE];
struct sockaddr_in saddr;

  //����tbl���ݱ��Ƿ����,�粻��������
  sprintf(sql, "SELECT * FROM sqlite_master WHERE name=\'%s\';", tblname);
  res = SqliteSelect(sql, &sqlres);
  if(res == SQLITE_OK)
  {
  	if(sqlres.nRow > 0)//tblname���ݱ����
  	{
      sprintf(sql, "DROP TABLE %s;", tblname);
      res = SqliteCreate(sql);//����sqtbl���ݱ�
      if(res != SQLITE_OK)//����sqtbl���ݱ�ʧ��
     	{
     		DEBUGOUT("ClientDbTableInit:Drop %s Failure!\r\n", CLIENTTBL_HEAD);
			  sqlite3_free_table(sqlres.dbResult);
			  return res;
     	}
  	}

    sprintf(sql, "CREATE TABLE %s%s;", tblname, CLIENTTBL_HEAD);
    res = SqliteCreate(sql);//����sqtbl���ݱ�
    if(res != SQLITE_OK)//����sqtbl���ݱ�ʧ��
    {
      DEBUGOUT("ClientDbTableInit:Sqlite Create %s Failure!\r\n", tblname);
      result = res;
    }
    else
    {
      //����server��Ϣ
      GetSelfMac("eth0", ipbuf);
      MACToStr(ipbuf, macbuf);
      selfip = GetSelfIp("eth0");
			memcpy(&saddr.sin_addr, &selfip, 4);
  		inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
			//���ݱ�����������Ԫ��Ϣ,(Fd,DevNo,Mac,IP,DevType,LastComTime)
			sprintf(sql, "INSERT INTO %s(DevNo,Mac,IP,DevType) VALUES (0,\'%s\',\'%s\',1);", CLIENT_TBL, macbuf, ipbuf);
			SqliteInsert(sql);//��¼�������ݿ�
      DEBUGOUT("ClientDbTableInit:Sqlite Create %s Success!\r\n", tblname);
      result = 1;
    }
  }
  else
  {
  	result = res;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}
/*********************************End Of File*************************************/
