/********************  COPYRIGHT(C)***************************************
**--------------�ļ���Ϣ--------------------------------------------------------
**��   ��   ��: drudatabase.c
**��   ��   ��: �ں�ͼ
**�� ��  �� ��:
**���򿪷�������
**��        ��: ���ݿ⴦�����
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
#include "../protocol/approtocol.h"
#include "../net/netcom.h"
#include "drudatabase.h"

pthread_mutex_t mutex_flash_savedb;//�洢���ݿ���
extern int g_DevType;
extern DevicePara_t g_DevicePara;

/*******************************************************************************
*�������� : int DbTableInit(char *tblname)
*��     �� : ���ݿ����ݳ�ʼ��
*������� : char *tblname:���ݱ�����
*������� : �ɹ�����1,������<0
*******************************************************************************/
int DbTableInit(char *tblname)
{
int res, result;
unsigned int objectid;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], sbuf[250];
FILE *fp_cfg = NULL;

  //����tbl���ݱ��Ƿ����,�粻��������
  sprintf(sql, "SELECT * FROM sqlite_master WHERE name=\'%s\';", tblname);
  res = SqliteSelect(sql, &sqlres);
  if(res == SQLITE_OK)
  {
    if(sqlres.nRow == 0)//tblname���ݱ�����
    {
      sprintf(sbuf, "%s/%s_cfg.txt", CONFIGPATH, tblname);
      fp_cfg = fopen(sbuf, "r");
      if(fp_cfg == NULL) //��Ҫ�򿪵��ļ�������,���½���
      {
        DEBUGOUT("%s does not exist,Creat!\r\n", sbuf);
        fp_cfg = fopen(sbuf, "w+");
        sprintf(sbuf, "(%s,V0);", TABLE_HEAD);
        fwrite(sbuf, strlen(sbuf), 1, fp_cfg);//���������ļ����ݱ�ͷ
      }
      rewind(fp_cfg);//ָ���ļ�ͷ
      fgets(sbuf, sizeof(sbuf), fp_cfg);//���ж�ȡ�ļ�
      fclose(fp_cfg);
      sprintf(sql, "CREATE TABLE %s %s", tblname, sbuf);
      res = SqliteCreate(sql);//����sqtbl���ݱ�
      if(res != SQLITE_OK)//����sqtbl���ݱ�ʧ��
      {
        DEBUGOUT("DbTableInit:SqliteCreate %s Failure!\r\n", tblname);
        result = res;
		return -1;
      }
      else
      {
        DEBUGOUT("DbTableInit:SqliteCreate %s Success!\r\n", tblname);
      }
    }

    sprintf(sbuf, "%s/%s_cfg.txt", CONFIGPATH, tblname);
    fp_cfg = fopen(sbuf, "r");
    fgets(sbuf, sizeof(sbuf), fp_cfg);//���ж�ȡ�ļ�
    while(!feof(fp_cfg))
    {
      memset(sbuf, 0, sizeof(sbuf));
      fgets(sbuf, sizeof(sbuf), fp_cfg);//���ж�ȡ�ļ�
      if(strstr(sbuf, ");") != NULL)//�н�����־
      {
        sscanf(strstr(sbuf, "0x"), "0x%08X", &objectid);//��ʶ
        sprintf(sql, "SELECT * FROM %s WHERE ID =\"0x%08X\";", tblname, objectid);
		printf(sql);
        res = SqliteSelect(sql, &sqlres);
        if (res == SQLITE_OK)
        {
          if(sqlres.nRow == 0)
          {
            DEBUGOUT("INSERT INTO %s VALUES %s", tblname, sbuf);
            sprintf(sql, "INSERT INTO %s VALUES %s", tblname, sbuf);
            res = SqliteInsert(sql);//�����²���
            if(res != SQLITE_OK)
            {
              fclose(fp_cfg); //�ر��ļ�
              DEBUGOUT("DbTableInit:%s Failure!\r\n", sql);
              result = res;
            }
          }
        }
      }
      else
      {
        DEBUGOUT("DbTableInit:%s,Format Error!\r\n", sbuf);
      }
    }
    fclose(fp_cfg);
    DEBUGOUT("DbTableInit:Table:%s Create Success.\r\n", tblname);
    result = 1;
  }
  else
  {
    DEBUGOUT("DbTableInit:SqliteSelect Error!\r\n");
    result = -1;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}

/*******************************************************************************
*�������� : int SystemDataInit(void)
*��     �� : ϵͳ���ݳ�ʼ��
*������� : None
*������� : None
*******************************************************************************/
int DataBaseInit(void)
{
int  res;
char SaveDbName[100], RamDbName[100], TableName[20], sql[SQL_CMD_SIZE];

	pthread_mutex_init(&mutex_flash_savedb, NULL);//��������ʼ��
	memset(SaveDbName, 0, sizeof(SaveDbName));
	memset(RamDbName, 0, sizeof(RamDbName));
	memset(TableName, 0, sizeof(TableName));
	memset(sql, 0, sizeof(sql));

  if (g_DevType == MAIN_UNIT)
  {
  	sprintf(SaveDbName, DRURMUSAVEDB);
  	sprintf(RamDbName, DRURMUDB);
  	sprintf(TableName, RMU_TBL);
  }
  else if (g_DevType == EXPAND_UNIT)
  {
  	sprintf(SaveDbName, DRUREUSAVEDB);
  	sprintf(RamDbName, DRUREUDB);
  	sprintf(TableName, REU_TBL);
  }
  else
  {
  	sprintf(SaveDbName, DRURAUSAVEDB);
  	sprintf(RamDbName, DRURAUDB);
  	sprintf(TableName, RAU_TBL);
  }
  
	res = SqliteOpen(SaveDbName);
	if(res == SQLITE_OK)//��ʼ�����м�ز������ݿ�
	{
		if (DbTableInit(TableName) == 1)
		{
			sprintf(sql, "cp %s %s", SaveDbName, RamDbName);
			system(sql);
			sleep(1);
			while (access(RamDbName, F_OK) != 0)
			{
				usleep(10000);
			}
		}
		SqliteClose();
		res = SqliteOpen(RamDbName);
		if(res != SQLITE_OK)//��ʼ�����м�ز������ݿ�
	  	{
	    	DEBUGOUT("DataBaseInit:SqliteOpen %s Failure!\r\n", RamDbName);
	  	}
	  	return res;
	}
  	else
  	{
    	DEBUGOUT("DataBaseInit:SqliteOpen %s Failure!\r\n", SaveDbName);
    	return res;
  	}
}

