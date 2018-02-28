/*******************************************************************************************
	Copyright 2006 Poson Co.,Ltd. Inc. All Rights Reversed   
********************************************************************************************

**Class			: ProcessRec_Mgr
**Description		: 综合结算--话单分析类
**Author		: Wulf
**StartTime		: 2005/12/02
**Last Change Time 	: 2006/02/08

*******************************************************************************************/

#ifndef _PROCESSREC_MGR_H_
#define _PROCESSREC_MGR_H_

//#include "COracleDB.h"

#include "C_PluginMethod.h"

#include <vector>
#include <map>

const int PATHLENTH = 250;

class Input2Output
{
protected:
	int In_Idx;
	int Out_Idx;
public:
	Input2Output(int in,int out);
	~Input2Output();
	
	//获取In_Idx
	int get_In_Idx() { return In_Idx; }
	//获取Out_Idx
	int get_Out_Idx() { return Out_Idx; }
};
typedef vector<Input2Output*> In2Out;

class ClassifyRule
{
protected:
	/*优先级别*/
	int Priority;
	/*分拣条件*/
	char ExpMethod[METHODLENTH];	
	/*分拣路径*/
	char PickPath[PATHLENTH];
	/*分拣标识*/
	char PickFlag[2];
public:
	ClassifyRule(int priority, char *expmth, char *pickpath, char *pickflag);
	~ClassifyRule();
	
	int get_Priority() { return Priority; }
	char * get_ExpMethod() { return ExpMethod ;}
	char * get_PickPath() { return PickPath; }
	char * get_PickFlag() { return PickFlag; }
	
	int set_ExpMethod(char *expMethod );
};

typedef vector<ClassifyRule*> ClRule;

class ProcessRec_Mgr
{
protected:
	/*工作模板ID*/
	int WorkFlow_ID;
	/*过程标识ID*/
	int Process_ID;
	/*输出话单结构*/
	CFmt_Change OutRec;
	/*处理方法容器*/
	C_PluginMethod *m_Method;
	/*处理方法数量*/
	int  m_Method_cnt;
	/*分拣类*/
	ClRule m_Clrule;
	//2005-06-16分拣类，资料分析前的CLASSIFY_LOCATION=F
	ClRule m_ClFrule;
	/*动态编译解释器*/
	C_Compile m_Compile;
	/*输出文件类型ID*/
	char OutFileTypeID[6];
	/*输入文件类型ID*/
	char InFileTypeID[6];
	/*输入输出格式转换对应关系*/
	In2Out	FileType_Chg;
	/*共享库文件的绝对路径*/
	char SL_fname[500];
	/*PluginProxy*/
	PluginProxy pluginp;
	/*map<string,char *>*/
	VAR_CHAR var_char;

private:
	/*向动态编译器加参数*/
	int AddVariable( int WorkFlowID, int ProcessID );
	/*初始化输入输出格式对应关系*/
	int Init_In2Out( int WorkFlowID, int ProcessID );
	/*初始化分拣类*/
	int InitClRule( int WorkFlowID, int ProcessID );
	/*初始化处理方法容器*/
	int InitPrcM( int WorkFlowID, int ProcessID, char *slfile );
	
public:
	//构造函数
	ProcessRec_Mgr();
	//析构函数
	~ProcessRec_Mgr();
	//初始化函数
	int Init( int WorkFlowID, int ProcessID, char *slfile );
	//同步数据库资料
	//int SycDB();
	//写入输入话单
	int Set_InRec( CFmt_Change &inrec );
	//读取输出话单
	int Get_OutRec( CFmt_Change &outr );
	//读取输出话单
	int Get_OutRec( char *outr, int len );
	//处理分析后的分拣业务
	int Process_Classify( char* retid, int& retcnt );
	//处理分析前的分拣业务
	int Process_FClassify( char* retid, int& retcnt );
	//处理插件方法
	int Process(int&);
	/*向动态编译器加参数*/
	int AddVar( const char *varname, char *varvalue );
	/* 打印插件版本号 */
	int PrintSLNo(char *SLName);
	/*绑定共享内存和检测是否需要更新共享内存*/
	int LoadOrUpdateMem(char *SLName, char* ShmPath);
	friend class C_PluginMethod;
};



#endif
/****************************The End!*********************************/