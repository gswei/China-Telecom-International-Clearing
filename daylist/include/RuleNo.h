/***************************************************************
filename: RuleNo.h
module:
created by: xiehp
create date: 2006-06-22
version: 1.0.0
description: 生成合帐模块所需要的规则号定义信息，
		   对应list_ruleno_define 、list_ruleno_condiction表的信息
*****************************************************************/
#ifndef _RULENO_
#define _RULENO_


#include "CF_Common.h"
#include "CF_Cerrcode.h"
#include "CF_CFmtChange.h"

#include <string>
#include <map>
#include <vector>
#include <iostream.h>
#include "psutil.h"

#ifndef ERR_SELECT
#define          ERR_SELECT                   10003  /*查询数据出错*/
#endif

#define FIELD_VALUE_LENGTH  50

typedef vector <int> v_Refer_Field;

struct Field_Value
{
	char value[FIELD_VALUE_LENGTH];
	Field_Value()
		{
		memset( value, 0, FIELD_VALUE_LENGTH );
		}
};

struct Refer_Field_Value
{
	int refer_field_index;
	vector <Field_Value> v_refer_field_value;
};

typedef vector<Refer_Field_Value> v_Refer_Field_Value;
typedef map<int,Field_Value> map_Record_Refer_Value;
typedef map<int, v_Refer_Field_Value> map_Ruleno_Condiction;

class CRuleno_Condiction
{
	public:
		map<string, map_Ruleno_Condiction> map_listID_rulenoCondiction;
		map<int,int> m_refered_field;
		CRuleno_Condiction();
		~CRuleno_Condiction();		
		void Init(char* listConfigID,char *szInputFiletypeId);
		int GetRuleNo(CFmt_Change &inrcd, string list_id);
		void PrintMe();
};




#endif

