// SchInfo.cpp: implementation of the CSchInfo class.
//
//////////////////////////////////////////////////////////////////////

#include <errno.h>

#include "Common.h"
#include "CF_CError.h"
#include "COracleDB.h"
#include "SchInfo.h"

#define QUERY_RETURN_NONE(sql, errcode) \
	{\
		char pchMsg[ERROR_MSG_LEN]; \
		sprintf(pchMsg, "%s returned no rows.", sql); \
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, errcode, errno, pchMsg, __FILE__, __LINE__); \
	}
	
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSchInfo::CSchInfo()
: m_nProcessID(-1)
{
	memset(m_pchPipeID, 0, sizeof(m_pchPipeID)); 
}

CSchInfo::~CSchInfo()
{

}

bool CSchInfo::SetPipeID(char* pchPipeID)
{
	if(pchPipeID == NULL || strlen(pchPipeID) > 5)
		return false;

	strcpy(m_pchPipeID, pchPipeID);
	return true;
}

bool CSchInfo::SetProcessID(int nProcessID)
{
	if(nProcessID < 0)
		return false;
	
	m_nProcessID = nProcessID;
	return true;
}

int CSchInfo::Init()
{
	if(QueryWorkflowID() != 0)
		return -1;

	if(QueryInterfaceID() != 0)
		return -1;

	if(QueryInterfaceInfo() != 0)
		return -1;

	if(QueryRecordType() != 0)
		return -1;

	if(QuerySource() < -1)
		return -1;

	return 0;
}

int CSchInfo::QueryWorkflowID()
{
	char pchSql[MAX_SQL_LENGTH];
	CBindSQL ds(DBConn);

	sprintf(pchSql, "SELECT WORKFLOW_ID FROM PIPE "
		"WHERE PIPE_ID = '%s'", m_pchPipeID);

	ds.Open(pchSql,	SELECT_QUERY);

	if (!(ds >> m_nWorkflowID))
		QUERY_RETURN_NONE(pchSql, SCHINFO_ERR_CONFIG_PIPE);

	ds.Close();

	return 0;
}

int CSchInfo::QueryInterfaceID()
{
	char pchSql[MAX_SQL_LENGTH];
	CBindSQL ds(DBConn);

	sprintf(pchSql, "SELECT INPUT_ID,OUTPUT_ID FROM WORKFLOW "
		"WHERE PROCESS_ID = %d AND WORKFLOW_ID = %d", m_nProcessID, m_nWorkflowID);

	ds.Open(pchSql, SELECT_QUERY);

	if (!(ds >> m_nInputID >> m_nOutputID)) 
		QUERY_RETURN_NONE(pchSql, SCHINFO_ERR_CONFIG_INTERFACE);

	ds.Close();

	return 0;   
}

int CSchInfo::QueryInterfaceInfo()
{
	char pchSql[MAX_SQL_LENGTH];
	CBindSQL ds(DBConn);

	//input info
	sprintf(pchSql, "SELECT CTL_TABNAME, PATH, FILETYPE_ID FROM MODEL_INTERFACE "
		"WHERE INTERFACE_ID = %d", m_nInputID);

	ds.Open(pchSql, SELECT_QUERY);

	if (!(ds >> m_pchInCtlTabname >> m_pchInPath >> m_pchInFileType)) 
		QUERY_RETURN_NONE(pchSql, SCHINFO_ERR_CONFIG_MODEL_INTERFACE);
	
	ds.Close();

	if (m_pchInPath[strlen(m_pchInPath) -1] != '/')
		strcat(m_pchInPath, "/");
	
	//output info
	sprintf(pchSql, "SELECT CTL_TABNAME, PATH, FILETYPE_ID FROM MODEL_INTERFACE "
		"WHERE INTERFACE_ID = %d", m_nOutputID);

	ds.Open(pchSql, SELECT_QUERY);
	
	if (!(ds >> m_pchOutCtlTabname >> m_pchOutPath >> m_pchOutFileType)) 
		QUERY_RETURN_NONE(pchSql, SCHINFO_ERR_CONFIG_MODEL_INTERFACE);

	ds.Close();
	
	if (m_pchOutPath[strlen(m_pchOutPath) -1] != '/')
		strcat(m_pchOutPath, "/");

	return 0;
}

