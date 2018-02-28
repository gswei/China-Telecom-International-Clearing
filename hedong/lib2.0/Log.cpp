// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "Log.h"

CLog theLog;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog(bool bOutput)
: streambuf(), ostream((streambuf*) this)
{
	m_bOutput = bOutput;

	char pchProcessID[10];
	sprintf(pchProcessID, "[%06d]", getpid());
	m_strProcessID = pchProcessID;
	m_strLogPath = "";
}

CLog::~CLog()
{
	
}

int CLog::overflow(int nCh)
{
	if (nCh == ende || nCh == endw || nCh == endi || nCh == enda)
	{
		string strType;
		switch(nCh)
		{
		case endi:
			strType = "[信息]";
			break;
		case endw:
			strType = "[警告]";
			break;
		case ende:
			strType = "[出错]";
			break;
		case enda:
			strType = "[归档]";
			break;
		}
		
		string strTime; 
		CLog::GetCurrentTime(strTime);

		m_ofs << strTime << m_strProcessID << "    "
			<< strType << " " << m_strBuf << endl;
		m_ofs.flush();
		
		if (m_bOutput)
		{
			clog << strTime << m_strProcessID << "    "
				<< strType << " " << m_strBuf << endl;
		}
		
		m_strBuf = "";
	}
	else
	{
		m_strBuf += nCh;
	}
	return nCh;
}

void CLog::SetOutPutFlag(bool bOutput)
{
	m_bOutput = bOutput;
}

void CLog::SetLogPath(const char* pchLogPath)
{
	if(pchLogPath != NULL)
		m_strLogPath = pchLogPath;
}

void CLog::ResetPid(void)
{
	char pchProcessID[10];
	sprintf(pchProcessID, "[%06d]", getpid());
	m_strProcessID = pchProcessID;
}

int CLog::Open(const string& strFile)
{
	char pchProcessID[10];
	sprintf(pchProcessID, "[%06d]", getpid());
	m_strProcessID = pchProcessID;
	
	m_ofs.close();
	m_ofs.clear();
	string strFullName = m_strLogPath + strFile;
	m_ofs.open(strFullName.c_str(), ios::out|ios::app);
	if (!m_ofs.good()) 
		return -1;
	
	return 1;
}
