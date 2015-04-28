/********************  COPYRIGHT(C)***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: approtocol.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��:
**���򿪷�������
**��        ��: �й��ƶ�����㣨AP�����������ѯ���ó���
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
#include "../common/commonfun.h"
#include "../net/netcom.h"
#include "../sqlite/drudatabase.h"
#include "approtocol.h"
#include "mcpb_protocol.h"
#include <sys/wait.h>
#include <sys/types.h>
extern int system_time_config(void);
extern int g_OMCLoginFlag, g_OMCHeartBeatFlag, g_OMCSetParaFlag;
extern int g_OmcBroadcastFlag, g_DevType;;
extern volatile int	g_alarm_report_cnt;
extern volatile int	g_alarm_report_time; 
extern DevicePara_t g_DevicePara;
extern int settimeflag;
extern int g_OMCSetParaRs485Flag;

/*******************************************************************************
*�������� : void  ClearAPPackBuf(APPack_t *p_packbuf)
*��    �� : ���ͨѶ����Ϊ��ʼ״̬
*������� : None
*������� : None
*******************************************************************************/
void ClearAPPackBuf(APPack_t *p_packbuf)
{
/*
  p_packbuf->StartFlag = ' '; //��ͨѶ��ʼ��־����
  p_packbuf->EndFlag = ' ';   //��ͨѶֹͣ��־����
  p_packbuf->ComFlag = 0; 		//ͨѶ״̬:����//COMIDLE;
  p_packbuf->ComBufOverflow = 0;	//ͨѶ�����������־
  p_packbuf->PtrComWr = &p_packbuf->APType;  //ָ����һ�ַ����������λ��
  p_packbuf->PtrComRd = &p_packbuf->APType;  //ָ����һ�ַ�������ȡ��λ��
  p_packbuf->PackLen = 0;  //ͨѶ�������ַ���
  memset(p_packbuf->PackValue, 0, PACK_VALUE_SIZE);	//�����ջ���ȫ��Ϊ0
*/
  memset(p_packbuf, 0, sizeof(APPack_t));//���ͨѶ���ݰ�����
}

/*******************************************************************************
*�������� : int GetDevInfo(DevInfo_t *p_devinfo, APPack_t *p_packbuf)
*��    �� : �����ݰ��н���DEVICETYPE_ID��Ϣ
*������� : DevInfo_t *p_devinfo:�豸��Ϣ�ṹ
*           APPack_t *p_packbuf:Э�����ݰ��ṹָ��
*������� : �������ݵ�Ԫ��ʼ��ַ
*******************************************************************************/
int GetDevInfo(DevInfo_t *p_devinfo, APPack_t *p_packbuf)
{
int objectid;

  p_devinfo->DeviceNo = p_packbuf->DeviceNo;
  objectid = p_packbuf->PackValue[1] + (p_packbuf->PackValue[2] * 0x100);
  //�豸�����ж�
  if (objectid == DEVICETYPE_ID)//��ѯ�豸��ģ��
  {
    p_devinfo->ModuleAddr = p_packbuf->PackValue[3];//Զ�˵�Ԫ����ģ��
    p_devinfo->ModuleType = p_packbuf->PackValue[4] + (p_packbuf->PackValue[5] * 0x100);
    p_devinfo->PduStart = 6;
  }
  else//�豸����
  {
    p_devinfo->ModuleAddr = 0;
    p_devinfo->ModuleType = 0;
    p_devinfo->PduStart = 0;
  }
  return p_devinfo->PduStart;
}

/*******************************************************************************
*�������� : APPack(APPack_t *p_packbuf, char *sdbuf)
*��    �� : �����й��ƶ�APЭ�飬��Э�����ݰ����д���ĺ���
*������� : APPack_t *p_packbuf��Э�����ݰ��ṹָ��
*           char  *sdbuf:��������ݻ���
*������� : �������Ҫ���͵����ݰ�����
*******************************************************************************/
int APPack(APPack_t *p_packbuf, char *sdbuf)
{
int sd, rd;

  sd = 0;
  sdbuf[sd++] = p_packbuf->StartFlag; //ͨѶ��ʼ��־����
  sdbuf[sd++] = p_packbuf->APType;		//AP:��Э������
  sdbuf[sd++] = p_packbuf->VPType;		//VP:A����Э������
  sdbuf[sd++] = (char)p_packbuf->StationNo;//վ����
  sdbuf[sd++] = (char)(p_packbuf->StationNo/0x100);
  sdbuf[sd++] = (char)(p_packbuf->StationNo/0x10000);
  sdbuf[sd++] = (char)(p_packbuf->StationNo/0x1000000);
  sdbuf[sd++] = p_packbuf->DeviceNo;	    //�豸���
  sdbuf[sd++] = (char)p_packbuf->PackNo;	//ͨѶ����ʶ��
  sdbuf[sd++] = (char)(p_packbuf->PackNo/0x100);
  sdbuf[sd++] = p_packbuf->VPInteractFlag;  //VP�㽻����־
  sdbuf[sd++] = p_packbuf->MCPFlag;			    //MCP��Э���ʶ
  sdbuf[sd++] = p_packbuf->CommandFlag;	    	//�����ʶ
  sdbuf[sd++] = p_packbuf->ResponseFlag;	  	//Ӧ���־
	//����
  for(rd = 0; rd < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); rd++)
    sdbuf[sd++] = p_packbuf->PackValue[rd];

  p_packbuf->CRCData = CCITT_CRC16(&sdbuf[1], (p_packbuf->PackLen - AP_MSG_UNCRC_LEN));//CRCУ��
  sdbuf[sd++] = (char)p_packbuf->CRCData;//CRCУ����
  sdbuf[sd++] = (char)(p_packbuf->CRCData/0x100);
  sdbuf[sd++] = p_packbuf->EndFlag;
  
  return sd;
}

/*******************************************************************************
*�������� : int	APUnpack(char *rcbuf, int rcsum, APPack_t *p_packbuf)
*��    �� : �����һ�����ݽ���ʱ�����ô˺����������й��ƶ�APЭ����
*������� : char  *rcbuf:�������ݻ���
*						int   rcsum������������
*						APPack_t *p_packbuf�����հ���ʽ������������
*������� : ��ȷ,���ݰ�����,���󷵻�MSG_CRC_ERR
*******************************************************************************/
int	APUnpack(char *rcbuf, int rcsum, APPack_t *p_packbuf)
{
int j;

  ClearAPPackBuf(p_packbuf);
  p_packbuf->PackLen = rcsum;//ͨѶ���ݰ�����
  p_packbuf->CRCData = rcbuf[rcsum-3] + rcbuf[rcsum-2]*0x100;//CRCУ����
  p_packbuf->EndFlag = rcbuf[rcsum-1];//ͨѶֹͣ��־����
  //AP���CRCУ��,������ʱӦ�����ð�
  if(CCITT_CRC16(&rcbuf[1], (rcsum-AP_MSG_UNCRC_LEN)) == p_packbuf->CRCData)
  {
	  p_packbuf->StartFlag = rcbuf[0];    //ͨѶ��ʼ��־����
	  p_packbuf->APType = rcbuf[1];     //AP��Э������
	  p_packbuf->VPType = rcbuf[2];     //VP:A����Э������
	  p_packbuf->StationNo = rcbuf[3] + rcbuf[4]*0x100//վ����
		     				 				 + rcbuf[5]*0x10000 + rcbuf[6]*0x1000000;
	  p_packbuf->DeviceNo = rcbuf[7];		//�豸���
	  p_packbuf->PackNo = rcbuf[8] + rcbuf[9]*0x100;//ͨѶ����ʶ��
	  p_packbuf->VPInteractFlag = rcbuf[10];   //VP�㽻����־
	  p_packbuf->MCPFlag = rcbuf[11];	     //MCP��Э���ʶ
	  p_packbuf->CommandFlag = rcbuf[12];  //�����ʶ
	  p_packbuf->ResponseFlag = rcbuf[13]; //Ӧ���־

    for(j = 0; j < (rcsum - AP_MSG_HEAD_TAIL_LEN); j++)
		  p_packbuf->PackValue[j] = rcbuf[14 + j];//����,������Ҫȷ�������ݻ����ַ��� 
		return p_packbuf->PackLen;
	}
	else
	  return MSG_CRC_ERR;
}

