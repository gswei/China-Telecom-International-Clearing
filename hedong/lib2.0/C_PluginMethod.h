/*******************************************************************************************
	Copyright 2004 Poson Co.,Ltd. Inc. All Rights Reversed   
********************************************************************************************

**Class			: C_PluginMethod
**Description		: �ۺϽ���--��������
**Author		: sunhua
**StartTime		: 2004/09/02
**Last Change Time 	: 2004/09/10
											2004/12/03	�޸ķ�����Чʱ��ʱ���ñ������ж�

*******************************************************************************************/

#ifndef _C_PLUGINMETHOD_H_
#define _C_PLUGINMETHOD_H_

#include "config.h"
#include "COracleDB.h"
#include "interpreter.h"
#include "PluginLoader.h"
#include "CFmt_Change.h"
#include "CF_CError.h"
#include "Packet.h"
#include <map>

const int METHODLENTH = 500;

typedef map<string,string> VAR_MAP;
typedef map<string,char *> VAR_CHAR;

int DeleteSpace( char *ss );
int SplitBuf( char *ss,char flag,int *buf );

class C_Method
{
public:
	/*���ִ������*/
	int  Seq;
	/*������ʽ*/
	char Plugin[METHODLENTH];
	/*����������У����Զ���*/
	int Output_Col[50];
	//�������Ŀ
	int  Output_Cnt;
	//�������
	char PluginName[20];
	//�����������
	PacketParser pparser;
	//�������ָ��
	BasePlugin* m_pPluginObj;
public:
	//���캯��
	C_Method();
	//��������
	~C_Method();	
	//��ʼ������
	//int Init(int seq, char *plugin,int out_col);
	int Init(int seq, char *plugin,char *out_col, VAR_CHAR &var, PluginProxy &pluginp);
	//��ȡ����������е�ֵ
	int get_Output_Col(int i) ;
	//��ȡ����������е�����
	int get_Output_Cnt();
	//��ȡ������ʽ
	char* get_Plugin() ;

};


class C_PluginMethod
{
protected:
	/*����ID*/
	int  MethodID;
	/*ִ�з�����ǰ������*/
	char m_szCondition[METHODLENTH];
	/*��������Ч�ڣ���ʼʱ��*/
	char StartTime[15];
	/*��������Ч�ڣ�����ʱ��*/
	char EndTime[15];
	/*�����ָ��*/
	C_Method *Method;
	/*���������*/
	int MethodCnt;	
public:
	//���캯��
	C_PluginMethod();
	//��������
	~C_PluginMethod();
	//��ʼ������
	int Init( int methodid,char *condition,char *starttime,char *endtime ,VAR_CHAR &var,PluginProxy &pluginp );
	//ִ�д���
	int Excute(C_Compile &p_Compile,CFmt_Change &p_OutRec, PluginProxy& pluginp );
	//�������������
	int set_Condition( char *cond );
	
};

#endif
