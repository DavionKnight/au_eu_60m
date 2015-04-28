/********************  COPYRIGHT(C)***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: netcom.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��:
**���򿪷�������
**��        ��: ���紦�����
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
#include "../common/commonfun.h"
#include "../protocol/approtocol.h"
#include "../protocol/apbprotocol.h"
#include "../protocol/apcprotocol.h"
#include "netcom.h"

struct sockaddr_in Receive_addr;	//���յ�������ip��ַ
extern DevicePara_t g_DevicePara;

/**********************************************************************
* �������ƣ�int GetNetlinkStatus(const char *if_name) 
* ����������������������Ƿ�Ͽ�
* ��������� const char *��Ҫ�������ڣ��磺eth0��eth1��
* �����������������0,�Ͽ�����-1
***********************************************************************/
int GetNetlinkStatus(const char *if_name)
{
struct ifreq ifr;
int res, skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, if_name);
	res = -1;
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		res = -1;
	}
	else
	{
		if(ifr.ifr_flags & IFF_RUNNING)
		{
			res = 0;//�����Ѳ�������
		}
	 	else
	 	{
	 		res = -1;
	 	}
	}
	close(skfd);
	if (res == 0)
	{
		usleep(10000);
	}
	else
	{
		sleep(1);
	}
	return res;
}

/*******************************************************************************
*�������� : int GetSelfMac(char *ethname, char *pbuf)
*��    �� : ��ȡ��������"devname"��MAC��ַ
*������� : char *devname:Ҫ��ȡ���豸����,eth0/ppp0;char *pbuf:�ش�ָ��
*������� : �ɹ�����1
*******************************************************************************/
int GetSelfMac(char *ethname, char *pbuf)
{
int i, skfd;
char buf[50];
struct ifreq ifr;

  skfd = socket(AF_INET, SOCK_STREAM, 0);
  if (skfd == -1)
  {
    sprintf(buf, "%s socket:", ethname);
    perror(buf);
    return 0;
  }
  strcpy(ifr.ifr_name, ethname);
  if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
  {
    sprintf(buf, "��ȡ %s MAC Error:", ethname);
    perror(buf);
    close(skfd);
    return 0;
  }
  close(skfd);
  for (i = 0; i < 6; i++)
  {
    *pbuf++ = (char)ifr.ifr_hwaddr.sa_data[i];
  }
  for (i = 0; i < 6; i++)
  {
    if ((char)ifr.ifr_hwaddr.sa_data[i] != 0)
    {
      DEBUGOUT("%s MAC", ethname);
      for (i = 0; i < 6; i++)
        DEBUGOUT(":%02X", ifr.ifr_hwaddr.sa_data[i]);
      DEBUGOUT("\r\n");
      return 1;
    }
  }
  return 0;
}

/*******************************************************************************
*�������� : INT32U GetSelfIp(char *ethname)
*��    �� : ��ȡ��������"devname"��ip��ַ,��gprsͨѶʱ���ڶ�����
*������� : char *devname:Ҫ��ȡ���豸����,eth0/ppp0
*������� : �ɹ���ȡ����ip��ַ,���ص��������ֽ����IP��ַ
*******************************************************************************/
INT32U GetSelfIp(char *ethname)
{
int skfd;
char buf[50];
struct ifreq ifr; 
struct sockaddr_in saddr;

  skfd = socket(AF_INET, SOCK_STREAM, 0);
  if (skfd == -1)
  {
    DEBUGOUT(buf, "%s socket:", ethname);
    perror(buf);
    return 0;
  }
  strcpy(ifr.ifr_name, ethname);
  if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0)
  {
    DEBUGOUT(buf, "��ȡ %s IP Error:", ethname);
    perror(buf);
    close(skfd);
    return 0;
  }
  close(skfd);

  memcpy(&saddr, &ifr.ifr_addr, sizeof(saddr));
  inet_ntop(AF_INET, (void *)&saddr.sin_addr, buf, sizeof(buf));
  DEBUGOUT("%s IP:%s\r\n", ethname, buf);
  return saddr.sin_addr.s_addr;
}

