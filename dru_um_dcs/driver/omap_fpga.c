/*******************************************************************************
********************************************************************************
* �ļ�����:  omap_fpga.c
* ��������:  fpga ��д����
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
#include "../../driver/omap_fpga.h"
#include "../../driver/dru_reg.h"
#include "common.h"

/*******************************************************************************
* ��������: dru_fpga_write
* ��    ��: fpga д����
* ��    ��:
* ��������         ����                ����
* addr      unsigned int      fpgaƫ�Ƶ�ַ
*	data			unsigned short    Ҫд�������
* ����ֵ:
*           unsigned short    ��д�������
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/10/17  V1.0     H4     ��       ��
*******************************************************************************/
unsigned short dru_fpga_write(unsigned int addr,unsigned short data)
{
		*DRU_REGADDR(fpga_base_addr,addr)=data;
		return data;
}
int dru_fpga_write_change(int argc, char * argv[])
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
	dru_fpga_write((unsigned int)para1, (unsigned short)para2);
	sprintf(msg_tmp.mtext, "dru_fpga_write reg:0x%x=0x%x.\r\n", para1, para2);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}
/*******************************************************************************
* ��������: dru_fpga_read
* ��    ��: ����fpga �Ĵ�����ֵ
* ��    ��:
* ��������         ����                ����
* addr      unsigned int      fpga�е�ƫ�Ƶ�ַ
* data			unsigned short *  �������ݷ��õ�ַ
* ����ֵ:
* 					unsigned short  	����������ֵ
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/10/17  V1.0     H4     ��       ��
*******************************************************************************/
unsigned short dru_fpga_read(unsigned int addr,unsigned short *data)
{
		*data=*DRU_REGADDR(fpga_base_addr,addr);
		return *DRU_REGADDR(fpga_base_addr,addr);
}
int dru_fpga_read_change(int argc, char * argv[])
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
	dru_fpga_read((unsigned int)para1, (unsigned short *)&para2);
	sprintf(msg_tmp.mtext, "dru_fpga_read reg:0x%x=0x%x.\r\n", para1, para2);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}
