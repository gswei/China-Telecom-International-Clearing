/***************************************************************
filename: ListSpecialCtl.h
module:
created by: xiehp
create date: 2006-08-28
version: 1.0.0
description: 对应LIST_SPECIAL_DEFINE表的信息,提供获取目标生成树的方法
*****************************************************************/
#ifndef _LISTSPCLCTL_
#define _LISTSPCLCTL_


#include "CF_CInterpreter.h"
#include "CF_Common.h"
#include "CF_Cerrcode.h"
#include "CF_CFmtChange.h"
#include "psutil.h"

#include <string>
#include <map>
#include <vector>
#include <iostream.h>

#define FIELD_VALUE_LENGTH  50

#ifndef ERROR_EXPRESS
#define  ERROR_EXPRESS               6009    //表达式错误
#endif

typedef struct _Express_Ctl
{
	string list_id;
	char contex[1024];
	vector <string> v_varName;//变量名，实际上为字段名称
	Interpreter theCompiler;//表达式分析器
	
}Express_Ctl;

typedef struct _VariableValue
{
	char c_value[1024];
}strct_VarValue;



class CList_Special_CTL
{
	public:		
		CList_Special_CTL();
		~CList_Special_CTL();		
		void Init(char *ListConfigID,char *szInputFiletypeId);			
		char * Operation(CFmt_Change &inrcd,char* ret);
		void PrintMe();
		map<string, strct_VarValue> m_varname_value;//<变量名，变量值>
		
	private:
		map<int,Express_Ctl> m_list_special;//first为LIST_SPECIAL_DEFINE表的list_id
		int DefineVariable();
		void GetVariable(char *contex, vector<string> &v_var);
};




#endif