/*******************************************************************************
*�������� : int SetDevIPpara(char *ethname, INT32U ip, char settype)
*��    �� : ���ñ�������"devname"��ip��ַ,��gprsͨѶʱ���ڶ�����
*������� : char *devname:Ҫ��ȡ���豸����,eth0/ppp0;INT32U ip:�豸IP��ַ,4�ֽڳ���������,char settype:IP��ַ,��������,����
*������� : �ɹ���������ip��ַ,����1,���򷵻�-1
*******************************************************************************/
int SetDevIPpara(char *ethname, INT32U ip, char settype)
{
int skfd, res = 0;
char buf[50], ipbuf[20];
struct ifreq ifr; 
struct sockaddr_in saddr;

  skfd = socket(AF_INET, SOCK_STREAM, 0);
  if (skfd == -1)
  {
    sprintf(buf, "%s socket:", ethname);
    perror(buf);
    return 0;
  }
  strcpy(ifr.ifr_name, ethname);
  saddr.sin_family = AF_INET;
  memcpy(&saddr.sin_addr, &ip, 4);
  memcpy(&ifr.ifr_addr, &saddr, sizeof(saddr));
  
  inet_ntop(AF_INET, (void *)&saddr.sin_addr, ipbuf, sizeof(ipbuf));
  if (settype == SET_IP)
  {
    DEBUGOUT("%s Set IP:%s\r\n", ethname, ipbuf);
    res = ioctl(skfd, SIOCSIFADDR, &ifr);
  }
  else if (settype == SET_NETMASK)
  {
    DEBUGOUT("%s Set NetMask:%s\r\n", ethname, ipbuf);
    res = ioctl(skfd, SIOCSIFNETMASK, &ifr);
  }
  else if (settype == SET_GATEWAY)
  {
    sprintf(buf, "/sbin/route add default gw %s\r", ipbuf);
    system(buf);
    DEBUGOUT("%s Set Gateway:%s.\r\n", ethname, ipbuf);
    res = 1;
  }
  if (res < 0)
  {
    sprintf(buf, "Set %s Error:", ethname);
    perror(buf);
    close(skfd);
    return 0;
  }
  close(skfd);
  return 1;
}

/*******************************************************************************
*�������� : int SocketServerInit(int serverport, char iptype)
*��    �� : SocketServerInit���Ӻ���
*������� : int serverport:server IP��ַ�˿ں�;char iptype:IP��������tcp/udp
*������� : fd
*******************************************************************************/
int SocketServerInit(int serverport, char iptype)
{
int er, skfd = -1;
struct sockaddr_in skaddr;

  if (iptype == IP_TCP_MODE)
  {
    skfd = socket(AF_INET, SOCK_STREAM, 0);
    DEBUGOUT("IP+TCP:");
  }
  else if (iptype == IP_UDP_MODE)
  {
    skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    DEBUGOUT("IP+UDP:");
  }
  if (skfd < 0)
  {
    DEBUGOUT("SocketServerInit:Create Socket Failed!\r\n");
    return skfd;
  }
  er = 1;
  //bind��������,Ĭ�������,һ���˿��ǲ������ظ��󶨵�,ͨ����������������˿��ظ���
  setsockopt(skfd, SOL_SOCKET, SO_REUSEADDR, &er, sizeof(er));
  
  //�ѿͻ�����socket�Ϳͻ�����IP��ַ,�˿���ϵ����
  bzero(&skaddr, sizeof(skaddr));
  skaddr.sin_family = AF_INET;
  skaddr.sin_port = htons(serverport);
  skaddr.sin_addr.s_addr = INADDR_ANY;//����IP
  if (bind(skfd, (struct sockaddr *)&skaddr, sizeof(skaddr)) < 0)
  {
    DEBUGOUT("SocketServerInit:Bind Port Failed!\r\n"); 
    perror("Socket bind:");
    return -1;
  }
  if (listen(skfd, SOMAXCONN) == -1)//����listen��ʼ����
  {
    DEBUGOUT("SocketServerInit:Listen Error!\r\n"); 
    return -1;
  }
  return skfd;
}

