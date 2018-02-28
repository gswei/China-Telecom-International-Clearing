#include "Classify.h"
using namespace std;
using namespace tpss;

CClassify::CClassify()
{
	m_NewFile = false;
	m_Initflag = 0;
}

CClassify::~CClassify()
{
	rule.clear();
}

bool CClassify::initFile(char* ServiceId, char* SourceId, char* SourcePath, char* FileName, char* FileType, char* StartTime)
{
	strcpy(m_ServiceId, ServiceId);
	strcpy(m_SourceId, SourceId);
	strcpy(m_SourcePath, SourcePath);
	strcpy(m_FileName, FileName);
	strcpy(m_FileType, FileType);
	strcpy(m_Time, StartTime);
	m_NewFile = true;
	for(CLASSIFY_RULE::iterator iter = rule.begin(); iter != rule.end(); iter++)
	{
		iter->second.newFile(m_ServiceId, m_SourceId, m_SourcePath, m_FileName, m_Time);
	}
}

/* 根据输入的分拣规则ID和话单进行分拣
返回值：1	互斥分拣
		3	备份分拣
*/
int CClassify::dealRecord(char* RuleId, CFmt_Change& inrcd)
{
	int ruleId = atoi(RuleId);
	CLASSIFY_RULE::iterator it = rule.find(ruleId);
	if(it == rule.end())
	{
		CClassifyRule newRule;
		newRule.setRule(ruleId);
		rule.insert(CLASSIFY_RULE::value_type(ruleId, newRule));
		it = rule.find(ruleId);
	}
	if(it->second.getInitFlag() == 0)
	{
		it->second.initFile(m_ServiceId, m_SourceId, m_SourcePath, m_FileName, m_FileType, m_Time);
		m_NewFile = false;
	}
	int Ret;
	Ret = it->second.dealRecord(inrcd);
	return Ret;
}

void CClassify::endFile()
{
	for(CLASSIFY_RULE::iterator it=rule.begin(); it != rule.end(); it++)
	{
		it->second.endFile();
	}
}

bool CClassify::commit()
{
	for(CLASSIFY_RULE::iterator it=rule.begin(); it != rule.end(); it++)
	{
		it->second.commit();
	}
}

bool CClassify::rollback()
{
	for(CLASSIFY_RULE::iterator it=rule.begin(); it != rule.end(); it++)
	{
		it->second.rollback();
	}
}

CClassifyRule::CClassifyRule()
{
	m_Pick2TabConfigId = 0;
	m_PickRecordCount = 0;
	m_Pick2Table = NULL;
	m_OutInfo.clear();
	memset(tmp_DealMonth, 0, sizeof(tmp_DealMonth));
	memset(m_FileName, 0, sizeof(m_FileName));
	memset(m_PickTableName, 0, sizeof(m_PickTableName));
	iInitFlag = 0;
}

CClassifyRule::~CClassifyRule()
{
	if(m_Pick2Table != NULL)
	{
		delete m_Pick2Table;
		m_Pick2Table = NULL;
	}
	m_OutInfo.clear();
}

int CClassifyRule::getInitFlag()
{
	return iInitFlag;
}

int CClassifyRule::setRule(int RuleId)
{
	char szSqlStr[1024];
	char errmsg[512];

	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "select pickflag, pick_target, filename_prefix, filename_postfix, pickpath, \
		output_type, stat_config, fmt_stat_config from c_classify_rule where classify_type_id=:v1";		
			stmt.setSQLString(sql);
			stmt << RuleId;
			stmt.execute();
			if(!(stmt >> m_PickFlag>>m_PickTarget>>m_FileNamePrefix>>m_FileNamePostfix
                	>>m_PickPath>>m_OutputType>>m_StatConfig>>m_LogFlag) )
			{
              sprintf(errmsg,"select record  from CLASSIFY_RULE WHERE CLASSIFYRULE_ID=%d error!!", RuleId);
		      throw jsexcp::CException(SELECT_ERROR_FROM_DB, errmsg, __FILE__, __LINE__);
			}
	  if(m_PickTarget[0] == 'T')
	  {
		/* 查入库的表名 */
		sql = "select table_name from C_STAT_TABLE_DEFINE where config_id=:v1";
		stmt<<m_PickPath;
		stmt.execute();
		if(!(stmt>>m_PickTableName))
		{
			sprintf(errmsg,"select table_name from C_STAT_TABLE_DEFINE WHERE config_id=%d error!!", m_PickPath);
			throw jsexcp::CException(SELECT_ERROR_FROM_DB, errmsg, __FILE__, __LINE__);
		}
	  }
	     sprintf(m_PickType, "%d", RuleId);
			
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	   conn.commit();
	   conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
    } 
}

