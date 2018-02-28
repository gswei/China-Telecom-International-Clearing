/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-11-13
File:			 CSettle.h
Description:	 ���ݶ��Ž���ģ��


�ӿ�˵��  �������jspresettle IIXC 201310 (jspresettle /����Դ/ ����)
**************************************************************************/

#ifndef _CSETTLE_H
#define _CSETTLE_H

#include "psutil.h"
#include "tp_log_code.h"
#include <string>
#include <vector>

#include <iostream>
#include <fstream>
#include "RTInfo.h"   //petri��״̬

using namespace tpss;  //��psutil.h��Ӧ
//#include "CF_Common.h"
#include "CF_CLogger.h"
 #include "bill_process.h"
 
using namespace std;

class CSettle
{
private:
    DBConnection conn;//���ݿ�����   
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
	//����
	bool DoforIISM(char* rate_cycle);
	bool DoforIOSM(char* rate_cycle);

};

#endif

