#include<iostream>
#include<vector>
//#include<stdio.h>
#include "CF_Common.h"
#include "CF_CLogger.h"
//#include "CF_COracleDB.h"
#include "CfgParam.h"
#include<dirent.h>

//产品资料表
struct SP_PRODUCT
{
	int  cust_id ;
	long CATALOG_ID;
	char eff_date[14];
	char exp_date[14];
	long cp_code; 
	long child_cp_code;
	char product_id[21];
	int  product_type;
	int  rule_id;
	int  settle_rule_type;
	int  settle_base;
};

struct CFG_SMS_CP_CODE
{
	long  cp_code;
	int   prov_id;
	char  eff_date[14];
	char  exp_date[14];
	int   flag;
};

struct SP_CDR_INFO
{
	long CATALOG_ID;
	long cp_code; 
	char product_id[21+1];
	char cdr_time[14+1];
	//int  billing_type;
	//int  settleprov;
	//int  billing_area_code;
};

class Ismp
{
	private:
		char ratecycle[6+1];
	    char filetime[8+1];

		char sql[4096];
		char erro_msg[4096];
	
		DBConnection conn;
		int type;
		char currTime[14+1];

	public:
		Ismp();
		~Ismp();

		bool init(int argc,char** argv);
		void dealType();

		void run();
		//void run2();

};


