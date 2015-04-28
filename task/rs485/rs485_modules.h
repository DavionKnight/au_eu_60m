#ifndef _RS485_MODULES_H_
#define _RS485_MODULES_H_

// led状态数据内容结构
struct led_data{
	unsigned char len;		//���ݳ���
	unsigned short id;		//����id
	unsigned char sta;		//����
};

/*
** 函数功能：远端模块参数数据库初始化
** 输入参数：config_name=配置文件名称
** 输出参数：无
** 返回值：1=成功 其他=失败
** 备注：
*/
extern int ru_para_init(void);
/*
** 函数功能: 刷新参量列表
** 输入参数：无
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int read_id_list(void);
/*
** 函数功能: 校验模块地址和类型
** 输入参数：tb_name=数据表名称 len=数据长度(len,id和data总长度) id=数据ID data=数据内容
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int check_module_type(unsigned char addr, unsigned short int type);
/*
** 函数功能: 将参量列表存入数据库
** 输入参数：tb_name=数据表名称 len=数据长度(len,id和data总长度) id=数据ID data=数据内容
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int write_id_list(int len, int id, unsigned char * data);
/*
** 函数功能: 查询所有模块的所有数据
** 输入参数：无
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int read_para_all(void);
/*
** 函数功能: 将参数存入数据库
** 输入参数：tb_name=数据表名称 len=数据长度(len,id和data总长度) id=数据ID data=数据内容
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int write_para(int len, int id, unsigned char * data);
/*
** 函数功能: 将告警状态存入数据库
** 输入参数：id=数据ID data=数据内容
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int write_alarm(int id, unsigned char * data);

/*
** 函数功能: 数据转义，发送
** 输入参数：tb_name=数据表名称 len=数据长度(len,id和data总长度) id=数据ID data=数据内容
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int data_change(unsigned char * buf, int len);
/*
** 函数功能: 设置灯板
** 输入参数：无
** 输出参数：无
** 返回值：0=成功 其他=失败
** 备注：
*/
extern int set_led_power(void);
extern int set_led_led(void);

#endif
