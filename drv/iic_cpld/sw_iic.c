#define _SW_IIC_C_

#define uint8 unsigned char

#include <linux/kernel.h>
#include "sw_iic.h"

#define WRITE_LOG(format,args...) printk(format,##args)

//����ģ���ⲿ����

//ģ���ڶ����ȫ�ֱ���

//ģ���ڲ�����


//�������ƣ�SDA�������������
//�����������
//�����������
//����ֵ����
void _tx_sda(uint8 a) 	
{
	;
}
void tx_sda(uint8 a) __attribute__((weak, alias("_tx_sda")));

//�������ƣ���ȡSDA�����ߵ�ƽ
//�����������
//�����������
//����ֵ��SDA��ƽ 0=�͵�ƽ 1=�ߵ�ƽ
uint8 _rx_sda(void)
{
	return 0;
}
uint8 rx_sda(void) __attribute__((weak, alias("_rx_sda")));

//�������ƣ�SCL�������������
//�����������
//�����������
//����ֵ����
void _tx_scl(uint8 a) 	
{
	;
}
void tx_scl(uint8 a) __attribute__((weak, alias("_tx_scl")));

//�������ƣ���ȡSCL�����ߵ�ƽ
//�����������
//�����������
//����ֵ��SCL��ƽ 0=�͵�ƽ 1=�ߵ�ƽ
uint8 _rx_scl(void)
{
	return 0;
}
uint8 rx_scl(void) __attribute__((weak, alias("_rx_scl")));
// iic����������־
unsigned char g_start = 0;
 
//�������ƣ�5us��ʱ����
//�����������
//�����������
//����ֵ����
void _delay5us(void)
{
	;
}
void delay5us(void) __attribute__((weak, alias("_delay5us")));

//�������ƣ�6us��ʱ����
//�����������
//�����������
//����ֵ����
void _delay6us(void)
{
	;
}
void delay6us(void) __attribute__((weak, alias("_delay6us")));

//�������ƣ����� start �ź�
//�����������
//�����������
//����ֵ����
uint8 iic_start(void)
{
	if(g_start == 1) // ��iic�Ѿ���ʼ���������¿�ʼ 
	{		
		rx_sda();		// �ͷ�SDA��SCL���� 	
		rx_scl();		
		delay5us(); 	
		if(rx_scl() == 0)	// ���豸�Ƿ��ӳ��͵�ƽʱ�� 	
		{			
			WRITE_LOG("IIC restart error!\r\n"); // ��ӳ�ʱ����
			return 0 ;		
		}
		delay5us(); // Repeated start setup time, minimum 4.7us 
	} 
	rx_sda();
	rx_scl();
	delay6us();
	if((rx_sda() == 0)||(rx_scl() == 0))	
	{
		WRITE_LOG("IIC start error!\r\n");	// �������æ�������� 	
		return 0;	
	} 
	delay5us();
	delay5us(); 	
	tx_sda(0);	
	delay5us(); 
	tx_scl(0);	
	g_start = 1;
	
	return 1;//added by zxb
}
//�������ƣ����� stop �ź�
//�����������
//�����������
//����ֵ����
void iic_stop(void)
{
	tx_sda(0);
	delay5us();
	rx_scl();
   	delay6us();
	if(rx_scl() == 0)	// ���豸�Ƿ��ӳ��͵�ƽʱ��
	{
		WRITE_LOG("IIC stop error!\r\n");
	}
	delay5us();
	tx_sda(1);
	delay5us();
	g_start = 0;
}
//�������ƣ�iic д1��bit����
//���������value=Ҫд���1bit����
//�����������
//����ֵ����
void iic_write_bit(uint8 value)
{
	delay5us();
	tx_sda(value);
	delay5us();
	rx_scl();
	delay6us();
	if(rx_scl() == 0)		// SCL �ø�
	{
		WRITE_LOG("IIC SCL error!\r\n");
	}
	delay5us();
	tx_scl(0);
}
//�������ƣ�iic ��1��bit����
//�����������
//�����������
//����ֵ����ȡ����1bit����
uint8 iic_read_bit(void)
{
	uint8 value = 0;
	
	tx_scl(0);
	delay5us();
	rx_sda();					// �ͷ�SDA������
	delay5us();
	rx_scl();
	delay6us();
	if(rx_scl() == 0)		// SCL �ø�
	{
		WRITE_LOG("IIC SCL error!\r\n");;
	}
	delay5us();
	value = rx_sda();
	delay5us();
	tx_scl(0);
	delay5us();
	return value;
}
//�������ƣ�iic ����1�ֽ�����
//���������dat=Ҫ���͵�1�ֽ�����
//�����������
//����ֵ��0=���յ�ACK���ɹ�  1=���յ�NOACK��ʧ��
uint8 iic_write_byte(uint8 dat)
{
	uint8 i = 0;
	uint8 value =0;
	
	for(i = 0; i < 8; i++)
	{
		iic_write_bit((dat>>(7-i))&0x1);
	}
	value = iic_read_bit();
	return (value);
}
//�������ƣ�iic ���Ͷ��ֽ�����
//���������buf=Ҫ���͵����ݻ�����ָ�룬 len=���ݳ���
//�����������
//����ֵ��0=���յ�ACK���ɹ�  1=���յ�NOACK��ʧ��
unsigned char iic_write_bytes(unsigned char * buf, unsigned char len)
{
	uint8 i = 0;
	uint8 value =0;
	
	for(i = 0; i < len; i++)
	{
		value = iic_write_byte(buf[i]);
		if(value == 0)
		{// д��ɹ�
			//WRITE_LOG("Send ADDR , Recv ACK!\n");
		}
		else if(value == 1)
		{// û���յ�ACKӦ��
			iic_stop();
			WRITE_LOG("Send ADDR , Recv NACK!\n");
			return 1;
		}
	}
	return (0);
}
//�������ƣ�iic ����1�ֽ�����
//���������ack=���յ�1�ֽ����ݺ����Ӧ ACK��NACK
//�����������
//����ֵ�����յ���1�ֽ�����
uint8 iic_read_byte(uint8 ack)
{
	unsigned char i = 0;
	unsigned char cc = 0;
	
	for(i = 0; i < 8; i++)
	{
		cc |= (iic_read_bit()<<(7-i));
	}
	iic_write_bit(ack);
	return cc;
}
//�������ƣ�iic ���ն��ֽ�����
//���������buf=���յ������ݻ�����, len=���յ������ݳ���
//�����������
//����ֵ��0=�ɹ�
uint8 iic_read_bytes(uint8 * buf, uint8 len)
{
	uint8 i = 0;
	
	for(i = 0; i < len-1; i++)
	{
		buf[i] = iic_read_byte(ACK);
	}
	buf[len-1] = iic_read_byte(NACK);
	return (0);
}

