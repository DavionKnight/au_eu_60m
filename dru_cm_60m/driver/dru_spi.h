/*******************************************************************************
********************************************************************************
* �ļ�����:  dru_spi.h
* ��������:  spiͷ�ļ�����epld��fpga�����Ĵ�����spi�ܽ���ӳ��
* ʹ��˵��: 	�����й�spi�ĺ궨�嶼������ļ��С�������gpioģ�⻹�ǼĴ���ģ��
*						Ҳ�����Ǹ��ĸ�����������
* �ļ�����:	H4
* ��д����: ��2012/06/18��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#ifndef _DRU_SPI_H
#define _DRU_SPI_H

#define SPI_TIME_DELAY 1
#define SPI_H 1
#define SPI_L 0

#define EMFI_CS_CONFIG 0X0422221D





void omap_pinmux_init(void);
void emif_epld_spi_channel(unsigned short channel);
int emif_epld_spi_write(unsigned int data,unsigned char length);
int emif_epld_adc_spi_write(unsigned int data,unsigned char length);
int emif_fpga_spi_write(unsigned int data,unsigned char length,unsigned char choice);
int emif_fpga_spi_read(unsigned int *data,unsigned char length,unsigned char choice);
void omap_emif_init(void);
void emif_fpga_ddc_carrier_f8_write(unsigned char addr,unsigned short data);
void emif_fpga_ddc_carrier_m8_write(unsigned char addr,unsigned short data);
void emif_fpga_duc_carrier_f8_write(unsigned char addr,unsigned short data);
void emif_fpga_duc_carrier_m8_write(unsigned char addr,unsigned short data);
unsigned short emif_fpga_carrier_read(unsigned char addr);
#endif