int CSchInfo::QueryRecordType()
{
	char pchSql[MAX_SQL_LENGTH];
	CBindSQL ds(DBConn);

	//input file type
	sprintf(pchSql, "SELECT RECORD_TYPE FROM FILETYPE_DEFINE "
		"WHERE FILETYPE_ID = '%s'", m_pchInFileType);

	ds.Open(pchSql, SELECT_QUERY);

    if (!(ds >> m_cInRecordType))
		QUERY_RETURN_NONE(pchSql, SCHINFO_ERR_CONFIG_FILETYPE_DEFINE);

	ds.Close();
	
	//output file type
	sprintf(pchSql, "SELECT RECORD_TYPE FROM FILETYPE_DEFINE "
		"WHERE FILETYPE_ID = '%s'", m_pchOutFileType);

	ds.Open(pchSql, SELECT_QUERY);

	if(!(ds>>m_cOutRecordType)) 
		QUERY_RETURN_NONE(pchSql, SCHINFO_ERR_CONFIG_FILETYPE_DEFINE);

	ds.Close();

	return 0;
}

int CSchInfo::QuerySource()
{
	SSourceInfo srcInfo;
	int nCount = 0;
	char pchSql[MAX_SQL_LENGTH];
	CBindSQL ds(DBConn);

	sprintf(pchSql, "SELECT SOURCE_ID, SOURCE_PATH FROM SOURCE "
		"WHERE PIPE_ID = '%s'", m_pchPipeID);

	ds.Open(pchSql, SELECT_QUERY);

	while(ds >> srcInfo.m_pchSourceID >> srcInfo.m_pchSourcePath) 
	{
		if (srcInfo.m_pchSourcePath[strlen(srcInfo.m_pchSourcePath) -1] != '/')
			strcat(srcInfo.m_pchSourcePath, "/");

		m_vecSrcInfo.push_back(srcInfo);
		nCount++;
	}

	ds.Close();

	if(nCount <= 0)
		QUERY_RETURN_NONE(pchSql, SCHINFO_ERR_CONFIG_SOURCE);
	
	return nCount;
}

int CSchInfo::GetInputFile(const char* pchSrcID, vector<SFileInfo> &vecFiles)
{
	int nCount = 0;
	SFileInfo fileInfo;
	char pchSql[MAX_SQL_LENGTH];
	
	CBindSQL ds(DBConn);

	sprintf(pchSql, "SELECT FILENAME FROM %s "
		"WHERE SOURCE_ID = '%s' AND DEAL_FLAG = 'W' AND VALIDFLAG = 'Y' "
		"ORDER BY FILENAME", m_pchInCtlTabname, pchSrcID);

	ds.Open(pchSql, SELECT_QUERY);

	while(ds >> fileInfo.m_strFileName) 
	{
		vecFiles.push_back(fileInfo);
		nCount++;
	}

	ds.Close();

	if(nCount <= 0)
		cout << "There is no input file!" << endl;

	return nCount;
}

int CSchInfo::UpdateInputFileState(const char* pchSrcID, vector<SFileInfo> &vecFiles)
{
	vector<SFileInfo>::iterator it;
	for(it = vecFiles.begin(); it != vecFiles.end(); it++)
	{
		if(UpdateInputFileState(pchSrcID, *it) != 0)
		{
			DBConn.Rollback();
			return -1;
		}
	}

	return 0;
}

