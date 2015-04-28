/********************  COPYRIGHT(C) ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: druheader.h
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: ������ñ������õ�ͷ�ļ�
**--------------��ʷ�汾��Ϣ----------------------------------------------------
** ������: �ں�ͼ
** ��  ��: v1.0
** �ա���: 
** �衡��: ԭʼ�汾
**--------------��ǰ�汾�޶�----------------------------------------------------
** �޸���:
** ��  ��:
** �ա���:
** �衡��:
**------------------------------------------------------------------------------
**
*******************************************************************************/
#ifndef _DRUHEADER_H
#define _DRUHEADER_H

#include <ctype.h> 		//isprint isupper isspace
#include <errno.h>    //������Ŷ���
#include <fcntl.h>    //�ļ����ƶ���
#include <stdio.h>		//��׼�������
#include <stdlib.h>		//��׼�⺯��
#include <string.h>   //�ַ������ܺ���
#include <stropts.h>	//ioctl
#include <signal.h>
#include <termios.h>  //POSIX�ն˿��ƶ���
#include <time.h>
#include <unistd.h>	  //Unix��׼��������
#include <math.h>

//#include <sys/queue.h>
//#include <sys/signal.h>
#include <sys/socket.h> //for socket
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>  //for socket

#include <net/if.h>
#include <netinet/in.h> //for sockaddr_in
#include <arpa/inet.h>

#include <poll.h>
#include <sys/epoll.h> 

//#include <stdarg.h>
//#include <linux/rtc.h>
#include <linux/sched.h>
#include <semaphore.h>
#include <pthread.h>
#include <sqlite3.h>

#include "usertype.h"
#include "apparadef.h"
#include "drudefstruct.h"

#define PRINTDEBUG    0	//������Ϣ�������

#define DEBUGOUT printf
//#define DEBUGOUT 

#define MAIN_UNIT     1 //����Ԫ
#define EXPAND_UNIT   2 //��չ��Ԫ
#define RAU_UNIT      3 //Զ�˵�Ԫ

#define LOCALCOM      2 //����RS485�ӿڽ��б���ͨѶ
#define MODEMUART     2 //MODEMͨѶ��

#define CONFIGPATH    "/flashDev/program/config"
#define	LOGSAVEDPATH  "/flashDev/data/log"
#define	LOGFILENUM    10
#define	LOGFILESIZE   0x100000


#define SET_IP                  1   //1:����IP��ַ
#define SET_NETMASK             2   //2:������������
#define SET_GATEWAY             3   //3:����Ĭ������

#define UDP_BROADCAST_MODE      3   //3:ʹ��UDP�㲥ģʽ

#define NET_NULL                0
#define NET_CONNET              0xC0
#define NET_DISCONNET           0xCD

#define LOGINSUCCESS            0xCF
#define NETCONNETINTERVAL       60  //����ͨ����·ʱ����60s
#define LOGININTERVAL           60  //������ĵ�¼ʱ����60s
#define SOCKETTIMEOUT           3   //������ĵ�¼ʱ����3s

#define CONNETFAILSUM           3   //����ʧ�ܴ���
#define LOGINFAILSUM            3   //loginʧ�ܴ���
#define NOHEARTBEATSUM          3   //���������������

#define IRCONNETINTERVAL        10  //IRЭ�齨��ͨ����·ʱ����6s
#define IRLOGININTERVAL         10  //IRЭ��client��¼serverʱ����s

#define IRCONNETFAILSUM         3   //IRЭ������ʧ�ܴ���
#define IRLOGINFAILSUM          3   //IRЭ��loginʧ�ܴ���
#define IRNOHEARTBEATSUM        3   //IRЭ�����������������

#define IRTCPRMUPORT            30000 //IRЭ��TCP��������Ԫ�˿ں�
#define IRUDPRMUPORT            33333 //IRЭ��UDP�㲥����Ԫ�˿ں�
#define IRUDPRAUPORT            33334 //IRЭ��UDP�㲥��չ,Զ�˶˿ں�

#define COMWAITTIME             0//500

#define THREAD_STATUS_NEW       0 //�̵߳�״̬���½�
#define THREAD_STATUS_RUNNING   1 //�̵߳�״̬����������
#define THREAD_STATUS_START		2 //�̵߳�״̬����������
#define THREAD_STATUS_EXIT     -1 //�̵߳�״̬�����н���

#define CONNECT_TOTALNUMBER     255 //server������
#define DEFAULT_STARTIP         2  //ϵͳĬ����ʼIP��ַ

#endif//_DRUHEADER_H
/*********************************End Of File*************************************/
