#include "apue.h"
#include "common.h"
#include <fcntl.h>
/*****************�����ⲿ����*************************************************/
extern int jed2eca(char * filename);

/*****************ģ���ڲ�����*************************************************/
int iic_fd = 0;

/*****************�궨��*******************************************************/ 
#define CPLD_ADDR 0x80
#define CPLD_READ (CPLD_ADDR|0x1)
#define CPLD_WRITE (CPLD_ADDR)

#define START 0x00000001
#define STOP 0x00000002
#define SET 0x80000001
#define CLR 0x80000002
#define READ_ST 0x80000003
#define DEBUG 0x80000004
#define SDA (1<<27)
#define SCL (1<<28)

#define ERASE_FR 0x2
#define ERASE_CF 0x4
#define ERASE_UFM 0x8

/*****************ģ���ڽṹ����***********************************************/ 
struct machxo2{//  MAchXO2 sysCONFIG Programming Command Struct
	unsigned char oper_type;
	unsigned char cmd;
	unsigned char operands[3];
	int operands_len;
};

struct buf_st{  // ��д�������ṹ
	unsigned char buf[256];
	unsigned int len;
};
struct buf_st write_data;
struct buf_st read_data;
/*
** �������ܣ���ȡ�����������
** ���������cmd=������
** �����������
** ����ֵ�������������
** ��ע��
*/
int get_operands_len(unsigned char cmd)
{
	switch(cmd){
		case 0x26:
		case 0x79:
		case 0x74:
			return 2;
			break;
		default:
			return 3;
	}
}
/*
** �������ܣ���ȡ��������
** ���������cmd=������
** �����������
** ����ֵ���������� CPLD_READ or CPLD_WRITE
** ��ע��
*/
int get_oper_type(unsigned char cmd)
{
	switch(cmd){
		case 0xe0: // read
		case 0xf0:
		case 0x3c:
		case 0xc0:
		case 0x19:
		case 0x73:
			return CPLD_READ;	
			break;
		case 0x70: // write
		case 0xc2:
		case 0xb4:
			return CPLD_WRITE;	
			break;
		default:
			return 0;
			break;
	}
	return 0;
}
/*
** �������ܣ���֯�����������
** ���������pack=�������ṹ
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int packed_operands(struct machxo2 * pack)
{
	unsigned char * p = pack->operands;
	memset(p, 0, 3);
	switch(pack->cmd){
		case 0x74:
			p[0] |= 0x8;
			break;
		case 0x0e:
			p[0] |= ERASE_CF;
			break;
		case 0x70:
			p[2] |= 0x1;
			break;
		case 0x73:
			p[2] = 1;
			break;
		default:
			break;
	}
	return 0;			
}
/*
** �������ܣ���֯write����
** ���������cmd=������ buf=��Ҫд������ݻ�����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int packed_write_data(unsigned char cmd, unsigned char * buf)
{
	switch(cmd){
		case 0x70:
			write_data.len = 16;
			memcpy(write_data.buf, buf, write_data.len);
			break;
		case 0xc2:
			write_data.len = 4;
			memcpy(write_data.buf, buf, write_data.len);
			break;
		case 0xb4:
			write_data.len = 4;
			memcpy(write_data.buf, buf, write_data.len);
			break;
		default:
			write_data.len = 0;
			break;
	}
	return 0;
}
/*
** �������ܣ���֯read����
** ���������cmd=������ 
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int packed_read_data(unsigned char cmd)
{
	switch(cmd){
		case 0xe0:
			read_data.len = 4;
			break;
		case 0xf0:
			read_data.len = 1;
			break;
		case 0x3c:
			read_data.len = 4;
			break;
		case 0xc0:
			read_data.len = 4;
			break;
		case 0x19:
			read_data.len = 4;
			break;
		case 0x73:
			read_data.len = 16;
			break;
		default:
			read_data.len = 0;
			break;
	}
	return 0;
}
/*
** �������ܣ���֯��������
** ���������cmd=������, buf=��Ҫд������ݻ����� 
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int pack_machxo2(unsigned char cmd, unsigned char * buf)
{
	struct machxo2 pack;
	unsigned char tbuf[256];
	int cnt = 0;

	// ��ʼ������ṹ
	pack.cmd = cmd;
	pack.oper_type = get_oper_type(cmd);
	pack.operands_len = get_operands_len(cmd);
	packed_operands(&pack);
	// ��ʼ����д�ṹ
	packed_write_data(cmd, buf);
	packed_read_data(cmd);
	if(pack.oper_type == CPLD_WRITE){// CPLDд����
		cnt = 0;
		tbuf[cnt++] = CPLD_WRITE;
		tbuf[cnt++] = pack.cmd;
		memcpy(tbuf+cnt, pack.operands, pack.operands_len);
		cnt += pack.operands_len;
		memcpy(tbuf+cnt, write_data.buf, write_data.len);
		cnt += write_data.len;
		if(write(iic_fd, tbuf, cnt)){
			printf("write1 error.\n");
			return -1;
		}
		ioctl(iic_fd, STOP, NULL); // ������,write����iic��start�ź�, ��stop�ź�
	}else if(pack.oper_type == CPLD_READ){// CPLD������
		cnt = 0;
		tbuf[cnt++] = CPLD_WRITE;
		tbuf[cnt++] = pack.cmd;
		memcpy(tbuf+cnt, pack.operands, pack.operands_len);
		cnt += pack.operands_len;
		if(write(iic_fd, tbuf, cnt)){
			printf("write2 error.\n");
			return -2;
		}
		cnt = 0;
		tbuf[cnt++] = CPLD_READ;
		if(write(iic_fd, tbuf, cnt)){
			printf("write3 error.\n");
			return -3;
		}
		read(iic_fd, read_data.buf, read_data.len); // �����У�read����iic��stop�źţ���start�ź�
	}else{// CPLD��������޶�д����
		cnt = 0;
		tbuf[cnt++] = CPLD_WRITE;
		tbuf[cnt++] = pack.cmd;
		memcpy(tbuf+cnt, pack.operands, pack.operands_len);
		cnt += pack.operands_len;
		if(write(iic_fd, tbuf, cnt)){
			printf("write2 error.\n");
			return -2;
		}
		ioctl(iic_fd, STOP, NULL);
	}
	return 0;
}
/*
** �������ܣ���ȡCPLDоƬID
** �����������
** �����������
** ����ֵ��оƬID��
** ��ע��
*/
unsigned int read_device_id(void)
{
	unsigned int tmp = 0;
	unsigned int id = 0x012BB043;

	while(id != tmp){
		if(0 != pack_machxo2(0xe0, NULL)){
			printf("read device id error.\n");
		}else{
			tmp =( (read_data.buf[0]<<24) | (read_data.buf[1]<<16) | (read_data.buf[2]<<8) | read_data.buf[3]);
		}
		printf("dev_id = 0x%08X.\n", tmp);
		sleep(1);
	}
	return tmp;
}
/*
** �������ܣ���ȡоƬæ��־
** �����������
** �����������
** ����ֵ��оƬæ״̬
** ��ע��оƬæʱ��������ѯ��ֱ������
*/
unsigned int read_busy_flag(void)
{
	unsigned int tmp = 0xffffffff;
	
	while(tmp & 0x80){
		if(0 != pack_machxo2(0xf0, NULL)){
			printf("read busy flag error.\n");
		}else{
			tmp = read_data.buf[0];
		}
		usleep(100);
	}
	printf("device is ready.\n");
	return tmp;
}
/*
** �������ܣ���ȡоƬ����״̬��־
** �����������
** �����������
** ����ֵ��оƬ����״̬��־
** ��ע��оƬERRORʱ��������ѯ��ֱ��OK
*/
unsigned int read_status_flag(void)
{
	unsigned int tmp = 0xffffffff;

	while(tmp & (3<<12)){
		if(0 != pack_machxo2(0x3c, NULL)){
			printf("read status flag error.\n");
		}else{
			tmp =( (read_data.buf[0]<<24) | (read_data.buf[1]<<16) | (read_data.buf[2]<<8) | read_data.buf[3]);
			printf("status = 0x%08x.\n", tmp);
		}
		usleep(100);
	}
	printf("device is ok.\n");
	return tmp;
}
/*
** �������ܣ�ʹ��config flash�ӿ�
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int enable_config_interface(void)
{
	unsigned int tmp = 0;
	if(0 != pack_machxo2(0x74, NULL)){
		printf("enable_config_interface error.\n");
		tmp = -1;
	}else{
		printf("enable_config_interface ok.\n");
	}
	return tmp;
}
/*
** �������ܣ�����config flash
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int erase_cf(void)
{
	unsigned int tmp = 0;
	if(0 != pack_machxo2(0x0e, NULL)){
		printf("erase_cf error.\n");
		tmp = -1;
	}else{
		printf("erase_cf ok.\n");
	}
	return tmp;
}
/*
** �������ܣ���λconfig flash��ַ
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int reset_addr(void)
{
	unsigned int tmp = 0;
	if(0 != pack_machxo2(0x46, NULL)){
		printf("reset_addr error.\n");
		tmp = -1;
	}else{
		printf("reset_addr ok.\n");
	}
	return tmp;
}
/*
** �������ܣ�дconfig flash
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int write_flash(char * file_name)
{
	int ret = 0;
	size_t tmp = 0;
	FILE * fp = NULL;
	unsigned char tbuf[16];  
	long file_len;
	//char file_name[128];

	//sprintf(file_name, "/flashDev/program/%s", arg);
	fp = fopen(file_name, "rb");// �������ļ�
	if(fp == NULL){
		printf("open %s error.\n", file_name);
		ret = -1;
		goto PROEXIT;
	}
	if(0 != fseek(fp, 0, SEEK_END)){// ���ļ�ָ�룬ָ���ļ�β
		printf("fseek error.\n");
		ret = -2;
		goto PROEXIT;
	}
	file_len = ftell(fp);// ��ȡ��ǰ�ļ�ָ��λ�ã����ļ���С
	if(file_len == -1){
		printf("ftell error.\n");
		ret = -3;
		goto PROEXIT;
	}
	if(0 != fseek(fp, 0, SEEK_SET)){// �ָ��ļ�ָ��Ϊ�ļ�ͷ
		printf("fseek 2 error.\n");
		ret = -4;
		goto PROEXIT;
	}
	printf("file len is 0x%08x.\n", (unsigned int)file_len);
	while(file_len){
		tmp = fread(tbuf, 1, 16, fp);// ��ȡ1ҳ����
		if(tmp != 16){
			printf("file page len error.\n");
			ret = -5;
			goto PROEXIT;
		}
		if(0 != pack_machxo2(0x70, tbuf)){ // ����ҳ����д��flash
			printf("write page error.\n");
			ret = -7;
			goto PROEXIT;
		}
		file_len -= 16;
		read_busy_flag();
		read_status_flag();
	}
	
PROEXIT:
	fclose(fp);
	return ret;
}
/*
** �������ܣ����ñ�̽���, program done
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int set_flash_done(void)
{
	unsigned int tmp = 0;
	if(0 != pack_machxo2(0x5e, NULL)){
		printf("set_flash_done error.\n");
		tmp = -1;
	}else{
		printf("set_flash_done ok.\n");
	}
	return tmp;
}
/*
** �������ܣ�����security
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int set_security(void)
{
	unsigned int tmp = 0;
	if(0 != pack_machxo2(0xce, NULL)){
		printf("set_security error.\n");
		tmp = -1;
	}else{
		printf("set_security ok.\n");
	}
	return tmp;
}
/*
** �������ܣ��ر�config flash�ӿ�
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int disable_config(void)
{
	unsigned int tmp = 0;
	if(0 != pack_machxo2(0x26, NULL)){
		printf("disable_config error.\n");
		tmp = -1;
	}else{
		printf("disable_config ok.\n");
	}
	return tmp;
}
/*
** �������ܣ����¼���flash
** �����������
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int refresh(void)
{
	unsigned int tmp = 0;
	if(0 != pack_machxo2(0x79, NULL)){
		printf("refresh error.\n");
		tmp = -1;
	}else{
		printf("refresh ok.\n");
	}
	return tmp;
}
//int main(int argc, char * argv[])
int write_cpld(char * file_name)
{
	iic_fd = open("/dev/iic_cpld", O_RDWR);	
	if(iic_fd < 0){
		printf("open /dev/iic_cpld error.\n");
		return -1;
	}
	read_busy_flag();
	read_status_flag();
	read_device_id();
	read_busy_flag();
	read_status_flag();
	enable_config_interface();
	read_busy_flag();
	read_status_flag();
	erase_cf();
	read_busy_flag();
	read_status_flag();
	reset_addr();
	read_busy_flag();
	read_status_flag();
	write_flash(file_name);
	set_flash_done();
	read_busy_flag();
	read_status_flag();
	disable_config();
	read_busy_flag();
	read_status_flag();
	refresh();
	read_busy_flag();
	read_status_flag();
	close(iic_fd);
	printf("write cpld exit.\n");
    return 0;
}  
int updata_cpld(void)
{
	jed2eca("/ramDisk/cpld.jed");
	write_cpld("/ramDisk/tmp.bin");
}

int updata_cpld_change(int argc, char * argv[])
{
	msg_tmp.mtype = MSG_FUN_RESULT;	
	if(argc != 1){
		printf("input para cnt is not 1.\r\n");
		sprintf(msg_tmp.mtext, "input para cnt is not 1.\r\n");
		msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
		return 1;
	}
	updata_cpld();	
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}