/*******************************************************************************
*�������� : int APProcess(APPack_t *p_packbuf, DevicePara_t *p_dev)
*��    �� : �����һ�����ݽ���ʱ�����ô˺����������й��ƶ�Э����
*           ����������ȷ���Ǳ�ģ������ݣ�����MSG_RIGHT����������й��ƶ���ͨ�Ŵ��������ȼ����ش�����Ϣ
*           ������Ϣ��APЭ�����ʹ���AP CRCУ�����AP���ز����VPվ���Ŵ����豸��Ŵ���
*           VP������־����VP���ؿ��Ʋ�Э�����ͨ�Ű�ȫ����.........
*������� : APPack_t *p_packbuf���������ݰ�����ṹָ��
*           DevicePara_t *p_dev:�����豸����
*������� : ������,�����ȷ������ݴ���ķ��ش�����Ϣ
*******************************************************************************/
int APProcess(APPack_t *p_packbuf, DevicePara_t *p_dev)
{
int i;

  //AP��Э�����ͳ���,������ʱӦ�����ð�
  if (p_packbuf->APType == AP_B )
  {
  	if(p_packbuf->StartFlag != APB_STARTFLAG)
  	{
    	ClearAPPackBuf(p_packbuf);
    	return MSG_AP_ERR;
    }
  }
  if (p_packbuf->APType == AP_C )
  {
  	if(p_packbuf->StartFlag != APC_STARTFLAG)
  	{
    	ClearAPPackBuf(p_packbuf);
    	return MSG_AP_ERR;
    }
  }
  //(2)AP��ĳ���Э������,������ʱӦ�����ð�
  if(p_packbuf->VPType != VP_A )
  {
    ClearAPPackBuf(p_packbuf);
    return  MSG_VP_ERR;
  }
  //(3)VP���վ����У��,������ʱӦ�����ð�
  //if ((p_packbuf->StationNo != BROADCAST_STATION)//�㲥վ����200.200.200
   // &&(p_packbuf->StationNo != p_dev->StationNo))
  if (p_packbuf->StationNo != p_dev->StationNo)//�㲥վ����200.200.200
  {
    ClearAPPackBuf(p_packbuf);//վ���Ȩʧ�ܣ����յ���������Ӧ
    return MSG_STATION_ERR;
  }
  //�豸���У��,����豸��ӦΪ��չ��Ԫ�������豸���У��,��������ԪԶ�̼�ص�Ԫ��У��
  //if ((p_packbuf->DeviceNo != BROADCAST_DEV)//�㲥�豸���0xFF
   //&& (p_packbuf->DeviceNo != p_dev->DeviceNo))
   if (p_packbuf->DeviceNo != p_dev->DeviceNo)
  {
    ClearAPPackBuf(p_packbuf);//�豸��Ȩʧ�ܣ����յ���������Ӧ
    return MSG_DEV_ERR;
  }

  //(4)VP�㽻����־��������ʱӦ�����ð�
  if ((p_packbuf->VPInteractFlag != VP_INTERACT_NORMAL)  //ͨ�������ִ�н������ʾִ������
    &&(p_packbuf->VPInteractFlag != VP_INTERACT_REQUEST))//��������������
    //&&(p_packbuf->VPInteractFlag != VP_INTERACT_BUSY)    //ͨ�������ִ�н������ʾ�豸æ���޷�������������
  {
    ClearAPPackBuf(p_packbuf);
    return MSG_VP_INTERACT_ERR;
  }
  //(5)VP��ļ�ؿ��Ʋ�Э���ʶ��������ʱӦ�����ð�
  if ((p_packbuf->MCPFlag != MCP_A)
    &&(p_packbuf->MCPFlag != MCP_B)
    &&(p_packbuf->MCPFlag != MCP_C))
  {
    ClearAPPackBuf(p_packbuf);
    return MSG_MCP_ERR;
  }
  //�豸��֧��MCP:A Э�飬����0x05����
  if (p_packbuf->MCPFlag == MCP_A)
  {
  	p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;//ͨѶ����ִ������,VP�㽻����־,0x00
    p_packbuf->ResponseFlag = RESPONSE_MCPAC_ERR;
    return MSG_QUERY_OK;
  }
  //(6)ͨ�Ű�ȫ�����绰�����Ȩ����������ʱӦ�����ð�
  
  //(7)MCP�����Ԫ�е�Ӧ���־������Ϊ0xFFʱӦ�����ð�
  if ((p_packbuf->ResponseFlag != RESPONSE_COMMAND)   //0xFF
    &&(p_packbuf->ResponseFlag != RESPONSE_SUCCESS))  //0x00
  {
    ClearAPPackBuf(p_packbuf);
    return MSG_MCP_RESPONSE_ERR;
  }
  //(8)MCP�����ݵ�Ԫ�еļ�������ܳ���
  //���ȴ�,���յ���ʵ�ʳ��������ݰ��������Ĳ�������в������Ĳ�����
	for (i = 0; i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN);)
 	{
 		if (p_packbuf->MCPFlag == MCP_B)
 		{
 			i = (p_packbuf->PackValue[i]+p_packbuf->PackValue[i+1]*0x100) + i;
 		}
 		else
 			i = p_packbuf->PackValue[i] + i;
 	}
 	if (i != (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN))
 	{
 		p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;//ͨѶ����ִ������,VP�㽻����־,0x00
 		p_packbuf->ResponseFlag = RESPONSE_LENGTH_ERR;
 		return MSG_QUERY_OK;
 	}

  if (p_packbuf->MCPFlag == MCP_A) //0x01
  {
    if (p_packbuf->CommandFlag != COMMAND_REPORT)//�ϱ��ظ������ݵ�Ԫ���������ȼ������
    {
      //���á���ѯ������г��ȼ������
    }
  }
  //(9)MCP�����Ԫ�е������ʶ
  //�ϱ����� 
  if (p_packbuf->CommandFlag == COMMAND_REPORT)
  {
    //����Ϊ����,���յ��ӻ��ϱ�����,��Ҫ�����ϱ�����,��Ҫ������
    if (p_packbuf->ResponseFlag == RESPONSE_COMMAND)
    {
      if (p_packbuf->VPInteractFlag == VP_INTERACT_REQUEST)
      {
        ReportCommand(p_packbuf);//�ϱ����ݰ�
        return MSG_REPORT_OK;
      }
      else
        return MSG_REPORT_ERR;
    }
    //����Ϊ�ӻ�,�������ϱ������������ϱ��������ķ��ذ�
    else if (p_packbuf->ResponseFlag == RESPONSE_SUCCESS)
    {
      ReportCommandBack(p_packbuf);//�ϱ��������ݰ�
      return MSG_REPORTBACK_OK;
    }
  }
  //��ѯ����(MCP_A:��ѯ����;MCP_B:��ȡ����)
  else if (p_packbuf->CommandFlag == COMMAND_QUERY)
  {
    //MCP_A:��ѯ����
    if (p_packbuf->MCPFlag == MCP_A)
    {
      //����Ϊ�ӻ�,���յ���ѯ����,��Ҫ��ȡ����,�������
      if (p_packbuf->ResponseFlag == RESPONSE_COMMAND)
      {
        QueryCommand(p_packbuf);//��ѯ����
        return MSG_QUERY_OK;
      }
      //����Ϊ����,���յ���ѯ���������,������Ҫд�뵽���ݿ�
      else if (p_packbuf->ResponseFlag == RESPONSE_SUCCESS)
      {
        QueryCommandBack(p_packbuf);//��ѯ,���ݷ���
        return MSG_QUERYBACK_OK;
      }
    }
    //MCP_B:��ȡ����,�������ʹ��
	else if (p_packbuf->MCPFlag == MCP_B)
	{
		printf("recv mcp_b read data\n");
		MCP_B_QueryCommand(p_packbuf);//��������ش�,������ѯ
		return MSG_SW_UPDATE_QUERY_OK;
	}
	else if (p_packbuf->MCPFlag == MCP_C)
	{
		//����Ϊ�ӻ�,���յ���ѯ����,��Ҫ��ȡ����,�������
		if (p_packbuf->ResponseFlag == RESPONSE_COMMAND)
		{
			QueryCommand_MCP_C(p_packbuf);//��ѯ����
			return MSG_QUERY_OK;
		}
	}
  }
  //��������, MCP_A:��������;MCP_B:д������
  else if (p_packbuf->CommandFlag == COMMAND_SET)
  {
    //MCP_A:��������
    if (p_packbuf->MCPFlag == MCP_A)
    {
      //����Ϊ�ӻ�,���յ���������,��Ҫ���ò���,���������
      if (p_packbuf->ResponseFlag == RESPONSE_COMMAND)
      {
        SetCommand(p_packbuf);//��������
        return MSG_SET_OK;
      }
      //����Ϊ����,���յ��������������
      else if (p_packbuf->ResponseFlag == RESPONSE_SUCCESS)
      {
        SetCommandBack(p_packbuf);//��ѯ�����ݷ���
        return MSG_SETBACK_OK;
      }
    }
    //MCP_B:д������0x03���������ʹ��
    else if (p_packbuf->MCPFlag == MCP_B)
    {
		printf("recv mcp_b set data\n");
      MCP_B_SetCommand(p_packbuf);//�������
      return MSG_SW_UPDATE_OK;
    }
		else if(p_packbuf->MCPFlag == MCP_C)
		{
			//����Ϊ�ӻ�,���յ���������,��Ҫ���ò���,���������
			if (p_packbuf->ResponseFlag == RESPONSE_COMMAND)
			{
				SetCommand_MCP_C(p_packbuf);//��������
				return MSG_SET_OK;
			}
		}
  }
  //ת�����������ģʽ
  else if (p_packbuf->CommandFlag == COMMAND_SW_UPDATE_MOD)
  {
    TurnToUpdateMode(p_packbuf);//�л�������״̬
    return MSG_SW_UPDATE_MOD_OK;
  }
  //�л��������汾
  else if(p_packbuf->CommandFlag == COMMAND_SWVERISONSWITCH)
  {
    SWVerisonSwitch(p_packbuf);//�л��������汾
    return MSG_SW_UPDATE_VER_OK;
  }
  // �����Ŵ�,��Ч������
  else
  {
  	p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;//ͨѶ����ִ������,VP�㽻����־,0x00
    p_packbuf->ResponseFlag = RESPONSE_COMMANDNO_ERR;
    return MSG_QUERY_OK;
//  	p_packbuf->ResponseFlag = RESPONSE_COMMANDNO_ERR;
    //ClearAPPackBuf(p_packbuf);//�����ʶ��,��Ч������
    //return MSG_MCP_COMMAND_ERR;
  }
  return MSG_PACK_ERR;
}

