/********************  COPYRIGHT(C)  ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: uartdev.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: ���ڴ��ڲ���
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
#include "../../common/commonfun.h"
#include "../../protocol/approtocol.h"
#include "../../protocol/apbprotocol.h"
#include "../../protocol/apcprotocol.h"
#include "uartdev.h"

//int  g_UartRecIsrFlag = 0;  //UART�����жϱ�ʶ,�յ�1,δ�յ�Ϊ0
/*******************************************************************************
*�������� : void  Uart_ISR(int  signo, siginfo_t *info,void *context)
*��    �� : ͨѶ�ж��źŴ������,��UART�ж�g_UartIsrFlag��1
*������� : None
*������� : None
*******************************************************************************/
/*
void Uart_ISR(int signo, siginfo_t *info, void *context)
{
#if PRINTDEBUG
  printf("UART signo:%04x;si_code:%04x.\r\n", signo,info->si_code);
  printf("UARTSIGPOLL:%04x;%04x;%04x;%04x;%04x;%04x.\r\n", POLL_IN,POLL_OUT,POLL_MSG,POLL_ERR,POLL_PRI,POLL_HUP);
#endif
  g_UartRecIsrFlag = 1;
}
*/

/*******************************************************************************
*�������� : int OpenCommPortISR(int fd, int comport, struct sigaction saio) 
*��    �� : �жϷ�ʽ�򿪴���comport,��װ�źŴ���saio
*������� : ���ں�,�ź�
*������� : fd
*******************************************************************************/
int OpenCommPortISR(int fd, int comport, struct sigaction saio) 
{
char *dev[5]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4"};

  printf("Open Serial Port%d ISR:\n", comport);
  //����װ��Ϊ non-blocking (��ȡ���ܻ����Ͻ�������)
  fd = open(dev[comport], O_RDWR|O_NOCTTY|O_NONBLOCK); 
  if (fd == -1)
  { 
    perror("Failed"); 
    return -1; 
  }
  //��ʹװ�÷�ͬ����ǰ, ��װ�źŴ������
  //sigaction(SIGIO, &saio, NULL); //SIGPOLL��SISTEM V���ź�,BSDϵͳ��SIGIO;��SVR4��,SIGIO��SIGPOLL��ͬ
  sigaction(SIGPOLL, &saio, NULL);
  
  //ioctl(fd1 , I_SETSIG , S_INPUT | S_HANGUP ); //���õ��ж�����ʱ���� SIGPOLL�ź�
  ioctl(fd, I_SETSIG, POLL_IN); //���õ��ж�����ʱ���� SIGPOLL�ź�
  //ioctl(fd, I_SETSIG, S_INPUT);

  //�������ȥ���� SIGIO �ź�
  fcntl(fd, F_SETOWN, getpid());
  //���ô��ڵ��ļ�������Ϊ�첽,man��˵,ֻ�� O_APPEND �� O_NONBLOCK ����ʹ��F_SETFL...
  fcntl(fd, F_SETFL, O_ASYNC|O_NONBLOCK);//FASYNC);
  return fd;
}

/*******************************************************************************
*�������� : int OpenCommPort(int fd, int comport)
*��    �� : �򿪴���comport
*������� : ���ں�
*������� : fd
*******************************************************************************/
int OpenCommPort(int fd, int comport)
{
int flags;
char *dev[5]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4"};

  printf("Open Serial Port%d:\r\n", comport);
  fd = open(dev[comport], O_RDWR|O_NOCTTY|O_NDELAY|O_NONBLOCK);  //���������մ�������
  if(fd == -1)
  { 
    perror("Failed"); 
    return(-1); 
  }
  //�ڴ򿪴��ڳɹ���,ʹ��fcntl(fd, F_SETFL, FNDELAY)���,����ʹread�����������ض���������FNDELAYѡ��ʹread�����ڴ������ַ�ʱ����������Ϊ0��

  //�жϴ��ڵ�״̬�Ƿ�Ϊ����״̬,
  //���ô��ڵ�״̬Ϊ����״̬,���ڵȴ��������ݵĶ���
  flags = fcntl(fd, F_GETFL, 0);
  flags = flags|O_NONBLOCK;//|FNDELAY;
  if(fcntl(fd, F_SETFL, flags) < 0) 
    printf("OpenCommPort:fcntl failed!\n"); 
  else 
    printf("fcntl=%d\n", fcntl(fd, F_SETFL, flags));
/*
  if(fcntl(fd, F_SETFL, 0) < 0) 
    printf("fcntl failed!\n"); 
  else 
    printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
*/
  //�����Ƿ�Ϊ�ն��豸
  if(isatty(STDIN_FILENO) == 0) 
    printf("Standard Input is not a Terminal device\n"); 
  else 
    printf("isatty success!\n"); 

  printf("fd-open=%d\n",fd); 
  return fd;
}

