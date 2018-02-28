/***********************************************************************
*CF_CDealLog.cpp
*通用处理日志接口
*20050921 by tanj
**********************************************************************/
#include "CF_CDealLog.h"


//处理日志文件命名规则：$PIPEID_$PROCESSID_$FILENAME_$DEALTIME.$SUFFIX
CF_CDealLog::CF_CDealLog()
{
	memset(m_szPipeId, 0, sizeof(m_szPipeId));
	memset(m_szProcessId, 0, sizeof(m_szProcessId));
  memset(m_szLogPath, 0, sizeof(m_szLogPath));
  m_bLogFlag = false;
}
bool CF_CDealLog::DealLog_Collect(const char *szFileName, const char *szOrgFileName,
                       const char *szStartTime, const char *szEndTime,
                       const char *szSwitchTime, int iSN)
{
	sprintf(m_szProcessId, "%s",DEALLOG_COLLECT_CODE);
	
	char szLogFileName[256];
	sprintf(szLogFileName, "%d_%s_%s_%s_%s_%s",
	                                     DEALLOG_COLLECT_CODE,
	                                     m_szPipeId, 
	                                     m_szProcessId,
	                                     szFileName,
	                                     szEndTime,
	                                     DEALLOG_SUFFIX);
	char szLog[1024];
	sprintf(szLog, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%d", m_szPipeId,
	                                                 DEALLOG_SEPERATOR,   
	                                                 m_szProcessId,
	                                                 DEALLOG_SEPERATOR,
	                                                 szFileName,
	                                                 DEALLOG_SEPERATOR,
	                                                 szEndTime,
	                                                 DEALLOG_SEPERATOR,
	                                                 szOrgFileName,
	                                                 DEALLOG_SEPERATOR,
	                                                 szStartTime,
	                                                 DEALLOG_SEPERATOR,
	                                                 szSwitchTime,
	                                                 DEALLOG_SEPERATOR,
	                                                 iSN);
	return WriteLog(szLogFileName, szLog, szEndTime);
}
bool CF_CDealLog::DealLog_Recollect(
                                    const char *szOrgFileName, const char *szBFileName, const char *szAFileName,
                                    const char *szReferTime, const char *szCompleteTime, const char cDealFlag)
{
	sprintf(m_szProcessId,"%s", DEALLOG_RECOLLECT_CODE);
	char szLogFileName[256];
	sprintf(szLogFileName, "%d_%s_%s_%s_%s_%s",
	                                     DEALLOG_RECOLLECT_CODE,
	                                     m_szPipeId, 
	                                     m_szProcessId,
	                                     szOrgFileName,
	                                     szCompleteTime,
	                                     DEALLOG_SUFFIX);
	char szLog[1024];
	sprintf(szLog, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%c", m_szPipeId,
	                                                 DEALLOG_SEPERATOR,   
	                                                 m_szProcessId,
	                                                 DEALLOG_SEPERATOR,
	                                                 szOrgFileName,
	                                                 DEALLOG_SEPERATOR,
	                                                 szBFileName,
	                                                 DEALLOG_SEPERATOR,
	                                                 szAFileName,
	                                                 DEALLOG_SEPERATOR,
	                                                 szReferTime,
	                                                 DEALLOG_SEPERATOR,
	                                                 szCompleteTime,
	                                                 DEALLOG_SEPERATOR,
	                                                 cDealFlag);
	return WriteLog(szLogFileName, szLog, szCompleteTime);
}
	                                     
	                                     
bool CF_CDealLog::DealLog_Trans(const char *szFileName, const char *szStartTime,
                          const char *szEndTime, int iFileSize, const char *szOrgMachine,
                          const char *szOrgPath, const char *szDestMachine, const char *szDestPath)
{
	strcpy(m_szPipeId, "TRANS");
	sprintf(m_szProcessId,"%d", DEALLOG_TRANS_CODE);
	char szLogFileName[256];
	sprintf(szLogFileName,"%d_%s_%s_%s_%s_%s",
	                                       DEALLOG_TRANS_CODE,
	                                       m_szPipeId,
	                                       m_szProcessId,
	                                       szFileName,
	                                       szEndTime,
	                                       DEALLOG_SUFFIX);
	char szLog[1024];
	sprintf(szLog,"%s%c%s%c%s%c%s%c%s%c%d%c%s%c%s%c%s%c%s",
	                  m_szPipeId,
	                  DEALLOG_SEPERATOR,
	                  m_szProcessId,
	                  DEALLOG_SEPERATOR,
	                  szFileName,
	                  DEALLOG_SEPERATOR,
	                  szStartTime,
	                  DEALLOG_SEPERATOR,
	                  szEndTime,
	                  DEALLOG_SEPERATOR,
	                  iFileSize,
	                  DEALLOG_SEPERATOR,
	                  szOrgMachine,
	                  DEALLOG_SEPERATOR,
	                  szOrgPath,
	                  DEALLOG_SEPERATOR,
	                  szDestMachine,
	                  DEALLOG_SEPERATOR,
	                  szDestPath);
	return WriteLog(szLogFileName, szLog, szEndTime);
}
	