/*******************************************************************************
*�������� : int SocketClientConnet(struct combuf *pcombuf, char *devname, char *server_ip, int server_port)
*��    �� : SocketClient���Ӻ���
*������� : ComBuf_t *p_combuf:��Ӧ�豸����;char *ethname:�����������豸��;
            INT32U serverip:server IP��ַ; int serverport:server IP��ַ�˿ں�;
            INT32U devip:�豸IP��ַ; int devport:�豸IP��ַ�˿ں�;char iptype:IP��������tcp/udp
*������� : fd
*******************************************************************************/
int SocketClientConnet(ComBuf_t *pcombuf, INT32U serverip, int serverport, INT32U devip, int devport, char iptype)
{
int result, er;
socklen_t len;
fd_set fdr, fdw;
struct timeval  tm;
struct in_addr ipadder;
struct sockaddr_in skaddr;

  memcpy(&ipadder, &serverip, 4);
  DEBUGOUT("SocketClientConnet:ServerIp:%s;ServerIPPort:%d.\r\n", inet_ntoa(ipadder), serverport);
  memcpy(&ipadder, &devip, 4);
  DEBUGOUT("SocketClientConnet:DevIp:%s;DevIPPort:%d.\r\n", inet_ntoa(ipadder), devport);

  if (iptype == IP_TCP_MODE)
  {
    pcombuf->Fd = socket(AF_INET, SOCK_STREAM, 0);
    DEBUGOUT("IP+TCP:");
  }
  else if ((iptype == IP_UDP_MODE) || (iptype == UDP_BROADCAST_MODE))
  {
    pcombuf->Fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    DEBUGOUT("IP+UDP:");
  }
  if (pcombuf->Fd < 0)
  {
    DEBUGOUT("Create Socket Failed!\r\n");
    pcombuf->Status = NET_DISCONNET;
    close(pcombuf->Fd);
    return pcombuf->Fd;
  }
  er = 1;
  //bind��������,Ĭ�������,һ���˿��ǲ������ظ��󶨵�,ͨ����������������˿��ظ���
  setsockopt(pcombuf->Fd, SOL_SOCKET, SO_REUSEADDR, &er, sizeof(er));
  if (iptype == UDP_BROADCAST_MODE)//����㲥
  {
    er = 1;
    setsockopt(pcombuf->Fd, SOL_SOCKET, SO_BROADCAST, &er, sizeof(er));
  }

  //�ѿͻ�����socket�Ϳͻ�����IP��ַ,�˿���ϵ����
  bzero(&skaddr, sizeof(skaddr));
  skaddr.sin_family = AF_INET;
  skaddr.sin_port = htons(devport);//�豸�˿�
  skaddr.sin_addr.s_addr = devip;//�豸IP
  if (bind(pcombuf->Fd, (struct sockaddr *)&skaddr, sizeof(struct sockaddr)) < 0)
  {
    DEBUGOUT("Client Bind Port Failed!\n"); 
    close(pcombuf->Fd);
    return -1;
  }
  er = 4*1024;//����Ϊ4K
  setsockopt(pcombuf->Fd, SOL_SOCKET, SO_RCVBUF, &er, sizeof(er));//���ջ�����
  setsockopt(pcombuf->Fd, SOL_SOCKET, SO_SNDBUF, &er, sizeof(er));//���ͻ�����

  if (iptype == IP_TCP_MODE)
  {
    DEBUGOUT("���������������......\r\n");
    //��ȡ������pcombuf->Fd�ĵ�ǰ״̬
    er = fcntl(pcombuf->Fd, F_GETFL, 0);
    if (er < 0)
    {
      perror("fcntl(Socket,F_GETFL,0)");
      close(pcombuf->Fd);
      return -1;
    }
    er = er | O_NONBLOCK;//����Ϊ������ģʽ
    if (fcntl(pcombuf->Fd, F_SETFL, er) < 0)
    {
      perror("fcntl(Socket,F_SETFL,0)");
      close(pcombuf->Fd);
      return -1;
    }
    
    bzero(&skaddr, sizeof(skaddr));
    skaddr.sin_family = AF_INET;
    skaddr.sin_port = htons(serverport);//�������˿�
    skaddr.sin_addr.s_addr = serverip;//������IP
  //connect���ú�,���������Ƿ�����������-1,ͬʱ��errno(����errno.h�Ϳ���ֱ��ʹ��)����ΪEINPROGRESS
  //��ʾ��ʱtcp���������Ծɽ���,���errno����EINPROGRESS,��˵�����Ӵ���,���������
    if (connect(pcombuf->Fd, (struct sockaddr *)&skaddr, sizeof(skaddr)) < 0)
    {
      //��connect����,��ʱ����-1,����errno����ΪEINPROGRESS,�⼴connect�Ծ��ڽ��л�û�����
      if (errno != EINPROGRESS)
      {
        perror("Socket Connect");
        return -1;
      }

      tm.tv_sec = 6;//6s 
      tm.tv_usec = 0; 
      FD_ZERO(&fdr);
      FD_ZERO(&fdw);
      FD_SET(pcombuf->Fd, &fdr);
      FD_SET(pcombuf->Fd, &fdw);

      er = select(pcombuf->Fd + 1, &fdr, &fdw, NULL, &tm);
      if (er > 0)
      {
        if (FD_ISSET(pcombuf->Fd, &fdr) || FD_ISSET(pcombuf->Fd, &fdw))//����ж���д��������
        {
          len = sizeof(er);
          if (getsockopt(pcombuf->Fd, SOL_SOCKET, SO_ERROR, &er, &len) < 0)
            result = 0; 
      
          if (er == 0)//������ӳɹ�,�˵��÷���0
            result = 1;
          else
            result = 0; 
        }
      }
      else
      {
        DEBUGOUT("Socket Connect Timeout!\r\n");
        close(pcombuf->Fd);
        result = 0;
      }
    }
    else
    {
      result = 1;
    }

    if (result == 1)
    {
      DEBUGOUT("�������(TCP)���ӳɹ�!\r\n");
      pcombuf->Status = NET_CONNET;
      return pcombuf->Fd;
    }
    else
    {
      DEBUGOUT("�������(TCP)����ʧ��!\r\n");
      pcombuf->Status = NET_DISCONNET;
      close(pcombuf->Fd);
      return -1;
    }
  }
  else//UDP
  {
    pcombuf->Status = NET_CONNET;
    return pcombuf->Fd;
  }
}

