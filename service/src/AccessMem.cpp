/****************************************************************
 filename: AccessMem.cpp
 module:
 created by:
 create date:
 version: 3.0.0
 description:

 update:

 *****************************************************************/
#include "AccessMem.h"
using namespace std;
using namespace tpss;

inline bool operator < (const struct tableIndex &ls, const struct tableIndex &rs)
{
	int i = strcmp(ls.szName, rs.szName);
	if( i < 0 )
		return true;
	else if( i > 0 )
		return false;
	else 
		return (ls.iIndex < rs.iIndex);
}


//C_AccessMem g_accessMem;

C_AccessMem::C_AccessMem()
{
	m_iTableNum = 0;
	/*
	tableInfo.clear();
	tableMap.clear();
	for(int i =0; i<m_iTableNum; i++)
	{
		if( table[i] != NULL )
		{
			delete table[i];
		}
		table[i] = NULL;
	}
	*/
	tableSearchType.clear();
	tableInfo = NULL;
	m_pCommonMem = new S_MemManager();
	//timePart1 = 0;
	//timePart2 = 0;
	searchNum = 0;
}

C_AccessMem::~C_AccessMem()
{
	/*
	for(int i =0; i<m_iTableNum; i++)
	{
		if( table[i] != NULL )
		{
			table[i]->detach();
			delete table[i];
			table[i] = NULL;
		}
	}
	tableInfo.clear();
	for(map<string, CommonMemClient*>::iterator it = tableMap.begin(); it != tableMap.end(); it++)
	{
		//(*it->second).detach();
		delete (*it).second;
		(*it).second = NULL;
	}
	tableMap.clear();
	*/
	if( m_pCommonMem != NULL )
	{
		delete m_pCommonMem;
		m_pCommonMem = NULL;
	}
}

int C_AccessMem::init(char* serverId, char* iniPath)
{
	char ErrorMsg[LOG_MSG_LEN+1];

	strcpy(m_serverId, serverId);
	strcpy(m_iniPath, iniPath);
	
	//char memName[TABLENAME_LEN+1];
	//int versionId;
	//int columnNum;
	m_iTableNum = 0;
	int iNum = 0;
	cout << "serverId = " <<serverId<<endl;

	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "select count(a.rowid) from c_commem_table_def a, c_commem_control c where \
		a.mem_define_id=c.shmkey_id and c.server_id=:v1 and c.valid_flag = 'Y'";		
			stmt.setSQLString(sql);
			stmt << serverId;
			stmt.execute();
			if(!(stmt >> m_iTableNum))
	         {
		        cout<<"没找到num"<<endl;
		        return -1;
	         }
			if(tableInfo != NULL)
	       {
		       delete tableInfo;
		       tableInfo = NULL;
	        }
	        tableInfo = new tableStruct[m_iTableNum];
	         if(tableInfo == NULL)
	      {
		    sprintf( ErrorMsg, "给 tableStruct 动态分配共享内存失败");
		    throw jsexcp::CException(PREDEAL_ERR_NOT_ENOUGH_MEMORY, ErrorMsg, __FILE__, __LINE__);
	       }

	      sql =  "select trim(a.update_table_name), to_number(c.version_id), count(b.column_id) \
		from c_commem_table_def a, c_commem_column_def b, c_commem_control c where \
		a.mem_define_id=b.mem_define_id and a.mem_define_id=c.shmkey_id and c.server_id=:v1 \
		and c.valid_flag = 'Y' group by a.update_table_name, c.version_id ";
		stmt.setSQLString(sql);
		stmt << serverId;
		stmt.execute();
		for(iNum =0; iNum < m_iTableNum; iNum++)
	    {
		  stmt >>tableInfo[iNum].tableName>>tableInfo[iNum].tableVersion>>tableInfo[iNum].colNum;
		  theJSLog<<"连接共享内存 "<<serverId<<"==="<<tableInfo[iNum].tableName<<"==="<<tableInfo[iNum].tableVersion<<"==="<<iniPath<<endi;
		  tableInfo[iNum].table = new CommonMemClient(serverId, tableInfo[iNum].tableName, tableInfo[iNum].tableVersion, iniPath);	
	     }

		char type[11];
	   int searchType;
	   tableIndex tblIndex;
	  sql = "select trim(b.update_table_name), to_number(a.index_id), a.index_type from c_commem_index_def a, \
		c_commem_table_def b , c_commem_mem_version c where a.mem_define_id = b.mem_define_id and \
		b.mem_define_id=c.mem_define_id and c.server_id=:v1";
	  stmt.setSQLString(sql);
	  cout << "serverId = " <<serverId<<endl;
	  stmt << serverId;
	  stmt.execute();	 
	while(stmt >>tblIndex.szName>>tblIndex.iIndex>>type)
	{
		if(!strcmp(type, "LONGIST"))
			searchType = 2;
		else
			searchType = 1;
		//cout <<"index mem查询内容 = "<<tblIndex.szName<<tblIndex.iIndex<<type<<endl;
		tableSearchType.insert(pair<tableIndex, int>(tblIndex, searchType));
	}
	   conn.commit();
	   conn.close();
	    m_oRWLock.InitLock(m_pCommonMem->memlock, NULL);  			
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
    }	
	return 0;
}

