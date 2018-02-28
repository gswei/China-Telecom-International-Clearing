/***************************************************************
filename: ListSpecialCtl.h
module:
created by: xiehp
create date: 2006-08-28
version: 1.0.0
description: ��ӦLIST_SPECIAL_DEFINE�����Ϣ,�ṩ��ȡĿ���������ķ���
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
#define  ERROR_EXPRESS               6009    //���ʽ����
#endif

typedef struct _Express_Ctl
{
	string list_id;
	char contex[1024];
	vector <string> v_varName;//��������ʵ����Ϊ�ֶ�����
	Interpreter theCompiler;//���ʽ������
	
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
		map<string, strct_VarValue> m_varname_value;//<������������ֵ>
		
	private:
		map<int,Express_Ctl> m_list_special;//firstΪLIST_SPECIAL_DEFINE���list_id
		int DefineVariable();
		void GetVariable(char *contex, vector<string> &v_var);
};




#endif

