// SchInfo.h: interface for the CSchInfo class.
//
//////////////////////////////////////////////////////////////////////
/********************************************************************
 *	Usage		: 流水配置信息及调度表信息接口类
 *	Author		: 周亚军
 *  Create date	: 2005-03-28
 *	Version		: 1.0.1
 *	Updatelist	:
 *		2005-04-29 周亚军 使用抛出异常方式返回错误
 ********************************************************************/

#if !defined(_SCHINFO_H_)
#define _SCHINFO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "config.h"

#define MAX_SQL_LENGTH			1024
#define MAX_PATH_LENGTH			260
#define MAX_TABLENAME_LENGTH	33
#define DATETIME_STRING_LENGTH	15
#define SOURCE_ID_LENGTH		6
#define PIPE_ID_LENGTH			6
#define FILETYPE_LENGTH			6

struct SSourceInfo
{
	char m_pchSourceID[SOURCE_ID_LENGTH];
	char m_pchSourcePath[MAX_PATH_LENGTH];
};

struct SFileInfo
{
	string			m_strFileName;
	char			m_cFileState;
	string			m_strDealStartTime;
	string			m_strDealEndTime;
	unsigned int	m_nInputCnt;
	unsigned int	m_nMainFlowCnt;
	unsigned int	m_nErrCnt;
	unsigned int	m_nLackInfoCnt;
	unsigned int	m_nPickCnt;
	unsigned int	m_nOtherCnt;

	SFileInfo()
	{
		m_strFileName = "";
		m_strDealStartTime = "00000000000000";
		m_strDealEndTime = "00000000000000";
		m_nInputCnt = 0;
		m_nMainFlowCnt = 0;
		m_nErrCnt = 0;
		m_nLackInfoCnt = 0;
		m_nPickCnt = 0;
		m_nOtherCnt = 0;
	}

	void ResetCount()
	{
		m_nInputCnt = 0;
		m_nMainFlowCnt = 0;
		m_nErrCnt = 0;
		m_nLackInfoCnt = 0;
		m_nPickCnt = 0;
		m_nOtherCnt = 0;
	}
};

class CSchInfo
{
public:
	vector<SSourceInfo> m_vecSrcInfo;

private:
	char m_pchPipeID[PIPE_ID_LENGTH];		//流水线标识
	int m_nProcessID;						//处理过程序列号
	int m_nWorkflowID;						//工作流模板标识

	int m_nInputID;
	int m_nOutputID;
	char m_pchInCtlTabname[MAX_TABLENAME_LENGTH];
	char m_pchOutCtlTabname[MAX_TABLENAME_LENGTH];
	char m_pchInPath[MAX_PATH_LENGTH];
	char m_pchOutPath[MAX_PATH_LENGTH];
	char m_pchInFileType[FILETYPE_LENGTH];
	char m_pchOutFileType[FILETYPE_LENGTH];
	char m_cInRecordType;
	char m_cOutRecordType;

public:
	CSchInfo();
	virtual ~CSchInfo();

	int Init();

	bool SetPipeID(char* pchPipeID);
	bool SetProcessID(int nProcessID);

	const char* GetPipeID();
	int GetProcessID();
	const char* GetInPath();
	const char* GetOutPath();
	const char* GetInFileType();
	const char* GetOutFileType();
	char GetInRecordType();
	char GetOutRecordType();
	
	int GetInputFile(const char* pchSrcID, vector<SFileInfo> &vecFiles);
	int UpdateInputFileState(const char* pchSrcID, SFileInfo &fileInfo);
	int UpdateInputFileState(const char* pchSrcID, vector<SFileInfo> &vecFiles);
	int InsertOutputFile(const char* pchSrcID, SFileInfo &fileInfo);
	int InsertOutputFile(const char* pchSrcID, vector<SFileInfo> &vecFiles);

private:
	int QueryWorkflowID();
	int QueryInterfaceID();
	int QueryInterfaceInfo();
	int QueryRecordType();
	int QuerySource();
};

inline const char* CSchInfo::GetPipeID()
{
	return m_pchPipeID;
}

inline int CSchInfo::GetProcessID()
{
	return m_nProcessID;
}

inline const char* CSchInfo::GetInPath()
{
	return m_pchInPath;
}

inline const char* CSchInfo::GetOutPath()
{
	return m_pchOutPath;
}

inline const char* CSchInfo::GetInFileType()
{
	return m_pchInFileType;
}

inline const char* CSchInfo::GetOutFileType()
{
	return m_pchOutFileType;
}

inline char CSchInfo::GetInRecordType()
{
	return m_cInRecordType;
}

inline char CSchInfo::GetOutRecordType()
{
	return m_cOutRecordType;
}

#endif // !defined(_SCHINFO_H_)
