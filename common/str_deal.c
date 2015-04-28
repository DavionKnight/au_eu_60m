/*******************************************************************************
********************************************************************************
* �ļ�����:  str_deal.c
* ��������:  �ַ����߼�����
* ʹ��˵��:  
* �ļ�����:	H4
* ��д����: ��2013/04/11��
* �޸���ʷ:
* �޸�����    �޸���       �޸�����
*-------------------------------------------------------------------------------

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#define MAX_STRING_LENGTH 200
/*******************************************************************************
* ��������: strdelnell
* ��    ��: ȥ���ַ����в��ɼ��ַ���ASC||��
* ��    ��: oarg sarg
* ��������         ����                ����
*	darg					char *						Ŀ�Ĵ���ַ
* sarg					const char * 			Դ����ַ
* length				unsigned int      Դ�����ȣ�ӦС�ڻ����Ŀ�Ĵ�����
* ����ֵ:	
* 0:�ɹ�   -1�����볤�ȴ�
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2013/04/11  V1.0     H4     ��       ��
*******************************************************************************/
int strdelnull(const char* sarg,char *darg,unsigned int length)
{
	int i,j=0;
	for(i=0;i<length;i++){
		if((sarg[i]>='!')&&(sarg[i]<='~')){
			darg[j]=sarg[i];
			j++;
		}
	}
	return 0;
}
/*******************************************************************************
* ��������: strcmpasc
* ��    ��: ȥ���������в��ɼ��ַ���Ƚϴ�
* ��    ��: str1 str2
* ��������         ����                ����
*	str1					const char *						Ŀ�Ĵ���ַ
* str2					const char * 			Դ����ַ
* ����ֵ:	
* 0:��ͬ   -1��1��2������Ϊ����ͬ
* ˵   ��:
* ��   ��     �汾    ����   �޸���      DEBUG
* -----------------------------------------------------------------
* 2013/04/11  V1.0     H4     ��       ��
*******************************************************************************/
int strcmpasc(const char *str1,const char *str2,unsigned int length)
{
	int ret;
	char str1tmp[MAX_STRING_LENGTH];
	char str2tmp[MAX_STRING_LENGTH];
	if((length>MAX_STRING_LENGTH)&&(length=0)){
		return 2;
	}
	memset(str1tmp,0,MAX_STRING_LENGTH);
	memset(str2tmp,0,MAX_STRING_LENGTH);
	strdelnull(str1,str1tmp,length);
	strdelnull(str2,str2tmp,length);
	ret=strcmp(str1tmp,str2tmp);
	//printf("str1 = %s\r\nstr2 = %s\r\n",str1tmp,str2tmp);
	//printf("ret = %d\r\n",ret);
	return ret;
}

/*int main(void)
{
	char mstr[40]="dru201301 00.33 ";
	char rstr[40]="dru20 1301 00. 33";
		int i;
		i=strcmpasc(mstr,rstr,40);
		
		printf("ret = %d\r\n",i);
}*/
