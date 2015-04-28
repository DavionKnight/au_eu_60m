#include "../../common/apue.h"
#include "../../common/common.h"
#include "../../driver/omap.h"
#include "exel_fun.h"
#include "../../sqlite/drudatabase.h"

extern int g_OMCSetParaRs485Flag;

// �����ⲿ����
extern struct mymesg msg_tmp;
/******************************************************************************/

/******************************************************************************/


/******************************************************************************/ 
struct mymesg * p_msg_buf;
struct fun_para{
	int argc;
	char *argv[10];
};

struct fun_para fun_para_name;
/*
** �������ܣ�������Ϣ���ݣ��ֽ���������Ͳ���
** ���������buf=��Ϣ��������len=��Ϣ����
** �����������
** ����ֵ��0=�ɹ� 1=ʧ��
** ��ע��
*/
int analyse_fun(char * buf, int len)
{
	char * p = buf;
	int i = 0;

	buf[len] = 0;				// ���ַ�����ӽ�����
	for(i = 0; i < len; i++){   // ���ַ����еĿո��Ϊ0�����ַ����ֽ⿪
		if(buf[i] == 0x20){
			buf[i] = 0;
		}
	}
	// ���������Ͳ���ȫ���洢��fun_para_name�Ľṹ����
	fun_para_name.argc = 0;
	i = 0;
	while(i < len){
		if(buf[i] != 0){
			p = buf+i;
			if(fun_para_name.argc == 10){
				printf("������������С��10\r\n");
				return 1;
			}
			fun_para_name.argv[fun_para_name.argc++] =  p;
			i += strlen(p);
		}else{
			i++;
		}
	}
	return 0;
}
struct mymesg msg_cgi;
int GetDataById(int id, char * data)
{
	switch(id){
		case 0x0004:
			strcpy(data, "DRU-RAU");
			break;
		case 0x000a:
			strcpy(data, "DRU20130227.0");
			break;
		case 0x0021:
			strcpy(data, "V17");
			break;
		case 0x01a0:
			strcpy(data, "V8");
			break;
		default:
			strcpy(data, "not find");
	}
	return 0;
}
extern int g_OMCSetParaFlag;
int checkType(char * data)
{
	char * p = NULL;
	int flag = 0;
	char tbuf[128];
	char id[128];
	char var[128];
	char * s = msg_cgi.mtext;
	char * sm = NULL;
	int i = 0;
	int cnt = 0;

	printf("checkType\r\n");
	p = strsep(&data, "&");
	if(p == NULL){
		sprintf(s, "url get error");
		goto OUT;
	}
	if((strstr(p , "read") != NULL)){
		flag = 1;
		cnt = SqliteGetCntEx();
		printf("para_cnt=%d.\n", cnt);
		if(cnt%10){
			cnt = (cnt/10)+1;
		}else{
			cnt = cnt/10;
		}
		printf("para_read_cnt=%d.\n", cnt);
		for(i = 0; i < cnt; i++){
			if(i == 0){
				sprintf(s, "cmd=read");
				sm = s + strlen(s);
			}else{
				sprintf(s, "");
				sm = s;
			}
			if(0 == sqlite_read_data(sm, i)){
				printf("send msg: len=%d\r\n", strlen(msg_cgi.mtext));
				printf("%s\r\n", msg_cgi.mtext);
				msg_cgi.mtype = MSG_CGI_RESULT|0x80;	
				if(0 != msg_send(TASK3_MODULE, (char *)&msg_cgi, strlen(msg_cgi.mtext))){
					printf("msg_send error.\r\n");
				}else{
					printf("msg_send ok.\r\n");
				}
			}
		}
		sprintf(s, "");
		goto OUT;
	}else if((strstr(p, "write") != NULL)){
		sprintf(s, "cmd=write");
		flag = 2;
		for(p = strsep(&data, "&"); p != NULL; p = strsep(&data, "&")){
			memset(id, 0, 128);
			memset(var, 0, 128);
			memset(tbuf, 0, 128);
			sscanf(p, "%[^=]=%s", id, var);
			printf("id=%s, var=%s\r\n", id, var);
			sqlite_write_data(id, var);
			sprintf(tbuf, "&%s=%s", id, var);
			strcat(s, tbuf);
			printf("%s\r\n", s);
		}
		g_OMCSetParaFlag = 1;
		g_OMCSetParaRs485Flag = 1;
	}else if((strstr(p, "para_list") != NULL)){
		cnt = SqliteGetCntEx();
		printf("para_cnt=%d.\n", cnt);
		if(cnt%10){
			cnt = (cnt/10)+1;
		}else{
			cnt = cnt/10;
		}
		printf("para_read_cnt=%d.\n", cnt);
		for(i = 0; i < cnt; i++){
			if(i == 0){
				sprintf(s, "cmd=para_list");
				sm = s + strlen(s);
			}else{
				sprintf(s, "");
				sm = s;
			}
			if(0 == SqliteReadList(sm, i)){
				printf("send msg: len=%d\r\n", strlen(msg_cgi.mtext));
				printf("%s\r\n", msg_cgi.mtext);
				msg_cgi.mtype = MSG_CGI_RESULT|0x80;	
				if(0 != msg_send(TASK3_MODULE, (char *)&msg_cgi, strlen(msg_cgi.mtext))){
					printf("msg_send error.\r\n");
				}else{
					printf("msg_send ok.\r\n");
				}
			}
		}
		sprintf(s, "");
		goto OUT;
	}else{
		sprintf(s, "cmd error");
	}
OUT:
	printf("send msg: len=%d\r\n", strlen(msg_cgi.mtext));
	//printf("%s\r\n", msg_cgi.mtext);
	msg_cgi.mtype = MSG_CGI_RESULT;	
	if(0 != msg_send(TASK3_MODULE, (char *)&msg_cgi, strlen(msg_cgi.mtext))){
		printf("msg_send error.\r\n");
	}else{
		printf("msg_send ok.\r\n");
	}
	return flag;
}
/*
** �������ܣ�����web����������������
** ���������buf=��Ϣ������
** �����������
** ����ֵ��0=�ɹ� 1=ʧ��
** ��ע��
*/
int deal_cgi(char * buf)
{
	//int len = 0;

	//p = malloc(len);
//	p = buf;
	//if(p == NULL){
//		printf("malloc error");
//		return 1;
//	}
//	strncpy(p, buf, len);
	checkType(buf);
	//free(p);
/*	
	char * s = msg_cgi.mtext;
	printf("return msg.=======================\r\n");
	printf(buf);
	sprintf(s, "%s", "�Ұ����й�");
	msg_cgi.mtype = MSG_CGI_RESULT;	
	msg_send(TASK3_MODULE, (char *)&msg_cgi, strlen(msg_cgi.mtext));	
	*/
}
/*
** �������ܣ�����fun���ݹ����ĺ���
** ���������buf=��Ϣ��������len=��Ϣ����
** �����������
** ����ֵ��0=�ɹ� 1=ʧ��
** ��ע��
*/
int deal_fun_name(char * buf, int len)
{
	p_msg_buf = (struct mymesg *)buf;
	switch(p_msg_buf->mtype)	// ������Ϣ��������ͬ����
	{
		case MSG_FUN:			// ���յ�������Ϣ
			printf("���յ�������Ϣ\r\n");
			// ���������������������������ݷֽ⿪
			if(0 == analyse_fun(p_msg_buf->mtext, len))	//����ִ�к���
			{
				exel_fun(fun_para_name.argc, fun_para_name.argv);
			}
			else		// ������������10��
			{
				sprintf(msg_tmp.mtext, "error. para cnt is > 10.\r\n");
				msg_tmp.mtype = MSG_FUN_RESULT;	
				msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));	
			}
			break;
		case MSG_CGI:			// ���յ�����������Ϣ
			printf("MSG_CGI recv:\r\n");
			printf(p_msg_buf->mtext);
			printf("\r\n");
			deal_cgi(p_msg_buf->mtext);
			break;
		default:				// ���յ��޷�ʶ�����Ϣ
			printf("�޷�ʶ����Ϣ����\r\n");
	}
	return 0;
}
/*
** �������ܣ���Ϣ�����߳�
** ���������arg=�������
** �����������
** ����ֵ����
** ��ע��
*/
void * msg_recv_pthread(void * arg)
{
    // ����fun���ݹ����ĺ�����������
	msg_recv(TASK1_MODULE, deal_fun_name);
    return (void *)NULL;   
}
/*
** �������ܣ�������Ϣ�����߳�
** ���������arg=�������
** �����������
** ����ֵ����
** ��ע��
*/
int creat_recv_msg_task(void)
{ 
    pthread_t msg_recv_id;		// ��Ϣ�����߳�ID
    
    printf("creat_recv_msg_task declare ok.\r\n");
    // ��������fun���ݹ����ĺ�����������
    if( pthread_create(&msg_recv_id, NULL, msg_recv_pthread, NULL) )
	{
		printf("pthread_creat msg_recv_pthread error.\r\n");
		return 1;
	}
    return 0;
}