/*******************************************************************************
*�������� : int SetCommState(int fd, int nSpeed, int nBits, char nEvent, int nStop)
*��    �� : ���ô��ں���
*������� : (int fd, int nSpeed, int nBits, char nEvent, int nStop)
*������� : �ɹ�����1,������-1
*******************************************************************************/
int SetCommState(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{ 
struct termios newtio, oldtio; 
//termios:c_iflag:����ģʽ;c_oflag:���ģʽ;c_cflag:����ģʽ;c_lflag:����ģʽ;c_cc[NCCS]:�����ַ�

  //����������д��ڲ�������,������������ںŵȳ���,������صĳ�����Ϣ
  if(tcgetattr(fd, &oldtio)!=0)
  {  
    perror("SetupSerial:"); 
    return -1; 
  } 

  bzero(&newtio, sizeof(newtio));     //����ṹ���Է����µ����в��趨ֵ
  newtio.c_cflag  |=  CLOCAL | CREAD; //CLOCAL:��������, �������ݻ����ƹ���;CREAD:�򿪽�����
  newtio.c_cflag &= ~CSIZE;           //CSIZE:�ַ���������
  //����ֹͣλ
  switch(nBits)//����λȡֵΪ7����8 
  { 
    case 7: 
      newtio.c_cflag |= CS7; 
    break; 
    case 8: 
      newtio.c_cflag |= CS8; 
    break;
    default:
      newtio.c_cflag |= CS8; 
    break;
  } 
  //������żУ��λ
  switch( nEvent ) //Ч������ ȡֵΪO,E,N
  { 
    case 'O': //����
    case 'o':
      newtio.c_iflag |= (INPCK | ISTRIP); //INPCK:����������ż���,ISTRIP:��ȥ��żУ��λ,ȥ���ڰ�λ
      newtio.c_cflag |= (PARENB | PARODD);//����Ϊ��Ч��,PARENB:У��λʹ��;PARODD:������������У��
    break; 
    case 'E'://ż��
    case 'e':  
      newtio.c_iflag |= (INPCK | ISTRIP); //INPCK:����������ż���,ISTRIP:��ȥ��żУ��λ,ȥ���ڰ�λ
      newtio.c_cflag |= PARENB;           //PARENB:У��λʹ��
      newtio.c_cflag &= ~PARODD;          //PARODD:ʹ����У�����ʹ��żУ��->ת��ΪżЧ��
    break; 
    case 'N'://����żУ��λ
    case 'n':
      newtio.c_cflag &= ~PARENB;  //PARENB:У��λʹ��
    break;
    default:
      newtio.c_cflag &= ~PARENB;  //PARENB:У��λʹ��
    break;
  } 
  //���ô���ͨ������
  switch(nSpeed)
  { 
    case 2400: 
      cfsetispeed(&newtio, B2400); 
      cfsetospeed(&newtio, B2400); 
    break; 
    case 4800: 
      cfsetispeed(&newtio, B4800); 
      cfsetospeed(&newtio, B4800); 
    break; 
    case 9600: 
      cfsetispeed(&newtio, B9600); 
      cfsetospeed(&newtio, B9600); 
    break; 
    case 19200: 
      cfsetispeed(&newtio, B19200); 
      cfsetospeed(&newtio, B19200); 
    break;
    case 38400: 
      cfsetispeed(&newtio, B38400); 
      cfsetospeed(&newtio, B38400); 
    break;
    case 57600: 
      cfsetispeed(&newtio, B57600); 
      cfsetospeed(&newtio, B57600); 
    break; 
    case 115200: 
      cfsetispeed(&newtio, B115200); 
      cfsetospeed(&newtio, B115200); 
    break; 
    default: 
      cfsetispeed(&newtio, B9600); 
      cfsetospeed(&newtio, B9600); 
    break; 
  }
  //����ֹͣλ
  if( nStop == 1 ) //ֹͣλ ȡֵΪ 1 ����2
    newtio.c_cflag &=  ~CSTOPB; 
  else if ( nStop == 2 ) 
    newtio.c_cflag |=  CSTOPB; //��������ֹͣλ
    
  //������ǿ����ն�֮���,ֻ�Ǵ��ڴ�������,������Ҫ����������,��ôʹ��ԭʼģʽ(Raw Mode)��ʽ��ͨѶ,���÷�ʽ����
  newtio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  //c_lflag:����ģʽ
  newtio.c_oflag  &= ~OPOST;   //Output,c_oflag:Output options
  
  //���õȴ�ʱ�����С�����ַ�
  newtio.c_cc[VTIME]  = 0;  //0.1s,VTIME:Time to wait for data (1/10 seconds)
  newtio.c_cc[VMIN] = 1;   //VMIN:Minimum number of characters to read
  //������ڻ���,����δ�����ַ�
  tcflush(fd, TCIFLUSH);   //Update the options and do it NOW
  //����������
  if((tcsetattr(fd, TCSANOW, &newtio))!=0) 
  { 
    perror("Set Serial Failure!\n"); 
    return -1; 
  } 
  printf("Set Serial Success!\n"); 
  return 1; 
}

/*******************************************************************************
*�������� : int UartInit(ComBuf_t *p_combuf, int ttyid)
*��    �� : UART�����ú���
*������� : ComBuf_t *p_combuf:��Ӧ�豸����; int ttyid:UART�ں�
*������� : �ɹ�p_combuf->Fd;ʧ�ܷ���-1
*******************************************************************************/
int UartInit(ComBuf_t *p_combuf, int ttyid)
{
  p_combuf->Fd = -1;
  p_combuf->Timer = 0;
  p_combuf->Status = 0;
  p_combuf->RecvLen = 0;
  memset(p_combuf->Buf, 0, COMBUF_SIZE);
  
  //��������
  p_combuf->Fd = OpenCommPort(p_combuf->Fd, ttyid);
  if (p_combuf->Fd < 0)  //���豸ʧ��
    return -1;
  SetCommState(p_combuf->Fd, 115200, 8, 'N', 1); //���ô����豸
  return p_combuf->Fd;
}

// �ⲿ����omc��rs485ת�����ݻ������ṹ
extern swap_t omc_rs485_swap;

// �ⲿ����rs485��omcת�����ݻ������ṹ
extern swap_t rs485_omc_swap;
/*******************************************************************************
*�������� : int UartReceiveData(ComBuf_t *pcombuf, int waittime)
*��    �� : ���յ�����ת�浽pcombuf.Buf���ݻ�������,������,��ʱʱ��waittime
*������� : ComBuf_t *pcombuf:��Ӧ�豸����;int waittime:��ʱʱ��ms
*������� : ���յ����ݳ��Ȼ�����ʶ
*******************************************************************************/
int UartReceiveData(ComBuf_t *pcombuf, int waittime)
{
	if(0 != rs485_omc_swap.len){
		memcpy((char *)&pcombuf->Buf[0], (char *)rs485_omc_swap.buf, rs485_omc_swap.len);
		pcombuf->RecvLen = rs485_omc_swap.len;
		rs485_omc_swap.len = 0;
		return pcombuf->RecvLen;
	}
	return 0;
	/*
int res, rcsum;
fd_set readfs;
struct timeval tv;

_RERECVFLAG:
  tv.tv_sec = waittime/1000;//SECOND
  tv.tv_usec = (waittime%1000)*1000;//USECOND
  FD_ZERO(&readfs);
  FD_SET(pcombuf->Fd, &readfs);

  res = select(pcombuf->Fd + 1, &readfs, NULL, NULL, &tv);
  if (res > 0)
  {
    rcsum = read(pcombuf->Fd, &pcombuf->Buf[pcombuf->RecvLen], (COMBUF_SIZE - pcombuf->RecvLen));
    if (rcsum > 0)
    {
      pcombuf->RecvLen = pcombuf->RecvLen + rcsum;
      waittime = 10;
      goto _RERECVFLAG;//���½��յȴ�����
    }
    else if (rcsum < 0)
    {
      perror("UartReceiveData:read() error!");
      return -1;
    }
  }
  else if (res < 0)
  {
    perror("UartReceiveData:select() error!");
    return -1;
  }
  return pcombuf->RecvLen;
  */
}

/*******************************************************************************
*�������� : int UartSendData(int fd, char *sbuf, int len)
*��    �� : ����fd ��������
*������� : ����fd,sbuf,����
*������� : �������ɹ�����1,���򷵻�-1
*******************************************************************************/
int UartSendData(int fd, char *sbuf, int len)
{
//int sr;

  ComDataHexDis(sbuf, len);
	if(0 == omc_rs485_swap.len){
		memcpy((char *)omc_rs485_swap.buf, sbuf, len);
		omc_rs485_swap.len = len;
	}else{
		printf("omc_rs485_swap error!!!\r\n");
	}
	return 1;
  /*
  sr = write(fd, sbuf, len);
  if(sr == -1)
  {
    printf("Write sbuf error!\r\n");
    return -1;
  }
  return 1;
  */
}

/*******************************************************************************
*�������� : int UartSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf)
*��    �� : ����Э���,��p_packbuf��ָ�Ĵ��������ݷ��͸�pcombuf��ָ�豸
*������� : ComBuf_t *pcombuf:��Ӧ�豸����;APPack_t *p_packbuf:��������Ӧ���ݰ��ṹָ��
*������� : �ɹ�����1,���򷵻�-1
*******************************************************************************/
int UartSendPack(ComBuf_t *pcombuf, APPack_t *p_packbuf)
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
	  DEBUGOUT("Fd:%d.UartSendData:%d.\r\n", pcombuf->Fd, sdsum);
		UartSendData(pcombuf->Fd, sdbuf, sdsum);
   	ComDataWriteLog(sdbuf, sdsum);
   	return 1;
	}
	else
	  return -1;
}

/*********************************End Of File*************************************/
