/*******************************************************************************
********************************************************************************
* 文件名称:  dru_connect.c
* 功能描述:  网络连接代码
* 使用说明:  
* 文件作者:	H4
* 编写日期: （2012/10/23）
* 修改历史:
* 修改日期    修改人       修改内容
*-------------------------------------------------------------------------------
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "dru_connect.h"
#include "socketcommon.h"
#include "../protocol/ir.h"
#include "../protocol/ir_def.h"
#include "../protocol/irdeal.h"
#include "../common/status.h"
#include "../common/common.h"
#include "../common/druheader.h"
#include "../common/drudefstruct.h"
#include "../sqlite/sqliteops.h"
#include "../sqlite/drudatabase.h"
#include <arpa/inet.h>

volatile pthread_t client_thread;
volatile unsigned char connectflag=CLIENT_UNCONNECTED;	
volatile int current_link_dev;
volatile unsigned char client_time=0;
volatile connect_record_t connect_buffer[MAX_CONNECT_NUMBER];
send_list_t send_list;
send_message_t send_buffer[MAX_BUFFER_NUMBER];
void deal_accept(int socketfd,struct sockaddr_in accept_in);
/**********************************全局变量***********************************/
volatile int tcp_server;
volatile int udp_server;

/*******************************************************************************
* 函数名称: get_current_dev
* 功    能:获取当前链接的设备号 
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/12/01  V1.0     H4     无       ：
* *****************************************************************************/
int get_current_dev(void)
{
	return current_link_dev;
}
/*******************************************************************************
* 函数名称: get_dev_byindex
* 功    能:由数组号查询设备号若输出为0则查询失败 
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2013/1/30  V1.0     H4     无       ：
*******************************************************************************/
int get_dev_byindex(unsigned int index)
{
	if(connect_buffer[index].connect_status==CLIENT_CONNECTED){
		return connect_buffer[index].dev;
	}
}
/***********************************************************************
** Funcation Name : get_index_byid
** Author         : H4
** Description    : 有设备id查询所在组号 
** Input          : 
** Output         : 
** date           : 2013年02月26日 星期二 10时38分31秒
** Version        : V1.0
** Modify         : 
***********************************************************************/
int get_index_byid(unsigned long long id)
{
	U16 i;
	if(get_device_type()==DEVICE_TYPE_MAIN){
		for(i=0;i<MAX_CONNECT_NUMBER;i++){
			if(connect_buffer[i].dev_id==id){
				return i;
			}
		}
		if(i==MAX_CONNECT_NUMBER){
			return -1;
		}
	}else{
		return 0;
	}
}
/*******************************************************************************
* 函数名称: get_index_bydev
* 功    能: 由设备号查询出所在数组号 
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/11/28  V1.0     H4     无       ：
*******************************************************************************/
int get_index_bydev(int dev)
{
	U16 i;
	if(get_device_type()==DEVICE_TYPE_MAIN){
		for(i=0;i<MAX_CONNECT_NUMBER;i++){
			if(connect_buffer[i].dev==dev){
				return i;	
			}
		}
		if(i==MAX_CONNECT_NUMBER){
			return -1;
		}
	}else{
		return 0;
	}
}
/*******************************************************************************
* 函数名称:display
* 功    能:显示链接信息 
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/12/26  V1.0     H4     无       ：
*******************************************************************************/
void display(void)
{
	int res;
	U16 i,j;
	U8 data_8;
	U64 data_tmp;
	U64 data;
	SqlResult_t sqlres;
	char ipbuf[20],st1[50], sql[SQL_CMD_SIZE];
	struct sockaddr_in sip;
	for(i=1;i<MAX_CONNECT_NUMBER;i++){
		if(connect_buffer[i].connect_status==CLIENT_CONNECTED){
			if(connect_buffer[i].dev_type==DEVICE_TYPE_EXPEND){
				//printf("%d: client type is EXPEND",i);
				sprintf(st1,"%d: client type is EXPEND;",i);
				strcat(msg_tmp.mtext,st1);
			}else{
				//printf("%d:RRU",i);
				sprintf(st1,"%d: client type is RRU;",i);
				strcat(msg_tmp.mtext,st1);
			}
			memset(st1,0,50);
			sip.sin_addr.s_addr=connect_buffer[i].client_ip;
			//printf("client ip is %s",inet_ntoa(sip.sin_addr));	
			sprintf(st1,"client ip is %s;",inet_ntoa(sip.sin_addr));	
			strcat(msg_tmp.mtext,st1);
			memset(st1,0,50);
			sprintf(ipbuf,"%s",inet_ntoa(sip.sin_addr));	
			sprintf(sql, "SELECT * FROM %s WHERE IP=\'%s\';", CLIENT_TBL, ipbuf);
			res = SqliteSelect(sql, &sqlres);
			sprintf(st1,"client number is %s;",sqlres.dbResult[sqlres.nColumn + CLIENTTBL_DEVNO]);
			strcat(msg_tmp.mtext,st1);
			memset(st1,0,50);
			sprintf(st1,"dev_id=%llx;",connect_buffer[i].dev_id);
			strcat(msg_tmp.mtext,st1);
			memset(st1,0,50);
			data=connect_buffer[i].dev_id;
			data&=0xFFFFFFFFFFF;
			for(j=0;j<11;j++){
				data_tmp=8;
				if((data&(data_tmp<<(j*4)))>0){
					data_8=data>>(j*4);
					//printf("data_8 is %x",data_8);
					st1[2*j+1]='-';
					st1[2*j+2]=((data_8&0x07)|0x30)+1;
				}else{
					st1[0]=0x31;
					st1[j*2+1]='\r';
					st1[j*2+2]='\n';
					st1[j*2+3]='\0';
					break;
				}
			}
			strcat(msg_tmp.mtext,st1);
		}
	}
}

