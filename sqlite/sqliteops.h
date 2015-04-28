/********************  COPYRIGHT(C)***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: sqliteops.h
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: sqlite���ݿ����ͷ�ļ�
**--------------��ʷ�汾��Ϣ----------------------------------------------------
** ������: �ں�ͼ
** ��  ��:
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
#ifndef _SQLITEOPS_H_
#define _SQLITEOPS_H_

#include "../common/druheader.h"

void SqliteInit(void);
void SqliteExit(void);
int SqliteOpen(const char *dbName);
void SqliteClose(void);
int SqliteSelect(const char *sql, SqlResult_t *prs);
int  SqliteUpdate(const char *sql);
int SqliteInsert(const char *sql);
int SqliteDelete(const char *sql);
int SqliteCreate(const char *sql);
int SqliteTransaction(const char *sql);

#endif /*_SQLITEOPS_H_*/
/*******************************************************************************/
