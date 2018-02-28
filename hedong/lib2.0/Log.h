// Log.h: interface for the CLog class.
//
//////////////////////////////////////////////////////////////////////
/********************************************************************
 *	Usage		: 标准运行日志类
 *	Author		: 周亚军
 *  Create date	: 2005-04-26
 *	Version		: 1.0.0
 *	Updatelist	:
 *				  2005-05-12	周亚军	增加设置日志目录功能
 *				  2006-03-30	周亚军	增加重新设置日志显示中的pid接口
 *				  2006-04-18	周亚军	修改CLog::Open()接口,删去不用的参数nMode,以适应gcc2.9.6版本编译
 ********************************************************************/

#if !defined(_LOG_H_)
#define _LOG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "config.h"
//#include <streambuf>
//#include <ostream>

class CLog;
extern CLog theLog;

#define ende ('\001') // error
#define endw ('\002') // warning
#define endi ('\003') // information
#define enda ('\004') // archive

class CLog : public streambuf, public ostream  
{
private:
	ofstream m_ofs;
	string m_strBuf;
	string m_strProcessID;
	string m_strLogPath;
	bool m_bOutput;

public:
	typedef enum { INFO, WARNING, ERROR, ARCHIVE, UNKNOWN } INFO_TYPE;

	CLog(bool bOutput = true);
	virtual ~CLog();

//#ifdef _LINUXOS
//	int Open(const string& strFile, std::_Ios_Openmode nMode = ios::out|ios::app);
//#else
//	int Open(const string& strFile, int nMode = ios::out|ios::app);
//#endif
	
	int Open(const string& strFile);

	int overflow(int nCh = EOF);
	int underflow() { return 0;}
	
	void SetOutPutFlag(bool bOutput = true);
	void SetLogPath(const char* pchLogPath);
	void ResetPid(void);

	static void GetCurrentTime(string &strTime)
	{
		struct tm  *tm;
		time_t	t1;
		
		time(&t1);
		tm = localtime(&t1);
		
		char pchTmp[20];
		sprintf(pchTmp, "[%02d-%02d %02d:%02d:%02d]",
			tm->tm_mon+1, tm->tm_mday, 
			tm->tm_hour, tm->tm_min, tm->tm_sec);
		
		strTime = pchTmp;
	};
};

#endif // !defined(_LOG_H_)
