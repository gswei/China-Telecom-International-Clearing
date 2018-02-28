/****************************************************************
filename: CF_CLogger.h
module: for log
created by: Tan Zehua
create date: 2010-05-10
update list: 

version: 1.0.0
description:
    the header file of the log class
****************************************************************/
#ifndef CF_CLOGGER_H_
#define CF_CLOGGER_H_


#include <string>
#include <stdio.h>
#include <sys/stat.h>

#include "CF_CException.h"
#include "CF_Config.h"
using namespace std;
using std::string;
using std::vector;

class CLog;
extern CLog theLog;

#define endd ('\001') 
#define endi ('\002') 
#define endw ('\003') 
#define ende ('\004') 
#define endf ('\005')

#define LEVEL_DEBUG  1  
#define LEVEL_INFO   2
#define LEVEL_WARN   3 
#define LEVEL_ERROR  4 
#define LEVEL_FATAL  5

#define DEBUG_LOG  if(theLog.m_bDebugFlag) theLog

class CLog : public streambuf, public ostream  
{
	private:
		ofstream run_ofs;
		ofstream err_ofs;
		
		string m_strBuf;
		string m_strProcessID;
		string m_strLogPath;
	 	int    m_nLogLevel;
	 	string m_strPlatName;
	 	string m_strSourceGroup;	
	 	int    m_strProcessNO;
	 	string m_strRunLogPath;
	 	string m_strErrLogPath;
	 	string m_strRunLogName;
	 	string m_strErrLogName;
	 	string m_strCurrentTime;
	 	bool   m_bOutputFlag;

		int chkDir(char *fullpath);
		static void GetTime(string &strTime);
		string GetCurrentTime();
		void SetDebugFlag(bool bOutput = true);
	
	public:
	
		CLog(bool bOutput = true);
		virtual ~CLog();
		
		bool   m_bDebugFlag;
		
		static char* getVersion();
		
		void setOutputFlag(bool bOutput = true);
			
		int overflow(int nLogLevel = EOF);
		int underflow() { return 0;}
		
		int Open(int ofstype,const string& strFile);
		void setLog(char * szLogPath, int nLogLevel, char * szFlatName, char * szSourceGroup, int nProcessID);
		void reSetLog();
		void setErrLog();
		void closeErrLog();
		
};

void runLog(int loglevel, char* logstr);

void errLog(int loglevel, const char* erl_filename, int errCode, const	char* message, const  char* file, int line);

void errLog(int loglevel, const char* erl_filename, int errCode,  const char* message, const  char* file, int line, CException e);


#endif /* CF_CLOGGER_H_ */
