#include "../../driver/omap_epld.h"
#include "../../driver/dru_spi.h"
#include "common.h"
#include "apue.h"
#include "commonfun.h"
#include "druhwinfo.h"
#include "../../sqlite/drudatabase.h"
#include "../../task/omccom/omccom.h"
#include "status.h"
#include "dru_ss112.h"

extern int g_OMCSetParaFlag;
extern int g_RuSetParaFlag;
extern int g_OmcBroadcastFlag;
extern DevicePara_t g_DevicePara;
extern int g_DevType;
extern time_t g_AlarmDectTime;//�澯��ѯ��ʱ
time_t m_query_time = 0;

extern int GetVGAVal(unsigned int id, int *pval);
extern int GetDataGainVal(unsigned int id, int *pval);
extern int dru_lmx2531_wcdma_config(unsigned int freq);
extern int dru_lmx2531_fdd_lte1_config(unsigned int freq);
extern int dru_lmx2531_fdd_lte2_config(unsigned int freq);

extern int get_max_delay(void);
extern int set_auto_delay_sw(int val);

extern unsigned short dru_fpga_write(unsigned int addr,unsigned short data);
extern unsigned short dru_fpga_read(unsigned int addr,unsigned short *data);
extern void actual_alarm(void);
extern void common_alarm(void);
extern int is_check_alarm(void);
int m_lte_num = 0;
/*
** �������ܣ���FPGA�ӿں���
** �������: addr=FPGA��ַ
** ���������pdata=��FPGA����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_read_fpga(unsigned int addr, unsigned short * pdata)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_fpga_read(addr, pdata);
	if(tmp == *pdata){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ�дFPGA�ӿں���
** �������: addr=FPGA��ַ data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_fpga(unsigned int addr, unsigned short data)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_fpga_write(addr, data);
	if(tmp == data){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ���EPLD�ӿں���
** �������: addr=EPLD��ַ
** ���������pdata=��EPLD����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_read_epld(unsigned int addr, unsigned short * pdata)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_epld_read(addr, pdata);
	if(tmp == *pdata){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ�дEPLD�ӿں���
** �������: addr=EPLD��ַ data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_epld(unsigned int addr, unsigned short data)
{
	unsigned short tmp;
	int ret;

	lock_sem(SEM_DRV);
	tmp = dru_epld_write(addr, data);
	if(tmp == data){
		ret = 0;
	}else{
		ret = -1;
	}
	unlock_sem(SEM_DRV);

	return ret;
}
/*
** �������ܣ�дDDC���ýӿں���
** �������: addr=�ŵ��� data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_ddc(unsigned char addr,unsigned short data)
{
	lock_sem(SEM_DRV);
	if (addr < 8)
	{
		emif_fpga_ddc_carrier_f8_write(addr,data);
	}
	else
	{
		emif_fpga_ddc_carrier_m8_write(addr-8,data);
	}
	unlock_sem(SEM_DRV);
	return 0;
}
/*
** �������ܣ�дDUC���ýӿں���
** �������: addr=�ŵ��� data=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
*/
int drv_write_duc(unsigned char addr,unsigned short data)
{
	lock_sem(SEM_DRV);
	if (addr < 8)
	{
		emif_fpga_duc_carrier_f8_write(addr,data);
	}
	else
	{
		emif_fpga_duc_carrier_m8_write(addr-8,data);
	}
	unlock_sem(SEM_DRV);
	return 0;
}
extern int init_alarm_en(void);