int C_AccessMem::LoadOrUpdate()
{
	//for(map<string, CommonMemClient*>::iterator it=tableMap.begin(); it != tableMap.end(); it++)
	for(int iNum=0; iNum<m_iTableNum; iNum++)
	{
		//theJSLog<<"更新表"<<it->first<<"的资料……"<<endi;
		//theJSLog << "更新表" << endd;
		/* 获得读锁 */
		m_oRWLock.GetReadLock(m_pCommonMem->memlock);
		//(it->second)->reBind();
		tableInfo[iNum].table->reBind();
		/* 释放读锁 */
		m_oRWLock.UnLock(m_pCommonMem->memlock);
	}
	/*
	table = 0;
	for(map<string,int>::iterator it=tableVersion.begin(); it != tableVersion.end(); it++)
	{
		table = new CommonMemClient(m_serverId, it->first.c_str(), it->second, m_iniPath);
		table->reBind();
		delete table;
		table = 0;
	}
	*/
	return 0;
}

int C_AccessMem::getTableOffset(char* tableName)
{
	for(int iNum=0; iNum<m_iTableNum; iNum++)
	{
		if(!(strcmp(tableInfo[iNum].tableName, tableName)))
		{
			return iNum;
		}
	}

	return -1;
}

/* 查找指定表名的index是否存在
0：找到 -1：没找到 */
int C_AccessMem::checkSearchIndex(char* tableName, int iSearchIndex)
{
	for(int iNum=0; iNum<m_iTableNum; iNum++)
	{
		if(!(strcmp(tableInfo[iNum].tableName, tableName) && tableInfo[iNum].tableVersion==iSearchIndex))
		{
			return 0;
		}
	}

	return -1;
}