/*******************************************************************************
*�������� : void SocketClientDisconnet(struct combuf *pcombuf)
*��    �� : �Ͽ�SocketClient���Ӻ���
*������� : struct combuf *pcombuf:��Ӧ�豸����
*������� : none
*******************************************************************************/
void SocketClientDisconnet(ComBuf_t *pcombuf)
{
  if (pcombuf->Fd > 0)
  {
    DEBUGOUT("Fd:%d,�Ͽ��������������!\r\n", pcombuf->Fd);
    shutdown(pcombuf->Fd, SHUT_RDWR);//close(pcombuf->Fd);
    pcombuf->Fd = -1;
  }
  pcombuf->Status = NET_DISCONNET;
}

/*******************************************************************************
*�������� :	int	TCPSocketReceiveData(ComBuf_t *pcombuf, int waittime)
*��    �� :	���յ�����ת�浽PackBuf���ݻ�������,������
*������� : ComBuf_t *p_combuf:��Ӧ�豸����;int waittime:��ʱʱ��ms
*������� : ���յ����ݳ��Ȼ�����ʶ
*******************************************************************************/
int	TCPSocketReceiveData(ComBuf_t *pcombuf, int waittime)
{
int	res, rcsum;
fd_set readfs;
struct timeval tv;

	tv.tv_sec = waittime/1000;
	tv.tv_usec = (waittime%1000)*1000;
	FD_ZERO(&readfs);
	FD_SET(pcombuf->Fd, &readfs);

	res = select(pcombuf->Fd + 1, &readfs, NULL, NULL, &tv);
  if (res > 0)
  {
    rcsum = recv(pcombuf->Fd, &pcombuf->Buf[pcombuf->RecvLen], (COMBUF_SIZE - pcombuf->RecvLen), 0);
    if (rcsum > 0)
    {
  		pcombuf->RecvLen = pcombuf->RecvLen + rcsum;
  		return pcombuf->RecvLen;
  	}
  	else
  	{
  	  perror("TCPSocketReceiveData:recv() error!");
      //�����Ƿ�������ģʽ,���Ե�errnoΪEAGAINʱ,��ʾ��ǰ�������������ݿɶ�;�������ʾ���¼���������
      if (rcsum == -1)
      {
      	if(errno == EINTR || errno == EAGAIN)
        	return pcombuf->RecvLen;
        else
        	goto _SOCKET_SHUTDOWN;
      }
      else
      {
    	  //���ӹض�
_SOCKET_SHUTDOWN:
        shutdown(pcombuf->Fd, SHUT_RDWR);//close(g_NetCom.Fd);
        pcombuf->Status = NET_DISCONNET;
        DEBUGOUT("TCPSocketReceiveData:recv() Error, SHUT_RDWR!\r\n");
        return -1;
      }
  	}
  }
  else if (res < 0)
  {
    perror("TCPSocketReceiveData:select() Error!");
    return -1;
  }
  return pcombuf->RecvLen;
}

