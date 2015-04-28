/*******************************************************************************
********************************************************************************
* �ļ�����:  omap_epld.c
* ��������:  epld ��д����
*						
* ʹ��˵��: 
* �ļ�����:	H4
* ��д����: ��2012/10/16��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../driver/omap_epld.h"
#include "../../driver/dru_reg.h"
#include "common.h"

/*******************************************************************************
* ��������: dru_epld_write
* ��    ��: epld ������
* ��    ��:
* ��������         ����                ����
* addr      unsigned int      epldƫ�Ƶ�ַ
*	data			unsigned short    Ҫд�������
* ����ֵ:
*           unsigned short    ��д�������
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/10/116  V1.0     H4     ��       ��
*******************************************************************************/
unsigned short dru_epld_write(unsigned int addr,unsigned short data)
{
		*DRU_REGADDR(epld_base_addr,addr)=data;
		return data;
}

int dru_epld_write_change(int argc, char * argv[])
{
	unsigned int para1;
	unsigned short para2;

	msg_tmp.mtype = MSG_FUN_RESULT;	
	if(argc != 3){
		printf("input para cnt is not 3.\r\n");
		sprintf(msg_tmp.mtext, "input para cnt is not 3.\r\n");
		msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
		return 1;
	}
	para1 = strtol(argv[1], NULL, 16);
	para2 = strtol(argv[2], NULL, 16);
	printf("para1=%x. para2=%x.\r\n", para1, para2);
	dru_epld_write((unsigned int)para1, (unsigned short)para2);
	sprintf(msg_tmp.mtext, "dru_cpld_write reg:0x%x=0x%x.\r\n", para1, para2);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}
/*******************************************************************************
* ��������: dru_epld_read
* ��    ��: ����epld �Ĵ�����ֵ
* ��    ��:
* ��������         ����                ����
* addr      unsigned int      epld�е�ƫ�Ƶ�ַ
* data			unsigned short *  �������ݷ��õ�ַ
* ����ֵ:
* 					unsigned short  	����������ֵ
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/10/16  V1.0     H4     ��       ��
*******************************************************************************/
unsigned short dru_epld_read(unsigned int addr,unsigned short *data)
{
		*data=*DRU_REGADDR(epld_base_addr,addr);
		return *DRU_REGADDR(epld_base_addr,addr);
}

int dru_epld_read_change(int argc, char * argv[])
{
	unsigned int para1;
	unsigned short para2;

	msg_tmp.mtype = MSG_FUN_RESULT;	
	if(argc != 2){
		printf("input para cnt is not 2.\r\n");
		sprintf(msg_tmp.mtext, "input para cnt is not 2.\r\n");
		msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
		return 1;
	}
	para1 = strtol(argv[1], NULL, 16);
	printf("para1=%x. \r\n", para1);
	dru_epld_read((unsigned int)para1, (unsigned short *)&para2);
	sprintf(msg_tmp.mtext, "dru_cpld_read reg:0x%x=0x%x.\r\n", para1, para2);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}