/* 在共享内存中查找资料
0：找到资料	-1：没找到资料 */
int C_AccessMem::getData(int tableOffset, const DataStruct* in_DataStruct, DataStruct* out_DataStruct, int iSearchIndex)
//int C_AccessMem::getData(char* tableName, const DataStruct* in_DataStruct, DataStruct* out_DataStruct, int iSearchIndex)
{
	//timerclear(&start);
	//timerclear(&finish);
	//gettimeofday(&start, NULL);

	char ErrorMsg[LOG_MSG_LEN+1];

	char temp[RECORD_LENGTH+1];
	int outNum = 0;
	
	/*
	map<string, CommonMemClient*>::iterator it = tableMap.find(tableName);
	if(it == tableMap.end())
	{
		sprintf( ErrorMsg, "没找到表%s的记录", tableName);
		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}

	map<string, int>::iterator it_Num = tableInfo.find(tableName);
	if(it_Num == tableInfo.end())
	{
		sprintf( ErrorMsg, "没找到表%s的记录", tableName);
		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}
	else
	{
		out_DataStruct->itemNum = it_Num->second;
	}
	*/

	if(tableInfo[tableOffset].colNum > MAX_COLUMN)
	{
//		cout<<"表属性个数超长"<<endl;
		//gettimeofday(&finish, NULL);
		//timePart1 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
		searchNum++;
		return -1;
	}
	else
		out_DataStruct->itemNum = tableInfo[tableOffset].colNum;

	string strIn = "";
	for(int i=0; i<in_DataStruct->itemNum; i++)
	{
		if(i!=0)
			strIn += ",";
		strIn += string(in_DataStruct->values[i]);
	}
	char szTime[14+1];
	strcpy(szTime, in_DataStruct->startTime);

	char szStartTime[15];
	char szEndTime[15];
	int iTimeLen=0, iStartCol=0, iEndCol=0;
	int i, j;
	bool bLack = true;

	//gettimeofday(&finish, NULL);
	//timePart1 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
	//timerclear(&start);
	//timerclear(&finish);
	//gettimeofday(&start, NULL);
	
	//cout<<"查表"<<tableInfo[tableOffset].tableName<<"=="<<strIn<<"=="<<szTime<<endl;

	vector<mysize_t> tableResult;
	tableResult.clear();
	tableInfo[tableOffset].table->queryTableRecord(strIn.c_str(), iSearchIndex, tableResult);
	int iResultNum = tableResult.size();
//	if(iResultNum < 1)
//		theJSLog<<"共享内存无资料"<<endd;
	for(int i=0; i<iResultNum; i++ )
	{
		/* 效验时间是否匹配 */
		if(strlen(szTime) > 0)
		{
			iStartCol = tableInfo[tableOffset].table->getStartCol();
//			cout<<"开始时间字段："<<iStartCol<<endl;
			if(iStartCol >0)
				tableInfo[tableOffset].table->getString(iStartCol, szStartTime, sizeof(szStartTime), tableResult[i]);
			iEndCol = tableInfo[tableOffset].table->getEndCol();
//			cout<<"结束时间字段："<<iEndCol<<endl;
			if(iEndCol >0)
				tableInfo[tableOffset].table->getString(iEndCol, szEndTime, sizeof(szEndTime), tableResult[i]);
			if(iStartCol>0 && iEndCol>0)
			{
//				cout<<"开始匹配时间"<<szStartTime<<"\t"<<szTime<<"\t"<<szEndTime<<endl;
				iTimeLen = strlen(szStartTime)<strlen(szTime)? strlen(szStartTime):strlen(szTime);
				iTimeLen = iTimeLen<strlen(szEndTime)? iTimeLen:strlen(szEndTime);
				if( strncmp(szTime, szStartTime, iTimeLen)<0 || strncmp(szTime, szEndTime, iTimeLen)>0 )
				{
//					cout<<"匹配时间失败"<<endl;
					continue;
				}
			}
//			else
//				cout<<"无时间字段"<<endl;
		}
		/* 时间匹配则取出结果 */
//		cout<<"时间匹配或无时间"<<endl;
		bLack = false;
		//int size = tableInfo[tableOffset].table->tableRecord.columnList.size();
		//cout<<"size:"<<size<<endl;
		for ( int j=0; j<out_DataStruct->itemNum; j++ )
		{
			memset(temp, 0, sizeof(temp));
			tableInfo[tableOffset].table->getString( j+1, temp, sizeof(temp), tableResult[i]);
//			cout<<"temp["<<j+1<<"]:"<<temp<<endl;
			strcpy(out_DataStruct->values[j], temp);
		}
		memset(temp, 0, sizeof(temp));
		tableInfo[tableOffset].table->getString(iStartCol, temp, sizeof(temp), tableResult[i]);
		strcpy(out_DataStruct->startTime, temp);
		memset(temp, 0, sizeof(temp));
		tableInfo[tableOffset].table->getString(iEndCol, temp, sizeof(temp), tableResult[i]);
		strcpy(out_DataStruct->endTime, temp);
		break;
	}

	if(bLack)
	{
//		cout<<"=======没找到"<<endl;
		//gettimeofday(&finish, NULL);
		//timePart2 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
		searchNum++;
		return -1;
	}

//	cout<<"=======找到"<<endl;
	//gettimeofday(&finish, NULL);
	//timePart2 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
	searchNum++;
	return 0;
}

int C_AccessMem::getColNum(char* tableName)
{
	/*
	char ErrorMsg[LOG_MSG_LEN+1];
	map<string, int>::iterator it_Num = tableInfo.find(tableName);
	if(it_Num == tableInfo.end())
	{
		sprintf( ErrorMsg, "没找到表%s的记录", tableName);
		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}
	else
	{
		return it_Num->second;
	}*/
	for(int iNum=0; iNum<m_iTableNum; iNum++)
	{
		if(!(strcmp(tableInfo[iNum].tableName, tableName)))
		{
			return tableInfo[iNum].colNum;
		}
	}

	return -1;
}

int C_AccessMem::getSearchType(char* tableName, int iTableIndex)
{
	char ErrorMsg[LOG_MSG_LEN+1];
	tableIndex tbl;
	//cout << "tableName = " << tableName <<endl;
	//cout << "iTableIndex = " <<iTableIndex<<endl;
	strcpy(tbl.szName, tableName);
	tbl.iIndex = iTableIndex;
	//tbl.iIndex = 1;
	map<tableIndex, int>::iterator it_type = tableSearchType.find(tbl);
	if(it_type == tableSearchType.end())
	{
		sprintf( ErrorMsg, "没找到表%s的记录", tableName);
		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}
	else
	{
		return it_type->second;
	}
	return -1;
}

void C_AccessMem::printTime()
{
	//theJSLog<<"查询共享内存"<<searchNum<<"次，用时"<<timePart1<<";"<<timePart2<<endi;
	searchNum=0;
	//timePart1=0;
	//timePart2=0;
}
