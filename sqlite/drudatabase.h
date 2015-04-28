/********************  COPYRIGHT(C) ***************************************
**                               ��������ͨ�����޹�˾
**                                     ���߲�Ʒ�з���
**
**                                 http:// www.aceway.com.cn
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: drudatabase.h
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: ���ݿ⴦�����ͷ�ļ�
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
#ifndef _DRUDATABASE_H
#define _DRUDATABASE_H

#include "../common/druheader.h"
#include "sqliteops.h"

#define TBL_ID          0
#define TBL_NAME        1
#define TBL_MODE        2
#define TBL_VARTYPE     3
#define TBL_COEF        4
#define TBL_LEN         5
#define TBL_VAR         6
#define TBL_MIN         7
#define TBL_MAX         8
#define TBL_AUTHORITY		9

#define TBL_NAME_SIZE   50
#define TBL_VAR_SIZE    20
#define SQL_CMD_SIZE   1024 

#define IDLISTDB      "/flashDev/program/config/Sq3IdList.txt"
#define SYSCONFIGFILE "/flashDev/program/config/sysconfig.txt"

#define DRURMUSAVEDB  "/flashDev/program/config/drurmu.db"
#define DRUREUSAVEDB  "/flashDev/program/config/drureu.db"
#define DRURAUSAVEDB  "/flashDev/program/config/drurau.db"

#define DRURMUDB      "/ramDisk/drurmu.db"
#define DRUREUDB      "/ramDisk/drureu.db"
#define DRURAUDB      "/ramDisk/drurau.db"
//(ID,name,mode,var_type,coefficient,len,V0,min integer,max integer,pro integer);
//���ݿ������ļ��б�ͷ����var_min,var_max
//#define TABLE_HEAD    "ID,name,mode,var_type,coefficient,len,var_min,var_max"//yuht��2014.5.22
#define TABLE_HEAD    "ID,name,mode,var_type,coefficient,len"
#define CLIENTTBL_FD          0
#define CLIENTTBL_DEVNO       1
#define CLIENTTBL_MAC        	2
#define CLIENTTBL_IPADDR     	3
#define CLIENTTBL_DEVTYPE    	4
#define CLIENTTBL_LASTCOMTIME	5
#define CLIENTTBL_HEAD "(Fd,DevNo,Mac,IP,DevType,LastComTime)"

#define RMU_TBL     "rmu"
#define REU_TBL     "reu"
#define RAU_TBL     "rau"
#define CLIENT_TBL  "client_info"

int DataBaseInit(void);
int DbAddVarName(DevInfo_t *pdevinfo);
int DbGetIDList(DevInfo_t *pdevinfo, int *pbuf);
int DbGetParaValue(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu);
int DbGetTblName(DevInfo_t *pdevinfo, char *tblname);
int DbGetTblVarName(DevInfo_t *pdevinfo, char *tblname, char *varname);
int DbGetVarName(DevInfo_t *pdevinfo, char *varname);
int DbSaveParaValue(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu);
int DbGetThisIntPara(unsigned int objectid, int *pval);
int DbSaveThisIntPara(unsigned int objectid, int val);
int DbSaveThisStrPara(unsigned int objectid, char *buf);
int DbGetThisStrPara(unsigned int objectid, char *buf);
int DbTableInit(char *tblname);
int LoadDevicePara(DevInfo_t *pdevinfo, DevicePara_t *p_dev);
void SqlResultDis(SqlResult_t *psqlres);
int DbGetIDList_MCP_C(DevInfo_t *pdevinfo, int *pbuf);
int DbGetParaValue_MCP_C(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu);
int DbSaveParaValue_MCP_C(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu, int flag);
int DbSaveThisIntPara_MCP_C(unsigned int objectid, int val, int flag);
int DbGetAlarmValue(DevInfo_t *pdevinfo, char *pbuf);
int SqliteGetAlarmCnt(int port_num);
int SqliteGetReadCnt(DevInfo_t *pdevinfo);
int SqliteGetWriteCnt(DevInfo_t *pdevinfo);
int SqliteGetInitPara(DevInfo_t *pdevinfo, void * buf, int type);
int SqliteGetBPara(DevInfo_t *pdevinfo, unsigned char * buf);
int SqliteGetStPara(DevInfo_t *pdevinfo, unsigned char * buf);
int SqliteGetPro(DevInfo_t *pdevinfo, unsigned int id);
/*
** �������ܣ��������ݱ��е�V0
** ���������id=��ʾ���� var=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
** ���ߣ�huyubin
*/
int sqlite_write_data(char * id, char * var);

/*
** �������ܣ���ȡ���ݱ��е�V0,������
** ���������id=���ݱ�ʶ
** ���������var=��������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
** ���ߣ�huyubin
*/
int sqlite_read_data(char * buf, int idx);
int sqlite_read_data_ex(int id, char * var);

int SqliteGetCntEx(void);
/*
** �������ܣ���ȡ���ݱ��е�ID��name
** ���������id��name
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��
** ���ߣ�huyubin
*/
int SqliteReadList(char * buf, int idx);
#endif  // _RASDATABASE_H
/*********************************************************************************/
