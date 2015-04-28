#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/serial.h>
/* opt : ����ͨ�Ų�����ָ��
 * data_bits : ��Ч����λ 7��8
 * stop_bit : ֹͣλ 1��2
 * parity : У��λ 0��У�� 1��У�� 2żУ��
 * */
int set_parity(struct termios *opt, int data_bits, int stop_bit, int parity)
{
    int ret = 0;

    // �޸Ŀ���ģʽ����֤���򲻻�ռ�ô���
    opt->c_cflag |= CLOCAL;
    // �޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������
    opt->c_cflag |= CREAD;

    opt->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opt->c_oflag &= ~OPOST; 
    // ������������
    //opt->c_cflag |= CRTSCTS;  // ʹ��Ӳ������
    opt->c_cflag &= ~CRTSCTS;  // ��ʹ��������
    //����� XON/XOFF ������
    //opt->c_iflag &= ~IXOFF;//������
    //����� XON/XOFF ������
    opt->c_iflag &= ~(ICRNL|IXON);//������
	//opt->c_iflag |= IXANY;
    // ��������λ
    opt->c_cflag &= ~CSIZE;
    switch(data_bits){
        case 7:  // 7λ����λ
        opt->c_cflag |= CS7;
        break;
    case 8: // 8λ����λ
		opt->c_cflag |= CS8;
		break;
	default:
		printf("data_bits is error!\n");
		opt->c_cflag |= CS8;
	}
	// ����ֹͣλ
	if(stop_bit == 2){// 2λֹͣλ
		opt->c_cflag |= CSTOPB;
	}else{// 1λֹͣλ
		opt->c_cflag &= ~CSTOPB;
	}
	// ������żУ��λ
	switch(parity){
		case 0: // ��У��
			opt->c_cflag &= ~PARENB;
			break;
		case 1: //��У��
			opt->c_iflag |= INPCK;
			opt->c_cflag |= PARENB;
			opt->c_cflag |= PARODD;
			break;
		case 2: // żУ��
			opt->c_iflag |= INPCK;
			opt->c_cflag |= PARENB;
			opt->c_cflag &= ~PARODD;
			break;
		default:
			printf("parity is error!\n");
			opt->c_cflag &= ~PARENB;
	}

	// �޸Ŀ����ַ�����ȡ�ַ������ٸ���Ϊ1
	opt->c_cc[VMIN] = 0;
	// �޸Ŀ����ַ�����ȡ��һ���ַ��ȴ�1����1��10��s
	opt->c_cc[VTIME] =1; 

    return ret;
}
 
int get_baud_rate(unsigned long int baud_rate)
{
	switch(baud_rate){
		case 0:
			return B0;
		case 1200:
			return B1200;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		case 230400:
			return B230400;
		default:
			return -1;
	}	
}
// ��ʼ������ͨ��
#define BAUD_RATE (B2400)
#define STOP_BIT (1)
#define DATA_BITS (8)
// 2 żУ�� 1 ��У�� 0 ��У��
#define PARITY (2)
int init_uart(char * s_dev, unsigned long int baud, int stopbit, int databit, int parity)
{
    struct termios opt;
//	struct serial_rs485 rs485conf;
    int ret = 0;
    int tmp = 0;
    int fd = 0;
//	int flags = 0;

    // �򿪴���
    fd = open(s_dev, O_RDWR|O_NOCTTY|O_NDELAY|O_NONBLOCK);
    if(fd == -1)
	{
        printf("open %s error!\n",s_dev);
        ret = -1;
        goto ERRHANDLE;
    }
	else
	{
        printf("open %s ok!%d\n",s_dev, fd);
        ret = fd;
    } 
	/*
	flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flags)<0){
		printf("fcntl failed!\r\n");
	}else{
		printf("fcntl=%d\r\n", fcntl(fd, F_SETFL,flags));
	}
*/
#ifdef RS485_CONFIG
	rs485conf.flags |= SER_RS485_ENABLED;
	rs485conf.flags |= SER_RS485_RTS_ON_SEND;
	rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);
	rs485conf.delay_rts_before_send = 5;
	if(ioctl(fd, TIOCSRS485, &rs485conf) < 0)
	{
		printf("rs485 fcntl faild!\r\n");
		ret = -1;
	}
#endif
    // ���ô���ͨ�Ų���
    if(tcgetattr(fd, &opt) != 0)
	{
		printf("setup serial!\r\n");
		return -1;
	}
	//bzero(&opt, sizeof(opt));
#if 0
	printf("baud_rate=%dbps\n", get_baud_rate(baud));
	printf("B9600=%dbps\n", B115200);
	printf("databit=%d\n", databit);
	printf("stopbit=%d\n", stopbit);
	printf("parity=%d\n", parity);
#endif
    cfsetispeed(&opt, get_baud_rate(baud));		//ָ���������������
    cfsetospeed(&opt, get_baud_rate(baud));
    set_parity(&opt, databit, stopbit, parity);	//8E1 8λ���� 1ֹͣλ żУ��
    tcflush(fd, TCIOFLUSH);
    tmp =  tcsetattr(fd, TCSANOW, &opt);
    if(tmp < 0)
	{
        ret = -1;
        printf("tcsetattr error!\n");
        goto ERRHANDLE;
    }

ERRHANDLE:
    return ret;   
}

void my_printf(char * buf, int len)
{
	int i = 0;
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
	for(i = 0; i < len; i++){
		if(i%16 == 0){
			printf("\n");
		}
		printf("%02x ", buf[i]&0x0ff);
	}
	printf("\n##########################################\n");
}

int send_data(int fd, char *data, int data_len)
{
    int len = 0;

	if(data_len == 0)
	{
		return 0;
	}
	
    len = write(fd, data, data_len);
	
    if(len == data_len)
	{
        printf("send data cnt is %d\n", len);
        //my_printf(data, len);
    }
	else
	{
        tcflush(fd, TCOFLUSH);
        len = -1;
		perror("send data error\n");
        goto ERRHANDLE;
    }
ERRHANDLE:
    return len;
}
int recv_data(int fd, char *data, int data_len, int s, int us)
{
    int fs_sel = 0;
    fd_set fs_read;
    struct timeval time;
	int cnt = 0;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    time.tv_sec = s;
    time.tv_usec = us;
    // ʹ��selectʵ�ִ��ڵĶ�·ͨ��
    fs_sel = select(fd+1, &fs_read, NULL, NULL, &time);
    if(fs_sel){
        if(FD_ISSET(fd, &fs_read)){
            cnt = read(fd, data, data_len);
			//printf("recv data cnt is %d\n", cnt);
			//my_printf(data, cnt);
		}
    }
    return cnt;
}
void delay(int sec, int usec)
{
	int ret = 0;
	struct timeval timeout;
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
	ret = select(0, NULL, NULL, NULL, &timeout);
	if(ret == 0){
		return;
	}else{
		printf("select err!\n");
	}
}
