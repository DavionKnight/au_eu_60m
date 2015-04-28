/********************  COPYRIGHT(C)  ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: apcprotocol.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 2012.10.29
**���򿪷�������
**��        ��: APCЭ�鴦�����
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

#include "approtocol.h"
#include "apcprotocol.h"

/*******************************************************************************
*�������� : int APCEscSum(char *pbuf, INT16U sum)
*��    �� : �����й��ƶ�APCЭ�飬�������ݻ������е���Ҫת����ַ�����������ת���ַ��ĸ��� 
*������� : char  *pbuf�����ݻ�������ָ��
*           INT16U sum:���黺���������ֽڳ���
*������� : ����������Ҫת����ַ����� 
*******************************************************************************/
int APCEscSum(char *pbuf, INT16U sum)
{
char data;
INT16U j;

  j = 0;//ת���ַ���
  while(sum--)
  {
    data = *pbuf++;
    if(( data == APC_ESC0X5E )//ת�崦��0x5E 
     ||( data == APC_ESC0X7E ))
    {
      j++;//ת���ַ���
    }
  }
  return  j;
}

/*******************************************************************************
*�������� : int	APCEscProcess(char *pbuf, int sum)
*��    �� : �����й��ƶ�APCЭ�飬�������ݻ������еľ���ת����ַ���������ת���ַ�����������ת���ַ�����
*������� : char *pbuf�����ݻ�������ָ��
*           INT16U sum:���黺���������ֽڳ���
*������� : �������о���ת����ַ�����,ת���д���,����-1
*�汾�޶� : 2007.10.8,������ת���ʶ�������ַ�����Ĵ���,2014.4.23�޸�
*******************************************************************************/
int	APCEscProcess(char *pbuf, int sum)
{
int j;
char data, *wr;

  j = 0;//ת���ַ���
  wr = pbuf;
  while(sum)
  {
    data = *pbuf++;
    sum--;
    if (data == APC_ESCFLAG)//ת�崦�� 0x5E
    {
      data = *pbuf++;
      sum--;
      if(data == 0x5D)//0x5E 
      {
        *wr++ = 0x5E;
        j++;//ת���ַ���
      }
      else if(data == 0x7D)//0x7E 
      {
        *wr++ = 0x7E;
        j++;//ת���ַ���
      }
      else//ת���д���,��APC_ESCFLAG�������ַ�����
      {
      	return MSG_ESC_ERR;
      }
    }
    else
    {
      *wr++ = data;
    }
  }
  return j;
}

/*******************************************************************************
*�������� : int APCPack(APPack_t *p_packbuf, char *sdbuf)
*��    �� : �����й��ƶ�APCЭ�飬��Э�����ݰ����д���ĺ���
*������� : APPack_t *p_packbuf��Э�����ݰ��ṹָ��
*           char  *sdbuf:��������ݻ���
*������� : �������Ҫ���͵����ݰ�����
*******************************************************************************/
int APCPack(APPack_t *p_packbuf, char *sdbuf)
{
int rd, wr, sum, sd;

  if ((p_packbuf->APType == AP_A)
   || (p_packbuf->APType == AP_C))
  {
    sd = APPack(p_packbuf, sdbuf);
    sum = APCEscSum(&sdbuf[1], (p_packbuf->PackLen - 2));//���ת���ַ�������,������ͷβ
    if(sum > 0)
    {
      rd = sd - 1;//���ݻ��������ݶ���ַ
      wr = rd + sum;//����������д��ַ
      sd = sd + sum;
      sdbuf[wr--] = sdbuf[rd--];
      //���ݻ���Ӻ���ǰ�����ַ�
      while(rd > 0)//�Ӻ���ǰ�����ݴ洢�����з��ô����͵�����,��������ͷ7E
      {
        if((sdbuf[rd] == APC_ESC0X5E)  //ת�崦�� 5E->5E5D
        || (sdbuf[rd] == APC_ESC0X7E)) //ת�崦�� 7E->5E7D
        {
          sdbuf[wr--] = (sdbuf[rd] & 0xF0) + 0x0D;
          sdbuf[wr] = APC_ESCFLAG;
          sum--;
          if(sum == 0)
          {
            break;
          }
        }
        else
        {
          sdbuf[wr] = sdbuf[rd];
        }
        rd--;
        wr--;
      }
    }
    ClearAPPackBuf(p_packbuf);
    return sd;
  }
  else
  {
    ClearAPPackBuf(p_packbuf);
    return MSG_AP_ERR;
  }
}

/*******************************************************************************
*�������� : int	APCUnpack(char *rcbuf, int rcnum, APPack_t *p_packbuf)
*��    �� : �����һ�����ݽ���ʱ�����ô˺����������й��ƶ�AP-CЭ����
*������� : char  *rcbuf:�������ݻ���
*						int   rcnum������������
*						APPack_t *p_packbuf�����հ���ʽ������������
*������� : ��ȷ�����ݰ�����p_packbuf->PackLen>0,���󷵻ظ���������Ϣ
*******************************************************************************/
int	APCUnpack(char *rcbuf, int rcnum, APPack_t *p_packbuf)
{
	int sum, escsum, j, packstart;//sum:����APC���ݰ���Ч������,j:�������ݼ���,packstart:��ͷ��rcbuf�е�λ��
	char buf[COMBUF_SIZE];

	sum = 0;
	j = 0;
	packstart = 0;
	buf[0] = 0;
	while (j < rcnum)
	{
    	buf[sum] = rcbuf[j];
    	sum++;
    	if (buf[0] != APC_STARTFLAG)//�Ұ�ͷ
    	{
	      	packstart = j;
	      	sum = 0;
    	}
   	 	else
    	{
      		if (sum > 1)
      		{
        		if (buf[sum-1] == APC_ENDFLAG)//�ҵ���β,�жϸն���������Ƿ�Ϊ��β
        		{
          			if (sum < AP_MSG_HEAD_TAIL_LEN) //���ݰ����ж����ͷ����β��ʶ��������Ͻ��е����һ����Ϊ��ͷ��ʶ
          			{
            			packstart = j;
           			 	buf[0] = APC_STARTFLAG;	//�Ұ�ͷ
            			sum = 1;
          			}
         	 		else
          			{
        				//ת�崦��
        				escsum = APCEscProcess(buf, sum);//sumΪת�崦������������
        				if (escsum == MSG_ESC_ERR)//2014.4.23
        				{
        					return MSG_ESC_ERR;
        				}
        				else
        				{
        					sum = sum - escsum;//ת�崦������������
        				}
        				if(APUnpack(buf, sum, p_packbuf)==MSG_CRC_ERR)//CRCУ�����
        				{
							packstart = j;
							buf[0] = APC_STARTFLAG;	//�Ұ�ͷ
							sum = 1;
							if ((j+1) < rcnum)//���յ����ݰ���������,����ʣ������
							{
								DEBUGOUT("APC Receive Data Error!\r\n");
							}
							else
							{
								return MSG_CRC_ERR;
							}
        				}
        				else
        				{
        		 			 return	p_packbuf->PackLen;
        				}
         			}
        		}
      		}
    	}
    	j++;
  	}
  	return MSG_AP_ERR;//�����������ݰ�:APЭ�����ʹ���
}

/*********************************End Of File*************************************/