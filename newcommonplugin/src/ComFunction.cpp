/****************************************************************
 filename: ComFunction.cpp
 module:
 created by:
 create date:
 version: 3.0.0
 description:

 update:

 *****************************************************************/
#include "ComFunction.h"
using namespace std;
using namespace tpss;

char ErrorMsg[MSG_LEN];
//************************************************************
C_JudgeGsm::C_JudgeGsm()
{
	m_pGsmTab = NULL;
	m_iGsmHeaderLen = 0;
	m_iItemCount = 0;
}

C_JudgeGsm::C_JudgeGsm(const  C_JudgeGsm&  T)
{
	m_iGsmHeaderLen=T.m_iGsmHeaderLen;
	m_iItemCount=T.m_iItemCount;
	if (T.m_pGsmTab==NULL)
		m_pGsmTab=NULL;
	else
	{
		m_pGsmTab=new S_HcodeHeader();
		strcpy(m_pGsmTab->m_szHcodeHeader,T.m_pGsmTab->m_szHcodeHeader);
	}
}

int C_JudgeGsm::Init(const char* pchGsmTableName)
{
	//得到符合条件的行数
	char SqlStr[1024];
	//CBindSQL ds( DBConn );
	char szTmp[RECORD_LENGTH];
	
	DBConnection conn;
	if(!dbConnect(conn)){
		sprintf(ErrorMsg,"Connect DB err");
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);
		return -1;
	}
	Statement stmt = conn.createStatement();
	
	sprintf(SqlStr,"SELECT VARVALUE FROM C_GLOBAL_ENV WHERE VARNAME = 'GSM_HEADER_LEN'");
	//ds.Open(SqlStr,SELECT_QUERY);
	stmt.setSQLString(SqlStr);	
	if (!stmt.execute())
	//if(!(ds>>szTmp))
	{
		//告警退出
		conn.close();
		sprintf(ErrorMsg,"执行 %s 错误！,GSM_HEADER_LEN no define", SqlStr);
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);
		return -1;
	}
	stmt>>szTmp;
	m_iGsmHeaderLen = atoi(szTmp);

	sprintf(SqlStr,"select count(distinct(substr(hcode_header,1,%d))) from %s where length(hcode_header) >=%d", m_iGsmHeaderLen ,pchGsmTableName, m_iGsmHeaderLen);
	//ds.Open(SqlStr,SELECT_QUERY);
	//if(!(ds>>m_iItemCount))
	stmt.setSQLString(SqlStr);	
	try{
		stmt.execute();
	}catch(SQLException e )
	{
		//告警退出
		conn.close();
		sprintf(ErrorMsg,"执行 %s 错误！", SqlStr);
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);
		return -1;
	}
  stmt>>m_iItemCount;
  
	if(m_pGsmTab != NULL)
	{
		delete []m_pGsmTab;
		m_pGsmTab = NULL;
	}
	if(m_iItemCount==0){
		//告警退出
		conn.close();
		sprintf(ErrorMsg,"执行 %s 错误,count = 0",SqlStr);
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);
		return -1;
	}
	
	m_pGsmTab = new S_HcodeHeader[m_iItemCount];
	if( m_pGsmTab==NULL)
	{
		//告警退出
		conn.close();
		sprintf(ErrorMsg,"给m_pGsmTab动态分配内存失败！");
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);
		return -1;
	}

	sprintf(SqlStr,"select distinct(substr(hcode_header,1,%d)) from %s where length(hcode_header) >=%d order by substr(hcode_header,1,%d)",m_iGsmHeaderLen, pchGsmTableName, m_iGsmHeaderLen, m_iGsmHeaderLen);
	//ds.Open(SqlStr,SELECT_QUERY);
	stmt.setSQLString(SqlStr);	
	stmt.execute();
	int iTmpCount=0;
	//while(ds>>m_pGsmTab[iTmpCount].m_szHcodeHeader)
	while(stmt>>m_pGsmTab[iTmpCount].m_szHcodeHeader)
	{
		iTmpCount++;
		if(iTmpCount>m_iItemCount)
		{
			//告警退出
			conn.close();
			sprintf(ErrorMsg,"执行 %s 错误！", SqlStr);
			throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);
			return -1;
		}
	}
	//ds.Close();
	conn.close();
	return 0;
}

bool C_JudgeGsm::isGsm(const char* pchCallNo)
{
	char szTmp[RECORD_LENGTH];
	if(pchCallNo[0]== '0')
		memcpy(szTmp,pchCallNo+1,m_iGsmHeaderLen);
	else
		memcpy(szTmp,pchCallNo,m_iGsmHeaderLen);
	szTmp[m_iGsmHeaderLen]='\0';
	//cout<<"检查是否是手机号的号码头是："<<szTmp<<endl;
	int iHigh, iLow, iMid, iRet;
	iLow=0;
	iHigh=m_iItemCount-1;
	while(iLow <= iHigh)
	{
		iMid=(iLow+iHigh);
		iRet=strcmp(szTmp, m_pGsmTab[iMid].m_szHcodeHeader);
		if(iRet < 0)
			iHigh=iMid-1;
		else if(iRet > 0)
			iLow=iMid+1;
		else
		{
			//cout<<"是手机号"<<endl;
			return true;
		}
	}
	return false;
}


C_JudgeGsm::~C_JudgeGsm()
{
	if(m_pGsmTab != NULL)
		delete []m_pGsmTab;
}
//************************************************************

