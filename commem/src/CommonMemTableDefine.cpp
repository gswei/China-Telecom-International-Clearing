 /*
	* =====================================================================================
	*
	*		Filename: CommonMemTableDefine.cpp
	*		
	*		Description:
	*		
	*		Version: 1.0
	*		Created: 2010年4月3日
	*		Revision: none
	*		compiler:gcc
	*		Author:
	*		company:
	*	
	* ===================================================================================
	*/
	
#include <assert.h>
//#include <list>
#include <algorithm>
#include <math.h>
#include "CommonMemTableDefine.h"
#include<iostream>
#include<map>

using namespace std; 



using std::sort;

map<string, tableinfo> _map_table_info;   //ww	20150715


int CommonMemTableDefine::getTableInfo(const string& SERVER_ID,const string& mem_name, int version)
{
	char eMessage[512];
	tableinfo t_info;
	int ret;
	_map_table_info.clear();
	string sql = "SELECT  NVL(A.MEM_DEFINE_ID,0),C.TABLE_NAME,C.UPDATE_TABLE_NAME,C.WHERE_CONDITION FROM C_COMMEM_MEM_VERSION A,\
	C_COMMEM_TABLE_DEF C	WHERE  A.MEM_DEFINE_ID=C.MEM_DEFINE_ID AND A.SERVER_ID = :SERVER_ID AND A.MEM_NAME = :MEM_NAME AND A.VERSION_ID = :VERSION";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	dbConnect(conn);
	try
	{
   		Statement stmt = conn.createStatement();
		  stmt.setSQLString(sql);
	 	 stmt<<SERVER_ID<<mem_name<<version;
	 	 stmt.execute();
	  	stmt>>t_info.mem_define_id>>t_info.table_name>>t_info.update_table_name>>t_info.where_condition;
	//	theJSLog<<"wwtest00000    t_info.mem_define_id="<<t_info.mem_define_id<<"t_info.table_name="<<t_info.table_name<<endd;
		if(t_info.mem_define_id==0)
			{
				conn.close();
				theJSLog<<"共享内存配置表C_COMMEM_MEM_VERSION 的MEM_DEFINE_ID 字段配置为空，请检查配置!!!"<<endw;
				return -1;
			}
		if(t_info.table_name=="")
			{
				conn.close();
				theJSLog<<"共享内存配置表C_COMMEM_TABLE_DEF 的TABLE_NAME 字段配置为空，请检查配置!!!"<<endw;
				return -1;
			}
		if(t_info.update_table_name=="")
			{
				conn.close();
				theJSLog<<"共享内存配置表C_COMMEM_TABLE_DEF 的UPDATE_TABLE_NAME 字段配置为空，请检查配置!!!"<<endw;
				return -1;
			}
		string mname=mem_name;
//		theJSLog<<"wwtest11111    mem_name=" <<mname << " t_info.mem_define_id="<<t_info.mem_define_id<<"t_info.table_name="<<t_info.table_name<<endd;
		_map_table_info.insert(make_pair(mname, t_info));
//		theJSLog<<"wwtest 2222   mem_name=" <<mem_name << " t_info.mem_define_id="<<t_info.mem_define_id<<"t_info.table_name="<<t_info.table_name<<endd;
		conn.close();
	}
	catch ( CException e )
	{
		conn.close();
		sprintf(eMessage,"error_code=%d,程序执行sql出错,sql=%s,请检查C_COMMEM_MEM_VERSION 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
		return  -1;
	}
	catch( db::SQLException	e)
		{
			conn.close();
			theJSLog<<"数据库查询异常:"<<e.what()<<endw;
			return  -1;
		}
	return 1;	
		
}
/*
int CommonMemTableDefine::getTableConfigIdFromDB(const string& SERVER_ID,const string& mem_name, int version) {
	char eMessage[512];
	int ret;
	string sql = "SELECT \
		C.MEM_DEFINE_ID \
		FROM C_COMMEM_MEM_VERSION C \
		WHERE C.SERVER_ID = :SERVER_ID AND C.MEM_NAME = :MEM_NAME AND C.VERSION_ID = :VERSION";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	dbConnect(conn);
	try
	{
	//	CBindSQL bs(DBConn);
		//bs.Open(sql);
	//	bs<<SERVER_ID<<mem_name<<version;
		//bs>>ret; 
		//bs.Close();
		Statement stmt = conn.createStatement();
	  stmt.setSQLString(sql);
	  stmt<<SERVER_ID<<mem_name<<version;
	  stmt.execute();
	  stmt>>ret;
	conn.close();
	}
	catch ( CException e )
	{
		conn.close();
		sprintf(eMessage,"error_code=%d,程序执行sql出错,sql=%s,请检查C_COMMEM_MEM_VERSION 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	catch(SQLException	e)
		{
			theJSLog<<"数据库查询异常:"<<e.what()<<endw;
			return ret ;
		}

	return ret;
}

string CommonMemTableDefine::getRealTableNameFromDB(int tableConfigId) {
	char eMessage[512];
	string ret;
	string sql = "SELECT \
		C.TABLE_NAME \
		FROM C_COMMEM_TABLE_DEF C \
		where C.MEM_DEFINE_ID = :MEM_DEFINE_ID";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	dbConnect(conn);
	try
	{
		//CBindSQL bs(DBConn);
		//bs.Open(sql);
		///bs<<tableConfigId;
		//bs>>ret;
		//bs.Close();
		Statement stmt = conn.createStatement();
	  stmt.setSQLString(sql);
	  stmt<<tableConfigId;
	  stmt.execute();	
	  stmt>>ret;
	  conn.close();
	} catch ( CException e )
	{
		conn.close();
		sprintf(eMessage,"error_code=%d,执行sql出错,sqlStr=%s,请检查C_COMMEM_TABLE_DEF 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	return ret;


}


string CommonMemTableDefine::getUpdateTableNameFromDB(int tableConfigId) {
	char eMessage[512];
	string ret;
	string sql = "SELECT \
		C.UPDATE_TABLE_NAME \
		FROM C_COMMEM_TABLE_DEF C \
		where C.MEM_DEFINE_ID = :MEM_DEFINE_ID";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	dbConnect(conn);
	try
	{
		//CBindSQL bs(DBConn);
	//	bs.Open(sql);
	//	bs<<tableConfigId;
	//	bs>>ret;
	//	bs.Close();
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt<<tableConfigId;
	stmt.execute();	
	stmt>>ret;
	conn.close();
	} catch ( CException e )
	{
		sprintf(eMessage,"error_code=%d,执行sql出错,sqlStr=%s,请检查C_COMMEM_TABLE_DEF 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	return ret;
}

string CommonMemTableDefine::getWhereConditionFromDB(int tableConfigId) {
	char eMessage[512];
	string ret;
	string sql = "SELECT \
		C.WHERE_CONDITION \
		FROM C_COMMEM_TABLE_DEF C \
		where C.MEM_DEFINE_ID = :MEM_DEFINE_ID";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	dbConnect(conn);
	try
	{
		//CBindSQL bs(DBConn);
		//bs.Open(sql);
		//bs<<tableConfigId;
		//bs>>ret;
		//bs.Close();
		Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt<<tableConfigId;
	stmt.execute();
	stmt>>ret;
	conn.close();
	} catch ( CException e )
	{
		sprintf(eMessage,"error_code=%d,执行sql出错,sqlStr=%s,请检查C_COMMEM_TABLE_DEF 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	return ret;
}
*/
void CommonMemTableDefine::readTableStructFromDB(int tableConfigId) {
	char eMessage[512];
	string sql = "SELECT \
		C.COLUMN_ID,C.COLUMN_NAME, C.COLUMN_TYPE,C.TIME_COL, \
		C.COLUMN_SIZE \
		FROM C_COMMEM_COLUMN_DEF C  \
		WHERE C.MEM_DEFINE_ID = :MEM_DEFINE_ID \
		ORDER BY C.COLUMN_ID";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	dbConnect(conn);
	try
	{
		//CBindSQL bs(DBConn);
		//bs.Open(sql);
		//bs<<tableConfigId;
		Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt<<tableConfigId;
	stmt.execute();	
		char columnN[101];
		string columnName, columnType,timeCol;
		int columnSize;
		int columnID;
		//while (bs>>columnID>>columnN>>columnType>>timeCol>>columnSize) 
		while (stmt>>columnID>>columnN>>columnType>>timeCol>>columnSize) 
		{
			toUpper(columnN);
			columnName=columnN;
			if (columnType == "CHAR") {
				tableRecord.addColumn(columnName, CHAR, columnID,timeCol, columnSize);
			} else if (columnType == "INT64") {
				tableRecord.addColumn(columnName, INT64, columnID,timeCol, columnSize);
			} else if (columnType == "INT32") {
				tableRecord.addColumn(columnName, INT32, columnID,timeCol, columnSize);
			} else {
				sprintf(eMessage,"字段类型定义错误,columnN=%s,columnType=%s",columnName.c_str(),columnType.c_str());
				throw CException(10008,eMessage,__FILE__,__LINE__);	
			}
			theJSLog<<"columnName="<<columnName.c_str()<<",columnType="<<columnType.c_str()<<",columnSize="<<columnSize<<","<<__FILE__<<","<<__LINE__<<endd;
		} 
		//bs.Close();
		conn.close();
		bufferLen = tableRecord.getRecordSize()+1;
		recordBuffer = new byte_t[bufferLen];
		memset(recordBuffer,0,bufferLen);
		theJSLog<<"表"<<realTableName.c_str()<<"记录大小="<<tableRecord.getRecordSize()<<","<<__FILE__<<","<<__LINE__<<endd;
	} catch ( CException e )
	{
		sprintf(eMessage,"error_code=%d,执行sql出错,sqlStr=%s,请检查C_COMMEM_COLUMN_DEF 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	catch( db::SQLException	e)
		{
			conn.close();
			theJSLog<<"数据库查询异常:"<<e.what()<<endw;
			throw e;
		}
	return ;
}

void CommonMemTableDefine::readIndexStructFromDB(int tableConfigId) {
	char eMessage[512];
	string sql = "SELECT \
		C.INDEX_ID, C.INDEX_TYPE, C.INDEX_COLUMNS \
		FROM C_COMMEM_INDEX_DEF C \
		WHERE C.MEM_DEFINE_ID = :TABLE_DEFINE_ID \
		ORDER BY C.INDEX_ID";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	dbConnect(conn);
	try
	{
	//	CBindSQL bs(DBConn);
	//	bs.Open(sql);
	//	bs<<tableConfigId;
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt<<tableConfigId;
	stmt.execute();	
		int indexId;
		string indexType, indexColumns;
		RecordStruct *rs;
		char columnN[1001];
		//while (bs>>indexId>>indexType>>columnN) 
		while (stmt>>indexId>>indexType>>columnN) 
		{
			rs = new RecordStruct();
			toUpper(columnN);
			indexColumns=columnN;
			theJSLog<<"index indexId="<<indexId<<",indexType="<<indexType<<",columnName="<<indexColumns<<","<<__FILE__<<","<<__LINE__<<endd;
			vector<string> columnNames = getStrings(indexColumns,1);
			// Sort ColumnNames
			//sort(columnNames.begin(), columnNames.end());
			// 排序后的列名
			string orderColumnNames;
			for (int i = 0; i < columnNames.size(); i++) {
				int columnIndex = tableRecord.getColIndexByName(columnNames[i]);
				if (columnIndex == -1) 
				{
					sprintf(eMessage,"索引字段名在表字段名中找不到,columnNames=%s",columnNames[i].c_str());
					throw CException(10009,eMessage,__FILE__,__LINE__);	
				}
				#ifndef _LINUX_ACC
				rs->addColumn(tableRecord.getColumnList()->at(columnIndex).columnName,
						tableRecord.getColumnList()->at(columnIndex).columnType,
						tableRecord.getColumnList()->at(columnIndex).orderID,
						tableRecord.getColumnList()->at(columnIndex).timeCol,
						tableRecord.getColumnList()->at(columnIndex).columnSize);
				#else
				rs->addColumn((*tableRecord.getColumnList())[columnIndex].columnName,
						(*tableRecord.getColumnList())[columnIndex].columnType,
						(*tableRecord.getColumnList())[columnIndex].orderID,
						(*tableRecord.getColumnList())[columnIndex].timeCol,
						(*tableRecord.getColumnList())[columnIndex].columnSize);
				#endif
				orderColumnNames += columnNames[i] + ",";
			}
			IndexType iType;
			if (indexType == "UNIQUE") {
				iType = NON_UNIQUE;
			} else if (indexType == "NON_UNIQUE") {
				iType = NON_UNIQUE;
			} else if (indexType == "LONGIST") {
				iType = LONGIST;
				//LONGIST只允许单一列索引
				if (columnNames.size() > 1)
				{
					sprintf(eMessage,"长度优先索引只允许单字段索引,columncount=%d",columnNames.size());
					throw CException(10009,eMessage,__FILE__,__LINE__);	
				}
			} else {
					sprintf(eMessage,"索引类型定义错误,indexType=%s",indexType.c_str());
					throw CException(10009,eMessage,__FILE__,__LINE__);	
			}
  	
			indexTypes.push_back(iType);
			indexID.push_back(indexId);
			indexRecord.push_back(rs);
			//indexChoser[orderColumnNames] = indexRecord.size() - 1;
			theJSLog<<"索引"<<indexId<<"记录大小="<<rs->getRecordSize()<<","<<__FILE__<<","<<__LINE__<<endd;

		}
		//bs.Close();
		conn.close();
	} catch ( CException e )
	{
		sprintf(eMessage,"error_code=%d,执行sql出错,sqlStr=%s,请检查C_COMMEM_INDEX_DEF 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	catch( db::SQLException	e)
		{
			conn.close();
			theJSLog<<"数据库查询异常:"<<e.what()<<endw;
			throw e;
		}
	return ;
}

vector<string> CommonMemTableDefine::getStrings(const string &str,int isUpper) 
{
	char *sz_ids = new char[str.length() + 1];
	memset(sz_ids, 0, str.length() + 1);
	vector<string> ret;
	strncpy(sz_ids, str.c_str(), str.length());
	char *p;
	p = strtok(sz_ids, ",");
	while (p != NULL) {
					if ( isUpper ) toUpper(p);
	        ret.push_back(p);
	        p = strtok(NULL, ",");
	}
	
	if (sz_ids != NULL) 
	{
		delete[] sz_ids;
	}
	return ret;
}

mysize_t CommonMemTableDefine::getCapacityFromDB(const string& tablename, int version) 
{
	char eMessage[512];
	mysize_t ret;
	string sql = "SELECT \
		count(*) \
		FROM  ";
		sql = sql+realTableName;
		if ( whereCondition.size() != 0 )
		{
			sql = sql+" C WHERE "+whereCondition ;
		}
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	try
	{
		
		//CBindSQL bs(DBConn);
	//	bs.Open(sql);
	//	bs>>ret;
	//	bs.Close();
	dbConnect(conn);
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt.execute();	
	stmt>>ret;
	conn.close();
	} catch ( CException e )
	{
		sprintf(eMessage,"error_code=%d,执行sql出错,sqlStr=%s,请检查%s 表是否存在，配置是否正确!!!",e.GetAppError(),sql.c_str(),realTableName.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	catch( db::SQLException	e)
		{
			conn.close();
			theJSLog<<"数据库查询异常:"<<e.what()<<endw;
			throw e;
		}
	if ( ret==0 ) ret=1;
	theJSLog<<"Capacity="<<ret<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	return ret;

}

//modi by sunh 2010-11-22
void CommonMemTableDefine::getSHMkeys(const string& server_id,const string& tablename, int version, const string& key_filename) 
{
	int ret;
	char eMessage[512];
	string sql = "SELECT \
		C.SHMKEY_ID \
		FROM C_COMMEM_CONTROL C \
		WHERE C.SERVER_ID = :SERVER_ID AND C.MEM_NAME = :MEM_NAME AND C.VERSION_ID = :VERSION";
	theJSLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	try
	{
	//	CBindSQL bs(DBConn);
	//	bs.Open(sql);
	//	bs<<server_id<<tablename<<version;
	//	bs>>ret;
	//	bs.Close();
	dbConnect(conn);
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt<<server_id<<tablename<<version;
	cout <<server_id<<tablename<<version;
	stmt.execute();	
	stmt>>ret;
	conn.close();
	} catch ( CException e )
	{
		sprintf(eMessage,"error_code=%d,执行sql出错,sqlStr=%s,请检查C_COMMEM_CONTROL 表配置是否正确!!!",e.GetAppError(),sql.c_str());
		e.PushStack(10007,eMessage,__FILE__, __LINE__);
		throw e;
	}
	catch( db::SQLException	e)
		{
			conn.close();
			theJSLog<<"数据库查询异常:"<<e.what()<<endw;
			throw e;
		}
	theJSLog<<"ret="<<ret<<","<<__FILE__<<","<<__LINE__<<endd;

	char fname[255];
	sprintf(fname,"%s%d",key_filename.c_str(),ret);
	FILE *tmpfp;
	if ( ( tmpfp = fopen( fname,"r" ) ) != NULL )
	{
		fclose( tmpfp );
	} else {
		tmpfp = fopen( fname,"w" ) ;
		char buf[255];
		strcpy(buf,"COMMEM\n");
		fputs( buf,tmpfp );
		fclose( tmpfp );
	}
	
	int tableIndex = 160;
		
	//tableSHMKey = ftok(fname, tableIndex);

	//核心参数获取
	std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.commem.commemkeyvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "获取共享内存固定key 值" );
		commem_keyvalue = "91671000";
	}
	int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );
	
	tableSHMKey = commenKeyValue + ret;
	char tempStr[255];
	sprintf(tempStr,"%x",tableSHMKey);		
	
	theJSLog<<"tableIndex="<<tableIndex<<",tableSHMKey=0x"<<tempStr<<","<<__FILE__<<","<<__LINE__<<endi;
	for (int i = 0; i < indexRecord.size(); i++) {
		//int indexkey = ftok(fname, tableIndex + i + 1);
		int indexkey = commenKeyValue + (i+1)*100 + ret;
		sprintf(tempStr,"%x",indexkey);
		indexSHMKeys.push_back(indexkey);
		theJSLog <<"indexkey = " << indexkey <<endi;
		theJSLog<<"idxIndex="<<tableIndex + i + 1<<",idxSHMKey=0x"<<tempStr<<","<<__FILE__<<","<<__LINE__<<endi;
	}
	
	return ;

}

CommonMemTableDefine::CommonMemTableDefine()
{
	recordBuffer=NULL;
	conn = NULL;
}

CommonMemTableDefine::CommonMemTableDefine(const string &server_id, const char *table_name, int version_id,const char *key_filename,int site_id,S_MemManager *commonTable,C_NRWLock *m_RWLock) 
{
	conn = NULL;
	char eMessage[512];
	mysize_t capacityCreate;
	try
	{
		memName=table_name;
		version=version_id;
		
		// TODO get tableConfigId by table_name and version
	//	tableConfigId = getTableConfigIdFromDB(server_id,memName, version);
	//	theJSLog<<"tableConfigId="<<tableConfigId<<","<<__FILE__<<","<<__LINE__<<endd;
	//	realTableName = getRealTableNameFromDB(tableConfigId);
	//	if(realTableName=="")
	//	{	
	//			theJSLog<<"realTableName="<<realTableName.c_str()<<","<<__FILE__<<","<<__LINE__<<endd;
	//			theJSLog<<"realTableName  表名配置为空，请检查 C_COMMEM_MEM_VERSION 或 C_COMMEM_TABLE_DEF 表配置是否正确！！！"<<endd;
	//	}
	//	theJSLog<<"realTableName="<<realTableName.c_str()<<","<<__FILE__<<","<<__LINE__<<endd;
	//	updateTableName = getUpdateTableNameFromDB(tableConfigId);
	//	theJSLog<<"updateTableName="<<updateTableName.c_str()<<","<<__FILE__<<","<<__LINE__<<endd;
	//	whereCondition = getWhereConditionFromDB(tableConfigId);
	//	theJSLog<<"whereCondition="<<whereCondition.c_str()<<","<<__FILE__<<","<<__LINE__<<endd;
	          
		if(getTableInfo(server_id,memName, version)==-1)
			{
				theJSLog<<"读取配置信息失败，请检查相关配置!!!"<<endd;
			}
//		theJSLog<<"wwtest     _itertableinfo    "<<endd;
		map<string,tableinfo>::iterator	  _itertableinfo;
//		theJSLog<<"wwtest      tableinfo   "<<endd;
		tableinfo tainfo;
//		theJSLog<<"wwtest      _map_table_info.find   "<<endd;
		_itertableinfo=_map_table_info.find(memName);
		tainfo=_itertableinfo->second;
		tableConfigId=tainfo.mem_define_id;
		realTableName=tainfo.table_name;
		updateTableName=tainfo.update_table_name;
		whereCondition=tainfo.where_condition;

//		theJSLog<<"wwtest    tainfo.mem_define_id  =     "<<tainfo.mem_define_id<<endd;
		readTableStructFromDB(tableConfigId);
//		theJSLog<<"wwtest     readTableStructFromDB      "<<endd;
		readIndexStructFromDB(tableConfigId);
//		theJSLog<<"wwtest     readIndexStructFromDB      "<<endd;
		
		capacity = getCapacityFromDB(memName, version);
		theJSLog<<"key_filename="<<key_filename<<","<<__FILE__<<","<<__LINE__<<endd;
  	
		// Get shmkeys
  	
		getSHMkeys(server_id,memName, version, key_filename);  
	} catch ( CException e )
	{
		sprintf(eMessage,"初始化表相关信息出错,MEM_NAME=%s",memName.c_str());
		e.PushStack(11000,eMessage,__FILE__, __LINE__);
		throw e;
	}
	// Init share memorys
	//
	try
	{
		//如果上次已创建，则先删除共享内存
		if ( site_id < commonTable->iTotalTable )
		{
			theJSLog<<"上一次创建的共享内存还存在，先删掉"<<endi;
			shmctl(commonTable->m_TableInfo[site_id].ishmid,IPC_RMID,NULL);
			theJSLog<<"IPCRM 数据区:"<<commonTable->m_TableInfo[site_id].ishmid<<","<<__FILE__<<","<<__LINE__<<endd;
			for ( int idxid=0; idxid<MEM_MAX_INDEX; idxid++ )
			{
				if ( commonTable->m_TableInfo[site_id].iIdxshmid[idxid] > 0 )
				{
					shmctl(commonTable->m_TableInfo[site_id].iIdxshmid[idxid],IPC_RMID,NULL);
					theJSLog<<"IPCRM 索引区["<<idxid<<"]:"<<commonTable->m_TableInfo[site_id].iIdxshmid[idxid]<<","<<__FILE__<<","<<__LINE__<<endd;
				}
			}
		}
		capacityCreate=ceil(capacity*1.1);
		memTable = new CommonMemTable(&tableRecord, capacityCreate, tableSHMKey);
  	
  	
		for (int i = 0; i < indexRecord.size(); i++) {
			CommonMemIndex* index;
			switch(indexTypes[i]) {
			case UNIQUE :
				//CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key);
				index = new CommonMemAVLIndex(&tableRecord, indexRecord[i], capacityCreate, indexSHMKeys[i],indexID[i]);
				memIndexs.push_back(index);
				break;
			case NON_UNIQUE :
				//TODO
				//CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key);
				index = new CommonMemAVLIndex(&tableRecord, indexRecord[i], capacityCreate, indexSHMKeys[i],indexID[i]);
				memIndexs.push_back(index);
				break;
			case LONGIST :
				// TODO
				int idx_size;
				if ( capacityCreate < 101 ) idx_size=500;
				else if ( capacityCreate > 100 && capacityCreate < 1001 ) idx_size=4000;
				else if ( capacityCreate > 1000 && capacityCreate <10001 ) idx_size=30000;
				else idx_size=capacityCreate*2;
				index = new CommonMemLenIndex(&tableRecord, indexRecord[i], idx_size, indexSHMKeys[i],indexID[i]);
				memIndexs.push_back(index);
				break;
			}
		}

		m_RWLock->GetWriteLock(commonTable->memlock);
		strcpy(commonTable->m_TableInfo[site_id].MemName,memName.c_str());
		commonTable->m_TableInfo[site_id].version=version;
		commonTable->m_TableInfo[site_id].ishmid=memTable->getBlockManager()->getShmid();
		for ( int jj=0;jj<MEM_MAX_INDEX;jj++ )
		{
			commonTable->m_TableInfo[site_id].iIdxshmid[jj]=0;
		}
		for ( int kk=0;kk<memIndexs.size();kk++ )
		{
			#ifndef _LINUX_ACC
			int idxSite=memIndexs.at(kk)->getIndexId();
			commonTable->m_TableInfo[site_id].iIdxshmid[idxSite]=memIndexs.at(kk)->getBlockManager()->getShmid();
			#else
			int idxSite=memIndexs[kk]->getIndexId();
			commonTable->m_TableInfo[site_id].iIdxshmid[idxSite]=memIndexs[kk]->getBlockManager()->getShmid();
			#endif
		}		
		commonTable->isUsed = 0;   
		m_RWLock->UnLock(commonTable->memlock);
		
		if(loadData()==-1 )
			{
				theJSLog<<"共享内存导入数据时出错，请检查相关配置!!!"<<endd;
			}
		
		//创建完detach共享内存
		memTable->detach();
		for (int k = 0; k < memIndexs.size(); k++)
		{
			#ifndef _LINUX_ACC
			memIndexs.at(k)->detach();
			#else
			memIndexs[k]->detach();
			#endif
		}
		
	} catch ( CException e )
	{
		if ( memTable != NULL ) delete memTable;
		vector<CommonMemIndex*>::iterator it;
		for ( it=memIndexs.begin(); it!= memIndexs.end(); it ++)
		{
			delete &it;
		}
		memIndexs.clear();
		sprintf(eMessage,"创建表数据和索引时出错,table_name=%s",memName.c_str());
		e.PushStack(11000,eMessage,__FILE__, __LINE__);

		//modi by ww at 2015-7-8
		//释放内存锁
		m_RWLock->UnLock(commonTable->memlock);
		//更新标志位为'E'
		string m_name=memName;
		string sqlStr="update C_COMMEM_CONTROL set status='E' where updatetype=0 and valid_flag='Y' and mem_name='"+m_name+"'and server_id='";
		sqlStr=sqlStr+server_id+"'";
		theJSLog<<"sqlStr="<<sqlStr<<__FILE__<<","<<__LINE__<<endd;
		dbConnect(conn);
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sqlStr);    
  		stmt.execute();
 		 conn.commit();
 		 conn.close();
			 
		throw e;
	}
	//获取写锁
	theJSLog<<"读取表记录结束,更新公共表记录信息,"<<__FILE__<<","<<__LINE__<<endd;
	m_RWLock->GetWriteLock(commonTable->memlock);
	strcpy(commonTable->m_TableInfo[site_id].MemName,memName.c_str());
	commonTable->m_TableInfo[site_id].version=version;
	strcpy(commonTable->m_TableInfo[site_id].chTableName,realTableName.c_str());
	commonTable->m_TableInfo[site_id].iMemSize=memTable->getBlockManager()->getMemorySize();
	commonTable->m_TableInfo[site_id].iRealRows=capacity;
	commonTable->m_TableInfo[site_id].iMemRows=capacityCreate;
	commonTable->m_TableInfo[site_id].iTableKey=tableSHMKey;
	commonTable->m_TableInfo[site_id].ishmid=memTable->getBlockManager()->getShmid();
	for ( int j=0;j<MEM_MAX_INDEX;j++ )
	{
		commonTable->m_TableInfo[site_id].iIdxshmid[j]=0;
	}
	for ( int k=0;k<memIndexs.size();k++ )
	{
		#ifndef _LINUX_ACC
		int idxSite=memIndexs.at(k)->getIndexId();
		commonTable->m_TableInfo[site_id].iIdxKey[idxSite]=memIndexs.at(k)->getBlockManager()->getShmKey();
		commonTable->m_TableInfo[site_id].iIdxshmid[idxSite]=memIndexs.at(k)->getBlockManager()->getShmid();
		#else
		int idxSite=memIndexs[k]->getIndexId();
		commonTable->m_TableInfo[site_id].iIdxKey[idxSite]=memIndexs[k]->getBlockManager()->getShmKey();
		commonTable->m_TableInfo[site_id].iIdxshmid[idxSite]=memIndexs[k]->getBlockManager()->getShmid();	
		#endif
	}
	if (commonTable->isUsed == 0) commonTable->isUsed = 1;
	if (site_id==commonTable->iTotalTable) commonTable->iTotalTable++;
	//释放写锁
	m_RWLock->UnLock(commonTable->memlock);
	
	return ;
}

string CommonMemTableDefine::getMemName() const
{
	return memName;
}

int CommonMemTableDefine::getVersion() const
{
	return version;
}

int CommonMemTableDefine::loadData()
{
	int cnt=0;
	int isEnd;
	//clock_t t1=0;
	//clock_t tt=0;
    try
	{
	dbConnect(conn);
	//获取记录总数
	string countSql = makeSelectCountSQL();
	int iAllCount = 0;
	Statement stmtCount = conn.createStatement();
	stmtCount.setSQLString(countSql);
  stmtCount.execute();
  stmtCount>>iAllCount;
	stmtCount.close();
	
	//获取记录内容
	string querySql = makeSelectAllSQL();
	// DATABASE query sql
	//CBindSQL bs(DBConn);
	// for every row
	//   for every column
	theJSLog<<"打开sql游标."<<endi;
	//bs.Open(querySql);
	
	Statement stmt = conn.createStatement();
	stmt.setSQLString(querySql);
  stmt.execute();
	theJSLog<<querySql<<endi;
	isEnd=1;
	theJSLog<<"开始导入数据."<<endi;
	
        theJSLog<<"iAllCount:="<<iAllCount<<endd;
	//while (!bs.IsEnd()) {
	for(int i=0;i<iAllCount;i++){
		const vector<ColumnStruct>* columns = tableRecord.getColumnList();
		memset(recordBuffer,0,bufferLen);
		theJSLog<<"columns->size():"<<columns->size()<<endd;
		for (int i = 0; i < columns->size(); i++) 
		{
			#ifndef _LINUX_ACC
			switch(columns->at(i).columnType) {
			#else
			switch((*columns)[i].columnType) {
			#endif
			case INT32 :
				long buf32;
			//	bs>>buf32;
			stmt>>buf32;
			//	if(bs.IsEnd())
			//	{
			//		isEnd=1;
			//		break;
			//	}
				isEnd=0;
				DEBUG_LOG<<"get buf32="<<buf32<<",i="<<i<<__FILE__<<","<<__LINE__<<endd;
				tableRecord.setRealInt32(recordBuffer, i, buf32);
				break;
				/*这种是为了避免数据库配置有number(12)长整型，但stmt提供的接口里面不具备该功能，暂时注销
			case INT64 :
				long long buf64;
				float buf_f;
				//bs>>buf_f;
				stmt>>buf_f;
			//	if(bs.IsEnd())
			//	{
			//		isEnd=1;
			//		break;
			//	}
				isEnd=0;
				buf64=buf_f;
				DEBUG_LOG<<"get buf64="<<buf64<<",i="<<i<<__FILE__<<","<<__LINE__<<endd;
				tableRecord.setRealInt64(recordBuffer, i, buf64);
				break;
				*/
			case CHAR :
				string bufs;
			//	bs>>bufs;
				stmt>>bufs;
			//	if(bs.IsEnd())
			//	{
			//		isEnd=1;
			//		break;
			//	}
				isEnd=0;
				DEBUG_LOG<<"get char[]="<<bufs.c_str()<<",i="<<i<<__FILE__<<","<<__LINE__<<endd;
				tableRecord.setRealString(recordBuffer, i, bufs.c_str());
				break;
			}
		}
		theJSLog<<"insert recordBuffer="<<recordBuffer<<__FILE__<<","<<__LINE__<<endd;
		theJSLog<<"cnt:"<<cnt<<endd;
		//tt=clock();
		if ( isEnd==0 ) {
			insertRecord(recordBuffer);
		//t1+=clock()-tt;
			cnt++;
		}
		
		if ( cnt % 10000 == 0 && cnt!=0 )
		{
			theJSLog<<"已导入 "<<cnt<<" 条记录"<<endi;
		}
	 }
	//bs.Close();
	conn.close();
	}
	catch( db::SQLException	e)
		{
			conn.close();
			theJSLog<<"数据库查询异常:"<<e.what()<<endw;
			return  -1;
		}
		//}
	//theJSLog<<"耗时"<<t1//CLOCKS_PER_SEC<<endd;
	theJSLog<<"共导入 "<<cnt<<" 条记录"<<endi;
	for (int idxi = 0; idxi < memIndexs.size(); idxi++) 
	{
		theJSLog<<"第"<<idxi+1<<"个索引使用情况["<<memIndexs[idxi]->getBlockManager()->getUsingBlocksNum()+1<<"||"<<memIndexs[idxi]->getBlockManager()->getTotalBlocksNum()<<"]"<<endi;
	}
//}
	return 1;
		//}
}


CommonMemTableDefine::~CommonMemTableDefine() {
	delete recordBuffer;
	conn = NULL;
}
	
void CommonMemTableDefine::reloadTable( const string &SERVER_ID, const string& mem_name, int version, const int shmid_key,int site_id ,struct S_MemManager *commonTable,C_NRWLock *m_RWLock ) 
{
	mysize_t capacityCreate;
	memTable->destory();
	for (int i = 0; i < memIndexs.size(); i++) {
		#ifndef _LINUX_ACC
		memIndexs.at(i)->destory();
		#else
		memIndexs[i]->destory();
		#endif
	}
	
	capacity = getCapacityFromDB(memName, version);
	capacityCreate=ceil(capacity*1.1);
	memTable->create(capacityCreate, tableSHMKey);
	for (int j = 0; j < memIndexs.size(); j++) 
	{
		switch(indexTypes[j]) 
		{
		case UNIQUE :
			#ifndef _LINUX_ACC
			memIndexs.at(j)->create(capacityCreate, indexSHMKeys[j]);
			#else
			memIndexs[j]->create(capacityCreate, indexSHMKeys[j]);
			#endif
			break;
		case NON_UNIQUE :
			#ifndef _LINUX_ACC
			memIndexs.at(j)->create(capacityCreate, indexSHMKeys[j]);
			#else
			memIndexs[j]->create(capacityCreate, indexSHMKeys[j]);
			#endif
			break;
		case LONGIST :
			int idx_size;
			if ( capacityCreate < 101 ) idx_size=500;
			else if ( capacityCreate > 100 && capacityCreate < 1001 ) idx_size=4000;
			else if ( capacityCreate > 1000 && capacityCreate <10001 ) idx_size=30000;
			else idx_size=capacityCreate*2;
			#ifndef _LINUX_ACC
			memIndexs.at(j)->create(idx_size, indexSHMKeys[j]);
			#else
			memIndexs[j]->create(idx_size, indexSHMKeys[j]);
			#endif
			break;
		}
	}	
	if(loadData()==-1 )
		{
				theJSLog<<"共享内存导入数据时出错，请检查相关配置!!!"<<endd;
		}
	
	//创建完detach共享内存
	memTable->detach();
	theJSLog<<"memTable->detach();     完成0!"<<endd;
	for (int k = 0; k < memIndexs.size(); k++)
	{
		#ifndef _LINUX_ACC
		memIndexs.at(k)->detach();
		theJSLog<<"memIndexs.at(k)->detach();     完成1!"<<endd;
		#else
		memIndexs[k]->detach();
		theJSLog<<"memIndexs[k]->detach();     完成2!"<<endd;
		#endif
	}
	
	//获取写锁
	m_RWLock->GetWriteLock(commonTable->memlock);
	commonTable->m_TableInfo[site_id].iMemSize=memTable->getBlockManager()->getMemorySize();
	commonTable->m_TableInfo[site_id].iRealRows=capacity;
	commonTable->m_TableInfo[site_id].iMemRows=capacityCreate;
	commonTable->m_TableInfo[site_id].iTableKey=memTable->getBlockManager()->getShmKey();
	commonTable->m_TableInfo[site_id].ishmid=memTable->getBlockManager()->getShmid();

	theJSLog<<"获取写锁                       完成3!"<<endd;
	for ( int j=0;j<MEM_MAX_INDEX;j++ )
	{
		commonTable->m_TableInfo[site_id].iIdxshmid[j]=0;
	}
	theJSLog<<"for ( int j=0;j<MEM_MAX_INDEX;j++ )          完成4!"<<endd;
	for ( int k=0;k<memIndexs.size();k++ )
	{
		#ifndef _LINUX_ACC
		int idxSite=memIndexs.at(k)->getIndexId();
		commonTable->m_TableInfo[site_id].iIdxKey[idxSite]=memIndexs.at(k)->getBlockManager()->getShmKey();
		commonTable->m_TableInfo[site_id].iIdxshmid[idxSite]=memIndexs.at(k)->getBlockManager()->getShmid();
		#else
		int idxSite=memIndexs[k]->getIndexId();
		commonTable->m_TableInfo[site_id].iIdxKey[idxSite]=memIndexs[k]->getBlockManager()->getShmKey();
		commonTable->m_TableInfo[site_id].iIdxshmid[idxSite]=memIndexs[k]->getBlockManager()->getShmid();
		#endif
	}
	theJSLog<<"for ( int k=0;k<memIndexs.size();k++ )          完成5!"<<endd;
	if (commonTable->isUsed == 0) commonTable->isUsed = 1;
	//释放写锁
	m_RWLock->UnLock(commonTable->memlock);
	theJSLog<<"释放写锁                         完成6!"<<endd;

}

void CommonMemTableDefine::insertRecord(const byte_t* record) 
{
	//if (memIndexs.size() = 0) return ;
	//byte_t recordBuffer[RECORD_MAX];
	//byte_t keyBuffer[RECORD_MAX];
	//tableRecord.copyTo(keyBuffer, &indexRecord[0], record);
	//mysize_t recordPos = memIndexs[0]->queryUnique(keyBuffer);

	//if (recordPos != -1) {
		// TODO or update the record;
		//return;
	//} else {
	mysize_t rowIndex = memTable->insertRow(record);
	for (int i = 0; i < memIndexs.size(); i++) 
	{
			memIndexs[i]->insertIndex(record, rowIndex);
	}

	return;
}


void CommonMemTableDefine::destory() {
	memTable->destory();
	for (int i = 0; i < memIndexs.size(); i++) {
		memIndexs[i]->destory();
	}
}

void CommonMemTableDefine::detach() {
		memTable->detach();
		for (int k = 0; k < memIndexs.size(); k++)
		{
			#ifndef _LINUX_ACC
			memIndexs.at(k)->detach();
			#else
			memIndexs[k]->detach();
			#endif
		}
}

string CommonMemTableDefine::makeSelectAllSQL() {
	string ret = "SELECT ";
	const vector<ColumnStruct>* columns = tableRecord.getColumnList();
	for (int i = 0; i < columns->size(); i++) {
		#ifndef _LINUX_ACC
		ret += columns->at(i).columnName;
		#else
		ret += (*columns)[i].columnName;
		#endif
		if (i != columns->size() - 1) {
			ret += ", ";
		}
	}

	ret += " FROM " + realTableName;
	if (whereCondition.size() != 0) {
		ret += " WHERE " + whereCondition;
	}
	theJSLog<<"sqlStr="<<ret<<__FILE__<<","<<__LINE__<<endd;
	return ret;

}

string CommonMemTableDefine::makeSelectCountSQL() {
	string ret = "SELECT ";
  ret += " count(*) ";
	ret += " FROM " + realTableName;
	if (whereCondition.size() != 0) {
		ret += " WHERE " + whereCondition;
	}
	theJSLog<<"sqlStr="<<ret<<__FILE__<<","<<__LINE__<<endd;
	return ret;

}


void CommonMemTableDefine::AttachTable(const string& SERVER_ID,const char *mem_name, int version_id ,const char *iniPath, int tLine, S_MemManager *commonTable,C_NRWLock *m_RWLock)
{
	string serverId = SERVER_ID;
	memName=mem_name;
	version=version_id;
	
//	tableConfigId = getTableConfigIdFromDB(serverId,memName, version);
//	theJSLog<<"tableConfigId="<<tableConfigId<<","<<__FILE__<<","<<__LINE__<<endd;
//	realTableName = getRealTableNameFromDB(tableConfigId);
//	theJSLog<<"realTableName="<<realTableName.c_str()<<","<<__FILE__<<","<<__LINE__<<endd;
	if(getTableInfo(serverId,memName, version)==-1)
		{
			theJSLog<<"读取配置信息失败，请检查相关配置!!!"<<endd;
		}
	map<string,tableinfo>::iterator   _itertableinfo;
	tableinfo tainfo;
	_itertableinfo=_map_table_info.find(memName);
	tainfo=_itertableinfo->second;
	tableConfigId=tainfo.mem_define_id;
	realTableName=tainfo.table_name;
	readTableStructFromDB(tableConfigId);


	int m_memid=0;
	int tableLine=tLine;
	m_memid = commonTable->m_TableInfo[tableLine].ishmid;
	memTable = new CommonMemTable(&tableRecord,commonTable->m_TableInfo[tableLine].iMemRows,m_memid,0);

	readIndexStructFromDB(tableConfigId);
	
	for ( int i=0 ; i<indexID.size() ; i++ )
	{
		int indexId = indexID[i];
		CommonMemIndex* index;
		switch(indexTypes[i]) {
			case UNIQUE :
				//CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key);
				#ifndef _LINUX_ACC
				index = new CommonMemAVLIndex(&tableRecord, indexRecord.at(i), commonTable->m_TableInfo[tableLine].iMemRows, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				#else
				index = new CommonMemAVLIndex(&tableRecord, indexRecord[i], commonTable->m_TableInfo[tableLine].iMemRows, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				#endif
				memIndexs.push_back(index);
				break;
			case NON_UNIQUE :
				//TODO
				//CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key);
				#ifndef _LINUX_ACC
				index = new CommonMemAVLIndex(&tableRecord, indexRecord.at(i), commonTable->m_TableInfo[tableLine].iMemRows, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				#else
				index = new CommonMemAVLIndex(&tableRecord, indexRecord[i], commonTable->m_TableInfo[tableLine].iMemRows, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				#endif
				memIndexs.push_back(index);
				break;
			case LONGIST :
				// TODO
				#ifndef _LINUX_ACC
				index = new CommonMemLenIndex(&tableRecord, indexRecord.at(i), commonTable->m_TableInfo[tableLine].iMemRows*5, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				#else
				index = new CommonMemLenIndex(&tableRecord, indexRecord[i], commonTable->m_TableInfo[tableLine].iMemRows*5, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				#endif
				memIndexs.push_back(index);
				break;

		}
	}

}

void CommonMemTableDefine::queryTableRecord(const char *queryCondition,int id)
{
	char *tempString;
	tempString=(char *)queryCondition;
	char tempsub[1001];
	vector<mysize_t> ret;
	int tempid = id;
	for ( int i=0;i<indexID.size(); i++ )
	{
		if ( indexID[i] == tempid )
		{
			RecordStruct* tempStruct = indexRecord[i];
			int kk = tempStruct->columnList.size();
			for ( int j=0;j<tempStruct->columnList.size();j++ )
			{
				int t1=0;
				while (*tempString != 44 && *tempString !=0)
				{
					tempsub[t1++]=*tempString;
					tempString++;
				}
				tempsub[t1]=0;
				tempString++;

				int orderID = tempStruct->columnList[j].orderID;
				ColumnTypeJS columnType = tempStruct->columnList[j].columnType;

				switch(columnType) {
				case INT32 :
					int buf32 ;
					buf32= atoi(tempsub);
					tableRecord.setInt32(recordBuffer, orderID, buf32);
					break;
				case INT64 :
					long long buf64 ;
					buf64 = atol(tempsub);
					tableRecord.setInt64(recordBuffer, orderID, buf64);
					break;
				case CHAR :
					tableRecord.setString(recordBuffer, orderID, tempsub);
					break;
				}

			}
			
			memIndexs[i]->queryIndex(recordBuffer,ret);
			break;
		}
	}
	
	theJSLog<<"找到"<<ret.size()<<"条记录"<<endi;
	for ( int j=0; j< ret.size() ; j++ )
	{
		printRecordToString(ret[j]);
	}

}

void CommonMemTableDefine::printRecordToString(mysize_t nodeId)
{
	byte_t* rowP = memTable->getRowPointer(nodeId);
	char outstring[STRING_MAX_SIZE+1];
	memset(outstring,0,sizeof(outstring));
	for ( int j=0;j<tableRecord.columnList.size();j++ )
	{
		#ifndef _LINUX_ACC
		switch(tableRecord.columnList.at(j).columnType) {
		#else
		switch(tableRecord.columnList[j].columnType) {
		#endif
		case INT32 :
			long buf32;
			buf32=tableRecord.getRealInt32(rowP, j);
			#ifndef _LINUX_ACC
			sprintf(outstring,"%s%s=%d,",outstring,tableRecord.columnList.at(j).columnName.c_str(),buf32);
			#else
			sprintf(outstring,"%s%s=%d,",outstring,tableRecord.columnList[j].columnName.c_str(),buf32);
			#endif
			break;
		case INT64 :
			long long buf64;
			buf64=tableRecord.getRealInt64(rowP, j);
			#ifndef _LINUX_ACC
			sprintf(outstring,"%s%s=%ld,",outstring,tableRecord.columnList.at(j).columnName.c_str(),buf64);
			#else
			sprintf(outstring,"%s%s=%ld,",outstring,tableRecord.columnList[j].columnName.c_str(),buf64);
			#endif
			break;
		case CHAR :
			char c[STRING_MAX_SIZE];
			tableRecord.getRealString(rowP, j, c, sizeof(c));
			#ifndef _LINUX_ACC
			sprintf(outstring,"%s%s=%s,",outstring,tableRecord.columnList.at(j).columnName.c_str(),c);
			#else
			sprintf(outstring,"%s%s=%s,",outstring,tableRecord.columnList[j].columnName.c_str(),c);
			#endif
			break;
		}
	}
	theJSLog<<"RECORD IS :"<<outstring<<endi;
 // }

}

int CommonMemTableDefine::getStartCol() const
{
	return tableRecord.getStartCol();
}

int CommonMemTableDefine::getEndCol() const
{
	return tableRecord.getEndCol();
}


/*
void CommonMemTableDefine::scanTable() {
	// make select update table sql
	// for each result row
	//    if
	string updateIdSql = makeSelectUpdateIDSQL();
	CBindSQL bs(DBConn);
	bs.Open(updateIdSql);
	long updateId;
	string updateAction;
	int count;
	while (bs>>updateId>>updateAction>>count) {
	//while ((bs>>updateId)>>updateAction) {
		CBindSQL bs2(DBConn);
		if (updateAction == "UPDATE") {
			if (count == 2) {
				bs2.Open(makeSelectUpdateSQL());
				bs2<<updateId<<"UPDATE_NEW";
				makeRecordBuffer(&bs2, recordBuffer);
				bs2.Open(makeSelectUpdateSQL());
				bs2<<updateId<<"UPDATE_OLD";
				makeRecordBuffer(&bs2, recordBuffer2);
				updateRecord(recordBuffer, recordBuffer2);
			} else {
				if (getUpdateType(updateId) == "UPDATE_NEW") {
					bs2.Open(makeSelectUpdateSQL());
					bs2<<updateId<<"UPDATE_NEW";
					makeRecordBuffer(&bs2, recordBuffer);
					insertRecord(recordBuffer);
				} else {
					bs2.Open(makeSelectUpdateSQL());
					bs2<<updateId<<"UPDATE_OLD";
					makeRecordBuffer(&bs2, recordBuffer);
					deleteRecord(recordBuffer);
				}
			}
		} else if (updateAction == "INSERT") {
			bs2.Open(makeSelectUpdateSQL());
			bs2<<updateId<<updateAction;
			makeRecordBuffer(&bs2, recordBuffer);
			insertRecord(recordBuffer);
		} else if (updateAction == "DELETE") {
			bs2.Open(makeSelectUpdateSQL());
			bs2<<updateId<<updateAction;
			makeRecordBuffer(&bs2, recordBuffer);
			deleteRecord(recordBuffer);
		}
		// TODO update update table
	}

}
*/

/*
string CommonMemTableDefine::makeSelectUpdateSQL() {
	string ret = "SELECT ";
	const vector<ColumnStruct>* columns = tableRecord.getColumnList();
	for (int i = 0; i < columns->size(); i++) {
		ret += columns->at(i).columnName;
		if (i != columns->size() - 1) {
			ret += ", ";
		}
	}

	ret += " FROM " + updateTableName;
	ret += " WHERE UPDATE_ID = :UPDATE_ID AND UPDATE_ACTION = :UPDATE_ACTION";
	return ret;
}
*/

/*
void CommonMemTableDefine::makeRecordBuffer(CBindSQL* bs, byte_t* buffer) {
	const vector<ColumnStruct>* columns = tableRecord.getColumnList();
	for (int i = 0; i < columns->size(); i++) {
		switch(columns->at(i).columnType) {
		case INT32 :
			long buf32;
			if (!((*bs)>>buf32)) {
				// TODO throw exception
			}
			tableRecord.setRealInt32(buffer, i, buf32);
			break;
		case INT64 :
			long long buf64;

			if (!((*bs)>>buf64)) {
				// TODO throw exception
			}
			tableRecord.setRealInt64(buffer, i, buf64);

			break;
		case CHAR :
			string bufs;
			if (!((*bs)>>bufs)) {
				// TODO throw exception
			}
			tableRecord.setRealString(buffer, i, bufs.c_str());
			break;


		}
	}
}
*/


/*
void CommonMemTableDefine::deleteRecord(const byte_t* record) {
	assert(memIndexs.size() > 0);
	byte_t recordBuffer[RECORD_MAX];
	byte_t keyBuffer[RECORD_MAX];
	tableRecord.copyTo(keyBuffer, &indexRecord[0], record);
	mysize_t recordPos = memIndexs[0]->queryUnique(keyBuffer);

	if (recordPos == -1) {
		return;
	} else {
		for (int i = 0; i < memIndexs.size(); i++) {
			tableRecord.copyTo(keyBuffer, &indexRecord[i], record);
			memIndexs[i]->deleteIndex(keyBuffer, recordPos);
		}
		memTable->deleteRow(recordPos);
	}
}
*/

/*
string CommonMemTableDefine::getUpdateType(long long update_id) {
	string ret;
	string sql = "SELECT \
					UPDATE_ACTION \
					FROM ";
	sql += updateTableName;
	sql += " WHERE \
				UPDATE_ID = :UPDATE_ID";
	if (sqlWhereString.size() != 0) {
		sql += "( " + sqlWhereString + ")";
	}

	CBindSQL bs(DBConn);
	bs.Open(sql);
	bs>>ret;
	return ret;
}
*/

/*
string CommonMemTableDefine::makeSelectUpdateIDSQL() {
	string ret = "SELECT \
					UPDATE_ID, substr(UPDATE_ACTION, 1, 5), COUNT(*) \
					FROM ";
	ret += updateTableName;
	ret += " WHERE \
				UPDATE_STAT = 'W'";
	if (sqlWhereString.size() != 0) {
		ret += "( " + sqlWhereString + ")";
	}
	ret += " GROUP BY UPDATE_ID, substr(UPDATE_ACTION, 1, 5)";
	theJSLog<<"sqlStr="<<ret<<"=,"<<__FILE__<<","<<__LINE__<<endd;
}
*/

