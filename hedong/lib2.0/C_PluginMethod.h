/*******************************************************************************************
	Copyright 2004 Poson Co.,Ltd. Inc. All Rights Reversed   
********************************************************************************************

**Class			: C_PluginMethod
**Description		: 综合结算--处理方法类
**Author		: sunhua
**StartTime		: 2004/09/02
**Last Change Time 	: 2004/09/10
											2004/12/03	修改方法生效时间时采用闭区间判断

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
	/*插件执行序列*/
	int  Seq;
	/*插件表达式*/
	char Plugin[METHODLENTH];
	/*插件结果输出列，可以多列*/
	int Output_Col[50];
	//输出列数目
	int  Output_Cnt;
	//插件名称
	char PluginName[20];
	//插件输入数组
	PacketParser pparser;
	//插件对象指针
	BasePlugin* m_pPluginObj;
public:
	//构造函数
	C_Method();
	//析构函数
	~C_Method();	
	//初始化函数
	//int Init(int seq, char *plugin,int out_col);
	int Init(int seq, char *plugin,char *out_col, VAR_CHAR &var, PluginProxy &pluginp);
	//获取插件结果输出列的值
	int get_Output_Col(int i) ;
	//获取插件结果输出列的总数
	int get_Output_Cnt();
	//获取插件表达式
	char* get_Plugin() ;

};


class C_PluginMethod
{
protected:
	/*方法ID*/
	int  MethodID;
	/*执行方法的前提条件*/
	char m_szCondition[METHODLENTH];
	/*方法的有效期－起始时间*/
	char StartTime[15];
	/*方法的有效期－结束时间*/
	char EndTime[15];
	/*插件类指针*/
	C_Method *Method;
	/*插件的数量*/
	int MethodCnt;	
public:
	//构造函数
	C_PluginMethod();
	//析构函数
	~C_PluginMethod();
	//初始化函数
	int Init( int methodid,char *condition,char *starttime,char *endtime ,VAR_CHAR &var,PluginProxy &pluginp );
	//执行处理
	int Excute(C_Compile &p_Compile,CFmt_Change &p_OutRec, PluginProxy& pluginp );
	//保存编译后的条件
	int set_Condition( char *cond );
	
};

#endif
