#include "../common/apue.h"
#include "../common/common.h"
#include "../task/msg/exel_fun.h"
#include "../task/msg/msg.h"
#include "../task/log/thread_log.h"
#include "../task/hdalarm/hdalarm.h"
#include "../driver/dru_object.h"
#include "../driver/omap_epld.h"
#include "../driver/omap.h"
#include "../driver/dru_mmap.h"
#include "../sqlite/drudatabase.h"
#include "../sqlite/sqliteops.h"
#include "../task/rs485/rs485_modules.h"
#include "../common/status.h"
#include "../net/net_thread.h"
#include "../protocol/irdeal.h"
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

extern int _Omc_Thread(void);
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
	if(read_device_type()!=0){
		exit(0);
	}
	log_thread_init();
	dru_obj_main(str);
	msg_init();             // ���̼�ͨ�ŵ���Ϣ���г�ʼ��
	_sem_init();			// �ź�����ʼ��
    software_init();        // �����ʼ�� 
    creat_recv_msg_task();  // ����������Ϣ�߳�
	creat_rs485_task();     // ����rs485ͨ���߳�
	_Omc_Thread();
    creat_net_task();
	create_hardware_alarm_thread();
    while(1)
	{
		sleep(10);
	}
    return 0;
}  
