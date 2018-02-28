#include<iostream>
#include<vector>
//#include<stdio.h>
#include "CF_Common.h"
#include "CF_CLogger.h"
//#include "CF_COracleDB.h"
#include "CfgParam.h"
#include<dirent.h>
#include "CF_CFscan.h"
#include "CF_Common.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"

struct GWFileDefine
{
	char report_id[30];
	char filename[256];
	char outpath[256];
	int  sheet_no;
	int  col_num;
	char seperator[10];
	char endSep[10];
	char sqltxt[512];

	clear()
	{
		sheet_no = 0;
		col_num = 0;
		memset(report_id,0,sizeof(report_id));
		memset(filename,0,sizeof(filename));
		memset(outpath,0,sizeof(outpath));
		memset(seperator,0,sizeof(seperator));
		memset(endSep,0,sizeof(endSep));
		memset(sqltxt,0,sizeof(sqltxt));
	}
};

struct IACC_GJYY
{
	int DIRECTION;
	int BATCH_TYPE;
	int BATCH_CREATION_DATE;
	char HOME_BID[12];
	char SERVING_BID[12];
	long BATCH_NUMBER;
	long RECORD_NUMBER;
	long BATCH_AMOUNT;
	char BATCH_AMOUNT_CURRENC[4];

	clear()
	{
		DIRECTION=0;
		BATCH_TYPE=0;
		BATCH_CREATION_DATE=0;
		memset(HOME_BID,0,sizeof(HOME_BID));
		memset(SERVING_BID,0,sizeof(SERVING_BID));
		BATCH_NUMBER=0;
		RECORD_NUMBER=0;
		BATCH_AMOUNT=0;
		memset(BATCH_AMOUNT_CURRENC,0,sizeof(BATCH_AMOUNT_CURRENC));
	}
	
};

class CarrierAct
{
	private:
		char ratecycle[6+1];
		char sql[4096];
		char erro_msg[4096];
		vector<GWFileDefine> vFile;
		DBConnection conn;
		//CDatabase *m_DBConn;
		int type;
		char currTime[14+1];

	public:
		CarrierAct();
		~CarrierAct();

		bool init(int argc,char** argv);
		void dealType();

		void run();
		void run2();
		void run3();
		void run4();
		void run5();
		void run6();
};