/*******************************************************************************
*�������� : int	TCPSocketSendData(int fd, char *sbuf, int len)
*��    �� : TCPSocket->fd ��������
*������� : TCPSocket->fd:sbuf:����;len:��
*������� : �����ɹ�����1�����򷵻�-1
*******************************************************************************/
int	TCPSocketSendData(int fd, char *sbuf, int len)
{
  //����ΪMSG_NOSIGNAL����ʾ�������ϵͳ���źţ����������˳�
  if (send(fd, sbuf, len, MSG_NOSIGNAL) == -1)
  {
    perror("can't send message:");
    DEBUGOUT("TCPSocket can't send message!\r\n");//�������Ϣ
    return -1;
  }
  return 1;
}

/*******************************************************************************
*�������� : int TCPSocketSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf)
*��    �� : ����Э���,��p_packbuf��ָ�Ĵ��������ݷ��͸�pcombuf��ָ�豸
*������� : ComBuf_t *pcombuf:��Ӧ�豸����;APPack_t *p_packbuf:��������Ӧ���ݰ��ṹָ��
*������� : �ɹ�����1,���򷵻�-1
*******************************************************************************/
int TCPSocketSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf)
{
int sdsum;
char sdbuf[COMBUF_SIZE];

	sdsum = 0;
	switch(p_packbuf->APType)
	{
	  case AP_A:
	  case AP_C:
	    sdsum = APCPack(p_packbuf, sdbuf);
    break;
    case AP_B:
      sdsum = APBPack(p_packbuf, sdbuf);
    break;
	}
	if(sdsum > 0)
	{
	  DEBUGOUT("Fd:%d.TCPSocketSendData:%d.\r\n", pcombuf->Fd, sdsum);
		TCPSocketSendData(pcombuf->Fd, sdbuf, sdsum);
   	ComDataWriteLog(sdbuf, sdsum);
   	return 1;
	}
	else
	  return -1;
}