extern void delay_measure_start(void);
/*******************************************************************************
*�������� : void OMCDevParaDeal(void)
*��    �� : �豸�澯������,����3min���и澯�ϱ�
*������� : none
*������� : none
*******************************************************************************/ 
int no_alarm_flag = 0;
void OMCDevParaDeal(void)
{
int i, val, val2, val1, val3;
unsigned short para2, temp;
DevInfo_t devinfo;
		  float power = 0.0;
		  float f1 = 0.0;
		  float f2 = 0.0;
  //�в�������
  if (g_OMCSetParaFlag==1)
  {
	  if(g_DevType == EXPAND_UNIT){
		  g_RuSetParaFlag = 1;
		  init_alarm_en();
	  }
	  update();
	  printf("set china mobile dcs para\r\n");
	  g_OMCSetParaFlag = 0;
  	memset(&devinfo, 0, sizeof(DevInfo_t));
  	LoadDevicePara(&devinfo, &g_DevicePara);

    //GSM VGA����
    if (GetVGAVal(GSMVGA_ID, &val) == 1)
    {
      OMCWriteFpga(GSMVGA_ADDR, val);
    }
    //3G VGA����
    if (GetVGAVal(G3VGA_ID, &val) == 1)
    {
      OMCWriteFpga(G3VGA_ADDR, val);
    }
    //LTE1 VGA����
    if (GetVGAVal(LTE1VGA_ID, &val) == 1)
    {
      OMCWriteFpga(LTE1VGA_ADDR, val);
    }
    //LTE2 VGA����
    if (GetVGAVal(LTE2VGA_ID, &val) == 1)
    {
      OMCWriteFpga(LTE2VGA_ADDR, val);
    }
    
    if (GetDataGainVal(GSMDATAGAIN_ID, &val) == 1)//GSM ������������
    {
      val = (INT16U)(pow(10.0, (double)val/200) * 8192 + 0.5);
      OMCWriteFpga(GSMDATAGAIN_ADDR, val);
    }
    if (GetDataGainVal(G3DATAGAIN_ID, &val) == 1)//3G ������������
    {
      val = (INT16U)(pow(10.0, (double)val/200) * 8192 + 0.5);
      OMCWriteFpga(G3DATAGAIN_ADDR, val);
    }
    if (GetDataGainVal(LTE1DATAGAIN_ID, &val) == 1)//LTE1 ������������
    {
      val = (INT16U)(pow(10.0, (double)val/200) * 8192 + 0.5);
      OMCWriteFpga(LTE1DATAGAIN_ADDR, val);
    }
    if (GetDataGainVal(LTE2DATAGAIN_ID, &val) == 1)//LTE2 ������������
    {
      val = (INT16U)(powf(10.0, (double)val/200) * 8192 + 0.5);
      OMCWriteFpga(LTE2DATAGAIN_ADDR, val);
    }
	if(get_net_group() != 0x331){
		val2 = 0xffff; // �ŵ�����16
		for(i = 0; i < 16; i++){
			if(DbGetThisIntPara(WORKINGCHANNEL_SW_ID+i, &val) == 1){
				if((val&0x1) == 1){
					val2 &= ~(1<<i);
				}
			}
		}
	}
    OMCWriteFpga(GSMCARRIERMASK_ADDR, val2);
    if (DbGetThisIntPara(AGCREFERENCE_ID, &val) == 1)//AGC �ο�ֵ����
      OMCWriteFpga(AGCREFERENCE_ADDR, val);
    if (DbGetThisIntPara(TESTMODELSWITCH_ID, &val) == 1)//����ģʽ������ģʽ�л�
      drv_write_fpga(TESTMODELSWITCH_ADDR, val);  //�˲���Ϊֻд�Ĵ���
    if (DbGetThisIntPara(INTERTESTEN_ID, &val) == 1)//�ڲ�����Դʹ��
      OMCWriteFpga(INTERTESTEN_ADDR, val);
    if (DbGetThisIntPara(INTEFREQRSRC_ID, &val) == 1)//�ڲ�ԴƵ������
      OMCWriteFpga(INTEFREQRSRC_ADDR, val);
    //�ز�Ƶ������
    if (g_DevType == MAIN_UNIT)
    {
		// �Զ�ʱ��У������
		if(get_net_group() == 0x331){
			DbGetThisIntPara(CDMA_AUTO_DELAY_SW_ID, &val1);
		}else{
			DbGetThisIntPara(GSM_AUTO_DELAY_SW_ID, &val1);
			DbGetThisIntPara(TD_AUTO_DELAY_SW_ID, &val2);
			DbGetThisIntPara(LTE1_AUTO_DELAY_SW_ID, &val3);
		}
		printf("\n\n\n********************\n\n");
		printf("sw=%d\n", val1);
		if(val1 == 1){
		//if((val1 == 1) || (val2 == 1) || (val3 == 1)){
			set_auto_delay_sw(1);
		}else{
			set_auto_delay_sw(0);
		}
		if(dru_dev_p->dev_att==DEV_SS1112_TDD){
			//LTE1��������˥��
			if (DbGetThisIntPara(LTE1ULATT_ID, &val) == 1);
			ss112_lte1_config(val*2);
			//LTE2��������˥��
			if (DbGetThisIntPara(LTE2ULATT_ID, &val) == 1);
			ss112_lte2_config(val*2);
			//TD��������˥��
			if (DbGetThisIntPara(G3ULATT_ID, &val) == 1);
			ss112_td_config(val*2);
			//GSM��������˥��
			if (DbGetThisIntPara(GSMULATT_ID, &val) == 1);
			ss112_gsm_config(val*2);
		}else if(dru_dev_p->dev_att==DEV_SS1112_FDD){
			//LTE1��������˥��
			if (DbGetThisIntPara(FDD1ULATT_ID, &val) == 1);
			ss112_lte1_config(val*2);
			//LTE2��������˥��
			if (DbGetThisIntPara(FDD2ULATT_ID, &val) == 1);
			ss112_lte2_config(val*2);
			//TD��������˥��
			if (DbGetThisIntPara(G3ULATT_ID, &val) == 1);
			ss112_td_config(val*2);
			//GSM��������˥��
			if (DbGetThisIntPara(GSMULATT_ID, &val) == 1);
			ss112_gsm_config(val*2);
		}
		if(get_net_group() != 0x331){
			// LTE ʱ϶�Զ�����
			if (DbGetThisIntPara(LTE_AUTO_CONFIG_ID, &val) == 1)
				OMCWriteFpga(LTE_AUTO_CONFIG_ADDR, val);
			// 2G���ѡƵģʽѡ��
			if (DbGetThisIntPara(BINDWIDTH_MODEL_ID, &val) == 1)
				OMCWriteFpga(BINDWIDTH_MODEL_ADDR, val);

			// TD-SCDMAʱ϶����, �ڶ�ʱ϶�л���
			if (DbGetThisIntPara(TDS_TIMESLOT_ID, &val) == 1)
				OMCWriteFpga(TDS_TIMESLOT_ADDR, val);
			// TD-LTEʱ϶���� ʱ϶��ǰ��������֡�ں�
			if (DbGetThisIntPara(TDLTE_TIMESLOT_ID, &val) == 1){
				if (DbGetThisIntPara(TDLTE_SUB_ID, &val2) == 1){
					OMCWriteFpga(TDLTE_TIMESLOT_ADDR, (((val&0xf)<<4) | (val2&0xf)));
				}
			}
			// TD-SCDMAͬ����������
			if (DbGetThisIntPara(TDS_SYN_THR_ID, &val) == 1){
				f1 = (float)val; 
				f2 = 11610*exp(0.01156*f1);  // y=11610*exo(0.1156*x)  �������޵Ĵ������10
				OMCWriteFpga(TDS_SYN_THR_ADDR, (short)f2);
			}
			// TD-LTEͬ����������
			if (DbGetThisIntPara(TDLTE_SYN_THR_ID, &val) == 1){
				f1 = (float)val; 
				f2 = 11610*exp(0.01156*f1);  // y=11610*exo(0.1156*x)  �������޵Ĵ������10
				OMCWriteFpga(TDLTE_SYN_THR_ADDR, (short)f2);
			}
			//TD��һת����΢��
			if (DbGetThisIntPara(TD_FIRTRP_FINE_ID, &val) == 1)
				drv_write_fpga(TD_FIRTRP_FINE_REG,(val*0x1f + 0x399));
			//TD�ڶ�ת����΢��
			if (DbGetThisIntPara(TD_SECTRP_FINE_ID, &val) == 1)
				drv_write_fpga(TD_SECTRP_FINE_REG,(val*0x1f + 0x399));
			//LTE��һת����΢��
			if (DbGetThisIntPara(LTE_FIRTRP_FINE_ID, &val) == 1)
				drv_write_fpga(LTE_FIRTRP_FINE_REG,(val*0x1f + 0x433));
			//LTE�ڶ�ת����΢��
			if (DbGetThisIntPara(LTE_SECTRP_FINE_ID, &val) == 1)
				drv_write_fpga(LTE_SECTRP_FINE_REG,(val*0x1f + 0x5a7));
			//LTE����ͨ��ѡ��
			if (DbGetThisIntPara(LTE_CHNCHS_ID, &val) == 1)
				drv_write_fpga(LTE_CHNCHS_REG, val);
		}
		if((get_net_group() == 0x331)){ // ����
			//LTE1 Ƶ������ 
			DbGetThisIntPara(FDD1WORKCHANNELNO_ID, &val);
			if(val < 600){ // 0~588 lte-fdd ����
				val = 2110000 + 100*(val-0) - 153600;
				dru_lmx2531_fdd_lte1_config(val);
			}else if((1200<=val) && (val<=1949)){ //1200~1949 ����Ƶ��33 fdd
				val = 1805000 + 100*(val-1200) - 153600;//153.6MHzƵ��ƫ��
				dru_lmx2531_fdd_lte1_config(val);
			}
			//LTE2 Ƶ������ 
			DbGetThisIntPara(FDD2WORKCHANNELNO_ID, &val);
			if(val < 600){ // 0~588 lte-fdd ����
				val = 2110000 + 100*(val-0) - 153600;
				dru_lmx2531_fdd_lte2_config(val);
			}else if((1200<=val) && (val<=1949)){ //1200~1949 ����Ƶ��33 fdd
				val = 1805000 + 100*(val-1200) - 153600;//153.6MHzƵ��ƫ��
				dru_lmx2531_fdd_lte2_config(val);
			}
		}else{
			//2G Ƶ������
			for(i=0; i<16; i++)
			{
				if (DbGetThisIntPara((WORKINGCHANNELNO_ID+i), &val) == 1)//WORKINGCHANNELNO_ID�ŵ���
				{
					//����Ԫ����:DDC ����:y=(x-957)/0.04 ��y>=0ʱ,y=y;��y<0ʱ,y=768-y;DUC ����:z=768-y;
					if (val < 96)//��Ƶ�� china_mobile gsm
					{
						val = ((93500+20*val)-94400)/4;
					}
					else if ((val<125) && (val > 95))//��Ƶ��china unicom  gsm
					{
						val = ((93500+20*val)-95700)/4;
					}
					else if((1000<=val) && (val<=1023))//1000��1023 china mobile e-gsm
					{
						val = ((93500+20*(val-1024))-94400)/4;
					}
					else if((val >= 512) && (val <= 661))//china mobile DCS
					{
						val = ((180500+20*(val-511))-181760)/4;
					}
					else if((val >= 662) && (val <= 736))//china unicom DCS
					{
						val = ((180500+20*(val-511))-184000)/4;
					}
					if (val < 0)
					{
						val = val*(-1);
						val = 768 - val;
					}
					//����DDC
				//	drv_write_ddc(i, val);
					drv_write_ddc(i, 768-val);
					//����DUC
				//	drv_write_duc(i, (768-val));
					drv_write_duc(i, val);
				}
			}
			//3G Ƶ������ 
		/*	DbGetThisIntPara(CH2WORKCHANNELNO_ID, &val);
			printf("3g channel number is %d\n", val);
			if((10562<=val) && (val<=10838))  //10562��10838 wcdma
			{
				//printf("wcdma\n");
				val = 200*val - 153600;//153.6MHzƵ��ƫ��
				dru_lmx2531_wcdma_config(val);
			}
			else if((10050<=val) && (val<=10125))  // TD-SCDMA AƵ��
			{
				//printf("td-scdma  a\n");
				val = 200*val - 92160;//92.16MHzƵ��ƫ��
				dru_lmx2531_wcdma_config(val);
			}else if((9400<=val) && (val<=9600)){ // TD-SCDMA FƵ��
				//printf("td-scdma  f\n");
				val = 200*val - 92160;//92.16MHzƵ��ƫ��
				printf("lo=%dkhz\n", val);
				dru_lmx2531_wcdma_config(val);
			}else{
				//printf("3g channel number error\n");
			}
			//LTE1 Ƶ������ 
			DbGetThisIntPara(CH3WORKCHANNELNO_ID, &val);
			if((1200<=val) && (val<=1949))//1200~1949 ����Ƶ��33 fdd
			{
				val = 1805000 + 100*(val-1200) - 153600;//153.6MHzƵ��ƫ��
				dru_lmx2531_fdd_lte1_config(val);
			}
			else if((2750<=val) && (val<=3449))//1200~1949 ����Ƶ��37 fdd
			{
				val = 2620000 + 100*(val-2750) - 153600;//153.6MHzƵ��ƫ��
				dru_lmx2531_fdd_lte1_config(val);
			}
			else if((val >= 37750) && (val<=38249))// ����Ƶ��D tdd
			{
				val = 2570000 + 100*(val-37750) - 92160;//92.16MHzƵ��ƫ��
				dru_lmx2531_lte1_config(val);
			}
			else if((val >= 38250) && (val<=38649))// ����Ƶ��F tdd
			{
				val = 1880000 + 100*(val-38250) - 92160;//92.16MHzƵ��ƫ��
				dru_lmx2531_lte1_config(val);
			}
			else if((val >= 38650) && (val<=39649))// ����Ƶ��E tdd
			{
				val = 2300000 + 100*(val-38650) - 92160;//92.16MHzƵ��ƫ��
				dru_lmx2531_lte1_config(val);
			}
			//LTE2 Ƶ������ 
			DbGetThisIntPara(CH4WORKCHANNELNO_ID, &val);
			if((1200<=val) && (val<=1949))//1200~1949
			{
				val = 1805000 + 100*(val-1200) - 153600;//153.6MHzƵ��ƫ��
				dru_lmx2531_fdd_lte2_config(val);
			}
			else if((2750<=val) && (val<=3449))//1200~1949 ����Ƶ��37
			{
				val = 2620000 + 100*(val-2750) - 153600;//153.6MHzƵ��ƫ��
				dru_lmx2531_fdd_lte2_config(val);
			}
			else if((val >= 37750) && (val<=38249))// ����Ƶ��D tdd
			{
				val = 2570000 + 100*(val-37750) - 92160;//92.16MHzƵ��ƫ��
				dru_lmx2531_lte2_config(val);
			}
			else if((val >= 38250) && (val<=38649))// ����Ƶ��F tdd
			{
				val = 1880000 + 100*(val-38250) - 92160;//92.16MHzƵ��ƫ��
				dru_lmx2531_lte2_config(val);
			}
			else if((val >= 38650) && (val<=39649))// ����Ƶ��E tdd
			{
				val = 2300000 + 100*(val-38650) - 92160;//92.16MHzƵ��ƫ��
				dru_lmx2531_lte2_config(val);
			}*/
		}

		if (DbGetThisIntPara(LTE_ROMOTE_SW_ID, &val) == 1){ //LTEԶ�̹��ʿ���
			drv_read_fpga(0x5a, &temp);
			if((val&3) != (temp&3)){
				drv_write_fpga(0x5a, val&3);
			}
		}
		if (DbGetThisIntPara(TD_ROMOTE_SW_ID, &val) == 1){ //TDԶ�̹��ʿ���
			drv_read_fpga(0x5b, &temp);
			if((val&3) != (temp&3)){
				drv_write_fpga(0x5b, val&3);
			}
		}

	}
    else if (g_DevType == RAU_UNIT)
   	{
	
	}
  }
  if ((time(NULL)-m_query_time) > 2)//�澯��ѯ��ʱ,���6s
  {
	  m_query_time = time(NULL);
	  
	  //LTE1 ���ֹ���
	  drv_read_fpga(LTE1DATAPOWER_ADDR, &para2);
	  DbSaveThisIntPara_MCP_C(LTE1DATAPOWER_ID, para2&0x1ff, 1);
	  //LTE2 ���ֹ���
	  drv_read_fpga(LTE2DATAPOWER_ADDR, &para2);
	  DbSaveThisIntPara_MCP_C(LTE2DATAPOWER_ID, para2&0x1ff, 1);
	  //3G ���ֹ���
	  drv_read_fpga(G3DATAPOWER_ADDR, &para2);
	  DbSaveThisIntPara_MCP_C(G3DATAPOWER_ID, para2&0x1ff, 1);
	  //GSM ���ֹ���
	  drv_read_fpga(GSMDATAPOWER_ADDR, &para2);
	  DbSaveThisIntPara_MCP_C(GSMDATAPOWER_ID, para2&0x1ff, 1);
	  if (g_DevType == MAIN_UNIT)
	  {
		  // ϵͳ���ʱ��
		  val = get_max_delay();
		  val = val*8.138;
		  if(get_net_group() == 0x331){
			  DbSaveThisIntPara_MCP_C(CDMA_MAX_DELAY_ID, val+3800, 1);
			  DbSaveThisIntPara_MCP_C(FDD1_MAX_DELAY_ID, val+3800, 1);
			  DbSaveThisIntPara_MCP_C(FDD2_MAX_DELAY_ID, val+3800, 1);
		  }else{
			  DbSaveThisIntPara_MCP_C(GSM_MAX_DELAY_ID, val+8000, 1);
			  DbSaveThisIntPara_MCP_C(TD_MAX_DELAY_ID, val+3800, 1);
			  DbSaveThisIntPara_MCP_C(LTE1_MAX_DELAY_ID, val+3800, 1);
			  DbSaveThisIntPara_MCP_C(LTE2_MAX_DELAY_ID, val+3800, 1);
		  }
		  //GSM�������빦�ʵ�ƽ(ϵ��1),-15dBm+([14'h0054]-[14'h0049]+[14'h004d]-[14'h0045])/2
		  drv_read_fpga(0x0053, &para2);
		  val = (para2&0x3f);
		  drv_read_fpga(0x0049, &para2);
		  val = val - (para2&0x3f);
		  drv_read_fpga(0x004d, &para2);
		  val = val + (para2&0x1ff);
		  drv_read_fpga(0x0045, &para2);
		  val = val - (para2&0x1ff);
		  val = val/2;
		  val2 = -15 + val ;
		  DbGetThisIntPara(GSMDLPOWEROFFSET_ID, &val);
		  val = signed_2to4((short)val);
		  val2 = val2 + val/10;
		  if(get_net_group() == 0x331){
			  DbGetThisIntPara(CDMADLATT_ID, &val);
		  }else{
			  DbGetThisIntPara(GSMDLATT_ID, &val);
		  }
		  val2 = val2 + val;
		  if(get_net_group() == 0x331){
			  DbSaveThisIntPara_MCP_C(CDMADLPOWER_ID, val2&0xff, 1);
		  }else{
			  DbSaveThisIntPara_MCP_C(GSMDLPOWER_ID, val2&0xff, 1);
		  }
		  //3G�������빦�ʵ�ƽ(ϵ��1),-15dBm+([14'h0053]-[14'h0048]+[14'h004c]-[14'h0045])/2
		  drv_read_fpga(0x0053, &para2);
		  val = para2&0x3f;
		  drv_read_fpga(0x0048, &para2);
		  val = val - (para2&0x3f);
		  drv_read_fpga(0x004c, &para2);
		  val = val + (para2&0x1ff);
		  drv_read_fpga(0x0045, &para2);
		  val = val - (para2&0x1ff);
		  val = val/2;
		  val2 = -15 + val;
		  DbGetThisIntPara(G3DLPOWEROFFSET_ID, &val);
		  val = signed_2to4((short)val);
		  val2 = val2 + val/10;
		  DbGetThisIntPara(G3DLATT_ID, &val);
		  val2 = val2 + val;
		  DbSaveThisIntPara_MCP_C(G3DLPOWER_ID, val2&0xff, 1);
		  
		  if(get_net_group() == 0x331){ // ����
			  //LTE1�������빦�ʵ�ƽ(ϵ��10),-15dBm+([14'h0051]-[14'h0046]+[14'h004a]-[14'h0045])/2
			  drv_read_fpga(0x0051, &para2);
			  val = (para2&0x3f);
			  drv_read_fpga(0x0046, &para2);
			  val = val - (para2&0x3f);
			  drv_read_fpga(0x004a, &para2);
			  val = val + (para2&0x1ff);
			  drv_read_fpga(0x0045, &para2);
			  val = val - (para2&0x1ff);
			  val = val/2;
			  val2 = -15 + val;
			  DbGetThisIntPara(LTE1DLPOWEROFFSET_ID, &val);
			  val = signed_2to4((short)val);
			  val2 = val2 + val/10;
			  if(get_net_group() == 0x331){
				  DbGetThisIntPara(FDD1DLATT_ID, &val);
			  }else{
				  DbGetThisIntPara(LTE1DLATT_ID, &val);
			  }
			  val2 = val2 + val;
			  if(get_net_group() == 0x331){
				  DbSaveThisIntPara_MCP_C(FDD1DLPOWER_ID, val2&0xff, 1);
			  }else{
				  DbSaveThisIntPara_MCP_C(LTE1DLPOWER_ID, val2&0xff, 1);
			  }
			  //LTE2�������빦�ʵ�ƽ(ϵ��10),-15dBm+([14'h0052]-[14'h0047]+[14'h004b]-[14'h0045])/2
			  drv_read_fpga(0x0052, &para2);
			  val = (para2&0x3f);
			  drv_read_fpga(0x0047, &para2);
			  val = val - (para2&0x3f);
			  drv_read_fpga(0x004b, &para2);
			  val = val + (para2&0x1ff);
			  drv_read_fpga(0x0045, &para2);
			  val = val - (para2&0x1ff);
			  val = val/2;
			  val2 = -15 + val;
			  DbGetThisIntPara(LTE2DLPOWEROFFSET_ID, &val);
			  val = signed_2to4((short)val);
			  val2 = val2 + val/10;
			  if(get_net_group() == 0x331){
				  DbGetThisIntPara(FDD2DLATT_ID, &val);
			  }else{
				  DbGetThisIntPara(LTE2DLATT_ID, &val);
			  }
			  val2 = val2 + val;
			  if(get_net_group() == 0x331){
				  DbSaveThisIntPara_MCP_C(FDD2DLPOWER_ID, val2&0xff, 1);
			  }else{
				  DbSaveThisIntPara_MCP_C(LTE2DLPOWER_ID, val2&0xff, 1);
			  }
		  }else{
			  drv_read_fpga(0x0067, &para2);
			  power = 8.81*log(para2)-87.2;
			  //printf("power=%f.\n", power);
			  if((m_lte_num&0x1) == 0){
				  //LTE1�������빦�ʵ�ƽ(ϵ��1),-15dBm+([14'h0051]-[14'h0046]+[14'h004a]-[14'h0045])/2
				  drv_read_fpga(0x0051, &para2);
				  val = (para2&0x3f);
				  drv_read_fpga(0x0046, &para2);
				  val = val - (para2&0x3f);
				  val2 = val/2;
				  DbGetThisIntPara(LTE1DLPOWEROFFSET_ID, &val);
				  val = signed_2to4((short)val);
				  val2 = val2 + val/10;
				  DbGetThisIntPara(LTE1DLATT_ID, &val);
				  val2 = val2 + val;
				  val2 = (int)power + val2;
				  drv_read_fpga(LTE_ASYNC_ADDR, &para2);
				  DbGetThisIntPara(LTE_CHNCHS_ID, &val);
				  if(((para2&0x1) == 0) || (val == 2)){ // ʧ�� ͨ��ѡ��2
					  DbSaveThisIntPara_MCP_C(LTE1DLPOWER_ID, -40, 1);
				  }else{
					  DbSaveThisIntPara_MCP_C(LTE1DLPOWER_ID, val2, 1);
				  }
				  //printf("let1 power=%f, val2=%d\n", power, val2);
			  }else{
				  //LTE2�������빦�ʵ�ƽ(ϵ��1),-15dBm+([14'h0052]-[14'h0047]+[14'h004b]-[14'h0045])/2
				  drv_read_fpga(0x0052, &para2);
				  val = para2;
				  drv_read_fpga(0x0047, &para2);
				  val = val - para2;
				  val2 = val/2;
				  DbGetThisIntPara(LTE2DLPOWEROFFSET_ID, &val);
				  val = signed_2to4((short)val);
				  val2 = val2 + val/10;
				  DbGetThisIntPara(LTE2DLATT_ID, &val);
				  val2 = val2 + val;
				  val2 = power + val2;
				  drv_read_fpga(LTE_ASYNC_ADDR, &para2);
				  DbGetThisIntPara(LTE_CHNCHS_ID, &val);
				  if(((para2&0x1) == 0) || (val == 1)){ // ʧ�� ͨ��ѡ��1
					  DbSaveThisIntPara_MCP_C(LTE2DLPOWER_ID, -40, 1);
				  }else{
					  DbSaveThisIntPara_MCP_C(LTE2DLPOWER_ID, val2, 1);
				  }
				  //printf("let2 power=%f, val2=%d\n", power, val2);
			  }
			  m_lte_num++;
			  drv_write_fpga(0x14C, m_lte_num&0x1);
		  }
	  }
	  
	  if (g_DevType == RAU_UNIT)
	  {
	  }
  }
  if ((time(NULL)-g_AlarmDectTime) > ALARMDECTINTERVAL){//�澯��ѯ��ʱ,���6s
	  g_AlarmDectTime = time(NULL);
	  if(no_alarm_flag == 0){ // ͨ��fun no_alarm_flag = 1, ���ø澯��⣬����CPUʹ����
		  if(is_check_alarm() == 0){
			  actual_alarm();
			  common_alarm();
		  }
	  }
  }
}
int no_alarm_change(int argc, char * argv[])
{
	unsigned int para1;

	msg_tmp.mtype = MSG_FUN_RESULT;	
	if(argc != 2){
		printf("input para cnt is not 2.\r\n");
		sprintf(msg_tmp.mtext, "input para cnt is not 2.\r\n");
		msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
		return 1;
	}
	para1 = strtol(argv[1], NULL, 16);
	printf("para1=%d. \r\n", para1);
	no_alarm_flag = para1;
	sprintf(msg_tmp.mtext, "no_alram=%d\r\n", para1);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}
