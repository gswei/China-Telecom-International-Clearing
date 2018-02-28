//filter_common.cpp
#include "filter_common.h"

SParameter Param;        //全局的变量，供各个文件使用

int selectFromPipeEnv(const char *szPipeId,const char *szEnvName, long &lEnvValue)
{
	char szTempValue[100];
	int ret = selectFromPipeEnv(szPipeId, szEnvName, szTempValue);
	lEnvValue = atol(szTempValue);
	return ret;
}

int selectFromPipeEnv(const char *szPipeId, int iProcessId,const  char *szEnvName, long &lEnvValue)
{
	char szTempValue[100];
	int ret = selectFromPipeEnv(szPipeId,iProcessId,szEnvName,szTempValue);
	lEnvValue = atol(szTempValue);
	return ret;
}

int selectFromPipeEnv(const char *szPipeId, int iProcessId,const  char *szEnvName, char *szEnvValue)
{
  CBindSQL ds(DBConn);
  ds.Open("select var_value from pipe_env where pipe_id=:szPipeId and varname= :varname and process_id = :process_id ", SELECT_QUERY );
  ds<<szPipeId<<szEnvName<<iProcessId;
  if (!(ds>>szEnvValue))
  {
  	return -1;
 
  }
  ds.Close();
  //expTrace(Param.szDebugFlag, __FILE__, __LINE__,"%s = %s;", szEnvName, szEnvValue);
}

int selectFromPipeEnv(const char *szPipeId, const char *szEnvName, char *szEnvValue)
{
  CBindSQL ds(DBConn);
  ds.Open("select var_value from pipe_env where pipe_id=:szPipeId and varname= :varname", SELECT_QUERY );
  ds<<szPipeId<<szEnvName;
  if (!(ds>>szEnvValue))
  {
    ds.Close();  	
  	return -1;
  	char szLogStr[500];
    sprintf(szLogStr, "select var_value from pipe_env where pipe_id='%s' and varname= '%s' is NULL", szPipeId, szEnvName);
    throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);

  }

  //expTrace(Param.szDebugFlag, __FILE__, __LINE__,"%s = %s;", szEnvName, szEnvValue);
}

//add by linyb 20071109
int selectFromSourceEnv(const char *szSourceId, int iProcessId,const  char *szEnvName, char *szEnvValue)
{
  CBindSQL ds(DBConn);
  ds.Open("select var_value from source_env where source_id=:szSourceId and varname= :varname and process_id = :process_id ", SELECT_QUERY );
  ds<<szSourceId<<szEnvName<<iProcessId;
  if (!(ds>>szEnvValue))
  {
  	return -1;
 
  }
  ds.Close();
  return 0;
  //expTrace(Param.szDebugFlag, __FILE__, __LINE__,"%s = %s;", szEnvName, szEnvValue);
}
//end of add by linyb 20071109

void HoldOn(SParameter &Param)
{
	CBindSQL ds(DBConn);
  ds.Open("update process_ctl set status = 2 where pipe_id = :pipe_id and process_id = :process_id",NONSELECT_DML);
  ds<<Param.szPipeId<<Param.iProcessId;
  ds.Execute();
  DBConn.Commit();
	for (; ;)
	{
	  expTrace(Param.szDebugFlag, __FILE__, __LINE__,
	      "HOLD ON FOR %d seconds", Param.iSleepTime);
	  sleep(Param.iSleepTime);
	  ds.Open("select runflag from process_ctl \
	           where pipe_id = :pipe_id and process_id = :process_id");
	  ds<<Param.szPipeId<<Param.iProcessId;
	  if (!(ds>>Param.iRunFlag))
	  {
	  	char szLogStr[1024];
	    sprintf(szLogStr, "select runflag from process_ctl where pipe_id= %s and \
	            process_id=%d is NULL", Param.szPipeId, Param.iProcessId);
	    expTrace(Param.szDebugFlag, __FILE__, __LINE__, szLogStr);
	    wrlog(Param.szPipeId, Param.iProcessId, (char *)"process_ctl", 
	      ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_NO_MATCH_RECORD_IN_DB, errno, NULL, 
	      szLogStr, __FILE__, __LINE__);
	    exit(-1);
	  }
	  ds.Close();
	  if (Param.iRunFlag != 2)
	  {//不需要处理暂停状态
	  	break;
	  }
	}
}



