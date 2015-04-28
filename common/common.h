#ifndef COMMON_H
#define COMMON_H
#include "apue.h"
// ģ�����
enum{
	FUN_MODULE=0,
	TASK1_MODULE=1,
	TASK2_MODULE=2,
	TASK3_MODULE=3,
};
// ��Ϣ����
enum {
	MSG_FUN = 1,
	MSG_FUN_RESULT = 2,
	MSG_CGI = 3,
	MSG_CGI_RESULT = 4,
};
// �ź�������
enum{
	SEM_RS485_SEND = 0,
	SEM_OMC_RS485 = 1,
	SEM_OMC_IR = 2,
	SEM_DRV = 3,
	SEM_DBSAVE = 4,
};
/*********************** �����������в��� **************************/
// ѭ�����л������ṹ
struct queue_buf{
	int len;		// �����ѭ�����л���������
	int head;		// ������ͷָ��
	int tail;		// ������βָ��
	char * buf;     // ������
};
/*
** �������ܣ���ʼ��ѭ�����л�����
** ���������queue_buf=����ʼ����ѭ�����нṹ��ָ�� len=ѭ�����г���
** �����������
** ����ֵ��0=�ɹ� -1=ʧ��
** ��ע��len����Ϊ128/256/512/1024/2048/4096
*/
extern int queue_buf_init(struct queue_buf * queue_buf, unsigned int len);

/*
** �������ܣ�ɾ��ѭ�����л�����
** ���������queue_buf=��ɾ����ѭ�����нṹ��ָ�� 
** �����������
** ����ֵ��0=�ɹ� -1=ʧ��
*/
extern void * queue_buf_exit(void * arg);

/*
** �������ܣ���ȡѭ�����л�������Ч���ݳ���
** �������: queue_buf=ѭ�����нṹ��ָ��
** �����������
** ����ֵ����Ч���ݳ���
*/
extern int get_queue_buf_len(struct queue_buf * queue_buf);

/*
** �������ܣ�ѭ�����л�������C
** �������: queue_buf=ѭ�����нṹ��ָ�� buf=����C������ָ�� len=����C���ݳ���
** �����������
** ����ֵ��0=�ɹ� -1=ʧ��
*/
extern int push_queue_buf(struct queue_buf * queue_buf, char * buf, unsigned int len);

/*
** �������ܣ�ѭ�����л��������C
** �������: queue_buf=ѭ�����нṹ��ָ�� buf=�����C������ָ�� len=�����C���ݳ���
** �����������
** ����ֵ��0=�ɹ� -1=ʧ��
*/
int pop_queue_buf(struct queue_buf * queue_buf, char * buf, unsigned int len);

/******************************************************************/
#define MSG_BUF_LEN (8192-4)
struct mymesg
{
	long mtype;
	char mtext[MSG_BUF_LEN];
};
extern struct mymesg msg_tmp;

typedef int (*operation)(char * buf, int len);

extern void test_lib(void);
extern int printk(char * buf, int len);
extern int msg_init(void);
extern int msg_send(int index, char * buf, int len);
extern int msg_recv(int index, operation handle);
extern int msg_recv_ex(int index, operation handle);
/*
** �������ܣ���HEXתΪunsigned short int �ͣ���������
** �������: p=hex����ָ��
** �����������
** ����ֵ��ת��ֵ
*/
extern unsigned short int hex2uint16(unsigned char * p);
/*
** �������ܣ���HEXתΪASCII
** �������: p=hex���� len=���ݳ���
** ���������out=ascii����
** ����ֵ��ת���������
*/
extern int hex2ascii(unsigned char * p, int len, unsigned char * out);
/*
** �������ܣ���ASCIIתΪHEX
** �������: p=ascii���� len=���ݳ���
** ���������out=hex����
** ����ֵ��ת���������
*/
int ascii2hex(unsigned char * p, int len, unsigned char * out);
/*
** �������ܣ������ֽ��з�����תΪ4�ֽ��з�����
** �������: val=���ֽ��з�����
** �����������
** ����ֵ��4�ֽ��з�����
*/
int signed_1to4(char val);
int signed_2to4(short val);
/*
** �������ܣ���ʼ���ź���
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
*/
extern int _sem_init(void);
/*
** �������ܣ��ź�������
** ����������ź�����ʶ(����ʱ����ţ��ɺ�ʵ��)
** �����������
** ����ֵ����
*/
extern void lock_sem(int idx);
/*
** �������ܣ��ź�������
** ����������ź�����ʶ(����ʱ����ţ��ɺ�ʵ��)
** �����������
** ����ֵ����
*/
extern void unlock_sem(int idx);
/*
** �������ܣ�ͨ��ftp����������
** �������: usr=�û��� pw=���� ip=������IP��ַ port=�˿� file_name=�ļ���
** �����������
** ����ֵ��0
*/
extern int updata_code(char * usr, char * pw, char * ip, char * port, char * file_name);
/*
** �������ܣ���ѹbz2��ѹ����
** �������: file_name=�ļ���
** �����������
** ����ֵ��0
*/
extern int uncompress(char * file_name);
#endif