/*******************************************************************************
*�������� : int DbGetIDList(DevInfo_t *pdevinfo, int *pbuf)
*��     �� : �����ݿ��ȡ����б�����
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������
*         : int *pbuf:pdevinfo��ָ�豸��Ӧ�����б�
*������� : ��ȷ���ز�������,���ݱ���δ�鵽������-1,���ݲ��ҳ���-2
*******************************************************************************/
int DbGetIDList(DevInfo_t *pdevinfo, int *pbuf)
{
int   i, result;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tbl_name[TBL_NAME_SIZE], var_name[TBL_VAR_SIZE];

  if (DbGetTblVarName(pdevinfo, tbl_name, var_name) == 1)
  {
    //��sqtbl���ݱ����ID��Ӧ����
    sprintf(sql, "SELECT ID,%s FROM %s;", var_name, tbl_name);
    if (SqliteSelect(sql, &sqlres) == SQLITE_OK)
    {
      DEBUGOUT("tbl:%s,var:%s.GetIDList.\r\n", tbl_name, var_name);
      if(sqlres.nRow > 0)
      {
        for(i = sqlres.nColumn; i < (sqlres.nRow + 1) * sqlres.nColumn;)
        {
          sscanf(sqlres.dbResult[i], "0x%4X", pbuf++);//ID��ʶ
          i = i + sqlres.nColumn;
        }
        result = sqlres.nRow;
      }
      else//���ݱ���δ�鵽������
      {
        result = -1;
      }
    }
    else//���ݲ��ҳ���
    {
      result = -2;
    }
  }
  else
  {
    result = -3;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}
// ��     �� : �����ݿ��ȡ����б�����MCPC ID
int DbGetIDList_MCP_C(DevInfo_t *pdevinfo, int *pbuf)
{
	int   i, result;
	SqlResult_t sqlres;
	char sql[SQL_CMD_SIZE], tbl_name[TBL_NAME_SIZE], var_name[TBL_VAR_SIZE];
	int author = 0;

  if(DbGetThisIntPara(AUTHOR_ID, &author) > 0){ // ��ȡ����Ȩ�޲��� 0:���ܲ��� 3�����в���
	  if((author > 0) && (author < 4)){
	  }else{
		  author = 0;
	  }
  }
  if (DbGetTblVarName(pdevinfo, tbl_name, var_name) == 1)
  {
    //��sqtbl���ݱ����ID��Ӧ����
    sprintf(sql, "SELECT ID,%s FROM %s where pro<%d;", var_name, tbl_name, (author+1)*10);
    if (SqliteSelect(sql, &sqlres) == SQLITE_OK)
    {
      DEBUGOUT("tbl:%s,var:%s.GetIDList.\r\n", tbl_name, var_name);
      if(sqlres.nRow > 0)
      {
        for(i = sqlres.nColumn; i < (sqlres.nRow + 1) * sqlres.nColumn;)
        {
          sscanf(sqlres.dbResult[i], "0x%8X", pbuf++);//ID��ʶ
          i = i + sqlres.nColumn;
        }
        result = sqlres.nRow;
      }
      else//���ݱ���δ�鵽������
      {
        result = -1;
      }
    }
    else//���ݲ��ҳ���
    {
      result = -2;
    }
  }
  else
  {
    result = -3;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}
/*******************************************************************************
*�������� : int DbGetParaValue(DevInfo_t *pdevinfo, INT16U objectid, Pdu_t *p_pdu)
*��     �� : �����ݿ��ȡ�洢������
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;
            INT16U objectid:����ID��;Pdu_t *p_pdu:������Ӧ���ݽṹ����ָ��
*������� : ��ȷ����1,�������<0
*******************************************************************************/
int DbGetParaValue(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu)
{
int i, res, result, buf[7];
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

  //�����豸��š��˿ںš�ģ������,��ȡ���ݿ��,������
  memset(p_pdu, 0, sizeof(Pdu_t));
  if (DbGetTblVarName(pdevinfo, tblname, varname) == 1)
  {
    sprintf(sql, "SELECT %s,%s FROM %s WHERE ID =\"0x%08X\";", TABLE_HEAD, varname, tblname, objectid);
    //DEBUGOUT("SqliteSelect:%s\r\n", sql);
    res = SqliteSelect(sql, &sqlres);
    if (res == SQLITE_OK)
    {
      //SqlResultDis(&sqlres);
      if(sqlres.nRow > 0)
      {
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//��ʶ
        p_pdu->id = res;
        strcpy(p_pdu->name, sqlres.dbResult[sqlres.nColumn + TBL_NAME]); //��������
        strcpy(p_pdu->mode, sqlres.dbResult[sqlres.nColumn + TBL_MODE]); //RO/RWģʽ
        p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //���ݳ���
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //��������
        p_pdu->coefficient = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]); //����ϵ��
        //printf("pdu:id:%d,name:%s,mode:%s,len:%d,type:%s,coe:%d\r\n",
        //       p_pdu->id,p_pdu->name,p_pdu->mode,p_pdu->len,p_pdu->var_type,p_pdu->coefficient);
        memset(buf, 0, sizeof(buf));
        if(strcmp(p_pdu->var_type, "str") == 0)//�ַ���
        {
          strncpy(p_pdu->var, sqlres.dbResult[sqlres.nColumn + TBL_VAR], p_pdu->len);
        }
        else if(strcmp(p_pdu->var_type, "dstr") == 0)//���ݴ�
        {
          //IP��ַ
          if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
            ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID)|| (p_pdu->id == DEVROUTE_ID))
          {
            sscanf(sqlres.dbResult[sqlres.nColumn + TBL_VAR], "%d.%d.%d.%d",
                   &buf[0], &buf[1], &buf[2], &buf[3]);
            for(i = 0; i < p_pdu->len; i++)
              p_pdu->var[i] = (char)buf[i];
          }
          //�豸��ǰʱ�����ɿ�ʼʱ��,7���ֽ����
          else if ((p_pdu->id == DEVICETIME_ID) || (p_pdu->id == SAMPLESTARTTIME_ID))
          {
            sscanf(sqlres.dbResult[sqlres.nColumn + TBL_VAR], "%02X%02X%02X%02X %02X:%02X:%02X", 
                   &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5], &buf[6]);
            for(i = 0; i < p_pdu->len; i++)
              p_pdu->var[i] = (char)buf[i];
          }
        }
        else//uint1,uint2,uint3,uint4,sint1,sint2,bit
        {
          res = atoi(sqlres.dbResult[sqlres.nColumn + TBL_VAR]);//����
          for(i = 0; i < p_pdu->len; i++)
          {
            p_pdu->var[i] = (char)res;
            res = res >> 8;
          }
        }
        result = 1;
      }
      else
      {
        DEBUGOUT("DbGetParaValue:%s,%s ID:0x%08X no Find!\r\n",varname, tblname, objectid);
        result = -1;
      }
    }
    else
    {
      DEBUGOUT("DbGetParaValue:SqliteSelect Error!\r\n");
      result = -2;
    }
  }
  else
  {
    DEBUGOUT("DbGetParaValue:DbGetTblVarName Error!\r\n");
    result = -3;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}

// ��ȡ�����������ֶΣ��жϸò����Ƿ���Ҫ�㲥
int SqliteGetPro(DevInfo_t *pdevinfo, unsigned int id)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

	if (DbGetTblVarName(pdevinfo, tblname, varname) == 1){
		sprintf(sql, "select pro from %s where id=\"0x%08X\";", tblname, id);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// ���ݱ�����
				goto EXIT_HANDLE;
			}else{
				printf("read broadcast id=0x%08X, pro=%s.\n", id, sqlres.dbResult[1]);
				sscanf(sqlres.dbResult[1], "%d", &ret);
				printf("ret=%d\n", ret);
				goto EXIT_HANDLE;
			}
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}
// ��չ��Ԫ��ѯ������һ����Ӧ�˿��µ�Զ���Ƿ��и澯
// ��Զ�˽���״̬ʱʹ�� ID = 0x08000080
int SqliteGetAlarmCnt(int port_num)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];
	DevInfo_t devinfo;

	memset((char *)&devinfo, 0, sizeof(devinfo));
	// ��ѯ�澯�������Ƿ��и澯
	if (DbGetTblVarName(&devinfo, tblname, varname) == 1){
		sprintf(sql, "select count(*) from %s where ((id like \'0x0800038%d\') or (id like \'0x0800060%d\') or (id like \'0x0800_48%d\') or (id like \'0x0800_58%d\' and id!='0x0800058%d') or (id like \'0x0800006%d\')) and (V0 like '1' or V0 like '129');", tblname, port_num, port_num, port_num, port_num, port_num, port_num);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// ���ݱ�����
				goto EXIT_HANDLE;
			}else{
				printf("%s\n", sqlres.dbResult[1]);
				sscanf(sqlres.dbResult[1], "%d", &ret);
				goto EXIT_HANDLE;
			}
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}

