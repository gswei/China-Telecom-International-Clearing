
#ifndef __SUM_DAY_H__
#define __SUM_DAY_H__

#include<iostream>
#include "Common.h"

#include "process.h"
#include "RTInfo.h"

//#include "dr_api.h"
#include "dr_deal.h"

using namespace std;

struct SDayList
{
	char szSourceGrpId[16];   //数据源组ID
	char szSourceId[16];	  //数据源ID
	//char szFlag[2];			  //是否有效标志  暂时没用
	vector<SItemPair> vItemInfo;
	SCom tableItem;
	//char org_source[16];	//2013-08-19 指定原始表的数据源ID 同一个清单表里面有多个数据源ID时进行区分
	//char rate_cycle[20];
};

struct  Check_Sum_Conf
{
	string check_type;
	string source_id;
	string sum_table;
	string cdr_count ;
	string cdr_duration;
	string cdr_dataflow;
	vector<string> cdr_fee;
	string rate_cycle;
	int busi_type;

	void clear()
	{
		check_type = "";
		source_id = "";
		sum_table = "";
		cdr_count = "";
		cdr_duration = "";
		cdr_dataflow = "";
		rate_cycle = "";
		cdr_fee.clear();
		busi_type = 0;
	}

};


class CDsum:public PS_Process
{
	public:	

		DBConnection conn;
		SDayList *pDayList;

		char szSumDate[8+1];//需要做日汇总的日期
		char erro_msg[2048];
		char sql[2048];

		char currTime[15];
		int iSourceCount ;		//有效的数据源个数
		bool flag1;				//是否常驻
		//bool flag2;				//是否删除以前的记录
		
		SGW_RTInfo	rtinfo;
		char module_id[32];
		long module_process_id;

		short petri_status;
		
		DR_Deal	 mdrDeal;
		//bool m_enable ;		//容灾状态
		//int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统	
		//char m_SerialString[4096];
		//char m_AuditMsg[4096];
		map< string,Check_Sum_Conf > monthSumMap ;

	public:

		CDsum();
		~CDsum();

		bool init(int argc,char** argv);
		bool init();
		bool init(char *source_id, char *source_group_id);
	
		int  loadSumConfig(char *source_id, char *source_group_id,int pos);
		
		void setDate(char* date);
		void setDaemon(bool flag = true);
		
		//bool drInit();
		//bool CheckTriggerFile();
		//int  drVarGetSet(char* m_SerialString);
		//int IsAuditSuccess(const char* dealresult);

		//int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char* org_source,char* source_id,char *sql);	//特殊情况

		//判断日汇总条件，解析结构SDayList中所有的文件都达到汇总条件，根据不同的情况，返回不同值，注意正常汇总和重汇总的返回值
		int checkDayCondition(SDayList  &Sday,char *sumday);

		int sum(SDayList &Sday,char *sumday);
		int redosum(SDayList &Sday,char *sumday,bool del);

		void run();
		int redorun(char* date,bool del = true);  //默认删除以前统计的数据
		
		void prcExit();

	private:
		//查D_SUMMERY_RESULT表接口
		//查D_SCH_FORMAT接口
		//查D_CHECK_FILE_DETAIL接口
		//查D_OUT_FILE_REG接口
};

#endif