bool CClassifyRule::initFile(char* ServiceId, char* SourceId, char* SourcePath, char* FileName, char* FileType, char* StartTime)
{
	memset(m_ServiceId, 0, sizeof(m_ServiceId));
	memset(m_SourceId, 0, sizeof(m_SourceId));
	memset(m_SourcePath, 0, sizeof(m_SourcePath));
	memset(m_FileName, 0, sizeof(m_FileName));
	memset(m_InFileType, 0, sizeof(m_InFileType));
	memset(m_Time, 0, sizeof(m_Time));
	memset(tmp_DealMonth, 0, sizeof(tmp_DealMonth));
	strcpy(m_ServiceId, ServiceId);
	strcpy(m_SourceId, SourceId);
	strcpy(m_SourcePath, SourcePath);
	strcpy(m_FileName, FileName);
	strcpy(m_InFileType, FileType);
	strcpy(m_Time, StartTime);
	strncpy(tmp_DealMonth, StartTime, 6);
	tmp_DealMonth[6]='\0';
	
	char errmsg[512];
	//char szSqlStr[1024];
	int iPickRecordCount = 0;

    try{			
	if (dbConnect(conn))
	 {
		Statement stmt = conn.createStatement();
		string sql;

	   if(m_PickTarget[0] == 'T')
	   {
		/*入库分拣，调用入库程序的接口*/		
		if(m_Pick2Table == NULL)
		{
			m_Pick2Table = new CF_CErrorLackRec();
			if(m_Pick2Table == NULL)
			{
				sprintf( errmsg, "not enough memory to apply CF_CErrorLackRec");
				throw jsexcp::CException(NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
			}	
		}
		abnormity_type type=abenum_lackinfo;
		m_Pick2TabConfigId = atoi(m_PickPath);
		if(m_StatConfig[0]=='N')
		{
			m_Pick2Table->Init( type, m_Pick2TabConfigId, NULL, m_ServiceId);
		}
		else
		{
			m_Pick2Table->Init( type, m_Pick2TabConfigId, m_StatConfig, m_ServiceId);
		}
	}
	else if(m_PickTarget[0] == 'F')
	{
		sql = "select record_type from c_txtfile_fmt_define where file_fmt = ':v1'";
		stmt.setSQLString(sql);
		stmt << m_InFileType;
		stmt.execute();
		if(!(stmt>>m_InRcdType))
		{
			sprintf(errmsg, "找不到文件类型%s", m_InFileType);
			throw jsexcp::CException(0, errmsg, __FILE__, __LINE__);
		}

		m_InFileFmt.Init(m_InFileType, m_InRcdType);
		if(!strcmp(m_OutputType,"I")) 
		{
			strcpy(m_OutFileType, m_InFileType);
			m_OutRcdType = m_InRcdType;
		}
		else 
		{
			strcpy(m_OutFileType, m_OutputType);
			sql = "select record_type from c_filetype_define where filetype_id=':v1'";
			stmt.setSQLString(sql);
		    stmt << m_OutFileType;
		    stmt.execute();
			if(!(stmt >> m_OutRcdType))
			{
				sprintf(errmsg, "找不到文件类型%s", m_OutFileType);
				throw jsexcp::CException(0, errmsg, __FILE__, __LINE__);
			}

			m_RecordChange.Init((char * )m_InFileType, m_OutFileType);
			m_OtherFileType.Init(m_OutFileType, m_OutRcdType);
			cout<<"chage:"<<m_InFileType<<"-"<<m_OutFileType<<endl;
			cout<<"out:"<<m_OutFileType<<":"<<m_OutRcdType<<endl;
		}
		//取输入格式的字段名、索引号、字段类型
		char ColName[50];
		int  ColIndex;
		char Col_Fmt[21];
		sql =  "select COLNAME, COL_INDEX, COL_FMT from C_TXTFILE_FMT where FILETYPE_ID = ':v1' order by col_index";
		stmt.setSQLString(sql);
		stmt << m_InFileType;
		stmt.execute();
		while ( stmt>>ColName>>ColIndex>>Col_Fmt )
		{
			//对动态编译器增加变量
			m_Compile.DefineVariable( ColName, this->m_InFileFmt.Get_Field(ColIndex) );
		}
		//添加自定义的变量
		m_Compile.DefineVariable("YYYYMM", tmp_DealMonth);
		m_Compile.DefineVariable("SOURCE_PATH", m_SourcePath);
	}
	else
	{
		sprintf(errmsg, "pick flag %s is illegal!!", m_PickFlag);
		throw jsexcp::CException(INVALID_PARAMETER, errmsg, __FILE__, __LINE__);
	}
	iInitFlag = 1;
			
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	    conn.commit();
	    conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
   } 
	return true;
}

void CClassifyRule::newFile(char* ServiceId, char* SourceId, char* SourcePath, char* FileName, char* StartTime)
{
	memset(m_ServiceId, 0, sizeof(m_ServiceId));
	memset(m_SourceId, 0, sizeof(m_SourceId));
	memset(m_SourcePath, 0, sizeof(m_SourcePath));
	memset(m_FileName, 0, sizeof(m_FileName));
	memset(m_Time, 0, sizeof(m_Time));
	memset(tmp_DealMonth, 0, sizeof(tmp_DealMonth));
	strcpy(m_ServiceId, ServiceId);
	strcpy(m_SourceId, SourceId);
	strcpy(m_SourcePath, SourcePath);
	strcpy(m_FileName, FileName);
	strcpy(m_Time, StartTime);
	strncpy(tmp_DealMonth, StartTime, 6);
	tmp_DealMonth[6]='\0';
}

void CClassifyRule::endFile()
{
	for(int idx=0; idx<m_OutInfo.size(); idx++)
	{
		if(m_OutInfo[idx].iEndFlag == 0)
		{
			fclose(m_OutInfo[idx].fp);
			m_OutInfo[idx].iEndFlag = 1;
		}
	}
}

int CClassifyRule::dealRecord(CFmt_Change& inrcd)
{
	char errmsg[512];
	char result[500];
	const char *theResult;
	int  errorno;
	//unsigned int crc;
	sPickInfo pick_info;

	m_PickRecordCount++;
	if(m_PickTarget[0] == 'T')
	{
			m_Pick2Table->saveErrLackRec(inrcd, m_SourceId, m_FileName, m_PickType, "CLASSIFY");
	}
	else
	{
		char szRealPickPath[PATH_NAME_LEN+FILE_NAME_LEN+1];
		char szRealPickFile[PATH_NAME_LEN+FILE_NAME_LEN+1];
		m_InFileFmt.Copy_Record( inrcd );
		//分析输出目录
		try
		{
			theResult = m_Compile.Operation( result, sizeof(result)-1, &errorno, m_PickPath);
		}
		catch(jsexcp::CException &e)
		{
			sprintf(errmsg, "分析表达式=%s=失败", m_PickPath);
			e.PushStack(0, errmsg, __FILE__, __LINE__);
			throw e;
		}
		//sprintf(szRealPickPath, "%s%s", m_SourcePath, result);
		strcpy(szRealPickPath, result);
		if( 0 != chkAllDir( szRealPickPath ) )
		{
			sprintf( errmsg, "create Path %s for Pick fail", szRealPickPath);
			throw jsexcp::CException(NOT_ENOUGH_MEMORY_TO_APPLY, errmsg, __FILE__, __LINE__);	
		}
		sprintf(szRealPickFile, "%s/%s%s%s", szRealPickPath, m_FileNamePrefix, m_FileName, m_FileNamePostfix);
		//crc = crc32( szRealPickFile, strlen(szRealPickFile) );
		VEC_PINFO::iterator iter = m_OutInfo.begin();
		for(; iter != m_OutInfo.end(); iter++)
		{
			//if(crc == iter->iPathId)
			if(strcmp(szRealPickFile, iter->szRealName) == 0)
			{
				//已有该路径的记录
				iter->iPickNum++;
				strcpy(pick_info.szRealName, iter->szRealName);
				strcpy(pick_info.szTmpName, iter->szTmpName);
				strcpy(pick_info.szOutPath, iter->szOutPath);
				pick_info.iPickNum = iter->iPickNum;
				//pick_info.iPathId = iter->iPathId;
				pick_info.fp = iter->fp;
				break;
			}
		}
		if( iter == m_OutInfo.end() )//不存在，增加
		{
			//pick_info.iPathId = crc;
			pick_info.iPickNum = 1;
			strcpy(pick_info.szOutPath, szRealPickPath);
			sprintf(pick_info.szRealName, "%s/%s%s%s", pick_info.szOutPath, m_FileNamePrefix, m_FileName, m_FileNamePostfix);
			sprintf(pick_info.szTmpName, "%s/%s%s%s.TMP", pick_info.szOutPath, m_FileNamePrefix, m_FileName, m_FileNamePostfix);
			if((pick_info.fp = fopen(pick_info.szTmpName, "w")) == NULL)
			{
				for(int it=0; it < m_OutInfo.size(); it++)
				{
					fclose(m_OutInfo[it].fp);
				}
				sprintf(errmsg, "打开文件(%s)失败", pick_info.szTmpName);
				throw jsexcp::CException(NOT_ENOUGH_MEMORY_TO_APPLY, errmsg, __FILE__, __LINE__);
			}
			m_OutInfo.push_back(pick_info);
		}
		/*cout<<"-----out file info:"<<endl;
		cout<<"real name:"<<pick_info.szRealName<<endl;
		cout<<"tmp name :"<<pick_info.szTmpName<<endl;
		cout<<"out path :"<<pick_info.szOutPath<<endl;
		cout<<"end flag :"<<pick_info.iEndFlag<<endl;
		cout<<"FILE     :"<<pick_info.fp<<endl;*/
		if(!strcmp(m_OutputType,"I"))
		{
cout<<m_InFileFmt.Get_record()<<endl;
			//fprintf(pick_info.fp, "%s\n", m_InFileFmt.Get_record());
fputs(m_InFileFmt.Get_record(),pick_info.fp);
fputs("\n",pick_info.fp);

		}
		else
		{
			m_RecordChange.Rec_Change(m_InFileFmt, m_OtherFileType);
cout<<m_OtherFileType.Get_record()<<endl;
			//fprintf(pick_info.fp, "%s\n", m_OtherFileType.Get_record());
fputs(m_OtherFileType.Get_record(),pick_info.fp);
fputs("\n",pick_info.fp);
		}
	}
	if(m_PickFlag[0] == 'B')
		return 3;
	else if(m_PickFlag[0] == 'O')
		return 1;
	else
	{
		sprintf(errmsg, "pick flag %s is illegal!!", m_PickFlag);
		throw jsexcp::CException(INVALID_PARAMETER, errmsg, __FILE__, __LINE__);
	}
	return 0;
}

void CClassifyRule::resetDealNum()
{
	m_PickRecordCount = 0;
}

int CClassifyRule::getDealNum()
{
	return m_PickRecordCount;
}

bool CClassifyRule::commit()
{
	theLog<<"classify("<<m_PickType<<") commit "<<endi;
	char errmsg[512];
	//CBindSQL ds(DBConn);
    try{			
    if (dbConnect(conn))
   {
	  Statement stmt = conn.createStatement();
	  string sql ;
	if(m_PickTarget[0] == 'T')
	{
		m_Pick2Table->Commit();
		//分拣记录插入D_FORMAT_STAT_LOG表
		if(m_LogFlag[0] == 'Y')
		{
		   sql = "insert into D_FORMAT_STAT_LOG(source_id, filename, pick_target, pick_path, classify_type_id, inserttime) \
				values(:v1,:v2,'T',:v3,:v4,:v5)";
		    stmt.setSQLString(sql);
			stmt<<m_SourceId<<m_FileName<<m_PickTableName<<m_PickType<<m_Time;
			stmt.execute();
			conn.commit();
		}
	}
	else
	{
		for(int idx=0; idx<m_OutInfo.size(); idx++)
		{
			if(m_OutInfo[idx].iPickNum > 0)
			{
				if( rename(m_OutInfo[idx].szTmpName, m_OutInfo[idx].szRealName) != 0 )
				{	
					sprintf( errmsg, "Rename File %s to %s Err!", m_OutInfo[idx].szTmpName, m_OutInfo[idx].szRealName);
					throw jsexcp::CException(NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
					return false;
				}
				theLog<<"classify("<<m_PickType<<") rename "<<m_OutInfo[idx].szTmpName<<" to "<<m_OutInfo[idx].szRealName<<endi;
				//分拣记录插入D_FORMAT_STAT_LOG表
				if(m_LogFlag[0] == 'Y')
				{
				    sql = "insert into D_FORMAT_STAT_LOG(source_id, filename, pick_target, pick_path, classify_type_id, inserttime) \
						values(:v1,:v2,'T',:v3,:v4,:v5)";
		            stmt.setSQLString(sql);
			        stmt<<m_SourceId<<m_FileName<<m_OutInfo[idx].szRealName<<m_PickType<<m_Time;
			        stmt.execute();
			        conn.commit();
				}
			}
			else
			{
				unlink(m_OutInfo[idx].szTmpName);
				theLog<<"classify("<<m_PickType<<") unlink "<<m_OutInfo[idx].szTmpName<<" : "<<m_OutInfo[idx].iPickNum<<endi;
			}
		 }
		m_OutInfo.clear();
	   }
	      resetDealNum();	
    	}else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
 } 	 
	return true;
}

bool CClassifyRule::rollback()
{
	if((m_PickTarget[0] == 'T') && (m_Pick2Table != NULL))
		m_Pick2Table->RollBack();
	for(int i=0; i<m_OutInfo.size(); i++)
	{
		if(m_OutInfo[i].iEndFlag == 0)
			fclose(m_OutInfo[i].fp);
		unlink(m_OutInfo[i].szTmpName);
	}
	m_OutInfo.clear();
	resetDealNum();
	return true;
}

void CClassifyRule::checkPath(char* szPath, char* szTime)
{
	char now_day[8+1];
	memset(now_day, 0, sizeof(now_day));
	string strTmpPath = szPath;
	string::size_type strPos(0);
	if( (strPos=strTmpPath.find("$YYYYMMDD"))!=string::npos )
	{
		strncpy(now_day, szTime, 8);
		replace_all(strTmpPath, "$YYYYMMDD", now_day);
	}
	else if( (strPos=strTmpPath.find("$YYYYMM"))!=string::npos )
	{
		strncpy(now_day, szTime, 6);
		replace_all(strTmpPath, "$YYYYMM", now_day);
	}
	else if( (strPos=strTmpPath.find("$YYYY"))!=string::npos )
	{
		strncpy(now_day, szTime, 4);
		replace_all(strTmpPath, "$YYYY", now_day);
	}
	strcpy(szPath, strTmpPath.c_str());
}

string& replace_all(string& str,const string& old_value,const string& new_value)   
{   
    while(true)
    {   
        string::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=string::npos   )   
            str.replace(pos,old_value.length(),new_value);   
        else   break;   
    }
    return   str;   
}

/*************************************************
*	函数名称:	crc32
*	功能描述:	把一个字符串加密成一个整数，不可逆,做为字符串的key值
*	参数列表:	需要加密的字符串, 长度
*	返回结果:	加密好的整数
*************************************************/
unsigned int crc32(char *string ,int len)
{
	unsigned short crc[2] ={0,0};
	int i ,j ,k;
	
	for (i =0 ;i <len ;i++) 
	{
		k = i%2;
		crc[k] ^=( (int)string[i] <<8); //低字节不变 ，高字节异或
		for (j =0 ;j <8 ;j++) 
		{
			if ( (crc[k] &0x8000) !=0)
			{
				crc[k] = (crc[k] <<1) ^ 0x1021;
			}
			else
			{
				crc[k] <<=1;
			}
		}
	}
	unsigned int crcint = crc[0];
	crcint = crcint << 16;
	crcint += crc[1];
	return crcint ;
}