// ��ѯ�����ֶθ�λ��2�Ĳ�������������չ��ѯԶ�˲�����
int SqliteGetReadCnt(DevInfo_t *pdevinfo)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

	if (DbGetTblVarName(pdevinfo, tblname, varname) == 1){
		sprintf(sql, "select count(*) from %s where pro%%10=2;", tblname);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// ���ݱ�����
				goto EXIT_HANDLE;
			}else{
				printf("%s\n", sqlres.dbResult[1]);
				sscanf(sqlres.dbResult[1], "%d", &ret);
				goto EXIT_HANDLE;
			}
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}

// ��ѯ�����ֶθ�λ��3�Ĳ�������������չ����Զ�˲�����
int SqliteGetWriteCnt(DevInfo_t *pdevinfo)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

	if (DbGetTblVarName(pdevinfo, tblname, varname) == 1){
		sprintf(sql, "select count(*) from %s where pro%%10=3;", tblname);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// ���ݱ�����
				goto EXIT_HANDLE;
			}else{
				printf("%s\n", sqlres.dbResult[1]);
				sscanf(sqlres.dbResult[1], "%d", &ret);
				goto EXIT_HANDLE;
			}
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}

// �������ֶθ�λ��2/3�Ĳ�����ȡ�������ŵ��ڴ��У���Ҫ��������
// type=2(��չ��ѯԶ�˲���) 3����չ����Զ�˲�����
int SqliteGetInitPara(DevInfo_t *pdevinfo, void * buf, int type)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];
	unsigned int id;
	int temp = 0;
	int idx = 0;
	int i = 0;

	if (DbGetTblVarName(pdevinfo, tblname, varname) == 1){
		sprintf(sql, "select id,len from %s where pro%%10=%d;", tblname, type);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// ���ݱ�����
				goto EXIT_HANDLE;
			}else if(sqlres.nRow > 0){
				idx = 0;
//				printf("get init para  nColum=%d, nRow=%d\r\n", sqlres.nColumn, sqlres.nRow);
				for(i = sqlres.nColumn; i < (sqlres.nRow+1)*sqlres.nColumn; ){
//					printf("get init para id: %s\r\n", sqlres.dbResult[i]);
					sscanf(sqlres.dbResult[i++], "0x%08X", &id);//��ʶ
//					printf("id=0x%08x\n", id);
					memcpy(buf+idx, (char *)&id, 4);
					idx += 4;
//					printf("get init para len: %s\r\n", sqlres.dbResult[i]);
					sscanf(sqlres.dbResult[i++], "%d", &temp);//��ʶ len
					memcpy(buf+idx, (char *)&temp, 1);
					idx += 1;
				}
				ret = 0;
				goto EXIT_HANDLE;
			}
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}
// ��ȡ��Ҫ�㲥���ò���, �����ֶθ�λ��1
int SqliteGetBPara(DevInfo_t *pdevinfo, unsigned char * buf)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];
	unsigned int  id;
	unsigned int  len;
	unsigned char * pbuf;
	unsigned char * pvalue;
	int temp = 0;
	int i = 0;
	int cnt = 0;

	if (DbGetTblVarName(pdevinfo, tblname, varname) == 1){
		sprintf(sql, "select id,len,var_type,V0 from %s where pro%%10=1;", tblname);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// ���ݱ�����
				goto EXIT_HANDLE;
			}else if(sqlres.nRow > 0){
				pbuf = buf;
				len = 0;
				for(i = sqlres.nColumn; i < (sqlres.nRow+1)*sqlres.nColumn; ){
					sscanf(sqlres.dbResult[i++], "0x%08X", &id);//��ʶ
					memcpy(pbuf+1, (char *)&id, 4);
					sscanf(sqlres.dbResult[i++], "%d", &temp);//��ʶ LEN
					pbuf[0] = temp+5;
					if(strcmp(sqlres.dbResult[i], "str") == 0){ // var_type
						memcpy(pbuf+5, sqlres.dbResult[i+1], temp); // V0 = str
					}else if(strcmp(sqlres.dbResult[i], "dstr") == 0){

					}else{
						cnt = atoi(sqlres.dbResult[i+1]);			
						memcpy(pbuf+5, (char *)&cnt, temp);
					}
					i += 2;
					len += (5+temp);
					pbuf = buf + len;
				}
				ret = len;
				goto EXIT_HANDLE;
			}
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}
// ��ȡ�㲥״̬����,�����ֶθ�λ��4
int SqliteGetStPara(DevInfo_t *pdevinfo, unsigned char * buf)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];
	unsigned int  id;
	unsigned int  len;
	unsigned char * pbuf;
	//unsigned char * pvalue;
	int temp = 0;
	int i = 0;
	int cnt = 0;

	if (DbGetTblVarName(pdevinfo, tblname, varname) == 1){
		sprintf(sql, "select id,len,var_type,V0 from %s where pro%%10=4;", tblname);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// ���ݱ�����
				goto EXIT_HANDLE;
			}else if(sqlres.nRow > 0){
				pbuf = buf;
				len = 0;
				for(i = sqlres.nColumn; i < (sqlres.nRow+1)*sqlres.nColumn; ){
					sscanf(sqlres.dbResult[i++], "0x%08X", &id);//��ʶ
					memcpy(pbuf+1, (char *)&id, 4);
					sscanf(sqlres.dbResult[i++], "%d", &temp);//��ʶ LEN
					pbuf[0] = temp+5;
					if(strcmp(sqlres.dbResult[i], "str") == 0){ // var_type
						memcpy(pbuf+5, sqlres.dbResult[i+1], temp); // V0 = str
					}else if(strcmp(sqlres.dbResult[i], "dstr") == 0){

					}else{
						cnt = atoi(sqlres.dbResult[i+1]);			
						memcpy(pbuf+5, (char *)&cnt, temp);
					}
					i += 2;
					len += (5+temp);
					pbuf = buf + len;
				}
				ret = len;
				goto EXIT_HANDLE;
			}
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}
// ��ȡ����ֵ��MCPC
int DbGetParaValue_MCP_C(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu)
{
int i, res, result, buf[7];
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

  //�����豸��š��˿ںš�ģ������,��ȡ���ݿ��,������
  memset(p_pdu, 0, sizeof(Pdu_t));
  if (DbGetTblVarName(pdevinfo, tblname, varname) == 1)
  {
    sprintf(sql, "SELECT %s,%s FROM %s WHERE ID =\"0x%08X\";", TABLE_HEAD, varname, tblname, objectid);
    //DEBUGOUT("SqliteSelect:%s\r\n", sql);
    res = SqliteSelect(sql, &sqlres);
    if (res == SQLITE_OK)
    {
      //SqlResultDis(&sqlres);
      if(sqlres.nRow > 0)
      {
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//��ʶ
        p_pdu->id = res;
        strcpy(p_pdu->name, sqlres.dbResult[sqlres.nColumn + TBL_NAME]); //��������
        strcpy(p_pdu->mode, sqlres.dbResult[sqlres.nColumn + TBL_MODE]); //RO/RWģʽ
        p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //���ݳ���
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //��������
        p_pdu->coefficient = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]); //����ϵ��
//        p_pdu->min = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]);;		  //��Сֵ
//  			p_pdu->max = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]);;          //���ֵ
        //printf("pdu:id:%d,name:%s,mode:%s,len:%d,type:%s,coe:%d\r\n",
        //       p_pdu->id,p_pdu->name,p_pdu->mode,p_pdu->len,p_pdu->var_type,p_pdu->coefficient);
        memset(buf, 0, sizeof(buf));
        if(strcmp(p_pdu->var_type, "str") == 0)//�ַ���
        {
          strncpy(p_pdu->var, sqlres.dbResult[sqlres.nColumn + TBL_VAR], p_pdu->len);
        }
        else if(strcmp(p_pdu->var_type, "dstr") == 0)//���ݴ�
        {
          //IP��ַ
          if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
            ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID)|| (p_pdu->id == DEVROUTE_ID)
            ||(p_pdu->id ==RW1_OMCIP_ID) ||(p_pdu->id ==RW2_OMCIP_ID))
          {
            sscanf(sqlres.dbResult[sqlres.nColumn + TBL_VAR], "%d.%d.%d.%d",
                   &buf[0], &buf[1], &buf[2], &buf[3]);
            for(i = 0; i < p_pdu->len; i++)
              p_pdu->var[i] = (char)buf[i];
          }
          //�豸��ǰʱ�����ɿ�ʼʱ��,7���ֽ����
          else if ((p_pdu->id == DEVICETIME_ID) || (p_pdu->id == SAMPLESTARTTIME_ID))
          {
            sscanf(sqlres.dbResult[sqlres.nColumn + TBL_VAR], "%02X%02X%02X%02X %02X:%02X:%02X", 
                   &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5], &buf[6]);
            for(i = 0; i < p_pdu->len; i++)
              p_pdu->var[i] = (char)buf[i];
          }
        }
        else//uint1,uint2,uint3,uint4,sint1,sint2,bit
        {
          res = atoi(sqlres.dbResult[sqlres.nColumn + TBL_VAR]);//����
          for(i = 0; i < p_pdu->len; i++)
          {
            p_pdu->var[i] = (char)res;
            res = res >> 8;
          }
        }
        result = 1;
      }
      else
      {
        //DEBUGOUT("DbGetParaValue:%s,%s ID:0x%08X no Find!\r\n",varname, tblname, objectid);
        result = -1;
      }
    }
    else
    {
      DEBUGOUT("DbGetParaValue:SqliteSelect Error!\r\n");
      result = -2;
    }
  }
  else
  {
    DEBUGOUT("DbGetParaValue:DbGetTblVarName Error!\r\n");
    result = -3;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}