//用填充符filter填充字符串str，直到长为len
void fillStr(char *str, long len, char filter)
{
	long i;
	for (i = 0; i < len; i++)
	{
		if (str[i] == 0)
		{
			break;
		}
	}
	for (;i < len; i++)
	{
		str[i] = filter;
	}
	str[i] = 0;
}

bool checkTimeFormat(const char *szTimeStr)
{
	//长度必须14字符
	if (strlen(szTimeStr) != 14)
	{
		return false;
	}
	//后面的4字符必须为数字
	for (int i = 10; i < 13; i++)
	{
		if (szTimeStr[i] > '9' || szTimeStr[i] < '0')
		{
			return false;
		}
	}
	//delete by linyb 20080506
	/*
	if(strncmp(szTimeStr + 10,"00",2) < 0 || strncmp(szTimeStr + 10,"59",2) > 0)
	{
		return false;
	}
	if(strncmp(szTimeStr + 12,"00",2) < 0 || strncmp(szTimeStr + 12,"59",2) > 0)
	{
		return false;
	}
	*/
	//end of delete by linyb 20080506
	return true;
}
//根据输入的字符串计算出一个索引值
unsigned long calcIndex(const char *szIndexStr)
{
	unsigned long lIndexValue = 0;
	for (int i = 0; szIndexStr[i] != NULL; i++)
	{
		if (isdigit(szIndexStr[i]))
		{
			lIndexValue += szIndexStr[i] - '0';
		}
		else if (' ' == szIndexStr[i])
		{
			continue;
		}
		else
		{
			lIndexValue += szIndexStr[i]%10;
		}
		if (i > 0)
		{
			lIndexValue *= 11;   //使用质数11，增加结果的发散性
		}
	}
	return lIndexValue;
}
		

bool addHours(int nHours, const char *szOrgTime, char *szDestTime)
{
	char szHour[3];
	memset(szHour, 0, sizeof(szHour));
	strncpy(szHour, szOrgTime + 8,2);
	int iHour = atoi(szHour);
	if (iHour > 23 ||iHour < 0)
	{
		return false;
	}
	
	int iNewHour = iHour + nHours;
	if (iNewHour > 23 || iNewHour < 0)
	{
		long iDestTime = timeStr2Time(szOrgTime) + nHours*60*60;
	  time2TimeStr(iDestTime, szDestTime);
	  return true;
	}
	else
	{
		sprintf(szHour, "%02d", iNewHour);
		strcpy(szDestTime, szOrgTime);
		strncpy(szDestTime + 8, szHour, 2);
		return true;
	}
} 