// ������
unsigned short read_pw(void)
{
	unsigned short para = 0;
	unsigned short temp = 0;
	int i = 0;

	for(i = 0; i < 50; i++){
		drv_read_fpga(0x0087, &temp);
		if(temp > para){
			para = temp;
		}
		usleep(100); // 100ms���һ��
	}
	//printf("pw = 0x%04x\n", para);
	return para;
}
// д���ŵ���
int write_channel(int num)
{	
	int val = 0;

	if(num < 96){// china mobile gsm
		val = ((93500+20*num)-94400)/4;
	}else if((num >= 512) && (num <= 661)){//china mobile DCS
		val = ((180500+20*(num-511))-181760)/4;
	}
	if (val < 0)
	{
		val = val*(-1);
		val = 768 - val;
	}
	//����DDC
	drv_write_ddc(15, val);
	//����DUC
	drv_write_duc(15, (768-val));
	return 0;
}
// 
void * auto_channel_pthread(void * arg)
{
	unsigned short para = 0;
	unsigned short temp = 0;
	int state = 0;
	int flag = 1; // �л��ŵ��ţ������״̬��־
	int channel_num = 0;
	int channel_buf[16];
	int tbuf[16];
	int idx = 0;
	int i = 0;
	int ret = 0;
	int val = 0;
	unsigned short para1 = 0;
	int end_flag = 0;
	int start_num = 0;
	int end_num = 0;

	if(get_net_type() == 1){ // 1:GSM 2:DCS
		start_num = -1;
		end_num = 95;
	}else{
		start_num = 511;
		end_num = 661;
	}
	channel_num = start_num;

	memset(channel_buf, 0, 64);
	memset(tbuf, 0, 64);
	while(1){
		DbGetThisIntPara(AUTO_CHANNEL_SW_ID, &val);
		if(val&0x1 == 1){
			//printf("\n\nauto channel start ....\n\n");
			drv_read_fpga(GSMCARRIERMASK_ADDR, &para1);
			para1 &= ~(1<<15);
			drv_write_fpga(GSMCARRIERMASK_ADDR, para1);
			//printf("state=%d\n", state);
			switch(state){
				case 0: // �����ŵ���
					channel_num++;
					if(channel_num > end_num){
						channel_num = start_num+1;
						end_flag = 1;
					}
					//printf("channel_num=%d\n", channel_num);
					write_channel(channel_num);
					state = flag;
					break;
				case 1: // ��⹦�� > 0x200
					temp = read_pw();
					if(temp > 0x200){
						para = temp;
						flag = 2;
					}else{
						flag = 1;
					}
					state = 0;
					break;
				case 2: // �ж��ŵ����Ƿ���Ч
					temp = read_pw();
					if(temp > para){
						para = temp;
						flag = 2;
					}else{
						printf("channel_num = %d, pw = 0x%04x.\n", channel_num-1, para);
						tbuf[idx++] = channel_num-1;
						flag = 3;
						para = temp;
					}
					state = 0;
					break;
				case 3: // �жϽ���
					temp = read_pw();
					if(temp < 0x200){
						flag = 1;
						temp = 0;
						para = 0;
					}else{
						if(temp > para){ // ������С
							flag = 2;
						}
						para = temp;
					}
					state = 0;
					break;
				default:
					state = 0;
					flag = 1;
			}
			if(end_flag == 1){ // �����Զ�������
				end_flag = 0;
				//printf("auto channel end.\n\n");
				//printk(channel_buf, 64);
				//printf("\n");
				//printk(tbuf, 64);
				ret = memcmp((void *)channel_buf, (void *)tbuf, 64);
				if(ret != 0){
					//printf("find new channel\n");
					for(i = 0; i < 16; i++){
						if(i < idx){
							DbGetThisIntPara(WORKINGCHANNELNO_ID+i, &val);
							if(val != tbuf[i]){
								DbSaveThisIntPara_MCP_C(WORKINGCHANNELNO_ID+i, tbuf[i], 0);
								DbSaveThisIntPara_MCP_C(WORKINGCHANNEL_SW_ID+i, 1, 0);
							}
						}else{
							DbGetThisIntPara(WORKINGCHANNELNO_ID+i, &val);
							if(val != 0){
								DbSaveThisIntPara_MCP_C(WORKINGCHANNELNO_ID+i, 0, 0);
								DbSaveThisIntPara_MCP_C(WORKINGCHANNEL_SW_ID+i, 0, 0);
							}
						}
					}
					g_OMCSetParaFlag = 1;
					g_OmcBroadcastFlag = 1;
				}
				memcpy((void *)channel_buf, (void *)tbuf, 64);
				memset((void *)tbuf, 0, 64);
				idx = 0;
				sleep(20);
			}	
		}else{
			sleep(10);
		}
	}
	return (void *)0;
}
// �Զ��ز������߳�
int creat_auto_channel(void)
{
	pthread_t auto_channel_id;	
	if( pthread_create(&auto_channel_id, NULL, auto_channel_pthread, NULL)){
		printf("pthread_create auto_channel_pthread error.\r\n");
		return -1;
	}
	return 0;
}
