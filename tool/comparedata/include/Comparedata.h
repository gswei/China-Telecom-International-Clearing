/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-12-10
File:			 Comparedata.h
Description:	 ���ʹ����Ƚ�����ģ��


�ӿ�˵��  �������jscompare 
**************************************************************************/

#ifndef _CIMPDATA_H
#define _CIMPDATA_H

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
 #include "CF_Common.h"
 
using namespace std;
 
struct GWYY
{
	char destination_id[9];
    char country_id[9];
    char country_name[9];
    char destination[400];
    char country_code[9];
    char area_codes[4000];
    char rate[20];
    char change[20];
    char effect_date[9];
    char expire_date[9];
    char dir[2];
    char comments[40];
    char callee_type[2];
};

struct GWDX
{
    char index_id[9];
    char country_id[9];
    char country_name[100];
    char chinese_name[100];
    char rate[20];
    char effect_date[9];
    char expire_date[9];
    char dir[2];
    char country_code[20];
};

class Comparedata
{
private:
    DBConnection conn;//���ݿ�����   

public:
	Comparedata();
	~Comparedata();
	
	char source[8];  //�����������߹�������
	//��������
	bool DoforGWYY();
	bool insertDataYY(char *destination_id);
	bool BakGWYY();
	//��������
	bool DoforGWDX();
	bool insertDataDX(char *index_id);
	bool BakGWDX();
	
	bool insertLog(char *flag,int num,char *destination_id);
};

#endif

