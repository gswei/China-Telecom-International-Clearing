
#include<iostream>
#include <vector>
#include<dirent.h>

#include "psutil.h"
#include "tp_log_code.h"
#include "CfgParam.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFmtChange.h"

#define  STAT_INSERT_CONNT             500	//更新库表时一次插入的记录书
#define	STAT_ITEM_COUNT                100    //表项个数
#define	STAT_ITEM_NAME_LEN             50    //表项名长度
#define	STAT_ITEM_CONTAINER_LEN        251    //表项内容长度
#define  STAT_GROUP_COUNT              10    //统计标识组个数
#define  MAX_STATID_NUM                50    //一条话单允许的最大统标识个数
#define  SQL_LENGTH					   2000

using namespace tpss;						//和psutil.h对应
using namespace std;

#define COMMIT_COUNT	500					//每次提交500条,若失败,通过file_id删除以前提交的数据

struct STAT_TABLE
{	
	char SzTable_Name[50];//表名
	int  NStat_Item_Count;//表字段个数
	char SzStat_Item[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];		//入库字段名
	char SzStat_FieldName[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];	//文件字段名
	int  NItem_Type[STAT_ITEM_COUNT];
	int  NStat_Item_Begin[STAT_ITEM_COUNT];
	int  NStat_Item_End[STAT_ITEM_COUNT];
	char NSql[1024];	
};


class CF_CNewError_Table
{
	public:

		CF_CNewError_Table();
		~CF_CNewError_Table();

		bool init(int config_id,char* OutTypeId);
		
		int setConfig(long file_id,char* rate_cycle);
		int	setBegin(DBConnection& conn);
		int	setRWFlag(bool flag);
		
		int setSQLconf(char* name,int count);

		bool insertData(char* data);	
		int Commit();
		int RollBack();

		void writeSQL(char* data);
		//vector<string> getvSQL();

	private:
		bool mRWFlag;
		int  m_count ;
		long mFileid;
		char mRateCycle[6+1];
		char m_szOutTypeId[10];
		
		char sql_path[256];
		char mFullSqlName[512];

		char erro_msg[2048];
		char sql[1024];
		STAT_TABLE m_SStat_Table;
		CFmt_Change outrcd;
		Statement stmt;

		//vector<string>  vsql;
		string* vsql;
		
};