/*******************************************************************************
*�������� :	int	UDPSocketReceiveData(ComBuf_t *pcombuf, INT32U serverip, int serverport, int waittime)
*��    �� :	���յ�����ת�浽PackBuf���ݻ�������,������
*������� : ComBuf_t *p_combuf:��Ӧ�豸����;
            INT32U serverip:server IP��ַ; int serverport:server IP��ַ�˿ں�;int waittime:��ʱʱ��ms
*������� : ���յ����ݳ��Ȼ�����ʶ
*******************************************************************************/
int	UDPSocketReceiveData(ComBuf_t *pcombuf, INT32U serverip, int serverport, int waittime)
{
	int	res, rcsum;
	socklen_t addlen;
	fd_set readfs;
	struct timeval tv;
	struct sockaddr_in skaddr;

	//----2015-03-10----------
	DevicePara_t *p_devpara;
	p_devpara = &g_DevicePara;
	//------------------------

	tv.tv_sec = waittime/1000;		//SECOND
	tv.tv_usec = (waittime%1000)*1000;	//USECOND
	FD_ZERO(&readfs);
	FD_SET(pcombuf->Fd, &readfs);

	res = select(pcombuf->Fd + 1, &readfs, NULL, NULL, &tv);
  	if (res > 0)
  	{
	    bzero(&skaddr, sizeof(skaddr));
	    skaddr.sin_family = AF_INET;
	    skaddr.sin_port = htons(serverport);//�������˿�
	    skaddr.sin_addr.s_addr = htonl(serverip);//������IP
	    addlen = sizeof(struct sockaddr);
	    rcsum = recvfrom(pcombuf->Fd, &pcombuf->Buf[pcombuf->RecvLen], (COMBUF_SIZE - pcombuf->RecvLen),
	                     0, (struct sockaddr *)&skaddr, &addlen);
	    if (rcsum>0)
	    {
			//-------2015-03-10---------------
			if((p_devpara->RW1_OmcIP == skaddr.sin_addr.s_addr)||(p_devpara->RW2_OmcIP == skaddr.sin_addr.s_addr)
				||(p_devpara->OmcIP == skaddr.sin_addr.s_addr))
				Receive_addr.sin_addr.s_addr = skaddr.sin_addr.s_addr;
			//printf("================= Receive_IP = %d\n",skaddr.sin_addr.s_addr);
			//--------------------------------
			
			pcombuf->RecvLen = pcombuf->RecvLen + rcsum;
	  		return pcombuf->RecvLen;
	  	}
	  	else
	  	{
	  	  return -1;
	  	}
  	}
  	else if (res < 0)
	{
		perror("Socket select() error!");
		return -1;
	}
//	printf("UDPSocketReceiveData time out!\n");
	return pcombuf->RecvLen;
}
/*
int	UDPSocketReceiveData(ComBuf_t *pcombuf, INT32U serverip, int serverport, int waittime)
{
	int	res, rcsum;
	int ReceiveBufNum;
	socklen_t addlen;
	fd_set readfs;
	struct timeval tv;
	struct sockaddr_in skaddr;

	tv.tv_sec = waittime/1000;		//SECOND
	tv.tv_usec = (waittime%1000)*1000;	//USECOND
	FD_ZERO(&readfs);
	FD_SET(pcombuf->Fd, &readfs);

	res = select(pcombuf->Fd + 1, &readfs, NULL, NULL, &tv);
	if (res > 0)
	{
		bzero(&skaddr, sizeof(skaddr));
		skaddr.sin_family = AF_INET;
		skaddr.sin_port = htons(serverport);		//�������˿�
		skaddr.sin_addr.s_addr = htonl(serverip);	//������IP
		addlen = sizeof(struct sockaddr);
		
		ReceiveBufNum = 0;
		while(1)
		{
			rcsum = recvfrom(pcombuf->Fd, &pcombuf->Buf[pcombuf->RecvLen], (COMBUF_SIZE - pcombuf->RecvLen),
							 0, (struct sockaddr *)&skaddr, &addlen);
			printf("====aaaaaaaaaaaaaaaaaaaaa rcsum = %d,ReceiveBufNum = %d\n", rcsum,ReceiveBufNum);
			
			if (rcsum > 0)
			{
				ReceiveBufNum++;
				pcombuf->RecvLen = pcombuf->RecvLen + rcsum;
				ComDataHexDis(pcombuf->Buf, pcombuf->RecvLen);
				printf("==================== ReceiveDataNumber = %d, ReceiveBufNumber %d\n",rcsum,ReceiveBufNum);
				usleep(2*1000);
			}
			else
			{
				if(ReceiveBufNum>0)
				{
					printf("===================== DataNumber = %d, ReceiveBufNumber %d\n",pcombuf->RecvLen,ReceiveBufNum);
					return pcombuf->RecvLen;
				}
				else
				{
					printf("========================== No Data%d\n");
					return -1;
				}
			}
		}
	}
	else if (res < 0)
	{
		perror("Socket select() error!");
		return -1;
	}
	return pcombuf->RecvLen;
}
*/
/*******************************************************************************
*�������� : int	UDPSocketSendData(int fd, char *sbuf, int len, INT32U serverip, int serverport)
*��    �� : UDPSocket->fd ��������
*������� : UDPSocket->fd:sbuf:����;len:��;INT32U serverip:server IP��ַ; int serverport:server IP��ַ�˿ں�;
*������� : �����ɹ�����1�����򷵻�-1
*******************************************************************************/
int	UDPSocketSendData(int fd, char *sbuf, int len, INT32U serverip, int serverport)
{
	struct sockaddr_in skaddr;
		
	bzero(&skaddr, sizeof(skaddr));
	skaddr.sin_family = AF_INET;
	skaddr.sin_port = htons(serverport);//�������˿�
	skaddr.sin_addr.s_addr = serverip;//������IP	

	//����ΪMSG_NOSIGNAL����ʾ�������ϵͳ���źţ����������˳�
	if (sendto(fd, sbuf, len, MSG_NOSIGNAL, (struct sockaddr *)&skaddr, sizeof(skaddr)) == -1)
	{
		perror("can't send message:");
		DEBUGOUT("UDPSocket can't send message!\r\n");//�������Ϣ
		return -1;
	}

	return 1;
}

