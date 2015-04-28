#include "../../driver/omap_epld.h"
#include "../../driver/drv_api.h"
#include "../../driver/dru_spi.h"
#include "common.h"
#include "apue.h"
#include "commonfun.h"
#include "druhwinfo.h"
#include "apparadef.h"
#include "../../sqlite/drudatabase.h"
#include "../../task/omccom/omccom.h"

extern unsigned char g_din_pw[4];  // �������빦��
extern unsigned char g_uatt[4];    // ����˥��ֵ
extern unsigned char g_din_under_alarm[4]; // ��������Ƿ���ʸ澯
extern unsigned char g_lo_alarm;   // ����ʧ���澯
extern unsigned char g_power_down_alarm; // ���뵥Ԫ����Դ����澯
extern unsigned char g_power_error_alarm; // ���뵥Ԫ����Դ���ϸ澯

int alarm_debug = 0;    // ������Ϣ��ӡ����
#define dprintf(format,...) \
	do{ \
		if(alarm_debug == 1) \
			printf("File:"__FILE__", Line:%05d: "format"", __LINE__, ##__VA_ARGS__);\
	}while(0);
extern volatile int g_alarm_report_cnt;
extern volatile int g_alarm_report_time;
extern int g_DevType;
struct alarm_st 
{
	unsigned int id; // ����ID
	int val;  // ���õĸ澯ֵ
	int pro;  // ���õĸ澯ֵ�Ƿ���Ч��1�����õĸ澯ֵ��Ч��0��ʵ�ʼ��ĸ澯ֵ��Ч
};
struct alarm_st ms_alarm[] = {
	{0x00000301, 0, 0}, // ��Դ����
	{0x00000302, 0, 0}, // ��Դ����
	{0x00000304, 0, 0}, // ��ع���
	{0x00000305, 0, 0}, // λ�ø澯
	{0x00000308, 0, 0}, // ����ģ��澯
	{0x00000309, 0, 0}, // ����ʧ���澯
	{0x00000320, 0, 0}, // �ⲿ�澯1
	{0x00000321, 0, 0}, // �ⲿ�澯2
	{0x00000322, 0, 0}, // �ⲿ�澯3
	{0x00000323, 0, 0}, // �ⲿ�澯4
	{0x00000324, 0, 0}, // �ⲿ��G5
	{0x00000325, 0, 0}, // �ⲿ�澯6
};
#define ALARM_IO_ADDR 0x14D

unsigned int mode_idx[4] = {0x0, 0x3000, 0x6000, 0x7000};

// ����������������ʱ����
int ru_dout_pw[8][4]; // �����������
int ru_dout_under_thd[4]; // ���������������4��
int ru_dout_over_thd[4]; // �������Ƿ������4��
int ru_dout_under_alarm_en[8][4]; // Ƿ����ʹ��
int ru_dout_over_alarm_en[8][4];  // ������ʹ��

// ��չ�澯ʹ��
struct alarm_en_st{
	int ru_rf_en[8]; // ��Ƶ��·����ʹ��
	int ru_dev_temp_en[8]; // �豸���¸澯ʹ��
	int ru_dout_under_en[8][4]; // �������Ƿ���ʸ澯ʹ��
	int ru_dout_over_en[8][4]; // ������������ʸ澯ʹ��
	int ru_laser_en[8]; // Զ�˹��շ�ģ����ϸ澯ʹ��
	int ru_link_en[8]; // Զ����·���ϸ澯ʹ��
};
struct alarm_en_st g_alarm_en; // �澯ʹ��

struct alarm_thd_st{ // �澯����

};
struct alarm_sta_st{ // �澯״̬

};
// ����ʽ��ʼ��mode_idx[]
void init_mode_idx(void)
{
	int flag = 0;

	flag = get_net_group();
	printf("net_group is 0x%08x\n", flag);
	if((flag&0x00000f00) == 0x00000300){ // ����
		printf("cdma mode.\n");
		mode_idx[0] = 0x2000;
		mode_idx[1] = 0x0000; // wcmda  ������
		mode_idx[2] = 0x4000;
		mode_idx[3] = 0x5000;
	}
}
// �澯ʹ�ܳ�ʼ��
int init_alarm_en(void)
{
	int i, j;
	int val = 0;

	for(i = 0; i < 8; i++){
		DbGetThisIntPara(RU_RF_EN_ID+i, &val); 
		g_alarm_en.ru_rf_en[i] = val; 
		DbGetThisIntPara(RU_DEV_TEMP_EN_ID+i, &val); 
		g_alarm_en.ru_dev_temp_en[i] = val; 
		DbGetThisIntPara(RU_LASER_EN_ID+i, &val); 
		g_alarm_en.ru_laser_en[i] = val; 
		DbGetThisIntPara(RU_LINK_EN_ID+i, &val); 
		g_alarm_en.ru_link_en[i] = val; 
		for(j = 0; j < 4; j++){
			DbGetThisIntPara(RU_DOUT_UNDER_EN_ID+i+mode_idx[j], &val); 
			g_alarm_en.ru_dout_under_en[i][j] = val; 
			printf("id_en_under=0x%08x, val=%d, mode_idx=%d, idx=%d\n", RU_DOUT_UNDER_EN_ID+i+mode_idx[j], val, mode_idx[j], j);
			DbGetThisIntPara(RU_DOUT_OVER_EN_ID+i+mode_idx[j], &val); 
			g_alarm_en.ru_dout_over_en[i][j] = val; 
			printf("id_en_over=0x%08x, val=%d\n", RU_DOUT_OVER_EN_ID+i+mode_idx[j], val);
		}
	}
	for(i = 0; i < 4; i++){
		DbGetThisIntPara(RU_DOUT_UNDER_THD+mode_idx[i], &val); 
		ru_dout_under_thd[i] = val;
		printf("id_thd_under=0x%08x, val=%d\n", RU_DOUT_UNDER_THD+mode_idx[i], val);
		DbGetThisIntPara(RU_DOUT_OVER_THD+mode_idx[i], &val); 
		ru_dout_over_thd[i] = val;
		printf("id_thd_over=0x%08x, val=%d\n", RU_DOUT_OVER_THD+mode_idx[i], val);
	}

	return 0;
}

// ��ȡ��Դ����״̬
int get_power_down(void)
{
	unsigned short para;

	if(ms_alarm[0].pro == 1){
		return ms_alarm[0].val;
	}else{
		drv_read_fpga(ALARM_IO_ADDR, &para);
		if((para&0x1) == 1){
			return 1;
		}else{
			return 0;
		}
	}
}
// ��ȡ��Դ����״̬
int get_power_error(void)
{
	unsigned short para;

	if(ms_alarm[1].pro == 1){
		return ms_alarm[1].val;
	}else{
		drv_read_fpga(ALARM_IO_ADDR, &para);
		if(((para>>1)&0x1) == 1){ //����
			return 0;
		}else{
			return 1;
		}
	}
}
// ��ȡ��ع���״̬
int get_bat_error(void)
{
	unsigned short para;

	if(ms_alarm[2].pro == 1){
		return ms_alarm[2].val;
	}else{
		drv_read_fpga(ALARM_IO_ADDR, &para);
		if(((para>>2)&0x1) == 1){ //����
			return 0;
		}else{
			return 1;
		}
		return 0;
	}
}
// ��ȡλ�ø澯״̬
int get_shift_error(void)
{
	unsigned short para;

	if(ms_alarm[3].pro == 1){
		return ms_alarm[3].val;
	}else{
		drv_read_fpga(ALARM_IO_ADDR, &para);
		if(((para>>3)&0x1) == 0){ //λ�ø澯
			return 1;
		}else{
			return 0;
		}
	};
}
// ��ȡ����ģ��澯״̬
int get_other_error(void)
{
	//unsigned short para;

	if(ms_alarm[4].pro == 1){
		return ms_alarm[4].val;
	}else{
		return 0;
	}
}
// �ⲿ�澯
int get_outside_error(void)
{
	unsigned short para;
	int i = 0;
	int ret = 0;

	for(i = 0; i < 6; i++){
		if(ms_alarm[6+i].pro == 1){
			ret |= (ms_alarm[6+i].val<<i);
		}else{
			drv_read_fpga(ALARM_IO_ADDR, &para);
			dprintf("outside para 0x%04x\n", para);
			if(((para>>(4+i))&0x1) == 0){
				ret |= (1<<i);
				dprintf("ret = 0x%04x\n", ret);
			}
		}
	}
	return ret;
}
// ����״̬
int get_lo_error(void)
{
	unsigned short para;

	if(ms_alarm[5].pro == 1){
		return ms_alarm[5].val;
	}else{
		drv_read_epld(LOSTATUS_ADDR, &para);//1:��������,0:����澯
		para &= 0xd;
		if(para != 0xd){  // ����澯
			return 1;
		}else{
			return 0;
		}
	}
}
extern volatile unsigned short line_old;
extern volatile unsigned short link_old;
extern volatile unsigned short recv_old;
extern volatile unsigned short laser_num_old;
extern volatile unsigned short laser_num_new;
extern volatile unsigned short laser_cnt_old;
extern volatile unsigned short laser_cnt_new;
// ���շ�ģ�����
int get_line_error(void)
{
	int i = 0;
	int ret = 0;

	for(i = 0; i < 6; i++){  // �ε����ˣ����޹⣬�澯
		if(((line_old>>i)&0x1) == 0){
			if(((link_old>>i)&0x1) == 1){
				ret |= (1<<i);
			}
		}
	}
	return ret;
}
// Զ����·���� 
int get_ru_link_error(int k)
{
	// �ε���չ��ģ�飬����Զ����·����
	if(((laser_cnt_old>>k)&0x1) == 0){ // �й�ģ��
		if(((laser_cnt_new>>k)&0x1) == 1){
			return 1;
		}
	}
	return 0;
}
// ������·�澯
int get_link_error(void)
{

	//dprintf("laser_num_old=%d\n", laser_num_old);
	//dprintf("laser_num_new=%d\n", laser_num_new);
	if(laser_num_old > laser_num_new){
		return 1;
	}
	return 0;
}
// TD-SCDMAʧ���澯
int get_td_async_error(void)
{
	unsigned short para;

	drv_read_fpga(TD_ASYNC_ADDR, &para);
	if((para&0x1) == 0x1){
		return 0;
	}else{
		return 1;
	}
}
// LTEʧ���澯
int get_lte_async_error(void)
{
	unsigned short para;

	drv_read_fpga(LTE_ASYNC_ADDR, &para);
	if((para&0x1) == 0x1){
		return 0;
	}else{
		return 1;
	}
}
// ���¸澯
int get_dev_temp_error(void)
{
	int val1, val2;

	DbGetThisIntPara(DEV_TEMP_THD_ID, &val1); 
	DbGetThisIntPara(DEVTEMP_ID, &val2); 
	if(val2 < val1){
		return 0;
	}else{
		return 1;
	}
}
// Ƿ���ʸ澯
int get_gsm_din_under_error(void)
{
    int val1, val2;

	if((get_net_group()&0xf00) == 0x300){
		DbGetThisIntPara(GSM_DIN_UNDER_THD+mode_idx[0], &val1); 
		DbGetThisIntPara(GSMDLPOWER_ID+mode_idx[0], &val2); 
	}else{
		DbGetThisIntPara(GSM_DIN_UNDER_THD, &val1); 
		DbGetThisIntPara(GSMDLPOWER_ID, &val2); 
	}
	if(val2 < val1){
		return 1;
	}else{
		return 0;
	}
}
int get_td_din_under_error(void)
{
    int val1, val2;

    DbGetThisIntPara(TD_DIN_UNDER_THD, &val1); 
	DbGetThisIntPara(G3DLPOWER_ID, &val2); 
	if(val2 < val1){
		return 1;
	}else{
		return 0;
	}
}
int get_lte1_din_under_error(void)
{
    int val1, val2;

	if((get_net_group()&0xf00) == 0x300){
		DbGetThisIntPara(LTE1_DIN_UNDER_THD-0x2000, &val1); 
		DbGetThisIntPara(LTE1DLPOWER_ID-0x2000, &val2); 
	}else{
		DbGetThisIntPara(LTE1_DIN_UNDER_THD, &val1); 
		DbGetThisIntPara(LTE1DLPOWER_ID, &val2); 
	}
	if(val2 < val1){
		return 1;
	}else{
		return 0;
	}
}
int get_lte2_din_under_error(void)
{
    int val1, val2;

	if((get_net_group()&0xf00) == 0x300){
		DbGetThisIntPara(LTE2_DIN_UNDER_THD-0x2000, &val1); 
		DbGetThisIntPara(LTE2DLPOWER_ID-0x2000, &val2); 
	}else{
		DbGetThisIntPara(LTE2_DIN_UNDER_THD, &val1); 
		DbGetThisIntPara(LTE2DLPOWER_ID, &val2); 
	}
	if(val2 < val1){
		return 1;
	}else{
		return 0;
	}
}
// �����ʸ澯
int get_gsm_din_over_error(void)
{
    int val1, val2;

	if((get_net_group()&0xf00) == 0x300){
		DbGetThisIntPara(GSM_DIN_OVER_THD+mode_idx[0], &val1); 
		DbGetThisIntPara(GSMDLPOWER_ID+mode_idx[0], &val2); 
	}else{
		DbGetThisIntPara(GSM_DIN_OVER_THD, &val1); 
		DbGetThisIntPara(GSMDLPOWER_ID, &val2); 
	}
	if(val2 > val1){
		return 1;
	}else{
		return 0;
	}
}
int get_td_din_over_error(void)
{
    int val1, val2;

    DbGetThisIntPara(TD_DIN_OVER_THD, &val1); 
	DbGetThisIntPara(G3DLPOWER_ID, &val2); 
	if(val2 > val1){
		return 1;
	}else{
		return 0;
	}
}
int get_lte1_din_over_error(void)
{
    int val1, val2;

	if((get_net_group()&0xf00) == 0x300){
		DbGetThisIntPara(LTE1_DIN_OVER_THD-0x2000, &val1); 
		DbGetThisIntPara(LTE1DLPOWER_ID-0x2000, &val2); 
	}else{
		DbGetThisIntPara(LTE1_DIN_OVER_THD, &val1); 
		DbGetThisIntPara(LTE1DLPOWER_ID, &val2); 
	}
	if(val2 > val1){
		return 1;
	}else{
		return 0;
	}
}
int get_lte2_din_over_error(void)
{
    int val1, val2;

	if((get_net_group()&0xf00) == 0x300){
		DbGetThisIntPara(LTE2_DIN_OVER_THD-0x2000, &val1); 
		DbGetThisIntPara(LTE2DLPOWER_ID-0x2000, &val2); 
	}else{
		DbGetThisIntPara(LTE2_DIN_OVER_THD, &val1); 
		DbGetThisIntPara(LTE2DLPOWER_ID, &val2); 
	}
	if(val2 > val1){
		return 1;
	}else{
		return 0;
	}
}
// Զ���豸���¼��
int get_ru_dev_temp_error(int idx)
{
    int val1, val2;

    DbGetThisIntPara(DEV_TEMP_THD_ID, &val1); 
	DbGetThisIntPara(RU_DEV_TEMP_ID+idx, &val2); 
	if(val2 < val1){
		return 0;
	}else{
		return 1;
	}
}
// Զ�����Ƿ���ʼ��
int get_ru_dout_under_error(int idx, int mode)
{
	int val1, val2, i;

	//DbGetThisIntPara(RU_DOUT_UNDER_THD + mode, &val1); 
	//DbGetThisIntPara(RU_DOUT_ID+idx+mode, &val2); 
	for(i = 0; i < 4; i++){
		if(mode == mode_idx[i]){
			break;
		}
	}
	val1 = ru_dout_under_thd[i];
	val2 = ru_dout_pw[idx][i];
	//if((idx == 1)&&(i == 1)){
		//dprintf("val1=%d, val2=%d.\n", val1, val2);
		if(val1 > 127){
			val1 = 0 - (256-val1);
		}
		if(val2 > 127){
			val2 = 0 - (256-val2);
		}
	//	dprintf("thd=%d, val=%d, dout_pw=0x%02x.\n\n\n", val1, val2, ru_dout_pw[idx][i]);
	//}
	if(val2 < val1){
		return 1;
	}else{
		return 0;
	}
}
// Զ����������ʼ��
int get_ru_dout_over_error(int idx, int mode)
{
	int val1, val2, i;
	//DbGetThisIntPara(RU_DOUT_OVER_THD + mode, &val1); 
	//DbGetThisIntPara(RU_DOUT_ID+idx+mode, &val2); 
	for(i = 0; i < 4; i++){
		if(mode == mode_idx[i]){
			break;
		}
	}
	val1 = ru_dout_over_thd[i];
	val2 = ru_dout_pw[idx][i];
	printf("i = %d, idx=%d\n", i, idx);
	printf("val1_thd = %02x, val2_pw=%02x\n", val1, val2);
	if(val1 > 127){
		val1 = 0 - (256-val1);
	}
	if(val2 > 127){
		val2 = 0 - (256-val2);
	}
	if(val2 > val1){
		return 1;
	}else{
		return 0;
	}
}
// Զ�˹��շ�ģ����ϼ�� 
extern volatile int ru_laser_error;
extern volatile int ru_rf_error;
int get_ru_laser_error(int idx)
{
	if((ru_laser_error>>idx)&0x1){
		return 1;
	}else{
		return 0;
	}
}
int get_ru_rf_error(int idx)
{
	if((ru_rf_error>>idx)&0x1){
		return 1;
	}else{
		return 0;
	}
}
// д��澯״̬bit0
void set_alarm(unsigned int id, unsigned int val, int en)
{
	int val1;

	if(DbGetThisIntPara(id, &val1)  == 1){
		if(en == 0){
			if(val1 != 0){
				DbSaveThisIntPara_MCP_C(id, 0, 0);
			}
		}else{
			if((val1&0x1) != (val&0x1)){
				g_alarm_report_time = 0;
				g_alarm_report_cnt = 0;
				val1 &= (~0x1);
				val1 |= (val&0x1);
				DbSaveThisIntPara_MCP_C(id, val1, 0);
			}
		}
	}
}
// ʵʱ�澯
void actual_alarm(void)
{
	int val1;
	int ret, i;
	
	// λ�ø澯
	DbGetThisIntPara(SHIFT_ERROR_EN_ID, &val1); // ʹ��
	if((get_shift_error() > 0) && ((val1&0x1) == 1)){
		//dprintf("shift alarm !!\n");
		set_alarm(SHIFT_ERROR_ALARM_ID, 1, 1);
	}else{
		set_alarm(SHIFT_ERROR_ALARM_ID, 0,(val1&0x1));
		//dprintf("shift alarm  recover !!\n");
	}
	// �ⲿ�澯
	ret = get_outside_error();	
	//dprintf("outside alarm : 0x%04x\n", ret);
	for(i = 0; i < 6; i++){
		DbGetThisIntPara(OUTSIDE_EN_ID+i, &val1); // ʹ��
		if((((ret>>i)&0x1) == 1) && ((val1&0x1) == 1)){
	//		dprintf("outside %d alarm!!!\n");
			set_alarm(OUTSIDE_ALARM_ID+i, 1, 1);
		}else{
	//		dprintf("outside %d alarm recover !!!\n");
			set_alarm(OUTSIDE_ALARM_ID+i, 0, (val1&0x1));
		}
	}
}

#define ALARM_CNT 13 
#define RECOVER_CNT 22 
//#define ALARM_CNT 3 
//#define RECOVER_CNT 5 


// ����
unsigned int pw_down_alarm_cnt = 0;
unsigned int pw_down_all_cnt = 0;
unsigned int pw_error_alarm_cnt = 0;
unsigned int pw_error_all_cnt = 0;
unsigned int bat_error_alarm_cnt = 0;
unsigned int bat_error_all_cnt = 0;
unsigned int other_error_alarm_cnt = 0;
unsigned int other_error_all_cnt = 0;
unsigned int lo_alarm_cnt = 0;
unsigned int lo_all_cnt = 0;
unsigned int link_alarm_cnt = 0;
unsigned int link_all_cnt = 0;
unsigned int td_async_alarm_cnt = 0;
unsigned int td_async_all_cnt = 0;
unsigned int lte_async_alarm_cnt = 0;
unsigned int lte_async_all_cnt = 0;
unsigned int line_alarm_cnt[8] = {0};
unsigned int line_all_cnt[8] = {0};
unsigned int dev_temp_alarm_cnt = 0;
unsigned int dev_temp_all_cnt = 0;
unsigned int gsm_din_under_alarm_cnt=0;
unsigned int gsm_din_under_all_cnt=0;
unsigned int gsm_din_over_alarm_cnt=0;
unsigned int gsm_din_over_all_cnt=0;
unsigned int td_din_under_alarm_cnt=0;
unsigned int td_din_under_all_cnt=0;
unsigned int td_din_over_alarm_cnt=0;
unsigned int td_din_over_all_cnt=0;
unsigned int lte1_din_under_alarm_cnt=0;
unsigned int lte1_din_under_all_cnt=0;
unsigned int lte1_din_over_alarm_cnt=0;
unsigned int lte1_din_over_all_cnt=0;
unsigned int lte2_din_under_alarm_cnt=0;
unsigned int lte2_din_under_all_cnt=0;
unsigned int lte2_din_over_alarm_cnt=0;
unsigned int lte2_din_over_all_cnt=0;
unsigned int ru_dev_temp_alarm_cnt[8] = {0};
unsigned int ru_dev_temp_all_cnt[8] = {0};
unsigned int ru_dout_under_alarm_cnt[8][4] = {0};
unsigned int ru_dout_under_all_cnt[8][4] = {0};
unsigned int ru_dout_over_alarm_cnt[8][4] = {0};
unsigned int ru_dout_over_all_cnt[8][4] = {0};
unsigned int ru_laser_alarm_cnt[8] = {0};
unsigned int ru_laser_all_cnt[8] = {0};
unsigned int ru_rf_alarm_cnt[8] = {0};
unsigned int ru_rf_all_cnt[8] = {0};
unsigned int ru_link_alarm_cnt[8] = {0};
unsigned int ru_link_all_cnt[8] = {0};
int g_stop_alarm = 0;
int is_check_alarm(void)
{
	if(g_DevType == EXPAND_UNIT){
		if(((line_old&0x1) == 1) || ((link_old&0x1) == 1)){
			printf("stop alarm\n\n");
			return 1;
		}else{
			printf("start alarm\n\n");
			return 0;
		}
	}
	return 0;
}
int pw_down = 0;
int pw_error = 0;
unsigned int g_pd_alarm_cnt = 0;
unsigned int g_pd_cnt = 0;
unsigned int g_pe_alarm_cnt = 0;
unsigned int g_pe_cnt = 0;
void common_alarm(void)
{
	int val1;
	int val2;
	int ret, i, j;

	/*
	if(g_DevType == EXPAND_UNIT){
		if(((line_old&0x1) == 1) || ((link_old&0x1) == 1)){
			printf("stop alarm\n\n");
			g_stop_alarm = 1;
		}else{
			printf("start alarm\n\n");
			g_stop_alarm = 0;
		}
	}
	*/
	// ��Դ����
	DbGetThisIntPara(PW_DOWN_EN_ID, &val1); // ʹ��
	if(get_power_down() > 0){  // ʵ��״̬
		pw_down_alarm_cnt++;
		g_pd_alarm_cnt++;
		dprintf("pw_down_alarm_cnt=%d\n", pw_down_alarm_cnt);
	}
	g_pd_cnt++;
	pw_down_all_cnt++;
	dprintf("pw_down_all_cnt=%d\n", pw_down_all_cnt);
	if(g_pd_alarm_cnt >= ALARM_CNT){   // �����澯
		pw_down = 1;
		g_pd_cnt = 0;
		g_pd_alarm_cnt = 0;
	}else if((g_pd_cnt-g_pd_alarm_cnt) > RECOVER_CNT){ // �澯�ָ�
		pw_down = 0;
		g_pd_cnt = 0;
		g_pd_alarm_cnt = 0;
	}
	if((pw_down_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){   // �����澯
		set_alarm(PW_DOWN_ALARM_ID, 1, 1);
		pw_down_alarm_cnt = 0;
		pw_down_all_cnt = 0;
	}else if(((pw_down_all_cnt-pw_down_alarm_cnt) > RECOVER_CNT) || ((val1&0x1 )== 0)){ // �澯�ָ�
		set_alarm(PW_DOWN_ALARM_ID, 0, (val1&0x1));
		pw_down_alarm_cnt = 0;
		pw_down_all_cnt = 0;
	}
	dprintf("pw_down=%d\n", pw_down);

	// ��Դ����
	DbGetThisIntPara(PW_ERROR_EN_ID, &val1); // ʹ��
	DbGetThisIntPara(PW_DOWN_ALARM_ID, &val2); // ��Դ����״̬ 
	if(get_power_error() > 0){
		pw_error_alarm_cnt++;
		g_pe_alarm_cnt++;
		dprintf("pw_error_alarm_cnt=%d\n", pw_error_alarm_cnt);
	}
	g_pe_cnt++;
	pw_error_all_cnt++;
	dprintf("pw_error_all_cnt=%d\n", pw_error_all_cnt);
	if(g_pe_alarm_cnt >= ALARM_CNT){   // �����澯
		pw_error = 1;
		g_pe_cnt = 0;
		g_pe_alarm_cnt = 0;
	}else if((g_pe_cnt-g_pe_alarm_cnt) > RECOVER_CNT){ // �澯�ָ�
		pw_error = 0;
		g_pe_cnt = 0;
		g_pe_alarm_cnt = 0;
	}
	if((pw_error_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)
			&& ((val2&0x1) != 1)){ // ���ι�ϵ����Դ�������ε�Դ����
		set_alarm(PW_ERROR_ALARM_ID, 1, 1);
		pw_error_alarm_cnt = 0;
		pw_error_all_cnt = 0;
	}else if(((pw_error_all_cnt-pw_error_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
		set_alarm(PW_ERROR_ALARM_ID, 0, (val1&0x1));
		pw_error_alarm_cnt = 0;
		pw_error_all_cnt = 0;
	}
	dprintf("pw_error=%d\n", pw_error);
	// ��ع���
	DbGetThisIntPara(BAT_ERROR_EN_ID, &val1); // ʹ��
	if(get_bat_error() > 0){
		bat_error_alarm_cnt++;
		dprintf("bat_error_alarm_cnt=%d\n", bat_error_alarm_cnt);
	}
	bat_error_all_cnt++;
	dprintf("bat_error_all_cnt=%d\n", bat_error_all_cnt);
	if((bat_error_alarm_cnt > ALARM_CNT) && ((val1&0x1) == 1)){
		set_alarm(BAT_ERROR_ALARM_ID, 1, 1);
		bat_error_alarm_cnt = 0;
		bat_error_all_cnt = 0;
	}else if(((bat_error_all_cnt-pw_error_alarm_cnt) > RECOVER_CNT)){ //|| ((val1&0x1) == 0)){ // �澯�ָ�
		bat_error_alarm_cnt = 0;
		bat_error_all_cnt = 0;
	}else if (((val1&0x1) == 0)){
		set_alarm(BAT_ERROR_ALARM_ID, 0, (val1&0x1));
		bat_error_alarm_cnt = 0;
		bat_error_all_cnt = 0;
	}

	//DbGetThisIntPara(PW_DOWN_ALARM_ID, &pw_down); // 
	//DbGetThisIntPara(PW_ERROR_ALARM_ID, &pw_error); // 

	// ����ģ��澯
	DbGetThisIntPara(OTHER_ERROR_EN_ID, &val1); // ʹ��
	if(get_other_error() > 0){
		other_error_alarm_cnt++;
	}
	other_error_all_cnt++;
	if((other_error_alarm_cnt > ALARM_CNT) && ((val1&0x1) == 1)){
		set_alarm(OTHER_ERROR_ALARM_ID, 1, 1);
		other_error_alarm_cnt = 0;
		other_error_all_cnt = 0;
	}else if (((other_error_all_cnt-other_error_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){
		set_alarm(OTHER_ERROR_ALARM_ID, 0, (val1&0x1));
		other_error_alarm_cnt = 0;
		other_error_all_cnt = 0;
	}
	// ����澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			DbGetThisIntPara(LOUNLOCKALEN_ID, &val1); // ʹ��
			if(get_lo_error() > 0){
				lo_alarm_cnt++;
				dprintf("lo_alarm_cnt=%d\n", lo_alarm_cnt);
			}
			lo_all_cnt++;
			dprintf("lo_all_cnt=%d\n", lo_all_cnt);
			if((lo_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				set_alarm(LOUNLOCKAL_ID, 1, 1);
				lo_alarm_cnt = 0;
				lo_all_cnt = 0;
			}else if(((lo_all_cnt-lo_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				set_alarm(LOUNLOCKAL_ID, 0, (val1&0x1));
				lo_alarm_cnt = 0;
				lo_all_cnt = 0;
			}
		}else{
			DbGetThisIntPara(LOUNLOCKAL_ID, &val1);
			if(val1 == 128){ 
				DbSaveThisIntPara_MCP_C(LOUNLOCKAL_ID, 129, 0);
			}else if(val1 == 1){
				set_alarm(LOUNLOCKAL_ID, 0, 0);
			}
		}
	}
	// ���Ӽ����·�澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			DbGetThisIntPara(MAIN_LINE_EN_ID, &val1); // ʹ��
			if(get_link_error() > 0){
				link_alarm_cnt++;
				dprintf("link_alarm_cnt=%d\n", link_alarm_cnt);
			}
			link_all_cnt++;
			dprintf("link_all_cnt=%d\n", link_all_cnt);
			if((link_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				set_alarm(MAIN_LINE_ALARM_ID, 1, 1);
				link_alarm_cnt = 0;
				link_all_cnt = 0;
			}else if(((link_all_cnt-link_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				set_alarm(MAIN_LINE_ALARM_ID, 0, (val1&0x1));
				link_alarm_cnt = 0;
				link_all_cnt = 0;
			}
		}else{
			//set_alarm(MAIN_LINE_ALARM_ID, 0, 0);
			DbGetThisIntPara(MAIN_LINE_ALARM_ID, &val1);
			if(val1 == 128){ 
				DbSaveThisIntPara_MCP_C(MAIN_LINE_ALARM_ID, 129, 0);
			}else if(val1 == 1){
				set_alarm(MAIN_LINE_ALARM_ID, 0, 0);
			}
		}
	}
	if((get_net_group()&0xf00) != 0x300){
		// TD-SCDMAʧ���澯
		if(g_DevType == MAIN_UNIT){
			if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
				DbGetThisIntPara(TD_ASYNC_EN_ID, &val1); // ʹ��
				if(get_td_async_error() > 0){
					td_async_alarm_cnt++;
				}
				td_async_all_cnt++;
				if((td_async_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
					set_alarm(TD_ASYNC_ALARM_ID, 1, 1);
					td_async_alarm_cnt = 0;
					td_async_all_cnt = 0;
				}else if(((td_async_all_cnt-td_async_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
					set_alarm(TD_ASYNC_ALARM_ID, 0, (val1&0x1));
					td_async_alarm_cnt = 0;
					td_async_all_cnt = 0;
				}
			}else{
				//set_alarm(TD_ASYNC_ALARM_ID, 0, 0);
				DbGetThisIntPara(TD_ASYNC_ALARM_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(TD_ASYNC_ALARM_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(TD_ASYNC_ALARM_ID, 0, 0);
				}
			}
		}
		// LTEʧ���澯
		if(g_DevType == MAIN_UNIT){
			if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
				DbGetThisIntPara(TD_ASYNC_EN_ID, &val1); // ʹ��
				DbGetThisIntPara(LTE_ASYNC_EN_ID, &val1); // ʹ��
				if(get_lte_async_error() > 0){
					lte_async_alarm_cnt++;
				}
				lte_async_all_cnt++;
				if((lte_async_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
					set_alarm(LTE_ASYNC_ALARM_ID, 1, 1);
					lte_async_alarm_cnt = 0;
					lte_async_all_cnt = 0;
				}else if(((lte_async_all_cnt-lte_async_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
					set_alarm(LTE_ASYNC_ALARM_ID, 0, (val1&0x1));
					lte_async_alarm_cnt = 0;
					lte_async_all_cnt = 0;
				}
			}else{
				//set_alarm(LTE_ASYNC_ALARM_ID, 0, 0);
				DbGetThisIntPara(LTE_ASYNC_ALARM_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE_ASYNC_ALARM_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE_ASYNC_ALARM_ID, 0, 0);
				}
			}
		}
	}
	// ���շ�ģ�����1~8
	ret = get_line_error();
	for(i = 0; i < 8; i++){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			DbGetThisIntPara(LASER_MODULE_EN_ID+i, &val1); // ʹ��
			if(((ret>>i)&0x1) > 0){
				line_alarm_cnt[i]++;
				dprintf("line_alarm_cnt[%d]=%d\n", i, line_alarm_cnt[i]);
			}
			line_all_cnt[i]++;
			dprintf("line_all_cnt[%d]=%d\n", i, line_all_cnt[i]);
			if((line_alarm_cnt[i] >= ALARM_CNT) && ((val1&0x1) == 1)){
				set_alarm(LASER_MODULE_ALARM_ID+i, 1, 1);
				line_alarm_cnt[i] = 0;
				line_all_cnt[i] = 0;
			}else if(((line_all_cnt[i] - line_alarm_cnt[i]) > RECOVER_CNT) || ((val1&0x1) == 0)){
				set_alarm(LASER_MODULE_ALARM_ID+i, 0, (val1&0x1));
				line_alarm_cnt[i] = 0;
				line_all_cnt[i] = 0;
			}
		}else{
			//set_alarm(LASER_MODULE_ALARM_ID, 0, 0);
			DbGetThisIntPara(LASER_MODULE_ALARM_ID+i, &val1);
			if(val1 == 128){ 
				DbSaveThisIntPara_MCP_C(LASER_MODULE_ALARM_ID+i, 129, 0);
			}else if(val1 == 1){
				set_alarm(LASER_MODULE_ALARM_ID+i, 0, 0);
			}
		}
	}
	// �豸���¸澯
	if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
		DbGetThisIntPara(DEV_TEMP_EN_ID, &val1); // ʹ��
		if(get_dev_temp_error() > 0){
			dev_temp_alarm_cnt++;
			dprintf("dev_temp_alarm_cnt=%d\n", dev_temp_alarm_cnt);
		}
		dev_temp_all_cnt++;
		dprintf("dev_temp_all_cnt=%d\n", dev_temp_all_cnt);
		if((dev_temp_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
			set_alarm(DEV_TEMP_ALARM_ID, 1, 1);
			dev_temp_alarm_cnt = 0;
			dev_temp_all_cnt = 0;
		}else if(((dev_temp_all_cnt-dev_temp_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
			set_alarm(DEV_TEMP_ALARM_ID, 0, (val1&0x1));
			dev_temp_alarm_cnt = 0;
			dev_temp_all_cnt = 0;
		}
	}else{
		//set_alarm(DEV_TEMP_ALARM_ID, 0, 0);
		DbGetThisIntPara(DEV_TEMP_ALARM_ID, &val1);
		if(val1 == 128){ 
			DbSaveThisIntPara_MCP_C(DEV_TEMP_ALARM_ID, 129, 0);
		}else if(val1 == 1){
			set_alarm(DEV_TEMP_ALARM_ID, 0, 0);
		}
	}
	// gsm ��������Ƿ���ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(GSM_DIN_UNDER_EN_ID+mode_idx[0], &val1); // ʹ��
			}else{
				DbGetThisIntPara(GSM_DIN_UNDER_EN_ID, &val1); // ʹ��
			}
			if(get_gsm_din_under_error() > 0){
				gsm_din_under_alarm_cnt++;
				dprintf("gsm_din_under_alarm_cnt=%d\n", gsm_din_under_alarm_cnt);
			}
			gsm_din_under_all_cnt++;
			dprintf("gsm_din_under_all_cnt=%d\n", gsm_din_under_all_cnt);
			if((gsm_din_under_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(GSM_DIN_UNDER_ID+mode_idx[0], 1, 1);
				}else{
					set_alarm(GSM_DIN_UNDER_ID, 1, 1);
				}
				gsm_din_under_alarm_cnt = 0;
				gsm_din_under_all_cnt = 0;
			}else if(((gsm_din_under_all_cnt-gsm_din_under_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(GSM_DIN_UNDER_ID+mode_idx[0], 0, (val1&0x1));
				}else{
					set_alarm(GSM_DIN_UNDER_ID, 0, (val1&0x1));
				}
				gsm_din_under_alarm_cnt = 0;
				gsm_din_under_all_cnt = 0;
			}
		}else{
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(GSM_DIN_UNDER_ID+mode_idx[0], &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(GSM_DIN_UNDER_ID+mode_idx[0], 129, 0);
				}else if(val1 == 1){
					set_alarm(GSM_DIN_UNDER_ID+mode_idx[0], 0, 0);
				}
			}else{
				DbGetThisIntPara(GSM_DIN_UNDER_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(GSM_DIN_UNDER_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(GSM_DIN_UNDER_ID, 0, 0);
				}
			}
		}
	}
	// td ��������Ƿ���ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			DbGetThisIntPara(TD_DIN_UNDER_EN_ID, &val1); // ʹ��
			if(get_td_din_under_error() > 0){
				td_din_under_alarm_cnt++;
				dprintf("td_din_under_alarm_cnt=%d\n", td_din_under_alarm_cnt);
			}
			td_din_under_all_cnt++;
			dprintf("td_din_under_all_cnt=%d\n", td_din_under_all_cnt);
			if((td_din_under_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				set_alarm(TD_DIN_UNDER_ID, 1, 1);
				td_din_under_alarm_cnt = 0;
				td_din_under_all_cnt = 0;
			}else if(((td_din_under_all_cnt-td_din_under_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				set_alarm(TD_DIN_UNDER_ID, 0, (val1&0x1));
				td_din_under_alarm_cnt = 0;
				td_din_under_all_cnt = 0;
			}
		}else{
			//set_alarm(TD_DIN_UNDER_ID, 0, 0);
			DbGetThisIntPara(TD_DIN_UNDER_ID, &val1);
			if(val1 == 128){ 
				DbSaveThisIntPara_MCP_C(TD_DIN_UNDER_ID, 129, 0);
			}else if(val1 == 1){
				set_alarm(TD_DIN_UNDER_ID, 0, 0);
			}
		}
	}
	// lte1 ��������Ƿ���ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE1_DIN_UNDER_EN_ID-0x2000, &val1); // ʹ��
			}else{
				DbGetThisIntPara(LTE1_DIN_UNDER_EN_ID, &val1); // ʹ��
			}
			if(get_lte1_din_under_error() > 0){
				lte1_din_under_alarm_cnt++;
				dprintf("lte1_din_under_alarm_cnt=%d\n", lte1_din_under_alarm_cnt);
			}
			lte1_din_under_all_cnt++;
			dprintf("lte1_din_under_all_cnt=%d\n", lte1_din_under_all_cnt);
			if((lte1_din_under_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE1_DIN_UNDER_ID-0x2000, 1, 1);
				}else{
					set_alarm(LTE1_DIN_UNDER_ID, 1, 1);
				}
				lte1_din_under_alarm_cnt = 0;
				lte1_din_under_all_cnt = 0;
			}else if(((lte1_din_under_all_cnt-lte1_din_under_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE1_DIN_UNDER_ID-0x2000, 0, (val1&0x1));
				}else{
					set_alarm(LTE1_DIN_UNDER_ID, 0, (val1&0x1));
				}
				lte1_din_under_alarm_cnt = 0;
				lte1_din_under_all_cnt = 0;
			}
		}else{
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE1_DIN_UNDER_ID-0x2000, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE1_DIN_UNDER_ID-0x2000, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE1_DIN_UNDER_ID-0x2000, 0, 0);
				}
			}else{
				DbGetThisIntPara(LTE1_DIN_UNDER_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE1_DIN_UNDER_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE1_DIN_UNDER_ID, 0, 0);
				}
			}
		}
	}
	// lte2 ��������Ƿ���ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE2_DIN_UNDER_EN_ID-0x2000, &val1); // ʹ��
			}else{
				DbGetThisIntPara(LTE2_DIN_UNDER_EN_ID, &val1); // ʹ��
			}
			if(get_lte2_din_under_error() > 0){
				lte2_din_under_alarm_cnt++;
				dprintf("lte2_din_under_alarm_cnt=%d\n", lte2_din_under_alarm_cnt);
			}
			lte2_din_under_all_cnt++;
			dprintf("lte2_din_under_all_cnt=%d\n", lte2_din_under_all_cnt);
			if((lte2_din_under_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE2_DIN_UNDER_ID-0x2000, 1, 1);
				}else{
					set_alarm(LTE2_DIN_UNDER_ID, 1, 1);
				}
				lte2_din_under_alarm_cnt = 0;
				lte2_din_under_all_cnt = 0;
			}else if(((lte2_din_under_all_cnt-lte2_din_under_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE2_DIN_UNDER_ID-0x2000, 0, (val1&0x1));
				}else{
					set_alarm(LTE2_DIN_UNDER_ID, 0, (val1&0x1));
				}
				lte2_din_under_alarm_cnt = 0;
				lte2_din_under_all_cnt = 0;
			}
		}else{
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE2_DIN_UNDER_ID-0x2000, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE2_DIN_UNDER_ID-0x2000, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE2_DIN_UNDER_ID-0x2000, 0, 0);
				}
			}else{
				DbGetThisIntPara(LTE2_DIN_UNDER_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE2_DIN_UNDER_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE2_DIN_UNDER_ID, 0, 0);
				}
			}
		}
	}
	// gsm ������������ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(GSM_DIN_OVER_EN_ID+0x2000, &val1); // ʹ��
			}else{
				DbGetThisIntPara(GSM_DIN_OVER_EN_ID, &val1); // ʹ��
			}
			if(get_gsm_din_over_error() > 0){
				gsm_din_over_alarm_cnt++;
				dprintf("gsm_din_over_alarm_cnt=%d\n", gsm_din_over_alarm_cnt);
			}
			gsm_din_over_all_cnt++;
			dprintf("gsm_din_over_all_cnt=%d\n", gsm_din_over_all_cnt);
			if((gsm_din_over_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(GSM_DIN_OVER_ID+0x2000, 1, 1);
				}else{
					set_alarm(GSM_DIN_OVER_ID, 1, 1);
				}
				gsm_din_over_alarm_cnt = 0;
				gsm_din_over_all_cnt = 0;
			}else if(((gsm_din_over_all_cnt-gsm_din_over_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(GSM_DIN_OVER_ID+0x2000, 0, (val1&0x1));
				}else{
					set_alarm(GSM_DIN_OVER_ID, 0, (val1&0x1));
				}
				gsm_din_over_alarm_cnt = 0;
				gsm_din_over_all_cnt = 0;
			}
		}else{
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(GSM_DIN_OVER_ID+0x2000, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(GSM_DIN_OVER_ID+0x2000, 129, 0);
				}else if(val1 == 1){
					set_alarm(GSM_DIN_OVER_ID+0x2000, 0, 0);
				}
			}else{
				DbGetThisIntPara(GSM_DIN_OVER_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(GSM_DIN_OVER_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(GSM_DIN_OVER_ID, 0, 0);
				}
			}
		}
	}
	// td ������������ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			DbGetThisIntPara(TD_DIN_OVER_EN_ID, &val1); // ʹ��
			if(get_td_din_over_error() > 0){
				td_din_over_alarm_cnt++;
				dprintf("td_din_over_alarm_cnt=%d\n", td_din_over_alarm_cnt);
			}
			td_din_over_all_cnt++;
			dprintf("td_din_over_all_cnt=%d\n", td_din_over_all_cnt);
			if((td_din_over_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				set_alarm(TD_DIN_OVER_ID, 1, 1);
				td_din_over_alarm_cnt = 0;
				td_din_over_all_cnt = 0;
			}else if(((td_din_over_all_cnt-td_din_over_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				set_alarm(TD_DIN_OVER_ID, 0, (val1&0x1));
				td_din_over_alarm_cnt = 0;
				td_din_over_all_cnt = 0;
			}
		}else{
			//set_alarm(TD_DIN_OVER_ID, 0, 0);
			DbGetThisIntPara(TD_DIN_OVER_ID, &val1);
			if(val1 == 128){ 
				DbSaveThisIntPara_MCP_C(TD_DIN_OVER_ID, 129, 0);
			}else if(val1 == 1){
				set_alarm(TD_DIN_OVER_ID, 0, 0);
			}
		}
	}
	// lte1 ������������ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE1_DIN_OVER_EN_ID-0x2000, &val1); // ʹ��
			}else{
				DbGetThisIntPara(LTE1_DIN_OVER_EN_ID-0x2000, &val1); // ʹ��
			}
			if(get_lte1_din_over_error() > 0){
				lte1_din_over_alarm_cnt++;
				dprintf("lte1_din_over_alarm_cnt=%d\n", lte1_din_over_alarm_cnt);
			}
			lte1_din_over_all_cnt++;
			dprintf("lte1_din_over_all_cnt=%d\n", lte1_din_over_all_cnt);
			if((lte1_din_over_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE1_DIN_OVER_ID-0x2000, 1, 1);
				}else{
					set_alarm(LTE1_DIN_OVER_ID, 1, 1);
				}
				lte1_din_over_alarm_cnt = 0;
				lte1_din_over_all_cnt = 0;
			}else if(((lte1_din_over_all_cnt-lte1_din_over_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE1_DIN_OVER_ID-0x2000, 0, (val1&0x1));
				}else{
					set_alarm(LTE1_DIN_OVER_ID, 0, (val1&0x1));
				}
				lte1_din_over_alarm_cnt = 0;
				lte1_din_over_all_cnt = 0;
			}
		}else{
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE1_DIN_OVER_ID-0x2000, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE1_DIN_OVER_ID-0x2000, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE1_DIN_OVER_ID-0x2000, 0, 0);
				}
			}else{
				DbGetThisIntPara(LTE1_DIN_OVER_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE1_DIN_OVER_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE1_DIN_OVER_ID, 0, 0);
				}
			}
		}
	}
    // lte2 ������������ʸ澯
	if(g_DevType == MAIN_UNIT){
		if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE2_DIN_OVER_EN_ID-0x2000, &val1); // ʹ��
			}else{
				DbGetThisIntPara(LTE2_DIN_OVER_EN_ID, &val1); // ʹ��
			}
			if(get_lte2_din_over_error() > 0){
				lte2_din_over_alarm_cnt++;
				dprintf("lte2_din_over_alarm_cnt=%d\n", lte2_din_over_alarm_cnt);
			}
			lte2_din_over_all_cnt++;
			dprintf("lte2_din_over_all_cnt=%d\n", lte2_din_over_all_cnt);
			if((lte2_din_over_alarm_cnt >= ALARM_CNT) && ((val1&0x1) == 1)){
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE2_DIN_OVER_ID-0x2000, 1, 1);
				}else{
					set_alarm(LTE2_DIN_OVER_ID, 1, 1);
				}
				lte2_din_over_alarm_cnt = 0;
				lte2_din_over_all_cnt = 0;
			}else if(((lte2_din_over_all_cnt-lte2_din_over_alarm_cnt) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				if((get_net_group()&0xf00) == 0x300){
					set_alarm(LTE2_DIN_OVER_ID-0x2000, 0, (val1&0x1));
				}else{
					set_alarm(LTE2_DIN_OVER_ID, 0, (val1&0x1));
				}
				lte2_din_over_alarm_cnt = 0;
				lte2_din_over_all_cnt = 0;
			}
		}else{
			if((get_net_group()&0xf00) == 0x300){
				DbGetThisIntPara(LTE2_DIN_OVER_ID-0x2000, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE2_DIN_OVER_ID-0x2000, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE2_DIN_OVER_ID-0x2000, 0, 0);
				}
			}else{
				DbGetThisIntPara(LTE2_DIN_OVER_ID, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(LTE2_DIN_OVER_ID, 129, 0);
				}else if(val1 == 1){
					set_alarm(LTE2_DIN_OVER_ID, 0, 0);
				}
			}
		}
	}
	// Զ����Ƶ��·���ϸ澯
	if(g_DevType == EXPAND_UNIT){
		for(i = 0; i < 8; i++){
			if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
				//DbGetThisIntPara(RU_RF_EN_ID+i, &val1); // ʹ��
				val1 = g_alarm_en.ru_rf_en[i];
				if(get_ru_rf_error(i) > 0){
					ru_rf_alarm_cnt[i]++;
					dprintf("ru_rf_alarm_cnt[%d]=%d\n", i, ru_rf_alarm_cnt[i]);
				}
				ru_rf_all_cnt[i]++;
				dprintf("ru_rf_all_cnt[%d]=%d\n", i, ru_rf_all_cnt[i]);
				if((ru_rf_alarm_cnt[i] >= ALARM_CNT) && ((val1&0x1) == 1)){
					set_alarm(RU_RF_ALARM_ID+i, 1, 1);
					ru_rf_alarm_cnt[i] = 0;
					ru_rf_all_cnt[i] = 0;
				}else if(((ru_rf_all_cnt[i]-ru_rf_alarm_cnt[i]) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
					set_alarm(RU_RF_ALARM_ID+i, 0, (val1&0x1));
					ru_rf_alarm_cnt[i] = 0;
					ru_rf_all_cnt[i] = 0;
				}
			}else{
				DbGetThisIntPara(RU_RF_ALARM_ID+i, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(RU_RF_ALARM_ID+i, 129, 0);
				}else if(val1 == 1){
					set_alarm(RU_RF_ALARM_ID+i, 0, 0);
				}
			}
		}
	}
	// Զ���豸���¸澯
	if(g_DevType == EXPAND_UNIT){
		for(i = 0; i < 8; i++){
			if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
				//DbGetThisIntPara(RU_DEV_TEMP_EN_ID+i, &val1); // ʹ��
				val1 = g_alarm_en.ru_dev_temp_en[i];
				if(get_ru_dev_temp_error(i) > 0){
					ru_dev_temp_alarm_cnt[i]++;
					dprintf("ru_dev_temp_alarm_cnt[%d]=%d\n", i, ru_dev_temp_alarm_cnt[i]);
				}
				ru_dev_temp_all_cnt[i]++;
				dprintf("ru_dev_temp_all_cnt[%d]=%d\n", i, ru_dev_temp_all_cnt[i]);
				if((ru_dev_temp_alarm_cnt[i] >= ALARM_CNT) && ((val1&0x1) == 1)){
					set_alarm(RU_DEV_TEMP_ALARM_ID+i, 1, 1);
					ru_dev_temp_alarm_cnt[i] = 0;
					ru_dev_temp_all_cnt[i] = 0;
				}else if(((ru_dev_temp_all_cnt[i]-ru_dev_temp_alarm_cnt[i]) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
					set_alarm(RU_DEV_TEMP_ALARM_ID+i, 0, (val1&0x1));
					ru_dev_temp_alarm_cnt[i] = 0;
					ru_dev_temp_all_cnt[i] = 0;
				}
			}else{
				DbGetThisIntPara(RU_DEV_TEMP_ALARM_ID+i, &val1);
				if(val1 == 128){ 
					DbSaveThisIntPara_MCP_C(RU_DEV_TEMP_ALARM_ID+i, 129, 0);
				}else if(val1 == 1){
					set_alarm(RU_DEV_TEMP_ALARM_ID+i, 0, 0);
				}
			}
		}
	}

	// Զ��Ƿ���ʸ澯
	if(g_DevType == EXPAND_UNIT){
		//for(i = 0; i < 4; i++){
			//DbGetThisIntPara(RU_DOUT_UNDER_THD+mode_idx[i], &ru_dout_under_thd[i]); //
		//}
		for(i = 0; i < 8; i++){ // Զ��1��8
			DbGetThisIntPara(RU_RF_ALARM_ID+i, &val2); //  ��Ƶ��·���ϸ澯
			for(j = 0; j < 4; j++){ // 4ͨ��
				DbGetThisIntPara(RU_RF_SW_ID+i+mode_idx[j], &val1); // ��Ƶ���� 
				if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1) && ((val1&0x1) == 1) // ��Դ/��Ƶ����
						&& ((g_din_under_alarm[j]&0x1) != 1) && ((g_lo_alarm&0x1) != 1) // ��������Ƿ����/����ʧ��/
						&& ((val2&0x1) != 1) && ((g_power_down_alarm&0x1) != 1) && ((g_power_error_alarm&0x1) != 1)){ // au�澯/��·����
					//DbGetThisIntPara(RU_DOUT_UNDER_EN_ID+i+mode_idx[j], &val1); // ʹ��
					val1 = g_alarm_en.ru_dout_under_en[i][j];
					dprintf("val1_en=%d\n", val1);
					if(get_ru_dout_under_error(i, mode_idx[j]) > 0){
						ru_dout_under_alarm_cnt[i][j]++;
						dprintf("ru_dout_under_alarm_cnt[%d][%d]=%d\n", i, j, ru_dout_under_alarm_cnt[i][j]);
					}
					ru_dout_under_all_cnt[i][j]++;
					dprintf("ru_dout_under_all_cnt[%d][%d]=%d\n", i, j, ru_dout_under_all_cnt[i][j]);
					if((ru_dout_under_alarm_cnt[i][j] >= ALARM_CNT) && ((val1&0x1) == 1)){
						dprintf("alarm[%d][%d] !!!\n", i, j);
						set_alarm(RU_DOUT_UNDER_ALARM_ID+i+mode_idx[j], 1, 1);
						ru_dout_under_alarm_cnt[i][j] = 0;
						ru_dout_under_all_cnt[i][j] = 0;
					}else if(((ru_dout_under_all_cnt[i][j]-ru_dout_under_alarm_cnt[i][j]) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
						dprintf("recover alarm[%d][%d] !!!\n", i, j);
						set_alarm(RU_DOUT_UNDER_ALARM_ID+i+mode_idx[j], 0, (val1&0x1));
						ru_dout_under_alarm_cnt[i][j] = 0;
						ru_dout_under_all_cnt[i][j] = 0;
					}
				}else{
					DbGetThisIntPara(RU_DOUT_UNDER_ALARM_ID+i+mode_idx[j], &val1);
					if(val1 == 128){ 
						DbSaveThisIntPara_MCP_C(RU_DOUT_UNDER_ALARM_ID+i+mode_idx[j], 129, 0);
					}else if(val1 == 1){
						set_alarm(RU_DOUT_UNDER_ALARM_ID+i+mode_idx[j], 0, 0);
					}
				}
			}
		}
	}
	// Զ�˹����ʸ澯
	if(g_DevType == EXPAND_UNIT){
		//for(i = 0; i < 4; i++){
			//DbGetThisIntPara(RU_DOUT_OVER_THD+mode_idx[i], &ru_dout_over_thd[i]); //
		//}
		for(i = 0; i < 8; i++){ // Զ��1��8
			for(j = 0; j < 4; j++){ // 4ͨ��
				if(((pw_down&0x1) != 1) && ((pw_error&0x1) != 1)){
					//DbGetThisIntPara(RU_DOUT_OVER_EN_ID+i+mode_idx[j], &val1); // ʹ��
					val1 = g_alarm_en.ru_dout_over_en[i][j];
					if(get_ru_dout_over_error(i, mode_idx[j]) > 0){
						ru_dout_over_alarm_cnt[i][j]++;
						dprintf("ru_dout_over_alarm_cnt[%d][%d]=%d\n", i, j, ru_dout_over_alarm_cnt[i][j]);
					}
					ru_dout_over_all_cnt[i][j]++;
					dprintf("ru_dout_over_all_cnt[%d][%d]=%d\n", i, j, ru_dout_over_all_cnt[i][j]);
					if((ru_dout_over_alarm_cnt[i][j] >= ALARM_CNT) && ((val1&0x1) == 1)){
						set_alarm(RU_DOUT_OVER_ALARM_ID+i+mode_idx[j], 1, 1);
						ru_dout_over_alarm_cnt[i][j] = 0;
						ru_dout_over_all_cnt[i][j] = 0;
					}else if(((ru_dout_over_all_cnt[i][j]-ru_dout_over_alarm_cnt[i][j]) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
						set_alarm(RU_DOUT_OVER_ALARM_ID+i+mode_idx[j], 0, (val1&0x1));
						ru_dout_over_alarm_cnt[i][j] = 0;
						ru_dout_over_all_cnt[i][j] = 0;
					}
				}else{
					DbGetThisIntPara(RU_DOUT_OVER_ALARM_ID+i+mode_idx[j], &val1);
					if(val1 == 128){ 
						DbSaveThisIntPara_MCP_C(RU_DOUT_OVER_ALARM_ID+i+mode_idx[j], 129, 0);
					}else if(val1 == 1){
						set_alarm(RU_DOUT_OVER_ALARM_ID+i+mode_idx[j], 0, 0);
					}
				}
			}
		}
	}

	// Զ�˹��շ�ģ����ϸ澯
	if(g_DevType == EXPAND_UNIT){
		for(i = 0; i < 8; i++){
			//DbGetThisIntPara(RU_LASER_EN_ID+i, &val1); // ʹ��
			val1 = g_alarm_en.ru_laser_en[i];
			if(get_ru_laser_error(i) > 0){
				ru_laser_alarm_cnt[i]++;
				dprintf("ru_laser_alarm_cnt[%d]=%d\n", i, ru_laser_alarm_cnt[i]);
			}
			ru_laser_all_cnt[i]++;
			dprintf("ru_laser_all_cnt[%d]=%d\n", i, ru_laser_all_cnt[i]);
			if((ru_laser_alarm_cnt[i] >= ALARM_CNT) && ((val1&0x1) == 1)){
				set_alarm(RU_LASER_ALARM_ID+i, 1, 1);
				ru_laser_alarm_cnt[i] = 0;
				ru_laser_all_cnt[i] = 0;
			}else if(((ru_laser_all_cnt[i]-ru_laser_alarm_cnt[i]) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				set_alarm(RU_LASER_ALARM_ID+i, 0, (val1&0x1));
				ru_laser_alarm_cnt[i] = 0;
				ru_laser_all_cnt[i] = 0;
			}
		}
	}
	
	// Զ����·���ϸ澯
	if(g_DevType == EXPAND_UNIT){
		for(i = 0; i < 8; i++){
			//DbGetThisIntPara(RU_LINK_EN_ID+i, &val1); // ʹ��
			val1 = g_alarm_en.ru_link_en[i];
			if(get_ru_link_error(i) > 0){
				ru_link_alarm_cnt[i]++;
				dprintf("ru_link_alarm_cnt[%d]=%d\n", i, ru_link_alarm_cnt[i]);
			}
			ru_link_all_cnt[i]++;
			dprintf("ru_link_all_cnt[%d]=%d\n", i, ru_link_all_cnt[i]);
			if((ru_link_alarm_cnt[i] >= ALARM_CNT) && ((val1&0x1) == 1)){
				set_alarm(RU_LINK_ALARM_ID+i, 1, 1);
				ru_link_alarm_cnt[i] = 0;
				ru_link_all_cnt[i] = 0;
			}else if(((ru_link_all_cnt[i]-ru_link_alarm_cnt[i]) > RECOVER_CNT) || ((val1&0x1) == 0)){ // �澯�ָ�
				set_alarm(RU_LINK_ALARM_ID+i, 0, (val1&0x1));
				ru_link_alarm_cnt[i] = 0;
				ru_link_all_cnt[i] = 0;
			}
		}
	}
}
int set_alarm_change(int argc, char * argv[])
{
	unsigned int para1;
	unsigned int para2;
	unsigned int para3;
	int i = 0;
	unsigned char dataa;

	msg_tmp.mtype = MSG_FUN_RESULT;	
	if(argc != 4){
		printf("input para cnt is not 3.\r\n");
		sprintf(msg_tmp.mtext, "input para cnt is not 3.\r\n");
		msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
		return 1;
	}
	para1 = strtol(argv[1], NULL, 16);
	para2 = strtol(argv[2], NULL, 16);
	para3 = strtol(argv[3], NULL, 16);
	for( i = 0; i < 12; i++){
		if(para1 == ms_alarm[i].id){
			ms_alarm[i].val = para2;
			ms_alarm[i].pro = para3;
		}
	}
	dataa = (unsigned char)para2;
	sprintf(msg_tmp.mtext, "0x%08X: val=%d, pro=%d.\r\n", para1, para2, para3);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}
int alarm_debug_change(int argc, char * argv[])
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
	alarm_debug = para1;
	sprintf(msg_tmp.mtext, "alarm_debug=%d.\r\n", para1);
	msg_send(TASK2_MODULE, (char *)&msg_tmp, strlen(msg_tmp.mtext));
	return 0;
}


