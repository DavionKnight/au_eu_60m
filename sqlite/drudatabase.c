/********************  COPYRIGHT(C)***************************************
**--------------文件信息--------------------------------------------------------
**文   件   名: drudatabase.c
**创   建   人: 于宏图
**创 建  日 期:
**程序开发环境：
**描        述: 数据库处理程序
**--------------历史版本信息----------------------------------------------------
** 创建人: 于宏图
** 版  本: v1.0
** 日　期:
** 描　述: 原始版本
**--------------当前版本修订----------------------------------------------------
** 修改人:
** 版  本:
** 日　期:
** 描　述:
**------------------------------------------------------------------------------
**
*******************************************************************************/
#include "../protocol/approtocol.h"
#include "../net/netcom.h"
#include "drudatabase.h"

pthread_mutex_t mutex_flash_savedb;//存储数据库锁
extern int g_DevType;
extern DevicePara_t g_DevicePara;

/*******************************************************************************
*函数名称 : int DbTableInit(char *tblname)
*功     能 : 数据库数据初始化
*输入参数 : char *tblname:数据表名称
*输出参数 : 成功返回1,出错返回<0
*******************************************************************************/
int DbTableInit(char *tblname)
{
int res, result;
unsigned int objectid;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], sbuf[250];
FILE *fp_cfg = NULL;

  //查找tbl数据表是否存在,如不存在则建立
  sprintf(sql, "SELECT * FROM sqlite_master WHERE name=\'%s\';", tblname);
  res = SqliteSelect(sql, &sqlres);
  if(res == SQLITE_OK)
  {
    if(sqlres.nRow == 0)//tblname数据表不存在
    {
      sprintf(sbuf, "%s/%s_cfg.txt", CONFIGPATH, tblname);
      fp_cfg = fopen(sbuf, "r");
      if(fp_cfg == NULL) //需要打开的文件不存在,重新建立
      {
        DEBUGOUT("%s does not exist,Creat!\r\n", sbuf);
        fp_cfg = fopen(sbuf, "w+");
        sprintf(sbuf, "(%s,V0);", TABLE_HEAD);
        fwrite(sbuf, strlen(sbuf), 1, fp_cfg);//创建配置文件数据表头
      }
      rewind(fp_cfg);//指向文件头
      fgets(sbuf, sizeof(sbuf), fp_cfg);//逐行读取文件
      fclose(fp_cfg);
      sprintf(sql, "CREATE TABLE %s %s", tblname, sbuf);
      res = SqliteCreate(sql);//创建sqtbl数据表
      if(res != SQLITE_OK)//创建sqtbl数据表失败
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
    fgets(sbuf, sizeof(sbuf), fp_cfg);//逐行读取文件
    while(!feof(fp_cfg))
    {
      memset(sbuf, 0, sizeof(sbuf));
      fgets(sbuf, sizeof(sbuf), fp_cfg);//逐行读取文件
      if(strstr(sbuf, ");") != NULL)//有结束标志
      {
        sscanf(strstr(sbuf, "0x"), "0x%08X", &objectid);//标识
        sprintf(sql, "SELECT * FROM %s WHERE ID =\"0x%08X\";", tblname, objectid);
		printf(sql);
        res = SqliteSelect(sql, &sqlres);
        if (res == SQLITE_OK)
        {
          if(sqlres.nRow == 0)
          {
            DEBUGOUT("INSERT INTO %s VALUES %s", tblname, sbuf);
            sprintf(sql, "INSERT INTO %s VALUES %s", tblname, sbuf);
            res = SqliteInsert(sql);//增加新参数
            if(res != SQLITE_OK)
            {
              fclose(fp_cfg); //关闭文件
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
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}

/*******************************************************************************
*函数名称 : int SystemDataInit(void)
*功     能 : 系统数据初始化
*输入参数 : None
*输出参数 : None
*******************************************************************************/
int DataBaseInit(void)
{
int  res;
char SaveDbName[100], RamDbName[100], TableName[20], sql[SQL_CMD_SIZE];

	pthread_mutex_init(&mutex_flash_savedb, NULL);//互斥锁初始化
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
	if(res == SQLITE_OK)//初始化所有监控参数数据库
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
		if(res != SQLITE_OK)//初始化所有监控参数数据库
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
*函数名称 : int DbGetIDList(DevInfo_t *pdevinfo, int *pbuf)
*功     能 : 从数据库读取监控列表数据
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型
*         : int *pbuf:pdevinfo所指设备对应参数列表
*输出参数 : 正确返回参数总数,数据表中未查到该数据-1,数据查找出错-2
*******************************************************************************/
int DbGetIDList(DevInfo_t *pdevinfo, int *pbuf)
{
int   i, result;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tbl_name[TBL_NAME_SIZE], var_name[TBL_VAR_SIZE];

  if (DbGetTblVarName(pdevinfo, tbl_name, var_name) == 1)
  {
    //在sqtbl数据表查找ID对应数据
    sprintf(sql, "SELECT ID,%s FROM %s;", var_name, tbl_name);
    if (SqliteSelect(sql, &sqlres) == SQLITE_OK)
    {
      DEBUGOUT("tbl:%s,var:%s.GetIDList.\r\n", tbl_name, var_name);
      if(sqlres.nRow > 0)
      {
        for(i = sqlres.nColumn; i < (sqlres.nRow + 1) * sqlres.nColumn;)
        {
          sscanf(sqlres.dbResult[i], "0x%4X", pbuf++);//ID标识
          i = i + sqlres.nColumn;
        }
        result = sqlres.nRow;
      }
      else//数据表中未查到该数据
      {
        result = -1;
      }
    }
    else//数据查找出错
    {
      result = -2;
    }
  }
  else
  {
    result = -3;
  }
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}
// 功     能 : 从数据库读取监控列表数据MCPC ID
int DbGetIDList_MCP_C(DevInfo_t *pdevinfo, int *pbuf)
{
	int   i, result;
	SqlResult_t sqlres;
	char sql[SQL_CMD_SIZE], tbl_name[TBL_NAME_SIZE], var_name[TBL_VAR_SIZE];
	int author = 0;

  if(DbGetThisIntPara(AUTHOR_ID, &author) > 0){ // 读取操作权限参数 0:网管参数 3：所有参数
	  if((author > 0) && (author < 4)){
	  }else{
		  author = 0;
	  }
  }
  if (DbGetTblVarName(pdevinfo, tbl_name, var_name) == 1)
  {
    //在sqtbl数据表查找ID对应数据
    sprintf(sql, "SELECT ID,%s FROM %s where pro<%d;", var_name, tbl_name, (author+1)*10);
    if (SqliteSelect(sql, &sqlres) == SQLITE_OK)
    {
      DEBUGOUT("tbl:%s,var:%s.GetIDList.\r\n", tbl_name, var_name);
      if(sqlres.nRow > 0)
      {
        for(i = sqlres.nColumn; i < (sqlres.nRow + 1) * sqlres.nColumn;)
        {
          sscanf(sqlres.dbResult[i], "0x%8X", pbuf++);//ID标识
          i = i + sqlres.nColumn;
        }
        result = sqlres.nRow;
      }
      else//数据表中未查到该数据
      {
        result = -1;
      }
    }
    else//数据查找出错
    {
      result = -2;
    }
  }
  else
  {
    result = -3;
  }
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}
/*******************************************************************************
*函数名称 : int DbGetParaValue(DevInfo_t *pdevinfo, INT16U objectid, Pdu_t *p_pdu)
*功     能 : 从数据库读取存储的数据
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;
            INT16U objectid:参数ID号;Pdu_t *p_pdu:参数对应数据结构参数指针
*输出参数 : 正确返回1,否则出错<0
*******************************************************************************/
int DbGetParaValue(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu)
{
int i, res, result, buf[7];
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

  //根据设备编号、端口号、模块类型,获取数据库参,数名称
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
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//标识
        p_pdu->id = res;
        strcpy(p_pdu->name, sqlres.dbResult[sqlres.nColumn + TBL_NAME]); //参数名称
        strcpy(p_pdu->mode, sqlres.dbResult[sqlres.nColumn + TBL_MODE]); //RO/RW模式
        p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //数据长度
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //数据类型
        p_pdu->coefficient = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]); //参数系数
        //printf("pdu:id:%d,name:%s,mode:%s,len:%d,type:%s,coe:%d\r\n",
        //       p_pdu->id,p_pdu->name,p_pdu->mode,p_pdu->len,p_pdu->var_type,p_pdu->coefficient);
        memset(buf, 0, sizeof(buf));
        if(strcmp(p_pdu->var_type, "str") == 0)//字符串
        {
          strncpy(p_pdu->var, sqlres.dbResult[sqlres.nColumn + TBL_VAR], p_pdu->len);
        }
        else if(strcmp(p_pdu->var_type, "dstr") == 0)//数据串
        {
          //IP地址
          if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
            ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID)|| (p_pdu->id == DEVROUTE_ID))
          {
            sscanf(sqlres.dbResult[sqlres.nColumn + TBL_VAR], "%d.%d.%d.%d",
                   &buf[0], &buf[1], &buf[2], &buf[3]);
            for(i = 0; i < p_pdu->len; i++)
              p_pdu->var[i] = (char)buf[i];
          }
          //设备当前时间批采开始时间,7个字节组成
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
          res = atoi(sqlres.dbResult[sqlres.nColumn + TBL_VAR]);//数据
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
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}

// 读取参数的属性字段，判断该参数是否需要广播
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
				ret = -1;// 数据表不存在
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
// 扩展单元查询他的下一级相应端口下的远端是否有告警
// 做远端接入状态时使用 ID = 0x08000080
int SqliteGetAlarmCnt(int port_num)
{
	int res;
	SqlResult_t sqlres;
	int ret = 0;
	char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];
	DevInfo_t devinfo;

	memset((char *)&devinfo, 0, sizeof(devinfo));
	// 查询告警参数，是否有告警
	if (DbGetTblVarName(&devinfo, tblname, varname) == 1){
		sprintf(sql, "select count(*) from %s where ((id like \'0x0800038%d\') or (id like \'0x0800060%d\') or (id like \'0x0800_48%d\') or (id like \'0x0800_58%d\' and id!='0x0800058%d') or (id like \'0x0800006%d\')) and (V0 like '1' or V0 like '129');", tblname, port_num, port_num, port_num, port_num, port_num, port_num);
		res = SqliteSelect(sql, &sqlres);
		if(res == SQLITE_OK){
			if(sqlres.nRow == 0){
				ret = -1;// 数据表不存在
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

// 查询属性字段个位是2的参数个数，（扩展查询远端参数）
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
				ret = -1;// 数据表不存在
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

// 查询属性字段个位是3的参数个数，（扩展设置远端参数）
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
				ret = -1;// 数据表不存在
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

// 将属性字段个位是2/3的参数读取出来，放到内存中，需要经常更新
// type=2(扩展查询远端参数) 3（扩展设置远端参数）
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
				ret = -1;// 数据表不存在
				goto EXIT_HANDLE;
			}else if(sqlres.nRow > 0){
				idx = 0;
//				printf("get init para  nColum=%d, nRow=%d\r\n", sqlres.nColumn, sqlres.nRow);
				for(i = sqlres.nColumn; i < (sqlres.nRow+1)*sqlres.nColumn; ){
//					printf("get init para id: %s\r\n", sqlres.dbResult[i]);
					sscanf(sqlres.dbResult[i++], "0x%08X", &id);//标识
//					printf("id=0x%08x\n", id);
					memcpy(buf+idx, (char *)&id, 4);
					idx += 4;
//					printf("get init para len: %s\r\n", sqlres.dbResult[i]);
					sscanf(sqlres.dbResult[i++], "%d", &temp);//标识 len
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
// 读取需要广播设置参数, 属性字段个位是1
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
				ret = -1;// 数据表不存在
				goto EXIT_HANDLE;
			}else if(sqlres.nRow > 0){
				pbuf = buf;
				len = 0;
				for(i = sqlres.nColumn; i < (sqlres.nRow+1)*sqlres.nColumn; ){
					sscanf(sqlres.dbResult[i++], "0x%08X", &id);//标识
					memcpy(pbuf+1, (char *)&id, 4);
					sscanf(sqlres.dbResult[i++], "%d", &temp);//标识 LEN
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
// 读取广播状态参数,属性字段个位是4
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
				ret = -1;// 数据表不存在
				goto EXIT_HANDLE;
			}else if(sqlres.nRow > 0){
				pbuf = buf;
				len = 0;
				for(i = sqlres.nColumn; i < (sqlres.nRow+1)*sqlres.nColumn; ){
					sscanf(sqlres.dbResult[i++], "0x%08X", &id);//标识
					memcpy(pbuf+1, (char *)&id, 4);
					sscanf(sqlres.dbResult[i++], "%d", &temp);//标识 LEN
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
// 读取参数值，MCPC
int DbGetParaValue_MCP_C(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu)
{
int i, res, result, buf[7];
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

  //根据设备编号、端口号、模块类型,获取数据库参,数名称
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
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//标识
        p_pdu->id = res;
        strcpy(p_pdu->name, sqlres.dbResult[sqlres.nColumn + TBL_NAME]); //参数名称
        strcpy(p_pdu->mode, sqlres.dbResult[sqlres.nColumn + TBL_MODE]); //RO/RW模式
        p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //数据长度
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //数据类型
        p_pdu->coefficient = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]); //参数系数
//        p_pdu->min = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]);;		  //最小值
//  			p_pdu->max = atoi(sqlres.dbResult[sqlres.nColumn + TBL_COEF]);;          //最大值
        //printf("pdu:id:%d,name:%s,mode:%s,len:%d,type:%s,coe:%d\r\n",
        //       p_pdu->id,p_pdu->name,p_pdu->mode,p_pdu->len,p_pdu->var_type,p_pdu->coefficient);
        memset(buf, 0, sizeof(buf));
        if(strcmp(p_pdu->var_type, "str") == 0)//字符串
        {
          strncpy(p_pdu->var, sqlres.dbResult[sqlres.nColumn + TBL_VAR], p_pdu->len);
        }
        else if(strcmp(p_pdu->var_type, "dstr") == 0)//数据串
        {
          //IP地址
          if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
            ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID)|| (p_pdu->id == DEVROUTE_ID)
            ||(p_pdu->id ==RW1_OMCIP_ID) ||(p_pdu->id ==RW2_OMCIP_ID))
          {
            sscanf(sqlres.dbResult[sqlres.nColumn + TBL_VAR], "%d.%d.%d.%d",
                   &buf[0], &buf[1], &buf[2], &buf[3]);
            for(i = 0; i < p_pdu->len; i++)
              p_pdu->var[i] = (char)buf[i];
          }
          //设备当前时间批采开始时间,7个字节组成
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
          res = atoi(sqlres.dbResult[sqlres.nColumn + TBL_VAR]);//数据
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
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}

/*******************************************************************************
*函数名称 : int SaveDbUpdate(const char *sql)
*功    能 : 数据库数据存储更新
*输入参数 : sql:更新数据库SQL语句
*输出参数 : SQLITE_OK：执行成功;否则，执行失败
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
    printf("数据存储文件打开失败.\n");//数据库打开失败
    return result;
  }
  //DEBUGOUT("数据存储:%s\r\n", sql);
  result = sqlite3_exec(savedb, sql, NULL, NULL, &errmsg);
  if (result != SQLITE_OK)
  {
    printf("数据存储更新记录失败,错误码:%d,错误原因:%s\r\n", result, errmsg);
  }
  sqlite3_close(savedb);
  pthread_mutex_unlock(&mutex_flash_savedb);
  return result;
}

/*******************************************************************************
*函数名称 : int DbSaveParaValue(DevInfo_t *pdevinfo, INT16U objectid, Pdu_t *p_pdu)
*功    能 : 更新数据库的数据
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;
            INT16U objectid:参数ID号;Pdu_t *p_pdu:参数对应数据结构参数指针
*输出参数 : 更新成功返回1,未更新成功返回-1,数据长度不符-2,数据表中未查到该数据-3,数据查找出错-4
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
    //在tblname数据表查找ID对应数据
    sprintf(sql, "SELECT %s,%s FROM %s WHERE ID =\"0x%08X\";", TABLE_HEAD, varname, tblname, objectid);
    res = SqliteSelect(sql, &sqlres);
    if (res == SQLITE_OK)
    {
      //SqlResultDis(&sqlres);
      if(sqlres.nRow > 0)
      {
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//标识
        p_pdu->id = res;
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //数据类型
        if(p_pdu->len == atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]))//长度相符合
        {
          memset(strbuf, 0, sizeof(strbuf));
          if(strcmp(p_pdu->var_type, "str") == 0)//字符串
          {
            strncpy(strbuf, p_pdu->var, p_pdu->len);//字符串
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else if(strcmp(p_pdu->var_type, "dstr") == 0)//数据串
          {
            //IP地址
            if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
              ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID) || (p_pdu->id == DEVROUTE_ID))
            {
              sprintf(strbuf, "%d.%d.%d.%d", p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3]);
            }
            //设备当前时间批采开始时间,7个字节组成
            else if ((p_pdu->id == DEVICETIME_ID) || (p_pdu->id == SAMPLESTARTTIME_ID))
            {
              sprintf(strbuf, "%02X%02X%02X%02X %02X:%02X:%02X", 
                      p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3], p_pdu->var[4], p_pdu->var[5], p_pdu->var[6]);
            }
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else//uint1,uint2,uint3,uint4,sint1,sint2,bit
          {
            if(strstr(p_pdu->var_type, "sint1") != NULL)//sint1有符号数
            {
            	sint8 = 0;
              memcpy(&sint8, p_pdu->var, p_pdu->len);
              res = sint8;
            }
            else if(strstr(p_pdu->var_type, "sint2") != NULL)//sint2有符号数
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
//            p_pdu->min == atoi(sqlres.dbResult[sqlres.nColumn + TBL_MIN]))//参数最小值
//            p_pdu->max == atoi(sqlres.dbResult[sqlres.nColumn + TBL_MAX]))//参数最大值
//						if ((res < p_pdu->min) || (res > p_pdu->max))
//						{
//							sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
//							return -6;
//						}
//						else
//						{
//            	sprintf(sql, "UPDATE %s SET %s=%d WHERE ID=\"0x%08X\";", tblname, varname, res, objectid);
//            }
            sprintf(sql, "UPDATE %s SET %s=%d WHERE ID=\"0x%08X\";", tblname, varname, res, objectid);
          }
          //DEBUGOUT("SqliteUpdate:%s\r\n", sql);
          res = SqliteUpdate(sql);//更新sqtbl数据表中objectid对应数据
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
          p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //数据长度
          DEBUGOUT("DbSaveParaValue:Para Len Error!\r\n");
          result = -2;
        }
      }
      else//数据表中未查到该数据
      {
        DEBUGOUT("DbSaveParaValue:%s,%s ID:0x%08X no Find!\r\n",varname, tblname, objectid);
        result = -3;
      }
    }
    else//数据查找出错
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
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}

// 保存数据MCPC
// flag=0(需要同步到FLASH中） 1（不需要同步到FLASH，例如输入输出功率，温度等参数）
int DbSaveParaValue_MCP_C(DevInfo_t *pdevinfo, unsigned int objectid, Pdu_t *p_pdu, int flag)
{
int res, result;
INT8S sint8;
INT16S sint16;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE], strbuf[21];

  if (DbGetTblVarName(pdevinfo, tblname, varname) == 1)
  {
    //在tblname数据表查找ID对应数据
    sprintf(sql, "SELECT %s,%s FROM %s WHERE ID =\"0x%08X\";", TABLE_HEAD, varname, tblname, objectid);
    res = SqliteSelect(sql, &sqlres);
    if (res == SQLITE_OK)
    {
      //SqlResultDis(&sqlres);
      if(sqlres.nRow > 0)
      {
        sscanf(sqlres.dbResult[sqlres.nColumn + TBL_ID], "0x%08X", &res);//标识
        p_pdu->id = res;
        strcpy(p_pdu->var_type, sqlres.dbResult[sqlres.nColumn + TBL_VARTYPE]); //数据类型
        if(p_pdu->len == atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]))//长度相符合
        {
          memset(strbuf, 0, sizeof(strbuf));
          if(strcmp(p_pdu->var_type, "str") == 0)//字符串
          {
            strncpy(strbuf, p_pdu->var, p_pdu->len);//字符串
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else if(strcmp(p_pdu->var_type, "dstr") == 0)//数据串
          {
            //IP地址
            if ((p_pdu->id == OMCIP_ID) || (p_pdu->id == FTPSERVERIP_ID) || (p_pdu->id == DEVICEIP_ID)
              ||(p_pdu->id == DEVNETMASK_ID) || (p_pdu->id == DEVDEFAULTGW_ID) || (p_pdu->id == DEVROUTE_ID)
              ||(p_pdu->id == RW1_OMCIP_ID) ||(p_pdu->id ==RW2_OMCIP_ID) )
            {
              sprintf(strbuf, "%d.%d.%d.%d", p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3]);
            }
            //设备当前时间批采开始时间,7个字节组成
            else if ((p_pdu->id == DEVICETIME_ID) || (p_pdu->id == SAMPLESTARTTIME_ID))
            {
              sprintf(strbuf, "%02X%02X%02X%02X %02X:%02X:%02X", 
                      p_pdu->var[0], p_pdu->var[1], p_pdu->var[2], p_pdu->var[3], p_pdu->var[4], p_pdu->var[5], p_pdu->var[6]);
            }
            sprintf(sql, "UPDATE %s SET %s=\"%s\" WHERE ID=\"0x%08X\";", tblname, varname, strbuf, objectid);
          }
          else//uint1,uint2,uint3,uint4,sint1,sint2,bit
          {
            if(strstr(p_pdu->var_type, "sint1") != NULL)//sint1有符号数
            {
            	sint8 = 0;
              memcpy(&sint8, p_pdu->var, p_pdu->len);
              res = sint8;
            }
            else if(strstr(p_pdu->var_type, "sint2") != NULL)//sint2有符号数
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
          res = SqliteUpdate(sql);//更新sqtbl数据表中objectid对应数据
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
          p_pdu->len = atoi(sqlres.dbResult[sqlres.nColumn + TBL_LEN]); //数据长度
		  printf("drudatabase.c DbSaveParaValue_MCP_C(), %d, 0x%08X\n", p_pdu->len, objectid);
          DEBUGOUT("DbSaveParaValue:Para Len Error!\r\n");
          result = -2;
        }
      }
      else//数据表中未查到该数据
      {
        DEBUGOUT("DbSaveParaValue:%s,%s ID:0x%08X no Find!\r\n",varname, tblname, objectid);
        result = -3;
      }
    }
    else//数据查找出错
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
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}
/*******************************************************************************
*函数名称 : int DbGetThisIntPara(INT16U objectid, int *pval)
*功    能 : 获取本机整形参数
*输入参数 : INT16U objectid:参数ID号;int *pval:参数返回值指针
*输出参数 : 更新成功返回1,未更新成功返回-1
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
    if(strstr(pdu.var_type, "sint1") != NULL)//sint1有符号数
    {
      memcpy(&sint8, &pdu.var, pdu.len);
      *pval = sint8;
    }
    else if(strstr(pdu.var_type, "sint2") != NULL)//sint2有符号数
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
*函数名称 : int DbSaveThisIntPara(INT16U objectid, int val)
*功    能 : 存储本机整形参数
*输入参数 : INT16U objectid:参数ID号;int val:参数值
*输出参数 : 更新成功返回1,未更新成功返回-1
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

// 存储本机整形参数 flag=0 需要同步到FLASH中，flag=1不需要同步到FLASH
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
*函数名称 : int DbSaveThisStrPara(INT16U objectid, char *buf)
*功    能 : 存储本机字符串型参数
*输入参数 : INT16U objectid:参数ID号;char *buf:参数值指针
*输出参数 : 更新成功返回1,未更新成功返回-1
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
// 读取字符串参数
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
*函数名称 : int DbGetTblName(DevInfo_t *pdevinfo, char *tblname)
*功     能 : 根据设备号,模块类型、地址,获取数据库中表单名
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;char *tblname:获取数据库中表单名
*输出参数 : 正常返回1,错误返回-1
*******************************************************************************/
int DbGetTblName(DevInfo_t *pdevinfo, char *tblname)
{
  *tblname = 0;
  switch(pdevinfo->ModuleType)
  {
    //主单元参数
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
*函数名称 : int DbGetTblName(DevInfo_t *pdevinfo, char *varname)
*功     能 : 根据设备号,模块类型、地址,获取数据库中表单中的变量名
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;char *varname:获取数据库中表单中的变量名
*输出参数 : 正常返回1,错误返回-1
*******************************************************************************/
int DbGetVarName(DevInfo_t *pdevinfo, char *varname)
{
INT16U mtype;

  *varname = 0;
  mtype = pdevinfo->ModuleType;
  //只有主单元、扩展单元、远程监控单元具有系统参数,port地址不复用
  if (mtype == 0x00)
  {
    if ((pdevinfo->DeviceNo == BROADCAST_DEV)//本设备,如主单元
      ||(pdevinfo->DeviceNo == g_DevicePara.DeviceNo))
    {
      strcpy(varname, "V0");
      return 1;
    }
    else
    {
      sprintf(varname, "V%d", pdevinfo->DeviceNo);//查找从机设备,V1,V2...
      if (pdevinfo->DeviceNo == 0)//查询本机参数
      	return 1;
      else
       return -1;
    }
  }
  return -1;
}

/*******************************************************************************
*函数名称 : int DbGetTblVarName(DevInfo_t *pdevinfo, char *tblname, char *varname)
*功     能 : 根据设备号,模块类型、地址,获取数据库中表单名以及变量名
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;
*           char *tblname:获取数据库中表单名;char *varname:获取数据库中表单中的变量名
*输出参数 : 正常返回1,错误返回-1
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
*函数名称 : int DbAddVarName(DevInfo_t *pdevinfo)
*功     能 : 根据设备号,设备端口号,模块类型,在数据库中表头增加一项变量
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;
*输出参数 : 正常建立表头项返回1,否则返回-1
*******************************************************************************/
int DbAddVarName(DevInfo_t *pdevinfo)
{
int res, result;
SqlResult_t sqlres;
char sql[SQL_CMD_SIZE], tblname[TBL_NAME_SIZE], varname[TBL_VAR_SIZE];

  //根据设备编号、端口号、模块类型,获取数据库名称
  if (DbGetTblVarName(pdevinfo, tblname, varname) == 1)
  {
    //在tblname数据表查找varname对应数据
    sprintf(sql, "SELECT %s FROM %s;", varname, tblname);
    res = SqliteSelect(sql, &sqlres);
    //在数据表tblname中对应varname表头项不存在
    if (res != SQLITE_OK)
    {
      //在数据表tblname中添加varname表头项
      sprintf(sql, "ALTER TABLE %s ADD COLUMN %s;", tblname, varname);
      DEBUGOUT("DbAddVarName:%s.\r\n", sql);
      res = SqliteInsert(sql);//记录插入数据库
      if (res == SQLITE_OK)//所有数据置为0
      {
        sprintf(sql, "UPDATE %s SET %s=0;", tblname, varname);
        res = SqliteUpdate(sql);//更新sqtbl数据表中对应数据
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
      //参数已经存在,无需增加
      result = 1;
    }
  }
  else
  {
    DEBUGOUT("DbAddVarName:DbGetTblVarName Error.\r\n");
    result = -2;
  }
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return result;
}

/*******************************************************************************
*函数名称 : int LoadDevicePara(DevInfo_t *pdevinfo, DevicePara_t *p_dev)
*功     能 : 从数据库读取存储需要的数据
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;
            DevicePara_t *p_dev:设备结构体对应参数
*输出参数 : 更新成功返回1,未更新成功返回-1
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
	//与MODEM相关参数
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

	//-------------2015_03_11-----查询ip-----------------------
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
*函数名称 : int DbGetAlarmValue(DevInfo_t *pdevinfo, char *buf)
*功    能 : 从数据库读取告警数据
*输入参数 : DevInfo_t *pdevinfo:设备编号,设备模块地址,设备模块类型;
            char *buf:读取数据存放缓存
*输出参数 : 返回数据长度
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
          sscanf(sqlres.dbResult[i], "0x%8X", &result);//ID标识
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
  sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
  return len;
}
// 测试函数，程序中未调用
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
					sscanf(sqlres.dbResult[i], "0x%8X", &result);//ID标识
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
	sqlite3_free_table(sqlres.dbResult);//释放掉sqlres.dbResult的内存空间
	return len;
}
/*******************************************************************************
*函数名称 : void SqlResultDis(SqlResult_t *psqlres)
*功     能 : 显示从数据库读取的数据
*输入参数 : SqlResult_t *psqlres:数据库读取的数据结构指针
*输出参数 : none
*******************************************************************************/
void SqlResultDis(SqlResult_t *psqlres)
{
int i;

  printf("row:%d; column=%d.\r\n", psqlres->nRow, psqlres->nColumn);
  for(i = 0; i < (psqlres->nRow+1)*psqlres->nColumn; i++)
    printf( "SqlResult.dbResult[%d]=%s\r\n", i, psqlres->dbResult[i]);
}
/*
** 函数功能：获取数据表名称
** 输入参数：无
** 输出参数：p=数据表名称内容
** 返回值：1=成功 -1=失败
** 备注：
** 作者：huyubin
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
** 函数功能：读取数据表中的V0,数据项
** 输入参数：id=数据标识
** 输出参数：var=数据内容
** 返回值：0=成功 其他=失败
** 备注：原始数据，未作任何处理，CGI通信使用,批量读取
** 作者：huyubin
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
			ret = -1;// 数据表不存在
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
// 读取单个参数
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
			ret = -1;// 数据表不存在
			goto EXIT_HANDLE;
		}else if(sqlres.nRow == 1){
			strcpy(var, (char *)sqlres.dbResult[sqlres.nColumn]);
			ret = 0;
			goto EXIT_HANDLE;
		}else{
			printf("数据库中相同标示的记录有多条.\r\n");
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
** 函数功能：更新数据表中的V0
** 输入参数：id=标示内容 var=数据
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注： CGI通信使用
** 作者：huyubin
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

// 读取数据库中记录条数，参数总个数
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
** 函数功能：读取数据表中的ID和name
** 输入参数：id和name
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：CGI
** 作者：huyubin
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
			ret = -1;// 数据表不存在
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
