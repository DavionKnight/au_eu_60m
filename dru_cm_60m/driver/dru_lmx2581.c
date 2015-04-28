/*******************************************************************************
********************************************************************************
* �ļ�����:  dru_lmx2581.c
* ��������:  ��lmx2581�����ò�����
*						
* ʹ��˵��: 
* �ļ�����:	H4
* ��д����: ��2012/06/18��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include "../../driver/dru_spi.h"
#include "../../driver/omap_epld.h"
#include "dru_lmx2581.h"
#include "common.h"
#include "status.h"
#include "../../driver/dru_reg.h"

#define LMX2581_XREFP 6144
#define LMX2581_DEN 0X2ee000
//=====================================================
//fpga�Ĵ�����ַ����
#define FPGA_LM2581_CLK 	0x002
#define FPGA_LM2581_LE		0x003
#define FPGA_LM2581_OUT	0x004
#define FPGA_LM2581_IN		0x005
#define FPGA_LM2581_CHIPEN 0X006


void lm2581_spi_channel(unsigned int data)
{
	 *DRU_REGADDR(epld_base_addr, FPGA_LM2581_CHIPEN)=data;
}

/*******************************************************************************
* ��������: lm2581_spi_clk
* ��    ��: ���ڼĴ���fpga spiͨ��clk�Ĵ������ơ�
* ��    ��:
* ��������         ����                ����
* level      unsigned char         Ϊfpga spi��clk�������ƽ
* clkChoice  unsigned char				clk�Ĵ���ѡ��
* ����ֵ:
* ��
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/06/15  V1.0     H4     ��       ��
*******************************************************************************/
void lm2581_spi_clk(unsigned char level)                                              
{                                                                             
      
  	  *DRU_REGADDR(epld_base_addr, FPGA_LM2581_CLK)=level;
                                
}                                                                             
/*******************************************************************************
* ��������: lm2581_spi_le
* ��    ��: ���ڼĴ���fpga spiͨ��le�Ĵ������ơ�
* ��    ��:
* ��������         ����                ����
* level      unsigned char         Ϊfpga spi��le�������ƽ
* leChoice   unsigned char				le�Ĵ���ѡ��
* ����ֵ:
* ��
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/06/15  V1.0     H4     ��       ��
*******************************************************************************/
void lm2581_spi_le(unsigned char level)                                              
{                                                                             
  	  	*DRU_REGADDR(epld_base_addr, FPGA_LM2581_LE)=level;
  	                                                       
}           
/*******************************************************************************
* ��������: lm2581_spi_out
* ��    ��: ���ڼĴ���fpga spiͨ��MOSI�Ĵ������ơ�
* ��    ��:
* ��������         ����                ����
* level      unsigned char         Ϊfpga spi��MOSI�������ƽ
* ����ֵ:
* ��
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/06/15  V1.0     H4     ��       ��
*******************************************************************************/
void lm2581_spi_out(unsigned char level)                                              
{                                                                             
 	
 	 	*DRU_REGADDR(epld_base_addr, FPGA_LM2581_OUT)=level;
                                                  
}           
/*******************************************************************************
* ��������: lm2581_spi_in
* ��    ��: ���ڼĴ���fpga spiͨ��MISO�Ĵ������ơ�
* ��    ��:
* ��������         ����                ����
* level      unsigned short         Ϊfpga spi��MISO�������ƽ
* ����ֵ:
* ��
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/06/15  V1.0     H4     ��       ��
*******************************************************************************/
void lm2581_spi_in(unsigned short *level)                                              
{                                                                             
    //READ_FPGA(FPGA_SPI_MISO_REG,*level);
    *level=*DRU_REGADDR(epld_base_addr, FPGA_LM2581_IN);
    return;                                                                   
}
/*******************************************************************************
* ��������: lm2581_spi_write
* ��    ��: ���ڼĴ���epld spiͨ��д���ݡ���д��λ ������д��
* ��    ��:
* ��������         ����                ����
* data      unsigned int        ��Ҫд�������
* length    unsigned char				����λ��
* ����ֵ:
* ��
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2012/06/15  V1.0     H4     ��       ��
*******************************************************************************/
int lm2581_spi_write(unsigned int data)
{
	unsigned char i;
	lm2581_spi_le(1);
	spi_delay(1);
	lm2581_spi_clk(0);
	lm2581_spi_le(0);
	for(i=0;i<32;i++){
		if((data&(1<<(32-1-i)))!=0)
		lm2581_spi_out(1);
		else
		lm2581_spi_out(0);
		lm2581_spi_clk(0);
		spi_delay(1);
		lm2581_spi_clk(1);
		spi_delay(1);
		lm2581_spi_clk(0);
	}
	lm2581_spi_le(1);
	spi_delay(1);
	return 0;
}
int dru_lmx2581_config(unsigned int freq,unsigned int ch)
{
	unsigned int is_mux=0;
	unsigned int r0,r1,r2,r3,r5;
	unsigned long long fra_tmp, fra_tmp1;
	unsigned int pll_n,pll_num,pll_num1,pll_num2;
	unsigned int r=2;
	unsigned int rf_div;
	unsigned int rf_int;
	rf_div=0;
	rf_int=freq;
	for(rf_div=0;rf_div<=18;rf_div++){
		if(is_mux==0){
			if((rf_int>=188000)&&(rf_int<=376000)){
				break;
			}
			is_mux=1;
		}
		if(is_mux==1){
			rf_int=(rf_div*2+2)*freq;
			if((rf_int>=188000)&&(rf_int<=376000)){
				break;
			}
		}
	}
	if(rf_div>18){
		printf("lmx2581_error:input fre too less than 50mHzr\n");
		exit(0);
	}
	if(rf_int>376000){
		printf("lmx2581_error:input fre more than 3760mHz\r\n");
		exit(0);
	}
	pll_n=rf_int/(LMX2581_XREFP/r);
	fra_tmp1=rf_int%(LMX2581_XREFP/r);
	fra_tmp=fra_tmp1*LMX2581_DEN;
	pll_num=fra_tmp/(LMX2581_XREFP/r);
	pll_num1=pll_num&0xfff;
	pll_num2=pll_num&0x3ff000;
	if(is_mux==1){
		r5=0x0010A805;
	}else{
		r5=0x00108005;
	}
	r3=0x20000FE3|(rf_div<<18);
	r2=0x0c000002|(LMX2581_DEN<<4);
	r1=0x70002021|(pll_num2<<3);
	r0=0x40000000|(pll_n<<16)|(pll_num1<<4);
	lm2581_spi_channel(ch);
	lm2581_spi_write(0x00108015);
	spi_delay(10000);  
	lm2581_spi_write(0x021FE80F);
	lm2581_spi_write(0x4082C10D);
	lm2581_spi_write(0x210050CA);
	lm2581_spi_write(0x03C7C039);
	lm2581_spi_write(0x207DDBF8);
	lm2581_spi_write(0x00082317);
	lm2581_spi_write(0x000004C6);
	lm2581_spi_write(r5);
	lm2581_spi_write(0x00000004);
  	lm2581_spi_write(r3);
	lm2581_spi_write(r2);
	lm2581_spi_write(r1);
	lm2581_spi_write(r0);
	spi_delay(50000);  
	lm2581_spi_write(r0);
	return 0;
}
void dru_lm2581_init(void)
{
	lm2581_spi_channel(1);
	lm2581_spi_write(0x00108015);
	spi_delay(10000);  
	lm2581_spi_write(0x021FE80F);
	lm2581_spi_write(0x4082C10D);
	lm2581_spi_write(0x210050CA);
	lm2581_spi_write(0x03C7C039);
	lm2581_spi_write(0x207DDBF8);
	lm2581_spi_write(0x00082317);
	lm2581_spi_write(0x000004C6);
	lm2581_spi_write(0x0010A805);
	
	lm2581_spi_write(0x00000004);
	lm2581_spi_write(0x2005E3c3);
	lm2581_spi_write(0x0FD09002);
	lm2581_spi_write(0xC53A2051);
	lm2581_spi_write(0x41062750);
	spi_delay(50000);  
	lm2581_spi_write(0x41062750);

	lm2581_spi_channel(2);
	lm2581_spi_write(0x00108015);
	spi_delay(10000);  
	lm2581_spi_write(0x021FE80F);
	lm2581_spi_write(0x4082C10D);
	lm2581_spi_write(0x210050CA);
	lm2581_spi_write(0x03C7C039);
	lm2581_spi_write(0x207DDBF8);
	lm2581_spi_write(0x00082317);
	lm2581_spi_write(0x000004C6);
	lm2581_spi_write(0x00108005);
	
	lm2581_spi_write(0x00000004);
	lm2581_spi_write(0x2001E3C3);
	lm2581_spi_write(0x2C001902);
	lm2581_spi_write(0xC4002021);
	lm2581_spi_write(0x401F0860);
	spi_delay(50000);  
	lm2581_spi_write(0x401F0860);
}

int dru_lmx2581_config_change(int argc, char * argv[])
{
	unsigned int para1;

	msg_tmp.mtype = MSG_FUN_RESULT;	
	if(argc != 2){
		printf("input para cnt is not 2.\r\n");
		sprintf(msg_tmp.mtext, "input para cnt is not 2.\r\n");
		msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
		return 1;
	}
	para1 = strtol(argv[1], NULL, 10);
	printf("para1=%d. \r\n", para1);
	dru_lmx2581_config((unsigned int)para1,1);
	sprintf(msg_tmp.mtext, "dru_lmx2581_wcdma_config=%d\r\n", para1);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}


