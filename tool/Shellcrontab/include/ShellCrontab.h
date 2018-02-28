


#ifndef SHELLCRONTAB_H
#define SHELLCRONTAB_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include  <signal.h>
#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <map>
#include "time.h"
#include <sys/types.h>
#include <sys/wait.h>

#include "util/cipher.h"
#include "CfgParam.h"
#include "CF_Common.h"
#include<dirent.h>

//#include "bill_process.h"
//#include "psutil.h"
//#include "tp_log_code.h"

#include "CF_CLogger.h"
#include "CF_CEncrypt.h"
#include "db/SQLException.h"
#include "db/Statement.h"
#include "db/DBConnection.h"
#include "db/DBConnectionFactory.h"
#include "Poco/String.h"

using namespace tpss;

USING_UTIL_NS
USING_UTIL_DB_NS

CLog theJSLog;	//2013-12-31

#ifndef WRITE_LOG
#define WRITE_LOG(msg)  { \
	theJSLog<<msg<<endi; \
}
#endif

//CLog theLog;//string szLogPath="/mboss/home/zhjs/log/run_log/201312/ShellCrontab.log";//string szLogLevel="1";


std::string szEnvFile="${SETTLEDIR}/etc/zhjs/zhjs.env";
struct ContrabID
{
	int group_id    ;
	std::string	script_type ;
	std::string	run_script;
	int	spec_order ;
	std::string	remark;
	std::string connectstring;
	int isRelation ;
	int item_id;
};


struct FileContrab
{
	int group_id  ;
	std::string	runtime ;
	int cycle ;
	int cycle_type;
	std::string	next_process_time;//
	int latn_id;
	int i_Fork ;
	int is_immediate_run;
	std::vector<ContrabID> vecContrab;

};

class FileRunContrab
{
public:
	

	bool operator!=( FileRunContrab &runcontrab ) const;

	FileRunContrab();
	~FileRunContrab();

	void output();
    bool Init();
	bool ReadConfig();
	bool AllocationForkNum();
	//bool Process();
	bool SyncSingleTable(FileContrab &fileContrab);
	std::string stringDecryption( std::string str );
   int getDbLoginInfo( std::string str, std::string& user, std::string& pwd, std::string& serv );   std::string CalTime(std::string strLastRunTime, int cycle, int cycle_type);   std::string CalFistTime(std::string curtime,std::string runtime, int cycle, int cycle_type);
	int RunSQL(ContrabID & contrabID,std::string &remark);
	int RunShell(ContrabID & contrabID,std::string &remark);
	int ChooseRun(ContrabID & contrabID,std::string &remark);
	 bool LoopProcess(int);
public:
	std::vector<FileContrab> vecFileContrab;
	DBConnection 	_conn;  
public:
	std::string user;
	std::string pwd;
	std::string serv;

};

/* 2013-12-31
class CReadIni
{   
public:
	CReadIni()
	{

	}
	~CReadIni()
	{

	}

	bool init(const char * pszConfigPath);
	bool GetValue(std::string &strValue, std::string strKey);
private:
	std::map<std::string,std::string> IniInfo;
};
*/

#endif
