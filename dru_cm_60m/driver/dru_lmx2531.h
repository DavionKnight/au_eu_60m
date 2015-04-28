/*******************************************************************************
********************************************************************************
* �ļ�����:  dru_lmx2531.h
* ��������:  �й���lmx2531�����ò����ĺ궨�弰��������
* ʹ��˵��: 	 ���к�lmx2531ֱ����صĶ��嶼������ļ��ж���
* �ļ�����:	H4
* ��д����: ��2012/06/18��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#ifndef _DRU_LMX2531_H
#define _DRU_LMX2531_H

#define SELECT_LMX2531 3
#define LMX2531_REG_LENGTH 24

#define LQ1146E_6144M 655 
#define LQ1226E_6144M 655 
#define LQ1312E_6144M 578 
#define LQ1415E_6144M 578 
#define LQ1515E_6144M 492
#define LQ1570E_6144M 468
#define LQ1650E_6144M 468
#define LQ1700E_6144M 427
#define LQ1742E_6144M 393
#define LQ1778E_6144M 378
#define LQ1910E_6144M 328
#define LQ2265E_6144M 246
#define LQ2080E_6144M 218
#define LQ2570E_6144M 164
#define LQ2820E_6144M 140
#define LQ3010E_6144M 123 


int dru_lmx2531_init(unsigned int);
int dru_lmx2531_gsm_init();
int dru_lmx2531_gsm1_init();
int dru_lmx2531_td_init();
int dru_lmx2531_lte1_init();
int dru_lmx2531_lte2_init();
int dru_lmx2531_lte1_config(unsigned int freq);
int dru_lmx2531_lte2_config(unsigned int freq);
int dru_lmx2531_fdd_lte1_config(unsigned int freq);
int dru_lmx2531_fdd_lte2_config(unsigned int freq);
int dru_lmx2531_fdd_wcdma_config(unsigned int freq);
int dru_lmx2531_fdd_lte2_init();
int dru_lmx2531_fdd_lte1_init();
int dru_lmx2531_wcdma_init();
int dru_lmx2531_config(unsigned int freq,unsigned int sel,unsigned int div,unsigned int  nu);


#endif