int paramInit()
{
	char szLogStr[1024];
	char szTemp[1024];
	CBindSQL ds(DBConn);
	//*********************************************************************
	// 从pipe_env中查询szDebugFlag的值
	// （不同模块查询的变量不同，根据实际需要作修改）
	//*********************************************************************
	if (selectFromPipeEnv(Param.szPipeId, Param.iProcessId, "DEBUG_FLAG", Param.szDebugFlag) == -1)
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select var_value from pipe_env where pipe_id='%s' and varname= '%s' is NULL", Param.szPipeId, "PICK_DEBUG_FLAG");
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	if (selectFromPipeEnv(Param.szPipeId,Param.iProcessId, "FILTER_BLOCK_NUM", Param.lBlockNum) == -1)
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select var_value from pipe_env where pipe_id='%s' and varname= '%s' is NULL", Param.szPipeId, "FILTER_BLOCK_NUM");
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	if (selectFromPipeEnv(Param.szPipeId,Param.iProcessId, "FILTER_INDEX_TABLE_NUM", Param.lIndexTableNum) == -1)
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select var_value from pipe_env where pipe_id='%s' and varname= '%s' is NULL", Param.szPipeId, "FILTER_INDEX_TABLE_NUM");
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	//delete by linyb 20071109
	//if (selectFromPipeEnv(Param.szPipeId,Param.iProcessId, "PICK_CONFIG_ID", Param.szPickConfigId) == -1)
	//{
	// 	char szLogStr[500];
	// sprintf(szLogStr, "select var_value from pipe_env where pipe_id='%s' and varname= '%s' is NULL", Param.szPipeId, "PICK_CONFIG_ID");
	//  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	//}
	//end of delete by linyb 20071109
	if (selectFromPipeEnv(Param.szPipeId,Param.iProcessId, "INFILE_BAK_FLAG",Param.szBackupFlag) == -1)
	{
		strcpy(Param.szBackupFlag, "N");
	}
	//delete by linyb 20071109
	//取去重关键字压缩标志，默认为false
	/*ds.Open("SELECT COMPRESS_FLAG FROM PICK_CONF_DEFINE WHERE PICK_GRP_ID =:PICK_GRP_ID");
	ds<<Param.szPickConfigId;
	if (!(ds>>szTemp))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "PICK_CONFIG_ID(%s) NOT IN PICK_CONF_DEFINE", Param.szPickConfigId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	if ('Y' == szTemp[0] || 'y' == szTemp[0])
	{
		Param.bCompressFlag = true;
	}
	else
	{
		Param.bCompressFlag = false;
	}
	*/
	//end of delete by linyb 20071109
	if (selectFromPipeEnv(Param.szPipeId,Param.iProcessId, "FILTER_DUP_DEAL_FLAG",Param.szDupDealFlag) == -1)
	{
		strcpy(Param.szDupDealFlag, "N");
	}
	
	if (selectFromPipeEnv(Param.szPipeId,Param.iProcessId, "FILTER_INDEX_MODE",Param.szFilterIndexMode) == -1)
	{
		strcpy(Param.szFilterIndexMode, "S");
	}
	
	//表达式需要在前面空出两个字符
	strcpy(Param.szFilterCondition,"\t\t");
	if (selectFromPipeEnv(Param.szPipeId,Param.iProcessId, "FILTER_CONDITION",Param.szFilterCondition + 2) == -1)
	{
		strcpy(Param.szFilterCondition, "'false'");
	}
	
	
	
	
	if (getEnvFromDB(DBConn, Param.szPipeId,Param.iProcessId, "DUP_INDEX_PATH",Param.szDupIndexPath) == -1)
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "Cannot get DUP_INDEX_PATH from PIPE_ENV or GLOBAL_ENV");
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}

	completeDir(Param.szDupIndexPath);
	

	//*********************************************************************
	// 查询输入输出接口ID（interface_id）
	//*********************************************************************
	ds.Open("select input_id, output_id from workflow where process_id=:iProcessId \
	  and workflow_id=(select workflow_id from pipe where pipe_id=:szPipeId)", SELECT_QUERY );
	ds<<Param.iProcessId<<Param.szPipeId;
	if (!(ds>>Param.iInputId>>Param.iOutputId))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select input_id, output_id from workflow where process_id=%d and workflow_id=(select workflow_id from pipe where pipe_id=%s) is NULL",Param.iProcessId, Param.szPipeId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();

	
	//*********************************************************************
	// 查询工作流模板ID（workflow_id）
	//*********************************************************************
	ds.Open("select workflow_id from pipe where pipe_id=:szPipeId", 
	  SELECT_QUERY );
	ds<<Param.szPipeId;
	if (!(ds>>Param.iWorkflowId))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select workflow_id from pipe where pipe_id=%d is NULL",Param.iProcessId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();

	
	//*********************************************************************
	// 查询输入输出控制表名和路径
	//*********************************************************************
	ds.Open("select ctl_tabname, path from model_interface where \
	  interface_id=:iInputId", SELECT_QUERY );
	ds<<Param.iInputId;
	if (!(ds>>Param.szInCtlTabname>>Param.szInPath))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select ctl_tabname, path from model_interface where interface_id=%d is NULL", Param.iInputId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();

	
	if (Param.szInPath[strlen(Param.szInPath)-1] != '/')
	  strcat(Param.szInPath, "/");
	
	ds.Open("select ctl_tabname, path from model_interface where \
	  interface_id=:iOutputId", SELECT_QUERY );
	ds<<Param.iOutputId;
	if (!(ds>>Param.szOutCtlTabname>>Param.szOutPath))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "No matching record in model_interface!");
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();

	
	if (Param.szOutPath[strlen(Param.szOutPath)-1] != '/')
	  strcat(Param.szOutPath, "/");
	
	//*********************************************************************
	// 获得输入输出文件类型
	//*********************************************************************
	ds.Open("select filetype_id from model_interface where interface_id=:in_id", SELECT_QUERY );
	ds<<Param.iInputId;
	if (!(ds>>Param.szInputFiletypeId))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select filetype_id from model_interface where interface_id=%d is NULL", Param.iInputId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();
	
	ds.Open("select filetype_id from model_interface where interface_id=:out_id", SELECT_QUERY );
	ds<<Param.iOutputId;
	if (!(ds>>Param.szOutputFiletypeId))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select filetype_id from model_interface where interface_id=%d is NULL", Param.iOutputId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();
	
	delSpace(Param.szInputFiletypeId, 0);
	delSpace(Param.szOutputFiletypeId, 0);

	
	//*********************************************************************
	// 获取输入输出记录类型
	//*********************************************************************
	ds.Open("select record_type from filetype_define where filetype_id= \
	  :filetype_id", SELECT_QUERY );
	ds<<Param.szInputFiletypeId;
	if (!(ds>>Param.szInrcdType))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select record_type from filetype_define where filetype_id= %s is NULL",Param.szInputFiletypeId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();
	
	ds.Open("select record_type from filetype_define where filetype_id=:filetype_id", SELECT_QUERY );
	ds<<Param.szOutputFiletypeId;
	if (!(ds>>Param.szOutrcdType))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "select record_type from filetype_define where filetype_id=%s is NULL",Param.szOutputFiletypeId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();
	
	Param.inrcd.Init(Param.szInputFiletypeId);
	Param.outrcd.Init(Param.szOutputFiletypeId);
	
	//为动态编译器绑定变量
	int iColIndex;
	char szFieldName[256];
	ds.Open("select col_index,colname from txtfile_fmt where filetype_id = :input_filetype_id");
	ds<<Param.szInputFiletypeId;
	while((ds>>iColIndex>>szFieldName))
	{
		Param.theCompile.DefineVariable(szFieldName, Param.inrcd.Get_Field(iColIndex));
	}
	ds.Close();
	//绑定文件名
	Param.theCompile.DefineVariable("FILENAME",Param.szFileName);

	//add by linyb 20071109
	char szSql[SQL_LEN+1];
	sprintf(szSql, "select SOURCE_ID from SOURCE where PIPE_ID = '%s'", Param.szPipeId);
	ds.Open(szSql, SELECT_QUERY);
	char szSourceId[10];
	while(ds>>szSourceId)
	{
		SSourceEnv *pSourceEnv = new SSourceEnv();
		if(pSourceEnv == NULL)
		{
			char szLogStr[500];
	  		sprintf(szLogStr, "can't new SSourceEnv() memory exhaust !! ");
	 		 throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,0, errno,  szLogStr, __FILE__, __LINE__);
		}
		if(selectFromSourceEnv(szSourceId, Param.iProcessId, "PICK_CONFIG_ID", pSourceEnv->m_szPickConfigId)==-1)
		{
			char szLogStr[500];
	  		sprintf(szLogStr, "can't get PICK_CONFIG_ID from source_env where source_id = %s and process_id = %d", szSourceId, Param.iProcessId);
	  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}

		//取去重关键字压缩标志，默认为false
		CBindSQL dsTemp(DBConn);
		dsTemp.Open("SELECT COMPRESS_FLAG FROM PICK_CONF_DEFINE WHERE PICK_GRP_ID =:PICK_GRP_ID");
		dsTemp<<pSourceEnv->m_szPickConfigId;
		if (!(dsTemp>>szTemp))
		{
	 		char szLogStr[500];
	  		sprintf(szLogStr, "PICK_CONFIG_ID(%s) NOT IN PICK_CONF_DEFINE", pSourceEnv->m_szPickConfigId);
	  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
		//edit by linyb 20080108
		if ('Y' == szTemp[0] || 'y' == szTemp[0])
		{
			pSourceEnv->m_iCompressFlag= 1;
		}
		else if('T' == szTemp[0] || 't' == szTemp[0])
		{
			pSourceEnv->m_iCompressFlag=2;
		}
		else
		{
			pSourceEnv->m_iCompressFlag= 0;
		}
		//end of edit by linyb 20080108
		dsTemp.Close();

		//******************************************************************
		//读取去重关键字的时间字段
		//******************************************************************
		dsTemp.Open("select a.colname,pick_time_flag,pick_key_type,pick_key_format, a.filetype_id,col_len from pick_key a,txtfile_fmt b where pick_grp_id = :pick_config_id \
	     and pick_time_flag = 'Y' and a.filetype_id = b.filetype_id and a.colname = b.colname ", SELECT_QUERY);
		char szFiletypeId[20];
		dsTemp<<pSourceEnv->m_szPickConfigId;    
		if (!(dsTemp>>pSourceEnv->m_PickKey[0].m_szColName>>pSourceEnv->m_PickKey[0].m_cPickTimeFlag>>pSourceEnv->m_PickKey[0].m_cPickKeyType>>pSourceEnv->m_PickKey[0].m_szPickKeyFormat>>szFiletypeId>>pSourceEnv->m_PickKey[0].m_iFieldLen))
		{
	 		char szLogStr[500];
	  		sprintf(szLogStr, "No matching colname in pick_key where pick_grp_id = %s and pick_time_flag = 'Y'",pSourceEnv->m_szPickConfigId);
	  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
		dsTemp.Close();
		if ('S' == pSourceEnv->m_PickKey[0].m_cPickKeyType)
		{
			if (!pSourceEnv->m_PickKey[0].m_StrExp.init(pSourceEnv->m_PickKey[0].m_szPickKeyFormat))
			{
			 	char szLogStr[500];
			  	sprintf(szLogStr, "PICK_KEY_FORMAT(%s) IN PICK_KEY ERROR!",pSourceEnv->m_PickKey[0].m_szPickKeyFormat);
		  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
			}
		}
		else if ('I' == pSourceEnv->m_PickKey[0].m_cPickKeyType)
		{
			//表示用公式编辑器得到字符串
			char conText[200];
			sprintf(conText, "\t\t%s", pSourceEnv->m_PickKey[0].m_szPickKeyFormat);
			sprintf(pSourceEnv->m_PickKey[0].m_szPickKeyFormat, "%s", conText);

			char result[500] = "";
			//add by linyb 20080430
			memset(result, 0, sizeof(result));
			//end of add by linyb 20080430
			int errorno = 0;
			//预编译
			Param.theCompile.Operation( result,sizeof(result)-1, &errorno, pSourceEnv->m_PickKey[0].m_szPickKeyFormat);
			if ( errorno )
			{
				char errorMsg[ERROR_MSG_LEN+1];
				sprintf(errorMsg,"excute a compiler =%s=%d err",pSourceEnv->m_PickKey[0].m_szPickKeyFormat, errorno);
				throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,0,errno,errorMsg,__FILE__,__LINE__);
			}
		}
		else
		{
			pSourceEnv->m_PickKey[0].m_cPickKeyType = 'F';
		}
		if (strcmp(szFiletypeId, Param.szInputFiletypeId) != 0)
		{
	 		char szLogStr[500];
			sprintf(szLogStr, "the InputFiletypeId=%s= not match the filetype_id=%s= in pick_key", Param.szInputFiletypeId, szFiletypeId);
	  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
		//******************************************************************
		//读取其它去重关键字
		//******************************************************************
		dsTemp.Open("select a.colname,pick_time_flag,pick_key_type,pick_key_format,col_len from pick_key a,txtfile_fmt b where pick_grp_id = :pick_config_id \
	     and pick_time_flag != 'Y' and a.filetype_id = b.filetype_id and a.colname = b.colname order by pick_key_no", SELECT_QUERY);

		dsTemp<<pSourceEnv->m_szPickConfigId;
		long i = 1;
		while(dsTemp>>pSourceEnv->m_PickKey[i].m_szColName>>pSourceEnv->m_PickKey[i].m_cPickTimeFlag>>pSourceEnv->m_PickKey[i].m_cPickKeyType>>pSourceEnv->m_PickKey[i].m_szPickKeyFormat>>pSourceEnv->m_PickKey[i].m_iFieldLen)
		{
			if ('S' == pSourceEnv->m_PickKey[i].m_cPickKeyType)
			{
				if (!pSourceEnv->m_PickKey[i].m_StrExp.init(pSourceEnv->m_PickKey[i].m_szPickKeyFormat))
				{
			 		char szLogStr[500];
  					sprintf(szLogStr, "PICK_KEY_FORMAT(%s) IN PICK_KEY ERROR!",pSourceEnv->m_PickKey[i].m_szPickKeyFormat);
			  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
				}
			}
			else if ('I' == pSourceEnv->m_PickKey[i].m_cPickKeyType)
			{
				//表示用公式编辑器得到字符串
				char conText[200];
				sprintf(conText, "\t\t%s", pSourceEnv->m_PickKey[i].m_szPickKeyFormat);
				sprintf(pSourceEnv->m_PickKey[i].m_szPickKeyFormat, "%s", conText);
	
				char result[500] ;
				memset(result, 0, sizeof(result));
				int errorno = 0;
				//预编译
				Param.theCompile.Operation( result,sizeof(result)-1, &errorno, pSourceEnv->m_PickKey[i].m_szPickKeyFormat);
				if ( errorno )
				{
					char errorMsg[ERROR_MSG_LEN+1];
					sprintf(errorMsg,"excute a compiler =%s=%d err",pSourceEnv->m_PickKey[i].m_szPickKeyFormat, errorno);
					throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,0,errno,errorMsg,__FILE__,__LINE__);
				}
			}
			else
			{
				pSourceEnv->m_PickKey[i].m_cPickKeyType = 'F';
			}
			i++;
		}
		if (i == 1)
		{
			char szLogStr[500];
			sprintf(szLogStr, "No matching colname,pick_key_type,pick_key_format in pick_key where pick_grp_id = %s and pick_time_flag = 'N'",pSourceEnv->m_szPickConfigId);
	  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
		if(2==pSourceEnv->m_iCompressFlag && i != 2)
		{
			char szLogStr[500];
			sprintf(szLogStr, "can't have more than two colname  in pick_key where pick_grp_id = '%s' and COMPRESS_FLAG =T !!",pSourceEnv->m_szPickConfigId);
	  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
		dsTemp.Close();
		//******************************************************************
		//读取去重索引(excluding date time field)的长度,不包括时间字段
		//******************************************************************
		long col_len;
		for (long i = 1, col_len = 0; pSourceEnv->m_PickKey[i].m_szColName[0] != 0; i++)
		{
			dsTemp.Open("select col_len from txtfile_fmt where filetype_id = :input_filetype_id and colname =:pick_key_", SELECT_QUERY);
			dsTemp<<Param.szInputFiletypeId<<pSourceEnv->m_PickKey[i].m_szColName;
			if (!(dsTemp>>col_len))
			{
				char szLogStr[500];
  	  			sprintf(szLogStr, "No matching col_len in txtfile_fmt where filetype_id = %s and colname = %s",Param.szInputFiletypeId,pSourceEnv->m_PickKey[i].m_szColName);    
		  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
			}
			dsTemp.Close();
			pSourceEnv->m_lPickLen += col_len;
		}

		Param.mapSourceKey.insert(map<string, SSourceEnv*>::value_type(string(szSourceId), pSourceEnv));
	}

	map<string, SSourceEnv*>::iterator ite;
	ite = Param.mapSourceKey.begin();
	char szFirstSourceId[20];
	if(ite != Param.mapSourceKey.end())
	{
		sprintf(szFirstSourceId, "%s", (ite->first).c_str());
		SSourceEnv *p = ite->second;
		//edit by linyb 20080108
		if (p->m_iCompressFlag==1 || p->m_iCompressFlag==2)
		//end of edit by linyb 20080108
  		{
  			Param.lPickKeyLen = Param.HashCompresser.getHashLen(p->m_lPickLen);
  		}
		else
		{
			Param.lPickKeyLen = p->m_lPickLen;
		}
	}
	else
	{
		char szLogStr[500];
		sprintf(szLogStr, "select source_id from source where pipe_id = '%s' is null", Param.szPipeId);
  		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	
	ite++;
	while(ite != Param.mapSourceKey.end())
	{
		SSourceEnv *p = ite->second;
		long lLen = 0;
		//edit by linyb 20080108
		if (p->m_iCompressFlag==1 || p->m_iCompressFlag==2)
		//end of edit by linyb 20080108
  		{
  			lLen = Param.HashCompresser.getHashLen(p->m_lPickLen);
  		}
		else
		{
			lLen = p->m_lPickLen;
		}
		if(Param.lPickKeyLen != lLen)
		{
			char szLogStr[500];
			sprintf(szLogStr, "the picklen(%d) of source=%s=  is not equal to the picklen(%d) of source=%s= !", Param.lPickKeyLen, szFirstSourceId, lLen, (ite->first).c_str());
  			throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
		ite++;
	}
	//end of add by linyb 20071109

	//delete by linyb 20071109
	/*
	//*****************read in the date time field**********************
	ds.Open("select a.colname,pick_time_flag,pick_key_type,pick_key_format, a.filetype_id,col_len from pick_key a,txtfile_fmt b where pick_grp_id = :pick_config_id \
	     and pick_time_flag = 'Y' and a.filetype_id = b.filetype_id and a.colname = b.colname ", SELECT_QUERY);
	//读取其他去重关键字
	char szFiletypeId[20];
	ds<<Param.szPickConfigId;    
	if (!(ds>>Param.PickKey[0].m_szColName>>Param.PickKey[0].m_cPickTimeFlag>>Param.PickKey[0].m_cPickKeyType>>Param.PickKey[0].m_szPickKeyFormat>>szFiletypeId>>Param.PickKey[0].m_iFieldLen))
	{
	 	char szLogStr[500];
	  sprintf(szLogStr, "No matching colname in pick_key where pick_grp_id = %s and pick_time_flag = 'Y'",Param.szPickConfigId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();
	if ('S' == Param.PickKey[0].m_cPickKeyType)
	{
		if (!Param.PickKey[0].m_StrExp.init(Param.PickKey[0].m_szPickKeyFormat))
		{
		 	char szLogStr[500];
		  sprintf(szLogStr, "PICK_KEY_FORMAT(%s) IN PICK_KEY ERROR!",Param.PickKey[0].m_szPickKeyFormat);
		  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
	}
	else
	{
		Param.PickKey[0].m_cPickKeyType = 'F';
	}
	if (strcmp(szFiletypeId, Param.szInputFiletypeId) != 0)
	{
	 	char szLogStr[500];
		sprintf(szLogStr, "the InputFiletypeId not match the filetype_id in pick_key");
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Open("select a.colname,pick_time_flag,pick_key_type,pick_key_format,col_len from pick_key a,txtfile_fmt b where pick_grp_id = :pick_config_id \
	     and pick_time_flag != 'Y' and a.filetype_id = b.filetype_id and a.colname = b.colname order by pick_key_no", SELECT_QUERY);
	//读取其他去重关键字
	ds<<Param.szPickConfigId;
	long i = 1;
	while(ds>>Param.PickKey[i].m_szColName>>Param.PickKey[i].m_cPickTimeFlag>>Param.PickKey[i].m_cPickKeyType>>Param.PickKey[i].m_szPickKeyFormat>>Param.PickKey[i].m_iFieldLen)
	{
		if ('S' == Param.PickKey[i].m_cPickKeyType)
		{
			if (!Param.PickKey[i].m_StrExp.init(Param.PickKey[i].m_szPickKeyFormat))
			{
			 	char szLogStr[500];
  			sprintf(szLogStr, "PICK_KEY_FORMAT(%s) IN PICK_KEY ERROR!",Param.PickKey[i].m_szPickKeyFormat);
			  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
			}
		}
		else
		{
			Param.PickKey[i].m_cPickKeyType = 'F';
		}
	
	
		i++;
	}
	if (i == 1)
	{
		char temp[500];
		sprintf(temp, "No matching colname,pick_key_type,pick_key_format in pick_key where pick_grp_id = %s and pick_time_flag = 'N'",Param.szPickConfigId);
	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
	}
	ds.Close();
	
	//读取去重索引(excluding date time field)的长度,不包括时间字段
	long col_len;
	for (long i = 1, col_len = 0; Param.PickKey[i].m_szColName[0] != 0; i++)
	{
		ds.Open("select col_len from txtfile_fmt where filetype_id = :input_filetype_id and colname =:pick_key_", SELECT_QUERY);
		ds<<Param.szInputFiletypeId<<Param.PickKey[i].m_szColName;
		if (!(ds>>col_len))
		{
			char temp[500];
  	  sprintf(temp, "No matching col_len in txtfile_fmt where filetype_id = %s and colname = %s",Param.szInputFiletypeId,Param.PickKey[i].m_szColName);    
		  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG,FILTERTM_NO_MATCH_RECORD_IN_DB, errno,  szLogStr, __FILE__, __LINE__);
		}
		ds.Close();
		Param.lPickKeyLen += col_len;
	}
	//end of delete by linyb 20071109
	*/
	return 0;
}

//add by linyb 20080108
/****************************************************************
*	Function Name	: TransHToD
*	Description	:H码制两个字节字符转化为10进制一个字节
*	Input param	: 
*		instr     入口字符串,要求双数字节数
*       outstr    出口字符串,要求字符串容量大于1/2入口字符串长度
*       outstrlen  出口字符串容量
*	Returns		: 
*		0:成功;
*		-1:入口字符串长度为单数
*		-2:出口字符串容量小于或等于1/2入口字符串长度
*		-3:入口字符串包含非H码字符
*	complete	:2005/10/18
*****************************************************************/
int TransHToD(char *instr,char *outstr,int outstrlen)
{
  int instrlen = strlen(instr);
  if((instrlen%2) !=0) return -1;
  if(outstrlen<=instrlen/2) return -2;
  
  for(int i =0;i<instrlen/2;i++)
  {
    int ascii_char=0;
    if((*(instr+i*2))<58&&(*(instr+i*2))>47) ascii_char = (*(instr+i*2) - 48) *16;
     else if((*(instr+i*2))<71&&(*(instr+i*2))>64) ascii_char = (*(instr+i*2) - 55) *16;
       else if((*(instr+i*2))<103&&(*(instr+i*2))>96) ascii_char = (*(instr+i*2) - 87) *16;
         else return -3;
    if((*(instr+i*2+1))<58&&(*(instr+i*2+1))>47) ascii_char += (*(instr+i*2+1) - 48) ;
     else if((*(instr+i*2+1))<71&&(*(instr+i*2+1))>64) ascii_char += (*(instr+i*2+1) - 55) ;
       else if((*(instr+i*2+1))<103&&(*(instr+i*2+1))>96) ascii_char += (*(instr+i*2+1) - 87) ;
         else return -3;
       
    outstr[i] = ascii_char; 
  }

  outstr[instrlen/2] = '\0';
  return 0;
}
//end of add by linyb 20080108
