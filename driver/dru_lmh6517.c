/*******************************************************************************
********************************************************************************
* �ļ�����:  dru_spi.c
* ��������:  ģ��spi���ֵ�ʵ�֡�����epld�Ĵ�����չspi ��
*						gpioģ��spi�ȵȡ�֧�������ض�д �½��ض�дģʽ���Լ�֧�ָ���оƬʱ��
* ʹ��˵��: 
* �ļ�����:	H4
* ��д����: ��2012/06/15��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include "dru_spi.h"
#include "dru_lmh6517.h"


int dru_LMH6517_gain_set(unsigned char channel,unsigned char gain,unsigned char chipsel)
{
	unsigned short data;
	unsigned char read_out=1;
	data = 0;
	data= (gain <<1) |(1<<7)|(channel << 8)|(0 << 15);       
	emif_fpga_spi_write(data,16,chipsel);

	
	return 0;
}

unsigned char dru_LMH6517_channel_gain_read(unsigned char channel,unsigned char chipsel)
{	unsigned char read_out = 0;
	unsigned short data=0;
	data= (0 <<1) |(0<<7)|(channel << 8)|(1 << 15);
	emif_fpga_spi_read(&data,16,chipsel);
	read_out = (data>>1)&0x3f;
	return read_out;
}


int dru_lte1_gain_set(unsigned char gain)
{
dru_LMH6517_gain_set(0,gain,2);

	
return 1;
}

unsigned char dru_lte1_gain_read()
{
	unsigned char temp =0;
temp = dru_LMH6517_channel_gain_read(0,2);

	
return temp;
}


int dru_lte2_gain_set(unsigned char gain)
{
dru_LMH6517_gain_set(1,gain,2);

	
return 1;
}

unsigned char dru_lte2_gain_read()
{
	unsigned char temp =0;
temp = dru_LMH6517_channel_gain_read(1,2);

	
return temp;
}

int dru_gsm_gain_set(unsigned char gain)
{
dru_LMH6517_gain_set(0,gain,1);

	
return 1;
}

unsigned char dru_gsm_gain_read()
{
	unsigned char temp =0;
temp = dru_LMH6517_channel_gain_read(0,1);

	
return temp;
}


int dru_td_gain_set(unsigned char gain)
{
dru_LMH6517_gain_set(1,gain,1);

	
return 1;
}

unsigned char dru_td_gain_read()
{
	unsigned char temp =0;
temp = dru_LMH6517_channel_gain_read(1,1);

	
return temp;
}


