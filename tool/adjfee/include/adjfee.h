/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2014-02-07
File:			 adjfee.h
Description:	 调整费用


接口说明  输入命令：jsadjfee 
**************************************************************************/

#ifndef _ADJFEE_H
#define _ADJFEE_H

#include "psutil.h"
#include "tp_log_code.h"
#include <string>
#include <vector>

#include <iostream>
#include <fstream>
#include "RTInfo.h"   //petri网状态

using namespace tpss;  //和psutil.h对应
#include "CF_CLogger.h"
#include "bill_process.h"
//#include <occi.h>
//#include <occiControl.h>
//using namespace oracle::occi;
using namespace std;

class Cadjfee
{
private:
    DBConnection conn;//数据库连接   
 public:
    char source_id[8];
    char adj_time[9];
    double adj_fee;

public:
	Cadjfee();
	~Cadjfee();
	
	bool testSQL(char *sourceid,char *adjdate);	
	bool testUpdate(char *sourceid,int adj_no,char *adjdate);
	bool dealAdj(char *sourceid,int adjno,char *adj_date);
	bool addLog(char *sourceid,char *adjdate);
};

#endif

