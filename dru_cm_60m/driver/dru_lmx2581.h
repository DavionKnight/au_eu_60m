/*******************************************************************************
********************************************************************************
* 文件名称:  dru_lmx2531.h
* 功能描述:  有关于lmx2531的配置操作的宏定义及函数声明
* 使用说明: 	 所有和lmx2531直接相关的定义都在这个文件中定义
* 文件作者:	H4
* 编写日期: （2012/06/18）
* 修改历史:
* 修改日期    修改人       修改内容
*-------------------------------------------------------------------------------

*******************************************************************************/
#ifndef _DRU_LMX2581_H
#define _DRU_LMX2581_H

#define SELECT_LMX2581 3
#define LMX2581_REG_LENGTH 24

int dru_lmx2581_config(unsigned int freq,unsigned int ch);
void dru_lm2581_init(void);
int dru_lmx2581_config_change(int argc, char * argv[]);
#endif