int CSchInfo::UpdateInputFileState(const char* pchSrcID, SFileInfo &fileInfo)
{
	char pchSql[MAX_SQL_LENGTH];

	CBindSQL ds(DBConn);

	sprintf(pchSql, "UPDATE %s "
		"SET DEAL_FLAG = '%c', DEALSTARTTIME = '%s', DEALENDTIME = '%s', "
		"INPUT_COUNT = %d, MAINFLOW_COUNT = %d, ERROR_COUNT = %d, "
		"LACKINFO_COUNT = %d, PICK_COUNT = %d, OTHER_COUNT = %d "
		"WHERE SOURCE_ID = '%s' AND FILENAME ='%s'",
		m_pchInCtlTabname, fileInfo.m_cFileState, 
		fileInfo.m_strDealStartTime.c_str(), fileInfo.m_strDealEndTime.c_str(),
		fileInfo.m_nInputCnt, fileInfo.m_nMainFlowCnt, fileInfo.m_nErrCnt,
		fileInfo.m_nLackInfoCnt, fileInfo.m_nPickCnt, fileInfo.m_nOtherCnt,
		pchSrcID, fileInfo.m_strFileName.c_str());
	
	ds.Open(pchSql, NONSELECT_DML);
	
	try
	{
		ds.Execute();
	}
	catch(CF_CError e) 
	{
		ds.Close();
		DBConn.Rollback();
		
		throw CF_CError(ERR_TYPE_DB, ERR_LEVEL_MID, 
			SCHINFO_ERR_UPDATE_TABLE, e.get_appErrorCode(), 
			e.get_errMessage(), __FILE__, __LINE__);
	}
	
/*
	if (ds.IsError())
	{
		ds.Close();
		DBConn.Rollback();

		char pchMsg[MAX_MSG_LENGTH]; 
		sprintf(pchMsg, "%s", ds.GetLastDBErrorMsg().c_str()); 
		throw CF_CError(ERR_TYPE_DB, ERR_LEVEL_MID, 
			ds.GetLastDBErrorCode(), SCHINFO_ERR_UPDATE_TABLE, 
			pchMsg, __FILE__, __LINE__);
	}
*/
	ds.Close();

	return 0;
}

int CSchInfo::InsertOutputFile(const char* pchSrcID, vector<SFileInfo> &vecFiles)
{
	vector<SFileInfo>::iterator it;
	for(it = vecFiles.begin(); it != vecFiles.end(); it++)
	{
		if(InsertOutputFile(pchSrcID, *it) != 0)
		{
			DBConn.Rollback();
			return -1;
		}
	}

	return 0;
}

int CSchInfo::InsertOutputFile(const char* pchSrcID, SFileInfo &fileInfo)
{
	char pchSql[MAX_SQL_LENGTH];
	
	CBindSQL ds(DBConn);

	sprintf(pchSql, "INSERT INTO %s "
		"(SOURCE_ID, FILENAME, DEAL_FLAG, VALIDFLAG) "
		"VALUES( '%s', '%s', '%c', 'Y' )",
		m_pchOutCtlTabname, pchSrcID, 
		fileInfo.m_strFileName.c_str(), fileInfo.m_cFileState);
	
	ds.Open(pchSql, NONSELECT_DML);
	
	try
	{
		ds.Execute();
	}
	catch(CF_CError e) 
	{
		ds.Close();
		DBConn.Rollback();
		
		throw CF_CError(ERR_TYPE_DB, ERR_LEVEL_MID, 
			SCHINFO_ERR_INSERT_TABLE, e.get_appErrorCode(), 
			e.get_errMessage(), __FILE__, __LINE__);
	}
	
/*
	if (ds.IsError())
	{
		ds.Close();
		DBConn.Rollback();

		char pchMsg[MAX_MSG_LENGTH]; 
		sprintf(pchMsg, "%s", ds.GetLastDBErrorMsg().c_str()); 
		throw CF_CError(ERR_TYPE_DB, ERR_LEVEL_MID, 
			ds.GetLastDBErrorCode(), SCHINFO_ERR_INSERT_TABLE, 
			pchMsg, __FILE__, __LINE__);
	}
*/
	ds.Close();
	
	return 0;
}