/*******************************************************************************
*�������� : int SaveDbUpdate(const char *sql)
*��    �� : ���ݿ����ݴ洢����
*������� : sql:�������ݿ�SQL���
*������� : SQLITE_OK��ִ�гɹ�;����ִ��ʧ��
*******************************************************************************/ 
int SaveDbUpdate(const char *sql)
{
int result = 0;
char dbName[100], *errmsg = NULL;
sqlite3 *savedb = NULL;

	memset(dbName, 0, sizeof(dbName));
  if (g_DevType == MAIN_UNIT)
  {
		sprintf(dbName, DRURMUSAVEDB);
  }
  else if (g_DevType == EXPAND_UNIT)
  {
		sprintf(dbName, DRUREUSAVEDB);
  }
  else
  {
  	sprintf(dbName, DRURAUSAVEDB);
  }
  pthread_mutex_lock(&mutex_flash_savedb);
  result = sqlite3_open(dbName, &savedb);
  if (result != SQLITE_OK)
  {
    printf("���ݴ洢�ļ���ʧ��.\n");//���ݿ��ʧ��
    return result;
  }
  //DEBUGOUT("���ݴ洢:%s\r\n", sql);
  result = sqlite3_exec(savedb, sql, NULL, NULL, &errmsg);
  if (result != SQLITE_OK)
  {
    printf("���ݴ洢���¼�¼ʧ��,������:%d,����ԭ��:%s\r\n", result, errmsg);
  }
  sqlite3_close(savedb);
  pthread_mutex_unlock(&mutex_flash_savedb);
  return result;
}

/*******************************************************************************
*�������� : int DbSaveParaValue(DevInfo_t *pdevinfo, INT16U objectid, Pdu_t *p_pdu)
*��    �� : �������ݿ������
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;
            INT16U objectid:����ID��;Pdu_t *p_pdu:������Ӧ���ݽṹ����ָ��
*������� : ���³ɹ�����1,δ���³ɹ�����-1,���ݳ��Ȳ���-2,���ݱ���δ�鵽������-3,���ݲ��ҳ���-4
*******************************************************************************/
int DbSaveParaValue(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu)
{
int res, result;
INT8S sint8;
INT16S sint16;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE], strbuf[21];

  if (DbGetTblVarName(pdevinfo, tblname, varname) == 1)
  {
    //��tblname���ݱ����ID��Ӧ����
    sprintf(sql, "SELECT %s,%s FROM %s WHERE ID =\"0x%08X\";", TABLE_HEAD, varname, tblname, objectid);
    res = SqliteSelect(sql, &sqlres);
    if (res == SQLITE_OK)
    {
      //SqlResultDis(&sqlres);
      if(sqlres.nRow > 0)
      {
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//��ʶ
        p_pdu->id = res;
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //��������
        if(p_pdu->len == atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]))//���������
        {
          memset(strbuf, 0, sizeof(strbuf));
          if(strcmp(p_pdu->var_type, "str") == 0)//�ַ���
          {
            strncpy(strbuf, p_pdu->var, p_pdu->len);//�ַ���
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else if(strcmp(p_pdu->var_type, "dstr") == 0)//���ݴ�
          {
            //IP��ַ
            if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
              ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID) || (p_pdu->id == DEVROUTE_ID))
            {
              sprintf(strbuf, "%d.%d.%d.%d", p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3]);
            }
            //�豸��ǰʱ�����ɿ�ʼʱ��,7���ֽ����
            else if ((p_pdu->id == DEVICETIME_ID) || (p_pdu->id == SAMPLESTARTTIME_ID))
            {
              sprintf(strbuf, "%02X%02X%02X%02X %02X:%02X:%02X", 
                      p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3], p_pdu->var[4], p_pdu->var[5], p_pdu->var[6]);
            }
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else//uint1,uint2,uint3,uint4,sint1,sint2,bit
          {
            if(strstr(p_pdu->var_type, "sint1") != NULL)//sint1�з�����
            {
            	sint8 = 0;
              memcpy(&sint8, p_pdu->var, p_pdu->len);
              res = sint8;
            }
            else if(strstr(p_pdu->var_type, "sint2") != NULL)//sint2�з�����
            {
            	sint16 = 0;
              memcpy(&sint16, p_pdu->var, p_pdu->len);
              res = sint16;
            }
            else//uint1,uint2,uint3,uint4,bit
            {
            	res = 0;
              memcpy(&res, p_pdu->var, p_pdu->len);
            }
//            p_pdu->min == atoi(sqlres.dbResult[sqlres.nColumn + TBL_MIN]))//������Сֵ
//            p_pdu->max == atoi(sqlres.dbResult[sqlres.nColumn + TBL_MAX]))//�������ֵ
//						if ((res < p_pdu->min) || (res > p_pdu->max))
//						{
//							sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
//							return -6;
//						}
//						else
//						{
//            	sprintf(sql, "UPDATE %s SET %s=%d WHERE ID=\"0x%08X\";", tblname, varname, res, objectid);
//            }
            sprintf(sql, "UPDATE %s SET %s=%d WHERE ID=\"0x%08X\";", tblname, varname, res, objectid);
          }
          //DEBUGOUT("SqliteUpdate:%s\r\n", sql);
          res = SqliteUpdate(sql);//����sqtbl���ݱ���objectid��Ӧ����
          if (res == SQLITE_OK)
          {
					  SaveDbUpdate(sql);
					  result = 1;
				  }
          else
          {
            DEBUGOUT("DbSaveParaValue:SqliteUpdate Error!\r\n");
            result = -1;
          }
        }
        else
        {
          p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //���ݳ���
          DEBUGOUT("DbSaveParaValue:Para Len Error!\r\n");
          result = -2;
        }
      }
      else//���ݱ���δ�鵽������
      {
        DEBUGOUT("DbSaveParaValue:%s,%s ID:0x%08X no Find!\r\n",varname, tblname, objectid);
        result = -3;
      }
    }
    else//���ݲ��ҳ���
    {
      DEBUGOUT("DbSaveParaValue:SqliteSelect Error!\r\n");
      result = -4;
    }
  }
  else
  {
    DEBUGOUT("DbSaveParaValue:DbGetTblVarName Error!\r\n");
    result = -5;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}

