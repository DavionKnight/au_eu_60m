#include "omap_epld.h"
#include "common.h"

extern unsigned short dru_fpga_write(unsigned int addr,unsigned short data);
extern unsigned short dru_fpga_read(unsigned int addr,unsigned short *data);
/*
** �������ܣ���FPGA�ӿں���
** �������: addr=FPGA��ַ
** ���������pdata=��FPGA����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_read_fpga(unsigned int addr, unsigned short * pdata)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_fpga_read(addr, pdata);
	if(tmp == *pdata){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ�дFPGA�ӿں���
** �������: addr=FPGA��ַ data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_fpga(unsigned int addr, unsigned short data)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_fpga_write(addr, data);
	if(tmp == data){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ���EPLD�ӿں���
** �������: addr=EPLD��ַ
** ���������pdata=��EPLD����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_read_epld(unsigned int addr, unsigned short * pdata)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_epld_read(addr, pdata);
	if(tmp == *pdata){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ�дEPLD�ӿں���
** �������: addr=EPLD��ַ data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_epld(unsigned int addr, unsigned short data)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_epld_write(addr, data);
	if(tmp == data){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ�дDDC���ýӿں���
** �������: addr=�ŵ��� data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_ddc(unsigned char addr,unsigned short data)
{
	lock_sem(SEM_DRV);
	if (addr < 8)
	{
		emif_fpga_ddc_carrier_f8_write(addr,data);
	}
	else
	{
		emif_fpga_ddc_carrier_m8_write(addr-8,data);
	}
	unlock_sem(SEM_DRV);
	return 0;
}
/*
** �������ܣ�дDUC���ýӿں���
** �������: addr=�ŵ��� data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_duc(unsigned char addr,unsigned short data)
{
	lock_sem(SEM_DRV);
	if (addr < 8)
	{
		emif_fpga_duc_carrier_f8_write(addr,data);
	}
	else
	{
		emif_fpga_duc_carrier_m8_write(addr-8,data);
	}
	unlock_sem(SEM_DRV);
	return 0;
}