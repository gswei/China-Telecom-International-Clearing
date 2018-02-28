/****************************************************************
filename: CF_CLogger.cpp
module: for log
created by: Tan Zehua
create date: 2010-05-10
update list: 

version: 1.0.0
description:
    the cpp of the log class
*****************************************************************/

#include "CF_CLogger.h"

//CLog theLog;

 
char* CLog::getVersion()
{
	return("3.0.0");
}

CLog::CLog(bool bOutput)
: streambuf(), ostream((streambuf*) this)
{
	m_bOutputFlag=bOutput;
		
	char pchProcessID[10];
	sprintf(pchProcessID, "[%06d]", getpid());
	m_strProcessID = pchProcessID;

}

CLog::~CLog()
{

}
void CLog::GetTime(string &strTime)
{
	struct tm  *tm;
	time_t	t1;
	time(&t1);
	tm = localtime(&t1);
	char pchTmp[20];
	sprintf(pchTmp, "[%02d:%02d:%02d]",
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	strTime = pchTmp;
};

string CLog::GetCurrentTime()
{
	struct tm  *tm;
	time_t	t1;
	
	time(&t1);
	tm = localtime(&t1);
	
	char pchTmp[20];
	sprintf(pchTmp, "%04d%02d%02d%02d%02d%02d",
	  tm->tm_year+1900,
		tm->tm_mon+1, tm->tm_mday, 
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	
	return pchTmp;
}
	

int CLog::overflow(int nLogLevel)
{
	if (endd==nLogLevel ||endi==nLogLevel ||endw==nLogLevel ||ende==nLogLevel ||endf==nLogLevel)
	{
		string strType;
		switch(nLogLevel)
		{
		case endd:
			strType = "[1-调试]";
			break;
		case endi:
			strType = "[2-信息]";
			break;
		case endw:
			strType = "[3-警告]";
			break;
		case ende:
			strType = "[4-错误]";
			break;
		case endf:
			strType = "[5-严重]";
			break;
		default:
			strType = "[0-未知]";
			break;
		}
		
		string strTime; 
		GetTime(strTime);
		if(nLogLevel>2)
		{
			setErrLog();
		}
		if(nLogLevel>1 || m_bDebugFlag)
		{
			if(nLogLevel>2)
			{
				int pos=m_strBuf.find('\n',0);
				string strErrInRun=m_strBuf.substr(0,pos);	
				run_ofs << strTime  << m_strProcessID
					<< strType<<" " << strErrInRun << endl;
				run_ofs.flush();
		  }
		  else
		  	{
		  		run_ofs << strTime  << m_strProcessID
						<< strType<<" " << m_strBuf << endl;
					run_ofs.flush();
		  		}
		  
		  if(m_bOutputFlag)
		  	{	
					clog << strTime  << m_strProcessID
					<< strType<<" " << m_strBuf << endl;
				}
	
	    err_ofs << strTime  << m_strProcessID
				<< strType <<" "<< m_strBuf << endl;
			err_ofs.flush();
  	}
  	if(nLogLevel>2)
		{
			closeErrLog();
		}
  	
		m_strBuf = "";
	}
		else
	{
		m_strBuf += nLogLevel;
	}
	return nLogLevel;
	
}


int CLog::Open(int ofstype,const string& strFile)
{
	char pchProcessID[10];
	sprintf(pchProcessID, "[%06d]", getpid());
	m_strProcessID = pchProcessID;
	
	string strFullName = strFile;
	switch(ofstype)
	{case 0:
			run_ofs.close();
			run_ofs.clear();
			run_ofs.open(strFullName.c_str(), ios::out|ios::app);
			if (!run_ofs.good()) 
				return -1;
			break;
		case 1:				
			err_ofs.close();
			err_ofs.clear();
			err_ofs.open(strFullName.c_str(), ios::out|ios::app);
			if (!err_ofs.good()) 
				return -1;
			break;
	}
	return 1;
}

int CLog::chkDir(char *fullpath)
{
	if(fullpath == NULL)
		return -1;

	if (fullpath[strlen(fullpath) - 1] != '/')
	{
		char tmppath[255];
		strcpy(tmppath,fullpath);
		strcat(tmppath,"/");
		tmppath[strlen(fullpath)+1]='\0';
		fullpath=tmppath;
	}
	
	char *p = fullpath + 1;
	char temppath[256];
	for (; ;)
	{
		p = strchr(p, '/');
		if (p == NULL)
		{
			break;
		}
		p++;
		memset(temppath, 0, sizeof(temppath));
		strncpy(temppath, fullpath, p - fullpath);
		if ( access( temppath,F_OK ) < 0 )
		 {
         if( mkdir( temppath,S_IRWXU|S_IRGRP|S_IXGRP ) ) 
         {
             return(-1);
         }
     }
	}
	return 0;
}

void CLog::setLog(char * szLogPath, int nLogLevel, char * szFlatName, char * szSourceGroup, int nProcessID)
{
	m_strLogPath=szLogPath;
 	m_nLogLevel=nLogLevel;
 	m_strPlatName=szFlatName;
 	m_strSourceGroup=szSourceGroup;
	m_strProcessNO=nProcessID;
	
	//重置进程ID
	char pchProcessID[10];
	sprintf(pchProcessID, "[%06d]", getpid());
	m_strProcessID = pchProcessID;
	
	if(1==m_nLogLevel)
		m_bDebugFlag=true;
	else
		m_bDebugFlag=false;
	
	//runlog dir and open ofs
	m_strCurrentTime=GetCurrentTime();
	string yearmonthday=m_strCurrentTime.substr(0,8);
	string yearmonth=m_strCurrentTime.substr(0,6);
	
	m_strRunLogPath="";
	m_strRunLogPath.append(m_strLogPath).append("/run_log/").append(yearmonth).append("/").append(m_strSourceGroup).append("/").append(m_strPlatName).append("/");
	chkDir(const_cast<char*>(m_strRunLogPath.c_str()));
	
	char szprocessid[8];
	sprintf(szprocessid,"%04d",m_strProcessNO);
	
	m_strRunLogName="";
	m_strRunLogName.append(yearmonthday).append("_").append(m_strSourceGroup).append("_").append(m_strPlatName).append("_").append(szprocessid).append(".log");
	
	string runfullpath=m_strRunLogPath;
	runfullpath.append(m_strRunLogName);
	Open(0,runfullpath);
	
	//errlog  dir
	m_strErrLogPath="";
	m_strErrLogPath.append(m_strLogPath).append("/error_log/");
	chkDir(const_cast<char*>(m_strErrLogPath.c_str()));

}

void CLog::reSetLog()
{
  //利用第一次初始化时保存的变量
	if(1==m_nLogLevel)
		m_bDebugFlag=true;
	else
		m_bDebugFlag=false;
		
	//重置进程ID
	char pchProcessID[10];
	sprintf(pchProcessID, "[%06d]", getpid());
	m_strProcessID = pchProcessID;
	
	if(1==m_nLogLevel)
		m_bDebugFlag=true;
	else
		m_bDebugFlag=false;
	
	//runlog dir and open ofs
	m_strCurrentTime=GetCurrentTime();
	string yearmonthday=m_strCurrentTime.substr(0,8);
	string yearmonth=m_strCurrentTime.substr(0,6);
	
	m_strRunLogPath="";
	m_strRunLogPath.append(m_strLogPath).append("/run_log/").append(yearmonth).append("/").append(m_strSourceGroup).append("/").append(m_strPlatName).append("/");
	chkDir(const_cast<char*>(m_strRunLogPath.c_str()));
	
	char szprocessid[8];
	sprintf(szprocessid,"%04d",m_strProcessNO);
	
	m_strRunLogName="";
	m_strRunLogName.append(yearmonthday).append("_").append(m_strSourceGroup).append("_").append(m_strPlatName).append("_").append(szprocessid).append(".log");
	
	string runfullpath=m_strRunLogPath;
	runfullpath.append(m_strRunLogName);
	Open(0,runfullpath);
	
	//errlog  dir
	m_strErrLogPath="";
	m_strErrLogPath.append(m_strLogPath).append("/error_log/");
	chkDir(const_cast<char*>(m_strErrLogPath.c_str()));

}

void CLog::setOutputFlag(bool bOutput)
{
	m_bOutputFlag=bOutput;
}

void CLog::setErrLog()
{
	m_strCurrentTime=GetCurrentTime();
	string yearmonthday=m_strCurrentTime.substr(0,8);
	string hour=m_strCurrentTime.substr(8,2);
	
	m_strErrLogPath="";
	m_strErrLogPath.append(m_strLogPath).append("/error_log/").append(yearmonthday).append("/");
	chkDir(const_cast<char*>(m_strErrLogPath.c_str()));
	
	char szprocessid[8];
	sprintf(szprocessid,"%04d",m_strProcessNO);
	
	m_strErrLogName="";
	m_strErrLogName.append("err_").append(m_strPlatName).append("_").append(m_strSourceGroup).append("_").append(szprocessid).append("_").append(hour).append(".log");

	string errFullPath=m_strErrLogPath;
	errFullPath.append(m_strErrLogName);
	Open(1,errFullPath);
	
}
void CLog::closeErrLog()
{
	err_ofs.close();
}

void runLog(int loglevel, char* logstr) 
{
	
	switch(loglevel)
	{
	case 1:
		theLog<<logstr<<endd;
		break;
	case 2:
		theLog<<logstr<<endi;
		break;
	}
		
}


void errLog(int loglevel,const char* erl_filename, int errCode,const char* message,const char* file, int line)
{
	
  theLog.setErrLog();	
  
	//char chString[2000];
	//sprintf(chString, "%d  %d  %s  (%s:%d)  %s", errCode,errno,erl_filename,file,line, message);char s_errCode[20];
	char s_errCode[20];
	sprintf(s_errCode, "%d", errCode);
	char s_errno[20];
	sprintf(s_errno, "%d", errno);
	char s_line[20];
	sprintf(s_line, "%d", line);
	
	string chString;
	chString.append(s_errCode).append("  ").append(s_errno)
	         .append("  ").append(erl_filename).append("  (")
	         .append(file).append(":").append(s_line).append(")  ").append(message);
	
	switch(loglevel)
	{
	case 3:
		theLog<<chString<<endw;
		break;
	case 4:
		theLog<<chString<<ende;
		break;
	case 5:
		theLog<<chString<<endf;
		break;
	}
}

void errLog(int loglevel,const char* erl_filename, int errCode, const char* message, const char* file, int line,CException e) 
{
	theLog.setErrLog();	
  
	//char chString[2000];
	//sprintf(chString, "%d  %d  %s  (%s:%d)  %s%s", errCode,errno,erl_filename,file,line,message,e.GetStack());
	char s_errCode[20];
	sprintf(s_errCode, "%d", errCode);
	char s_errno[20];
	sprintf(s_errno, "%d", errno);
	char s_line[20];
	sprintf(s_line, "%d", line);
	
	string chString;
	chString.append(s_errCode).append("  ").append(s_errno)
	         .append("  ").append(erl_filename).append("  (")
	         .append(file).append(":").append(s_line).append(")  ").append(message).append(e.GetStack());
	switch(loglevel)
	{
	case 3:
		theLog<<chString<<endw;
		break;
	case 4:
		theLog<<chString<<ende;
		break;
	case 5:
		theLog<<chString<<endf;
		break;
	}

}