// ��������MCPC
// flag=0(��Ҫͬ����FLASH�У� 1������Ҫͬ����FLASH����������������ʣ��¶ȵȲ�����
int DbSaveParaValue_MCP_C(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu, int flag)
{
int res, result;
INT8S sint8;
INT16S sint16;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE], strbuf[21];

  if (DbGetTblVarName(pdevinfo, tblname, varname) == 1)
  {
    //��tblname���ݱ����ID��Ӧ����
    sprintf(sql, "SELECT %s,%s FROM %s WHERE ID =\"0x%08X\";", TABLE_HEAD, varname, tblname, objectid);
    res = SqliteSelect(sql, &sqlres);
    if (res == SQLITE_OK)
    {
      //SqlResultDis(&sqlres);
      if(sqlres.nRow > 0)
      {
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//��ʶ
        p_pdu->id = res;
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //��������
        if(p_pdu->len == atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]))//���������
        {
          memset(strbuf, 0, sizeof(strbuf));
          if(strcmp(p_pdu->var_type, "str") == 0)//�ַ���
          {
            strncpy(strbuf, p_pdu->var, p_pdu->len);//�ַ���
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else if(strcmp(p_pdu->var_type, "dstr") == 0)//���ݴ�
          {
            //IP��ַ
            if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
              ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID) || (p_pdu->id == DEVROUTE_ID)
              ||(p_pdu->id == RW1_OMCIP_ID) ||(p_pdu->id ==RW2_OMCIP_ID) )
            {
              sprintf(strbuf, "%d.%d.%d.%d", p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3]);
            }
            //�豸��ǰʱ�����ɿ�ʼʱ��,7���ֽ����
            else if ((p_pdu->id == DEVICETIME_ID) || (p_pdu->id == SAMPLESTARTTIME_ID))
            {
              sprintf(strbuf, "%02X%02X%02X%02X %02X:%02X:%02X", 
                      p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3], p_pdu->var[4], p_pdu->var[5], p_pdu->var[6]);
            }
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else//uint1,uint2,uint3,uint4,sint1,sint2,bit
          {
            if(strstr(p_pdu->var_type, "sint1") != NULL)//sint1�з�����
            {
            	sint8 = 0;
              memcpy(&sint8, p_pdu->var, p_pdu->len);
              res = sint8;
            }
            else if(strstr(p_pdu->var_type, "sint2") != NULL)//sint2�з�����
            {
            	sint16 = 0;
              memcpy(&sint16, p_pdu->var, p_pdu->len);
              res = sint16;
            }
            else//uint1,uint2,uint3,uint4,bit
            {
            	res = 0;
              memcpy(&res, p_pdu->var, p_pdu->len);
            }
            sprintf(sql, "UPDATE %s SET %s=%d WHERE ID=\"0x%08X\";", tblname, varname, res, objectid);
          }
          //DEBUGOUT("SqliteUpdate:%s\r\n", sql);
          res = SqliteUpdate(sql);//����sqtbl���ݱ���objectid��Ӧ����
          if (res == SQLITE_OK)
          {
			  if(flag == 0){
				  SaveDbUpdate(sql);
			  }
            result = 1;
          }
          else
          {
            DEBUGOUT("DbSaveParaValue:SqliteUpdate Error 0x%08X\r\n", objectid);
            result = -1;
          }
        }
        else
        {
          p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //���ݳ���
		  printf("drudatabase.c DbSaveParaValue_MCP_C(), %d, 0x%08X\n", p_pdu->len, objectid);
          DEBUGOUT("DbSaveParaValue:Para Len Error!\r\n");
          result = -2;
        }
      }
      else//���ݱ���δ�鵽������
      {
        DEBUGOUT("DbSaveParaValue:%s,%s ID:0x%08X no Find!\r\n",varname, tblname, objectid);
        result = -3;
      }
    }
    else//���ݲ��ҳ���
    {
      DEBUGOUT("DbSaveParaValue:SqliteSelect Error!\r\n");
      result = -4;
    }
  }
  else
  {
    DEBUGOUT("DbSaveParaValue:DbGetTblVarName Error!\r\n");
    result = -5;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}
/*******************************************************************************
*�������� : int DbGetThisIntPara(INT16U objectid, int *pval)
*��    �� : ��ȡ�������β���
*������� : INT16U objectid:����ID��;int *pval:��������ֵָ��
*������� : ���³ɹ�����1,δ���³ɹ�����-1
*******************************************************************************/
int DbGetThisIntPara(unsigned int objectid, int *pval)
{
Pdu_t pdu;
DevInfo_t devinfo;
INT8S sint8;
INT16S sint16;

  memset(&devinfo, 0, sizeof(DevInfo_t));
  *pval = 0;
  if (DbGetParaValue(&devinfo, objectid, &pdu) == 1)
  {
    if(strstr(pdu.var_type, "sint1") != NULL)//sint1�з�����
    {
      memcpy(&sint8, &pdu.var, pdu.len);
      *pval = sint8;
    }
    else if(strstr(pdu.var_type, "sint2") != NULL)//sint2�з�����
    {
      memcpy(&sint16, &pdu.var, pdu.len);
      *pval = sint16;
    }
    else//uint1,uint2,uint3,uint4,bit
    {
      memcpy((char *)pval, &pdu.var, pdu.len);
    }
    return 1;
  }
	else
	{
	  return -1; 
	}
}

/*******************************************************************************
*�������� : int DbSaveThisIntPara(INT16U objectid, int val)
*��    �� : �洢�������β���
*������� : INT16U objectid:����ID��;int val:����ֵ
*������� : ���³ɹ�����1,δ���³ɹ�����-1
*******************************************************************************/
int DbSaveThisIntPara(unsigned int objectid, int val)
{
Pdu_t pdu;
DevInfo_t devinfo;

  memset(&devinfo, 0, sizeof(DevInfo_t));
  if (DbGetParaValue(&devinfo, objectid, &pdu) == 1)
  {
    memset(&pdu.var, 0, sizeof(pdu.var));
    memcpy(&pdu.var, &val, pdu.len);
    return DbSaveParaValue(&devinfo, objectid, &pdu);
  }
	else
	{
	  return -1; 
	}
}

// �洢�������β��� flag=0 ��Ҫͬ����FLASH�У�flag=1����Ҫͬ����FLASH
int DbSaveThisIntPara_MCP_C(unsigned int objectid, int val, int flag)
{
Pdu_t pdu;
DevInfo_t devinfo;

  memset(&devinfo, 0, sizeof(DevInfo_t));
  if (DbGetParaValue(&devinfo, objectid, &pdu) == 1)
  {
    memset(&pdu.var, 0, sizeof(pdu.var));
    memcpy(&pdu.var, &val, pdu.len);
    return DbSaveParaValue_MCP_C(&devinfo, objectid, &pdu, flag);
  }
	else
	{
	  return -1; 
	}
}

