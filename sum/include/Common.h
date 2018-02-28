#ifndef __SUM_COMMON_H__
#define __SUM_COMMON_H__


#include<iostream>
#include<vector>

//#include "psutil.h"
//#include "tp_log_code.h"

#include "CF_Common.h"
#include "CF_CLogger.h"

//using namespace tpss;  //和psutil.h对应

using namespace std;
//CLog theJSLog;

struct SettCon
{
	int  conSumt;				//结果表表名
	char iDestTableName[32];	//目标表表名
	char condition[100];		//统计条件
};

struct SCom
{
	int iOrgSumt;            //原始表的configid
	char szOrgSumtCol[30];   //指定原始表时间字段名
	int iDestSumt;           //汇总结果表的configid
	char szDestSumtCol[30];  //汇总结果表账务月字段
	char iOrgTableName[30];  //原始表表名
	char iDestTableName[30]; //汇总结果表表名

	int count;
	SettCon* mscontion;
	SCom()
	{
		count = 1;
		mscontion = NULL;
	}
};

struct SItem
{
   char szItemName[32];//统计表的字段名
   int iItemType;//字段类型（统计维度/统计字段）
   int iSpecType;//特殊标识（如取系统时间等，同统计手册）
   int iBeginPos;
   int iEndPos;

   char idefault[50];	//2014-01-09 增加默认值
};

struct SItemPair
{
   SItem fromItem;  //源头表结构
   SItem toItem;   //目标表结构
};

//根据SCom，初始化vItemInfo
int getItemInfo(SCom szSCom,vector<SItemPair> &vItemInfo);

int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue, char *sql,int type = 0,char* rate_cycle = NULL,int postion = 0);

int insertSql(char *sql);


int getDays(char* time);
#endif