/*******************************************************************************
*�������� : int QueryIDList(DevInfo_t *p_devinfo, INT16U pdustart, APPack_t *p_packbuf)
*��    �� : �����������͵Ĳ�ѯ��������,��ȡ��������б�,��ѯ����洢��*p_packbuf��
*������� : DevInfo_t *p_devinfo:�豸���,�豸ģ���ַ,�豸ģ������
*           INT16U start:���ݴ洢��ʼλ��
*           APPack_t *p_packbuf���ƶ�Э�����ݰ����ݻ���ṹָ��
*������� : �����������ݰ����ȣ����󷵻�<0
*******************************************************************************/
int QueryIDList(DevInfo_t *p_devinfo, INT16U pdustart, APPack_t *p_packbuf)
{
int idlist[250];
int i, totalsum, idmaxsum, sdsum;

  if(p_packbuf->APType == AP_B)
    idmaxsum = APB_IDPACK_MAX_LEN/2;
  else
    idmaxsum = APC_IDPACK_MAX_LEN/2;

  memset(idlist, 0, sizeof(idlist));
  totalsum = DbGetIDList(p_devinfo, idlist);//�������б�
  if(totalsum < 0)//��ѯ����
  {
    DEBUGOUT("Dev:%d,Maddr:%d,Mtype:%d IDList Query Failure!\r\n", p_devinfo->DeviceNo, p_devinfo->ModuleAddr, p_devinfo->ModuleType);
    return -3;
  }
 DEBUGOUT("DbGetIDList totalsum:%d!\r\n",totalsum);
  //��һ�β�ѯ,�����ܰ���
  if(p_packbuf->PackValue[pdustart+3] == 1)
  {
    p_packbuf->PackValue[pdustart+3] = (totalsum+idmaxsum-1)/idmaxsum;
  }
  //�Ѿ�������������
  sdsum = (p_packbuf->PackValue[pdustart+4]-1)*idmaxsum;

  //���ݵ�Ԫ���ݳ��� ������ݳ���(1)+������ݱ�ʶ(2)+�ܵĲ�ѯ����(1)��ǰ�Ĳ�ѯ���(1)
  p_packbuf->PackValue[pdustart] = 0x05;//��ʼ����,xx00090101
  for(i = 0; i < idmaxsum; i++)//�����б����� ���ݳ���(1)��ѯID(2)�ܰ���(1)��ѯ�����(1)
  {
    if(sdsum < totalsum)
    {
      p_packbuf->PackValue[pdustart+5+2*i] =(char)idlist[sdsum];
      p_packbuf->PackValue[pdustart+6+2*i] = (char)(idlist[sdsum]>>8);
      p_packbuf->PackValue[pdustart] = p_packbuf->PackValue[pdustart]+2;//���ݳ��ȼ�2
    }
    else
      break;
    sdsum++;
  }
  //�����б����ݰ�����
  p_packbuf->PackLen = p_packbuf->PackValue[pdustart]+pdustart+AP_MSG_HEAD_TAIL_LEN;
  return p_packbuf->PackLen;
}
int QueryIDList_MCP_C(DevInfo_t *p_devinfo, INT16U pdustart, APPack_t *p_packbuf)
{
unsigned int idlist[1024];
int i, totalsum, idmaxsum, sdsum;

  if(p_packbuf->APType == AP_B)
    idmaxsum = APB_IDPACK_MAX_LEN/4;
  else
    idmaxsum = APC_IDPACK_MAX_LEN/4;

  memset(idlist, 0, sizeof(idlist));
  totalsum = DbGetIDList_MCP_C(p_devinfo, idlist);//�������б�
  if(totalsum < 0)//��ѯ����
  {
    DEBUGOUT("Dev:%d,Maddr:%d,Mtype:%d IDList Query Failure!\r\n", p_devinfo->DeviceNo, p_devinfo->ModuleAddr, p_devinfo->ModuleType);
    return -3;
  }
 DEBUGOUT("DbGetIDList totalsum:%d!\r\n",totalsum);
  //��һ�β�ѯ,�����ܰ���
  if(p_packbuf->PackValue[pdustart+5] == 1)
  {
    p_packbuf->PackValue[pdustart+5] = (totalsum+idmaxsum-1)/idmaxsum;
  }
  //�Ѿ�������������
  sdsum = (p_packbuf->PackValue[pdustart+6]-1)*idmaxsum;
  //���ݵ�Ԫ���ݳ��� ������ݳ���(1)+������ݱ�ʶ(4)+�ܵĲ�ѯ����(1)��ǰ�Ĳ�ѯ���(1)
  p_packbuf->PackValue[pdustart] = 0x07;//��ʼ����,xx00090101
  for(i = 0; i < idmaxsum; i++)//�����б����� ���ݳ���(1)��ѯID(2)�ܰ���(1)��ѯ�����(1)
  {
    if(sdsum < totalsum)
    {
      p_packbuf->PackValue[pdustart+7+4*i] =(char)idlist[sdsum];
      p_packbuf->PackValue[pdustart+8+4*i] = (char)(idlist[sdsum]>>8);
      p_packbuf->PackValue[pdustart+9+4*i] = (char)(idlist[sdsum]>>16);
      p_packbuf->PackValue[pdustart+10+4*i] = (char)(idlist[sdsum]>>24);
      p_packbuf->PackValue[pdustart] = p_packbuf->PackValue[pdustart]+4;//���ݳ��ȼ�4
    }
    else
      break;
    sdsum++;
  }
  //�����б����ݰ�����
  p_packbuf->PackLen = p_packbuf->PackValue[pdustart]+pdustart+AP_MSG_HEAD_TAIL_LEN;
  return p_packbuf->PackLen;
}
/*******************************************************************************
*�������� : int QuerySampleData(APPack_t *p_packbuf)
*��    �� : ��ȡ��������
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : �����������ݰ����ȣ����󷵻�0
*******************************************************************************/
int QuerySampleData(APPack_t *p_packbuf)
{
  p_packbuf->PackLen = 0;
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int QueryLog(APPack_t *p_packbuf)
*��    �� : ��ȡ��־����
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : �����������ݰ����ȣ����󷵻�0
*******************************************************************************/
int QueryLog(APPack_t *p_packbuf)
{
  p_packbuf->PackLen = 0;
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int QueryParaValue(DevInfo_t *p_devinfo, INT16U objectid, char *p_buf)
*��    �� : �����ݿ��в�ѯobjectid��Ӧ�����ݣ����ز�ѯ����洢��*buf��
*������� : DevInfo_t *p_devinfo:�豸���,�豸ģ���ַ,�豸ģ������
*           objectid�����ݱ�ţ�p_buf���ݻ���
*������� : ��������1,���ݿ����޶�Ӧ��������-1
*******************************************************************************/
int QueryParaValue(DevInfo_t *p_devinfo, INT16U objectid, char *p_buf)
{
Pdu_t pdu;

	if(DbGetParaValue(p_devinfo, objectid, &pdu) == 1)
	{
		if(*p_buf == (pdu.len+AP_PDU_HEAD_LEN))
		{
		  if (strcmp(pdu.var_type, "bit") == 0)//bit������
		    *(p_buf+AP_PDU_HEAD_LEN) = (pdu.var[0] & 0x01);
		  else
		  {
		    memcpy((p_buf+AP_PDU_HEAD_LEN), pdu.var, pdu.len);
		    if (objectid == DEVICETIME_ID)	//�豸��ǰʱ��,7���ֽ����
    		{
    		  GetDevTime(p_buf+AP_PDU_HEAD_LEN);
    		}
		  }
		}
		else
		{
			*(p_buf+2) |= 0x40;//4:������ݱ�ʶ�������ݳ��Ȳ�ƥ��
		}
		return 1;
	}
	else//δ�鵽������,������ݱ�ʶ�޷�ʶ�����
	{
		*(p_buf+2) |= 0x10;//1:������ݱ�ʶ�޷�ʶ��
		DEBUGOUT("Dev:%d,Maddr:%d,Mtype:%d,ID:%d no Find!\r\n", 
		         p_devinfo->DeviceNo, p_devinfo->ModuleAddr, p_devinfo->ModuleType, objectid);
		return -1;
	}
}

int get_idx(int id)
{
	int ret = 0;

	switch((id>>12)&0xf){
		case 1:
			ret = 0;
			break;
		case 4:
			ret = 1;
			break;
		case 7:
			ret = 2;
			break;
		case 8:
			ret = 3;
			break;
	}
	return ret;
}
// ��ѯMCPC����
extern int ru_dout_pw[8][4]; // �����������
extern int g_din_pw[4];      // �������빦��
int QueryParaValue_MCP_C(DevInfo_t *p_devinfo, unsigned int objectid, char *p_buf)
{
	Pdu_t pdu;
	int val = 0;
	int val1, val2;
	int idx = 0;
	int port_num = 0;

	if(DbGetParaValue_MCP_C(p_devinfo, objectid, &pdu) == 1)
	{
		if(*p_buf == (pdu.len+AP_PDU_HEAD_LEN))
		{
			if (strcmp(pdu.var_type, "bit") == 0)//bit������
				*(p_buf+AP_PDU_HEAD_LEN) = (pdu.var[0] & 0x01);
			else
			{
				memcpy((p_buf+AP_PDU_HEAD_LEN), pdu.var, pdu.len);
				if (objectid == DEVICETIME_ID)	//�豸��ǰʱ��,7���ֽ����
				{
					GetDevTime(p_buf+AP_PDU_HEAD_LEN);
				}
			}
		}
		else
		{
			*(p_buf+4) &= 0x0F;
			*(p_buf+4) |= MCPVAR_LEN_ERR;//4:������ݱ�ʶ�������ݳ��Ȳ�ƥ��
		}
		if((objectid == 0x08001001) || (objectid == 0x08004001) 
				|| (objectid == 0x08007001) || (objectid == 0x08008001)){
			if(g_DevType == MAIN_UNIT){
				// �������빦�ʷ�Χ���ޣ����޷����
				DbGetThisIntPara(PW_DOWN_ALARM_ID, &val1);
				DbGetThisIntPara(PW_ERROR_ALARM_ID, &val2);
				if(((val1&0x1) == 1) || ((val2&0x1) == 1)){ // ��Դ��������
					*(p_buf+4) &= 0x0F;
					*(p_buf+4) |= MCPVAR_DATA_ERR;//7: �޷����
				}else{
					val = signed_1to4(pdu.var[0]);
					if(val > 5){
						*(p_buf+4) &= 0x0F;
						*(p_buf+4) |= MCPVAR_UP_ERR;//6: ��������
					}else if(val < -15){
						*(p_buf+4) &= 0x0F;
						*(p_buf+4) |= MCPVAR_UNDER_ERR;//5: ��������
					}
				}
			}
		}else if((objectid&0xffff0ff0) == 0x08000100){ // Զ������������ʳ��ޣ����޷����
			if(g_DevType == EXPAND_UNIT){
				DbGetThisIntPara(PW_DOWN_ALARM_ID, &val1);
				DbGetThisIntPara(PW_ERROR_ALARM_ID, &val2);
				if(((val1&0x1) == 1) || ((val2&0x1) == 1)){ // ��Դ��������
					*(p_buf+4) &= 0x0F;
					*(p_buf+4) |= MCPVAR_DATA_ERR;//7: �޷����
				}else{
					val = signed_1to4(pdu.var[0]);
					if(val > 29){
						*(p_buf+4) &= 0x0F;
						*(p_buf+4) |= MCPVAR_UP_ERR;//6: ��������
					}else if(val < 14){
						*(p_buf+4) &= 0x0F;
						*(p_buf+4) |= MCPVAR_UNDER_ERR;//5: ��������
					}
				}
			}
		}else if((objectid&0xffff0ff0) == 0x08000200){ // Զ������ʵ�����泬�ޣ����޷����
			if(g_DevType == EXPAND_UNIT){
				idx = get_idx(objectid);
				port_num = (objectid&0xf);
				DbGetThisIntPara(PW_DOWN_ALARM_ID, &val1);
				DbGetThisIntPara(PW_ERROR_ALARM_ID, &val2);
				if(((val1&0x1) == 1) || ((val2&0x1) == 1) // ��Դ��������
					|| (signed_1to4(g_din_pw[idx]) > 5) || (signed_1to4(g_din_pw[idx]) < -15) // ���빦�ʳ���
					|| (signed_1to4(ru_dout_pw[port_num][idx]) > 29) || (signed_1to4(ru_dout_pw[port_num][idx]) < 9)){ // ������ʳ���
					*(p_buf+4) &= 0x0F;
					*(p_buf+4) |= MCPVAR_DATA_ERR;//7: �޷����
				}
			}
		}else if(((objectid&0xfffffff0) == 0x08000500)||(objectid == 0x0000087c)){ // �豸�¶��޷����
			//if(g_DevType == EXPAND_UNIT){
				DbGetThisIntPara(PW_DOWN_ALARM_ID, &val1);
				DbGetThisIntPara(PW_ERROR_ALARM_ID, &val2);
				if(((val1&0x1) == 1) || ((val2&0x1) == 1)){ // ��Դ��������
					*(p_buf+4) &= 0x0F;
					*(p_buf+4) |= MCPVAR_DATA_ERR;//7: �޷����
				}
			//}
		}
			
		return 1;
	}
	else//δ�鵽������,������ݱ�ʶ�޷�ʶ�����
	{
		*(p_buf+4) &= 0x0F;
		*(p_buf+4) |= MCPVAR_NOID_ERR;//1:������ݱ�ʶ�޷�ʶ��
		DEBUGOUT("Dev:%d,Maddr:%d,Mtype:%d,ID:%d no Find!\r\n", 
				p_devinfo->DeviceNo, p_devinfo->ModuleAddr, p_devinfo->ModuleType, objectid);
		return 1;
	}
}
/*******************************************************************************
*�������� : int SetParaValue(DevInfo_t *p_devinfo, INT16U objectid, char *buf)
*��    �� : ����objectid��Ӧ�����ݣ�д���ݿ�
*������� : DevInfo_t *p_devinfo:�豸���,�豸ģ���ַ,�豸ģ������
*           objectid�����ݱ�ţ�buf���ݻ���
*������� : ��������1,���ݿ����޶�Ӧ��������-1
*******************************************************************************/
int SetParaValue(DevInfo_t *p_devinfo, INT16U objectid, char *buf)
{
Pdu_t pdu;
INT32U ip, res;

  memset(pdu.var, 0, sizeof(pdu.var));
	pdu.id = objectid; 
	pdu.len = *buf - 5 ;
	printf("pdu.len=%d\n", pdu.len);
	memcpy(pdu.var, (buf + AP_PDU_HEAD_LEN), pdu.len);
	res = DbSaveParaValue(p_devinfo, objectid, &pdu);
	if(res == 1)//�������óɹ�
	{
		if ((objectid == OMCIP_ID) || (objectid == OMCIPPORT_ID) || (objectid ==  GPRSAPN_ID)
		 || (objectid == GPRSUSER_ID) || (objectid == GPRSPASSWORD_ID) || (objectid == DEVICEIP_ID))
		{
		  memcpy(&ip, &pdu.var, pdu.len);
		  if (objectid == DEVICEIP_ID)
		  {
		    SetDevIPpara("eth0", ip, SET_IP);
		  }
		  if (objectid == DEVNETMASK_ID)
		  {
		    SetDevIPpara("eth0", ip, SET_NETMASK);
		  }
		  if (objectid == DEVDEFAULTGW_ID)
		  {
		    SetDevIPpara("eth0", ip, SET_GATEWAY);
		  }
			//gprs����
		}
		if (objectid == DEVICETIME_ID)	//�豸��ǰʱ��,7���ֽ����
		{
		  SetDevTime(pdu.var);
		  system_time_config();
		}
		return  1;
	}
	else//���ò��������д�
	{
		if(pdu.id != objectid)//δ�鵽������,������ݱ�ʶ�޷�ʶ�����
		{
			*(buf + 2) |= 0x10;//1:������ݱ�ʶ�޷�ʶ��
			return -1;
		}
		if((*buf - 3) != pdu.len)
		{
			*(buf + 2) |= 0x40;//4:������ݱ�ʶ�������ݳ��Ȳ�ƥ��
			return -1;
		}
		if (res == -6)
		{
			*(buf+4) &= 0x0F;
			*(buf+4) |= MCPVAR_OVER_ERR; //�豸������ô���,����Χ2��������ݵ�����ֵ������Χ
			return  1;
		}
	}
}
// ����MCPC����
int SetParaValue_MCP_C(DevInfo_t *p_devinfo, unsigned int objectid, char *buf, int flag)
{
	int i;
	Pdu_t pdu;
	INT32U ip;
	int val = 0;

	memset(pdu.var, 0, sizeof(pdu.var));
	pdu.id = objectid; 
	pdu.len = *buf - 5 ;
	memcpy(pdu.var, (buf + AP_PDU_HEAD_LEN), pdu.len);

	if(objectid == DEVICENO_ID)
	{
		if (g_DevType == MAIN_UNIT)
		{
			if(*(buf+5) > 0)
			{
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //�豸������ô���,����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}
		}
		else
		{
			if((*(buf+5) == 0xff) || (*(buf+5) == 0x0))
			{
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //�豸������ô���,����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}
		}
	}
	else if(objectid == 0x00000142){
		printf("\n\n>>>>>>>>>>>>>>>>>>>>>>\n");
		printf("objectid=0x%08x, val=%d\n", objectid, *(buf+5));
		if((*(buf+5) < 1) || (*(buf+5) > 2))
		{
			printf("ser id=0x%08x error\n", objectid);
			*(buf+4) &= 0x0F;
			*(buf+4) |= MCPVAR_OVER_ERR; //ͨ�ŷ�ʽ����,����Χ2��������ݵ�����ֵ������Χ
			return  1;
		}
	}
	//else if((objectid & (~(INT32U)0x2ff))==0x200)
	//else if((objectid & 0xffffff00)==0x200)
	else if((objectid == 0x00000201))
	{
		if(*(buf+5) > 1)
		{
			*(buf+4) &= 0x0F;
			*(buf+4) |= MCPVAR_OVER_ERR; //�豸������ô���,����Χ2��������ݵ�����ֵ������Χ
			return  1;
		}
	}else if(objectid == 0x00000172){ // �豸�����������ó�����Χ
		val = signed_1to4(*(buf+5));
		if((val > 125)||(val < -40)){
			*(buf+4) &= 0x0F;
			*(buf+4) |= MCPVAR_OVER_ERR; //����Χ2��������ݵ�����ֵ������Χ
			return  1;
		}
	}else if((objectid == 0x08001007) || (objectid == 0x08004007) || (objectid == 0x08007007) 
		  || (objectid == 0x08008007) || (objectid == 0x08001008) || (objectid == 0x08004008) 
		  || (objectid == 0x08007008) || (objectid == 0x08008008)) { // �������빦���������ó���
		if (g_DevType == MAIN_UNIT){
			val = signed_1to4(*(buf+5));
			if((val > 5)||(val < -15)){
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}
		}
	}else if((objectid == 0x08001009) || (objectid == 0x08004009) || (objectid == 0x08007009) 
			|| (objectid == 0x08008009) || (objectid == 0x0800100a) || (objectid == 0x0800400a) 
			|| (objectid == 0x0800700a) || (objectid == 0x0800800a)) { // ������������������ó���
		if (g_DevType == EXPAND_UNIT){
			val = signed_1to4(*(buf+5));
			if((val > 29)||(val < -14)){
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}
		}
	}else if(objectid == 0x00000872){ // �������������������ó���
		if (g_DevType == EXPAND_UNIT){
			val = signed_1to4(*(buf+5));
			printf("*buf+5=0x%02x, val=%d\n\n\n\n", *(buf+5), val);
			if((val > -40)||(val < -108)){
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}

		}
	}else if((objectid == 0x08001005)||(objectid == 0x08004005)||(objectid == 0x08007005)||
			 (objectid == 0x08008005)||(objectid == 0x08001006)||(objectid == 0x08004006)||
			 (objectid == 0x08007006)||(objectid == 0x08008006))	//����˥��ֵ
	{
		if (g_DevType == MAIN_UNIT)
		{
			val = signed_1to4(*(buf+5));
			if((val > 15)||(val < 0))
			{
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}
		}
	}else if(((objectid&0xffff0fff) == 0x08000300)||((objectid&0xffff0fff) == 0x08000301)||((objectid&0xffff0fff) == 0x08000302)||
			 ((objectid&0xffff0fff) == 0x08000303)||((objectid&0xffff0fff) == 0x08000304)||((objectid&0xffff0fff) == 0x08000305)||
			 ((objectid&0xffff0fff) == 0x08000306)||((objectid&0xffff0fff) == 0x08000307))	//Զ������˥��ֵ
	{
		if (g_DevType == EXPAND_UNIT)
		{
			val = signed_1to4(*(buf+5));
			if((val > 15)||(val < 0))
			{
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}
		}
	}else if(((objectid&0xffff0fff) == 0x08000380)||((objectid&0xffff0fff) == 0x08000381)||((objectid&0xffff0fff) == 0x08000382)||
			 ((objectid&0xffff0fff) == 0x08000383)||((objectid&0xffff0fff) == 0x08000384)||((objectid&0xffff0fff) == 0x08000385)||
			 ((objectid&0xffff0fff) == 0x08000386)||((objectid&0xffff0fff) == 0x08000387))	//Զ������˥��ֵ
	{
		if (g_DevType == EXPAND_UNIT)
		{
			val = signed_1to4(*(buf+5));
			if((val > 15)||(val < 0))
			{
				*(buf+4) &= 0x0F;
				*(buf+4) |= MCPVAR_OVER_ERR; //����Χ2��������ݵ�����ֵ������Χ
				return  1;
			}
		}
	}
	//�жϵ绰�����Ƿ��ʺ�
	else if((objectid == QUERYTEL_ID) || (objectid == (QUERYTEL_ID+1)) || (objectid == (QUERYTEL_ID+2))
			||(objectid == (QUERYTEL_ID+3)) || (objectid == (QUERYTEL_ID+4)) || (objectid == NOTIFYTEL_ID))
	{
		for(i=0;i<20;i++)
		{
			if ((*(buf+5+i) < '0') || (*(buf+5+i) > '9'))
				//			if((('0'-1) < *(buf+5+i)) && (*(buf+5+i) < ('9'+1)))
			{
				if (*(buf+5+i) != 0x00)
				{
					*(buf+4) &= 0x0F;
					*(buf+4) |= MCPVAR_CODE_ERR; //�豸������ô���,����Χ2��������ݵ�����ֵ������Χ
					return  1;
				}
			}
		}
	}
	if(DbSaveParaValue_MCP_C(p_devinfo, objectid, &pdu, flag) == 1)//�������óɹ�
	{
		/*
		   if ((objectid == OMCIP_ID) || (objectid == OMCIPPORT_ID) || (objectid ==  GPRSAPN_ID)
		   || (objectid == GPRSUSER_ID) || (objectid == GPRSPASSWORD_ID) || (objectid == DEVICEIP_ID))
		   {
		   memcpy(&ip, &pdu.var, pdu.len);
		   if (objectid == DEVICEIP_ID)
		   {
		   SetDevIPpara("eth0", ip, SET_IP);
		   }
		   if (objectid == DEVNETMASK_ID)
		   {
		   SetDevIPpara("eth0", ip, SET_NETMASK);
		   }
		   if (objectid == DEVDEFAULTGW_ID)
		   {
		   SetDevIPpara("eth0", ip, SET_GATEWAY);
		   }
		//gprs����
		}
		*/
		if (objectid == DEVICETIME_ID)	//�豸��ǰʱ��,7���ֽ����
		{
			SetDevTime(pdu.var);
			system_time_config();
		}
		if((g_DevType == EXPAND_UNIT) && (objectid == DEVICENO_ID)){
		//if((objectid == DEVICENO_ID)){
			UDPComThreadInit();
			UDPComThreadStart();
		}
		return 1;
	}
	else//���ò��������д�
	{
		if(pdu.id != objectid)//δ�鵽������,������ݱ�ʶ�޷�ʶ�����
			*(buf + 4) |= MCPVAR_NOID_ERR;//1:������ݱ�ʶ�޷�ʶ��
		if((*buf - 5) != pdu.len)
			*(buf + 4) |= MCPVAR_LEN_ERR;//4:������ݱ�ʶ�������ݳ��Ȳ�ƥ��
		return 1;
	}
}
/*******************************************************************************
*�������� : int APHeadPack(DevInfo_t *p_devinfo, int aptype, int commandid, int packno, APPack_t *p_packbuf)
*��    �� : �������𣬴ӻ����豸�ڲ�ģ������ͨѶ��ͷ���
*������� : DevInfo_t *p_devinfo:վ����,�豸���,�豸ģ���ַ,�豸ģ������
*           int aptype:APЭ������ 
*           int commandid�������ʶ����ѯ�����á��ϱ�......
*           int packno�����ݰ���
*           APPack_t *p_packbuf���ƶ�Э�����ݰ����ݻ���ṹָ��
*������� : �������ݵ�Ԫ��ʼ��ַ,���󷵻�-1
*******************************************************************************/
int APHeadPack(DevInfo_t *p_devinfo, int aptype, int commandid, int packno, APPack_t *p_packbuf)
{
int pvlen;

  if(aptype == AP_A)
  {
    p_packbuf->APType =AP_A;                //AP��Э������,�����Э��:AP:AЭ������0x01
    p_packbuf->StartFlag = APA_STARTFLAG;   //ͨѶ��ʼ��־
    p_packbuf->EndFlag = APA_ENDFLAG;       //ͨѶֹͣ��־
  }
  else if(aptype == AP_B)
  {
    p_packbuf->APType =AP_B;                //AP��Э������,�����Э��:AP:BЭ������0x02
    p_packbuf->StartFlag = APB_STARTFLAG;   //ͨѶ��ʼ��־
    p_packbuf->EndFlag = APB_ENDFLAG;       //ͨѶֹͣ��־
  }
  else if(aptype == AP_C)
  {
    p_packbuf->APType =AP_C;                //AP��Э������,�����Э��:AP:CЭ������0x03
    p_packbuf->StartFlag = APC_STARTFLAG;   //ͨѶ��ʼ��־
    p_packbuf->EndFlag = APC_ENDFLAG;       //ͨѶֹͣ��־
  }
  else
  {
    DEBUGOUT("AP Type Error!\r\n");
    return -1;//AP��Э�����ʹ���
  }

  p_packbuf->VPType = VP_A;                   //3 ���ز�Э������:VP:A0x01
  p_packbuf->StationNo = p_devinfo->StationNo;//4 վ���� uint4,�㲥վ����:200.200.200
  p_packbuf->DeviceNo = p_devinfo->DeviceNo;  //5 �豸��� uint1
  p_packbuf->PackNo = packno;                 //6 ͨ�Ű���ʶ�� uint2
  p_packbuf->VPInteractFlag = VP_INTERACT_REQUEST;//7 VP�㽻����־0x80
  p_packbuf->MCPFlag = MCP_C;                 //8 ��ؿ��Ʋ�Э��:MCP_A,0x01
  p_packbuf->CommandFlag = commandid;         //9 �����ʶ
  p_packbuf->ResponseFlag = RESPONSE_COMMAND; //10 Ӧ���־,����𷽸��ֶ����0xFF
  if (p_devinfo->ModuleType != 0)
  {
    pvlen = 0;
    p_packbuf->PackValue[pvlen++] = 0x06;//���ݳ���
    p_packbuf->PackValue[pvlen++] = (char)DEVICETYPE_ID;//0x06D2
    p_packbuf->PackValue[pvlen++] = (char)(DEVICETYPE_ID >> 8);
    p_packbuf->PackValue[pvlen++] = p_devinfo->ModuleAddr;
    p_packbuf->PackValue[pvlen++] = (char)p_devinfo->ModuleType;
    p_packbuf->PackValue[pvlen++] = (char)(p_devinfo->ModuleType >> 8);
    p_devinfo->PduStart = pvlen;
  }
  else
  {
    p_devinfo->PduStart = 0;
  }
  return p_devinfo->PduStart;
}

/*******************************************************************************
*�������� : int QueryCommand(APPack_t *p_packbuf)
*��    �� : ��ѯ������Ӧ�������Բ�ѯ����������ݴ��������ݰ�Ҫ����õ�ͨѶ���ݰ��в�����.
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : ���ݰ�����
*******************************************************************************/
int QueryCommand(APPack_t *p_packbuf)
{
int objectlen, objectid, pdustart, result, i;
DevInfo_t devinfo;

  //���¸�������ڻظ�ʱ���䣬��ʼ��������־,AP��Э������,VP:A����Э������,վ����,�豸���,ͨѶ����ʶ�Ų���,MCP��Э���ʶ
  //StartFlag, EndFlag, APType, VPType, StationNo, DeviceNo, PackNo, MCPFlag
  //���·����ݵ�Ԫ�������Э������޸�
  p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;// ͨѶ����ִ������,VP�㽻����־,0x00
  p_packbuf->CommandFlag = COMMAND_QUERY;// �����ʶ:��ѯ
  //p_packbuf->ResponseFlag = RESPONSE_SUCCESS;// Ӧ���־:�ɹ�
  p_packbuf->ResponseFlag = 0x05;// Ӧ���־: mcp ����֧��MCP:C

    return  p_packbuf->PackLen;

  pdustart = GetDevInfo(&devinfo, p_packbuf);

  objectlen = p_packbuf->PackValue[pdustart];//���ݳ���
  objectid = p_packbuf->PackValue[pdustart+1] + (p_packbuf->PackValue[pdustart+2] * 0x100);
  if (objectid == IDLIST_ID) //�豸����б��ѯ
  {
    return QueryIDList(&devinfo, pdustart, p_packbuf);//��ȷ��ѯ������б�
  }
  else if (objectid == QUERYSAMPLEDATA_ID)//�������ݲ�ѯ
  {
    return QuerySampleData(p_packbuf); //��ȡ��������
  }
  else if (objectid == QUERYLOG_ID)//��־���ݲ�ѯ
  {
    return QueryLog(p_packbuf); //��־����
  }
  else  //����������ѯ
  {
    result = 0;
    //���ݵ�Ԫ����Э������ID�Ž����޸�
    for (i = pdustart; i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); )//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
    {
	  	if (p_packbuf->PackValue[i] == 0)//���ݳ��ȴ���
	  	{
	  		DEBUGOUT("QueryCommand:PackValue Len Error...");
	  		goto _QUERYFAILURE;
	  	}
      //���ݳ���
      objectlen = p_packbuf->PackValue[i] - 3;
      // ���ݵ�Ԫ���ݱ�ʶ,ID
      objectid = p_packbuf->PackValue[i + 1] + (p_packbuf->PackValue[i + 2] * 0x100);
      //��ȡ���ݿ���ID��Ӧֵ
      if (QueryParaValue(&devinfo, objectid, &p_packbuf->PackValue[i]) > 0)
        result = 1;
      i = i + p_packbuf->PackValue[i];//���ݵ�Ԫ���ݳ���
    }
    if (result == 1)//��ѯ���ݳɹ�
    {
      return  p_packbuf->PackLen;
    }
    else
    {
_QUERYFAILURE:
      DEBUGOUT("Query Command Failure!\r\n");
      ClearAPPackBuf(p_packbuf);
      return 0;
    }
  }
}
int is_alarm(int id)
{
	if(((id&0xfffffff0) == 0x08000060) || ((id&0xfffffff0) == 0x08000600) || ((id&0xffff0ff0) == 0x08000480) 
			|| ((id&0xfffffff0) == 0x08001580) || ((id&0xfffffff0) == 0x08004580) || ((id&0xfffffff0) == 0x08007580)
			|| ((id&0xfffffff0) == 0x08008580) || ((id&0xffffff00) == 0x00000300) || ((id&0xffff0fff) == 0x0800000e)
			|| ((id&0xffff0fff) == 0x0800000f) || ((id > 0x00000050) && (id < 0x00000075))){
		printf("is alarm id\n");
		return 1;
	}
	return 0;
}
int QueryCommand_MCP_C(APPack_t *p_packbuf)
{
	unsigned int objectlen, objectid, pdustart;
	int result, i;
	DevInfo_t devinfo;
	int tmp;

	//���¸�������ڻظ�ʱ���䣬��ʼ��������־,AP��Э������,VP:A����Э������,վ����,�豸���,ͨѶ����ʶ�Ų���,MCP��Э���ʶ
	//StartFlag, EndFlag, APType, VPType, StationNo, DeviceNo, PackNo, MCPFlag
	//���·����ݵ�Ԫ�������Э������޸�
	p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;// ͨѶ����ִ������,VP�㽻����־,0x00
	p_packbuf->CommandFlag = COMMAND_QUERY;// �����ʶ:��ѯ
	p_packbuf->ResponseFlag = RESPONSE_SUCCESS;// Ӧ���־:�ɹ�

	pdustart = GetDevInfo(&devinfo, p_packbuf);

	objectlen = p_packbuf->PackValue[pdustart];//���ݳ���
	//objectid = p_packbuf->PackValue[pdustart+1] + (p_packbuf->PackValue[pdustart+2] * 0x100);
	objectid = p_packbuf->PackValue[pdustart+1] + ((p_packbuf->PackValue[pdustart+2]) <<  8) + 
		((p_packbuf->PackValue[pdustart+3]) << 16) + ((p_packbuf->PackValue[pdustart+4]) << 24); 
	if (objectid == IDLIST_ID) //�豸����б��ѯ
	{
		return QueryIDList_MCP_C(&devinfo, pdustart, p_packbuf);//��ȷ��ѯ������б�
	}
	else if (objectid == QUERYSAMPLEDATA_ID)//�������ݲ�ѯ
	{
		return QuerySampleData(p_packbuf); //��ȡ��������
	}
	else if (objectid == QUERYLOG_ID)//��־���ݲ�ѯ
	{
		return QueryLog(p_packbuf); //��־����
	}
	else  //����������ѯ
	{
		result = 0;
		//���ݵ�Ԫ����Э������ID�Ž����޸�
		for (i = pdustart; i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); )//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
		{
			if (p_packbuf->PackValue[i] == 0)//���ݳ��ȴ���
			{
				DEBUGOUT("QueryCommand:PackValue Len Error...");
				goto _QUERYFAILURE;
			}
			//���ݳ���
			objectlen = p_packbuf->PackValue[i] - 5;
			// ���ݵ�Ԫ���ݱ�ʶ,ID
			//objectid = p_packbuf->PackValue[i + 1] + (p_packbuf->PackValue[i + 2] * 0x100);
			objectid = p_packbuf->PackValue[i+1] + ((p_packbuf->PackValue[i+2]) <<  8) + 
				((p_packbuf->PackValue[i+3]) << 16) + ((p_packbuf->PackValue[i+4]) << 24); 
			//��ȡ���ݿ���ID��Ӧֵ
			if (QueryParaValue_MCP_C(&devinfo, objectid, &p_packbuf->PackValue[i]) > 0)
				result = 1;
			if(is_alarm(objectid)){
				DbGetThisIntPara(objectid, &tmp);
				printf("id=0x%08x, val=%d\n", objectid, tmp);
				if(tmp == 1){
					printf("val=1, write=129\n");
					p_packbuf->PackValue[i+5] = 129;
					SetParaValue_MCP_C(&devinfo, objectid, &p_packbuf->PackValue[i], 0); 
				}else if(tmp == 128){
					printf("val=128, write=0\n");
					p_packbuf->PackValue[i+5] = 0;
					SetParaValue_MCP_C(&devinfo, objectid, &p_packbuf->PackValue[i], 0); 
				}
			}
			i = i + p_packbuf->PackValue[i];//���ݵ�Ԫ���ݳ���
		}
		if (result == 1)//��ѯ���ݳɹ�
		{
			return  p_packbuf->PackLen;
		}
		else
		{
_QUERYFAILURE:
			DEBUGOUT("Query Command Failure!\r\n");
			ClearAPPackBuf(p_packbuf);
			return 0;
		}
	}
}

/*******************************************************************************
*�������� : int QueryCommandBack(APPack_t *p_packbuf)
*��    �� : ��ѯ�ӻ��������ش�������ѯ����д����Ӧ���ݿ���
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : ��ѯ���ݸ�����ȷ����1,��Ҫ������ѯ�������ݰ�����,���󷵻�-1
*******************************************************************************/
int QueryCommandBack(APPack_t *p_packbuf)
{
int objectlen, objectid, pdustart, i, j;
DevInfo_t devinfo;
Pdu_t pdu;

  pdustart = GetDevInfo(&devinfo, p_packbuf);
  
  objectlen = p_packbuf->PackValue[pdustart];//���ݳ���
  objectid = p_packbuf->PackValue[pdustart+1] + (p_packbuf->PackValue[pdustart+2] * 0x100);
  if(objectid == IDLIST_ID)//�豸����б���,�����ݿ��вμӲ���
  {
    //�����б���
    if (p_packbuf->PackValue[pdustart+3] < p_packbuf->PackValue[pdustart+4])//ID��ѯ����С��ID�ܰ���
    {
      QueryIDPack(&devinfo, p_packbuf->PackNo++, p_packbuf->PackValue[pdustart+3]++, p_packbuf->PackValue[pdustart+4], p_packbuf);
      return p_packbuf->PackLen;
    }
    else
    {
      ClearAPPackBuf(p_packbuf);
      return 1;//��ɲ����б��ѯ
    }
  }
  else//���ݲ�ѯ����
  {
    for (i = pdustart; i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); )//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
    {
      //���ݳ���
      pdu.len = p_packbuf->PackValue[i] - 3;
      // ���ݵ�Ԫ���ݱ�ʶ,ID
      pdu.id = p_packbuf->PackValue[i+1] + (p_packbuf->PackValue[i+2] * 0x100);
      memset(pdu.var, 0, sizeof(pdu.var));
      for(j = 0; j < pdu.len; j++)
		    pdu.var[j] = p_packbuf->PackValue[i+3+j];
      //��ȡ���ݿ���ID��Ӧֵ
      if (DbSaveParaValue(&devinfo, objectid, &pdu) != 1)//�������ò��ɹ�
      {
        DEBUGOUT("Query ID:%d Update Failure!\r\n", objectid);
      }
      i = i + p_packbuf->PackValue[i];//���ݵ�Ԫ���ݳ���
    }
    ClearAPPackBuf(p_packbuf);
    return 1;
  }
}

/*******************************************************************************
*�������� : int QueryIDPack(DevInfo_t *p_devinfo, int packno, int packsum, APPack_t *p_packbuf)
*��    �� : ��ѯdevno_port_mtype�ĵ�packno������б�����
*������� : DevInfo_t *p_devinfo:վ����,�豸���,�豸ģ���ַ,�豸ģ������
*           packno:��ѯ�����
*           idqueryno:�����б����(���),idpacksum:�����б��ܰ���
*           APPack_t *p_packbuf���ƶ�Э�����ݰ����ݻ���ṹָ��
*������� : ������ѯ�������ݰ�����;�����豸����-1
*******************************************************************************/
int QueryIDPack(DevInfo_t *p_devinfo, int packno, int idpackno, int idpacksum, APPack_t *p_packbuf)
{
int pvlen;

  //����Э�����ͷ
  pvlen = APHeadPack(p_devinfo, AP_C, COMMAND_QUERY, packno, p_packbuf);
  if (pvlen < 0)
  {
    DEBUGOUT("QueryIDPack Error!\r\n");
    return -1;//AP��Э�����ʹ���
  }
  //���ݵ�Ԫ�������
  p_packbuf->PackValue[pvlen++] = 0x05;
  p_packbuf->PackValue[pvlen++] = 0x09;
  p_packbuf->PackValue[pvlen++] = 0x00;
  p_packbuf->PackValue[pvlen++] = idpackno;
  p_packbuf->PackValue[pvlen++] = idpacksum;

  //���ݰ�����
  p_packbuf->PackLen = pvlen + AP_MSG_HEAD_TAIL_LEN;//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int QueryParaPack(DevInfo_t *p_devinfo, int packno, int *p_idlist, APPack_t *p_packbuf)
*��    �� : ��ѯdeviceno_port_mtype����
*������� : DevInfo_t *p_devinfo:վ����,�豸���,�豸ģ���ַ,�豸ģ������
*           packno:�����
*           int *p_idlist:��ѯ����id�б�
*           APPack_t *p_packbuf���ƶ�Э�����ݰ����ݻ���ṹָ��
*������� : ������ѯ�������ݰ�����;�����豸����-1
*******************************************************************************/
int QueryParaPack(DevInfo_t *p_devinfo, int packno, int *p_idlist, APPack_t *p_packbuf)
{
int pdustart, pvlen, i;
Pdu_t pdu;

  //����Э�����ͷ
  pdustart = APHeadPack(p_devinfo, AP_C, COMMAND_QUERY, packno, p_packbuf);
  if (pdustart < 0)
  {
    DEBUGOUT("QueryParaPack Error!\r\n");
    return -1;//AP��Э�����ʹ���
  }

  memset(&p_packbuf->PackValue[pdustart], 0, (sizeof(p_packbuf->PackValue)-pdustart));
  pvlen = pdustart;
  while (*p_idlist != 0x00)
  {
    if (DbGetParaValue(p_devinfo, *p_idlist, &pdu) == 1)
    {
      p_packbuf->PackValue[pvlen++] = pdu.len + AP_PDU_HEAD_LEN;
      p_packbuf->PackValue[pvlen++] = (char)pdu.id;
      p_packbuf->PackValue[pvlen++] = (char)(pdu.id >> 8);
      for (i = 0; i < pdu.len; i++)
        p_packbuf->PackValue[pvlen++] = 0;
    }
    if (pvlen > (APC_MSG_MAX_LEN-pdustart-AP_MSG_HEAD_TAIL_LEN))
    {
      DEBUGOUT("QueryParaPackLen More than APC_MSG_MAX_LEN!\r\n");
      break;
    }
    p_idlist++;
  }
  //�����б����ݰ�����
  p_packbuf->PackLen = pvlen + pdustart + AP_MSG_HEAD_TAIL_LEN;
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int SetCommand(APPack_t *p_packbuf)
*��    �� : ����������Ӧ����������������������ݴ��������ݰ�Ҫ����õ�ͨѶ���ݰ��в�����.
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : ���ݰ�����
*******************************************************************************/
int SetCommand(APPack_t *p_packbuf)
{
int objectlen, objectid, pdustart, result, i;
DevInfo_t devinfo;

  //���¸�������ڻظ�ʱ���䣬��ʼ��������־,AP��Э������,VP:A����Э������,վ����,�豸���,ͨѶ����ʶ�Ų���,MCP��Э���ʶ
  //StartFlag, EndFlag, APType, VPType, StationNo, DeviceNo, PackNo, MCPFlag

  //���·����ݵ�Ԫ�������Э������޸�
  p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;// ͨѶ����ִ������,VP�㽻����־,0x00
  p_packbuf->CommandFlag = COMMAND_SET; // �����ʶ:����
  p_packbuf->ResponseFlag = RESPONSE_SUCCESS;// Ӧ���־:�ɹ�

  pdustart = GetDevInfo(&devinfo, p_packbuf);

  result = 0;
  //���ݵ�Ԫ����Э������ID�Ž����޸�
  for(i = pdustart; i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); )//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
  {
  	if (p_packbuf->PackValue[i] == 0)//���ݳ��ȴ���
  	{
  		DEBUGOUT("SetCommand:PackValue Len Error...");
  		goto _SETFAILURE;
  	}
    //���ݳ���
    objectlen = p_packbuf->PackValue[i] - 3;
    // ���ݵ�Ԫ���ݱ�ʶ,ID
    objectid = p_packbuf->PackValue[i + 1] + (p_packbuf->PackValue[i + 2] * 0x100);
    //��ȡ���ݿ���ID��Ӧֵ
    if (SetParaValue(&devinfo, objectid, &p_packbuf->PackValue[i]) > 0)
      result = 1;
    i = i + p_packbuf->PackValue[i];//���ݵ�Ԫ���ݳ���
  }
  if (result)//�������ݳɹ�
  {
    g_OMCSetParaFlag = 1;
    return  p_packbuf->PackLen;
  }
  else
  {
_SETFAILURE:
    DEBUGOUT("Set Command Failure!\r\n");
    ClearAPPackBuf(p_packbuf);
    return 0;
  }
}

// MCPC���ݴ���
int SetCommand_MCP_C(APPack_t *p_packbuf)
{
	unsigned int objectlen, objectid, pdustart;
	int result, i;
	DevInfo_t devinfo;
	int temp;
	char cbuf[128];

	//���¸�������ڻظ�ʱ���䣬��ʼ��������־,AP��Э������,VP:A����Э������,վ����,�豸���,ͨѶ����ʶ�Ų���,MCP��Э���ʶ
	//StartFlag, EndFlag, APType, VPType, StationNo, DeviceNo, PackNo, MCPFlag

	//���·����ݵ�Ԫ�������Э������޸�
	p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;// ͨѶ����ִ������,VP�㽻����־,0x00
	p_packbuf->CommandFlag = COMMAND_SET; // �����ʶ:����
	p_packbuf->ResponseFlag = RESPONSE_SUCCESS;// Ӧ���־:�ɹ�

	pdustart = GetDevInfo(&devinfo, p_packbuf);

	result = 0;
	//���ݵ�Ԫ����Э������ID�Ž����޸�
	for(i = pdustart; i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); )//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
	{
		if (p_packbuf->PackValue[i] == 0)//���ݳ��ȴ���
		{
			DEBUGOUT("SetCommand:PackValue Len Error...");
			goto _SETFAILURE;
		}
		//���ݳ���
		objectlen = p_packbuf->PackValue[i] - 5;
		// ���ݵ�Ԫ���ݱ�ʶ,ID
		objectid = p_packbuf->PackValue[i+1] + ((p_packbuf->PackValue[i+2]) <<  8) + 
			((p_packbuf->PackValue[i+3]) << 16) + ((p_packbuf->PackValue[i+4]) << 24); 
		//��ȡ���ݿ���ID��Ӧֵ

		cbuf[0] = objectlen+AP_PDU_HEAD_LEN;
		QueryParaValue_MCP_C(&devinfo, objectid, cbuf);
		if(memcmp(&cbuf[5],&p_packbuf->PackValue[5], objectlen) != 0)
		{
			if((g_DevType == EXPAND_UNIT) && (objectid == 0x00000101)){   // վ����
				result = 1;
			}else{
				if (SetParaValue_MCP_C(&devinfo, objectid, &p_packbuf->PackValue[i], 0) > 0)
					result = 1;
			}
		}
		else
		{
			result = 1;
		}
		i = i + p_packbuf->PackValue[i];//���ݵ�Ԫ���ݳ���
		temp = SqliteGetPro(&devinfo, objectid); // ��ȡ�����ֶΣ����жϸ�λ�Ƿ���1�����򴥷��㲥
		if(temp > 0)
		{    // �㲥����
			if((temp % 10) == 1)
			{
				printf("broadcast para\n");
				g_OmcBroadcastFlag = 1;
				result = 1;
			}
		}
	}
	if (result)//�������ݳɹ�
	{
		g_OMCSetParaFlag = 1;
		g_OMCSetParaRs485Flag = 1;
		return  p_packbuf->PackLen;
	}
	else
	{
_SETFAILURE:
		DEBUGOUT("Set Command Failure!\r\n");
		ClearAPPackBuf(p_packbuf);
		return 0;
	}
}
/*******************************************************************************
*�������� : int SetCommandBack(APPack_t *p_packbuf)
*��    �� : APЭ����������ش�����
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : �����������ȷ����1,��Ҫ������ѯ�������ݰ�����,���󷵻�-1
*******************************************************************************/
int SetCommandBack(APPack_t *p_packbuf)
{
  ClearAPPackBuf(p_packbuf);
  return 1;
}

/*******************************************************************************
*�������� : void SetPack(int APType, int notify_type, struct cmcc_pack *p_packbuf)
*��    �� : APЭ���豸��Ϣ�ϱ�����,������ݱ�ʶ:0x0141
*������� : APType:APЭ������;int NotifyType:�ϱ�������;struct cmcc_pack *p_packbuf:ͨѶ���ݰ����ݽṹָ��
           struct ras_sys *psys:�����ṹָ��
*������� : None
*******************************************************************************/
/*******************************************************************************
*�������� : int SetParaPack(DevInfo_t *p_devinfo, int packno, char *buf, APPack_t *p_packbuf)
*��    �� : ����deviceno_port_mtype����
*������� : DevInfo_t *p_devinfo:վ����,�豸���,�豸ģ���ַ,�豸ģ������
*           packno:�����
*           char *buf:���ò���buf����
*           APPack_t *p_packbuf���ƶ�Э�����ݰ����ݻ���ṹָ��
*������� : �����������ݰ�����;�����豸����-1
*******************************************************************************/
int SetParaPack(DevInfo_t *p_devinfo, int packno, char *p_buf, int buflen, APPack_t *p_packbuf)
{
int pdustart, pvlen;

  //����Э�����ͷ
  pdustart = APHeadPack(p_devinfo, AP_C, COMMAND_SET, packno, p_packbuf);
  if (pdustart < 0)
  {
    DEBUGOUT("QueryParaPack Error!\r\n");
    return -1;//AP��Э�����ʹ���
  }

  memset(&p_packbuf->PackValue[pdustart], 0, (sizeof(p_packbuf->PackValue)-pdustart));
  pvlen = pdustart;
  while (buflen--)
  {
    p_packbuf->PackValue[pvlen++] = *p_buf--;
  }
  //�����б����ݰ�����
  p_packbuf->PackLen = pvlen + pdustart + AP_MSG_HEAD_TAIL_LEN;
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int ReportCommand(APPack_t *p_packbuf)
*��    �� : ��Ϊ�����Դӻ��ϱ�������Ӧ���������ϱ�������Ӧ���д���
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : ���ݰ�����
*******************************************************************************/
int ReportCommand(APPack_t *p_packbuf)
{
int objectlen, objectid, pdustart, i, reporttype;
DevInfo_t devinfo;

  //���¸�������ڻظ�ʱ���䣬��ʼ��������־,AP��Э������,VP:A����Э������,վ����,�豸���,ͨѶ����ʶ�Ų���,MCP��Э���ʶ
  //StartFlag, EndFlag, APType, VPType, StationNo, DeviceNo, PackNo, MCPFlag
  //���·����ݵ�Ԫ�������Э������޸�
  p_packbuf->VPInteractFlag = VP_INTERACT_NORMAL;// ͨѶ����ִ������,VP�㽻����־,0x00
  p_packbuf->CommandFlag = COMMAND_REPORT;// �����ʶ:�ϱ�
  p_packbuf->ResponseFlag = RESPONSE_SUCCESS;// Ӧ���־:�ɹ�

  pdustart = GetDevInfo(&devinfo, p_packbuf);

  reporttype = 0;
  objectlen = p_packbuf->PackValue[pdustart];//���ݳ���
  objectid = p_packbuf->PackValue[pdustart+1] + (p_packbuf->PackValue[pdustart+2] * 0x100);
  if (objectid == REPORTTYPE_ID)//�ϱ�����,��ȷ�ϱ�������(���ϱ�ʱ���������ݵ�Ԫ����ǰ��)
  {
    reporttype = p_packbuf->PackValue[pdustart+3];
    switch(reporttype)
    {
      case REPORT_ALARM://1:�澯�ϱ�
        //����ӻ��澯�ϱ�
        for (i = (pdustart+4); i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); )//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
        {
          //���ݳ���
          objectlen = p_packbuf->PackValue[i] - 3;
          // ���ݵ�Ԫ���ݱ�ʶ,ID
          objectid = p_packbuf->PackValue[i + 1] + (p_packbuf->PackValue[i + 2] * 0x100);
          /*if (SetObjectValue(devno, port, mtype, ObjectId, &p_packbuf->PackValue[i]) > 0)
            result = 1;*/
          i = i + p_packbuf->PackValue[i];//���ݵ�Ԫ���ݳ���
        }
        return p_packbuf->PackLen;
      break;
      case REPORT_CONFIG_CREATE://2:��վ�ϱ�
      break;
      case REPORT_CHECK://3:Ѳ���ϱ�
      break;
      case REPORT_RESTORE://4:�����޸��ϱ�
      break;
      case REPORT_CONFIG_CHANGE://5:���ñ���ϱ�
      break;
      case REPORT_LOGIN_OMC://6:��¼����������ϱ�
      break;
      case REPORT_HEART_BEAT://7:�����ϱ�
      break;
      case REPORT_SW_UPDATE://8:�豸�����������ϱ�
      break;
      case REPORT_PSLOGIN_FAIL://9:GPRS��¼ʧ���ϱ�
      break;
      case REPORT_SAMPLEJOB_END://10:���ɽ����ϱ�
      break;
      default:
        DEBUGOUT("Invalid Report Command!\r\n");
        ClearAPPackBuf(p_packbuf);
      break;
    }
  }
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int ReportCommandBack(APPack_t *p_packbuf)
*��    �� : �������ϱ����ش����������ϱ����д����Ӧ���ݿ���
*������� : APPack_t *p_packbuf:ͨѶ���ݰ����ݽṹָ��
*������� : ���ݰ�����
*******************************************************************************/
int ReportCommandBack(APPack_t *p_packbuf)
{
int objectlen, objectid, pdustart, i, reporttype;
DevInfo_t devinfo;

  pdustart = GetDevInfo(&devinfo, p_packbuf);

  objectlen = p_packbuf->PackValue[pdustart];//���ݳ���
  objectid = p_packbuf->PackValue[pdustart+1] + ((p_packbuf->PackValue[pdustart+2]) <<  8) + 
				((p_packbuf->PackValue[pdustart+3]) << 16) + ((p_packbuf->PackValue[pdustart+4]) << 24); 

  if (objectid == REPORTTYPE_ID)//�ϱ�����,��ȷ�ϱ�������(���ϱ�ʱ���������ݵ�Ԫ����ǰ��)
  {
    reporttype = p_packbuf->PackValue[pdustart+5];
	  switch(reporttype)
	  {
	    case REPORT_ALARM://1:�澯�ϱ�
			//�����Ѿ���ɵĸ澯�ϱ�,������ϱ�����
			for (i = (pdustart+6); i < (p_packbuf->PackLen - AP_MSG_HEAD_TAIL_LEN); )//AP_MSG_HEAD_TAIL_LEN:17,���ݰ�����Э���ֽ���(�����ݵ�Ԫ����������)
			{
				if (p_packbuf->PackValue[i] == 0)//���ݳ��ȴ���
				{
					DEBUGOUT("ReportCommandBack:PackValue Len Error...");
					goto _REPORTBACKFAILURE;
				}
				//���ݳ���
				objectlen = p_packbuf->PackValue[i] - 5;
				// ���ݵ�Ԫ���ݱ�ʶ,ID
				objectid = p_packbuf->PackValue[i+1] + ((p_packbuf->PackValue[i+2]) <<  8) + 
					((p_packbuf->PackValue[i+3]) << 16) + ((p_packbuf->PackValue[i+4]) << 24); 

				if (p_packbuf->PackValue[i+5] == 1)//�澯�ϱ��ɹ�
				{
					p_packbuf->PackValue[i+5] = p_packbuf->PackValue[i+5] | ALARMSUCCESSFLAG;
					printf("recv alarm back!!!\n");
				}
				else if(p_packbuf->PackValue[i+5] == 0)//�澯�ָ��ϱ��ɹ�
				{
					p_packbuf->PackValue[i+5] = 0;
				}
				SetParaValue_MCP_C(&devinfo, objectid, &p_packbuf->PackValue[i], 0);
				i = i + p_packbuf->PackValue[i];//���ݵ�Ԫ���ݳ���
				g_alarm_report_cnt = 0;
				g_alarm_report_time = 0xffffffff;
			}
_REPORTBACKFAILURE:
        ClearAPPackBuf(p_packbuf);
        return p_packbuf->PackLen;
	    break;
      case REPORT_CONFIG_CREATE://2:��վ�ϱ�
      case REPORT_CHECK://3:Ѳ���ϱ�
      case REPORT_RESTORE://4:�����޸��ϱ�
      case REPORT_CONFIG_CHANGE://5:���ñ���ϱ�
      break;
	    case REPORT_LOGIN_OMC://6:��¼����������ϱ�
	      g_OMCLoginFlag = 1;//��½�ɹ�
	    break;
	    case REPORT_HEART_BEAT://7:�����ϱ�
	      g_OMCHeartBeatFlag = 0;
	    break;
      case REPORT_SW_UPDATE://8:�豸�����������ϱ�
      break;
	    case REPORT_PSLOGIN_FAIL://9:GPRS��¼ʧ���ϱ�
	    break;
	    case REPORT_SAMPLEJOB_END://10:���ɽ����ϱ�
		case 11:
	    break;
      default:
        DEBUGOUT("Invalid Report Command!\r\n");
        ClearAPPackBuf(p_packbuf);
        return 0;
      break;
	  }
    ClearAPPackBuf(p_packbuf);
  }
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int ReportParaPack(int APType, DevInfo_t *p_devinfo, int reporttype, int packno, APPack_t *p_packbuf)
*��    �� : APЭ���豸��Ϣ�ϱ����ݴ������,������ݱ�ʶ:0x0141
*������� : APType:APЭ������;DevInfo_t *p_devinfo:վ����,�豸���,�豸ģ���ַ,�豸ģ������
*           int reporttype:�ϱ�������;packno:�����;
*           APPack_t *p_packbuf���ƶ�Э�����ݰ����ݻ���ṹָ��
*������� : ������ѯ�������ݰ�����;�����豸����-1
*******************************************************************************/
extern void set_alarm(unsigned int id, unsigned int val, int en);
int ReportParaPack(int APType, DevInfo_t *p_devinfo, int reporttype, int packno, APPack_t *p_packbuf)
{
int i, pvlen, allen;
char var[20];

  //����Э�����ͷ
  pvlen = APHeadPack(p_devinfo, APType, COMMAND_REPORT, packno, p_packbuf);
  //0x0141:�ϱ�
  p_packbuf->PackValue[pvlen++] = 6;//���ݳ���
  p_packbuf->PackValue[pvlen++] = (char)REPORTTYPE_ID;//������ݱ�ʶ
  p_packbuf->PackValue[pvlen++] = (char)(REPORTTYPE_ID >> 8);
  p_packbuf->PackValue[pvlen++] = (char)(REPORTTYPE_ID >> 16);
  p_packbuf->PackValue[pvlen++] = (char)(REPORTTYPE_ID >> 24);
  p_packbuf->PackValue[pvlen++] = reporttype;//�ϱ�������
  switch(reporttype)
  {
    case REPORT_ALARM://1:�澯�ϱ�
      //��ȡ�澯����
      allen = DbGetAlarmValue(p_devinfo, &p_packbuf->PackValue[pvlen]);
      if (allen > 0)
      {
      	p_packbuf->PackLen = pvlen + allen + AP_MSG_HEAD_TAIL_LEN;
      }
      else
      {
      	p_packbuf->PackLen = 0;
      }
    break;
    case REPORT_CONFIG_CREATE://2:��վ�ϱ�
    case REPORT_CHECK://3:Ѳ���ϱ�
    case REPORT_RESTORE://4:�����޸��ϱ�                    
		if(reporttype == REPORT_RESTORE){
			set_alarm(BAT_ERROR_ALARM_ID, 0, 0);
		}
    case REPORT_CONFIG_CHANGE://5:���ñ���ϱ�                
    case REPORT_LOGIN_OMC://6:��¼����������ϱ�              
    case REPORT_HEART_BEAT://7:�����ϱ�
    	p_packbuf->PackLen = pvlen + AP_MSG_HEAD_TAIL_LEN;
    break;                     
    case REPORT_SW_UPDATE://8:�豸�����������ϱ�
      //0x000A:��ǰ�������汾
      p_packbuf->PackValue[pvlen++] = 25;//���ݳ���
		  p_packbuf->PackValue[pvlen++] = (char)SWRUNVER_ID;//������ݱ�ʶ
		  p_packbuf->PackValue[pvlen++] = (char)(SWRUNVER_ID >> 8);
		  p_packbuf->PackValue[pvlen++] = (char)(SWRUNVER_ID >> 16);
		  p_packbuf->PackValue[pvlen++] = (char)(SWRUNVER_ID >> 24);
		  DbGetThisStrPara(SWRUNVER_ID, var);
      for(i = 0; i < 20; i++)
      {
        p_packbuf->PackValue[pvlen++] = var[i];
      }
      //0x0018:�豸ִ����������Ľ��
      p_packbuf->PackValue[pvlen++] = 6;//���ݳ���
		  p_packbuf->PackValue[pvlen++] = (char)SWUPDATERESULT_ID;//������ݱ�ʶ
		  p_packbuf->PackValue[pvlen++] = (char)(SWUPDATERESULT_ID >> 8);
		  p_packbuf->PackValue[pvlen++] = (char)(SWUPDATERESULT_ID >> 16);
		  p_packbuf->PackValue[pvlen++] = (char)(SWUPDATERESULT_ID >> 24);

      p_packbuf->PackValue[pvlen++] = g_DevicePara.SWUpdateResult;//�豸ִ����������Ľ��
      
      p_packbuf->PackLen = pvlen + AP_MSG_HEAD_TAIL_LEN;
    break;
    case REPORT_PSLOGIN_FAIL://9:GPRS��¼ʧ���ϱ�
    	p_packbuf->PackLen = pvlen + AP_MSG_HEAD_TAIL_LEN;
    break;
    case REPORT_SAMPLEJOB_END://10:���ɽ����ϱ�
	  case 11:  // ����쳣��λ�ϱ�
      p_packbuf->PackLen = pvlen + AP_MSG_HEAD_TAIL_LEN;
    break;
    default:
      DEBUGOUT("Invalid Report Command!\r\n");
      ClearAPPackBuf(p_packbuf);
    break;
  }
  return p_packbuf->PackLen;
}

/*******************************************************************************
*�������� : int BinToBcd(int data)
*��    �� : Bin��ת��ΪBCD���ݺ���
*������� : int data��Bin������
*������� : BCD ������
*******************************************************************************/
int BinToBcd(int data)
{
int	temp;

  temp = data / 10;
  temp = temp * 0x10 + (data % 10);
  return temp;
}

/*******************************************************************************
*�������� : int	BcdToBin(int data)
*��    �� : BCD��ת��ΪBin���ݺ���
*������� : int data��BCD������
*������� : BIN ������
*******************************************************************************/
int	BcdToBin(int data)
{
int temp;

  temp = (data >> 4) & 0x0F;
  temp = temp * 10 + (data & 0x0F);
  return temp;
}

/*******************************************************************************
*�������� : int	GetDevTime(char *tbuf)  
*��    �� : ���APЭ��ʱ���ʽ�ַ���
*������� : None
*������� : ����1
*******************************************************************************/
int	GetDevTime(char *tbuf)
{
time_t timep;
struct tm *p;

	time(&timep);
	p = gmtime(&timep);
	tbuf[0] = BinToBcd((1900 + p->tm_year) / 100);
	tbuf[1] = BinToBcd((1900 + p->tm_year) % 100);
	tbuf[2] = BinToBcd(1 + p->tm_mon);
	tbuf[3] = BinToBcd(p->tm_mday);
	tbuf[4] = BinToBcd(p->tm_hour);
	tbuf[5] = BinToBcd(p->tm_min);
	tbuf[6] = BinToBcd(p->tm_sec);
	return	1;
}

/*******************************************************************************
*�������� : int	SetDevTime(char *tbuf)  
*��    �� : ����CMCCЭ��ʱ���ʽ����ʱ��
*������� : None
*������� : ����1
*******************************************************************************/
int	SetDevTime(char *tbuf)
{
time_t timep;
struct tm tmp;

	tmp.tm_year = BcdToBin(tbuf[0])*100 + BcdToBin(tbuf[1]) - 1900;
	tmp.tm_mon = BcdToBin(tbuf[2]) - 1;
	tmp.tm_mday = BcdToBin(tbuf[3]);
	tmp.tm_hour = BcdToBin(tbuf[4]);
	tmp.tm_min = BcdToBin(tbuf[5]);
	tmp.tm_sec = BcdToBin(tbuf[6]);

	if(settimeflag==1)
	{
		settimeflag = 0;
		tmp.tm_min += 2 ;
		if(tmp.tm_min>60)
		{
			tmp.tm_hour++;
			tmp.tm_min -= 60;
		}
		
	}
	

	timep = mktime(&tmp);
	stime(&timep);
	system("hwclock -w");
	return	1;
}
// FTP��������������������,���ýű�
void update(void)
{
	int status = 0;
	int val = 0;
	char ftp_ip[20]; 
	unsigned short ftp_port;
	char ftp_user[40];
	char ftp_password[40];
	char ftp_path[48];
	char ftp_filename[48];
	char str_shell[256];

    if (DbGetThisIntPara(0x00000166, &val) == 1)// �Ƿ���յ���������
	{
		printf("val=%d\n", val);
		if(val == 1){ // ��������
			DbSaveThisIntPara(0x00000166, 0);
			sqlite_read_data_ex(0x00000160, ftp_ip);
			DbGetThisIntPara(0x00000161, &ftp_port);
			sqlite_read_data_ex(0x00000162, ftp_user);
			sqlite_read_data_ex(0x00000163, ftp_password);
			sqlite_read_data_ex(0x00000164, ftp_path);
			sqlite_read_data_ex(0x00000165, ftp_filename);
			sprintf(str_shell, "/ramDisk/update.sh %s %d %s %s %s %s", ftp_ip, ftp_port, ftp_user, ftp_password, ftp_path, ftp_filename);
			printf(str_shell);
			status = system(str_shell);
			if(-1 == status){
				printf("system error!\n");
			}else{
				if(WIFEXITED(status)){
					if (0 == WEXITSTATUS(status)){
						printf("run shell script successfully\n");
						system("reboot");
					}else{
						printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
					}
				}else{
					printf("exit status = [%d]\n", WEXITSTATUS(status));
				}
			}
		}
	}else{
		printf("read 0x00000166 error!!!!!!!!!!!!\n");
	}
}
/*********************************End Of File*************************************/