bool CF_CDealLog::DealLog_Recv(const char *szFileName, const char *szRecvTime, int iFileSize, const char *szOrgFileName)
{
	char szLogFileName[256];
	sprintf(szLogFileName,"%d_%s_%s_%s_%s_%s",
	                                       DEALLOG_RECV_CODE,
	                                       m_szPipeId,
	                                       m_szProcessId,
	                                       szFileName,
	                                       szRecvTime,
	                                       DEALLOG_SUFFIX);
	char szLog[1024];
	sprintf(szLog, "%s%c%s%c%s%c%s%c%d%c%s",
	               m_szPipeId,
	               DEALLOG_SEPERATOR,
	               m_szProcessId,
	               DEALLOG_SEPERATOR,
	               szFileName,
	               DEALLOG_SEPERATOR,
	               szRecvTime,
	               DEALLOG_SEPERATOR,
	               iFileSize,
	               DEALLOG_SEPERATOR,
	               szOrgFileName);
  return WriteLog(szLogFileName, szLog, szRecvTime);
}
bool CF_CDealLog::DealLog_Prep(const char *szFileName, const char *szLocalNet,
                               const char *szSourceId, const char *szStartTime, 
                               const char *szDealTime,int iTotalCdr, 
                               int iNormalCdr,int iPickCdr,
                               int iOutCdr,const char *szStartCdrTime, 
                               const char *szEndCdrTime)
{
	char szLogFileName[256];
	sprintf(szLogFileName,"%d_%s_%s_%s_%s_%s",
	                                       DEALLOG_MAIN_CODE,
	                                       m_szPipeId,
	                                       m_szProcessId,
	                                       szFileName,
	                                       szDealTime,
	                                       DEALLOG_SUFFIX);
	char szLog[1024];
	sprintf(szLog, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%d%c%d%c%d%c%d%c%d%c%s%c%s%",
	               m_szPipeId,
	               DEALLOG_SEPERATOR,
	               m_szProcessId,
	               DEALLOG_SEPERATOR,
	               szFileName,
	               DEALLOG_SEPERATOR,
	               szStartTime,
	               DEALLOG_SEPERATOR,
	               szDealTime,
	               DEALLOG_SEPERATOR,
	               szLocalNet,
	               DEALLOG_SEPERATOR,
	               szSourceId,
	               DEALLOG_SEPERATOR,
	               iTotalCdr,
	               DEALLOG_SEPERATOR,
	               iNormalCdr,
	               DEALLOG_SEPERATOR,
	               iPickCdr,
	               DEALLOG_SEPERATOR,
	               0,                    //lackinfo cdr
	               DEALLOG_SEPERATOR,
	               iOutCdr,
	               DEALLOG_SEPERATOR,
	               szStartCdrTime,
	               DEALLOG_SEPERATOR,
	               szEndCdrTime);
  return WriteLog(szLogFileName, szLog, szDealTime);
}
bool CF_CDealLog::DealLog_Billing(const char *szFileName,const char *szLocalNet,
                       const char *szSourceId,const char *szStartTime,
                       const char *szEndTime,  int iTotalCdr,int iNormalCdr,int iPickCdr,
                       int iLackInfoCdr,int iOutCdr)
{
	char szLogFileName[256];
	sprintf(szLogFileName,"%d_%s_%s_%s_%s_%s",
	                                       DEALLOG_MAIN_CODE,
	                                       m_szPipeId,
	                                       m_szProcessId,
	                                       szFileName,
	                                       szEndTime,
	                                       DEALLOG_SUFFIX);
	char szLog[1024];
	sprintf(szLog, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%d%c%d%c%d%c%d%c%d%c%c",
	               m_szPipeId,
	               DEALLOG_SEPERATOR,
	               m_szProcessId,
	               DEALLOG_SEPERATOR,
	               szFileName,
	               DEALLOG_SEPERATOR,
	               szStartTime,
	               DEALLOG_SEPERATOR,
	               szEndTime,
	               DEALLOG_SEPERATOR,
	               szLocalNet,
	               DEALLOG_SEPERATOR,
	               szSourceId,
	               DEALLOG_SEPERATOR,
	               iTotalCdr,
	               DEALLOG_SEPERATOR,
	               iNormalCdr,
	               DEALLOG_SEPERATOR,
	               iPickCdr,
	               DEALLOG_SEPERATOR,
	               iLackInfoCdr,
	               DEALLOG_SEPERATOR,
	               iOutCdr,
	               DEALLOG_SEPERATOR,
	               DEALLOG_SEPERATOR
	               );
  return WriteLog(szLogFileName, szLog, szEndTime);
}	                      
	
	
	
	
bool CF_CDealLog::Init_Pipe(const char *szPipeId, int iProcessId, const char *szEnvFile)
{
	strcpy(m_szPipeId, szPipeId);
	sprintf(m_szProcessId,"%d", iProcessId);
	
	return GetLogPath(szEnvFile, m_szLogPath, m_bLogFlag);
}

