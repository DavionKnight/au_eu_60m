/********************  COPYRIGHT(C) ***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: druhwinfo.h
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: DRU����ƽ̨Ӳ��������Ϣͷ�ļ�
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
#ifndef _DRUHWINFO_H
#define _DRUHWINFO_H

#include "usertype.h"
//CPLD��ַ����
#define CPLDVERSION_ADDR      0x0000  //bit15-0:CPLD�汾��
#define LOSTATUS_ADDR         0x0012  //����״̬
#define PLLSTATUS_ADDR        0x0017  //PLLʱ��״̬
#define OPTICDECT_ADDR        0x0020  //��ģ���ڲ���λ���
//FPGA��ַ����
#define LTE1DATAPOWER_ADDR    0x004A  //LTE1 ���ֹ��� uint2
#define LTE2DATAPOWER_ADDR    0x004B  //LTE2 ���ֹ��� uint2
#define G3DATAPOWER_ADDR      0x004C  //3G ���ֹ��� uint2
#define GSMDATAPOWER_ADDR     0x004D  //GSM ���ֹ��� uint2
#define LTE1VGA_ADDR          0x0046  //LTE1 VGA uint1
#define LTE2VGA_ADDR          0x0047  //LTE2 VGA uint1
#define G3VGA_ADDR            0x0048  //3G VGA uint1
#define GSMVGA_ADDR           0x0049  //GSM VGA uint1
#define LTE1DATAGAIN_ADDR     0x0040  //LTE1 �������� uint2
#define LTE2DATAGAIN_ADDR     0x0041  //LTE2 �������� uint2
#define G3DATAGAIN_ADDR       0x0042  //3G �������� uint2
#define GSMDATAGAIN_ADDR      0x0043  //GSM �������� uint2
#define GSMCARRIERMASK_ADDR   0x0050  //GSM �ز����μĴ��� uint2
#define TESTMODELSWITCH_ADDR  0x003F  //����ģʽ������ģʽ�л� uint1
#define INTERTESTEN_ADDR      0x003E  //�ڲ�����Դʹ�� uint1
#define INTEFREQRSRC_ADDR     0x003C  //�ڲ�ԴƵ������ uint2
#define AGCREFERENCE_ADDR     0x0045  //AGC �ο�ֵ�趨 uint2
#define AGCSTEP_ADDR          0x004E  //AGC �������� uint1
#define AGCEN_ADDR            0x004F  //AGC ʹ�ܼĴ��� 1 bit �����ش���VGA��������(д010)

#define FPGATOPVERSION_ADDR   0x0000  //bit15-0:FPGA���汾��,MER
#define FPGAVERMARK_ADDR      0x0001  //bit15-0:�汾��ʾ˵���Ĵ���,MER
#define INTMASKREG_ADDR       0x0002  //�ж����μĴ�������������Ӧ�ж��źŲ���,MER
#define INTENREG_ADDR         0x0005  //�ж�ʹ�ܼĴ���,MER
#define INTSOURCEREG_ADDR     0x0008  //�ж�Դ�Ĵ���,MER
#define INTCLEARREG_ADDR      0x000B  //�ж�����Ĵ���,MER
#define TX2RXTIMEOPT_ADDR     0x000E  //���ʱ�Ӳ���ֵ,�������ڹ���ʱ�Ӳ���ֵ,MER
#define UL2KMADJUST_ADDR      0x0016  //����2KM��������Ĵ���,MER
#define DL2KMADJUST_ADDR      0x0017  //����2KM��������Ĵ���,MER
#define SPDCFGREG_ADDR        0x0018  //�̶�ֵ5Gģʽ,MER
#define RSTIDREGH_ADDR        0x0019  //��λ�����ָ��ֽ�,MER
#define RSTIDREGL_ADDR        0x001A  //��λ�����ֵ��ֽ�,MER
#define OPTIQENREG_ADDR       0x001B  //�豸IQ����ʹ��,MER
#define OPTSYNCREG_ADDR       0x0100  //ͬ����ʶ,MER
#define DEVIDREGH_ADDR        0x0110  //�豸������Ÿ��ֽ�,MER
#define DEVIDREGL_ADDR        0x0111  //�豸������ŵ��ֽ�,MER
#define DEVIDLEVEL_ADDR       0x010d  //�豸��������,MER
#define MAINIDREG_ADDR        0x0020  //�������˵�����Ԫ���,MER
#define ULOPTSELREG_ADDR      0x0021  //�������ѡ��,MER
#define TDS_TIMESLOT_ADDR     0x005d  //TD-SCDMAʱ϶���üĴ���
#define TDLTE_TIMESLOT_ADDR   0x0058  //TD-LTEʱ϶���üĴ���
#define TDS_SYN_THR_ADDR      0x0055  //TD-SCDMAͬ���������޼Ĵ���
#define TDLTE_SYN_THR_ADDR    0x080  //TD-LTEͬ���������޼Ĵ���
#define	TD_DW_POWER_ADDR	  0x05F  // TD ��Ƶ����
#define TD_TS0_POWER_ADDR	  0x060  // TD TS0 ʱ϶����
#define TD_TS1_POWER_ADDR	  0x061  // TD TS0 ʱ϶����
#define TD_TS2_POWER_ADDR	  0x062  // TD TS0 ʱ϶����
#define TD_TS3_POWER_ADDR	  0x063  // TD TS0 ʱ϶����
#define TD_TS4_POWER_ADDR	  0x064  // TD TS0 ʱ϶����
#define TD_TS5_POWER_ADDR	  0x065  // TD TS0 ʱ϶����
#define TD_TS6_POWER_ADDR	  0x066  // TD TS0 ʱ϶����
#define	LTE_DW_POWER_ADDR	  0x067  // LTE ��Ƶ����
#define	LTE_TS0_POWER_ADDR	  0x068  // LTE TS0 ʱ϶����
#define	LTE_TS2_POWER_ADDR	  0x069  // LTE TS0 ʱ϶����
#define	LTE_TS3_POWER_ADDR	  0x06A  // LTE TS0 ʱ϶����
#define	LTE_TS4_POWER_ADDR	  0x06B  // LTE TS0 ʱ϶����
#define TD_ASYNC_ADDR         0x06E  // TD-SCDMAʧ���澯
#define LTE_ASYNC_ADDR        0x06F  // LTEʧ���澯
#define BINDWIDTH_MODEL_ADDR  0x073  // 2G ���ѡƵģʽѡ��
#define LTE_AUTO_CONFIG_ADDR  0x074  // LTE �Զ�����
#define TD_FIRTRP_FINE_REG	  0x082	 // TD��һת����΢��
#define TD_SECTRP_FINE_REG	  0x083	 // TD�ڶ�ת����΢��
#define LTE_FIRTRP_FINE_REG	  0x056	 // LTE��һת����΢��
#define LTE_SECTRP_FINE_REG	  0x057  // LTE�ڶ�ת����΢��
#define LTE_CHNCHS_REG		  0x076	 // LTE�Ľ���ͨ��ѡ��

#define MACSIZE       6 //MAC��ַ����
#define INTREG_SIZE   3 //�ж�Դ�Ĵ�������
#define OPT_SIZE      8 //�������

#define VGAMAXSETVAL  63//VGA�������ֵ
#define DATAPOWERBITS 0x1FF//���ֹ���

#pragma pack(1)
typedef struct HWInfo
{
  char DevType;         //�豸����
  int  DeviceNo;        //�ͻ����豸���,�����ƶ�Э��
	char Mac[MACSIZE];    //MAC��ַ
	INT32U IPAddr;        //�ͻ��˵�ַ
  //FPGA�汾����ʶ˵��
  INT16U FpgaTopVersion;  //bit15-0:FPGA���汾��,MER
  INT16U FpgaVerMark;     //bit15-0:�汾��ʾ˵���Ĵ���,MER
  //�жϿ���
  INT16U INTMaskReg[INTREG_SIZE];   //�ж����μĴ�������������Ӧ�ж��źŲ���,MER
  INT16U INTEnReg[INTREG_SIZE];     //�ж�ʹ�ܼĴ���,MER
  INT16U INTSourceReg[INTREG_SIZE]; //�ж�Դ�Ĵ���,MER
  INT16U INTClearReg[INTREG_SIZE];  //�ж�����Ĵ���,MER
  //ʱ�Ӳ���
  INT16U Tx2RxTimeOpt[8]; //���ʱ�Ӳ���ֵ,�������ڹ���ʱ�Ӳ���ֵ,MER
  INT16U UL2kmAdjust;     //����2KM��������Ĵ���,MER
  INT16U DL2kmAdjust;     //����2KM��������Ĵ���,MER
  //����������
  INT8U SpdCfgReg;        //�̶�ֵ5Gģʽ,MER
  INT32U RstIdReg;        //��λ������,MER
  INT8U OptIQEnReg;       //�豸IQ����ʹ��,MER
  INT8U OptSyncReg;       //ͬ����ʶ,MER
  INT32U DevIdReg;        //�豸�������,MER
  INT16U DevIdLevel;      //�豸��������,MER
  INT16U MainIdReg;       //�������˵�����Ԫ���,MER
  //���gtx
  INT8U ULOptSelReg;      //�������ѡ��,MER
}HWInfo_t, *HWInfo_tp;

#endif//_DRUHWINFO_H

/*********************************End Of File*************************************/
