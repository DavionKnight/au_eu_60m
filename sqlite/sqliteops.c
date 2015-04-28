/********************  COPYRIGHT(C)***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: sqliteops.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��: 
**���򿪷�������
**��        ��: sqlite���ݿ�����ļ�
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

#include "sqliteops.h"

pthread_mutex_t mutex_save_database;//�洢���ݿ���
sqlite3 *db = NULL; //����sqlite�ؼ��ṹָ��

/*******************************************************************************
*�������� : void SqliteInit(void)
*��    �� : ���ݿ���Ʋ�����ʼ������
*������� : None
*������� : None
*******************************************************************************/
void SqliteInit(void)
{
  pthread_mutex_init(&mutex_save_database, NULL);//��������ʼ��
}

/*******************************************************************************
*�������� : void SqliteExit(void)
*��    �� : ���ݿ���Ʋ����˳���������
*������� : None
*������� : None
*******************************************************************************/
void SqliteExit(void)
{
  pthread_mutex_destroy(&mutex_save_database);
}

/*******************************************************************************
*�������� : int SqliteOpen(const char *dbName)
*��    �� : �����ݿ�
*������� : �򿪵����ݿ�����
*������� : SQLITE_OK:�򿪳ɹ�;-1����ʧ��
*******************************************************************************/
int SqliteOpen(const char *dbName)
{
int result;

  result = sqlite3_open(dbName, &db);
  if (result != SQLITE_OK)
  {
    printf("���ݿ��ʧ��.\n");//���ݿ��ʧ��
  }
  return result;
}

/*******************************************************************************
*�������� : void SqliteClose(void)
*��    �� : �ر����ݿ�
*������� : none
*������� : none
*******************************************************************************/
void SqliteClose(void)
{
  sqlite3_close(db);
  printf("close Sqlite.\n");//���ݿ�ر�
}

/*******************************************************************************
*�������� : int SqliteSelect(const char *sql, SqlResult_t *prs)
*��    �� : ��ѯ���ݿ�
*������� : sql����ѯ���ݿ�SQL��䣻*prs:���صĽ����ָ��
*������� : SQLITE_OK��ִ�гɹ�;����ִ��ʧ��
*******************************************************************************/
int SqliteSelect(const char *sql, SqlResult_t *prs)
{
int result;

  result = sqlite3_get_table(db, sql, &prs->dbResult, &prs->nRow, &prs->nColumn, &prs->errmsg);
  if (result != SQLITE_OK)
  {
    printf("��ѯ��¼ʧ��,������:%d,����ԭ��:%s\n", result, prs->errmsg);
  }
  return result;
}

/*******************************************************************************
*�������� : int SqliteUpdate(const char *sql)
*��    �� : �������ݿ�
*������� : sql���������ݿ�SQL���
*������� : SQLITE_OK��ִ�гɹ�;����ִ��ʧ��
*******************************************************************************/
int SqliteUpdate(const char *sql)
{
int result;
char *errmsg = NULL;

  pthread_mutex_lock(&mutex_save_database);
  result = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (result != SQLITE_OK)
  {
    printf("���¼�¼ʧ��,������:%d,����ԭ��:%s\n", result, errmsg);
  }
  pthread_mutex_unlock(&mutex_save_database);
  return result;
}

/*******************************************************************************
*�������� : int SqliteInsert(const char *sql)
*��    �� : �������ݿ�
*������� : sql���������ݿ�SQL���
*������� : SQLITE_OK��ִ�гɹ�;����ִ��ʧ��
*******************************************************************************/
int SqliteInsert(const char *sql)
{
int result;
char *errmsg = NULL;

  pthread_mutex_lock(&mutex_save_database);
  result = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (result != SQLITE_OK)
  {
    printf("�����¼ʧ��,������:%d,����ԭ��:%s\n", result, errmsg);
  }
  pthread_mutex_unlock(&mutex_save_database);
  return result;
}

/*******************************************************************************
*�������� : int SqliteDelete(const char *sql)
*��    �� : ɾ�����ݿ�
*������� : sql��ɾ�����ݿ�SQL���
*������� : SQLITE_OK��ִ�гɹ�;����ִ��ʧ��
*******************************************************************************/
int SqliteDelete(const char *sql)
{
int result;
char *errmsg = NULL;

  pthread_mutex_lock(&mutex_save_database);
  result = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (result != SQLITE_OK)
  {
    printf("ɾ����¼ʧ��,������:%d,����ԭ��:%s\n", result, errmsg);
  }
  pthread_mutex_unlock(&mutex_save_database);
  return result;
}

/*******************************************************************************
*�������� : int SqliteCreate(const char *sql)
*��    �� : �������ݿ��
*������� : sql���������ݿ��SQL���
*������� : SQLITE_OK��ִ�гɹ�;����ִ��ʧ��
*******************************************************************************/
int SqliteCreate(const char *sql)
{
int result;
char *errmsg = NULL;

  pthread_mutex_lock(&mutex_save_database);
  result = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (result != SQLITE_OK)
  {
    printf("�������ݱ�ʧ��,������:%d,����ԭ��:%s\n", result, errmsg);
  }
  pthread_mutex_unlock(&mutex_save_database);
  return result;
}

/*******************************************************************************
*�������� : int SqliteTransaction(const char *sql)
*��    �� : ���ݿ��������
*������� : sql�����ݿ��������SQL���
*������� : SQLITE_OK��ִ�гɹ�;����ִ��ʧ��
*******************************************************************************/
int SqliteTransaction(const char *sql)
{
int result;
char *errmsg = NULL;

  return SQLITE_OK;

  pthread_mutex_lock(&mutex_save_database);
  result = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (result != SQLITE_OK)
  {
    printf("������ʧ��,������:%d,����ԭ��:%s\n", result, errmsg);
  }
  pthread_mutex_unlock(&mutex_save_database);
  return result;
}
/*******************************************************************************/