int getVersion( const char* tablename, const char* source_group_id, const char* server_id, const char* servcat_id )
{
	char szSql[2000];
	char szErrorMsg[2000];
	char version_id[32];
	//CBindSQL ds(DBConn);
	DBConnection conn;
	dbConnect(conn);
	Statement stmt = conn.createStatement();
	
	if ( !strcmp(servcat_id,"NO_SERVCAT") || strlen(servcat_id)==0 )
	{
		sprintf(szSql, "select version_id from c_commem_control where mem_name='%s' and server_id='%s'", tablename, server_id);
		try
		{
			//ds.Open( szSql, SELECT_QUERY );
			//if ( ds>>version_id )
			stmt.setSQLString(szSql);
			if (stmt.execute())
			{
				//ds.Close();
				conn.close();
				stmt>>version_id;
				return atoi(version_id);
			}
		}
		catch( SQLException e  )
		{
			//ds.Close();
			conn.close();
			sprintf( szErrorMsg,"执行 %s 错误！", szSql );
			throw jsexcp::CException(0, szErrorMsg, __FILE__, __LINE__); 
		}
	}
	else
	{
		sprintf(szSql, "select mem_define_id from c_commem_mem_version where server_id='%s and mem_name=(select real_table_name \
			from c_orgtable2realtable where org_table_name='%s' and serv_cat_id='%s')'", server_id, tablename, servcat_id);
		try
		{
			//ds.Open( szSql, SELECT_QUERY );
			//if ( ds>>version_id )
			stmt.setSQLString(szSql);
			if (stmt.execute())
			{
				//ds.Close();
				conn.close();
				stmt>>version_id;
				return atoi(version_id);
			}
		}
		catch( SQLException e  )
		{
			//ds.Close();
			conn.close();
			sprintf( szErrorMsg,"执行 %s 错误！", szSql );
			throw jsexcp::CException(0, szErrorMsg, __FILE__, __LINE__);
		}
		conn.close();
		return false;
	}
}


void DeleteSpace(char *ss)
{
	int  i,j,k,len;
	char temp[1024];
	len = strlen(ss);
	memset(temp,0,1024);
	for(j=0;j<len;j++) 
		if(ss[j]!=' ') 
	  	break;
	 
	strncpy(temp,ss+j,len-j);
	i = strlen(temp)-1;
	while ( i && temp[i] == ' ' ) 
		i--;
	temp[i+1] = 0;
	memset(ss,0,len);
	i = strlen(temp);
	strncpy(ss,temp,i);
}

int getFromGlobalEnv( char *Value, char *Name )
{
	//CBindSQL ds( DBConn );
	DBConnection conn;
	dbConnect(conn);
	Statement stmt = conn.createStatement();
	
	string sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME=:a";
	stmt.setSQLString(sql);
	stmt<<Name;
	if (!stmt.execute())
				
	//ds.Open("select VARVALUE from C_GLOBAL_ENV where VARNAME=:a", SELECT_QUERY );
 	//ds<<Name;
 	//if( !(ds>>Value) ) 
 	{  		
 		conn.close();
		sprintf(ErrorMsg,"在环境变量表 C_GLOBAL_ENV 中找不到 VARNAME=%s 的配置！", Name );
		printf("%s\n",ErrorMsg);
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);		
 	}
 	stmt>>Value;
	//ds.Close();
	conn.close();
	DeleteSpace(Value);	
	return 0;
}

int getFromGroupEnv( char *Value, char *Name, char *SourceGroupId, char *Service )
{
	char ErrorMsg[MSG_LEN];
	//CBindSQL ds( DBConn );
	DBConnection conn;
	dbConnect(conn);
	Statement stmt = conn.createStatement();
	
	string sql = "select VAR_VALUE from C_PROCESS_ENV where VARNAME=:a and source_group=:b and service=:c";
	stmt.setSQLString(sql);
	stmt<<Name<<SourceGroupId<<Service;
	
	//ds.Open("select VAR_VALUE from C_PROCESS_ENV where VARNAME=:a and source_group=:b and service=:c", SELECT_QUERY );
 	//ds<<Name<<SourceGroupId<<Service;
 	//if( !(ds>>Value) ) 
 	if (!stmt.execute())
 	{  		
 		conn.close();
		sprintf(ErrorMsg,"在环境变量表 C_PROCESS_ENV 中找不到 VARNAME=%s , source_group=%s , service=%s 的配置项！", Name, SourceGroupId, Service );
		printf("%s\n",ErrorMsg);
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);		
 	}
 	stmt>>Value;
	//ds.Close();
	conn.close();
	DeleteSpace(Value);	
	return 0;
}

int getFromSrcEnv( char *Value, char *Name, char *SourceId, char *Service )
{
	//CBindSQL ds( DBConn );
	//ds.Open("select VAR_VALUE from C_SOURCE_ENV where VARNAME=:a and source_id=:b  and service=:c", SELECT_QUERY );
 //	ds<<Name<<SourceId<<Service;
	DBConnection conn;
	dbConnect(conn);
	Statement stmt = conn.createStatement();
	
	string sql = "select VAR_VALUE from C_SOURCE_ENV where VARNAME=:a and source_id=:b  and service=:c";
	stmt.setSQLString(sql);
	stmt<<Name<<SourceId<<Service;
	if (!stmt.execute())
	//if( !(ds>>Value) )
	{
		conn.close();
		sprintf(ErrorMsg,"在环境变量表 C_SOURCE_ENV 中找不到 VARNAME=%s and source_id=%s and service=%d 的配置项！", Name, SourceId, Service );
		printf("%s\n",ErrorMsg);
		throw jsexcp::CException(SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);		
	}
	stmt>>Value;
	//ds.Close();
	conn.close();
	DeleteSpace(Value);	
	return 0;
}
