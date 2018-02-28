
#ifndef __SUM_SHARE_H__
#define __SUM_SHARE_H__

#include<iostream>
#include "Common.h"

#include<sys/types.h>
#include<dirent.h>

#include "CfgParam.h"

using namespace std;

//计算费用
struct SShareFee
{
	char settlesum_col[20];
	char settle_formula[20];
	char settle_formula_param[20];
};

//计算占比
struct SSharePercent
{
	char base_col[20];
	char province_col[20];
	int percent_pos;
	char outtable_name[20];
};


struct SShareList 
{
	char szSourceGrpId[16];   //数据源组ID
	char szSourceId[16];	  //数据源ID
	char szSettleId[6];           //摊分配置ID
	char szFlag[2];			  //是否有效标志  暂时没用
	int szSettleType;           //操作类型，1计算总费用，2计算使用占比

	vector<SItemPair> vItemInfo;
	SCom tableItem;

	SShareFee stShareFee;
	SSharePercent stSharePercent;
};

class CShare
{
	public:	

		DBConnection conn;
		SShareList  *pShareList;

		char szSumMonth[6];//需要做月汇总的月份
		char erro_msg[1024];
		char sql[1024];
		char sqlFile[1024];		//维护态sql写文件
		char currTime[15];
		int iSourceCount ;		//有效的数据源个数

		IBC_ParamCfgMng param_cfg;

	public:

		CShare();
		~CShare();

		bool init(int argc,char** argv);

		bool init(char* month);
		bool init(char* source_id, char* source_group_id,char* month);
		
		int  loadSumConfig(char *source_id, char *source_group_id,int pos);
		int	 loadShareConfig(char*source_id,SShareList &Share);

		void setDate(char* date);

		//判断日汇总条件，解析结构SDayList中所有的文件都达到汇总条件，根据不同的情况，返回不同值，注意正常汇总和重汇总的返回值
		int checkSettCondition(SShareList   &Smonth,char *sumMonth);

		int sum(int deal_type,SShareList  &Smonth,char *sumMonth,bool del);
		int shareCal(SShareList  &Smonth);

		int run(int deal_type,bool del);  //默认删除以前统计的数据

	private:
		//查D_SUMMERY_RESULT表接口
		//查D_SCH_FORMAT接口
		//查D_CHECK_FILE_DETAIL接口
		//查D_OUT_FILE_REG接口
};

#endif





