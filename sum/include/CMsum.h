
#ifndef __SUM_MON_H__
#define __SUM_MON_H__

#include<iostream>
#include "Common.h"

#include<sys/types.h>
#include<dirent.h>

#include "CfgParam.h"

#include "RTInfo.h"
//#include "dr_api.h"

using namespace std;

struct SMonthList
{
	char szSourceGrpId[16];   //数据源组ID
	char szSourceId[16];	  //数据源ID
	//char szFlag[2];			  //是否有效标志  暂时没用
	vector<SItemPair> vItemInfo;
	SCom tableItem;
};

class CMsum
{
	private:	

		DBConnection conn;
		SMonthList *pMonthList;
	
		int iSourceCount ;		//有效的数据源个数
		char szSumMonth[6+1];//需要做月汇总的月份
		char currTime[15];
		char erro_msg[2048];
		char sql[2048];
		
		short petri_status;
		//bool m_enable ;		//容灾状态
		//int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统	
		//char m_SerialString[4096];
		//char m_AuditMsg[4096];
		
		SGW_RTInfo	rtinfo;
		char module_id[32];
		long module_process_id;
		string m_triggerFile;

		//map<string,string> fileTypeSourceMap;			//文件名对应数据源
		IBC_ParamCfgMng param_cfg;

	public:

		CMsum();
		~CMsum();
		
		bool init(int argc,char** argv);
		bool init(char* month);
		bool init(char* source_id, char* source_group_id,char* month);
		
		int  loadSumConfig(char *source_id, char *source_group_id,int pos);
		
		void setDate(char* date);
		
		//bool drInit();
		//bool CheckTriggerFile();
		//int  drVarGetSet(char* m_SerialString);
		//int IsAuditSuccess(const char* dealresult);

		//判断日汇总条件，解析结构SDayList中所有的文件都达到汇总条件，根据不同的情况，返回不同值，注意正常汇总和重汇总的返回值
		int checkMonthCondition(SMonthList  &Smonth,char *sumMonth);

		int sum(int deal_type,SMonthList &Smonth,char *sumMonth,bool del);
		//int redosum(SMonthList &Smonth,char *sumMonth,bool del);

		int run(int deal_type,bool del);  //默认删除以前统计的数据

	private:
		//查D_SUMMERY_RESULT表接口
		//查D_SCH_FORMAT接口
		//查D_CHECK_FILE_DETAIL接口
		//查D_OUT_FILE_REG接口
};

#endif