/*******************************************************************************
*�������� : int DbSaveThisStrPara(INT16U objectid, char *buf)
*��    �� : �洢�����ַ����Ͳ���
*������� : INT16U objectid:����ID��;char *buf:����ֵָ��
*������� : ���³ɹ�����1,δ���³ɹ�����-1
*******************************************************************************/
int DbSaveThisStrPara(unsigned int objectid, char *buf)
{
Pdu_t pdu;
DevInfo_t devinfo;

  memset(&devinfo, 0, sizeof(DevInfo_t));
  if (DbGetParaValue(&devinfo, objectid, &pdu) == 1)
  {
    memset(&pdu.var, 0, sizeof(pdu.var));
    memcpy(&pdu.var, buf, pdu.len);
    return DbSaveParaValue(&devinfo, objectid, &pdu);
  }
	else
	{
	  return -1; 
	}
}
// ��ȡ�ַ�������
int DbGetThisStrPara(unsigned int objectid, char *buf)
{
Pdu_t pdu;
DevInfo_t devinfo;

  memset(&devinfo, 0, sizeof(DevInfo_t));
  if (DbGetParaValue(&devinfo, objectid, &pdu) == 1)
  {
    memcpy(buf, &pdu.var, pdu.len);
    return 0; 
  }
	else
	{
	  return -1; 
	}
}
/*******************************************************************************
*�������� : int DbGetTblName(DevInfo_t *pdevinfo, char *tblname)
*��     �� : �����豸��,ģ�����͡���ַ,��ȡ���ݿ��б���
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;char *tblname:��ȡ���ݿ��б���
*������� : ��������1,���󷵻�-1
*******************************************************************************/
int DbGetTblName(DevInfo_t *pdevinfo, char *tblname)
{
  *tblname = 0;
  switch(pdevinfo->ModuleType)
  {
    //����Ԫ����
    case 0x00:
      if (g_DevType == MAIN_UNIT)
      {
        strcpy(tblname, RMU_TBL);
        return 1;
      }
      else if (g_DevType == EXPAND_UNIT)
      {
        strcpy(tblname, REU_TBL);
        return 1;
      }
      else if (g_DevType == RAU_UNIT)
      {
        strcpy(tblname, RAU_TBL);
        return 1;
      }
      else
      {
        return -1;
      }
    break;

    default:
      DEBUGOUT("DruDB Undefined Table!\r\n");
      return -1;
    break;
  }
}

/*******************************************************************************
*�������� : int DbGetTblName(DevInfo_t *pdevinfo, char *varname)
*��     �� : �����豸��,ģ�����͡���ַ,��ȡ���ݿ��б��еı�����
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;char *varname:��ȡ���ݿ��б��еı�����
*������� : ��������1,���󷵻�-1
*******************************************************************************/
int DbGetVarName(DevInfo_t *pdevinfo, char *varname)
{
INT16U mtype;

  *varname = 0;
  mtype = pdevinfo->ModuleType;
  //ֻ������Ԫ����չ��Ԫ��Զ�̼�ص�Ԫ����ϵͳ����,port��ַ������
  if (mtype == 0x00)
  {
    if ((pdevinfo->DeviceNo == BROADCAST_DEV)//���豸,������Ԫ
      ||(pdevinfo->DeviceNo == g_DevicePara.DeviceNo))
    {
      strcpy(varname, "V0");
      return 1;
    }
    else
    {
      sprintf(varname, "V%d", pdevinfo->DeviceNo);//���Ҵӻ��豸,V1,V2...
      if (pdevinfo->DeviceNo == 0)//��ѯ��������
      	return 1;
      else
       return -1;
    }
  }
  return -1;
}

/*******************************************************************************
*�������� : int DbGetTblVarName(DevInfo_t *pdevinfo, char *tblname, char *varname)
*��     �� : �����豸��,ģ�����͡���ַ,��ȡ���ݿ��б����Լ�������
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;
*           char *tblname:��ȡ���ݿ��б���;char *varname:��ȡ���ݿ��б��еı�����
*������� : ��������1,���󷵻�-1
*******************************************************************************/
int DbGetTblVarName(DevInfo_t *pdevinfo, char *tblname, char *varname)
{
  if (DbGetTblName(pdevinfo, tblname) == 1)
  {
    if (DbGetVarName(pdevinfo, varname) == 1)
    {
      //DEBUGOUT("Dev:%d,Maddr:%d,Mtype:%d TblName:%s,VarName:%s!\r\n",
      //         pdevinfo->DeviceNo, pdevinfo->ModuleAddr, pdevinfo->ModuleType, tblname, varname);
      return 1;
    }
    else
    {
      DEBUGOUT("Dev:%d,Maddr:%d,Mtype:%d VarName no Find!\r\n",
               pdevinfo->DeviceNo, pdevinfo->ModuleAddr, pdevinfo->ModuleType);
      return -1;
    }
  }
  else
  {
    DEBUGOUT("Dev:%d,Maddr:%d,Mtype:%d TblName no Find!\r\n",
             pdevinfo->DeviceNo, pdevinfo->ModuleAddr, pdevinfo->ModuleType);
    return -1;
  }
}

