/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-11-13
File:			 CSettle.h
Description:	 数据短信结算模块


接口说明  输入命令：jspresettle IIXC 201310 (jspresettle /数据源/ 账期)
**************************************************************************/

#ifndef _CSETTLE_H
#define _CSETTLE_H

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

class CSettle
{
private:
    DBConnection conn;//数据库连接   
    char source_id[8];
    char rate_cycle[8];

public:
	CSettle();
	~CSettle();
	bool checkArg(int argc, char** argv);
	bool DoforIIXC(char* rate_cycle);
	bool DoforIOXC(char* rate_cycle);
	bool DoIIXCOut(char* rate_cycle);
	bool DoIOXCOut(char* rate_cycle);
	//短信
	bool DoforIISM(char* rate_cycle);
	bool DoforIOSM(char* rate_cycle);

};

#endif

