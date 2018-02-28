/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-12-13
File:			 balancecheck.h
Description:	 前台获取平衡数据


接口说明  输入命令：jsbalance 
**************************************************************************/

#ifndef _BALANCECHECK_H
#define _BALANCECHECK_H

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

class Cbalance
{
private:
    DBConnection conn;//数据库连接   
 public:
    char source_id[8];
    char check_time[9];

public:
	Cbalance();
	~Cbalance();
	
	bool checkData(char *checkdate);		
	bool checkDataforsource(char *sourceid,char *checkdate);
};

#endif