/*******************************************************************************
*�������� : int UDPSocketSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf, INT32U serverip, int serverport)
*��    �� : ����Э���,��p_packbuf��ָ�Ĵ��������ݷ��͸�pcombuf��ָ�豸
*������� : ComBuf_t *pcombuf:��Ӧ�豸����;APPack_t *p_packbuf:��������Ӧ���ݰ��ṹָ��;
            INT32U serverip:server IP��ַ; int serverport:server IP��ַ�˿ں�;
*������� : �ɹ�����1,���򷵻�-1
*******************************************************************************/
int UDPSocketSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf, INT32U serverip, int serverport)
{
int sdsum;
char sdbuf[COMBUF_SIZE];

	sdsum = 0;
	switch(p_packbuf->APType)
	{
	  case AP_A:
	  case AP_C:
	    sdsum = APCPack(p_packbuf, sdbuf);
    break;
    case AP_B:
      sdsum = APBPack(p_packbuf, sdbuf);
    break;
    default:
			DEBUGOUT("APType Error!\r\n");
    	ClearAPPackBuf(p_packbuf);//�����
   	break;
	}
	if(sdsum > 0)
	{
	  DEBUGOUT("Fd:%d.UDPSocketSendData:%d.\r\n", pcombuf->Fd, sdsum);
		UDPSocketSendData(pcombuf->Fd, sdbuf, sdsum, serverip, serverport);
   	ComDataWriteLog(sdbuf, sdsum);
   	return 1;
	}
	else
	  return -1;
}
/*
���ز�ѯIP��ַ
*/
INT32U GetReceiveIP(void)
{
	INT32U ReceiveIP;
	DevicePara_t *p_devpara;
	
	p_devpara = &g_DevicePara;
	ReceiveIP = p_devpara->OmcIP;
	
	if((p_devpara->RW1_OmcIP == Receive_addr.sin_addr.s_addr)||(p_devpara->RW2_OmcIP == Receive_addr.sin_addr.s_addr))
	{
		ReceiveIP = Receive_addr.sin_addr.s_addr;
		//Receive_addr.sin_addr.s_addr = 0;
	}
	printf("=============== OmcIPIP_0 = %d\n" ,p_devpara->OmcIP);
	printf("=============== OmcIPIP_1 = %d\n" ,p_devpara->RW1_OmcIP);
	printf("=============== OmcIPIP_2 = %d\n" ,p_devpara->RW2_OmcIP);
	printf("=============== Send_IP = %d\n" ,ReceiveIP);
	
	return ReceiveIP;
}

/*********************************End Of File*************************************/
