/*******************************************************************************
********************************************************************************
* �ļ�����:  dru_fpga_ram.h
* ��������: fpga��ram��дͷ�ļ�
* ʹ��˵��: 
* �ļ�����:	H4
* ��д����: ��2012/08/03��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#ifndef _DRU_FPGA_RAM
#define _DRU_FPGA_RAM
#define RAM_F8_SELECT 0
#define RAM_M8_SELECT 1
int dru_fpga_ddc_ram_write(unsigned short addr,unsigned short data,unsigned short channel);
int dru_fpga_ddc_ram_read(unsigned short addr,unsigned short *data,unsigned short channel);
int dru_fpga_ddc_ram_init(void);
#endif