unsigned int get_level_by_topologic(unsigned long long topologic_num)
{
	int i = 0; 

	for( i = 0; i < 15; i++){
		if( ((topologic_num>>(i<<2))&0xf) == 0 ){ 
			return i;
		}
	}
	return 15;
}
// 查询相关设备IP
unsigned long  get_ip_by_topologic(unsigned long long topologic_num, unsigned char port_num)
{
	int i = 0;
	unsigned long long tmp = 0;
	unsigned int level;

	port_num |= 0x8;
	for(i=1;i<MAX_CONNECT_NUMBER;i++){ // 遍历设备ID
		if(connect_buffer[i].connect_status==CLIENT_CONNECTED){ // 有效设备
			if(connect_buffer[i].dev_id == topologic_num){
				level = get_level_by_topologic(topologic_num);
				if(level < 15){
					//printf("level=%d, topologic_num=%llx\n", level, topologic_num);
					if(connect_buffer[i].dev_type == DEVICE_TYPE_EXPEND){
					//	printf("dev_type is EXPEND\n");
						tmp = topologic_num | (port_num<<(level<<2));
					}else if(connect_buffer[i].dev_type == DEVICE_TYPE_RAU){
					    tmp = topologic_num & (~(0xf<<((level-1)<<2)));
					}
				}else{
					return 0;
				}
				break;
			}
		}
	}
//	printf("tmp=%llx\n", tmp);
	for(i=1;i<MAX_CONNECT_NUMBER;i++){ // 遍历设备ID
		if(connect_buffer[i].connect_status==CLIENT_CONNECTED){ // 有效设备
			if(connect_buffer[i].dev_id == tmp){
				return connect_buffer[i].client_ip;
			}
		}
	}
	return 0;
}
//查询下级远端设备的端口号和个数 
unsigned int  get_child_port(unsigned long long topologic_num, unsigned char * buf)
{
	int i = 0;
	unsigned long long tmp = 0;
	unsigned int level = 0;
	unsigned int cnt = 0;
	unsigned int ret = 0;

	for(i=1;i<MAX_CONNECT_NUMBER;i++){ // 遍历设备ID
		if(connect_buffer[i].connect_status==CLIENT_CONNECTED){ // 有效设备
//			printf("dev_id[%d]=%llx, topologic_num=%llx\n", i, connect_buffer[i].dev_id, topologic_num);
			if(connect_buffer[i].dev_id == topologic_num){
				level = get_level_by_topologic(topologic_num);
//				printf("topologic_num=%llx\n", topologic_num);
//				printf("level=%d\n", level);
				if(level < 15){

				}else{
					return 0;
				}
				break;
			}
		}
	}
	//printf(">>>> level=%d\n", level);
	for(i=1;i<MAX_CONNECT_NUMBER;i++){ // 遍历设备ID
		if(connect_buffer[i].connect_status==CLIENT_CONNECTED){ // 有效设备
			//printf("dev_id[%d]=%llx, topologic_num=%llx\n", i, connect_buffer[i].dev_id, topologic_num);
			if((connect_buffer[i].dev_id & (~(0xf<<(level<<2)))) == topologic_num){
				ret = get_level_by_topologic(connect_buffer[i].dev_id);
				//printf("ret=%d, level=%d\n", ret, level);
				if(ret == (level+1)){
					buf[cnt++] = (connect_buffer[i].dev_id >> ((level)<<2))&0x07;
				}
			}
		}
	}
	return cnt;
}
// 通过设备编号查询IP地址
// 0=成功 1=失败
int get_ip_by_dnum(unsigned int dev_num, unsigned int* ip)
{
	int res;
	SqlResult_t sqlres;
	char sql[SQL_CMD_SIZE];

	sprintf(sql, "SELECT * FROM %s WHERE DevNo=\'%d\';", CLIENT_TBL, dev_num);
	res = SqliteSelect(sql, &sqlres);
	if(res == SQLITE_OK){
		res = inet_pton(AF_INET, sqlres.dbResult[sqlres.nColumn + CLIENTTBL_IPADDR], ip);
		if(res == 1){
			return 0;	
		}
	}
	return 1;
}
// 通过IP地址查询设备编号
// 0=成功 1=失败
int get_dnum_by_ip(unsigned int ip, unsigned long * dev_num)
{
	int res;
	SqlResult_t sqlres;
	char sql[SQL_CMD_SIZE];
	char str[20];

	res = inet_ntop(AF_INET, &ip, str, INET_ADDRSTRLEN);
	if(res == 0){
		sprintf(sql, "SELECT * FROM %s WHERE IP=\'%s\';", CLIENT_TBL, str);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			*dev_num = atoi(sqlres.dbResult[sqlres.nColumn + CLIENTTBL_DEVNO]);
			return 0;	
		}
	}
	return 1;
}
/*******************************************************************************
* 函数名称: dru_listen_connect
* 功    能: 监听连
*
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/10/24  V1.0     H4     无       ：
*******************************************************************************/
void *listen_connect_thread(void *arg)
{
		fd_set readfd;
		int socketfd;
		struct sockaddr_in client_sin;
		socklen_t acceptlen=sizeof(client_sin);
		DEBUG_PRINTF("\n listen thread\n");
		for(;;){
				usleep(100*1000);
				FD_ZERO(&readfd);
				FD_SET(tcp_server,&readfd);
				if(select(FD_SETSIZE,&readfd,NULL,NULL,NULL)>0){
						DEBUG_PRINTF("\n select ok in listen connect \n");
						if(FD_ISSET(tcp_server,&readfd)){
								socketfd=accept(tcp_server,(struct sockaddr *)&client_sin,&acceptlen);
								if(socketfd==-1){
										DEBUG_PRINTF("\n accept error \n");
								}else{
										DEBUG_PRINTF("\n client %d connect the server \n",socketfd);
										deal_accept(socketfd,client_sin);
								}
						}
				}
		}
		return (void *)NULL;
}
/*******************************************************************************
* 函数名称: deal_accept
* 功    能: 接收到连接处理函数
*
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* ----------------------------------------------------------------------------
* 2012/10/29  V1.0     H4     无       ：
* 2012/12/04				h4		链接buffer中增加子设备信息指针在链接时
*									为其分配内存空间
*******************************************************************************/
void deal_accept(int socketfd,struct sockaddr_in accept_in)
{
		unsigned int i;
		unsigned char flag=FLAG_FALSE;
		unsigned long ip;
		for(i=1;i<MAX_CONNECT_NUMBER;i++){
			if((accept_in.sin_addr.s_addr==connect_buffer[i].client_ip)&&\
					(connect_buffer[i].connect_status!=CLIENT_UNCONNECTED)){
				flag=FLAG_TRUE;
				if(socketfd!=connect_buffer[i].socket)
				{
					close(connect_buffer[i].socket);
					connect_buffer[i].socket=socketfd;
					DEBUG_PRINTF("\n accept from the same ip\n");
				}
				break;
			}else{
				continue;
			}
		}
		if(flag==FLAG_FALSE){
			for(i=1;i<MAX_CONNECT_NUMBER;i++){
				if(connect_buffer[i].connect_status==CLIENT_UNCONNECTED){
					connect_buffer[i].p_info=(rru_info *)malloc(sizeof(rru_info));
					connect_buffer[i].p_dc=(delay_control *)malloc(sizeof(delay_control));
					if(connect_buffer[i].p_dc!=NULL){
						memset(connect_buffer[i].p_dc,0,sizeof(delay_control));
						DEBUG_PRINTF("\n memory require ok\n");
					}else{
						DEBUG_PRINTF("\n memory require error\n");
						DEBUG_PRINTF("\n connect error\n");
						return;
					}
					if(connect_buffer[i].p_info!=NULL){
						memset(connect_buffer[i].p_info,0,sizeof(rru_info));
						DEBUG_PRINTF("\n memory require ok\n");
					}else{
						DEBUG_PRINTF("\n memory require error\n");
						DEBUG_PRINTF("\n connect error\n");
						return;
					}
					connect_buffer[i].socket=socketfd;
					connect_buffer[i].client_ip=accept_in.sin_addr.s_addr;
					ip=connect_buffer[i].client_ip;
					connect_buffer[i].connect_status=CLIENT_CONNECTED;
					connect_buffer[i].dev=((unsigned char)htonl(ip));
					DEBUG_PRINTF("\n accept a new  ip %d\n",connect_buffer[i].dev);
					break;
				}else{
					continue;
				}
			
			}

		}
		if(i==MAX_CONNECT_NUMBER){
			DEBUG_PRINTF("\n connect buffer is full \n");
		}
	
}
/*******************************************************************************
* 函数名称:close_client
* 功    能:关闭客户端链接并处于无连接状态
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/12/5  V1.0     H4     无       ：
* *****************************************************************************/
extern unsigned long serverip;
void close_client(void)
{
	serverip=0;
	connectflag=CLIENT_UNCONNECTED;
	usleep(500);
	close(tcp_server);
	DEBUG_PRINTF("lost the server connect wait connect againe\n");
}
/*******************************************************************************
* 函数名称: dru_tcp_client_init
* 功    能:初始化客户端连接
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/11/1  V1.0     H4     无       ：
*******************************************************************************/
int dru_tcp_client_init(void)
{
	unsigned long server_ip;
	int clientfd,err;
	server_ip=get_server_ip();
	err=socket_client_init(server_ip,TCP_SERVER_PORT,&clientfd,SOCK_STREAM);
	DEBUG_PRINTF("\nserverip %x\r\n",server_ip);
	if(err!=0){
		DEBUG_PRINTF("\nconnect error\n");
		return 1;
	}
	tcp_server=clientfd;
	return 0;
}
/*******************************************************************************
* 函数名称:check_connect
* 功    能:检测链接是否超时
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/12/5  V1.0     H4     无       ：
* *****************************************************************************/
void check_connect(void)
{
	unsigned short i;
	for(i=1;i<MAX_CONNECT_NUMBER;i++){
		if(connect_buffer[i].connect_status==CLIENT_CONNECTED){
			if(connect_buffer[i].time>OUT_TIME){
				connect_buffer[i].connect_status=CLIENT_UNCONNECTED;
				connect_buffer[i].time=0;
				close(connect_buffer[i].socket);
				free(connect_buffer[i].p_info);
				DEBUG_PRINTF("lost client %d\n",connect_buffer[i].dev);
				connect_buffer[i].dev=0;
			}else{
				connect_buffer[i].time++;
			}
		}
	}
}
/*******************************************************************************
* 函数名称: dru_tcp_server_init
* 功    能:初始化服务器
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/10/24  V1.0     H4     无       ：
*******************************************************************************/
int dru_tcp_server_init(void)
{
		int ret,i;
		for(i=0;i<MAX_CONNECT_NUMBER;i++){
			connect_buffer[i].socket= 0xffffffff;
			connect_buffer[i].client_ip=0;
			connect_buffer[i].connect_status=CLIENT_UNCONNECTED;
		}
		send_list.send_message_p=NULL;
		send_list.next=NULL;
		for(i=0;i<MAX_BUFFER_NUMBER;i++){
			memset(&send_buffer[i],0,sizeof(send_message_t));
		}
		ret=socket_server_init(TCP_SERVER_PORT,&tcp_server,SOCK_STREAM);
		if(ret!=0){
			DEBUG_PRINTF("\n init socket server error\n");
			return -1;
		}
		connect_buffer[0].dev_type=TYPE_MAIN;
		connect_buffer[0].connect_status=CLIENT_CONNECTED;
		connect_buffer[0].p_info=(rru_info *)malloc(sizeof(rru_info));
		connect_buffer[0].p_dc=(delay_control *)malloc(sizeof(delay_control));
		DEBUG_PRINTF("\n socket server %d\n",tcp_server);
		return 0;
}
/*******************************************************************************
* 函数名称: receive_server_thread
* 功    能:接收服务器数据线程
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/11/1  V1.0     H4     无       ：
*******************************************************************************/
void *receive_server_thread(void *arg)
{
		int ret ;
		unsigned char bufin[MAX_SEND_LENGTH];
		unsigned long  reclen;
		DEBUG_PRINTF("\nconnect and receive server thread\n");
		for(;;){
			usleep(100*1000);
			if(connectflag==CLIENT_CONNECTED){
				ret=socket_receive(tcp_server,bufin,MAX_SEND_LENGTH,MAX_WAITE_US, &reclen);
				if(ret==2){
				//	usleep(100);
					continue;
				}else if(ret==1){
					serverip=0;
					connectflag=CLIENT_UNCONNECTED;
					close(tcp_server);
					DEBUG_PRINTF("\n receive server error\n");
					continue;
				}else{
					DEBUG_PRINTF("receive somthing from tcp server %d\n",(unsigned int)reclen);
					current_link_dev=0;
				   client_time=0;
					resolve_package(bufin);
				}
			}
		}
		return (void *)NULL;
}
/*******************************************************************************
* 函数名称: receive_client_thread
* 功    能:接收客户端数据线程
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/10/24  V1.0     H4     无       ：
*******************************************************************************/
void *receive_client_thread(void *arg)
{
		int ret ,i;
		unsigned char bufin[MAX_CONNECT_NUMBER][MAX_SEND_LENGTH];
		unsigned long  reclen;
		DEBUG_PRINTF("\n receive thread is ok \n");
		for(;;){
			usleep(100*1000);
			for(i=1;i<MAX_CONNECT_NUMBER;i++){
				if(connect_buffer[i].connect_status==CLIENT_CONNECTED){
					ret=socket_receive(connect_buffer[i].socket,bufin[i],MAX_SEND_LENGTH, MAX_WAITE_US , &reclen);
					if((ret==2)){
						//sleep(1);
						continue;
					}else if(ret==1){
						connect_buffer[i].connect_status=CLIENT_UNCONNECTED;
						connect_buffer[i].time=0;
						close(connect_buffer[i].socket);
						free(connect_buffer[i].p_info);
						DEBUG_PRINTF("receive client %d error\n",connect_buffer[i].dev);
						connect_buffer[i].dev=0;
						continue;
					}else{
						DEBUG_PRINTF("\n receive somthing from tcp client %d\n",connect_buffer[i].dev);
						current_link_dev=connect_buffer[i].dev;
						connect_buffer[i].time=0;
						resolve_package(bufin[i]);
					}
				}
			}
		}
		return (void *)NULL;
}
/*******************************************************************************
* 函数名称:time_transimt
* 功    能:定时线程，
*			完成定时检测透传包和定时发送心跳包
*			并能定时检测链接是否正常
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2013/1/17  V1.0     H4     无       ：
*******************************************************************************/
void * time_transimt(void *arg)
{
	for(;;){
		usleep(10*1000);
		if(connectflag==CLIENT_CONNECTED){
			transimt_deal();
		}
		reboot_timing();
		if(get_device_type()==DEVICE_TYPE_MAIN){
			delay_measure_thread();
			time_control_delay();
		}
	}
}
/*******************************************************************************
* 函数名称:time_send_thread 
* 功    能:定时线程，
*			完成定时检测透传包和定时发送心跳包
*			并能定时检测链接是否正常
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/10/24  V1.0     H4     无       ：
*******************************************************************************/
extern SelfThread_t g_UDPComThread;
void * time_send_thread(void *arg)
{
	for(;;){
		sleep(1);
		if(get_device_type()==DEVICE_TYPE_MAIN){
			if(connectflag==CLIENT_UNCONNECTED){
				;
			}else{
				check_connect();	
			}
		}else{
			if(connectflag==CLIENT_CONNECTED){
				timing_rru();
			}else{
				hardware_init();
				sleep(2);
				UDPComThreadStart();
				if(dru_tcp_client_init()==0){
				   client_time=0;
				   sleep(5);
				   create_ch_req();
					connectflag=CLIENT_CONNECTED;
					DEBUG_PRINTF("IR client connect server is ok!\n");
			   }else{
				   sleep(1);
			   }
			}
		}
	}
}
/*******************************************************************************
* 函数名称: socket_send
* 功    能:socket 发送
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/10/24  V1.0     H4     无       ：
*******************************************************************************/
int socket_send(unsigned int index,unsigned char *pbuffer,unsigned int blenght,unsigned char type)
{
	int ret;
	if(NULL==pbuffer){
		DEBUG_PRINTF("the address of send buffer is NULL\n");
		return 1;
	}
	if(index>=MAX_CONNECT_NUMBER){
		DEBUG_PRINTF("the index out of the limit\n");
		return 1;
	}
	if(blenght>MAX_SEND_LENGTH){
		DEBUG_PRINTF("the lenght is out of the max\n");
		return 1;
	}
	if((type>3)||(type<1)){
		DEBUG_PRINTF("the type is error\n");
		return 1;
	}
	if(get_device_type()==TYPE_MAIN){
		if(connect_buffer[index].connect_status==CLIENT_CONNECTED){
				ret=send(connect_buffer[index].socket,pbuffer,blenght,TCP_SEND_FLAGS);
				if(ret==-1){
					DEBUG_PRINTF("send error \n");
					return 1;
				}else{
					DEBUG_PRINTF("server send to client %d\n",connect_buffer[index].socket);
					return 0;
				}
		}else{
			DEBUG_PRINTF("this client is  unconnetcted\n");
			return 1;
		}
	}else{
			ret=send(tcp_server,pbuffer,blenght,TCP_SEND_FLAGS);
			if(ret==-1){
				DEBUG_PRINTF("send error \n");
				return 1;
			}else{
				return 0;
			}
	}
}
/*******************************************************************************
* 函数名称:start_recv_server 
* 功    能:创建接收服务器线程
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/12/07  V1.0     H4     无       ：
*******************************************************************************/
void start_recv_server(void)
{
	int err;
	for(;;){
		err=pthread_create(&client_thread,NULL,receive_server_thread,NULL);
		if(err!=0){
			DEBUG_PRINTF("\n receive server thread error  \n");
			sleep(1);
			continue;
		}else{
			DEBUG_PRINTF("\n receive server thread ok \n");
			break;
		}
	}
}
/***********************************************************************
** Funcation Name : software_downloard_deal
** Author         : H4
** Description    : a thread to updata the software 
** Input          : 
** Output         : 
** date           : 2013年04月25日 星期四 15时14分05秒
** Version        : V1.0
** Modify         : 
***********************************************************************/
thread_struct update_thread={
	.control = THREAD_EXIT,	
	.status = THREAD_EXIT,
};
const char * dru_name="dru";
const char * dru_save_name="/flashDev/data/dru";
const char * efpga_name="fpga_exp.rbf";
const char * efpga_save_name="/flashDev/data/fpga_exp.rbf";
const char * rfpga_name="fpga_rrs.rbf";
const char * rfpga_save_name="/flashDev/data/fpga_rru.rbf";
const char * dru_pkg_name="dru_pkg.tar.gz";
const char * dru_pkg_save_name="/flashDev/data/dru_pkg.tar.gz";
void *update_software_thread(void *arg)
{
	int ret;
	unsigned long ftp_sip;
	ftp_sip=serverip;
	DEBUG_PRINTF("UPDATE_sOFTWARE_THREAD\r\n");
	ret=FtpGet(ftp_sip,NULL,NULL,dru_pkg_name,dru_pkg_save_name);
	if(ret==-1){
		DEBUG_PRINTF("DRU DONWLOARD ERROR\r\n");
		goto _OUT;
	}
	system("cp /flashDev/data/dru_pkg.tar.gz /flashDev/program/dru_pkg.tar.gz");
	sync();
_OUT:
	update_thread.status=THREAD_EXIT;
	system("reboot");
	
}
void software_downloard_deal(void)
{
	pthread_t pid;
	if(update_thread.status!=THREAD_RUNNING){
		DEBUG_PRINTF("BOOT THE UPDATE_THREAD\r\n");
		update_thread.status=THREAD_RUNNING;
		if(pthread_create(&pid,0,update_software_thread,0)){
			DEBUG_PRINTF("pthread_creat hardware_alarm_thread error.\r\n");
			update_thread.status=THREAD_NULL;
			return 1;
		}
	}
}
/*******************************************************************************
* 函数名称:exit_recv_server 
* 功    能:退出接收服务器线程
* 参    数:
* 参数名称         类型                描述
* 返回值:
* 
* 说   明:
* 日   期     版本    作者   修改人      DEBUG
* -----------------------------------------------------------------------------
* 2012/12/07  V1.0     H4     无       ：
*******************************************************************************/
void exit_recv_server(void)
{
	pthread_kill(client_thread,0);
	sleep(3);
	DEBUG_PRINTF("\n receive server thread is exit\n");
}

int display_change(int argc, char * argv[])
{
	msg_tmp.mtype = MSG_FUN_RESULT;	
	sprintf(msg_tmp.mtext, "display...\n");
	if(argc != 1){
		printf("input para cnt is not 1.\r\n");
		sprintf(msg_tmp.mtext, "input para cnt is not 1.\r\n");
		msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
		return 1;
	}
	display();	
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}



