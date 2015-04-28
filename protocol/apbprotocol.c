/********************  COPYRIGHT(C) ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: apbprotocol.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: APBЭ�鴦�����
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
#include "apbprotocol.h"

/*******************************************************************************
*�������� : void ByteToAscii(char hex_data, char *pbuf)
*��    �� : ����APBЭ��,��hex������hex_dataת��Ϊ2��Ascii���ݷ��õ�pbuf��ָλ��
*������� : hex_data��Ҫת����hex���ݣ�*pbuf���洢2��Ascii�����׵�ַλ��
*������� : None
*******************************************************************************/
void ByteToAscii(char hex_data, char *pbuf)
{
char  temp;

  temp = hex_data >> 4;//��4λ
  if(temp < 0x0A )
  {
    *pbuf = '0' + temp;
  }
  else
  {
    *pbuf = 'A' + (temp - 0x0A);
  }

  temp = hex_data & 0x0F;//��4λ
  if(temp < 0x0A )
  {
    *(pbuf + 1) = '0' + temp;
  }
  else
  {
    *(pbuf + 1) = 'A' + (temp - 0x0A);
  }
}

/*******************************************************************************
*�������� : int APBByteToAscii(char *pbuf, int sum)
*��    �� : �����й��ƶ�APBЭ�飬�������ݻ������е�HEX�ַ�ΪASCII�ַ�
*������� : char *pbuf�����ݻ�������ָ��
*           int sum:���黺���������ֽڳ���
*������� : �������о���ת����ַ�����
*******************************************************************************/
int APBByteToAscii(char *pbuf, int sum)
{
int   j;
char  *wr, *rd;

  wr = pbuf + 2*sum - 3;
  rd = pbuf + sum - 1;
  *wr = *rd;
  rd--;
  wr = wr - 2;
  j = sum - 2;
  while(j--)
  {
    ByteToAscii(*rd, wr);
    rd--;
    wr = wr - 2;
  }
  return  (2*sum - 2);
}

/*******************************************************************************
*�������� : char AsciiToByte(char *pbuf)
*��    �� : ����APBЭ��,��pbuf��ָλ�õ�2��Ascii����ת��Ϊhex������
*������� : *pbuf���洢2��Ascii�����׵�ַλ��
*������� : ת�����ɵ�hex���ݣ�
*******************************************************************************/
char AsciiToByte(char *pbuf)
{
char i, outdata;

  outdata = 0;
  
  for(i = 0; i < 2; i++ )
  {
    outdata = outdata << 4;  // ��������4λ
    if(('0' <= *pbuf) && (*pbuf <= '9'))  // 0~9����
    {
      outdata += (*pbuf - '0');
    }
    else  if(('a' <= *pbuf) && (*pbuf <= 'f'))  // a~f����
    {
      outdata += (*pbuf - 'a' + 0x0A);
    }
    else  if(('A' <= *pbuf) && (*pbuf <= 'F'))  // A~F����
    {
      outdata += (*pbuf - 'A' + 0x0A);
    }
    pbuf++;
  }
  return outdata;
}

/*******************************************************************************
*�������� : int APBAsciiToByte(char *pbuf, int sum)
*��    �� : �����й��ƶ�APBЭ�飬�������ݻ������е�ASCII�ַ�ΪHEX�ַ�
*������� : char *pbuf�����ݻ�������ָ��
*           int sum:���黺���������ֽڳ���
*������� : �������о���ת����ַ�����
*******************************************************************************/
int APBAsciiToByte(char *pbuf, int sum)
{
int   j;
char  *wr;

  wr = pbuf;
  j = sum /2;
  while(j--)
  {
    *wr = AsciiToByte(pbuf);
    wr++;
    pbuf = pbuf + 2;
  }
  return  (sum /2);
}

/*******************************************************************************
*�������� : int APBPack(APPack_t *p_packbuf, char *sdbuf)
*��    �� : �����й��ƶ�APBЭ�飬��Э�����ݰ����д���ĺ���
*������� : APPack_t *p_packbuf��Э�����ݰ��ṹָ��
*           char  *sdbuf:��������ݻ���
*������� : �������Ҫ���͵����ݰ�����
*******************************************************************************/
int APBPack(APPack_t *p_packbuf, char *sdbuf)
{
int sd;

  if (p_packbuf->APType == AP_B)
  {
    sd = APPack(p_packbuf, sdbuf);
    sd = APBByteToAscii(sdbuf, sd);
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
*�������� : int	APBUnpack(char *rcbuf, int rcnum, APPack_t *p_packbuf)
*��    �� : �����һ�����ݽ���ʱ�����ô˺����������й��ƶ�AP-BЭ����
*������� : char  *rcbuf:�������ݻ���
*						int   rcnum������������
*						APPack_t *p_packbuf�����հ���ʽ������������
*������� : ��ȷ�����ݰ�����p_packbuf->PackLen>0,���󷵻ظ���������Ϣ
*******************************************************************************/
int	APBUnpack(char *rcbuf, int rcnum, APPack_t *p_packbuf)
{
int sum, i, j, packstart;//sum:����APB���ݰ���Ч������,j:�������ݼ���,packstart:��ͷ��rcbuf�е�λ��
char buf[COMBUF_SIZE];

  sum = 0;
  j = 0;
  packstart = 0;
  buf[0] = 0;
  while (j < rcnum)
  {
    buf[sum] = rcbuf[j];
    sum++;
    if (buf[0] != APB_STARTFLAG)//�Ұ�ͷ
    {
      packstart = j;
      sum = 0;
    }
    else
    {
      if (sum > 1)
      {
        if (buf[sum-1] == APB_ENDFLAG)//�ҵ���β,�жϸն���������Ƿ�Ϊ��β
        {
          if (sum < (AP_MSG_HEAD_TAIL_LEN*2)) //���ݰ����ж����ͷ����β��ʶ��������Ͻ��е����һ����Ϊ��ͷ��ʶ
          {
            packstart = j;
            buf[0] = APB_STARTFLAG;	//�Ұ�ͷ
            sum = 1;
          }
          else
          {
        		//APBЭ�鴦��
        		i = APBAsciiToByte(&buf[1], (sum - 2));
        		buf[i+1] = buf[sum - 1];
        		sum = i + 2;
        		if(APUnpack(buf, sum, p_packbuf) == MSG_CRC_ERR)//CRCУ�����
        		{
              packstart = j;
              buf[0] = APB_STARTFLAG;	//�Ұ�ͷ
              sum = 1;
              if ((j+1) < rcnum)//���յ����ݰ���������
              {
                DEBUGOUT("APB Receive Data Error!\r\n");
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

/********************  COPYRIGHT(C) ***************************************/