bool CF_CDealLog::Init_Source(const char *szSourceId, const char *szEnvFileName)
{
	strcpy(m_szPipeId, szSourceId);
	return GetLogPath(szEnvFileName, m_szLogPath,m_bLogFlag);
}
bool CF_CDealLog::Init_None(const char *szEnvFile)
{
	return GetLogPath(szEnvFile, m_szLogPath, m_bLogFlag);
}

bool CF_CDealLog::GetLogPath(const char *szEnvFileName, char *szLogPath, bool &bLogFlag)
{
	char tmp[256];
	int ret = getEnv(szEnvFileName, DEALLOG_PATH_NAME, tmp);
	if (ret == -1)
	{
		char msg[500];
		sprintf(msg, "Cannot Open File:%s!", szEnvFileName);
    throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, DEALLOG_ERR_IN_OPEN_FILE,  msg, __FILE__, __LINE__);
  }
  if (ret == -2)
  {
  	//char msg[500];
  	//sprintf(msg, "Cannot get Env %s  from zhjs.env",DEALLOG_PATH_NAME);
    //throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,errno, DEALLOG_ERR_IN_GET_ENV,  msg, __FILE__, __LINE__);
    tmp[0] = NULL;
  }
  strcpy(m_szLogPath, tmp);
  ret = getEnv(szEnvFileName, DEALLOG_MODE_NAME, tmp);
	if (ret == -1)
	{
		char msg[500];
		sprintf(msg, "Cannot Open File:%s!", szEnvFileName);
    throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, DEALLOG_ERR_IN_OPEN_FILE,  msg, __FILE__, __LINE__);
  }
  if (ret = -2)
  {//没有指定DEALLOG_DIR_MODE 则默认为是false
  	bLogFlag = true;
  	return true;
  }
  
  if (strcmp(tmp,"Y") == 0)
  {
  	bLogFlag = true;
  }
  else 
  {
  	bLogFlag = false;
  }
}
	



bool CF_CDealLog::WriteLog(const char *szLogFileName, const char *szLog, const char *szDealTime)
{
	if (strlen(m_szLogPath) == 0)
	{
		return true;
	}
	
	
	char szFullName[256];
	memset(szFullName, 0, sizeof(szFullName));
	sprintf(szFullName, "%s", m_szLogPath);
	completeDir(szFullName);
	chkDir(szFullName);
	if (m_bLogFlag)
	{ //进一步建立目录层次
		strcat(szFullName, m_szPipeId);
		completeDir(szFullName);
		chkDir(szFullName);
		strcat(szFullName, m_szProcessId);
		completeDir(szFullName);
		chkDir(szFullName);
		char szDateTime[9];
		strncpy(szDateTime, szDealTime, 8);
		szDateTime[8] = NULL;
		strcat(szFullName, szDateTime);
		completeDir(szFullName);
		chkDir(szFullName);
	}
	strcat(szFullName, szLogFileName);
	char szTempName[256];
	sprintf(szTempName, "%s.tmp", szFullName);
	if (OutStream)
	{
		OutStream.close();
	}
	OutStream.open(szTempName);
	if (!OutStream)
	{
		char msg[500];
		sprintf(msg, "Cannot Open File:%s!", szTempName);
    throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, DEALLOG_ERR_IN_OPEN_FILE,  msg, __FILE__, __LINE__);
  }
  OutStream<<szLog<<endl;
	if (!OutStream.good())
	{
		char msg[500];
		sprintf(msg, "Cannot Write File:%s!", szTempName);
    throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, DEALLOG_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
  }
  OutStream.close();
  rename(szTempName, szFullName);
  return true;
}













