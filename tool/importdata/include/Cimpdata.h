/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-11-13
File:			 Cimpdata.h
Description:	 数据短信结算模块


接口说明  输入命令：jsimpdata 
**************************************************************************/

#ifndef _CIMPDATA_H
#define _CIMPDATA_H

#include "psutil.h"
#include "tp_log_code.h"
#include <string>
#include <vector>

#include <iostream>
#include <fstream>
#include "RTInfo.h"   //petri网状态

using namespace tpss;  //和psutil.h对应
//#include "CF_Common.h"
#include "CF_CLogger.h"
 #include "bill_process.h"
 
using namespace std;

class Cimpdata
{
private:
    DBConnection conn;//数据库连接   
 public:
    char source_id[8];

public:
	Cimpdata();
	~Cimpdata();
	//固网
	bool setData();
	bool dealGWYY();
	bool getYYSql(char *rategroup_id,char *formula_id,char *rule_id,char *start_time,char *end_time,char* fee,char *column,char* value);
	bool dealGWDX();
	bool getDXSql(char *rategroup_id,char *formula_id,char *rule_id,char *start_time,char *end_time,char *fee,char *column,char* value);
	

};

#endif