/*******************************************************************************
*�������� : int DbAddVarName(DevInfo_t *pdevinfo)
*��     �� : �����豸��,�豸�˿ں�,ģ������,�����ݿ��б�ͷ����һ�����
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;
*������� : ����������ͷ���1,���򷵻�-1
*******************************************************************************/
int DbAddVarName(DevInfo_t *pdevinfo)
{
int res, result;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

  //�����豸��š��˿ںš�ģ������,��ȡ���ݿ�����
  if (DbGetTblVarName(pdevinfo, tblname, varname) == 1)
  {
    //��tblname���ݱ����varname��Ӧ����
    sprintf(sql, "SELECT %s FROM %s;", varname, tblname);
    res = SqliteSelect(sql, &sqlres);
    //�����ݱ�tblname�ж�Ӧvarname��ͷ�����
    if (res != SQLITE_OK)
    {
      //�����ݱ�tblname�����varname��ͷ��
      sprintf(sql, "ALTER TABLE %s ADD COLUMN %s;", tblname, varname);
      DEBUGOUT("DbAddVarName:%s.\r\n", sql);
      res = SqliteInsert(sql);//��¼�������ݿ�
      if (res == SQLITE_OK)//����������Ϊ0
      {
        sprintf(sql, "UPDATE %s SET %s=0;", tblname, varname);
        res = SqliteUpdate(sql);//����sqtbl���ݱ��ж�Ӧ����
        DEBUGOUT("DbAddVarName:SqliteUpdate Success.\r\n");
        result = 1;
      }
      else
      {
        DEBUGOUT("DbAddVarName:SqliteInsert Failure.\r\n");
        result = -1;
      }
    }
    else
    {
      //�����Ѿ�����,��������
      result = 1;
    }
  }
  else
  {
    DEBUGOUT("DbAddVarName:DbGetTblVarName Error.\r\n");
    result = -2;
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return result;
}

/*******************************************************************************
*�������� : int LoadDevicePara(DevInfo_t *pdevinfo, DevicePara_t *p_dev)
*��     �� : �����ݿ��ȡ�洢��Ҫ������
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;
            DevicePara_t *p_dev:�豸�ṹ���Ӧ����
*������� : ���³ɹ�����1,δ���³ɹ�����-1
*******************************************************************************/
int LoadDevicePara(DevInfo_t *pdevinfo, DevicePara_t *p_dev)
{
	int i;
	Pdu_t pdu, *p_pdu;

	DEBUGOUT("LoadDevicePara:DeviceNo:%d;ModuleAddr:%d;ModuleType:0x%04X.\r\n",
	          pdevinfo->DeviceNo, pdevinfo->ModuleAddr, pdevinfo->ModuleType);
	p_pdu = &pdu;
	//memset(p_dev, 0, sizeof(DevicePara_t));

	DbGetParaValue_MCP_C(pdevinfo, STATIONNO_ID, p_pdu);
	memcpy(&p_dev->StationNo, &p_pdu->var, p_pdu->len);
	DbGetParaValue_MCP_C(pdevinfo, DEVICENO_ID, p_pdu);
	memcpy(&p_dev->DeviceNo, &p_pdu->var, p_pdu->len);
	//��MODEM��ز���
	DbGetParaValue(pdevinfo, SMSCTEL_ID, p_pdu);
	memcpy(&p_dev->SmscTel, &p_pdu->var, p_pdu->len);
	for ( i=0; i<5; i++)
	{
		DbGetParaValue_MCP_C(pdevinfo, (QUERYTEL_ID + i), p_pdu);
		memcpy(&p_dev->QueryTel[i], &p_pdu->var, p_pdu->len);
	}
	DbGetParaValue_MCP_C(pdevinfo, NOTIFYTEL_ID, p_pdu);
	memcpy(&p_dev->NotifyTel, &p_pdu->var, p_pdu->len);

	DbGetParaValue_MCP_C(pdevinfo, COMMODEMTYPE_ID, p_pdu);
	memcpy(&p_dev->ComModemType, &p_pdu->var, p_pdu->len);
	DbGetParaValue_MCP_C(pdevinfo, PSTRANPROTOCOL_ID, p_pdu);
	memcpy(&p_dev->PSTranProtocol, &p_pdu->var, p_pdu->len);
	DbGetParaValue_MCP_C(pdevinfo, DEVICECOMMTYPE_ID, p_pdu);
	memcpy(&p_dev->DeviceCommType, &p_pdu->var, p_pdu->len);
	DbGetParaValue_MCP_C(pdevinfo, HEARTBEATTIME_ID, p_pdu);
	memcpy(&p_dev->HeartBeatTime, &p_pdu->var, p_pdu->len);

	DbGetParaValue_MCP_C(pdevinfo, OMCIP_ID, p_pdu);
	memcpy(&p_dev->OmcIP, &p_pdu->var, p_pdu->len);	
	
	DbGetParaValue_MCP_C(pdevinfo, OMCIPPORT_ID, p_pdu);
	memcpy(&p_dev->OmcIPPort, &p_pdu->var, p_pdu->len);
	DbGetParaValue_MCP_C(pdevinfo, DEVICEIP_ID, p_pdu);
	memcpy(&p_dev->DeviceIP, &p_pdu->var, p_pdu->len);

	DbGetParaValue_MCP_C(pdevinfo, DEVNETMASK_ID, p_pdu);
	memcpy(&p_dev->DeviceNetmask, &p_pdu->var, p_pdu->len);

	DbGetParaValue_MCP_C(pdevinfo, DEVDEFAULTGW_ID, p_pdu);
	memcpy(&p_dev->DeviceGateway, &p_pdu->var, p_pdu->len);

	DbGetParaValue_MCP_C(pdevinfo, DEVICEIPPORT_ID, p_pdu);
	memcpy(&p_dev->DeviceIPPort, &p_pdu->var, p_pdu->len);

	DbGetParaValue_MCP_C(pdevinfo, FTPSERVERIP_ID, p_pdu);
	memcpy(&p_dev->FtpServerIP, &p_pdu->var, p_pdu->len);
	DbGetParaValue_MCP_C(pdevinfo, FTPSERVERIPPORT_ID, p_pdu);
	memcpy(&p_dev->FtpServerIPPort, &p_pdu->var, p_pdu->len);

	//-------------2015_03_11-----��ѯip-----------------------
	if (g_DevType == MAIN_UNIT)
	{
		DbGetParaValue_MCP_C(pdevinfo, RW1_OMCIP_ID, p_pdu);
		memcpy(&p_dev->RW1_OmcIP, &p_pdu->var, p_pdu->len);
		DbGetParaValue_MCP_C(pdevinfo, RW2_OMCIP_ID, p_pdu);
		memcpy(&p_dev->RW2_OmcIP, &p_pdu->var, p_pdu->len);
	}
	//---------------------------------------------------------
	
	if (g_DevType == MAIN_UNIT)
	{
		SetDevIPpara("eth0:1", p_dev->DeviceIP, SET_IP);
		SetDevIPpara("eth0:1", p_dev->DeviceNetmask, SET_NETMASK);
		if (p_dev->ComModemType == NET8023)
		{
		  	SetDevIPpara("eth0:1", p_dev->DeviceGateway, SET_GATEWAY);
		}
	}
	else
	{
		SetDevIPpara("eth0", p_dev->DeviceIP, SET_IP);
		SetDevIPpara("eth0", p_dev->DeviceNetmask, SET_NETMASK);
		SetDevIPpara("eth0", p_dev->DeviceGateway, SET_GATEWAY);
	}
	DbGetParaValue_MCP_C(pdevinfo, SWUPDATERESULT_ID, p_pdu);
	memcpy(&p_dev->SWUpdateResult, &p_pdu->var, p_pdu->len);
	return 1;
}

/*******************************************************************************
*�������� : int DbGetAlarmValue(DevInfo_t *pdevinfo, char *buf)
*��    �� : �����ݿ��ȡ�澯����
*������� : DevInfo_t *pdevinfo:�豸���,�豸ģ���ַ,�豸ģ������;
            char *buf:��ȡ���ݴ�Ż���
*������� : �������ݳ���
*******************************************************************************/
int DbGetAlarmValue(DevInfo_t *pdevinfo, char *pbuf)
{
int i, len, result;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tbl_name[TBL_NAME_SIZE], var_name[TBL_VAR_SIZE];

	len = 0;
  if (DbGetTblVarName(pdevinfo, tbl_name, var_name) == 1)
  {
		sprintf(sql, "SELECT ID,%s FROM %s WHERE (ID LIKE \'0x0800158_\' OR ID LIKE \'0x0800458_\' OR ID LIKE \'0x0800758_\' OR ID LIKE \'0x0800858_\' OR ID LIKE \'0x0800006_\' OR ID LIKE \'0x0800060_\' OR ID LIKE \'0x0800_48_\' OR ID LIKE \'0x000003__\' OR ID LIKE  \'0x0800_00E\' OR ID LIKE  \'0x0800_00F\' OR ID LIKE  \'0x0000005_\' OR ID LIKE  \'0x0000006_\' OR ID LIKE  \'0x0000007_\' OR ID LIKE \'0x0800038_\') AND (%s LIKE \'1\' OR %s LIKE \'128\');"
			, var_name, tbl_name, var_name, var_name);
    if (SqliteSelect(sql, &sqlres) == SQLITE_OK)
    {
      if(sqlres.nRow > 0)
      {
        for(i = sqlres.nColumn; i < (sqlres.nRow + 1) * sqlres.nColumn;)
        {
        	*pbuf++ = 6;
          sscanf(sqlres.dbResult[i], "0x%8X", &result);//ID��ʶ
		  printf("%s\n", sqlres.dbResult[i]);
          *pbuf++ = (char)result;
          *pbuf++ = (char)(result >> 8);
          *pbuf++ = (char)(result >> 16);
          *pbuf++ = (char)(result >> 24);
          sscanf(sqlres.dbResult[i+1], "%d", &result);// V0
		  printf("%s\n", sqlres.dbResult[i+1]);
          *pbuf++ = (char)(result&0x1);
          len = len + 6;
          i = i + sqlres.nColumn;
        }
      }
    }
  }
  sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
  return len;
}
// ���Ժ�����������δ����
int DbGetAlarmValue_test(DevInfo_t *pdevinfo, char *pbuf)
{
	int i, len, result;
	SqlResult_t sqlres;
	char sql[SQL_CMD_SIZE], tbl_name[TBL_NAME_SIZE], var_name[TBL_VAR_SIZE];

	len = 0;
	if (DbGetTblVarName(pdevinfo, tbl_name, var_name) == 1)
	{
		sprintf(sql, "SELECT ID,%s FROM %s WHERE (ID LIKE \'0x0800158_\' OR ID LIKE \'0x0800458_\' OR ID LIKE \'0x0800758_\' OR ID LIKE \'0x0800858_\' OR ID LIKE \'0x0800006_\' OR ID LIKE \'0x0800060_\' OR ID LIKE \'0x0800_48_\' OR ID LIKE \'0x000003__\' OR ID LIKE  \'0x0800_00E\' OR ID LIKE  \'0x0800_00F\' OR ID LIKE  \'0x0000005_\' OR ID LIKE  \'0x0000006_\' OR ID LIKE  \'0x0000007_\' OR ID LIKE \'0x0800038_\');"
				, var_name, tbl_name);
		printf("%s\n", sql);
		if (SqliteSelect(sql, &sqlres) == SQLITE_OK)
		{
			if(sqlres.nRow > 0)
			{
				for(i = sqlres.nColumn; i < (sqlres.nRow + 1) * sqlres.nColumn;)
				{
					*pbuf++ = 6;
					sscanf(sqlres.dbResult[i], "0x%8X", &result);//ID��ʶ
					printf("%s\n", sqlres.dbResult[i]);
					*pbuf++ = (char)result;
					*pbuf++ = (char)(result >> 8);
					*pbuf++ = (char)(result >> 16);
					*pbuf++ = (char)(result >> 24);
					sscanf(sqlres.dbResult[i+1], "%d", &result);// V0
					printf("%s\n", sqlres.dbResult[i+1]);
					*pbuf++ = (char)(result&0x1);
					len = len + 6;
					i = i + sqlres.nColumn;
				}
			}
		}
	}
	sqlite3_free_table(sqlres.dbResult);//�ͷŵ�sqlres.dbResult���ڴ�ռ�
	return len;
}
/*******************************************************************************
*�������� : void SqlResultDis(SqlResult_t *psqlres)
*��     �� : ��ʾ�����ݿ��ȡ������
*������� : SqlResult_t *psqlres:���ݿ��ȡ�����ݽṹָ��
*������� : none
*******************************************************************************/
void SqlResultDis(SqlResult_t *psqlres)
{
int i;

  printf("row:%d; column=%d.\r\n", psqlres->nRow, psqlres->nColumn);
  for(i = 0; i < (psqlres->nRow+1)*psqlres->nColumn; i++)
    printf( "SqlResult.dbResult[%d]=%s\r\n", i, psqlres->dbResult[i]);
}
/*
** �������ܣ���ȡ���ݱ�����
** �����������
** ���������p=���ݱ���������
** ����ֵ��1=�ɹ� -1=ʧ��
** ��ע��
** ���ߣ�huyubin
*/
int get_tbl_name(char * p)
{
	int type = 0;

	type = get_device_type();
	if (type == MAIN_UNIT)
	{
		strcpy(p, RMU_TBL);
		return 1;
	}
	else if (type == EXPAND_UNIT)
	{
		strcpy(p, REU_TBL);
		return 1;
	}
	else if (type == RAU_UNIT)
	{
		strcpy(p, RAU_TBL);
		return 1;
	}
	else
	{
		return -1;
	}

}
/*
** �������ܣ���ȡ���ݱ��е�V0,������
** ���������id=���ݱ�ʶ
** ���������var=��������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��ԭʼ���ݣ�δ���κδ���CGIͨ��ʹ��,������ȡ
** ���ߣ�huyubin
*/
int sqlite_read_data(char * buf, int idx)
{
	int res;
	char sql[256];
	SqlResult_t sqlres;
	int ret = 0;
	int i = 0;
	char tbl_name[16];

	memset(tbl_name, 0, 16);
	if(get_tbl_name(tbl_name) < 0){
		return -1;
	}
	sprintf(sql, "select ID,V0 from %s limit %d,10;", tbl_name, idx*10);
	printf("%s\r\n", sql);
	res = SqliteSelect(sql, &sqlres);
	if(res == SQLITE_OK){
		if(sqlres.nRow == 0){
			ret = -1;// ���ݱ�����
			goto EXIT_HANDLE;
		}else{
			for(i = 1; i < sqlres.nRow+1; i++){
				strcat(buf, "&");
				strcat(buf, sqlres.dbResult[i*sqlres.nColumn]);
				strcat(buf, "=");
				strcat(buf, sqlres.dbResult[i*sqlres.nColumn+1]);
			}
			ret = 0;
			goto EXIT_HANDLE;
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}
// ��ȡ��������
int sqlite_read_data_ex(int id, char * var)
{
	int res;
	char sql[256];
	SqlResult_t sqlres;
	int ret = 0;
	char tbl_name[16];

	memset(tbl_name, 0, 16);
	if(get_tbl_name(tbl_name) < 0){
		return -1;
	}
	sprintf(sql, "select V0 from %s where id=\'0x%08X\';", tbl_name, id);
	printf("%s\r\n", sql);
	res = SqliteSelect(sql, &sqlres);
	if(res == SQLITE_OK){
		if(sqlres.nRow == 0){
			ret = -1;// ���ݱ�����
			goto EXIT_HANDLE;
		}else if(sqlres.nRow == 1){
			strcpy(var, (char *)sqlres.dbResult[sqlres.nColumn]);
			ret = 0;
			goto EXIT_HANDLE;
		}else{
			printf("���ݿ�����ͬ��ʾ�ļ�¼�ж���.\r\n");
			ret = -1; // 
			goto EXIT_HANDLE;
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}
/*
** �������ܣ��������ݱ��е�V0
** ���������id=��ʾ���� var=����
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע�� CGIͨ��ʹ��
** ���ߣ�huyubin
*/
int sqlite_write_data(char * id, char * var)
{
	int res;
	char sql[256];
	int cnt = 0;
	char tbl_name[16];

	memset(tbl_name, 0, 16);
	if(get_tbl_name(tbl_name) < 0){
		return -1;
	}
	sprintf(sql, "update %s set V0=\'%s\' where id=\'%s\';", tbl_name, var, id);
	printf("%s\r\n", sql);
	res = SqliteUpdate(sql);
	if(res == SQLITE_OK){
		SaveDbUpdate(sql);
		return 0;
	}
	return -1;
}

// ��ȡ���ݿ��м�¼�����������ܸ���
extern int SqliteGetCnt(char * tb_name);
int SqliteGetCntEx(void)
{
	int cnt = 0;
	char tbl_name[16];

	memset(tbl_name, 0, 16);
	if(get_tbl_name(tbl_name) < 0){
		return -1;
	}
	cnt = SqliteGetCnt(tbl_name);

	return cnt;
}
/*
** �������ܣ���ȡ���ݱ��е�ID��name
** ���������id��name
** �����������
** ����ֵ��0=�ɹ� ����=ʧ��
** ��ע��CGI
** ���ߣ�huyubin
*/
int SqliteReadList(char * buf, int idx)
{
	int res;
	char sql[256];
	SqlResult_t sqlres;
	int ret = 0;
	int i = 0;
	char tbl_name[16];

	memset(tbl_name, 0, 16);
	if(get_tbl_name(tbl_name) < 0){
		return -1;
	}
	sprintf(sql, "select ID,name,var_type,coefficient from %s limit %d,10 ;", tbl_name, idx*10);
	printf("%s\r\n", sql);
	res = SqliteSelect(sql, &sqlres);
	if(res == SQLITE_OK){
		if(sqlres.nRow == 0){
			ret = -1;// ���ݱ�����
			goto EXIT_HANDLE;
		}else{
			for(i = 1; i < sqlres.nRow+1; i++){
				strcat(buf, "&");
				strcat(buf, sqlres.dbResult[i*sqlres.nColumn]); // id
				strcat(buf, "=");
				strcat(buf, sqlres.dbResult[i*sqlres.nColumn+1]); // name
				strcat(buf, ",");
				strcat(buf, sqlres.dbResult[i*sqlres.nColumn+2]); // var_type
				strcat(buf, ",");
				strcat(buf, sqlres.dbResult[i*sqlres.nColumn+3]); // coefficient
			}
			ret = 0;
			goto EXIT_HANDLE;
		}
	}
	ret = -2;
EXIT_HANDLE:
	sqlite3_free_table(sqlres.dbResult);
	return ret;
}

/********************  COPYRIGHT(C) ***************************************/
