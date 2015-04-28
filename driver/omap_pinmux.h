/*******************************************************************************
********************************************************************************
* �ļ�����:  omap_pinmux.h
* ��������:  ��omap�����ܽŹ��ܳ�ʼ���Ķ��塣��Ҫ���������塣
*						һ.XXX_XXX_PINMUXREG Ӧ��XXX_XXX���ܵĹܽŶ�Ӧ��pinmux�Ĵ�������0-19��
*						��.XXX_XXX_PINMUX �ùܽ���pingmux�Ĵ����е�λ�á�32λ�Ĵ�����0-7��
*						��.PINMUX_CONFIG_XXX �ùܽű����ó�XXX���ܡ�
* ʹ��˵��: ����Ҫ���ùܽŵ��������ݶ����ڸ��ļ��С�����PINMUX ��MUX �궨��
* �ļ�����:	H4
* ��д����: ��2012/06/18��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#ifndef _OMAP_PINMUX_H
#define _OMAP_PINMUX_H

#define PINMUX_PHY_ADDR_BASE 0x01C14000
#define PINMUX_ADDR_OFFSET 0x120

/*ѡ������һ��pinmux�Ĵ���������*/
#define TMP_SCL_REG 17
#define TMP_SDA_REG 17
#define KSZ8873_MDC_PINMUXREG 4
#define KSZ8873_MDIO_PINMUXREG  4
#define SPI_CLK_PINMUXREG 5
#define SPI_MISO_PINMUXREG 5
#define SPI_MOSI_PINMUXREG 5
#define SPI_LE_PINMUXREG 5
#define SPI_CS0_PINMUXREG 5
#define SPI_CS1_PINMUXREG 5
#define OMAP_I2C_SCL_PINMUXREG 4
#define OMAP_I2C_SDA_PINMUXREG 4
#define RS485_RX_PINMUXREG 4
#define RS485_TX_PINMUXREG 4
#define RS485_LE_PINMUXREG 0
#define EPLD_INT1_PINMUXREG 0
#define EPLD_INT2_PINMUXREG 0
#define KSZ8873_INT_PINMUXREG 1
#define FPGA_INT_PINMUXREG 0
#define EMIFA_CLK_PINMUXREG 6


/*pinmux�мĴ����ĵڼ���������*/
#define TMP_SCL_PIN  3
#define TMP_SDA_PIN  4
#define KSZ8873_MDC_PINMUX 0
#define KSZ8873_MDIO_PINMUX 1
#define SPI_CLK_PINMUX 2
#define SPI_MISO_PINMUX 4
#define SPI_MOSI_PINMUX 5
#define SPI_LE_PINMUX 3
#define SPI_CS0_PINMUX 1
#define SPI_CS1_PINMUX 0
#define OMAP_I2C_SCL_PINMUX 2
#define OMAP_I2C_SDA_PINMUX 3
#define RS485_RX_PINMUX 4
#define RS485_TX_PINMUX 5
#define RS485_LE_PINMUX 7
#define EPLD_INT1_PINMUX 3
#define EPLD_INT2_PINMUX 6
#define KSZ8873_INT_PINMUX 0
#define FPGA_INT_PINMUX 5
#define EMIFA_CLK_PINMUX 0
  

/*pinmux�����õ�ֵ*/
#define PINMUX_CONFIG_GPIO  0x00000008
#define PINMUX_CONFIG_MD_GPIO  0x00000004
#define PINMUX_CONFIG_UART  0x00000002
#define PINMUX_CONFIG_EMIFA 0x00000001


typedef struct {
   volatile unsigned int pinmux[20];
}pinmux_reg;


#define MUX(regindex,leftindex,value) pinmux_t->pinmux[regindex]=(((pinmux_t->pinmux[regindex])&(~(0x0000000f<<(leftindex*4))))|(value<<(leftindex*4)))


extern pinmux_reg *pinmux_t;

#endif 
