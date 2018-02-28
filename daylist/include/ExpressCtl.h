/***************************************************************
filename: ExpressCtl.h
module:
created by: xiehp
create date: 2006-08-28
version: 1.0.0
description: 封装表达式计算类
*****************************************************************/
#ifndef _EXPRESSCTL_
#define _EXPRESSCTL_


#include "CF_Common.h"
#include "CF_Cerrcode.h"
#include "CF_CFmtChange.h"
#include "CF_CInterpreter.h"
#include "psutil.h"

#include <map>
#include <vector>
#include <iostream.h>

#define FIELD_VALUE_LENGTH  50

#define  ERROR_EXPRESS               10009    //表达式错误

class CExpress_CTL
{
	public:		
		CExpress_CTL();
		~CExpress_CTL();		
		void Init(char *express,char* szInputFiletypeId,CFmt_Change &inrcd);		
		char *Operation(char *ResultOut);
		char contex[1024];
	private:
						
		Interpreter theCompiler;//表达式分析器
};

class CSource_Merge_Condiction
{
	private:
		map<string,CExpress_CTL*> map_source_mergeCondicion;
	public:
		CSource_Merge_Condiction();
		~CSource_Merge_Condiction();
		void init(char* szInputFiletypeId,CFmt_Change &inrcd, char *pipe);
		char *Operation(char *sourceId,char *result);		
		void printMe();
};

#endif

