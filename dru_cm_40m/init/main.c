#include "apue.h"
#include "common.h"
#include "../../task/msg/exel_fun.h"
#include "../../task/msg/msg.h"
#include "../../task/log/thread_log.h"
#include "../../task/hdalarm/hdalarm.h"
#include "../../driver/dru_object.h"
#include "../../driver/omap_epld.h"
#include "../../driver/omap.h"
#include "../../driver/drv_api.h"
#include "../../driver/dru_mmap.h"
#include "../../sqlite/drudatabase.h"
#include "../../sqlite/sqliteops.h"
#include "../../task/rs485/rs485_modules.h"
#include "status.h"
#include "../../net/net_thread.h"
#include "../../protocol/irdeal.h"

// ��������Ϣ���г�ʼ��
static int _msg_init(void)
{
	printf("msg_init pthread not declare.\r\n");
	return 0;
}
int msg_init(void) __attribute__((weak, alias("_msg_init")));

// ������Ӳ����ʼ��
/*static int _hardware_init(void)
{
	printf("hardware_init pthread not declare.\r\n");
	dru_obj_main(NULL);
	return 0;
}
int hardware_init(void) __attribute__((weak, alias("_hardware_init")));
*/
// �����������ʼ��
extern int _Omc_Thread_Init(void);
static int _software_init(void)
{
	printf("software_init pthread not declare.\r\n");
	_Omc_Thread_Init();
	if(get_device_type()== DEVICE_TYPE_RAU){
		ru_para_init();
	}
	return 0;
}
int software_init(void) __attribute__((weak, alias("_software_init")));

// ����������������Ϣ�߳�
static int _creat_recv_msg_task(void)
{
	printf("creat_recv_msg_task pthread not declare.\r\n");
	return 0;
}
int creat_recv_msg_task(void) __attribute__((weak, alias("_creat_recv_msg_task")));

// ������rs485ͨ���߳�
static int _creat_rs485_task(void)
{
	printf("creat_rs485_task pthread not declare.\r\n");
	return 0;
}
int creat_rs485_task(void) __attribute__((weak, alias("_creat_rs485_task")));

// ������druӲ�������߳�
static int _creat_dru_test_task(void)
{
	printf("creat_dru_test_task pthread not declare.\r\n");
	return 0;
}
int creat_dru_test_task(void) __attribute__((weak, alias("_creat_dru_test_task")));

void check_td(void)
{
	unsigned int td_base_reg = 0x5f;
	int i = 0; 
	unsigned short data_dw;
	unsigned short data;
	float tmp;

	drv_read_fpga(td_base_reg, &data_dw);	
	if(data_dw != 0){
		printf("\r\n******** TD-SCDMA ***************\r\n");
		printf(" dw=0x%04x. \r\n", data_dw);
		for(i = 1; i < 8; i++){
			drv_read_fpga(td_base_reg+i, &data);	
			printf("ts%d=0x%04x.  ", i-1, data);
			tmp = (float)data_dw/(float)data;
			printf("float=%-8.3f.  ", tmp);
			printf("sub=%-8.3f.\r\n", 20*(log10(tmp)));
		}
	}
	printf("\r\n");
}
void check_lte(void)
{
	unsigned int lte_base_reg = 0x67;
	int i = 0; 
	unsigned short data_dw;
	unsigned short data;
	float tmp;

	drv_read_fpga(lte_base_reg, &data_dw);	
	if(data_dw != 0){
		printf("\r\n******** LTE ***************\r\n");
		printf(" dw=0x%04x. \r\n", data_dw);
		for(i = 1; i < 5; i++){
			drv_read_fpga(lte_base_reg+i, &data);	
			if(i < 2){
				printf("ts%d=0x%04x.  ", i-1, data);
			}else{
				printf("ts%d=0x%04x.  ", i, data);
			}
			tmp = (float)data_dw/(float)data;
			printf("float=%-8.3f.  ", tmp);
			printf("sub=%-8.3f.\r\n", 20*(log10(tmp)));
		}
	}
	printf("\r\n");
}
extern void init_mode_idx(void);
extern int _Omc_Thread(void);
// �Զ��ز������߳�
extern int creat_auto_channel(void);
/*
** �������ܣ�dru������
** ���������argc=����������� argv=�ַ�����ʽ���������
** �����������
** ����ֵ��0=�ɹ� 1=ʧ��
** ��ע��
*/
int main(int argc, char * argv[])
{
	void *str;
	if(read_device_type()!=0)	// ��ȡ�豸���� sys_cfg.txt  mu/eu/ru
	{ 
		exit(0);
	}
	read_net_type();        	// ��ȡ�豸�ͺ� ��ȡdev_name�ļ�
	log_thread_init();      	// ��־�߳�
	dru_obj_main(str);      	// ������Ƶ����
	msg_init();             	// ���̼�ͨ�ŵ���Ϣ���г�ʼ��
	_sem_init();				// �ź�����ʼ��
	init_mode_idx();       	 	// ��ʽID��ʼ����
	software_init();        	// �����ʼ�� 
	creat_recv_msg_task();  	// ����������Ϣ�߳�
	creat_rs485_task();     	// ����rs485ͨ���߳�
	_Omc_Thread();          	// ����OMC����߳�
    creat_net_task();       	// ����IrЭ������߳�
	create_hardware_alarm_thread();
	creat_udp_relay();      	// udpת���̣߳��㲥�Ͳ�ѯԶ�˲���
	creat_auto_channel();   	// �Զ��ز������߳�
    while(1)
	{
		//check_td();
		//check_lte();
		//actual_alarm();
		//common_alarm();
		sleep(100);
	}
    return 0;
}  
