 /*
	* =====================================================================================
	*
	*		Filename: CommonMemClient.cpp
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
#include "CommonMemClient.h"

S_MemManager *commonTable=NULL;
int m_ishmidcommon=-1;

CommonMemClient::CommonMemClient(const string& SERVER_ID,const string& mem_name, int version_id ,char *inifile)
{
	serverId=SERVER_ID;
	memName=mem_name;
	version=version_id;
	shm_path=inifile;
	shm_path+="zhjs.ini";
	sem_path=inifile;
	sem_path+="service.ini";	
	// TODO get tableConfigId by table_name and version
	tableConfigId = getTableConfigIdFromDB(serverId,memName, version);
	theLog<<"tableConfigId="<<tableConfigId<<","<<__FILE__<<","<<__LINE__<<endd;
	realTableName = getRealTableNameFromDB(tableConfigId);
	theLog<<"realTableName="<<realTableName.c_str()<<","<<__FILE__<<","<<__LINE__<<endd;
	
	char eMessage[512];
	key_t commonkey;
	errno=0;
	//计算共享信息区的IPC键		
	string common_path = inifile;
	common_path +="commem/0";
	theLog<<"common_path="<<common_path.c_str()<<"="<<endi;
	//commonkey=ftok(common_path.c_str(),255);

	//核心参数获取
	std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.commem.commemkeyvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "获取共享内存固定key 值" );
		commem_keyvalue = "91671000";
	}
	int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );
	
	commonkey = commenKeyValue;
	if (commonkey==-1)
	{
		sprintf(eMessage,"链接共享内存公共区出错，ftok函数返回异常,commonkey=%d,errno=%d",commonkey,errno);
		throw CException(999,eMessage,__FILE__,__LINE__);	
	}
	theLog<<"共享内存公共区 commonkey="<<commonkey<<"="<<endd;
	
  errno = 0;
	if ( m_ishmidcommon == -1 )
	{
		m_ishmidcommon = shmget(commonkey, sizeof(S_MemManager), IPC_CREAT|0666);		
		if (m_ishmidcommon==-1)
		{
			sprintf(eMessage,"链接共享内存公共区出错，shmget函数返回异常,m_ishmidcommon=%d,errno=%d",m_ishmidcommon,errno);
			throw CException(999,eMessage,__FILE__,__LINE__);	
		}
		theLog<<"共享内存公共区 shmget后ishmidcommon="<<m_ishmidcommon<<"="<<endd;
  	   		
		errno = 0;
		//连接刚建立起来的共享信息区的内存,返回首地址
		commonTable = (S_MemManager *)shmat(m_ishmidcommon, NULL, 0);
    
		if (commonTable == (S_MemManager *) -1)
		{			
			sprintf(eMessage,"链接共享内存公共区出错，commonTable=%d,errno=%d",commonTable,errno);
			throw CException(999,eMessage,__FILE__,__LINE__);	
		}
	}
	m_RWLock.InitLock(commonTable->memlock,NULL);
	readTableStructFromDB(tableConfigId);
	readIndexStructFromDB(tableConfigId);
				
}

CommonMemClient::CommonMemClient()
{
}

void CommonMemClient::init(const string& SERVER_ID,const string& mem_name, int version_id ,char *inifile)
{
	serverId=SERVER_ID;
	memName=mem_name;
	version=version_id;
	shm_path=inifile;
	shm_path+="zhjs.ini";
	sem_path=inifile;
	sem_path+="service.ini";	
	// TODO get tableConfigId by table_name and version
	tableConfigId = getTableConfigIdFromDB(serverId,memName, version);
	//theLog<<"tableConfigId="<<tableConfigId<<","<<__FILE__<<","<<__LINE__<<endd;
	realTableName = getRealTableNameFromDB(tableConfigId);
	//theLog<<"realTableName="<<realTableName.c_str()<<","<<__FILE__<<","<<__LINE__<<endd;
	
	char eMessage[512];
	key_t commonkey;
	errno=0;

	//计算共享信息区的IPC键
	//commonkey=ftok(shm_path.c_str(),0);

	//核心参数获取
	std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.commem.commemkeyvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "获取共享内存固定key 值" );
		commem_keyvalue = "91671000";
	}
	int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );
	
	commonkey=commenKeyValue;
	if (commonkey==-1)
	{
		sprintf(eMessage,"链接共享内存公共区出错，ftok函数返回异常,commonkey=%d,errno=%d",commonkey,errno);
		throw CException(999,eMessage,__FILE__,__LINE__);	
	}
	//theLog<<"共享内存公共区 commonkey="<<commonkey<<"="<<endd;
	
  errno = 0;
	if ( m_ishmidcommon == -1 )
	{
		m_ishmidcommon = shmget(commonkey, sizeof(S_MemManager), IPC_CREAT|0666);		
		if (m_ishmidcommon==-1)
		{
			sprintf(eMessage,"链接共享内存公共区出错，shmget函数返回异常,m_ishmidcommon=%d,errno=%d",m_ishmidcommon,errno);
			throw CException(999,eMessage,__FILE__,__LINE__);	
		}
		//theLog<<"共享内存公共区 shmget后ishmidcommon="<<m_ishmidcommon<<"="<<endd;
  	   		
		errno = 0;
		//连接刚建立起来的共享信息区的内存,返回首地址
		commonTable = (S_MemManager *)shmat(m_ishmidcommon, NULL, 0);
    
		if (commonTable == (S_MemManager *) -1)
		{			
			sprintf(eMessage,"链接共享内存公共区出错，commonTable=%d,errno=%d",commonTable,errno);
			throw CException(999,eMessage,__FILE__,__LINE__);	
		}
	}
	m_RWLock.InitLock(commonTable->memlock,NULL);
	readTableStructFromDB(tableConfigId);
	readIndexStructFromDB(tableConfigId);
				
}

int CommonMemClient::getTableConfigIdFromDB(const string& SERVER_ID,const string& mem_name, int version) 
{
	char eMessage[512];
	int ret;
	string sql = "SELECT \
		C.MEM_DEFINE_ID \
		FROM C_COMMEM_MEM_VERSION C \
		WHERE C.SERVER_ID = :SERVER_ID AND C.MEM_NAME = :MEM_NAME AND C.VERSION_ID = :VERSION";
	theLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	try
	{
		CBindSQL bs(DBConn);
		bs.Open(sql);
		bs<<SERVER_ID<<mem_name<<version;
		bs>>ret; 
		bs.Close();
	} catch ( CException e )
	{
		sprintf(eMessage,"执行sql出错,error_code=%d,sqlStr=%s",e.GetAppError(),sql.c_str());
		e.PushStack(999,eMessage,__FILE__, __LINE__);
		throw e;
	}
	return ret;
}

string CommonMemClient::getRealTableNameFromDB(int tableConfigId) {
	char eMessage[512];
	string ret;
	string sql = "SELECT \
		C.TABLE_NAME \
		FROM C_COMMEM_TABLE_DEF C \
		where C.MEM_DEFINE_ID = :MEM_DEFINE_ID";
	theLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	try
	{
		CBindSQL bs(DBConn);
		bs.Open(sql);
		bs<<tableConfigId;
		bs>>ret;
		bs.Close();
	} catch ( CException e )
	{
		sprintf(eMessage,"执行sql出错,error_code=%d,sqlStr=%s",e.GetAppError(),sql.c_str());
		e.PushStack(999,eMessage,__FILE__, __LINE__);
		throw e;
	}
	return ret;


}

void CommonMemClient::readTableStructFromDB(int tableConfigId) {
	char eMessage[512];
	string sql = "SELECT \
		C.COLUMN_ID,C.COLUMN_NAME, C.COLUMN_TYPE,C.TIME_COL, \
		C.COLUMN_SIZE \
		FROM C_COMMEM_COLUMN_DEF C  \
		WHERE C.MEM_DEFINE_ID = :MEM_DEFINE_ID \
		ORDER BY C.COLUMN_ID";
	theLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	try
	{
		CBindSQL bs(DBConn);
		bs.Open(sql);
		bs<<tableConfigId;
		char columnN[101];
		string columnName, columnType, timeCol;
		int columnSize;
		int columnID;
		while (bs>>columnID>>columnN>>columnType>>timeCol>>columnSize) 
		{
			toUpper(columnN);
			columnName=columnN;
			if (columnType == "CHAR") {
				tableRecord.addColumn(columnName, CHAR, columnID, timeCol, columnSize);
			} else if (columnType == "INT64") {
				tableRecord.addColumn(columnName, INT64, columnID, timeCol, columnSize);
			} else if (columnType == "INT32") {
				tableRecord.addColumn(columnName, INT32, columnID, timeCol, columnSize);
			} else {
				sprintf(eMessage,"字段类型定义错误,columnN=%s,columnType=%s",columnName.c_str(),columnType.c_str());
				throw CException(999,eMessage,__FILE__,__LINE__);	
			}
			theLog<<"columnName="<<columnName.c_str()<<",columnType="<<columnType.c_str()<<",columnSize="<<columnSize<<","<<__FILE__<<","<<__LINE__<<endd;
		} 
		bs.Close();
		recordBuffer = new byte_t[tableRecord.getRecordSize()+1];
		memset(recordBuffer,0,sizeof(tableRecord.getRecordSize()+1));
		theLog<<"表"<<realTableName.c_str()<<"记录大小="<<tableRecord.getRecordSize()<<","<<__FILE__<<","<<__LINE__<<endd;
	} catch ( CException e )
	{
		sprintf(eMessage,"执行sql出错,error_code=%d,sqlStr=%s",e.GetAppError(),sql.c_str());
		e.PushStack(999,eMessage,__FILE__, __LINE__);
		throw e;
	}
	
		m_memid=0;
		tableLine=-1;
		m_RWLock.GetReadLock(commonTable->memlock);
		for( int i=0;i<commonTable->iTotalTable;i++ )
		{
			if ( memName == commonTable->m_TableInfo[i].MemName && version==commonTable->m_TableInfo[i].version )
			{
				m_memid = commonTable->m_TableInfo[i].ishmid;
				tableLine = i;
				theLog<<"在公共区找到内存块["<<memName.c_str()<<"],ishmid="<<m_memid<<",tableLine="<<tableLine<<", tablerow="<<commonTable->m_TableInfo[tableLine].iMemRows<<","<<__FILE__<<","<<__LINE__<<endd;
				memTable = new CommonMemTable(&tableRecord,commonTable->m_TableInfo[tableLine].iMemRows,m_memid,0);
				break;
			}
		}
		m_RWLock.UnLock(commonTable->memlock);
		if (tableLine == -1)
		{
			sprintf(eMessage,"链接共享内存公共区出错，找不到共享内存区[%s,%d]",memName.c_str(),version);
			throw CException(999,eMessage,__FILE__,__LINE__);	
		}

	return ;
}

void CommonMemClient::readIndexStructFromDB(int tableConfigId) {
	char eMessage[512];
	string sql = "SELECT \
		C.INDEX_ID, C.INDEX_TYPE, C.INDEX_COLUMNS \
		FROM C_COMMEM_INDEX_DEF C \
		WHERE C.MEM_DEFINE_ID = :TABLE_DEFINE_ID \
		ORDER BY C.INDEX_ID";
	theLog<<"sqlStr="<<sql<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	try
	{
		CBindSQL bs(DBConn);
		bs.Open(sql);
		bs<<tableConfigId;
		int indexId;
		string indexType, indexColumns;
		CommonMemIndex* index;
		RecordStruct *rs;
		char columnN[1001];
		while (bs>>indexId>>indexType>>columnN) 
		{
			rs = new RecordStruct();
			toUpper(columnN);
			indexColumns=columnN;
			theLog<<"index indexId="<<indexId<<",indexType="<<indexType<<",columnName="<<indexColumns<<","<<__FILE__<<","<<__LINE__<<endd;
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
					throw CException(999,eMessage,__FILE__,__LINE__);	
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
					throw CException(999,eMessage,__FILE__,__LINE__);	
				}
			} else {
					sprintf(eMessage,"索引类型定义错误,indexType=%s",indexType.c_str());
					throw CException(999,eMessage,__FILE__,__LINE__);	
			}
 
 			m_RWLock.GetReadLock(commonTable->memlock);
 			switch(iType) {
			case UNIQUE :
				//CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key);
				index = new CommonMemAVLIndex(&tableRecord, rs, commonTable->m_TableInfo[tableLine].iMemRows, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				memIndexs.push_back(index);
				break;
			case NON_UNIQUE :
				//TODO
				//CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key);
				index = new CommonMemAVLIndex(&tableRecord, rs, commonTable->m_TableInfo[tableLine].iMemRows, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				memIndexs.push_back(index);
				break;
			case LONGIST :
				// TODO
				int idx_size;
				if ( commonTable->m_TableInfo[tableLine].iMemRows < 1001 ) idx_size=5000;
				else idx_size=commonTable->m_TableInfo[tableLine].iMemRows*5;
				index = new CommonMemLenIndex(&tableRecord, rs, idx_size, commonTable->m_TableInfo[tableLine].iIdxshmid[indexId],indexId,0);
				memIndexs.push_back(index);
				break;
			}
 			m_RWLock.UnLock(commonTable->memlock);
			indexTypes.push_back(iType);
			indexID.push_back(indexId);
			indexRecord.push_back(rs);
			//indexChoser[orderColumnNames] = indexRecord.size() - 1;
			theLog<<"索引"<<indexId<<"记录大小="<<rs->getRecordSize()<<","<<__FILE__<<","<<__LINE__<<endd;

		}
		bs.Close();
	} catch ( CException e )
	{
		m_RWLock.UnLock(commonTable->memlock);
		sprintf(eMessage,"执行sql出错,error_code=%d,sqlStr=%s",e.GetAppError(),sql.c_str());
		e.PushStack(999,eMessage,__FILE__, __LINE__);
		throw e;
	}
	return ;
}

void CommonMemClient::reBind()
{
	m_RWLock.GetReadLock(commonTable->memlock);
	if ( m_memid != commonTable->m_TableInfo[tableLine].ishmid )
	{
		memTable->detach();
		m_memid=commonTable->m_TableInfo[tableLine].ishmid;
		memTable->attach(commonTable->m_TableInfo[tableLine].iMemRows,commonTable->m_TableInfo[tableLine].ishmid);
		for ( int i=0;i<memIndexs.size();i++ )
		{
			#ifndef _LINUX_ACC
			memIndexs.at(i)->detach();
			#else
			memIndexs[i]->detach();
			#endif
			int j;
			switch(indexTypes[i]) {
			case UNIQUE :
				j=indexID[i];
				#ifndef _LINUX_ACC
				memIndexs.at(i)->attach(commonTable->m_TableInfo[tableLine].iMemRows,commonTable->m_TableInfo[tableLine].iIdxshmid[j]);
				#else
				memIndexs[i]->attach(commonTable->m_TableInfo[tableLine].iMemRows,commonTable->m_TableInfo[tableLine].iIdxshmid[j]);
				#endif
				break;
			case NON_UNIQUE :
				j=indexID[i];
				#ifndef _LINUX_ACC
				memIndexs.at(i)->attach(commonTable->m_TableInfo[tableLine].iMemRows,commonTable->m_TableInfo[tableLine].iIdxshmid[j]);
				#else
				memIndexs[i]->attach(commonTable->m_TableInfo[tableLine].iMemRows,commonTable->m_TableInfo[tableLine].iIdxshmid[j]);
				#endif
				break;
			case LONGIST :
				j=indexID[i];
				int idx_size;
				int capacityCreate=commonTable->m_TableInfo[tableLine].iMemRows;
				if ( capacityCreate < 101 ) idx_size=500;
				else if ( capacityCreate > 100 && capacityCreate < 1001 ) idx_size=4000;
				else if ( capacityCreate > 1000 && capacityCreate <10001 ) idx_size=30000;
				else idx_size=capacityCreate*2;
				#ifndef _LINUX_ACC
				memIndexs.at(i)->attach(idx_size,commonTable->m_TableInfo[tableLine].iIdxshmid[j]);
				#else
				memIndexs[i]->attach(idx_size,commonTable->m_TableInfo[tableLine].iIdxshmid[j]);
				#endif
				break;
			}
		}
	}
	m_RWLock.UnLock(commonTable->memlock);
	return ;
}

void CommonMemClient::detach()
{
	memTable->detach();
	for ( int i=0;i<memIndexs.size();i++ )
	{
		#ifndef _LINUX_ACC
		memIndexs.at(i)->detach();
		#else
		memIndexs[i]->detach();
		#endif
	}
	return;
}

CommonMemClient::~CommonMemClient()
{
	detach();
	if (commonTable != NULL ) m_RWLock.DestroyLock(commonTable->memlock);
}

void CommonMemClient::printTableRecord(int recordCount)
{
	int rCount;
	byte_t rowP[STRING_MAX_SIZE];
	char outstring[STRING_MAX_SIZE+1];
	m_RWLock.GetReadLock(commonTable->memlock);
	if ( recordCount<commonTable->m_TableInfo[tableLine].iRealRows ) rCount=recordCount;
	else rCount=commonTable->m_TableInfo[tableLine].iRealRows;
	
	RecordHeader* header;
	mysize_t headerCnt=memTable->getBlockManager()->getLinkEnd();
	header = memTable->getHeaderPointer(headerCnt);
	
	for (int i=0;i<rCount;i++ )
	{
		memset(rowP,0,sizeof(rowP));		
		byte_t* recordP=memTable->getRowPointer(headerCnt);
		memcpy(rowP,recordP,tableRecord.getRecordSize());
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
		theLog<<"RECORD "<<i+1<<":"<<outstring<<endi;
		headerCnt=header->preNode;
		if ( headerCnt != -1) header = memTable->getHeaderPointer(headerCnt);
	}
	m_RWLock.UnLock(commonTable->memlock);
	return ;
}

vector<string> CommonMemClient::getStrings(const string &str,int isUpper) 
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

void CommonMemClient::query(const byte_t* key, vector<mysize_t> &nodeID, int indexid )
{
	vector<mysize_t> ret;
	for ( int i=0;i<indexID.size(); i++ )
	{
		if ( indexID[i] == indexid )
		{
			memIndexs[i]->queryIndex(key,nodeID);
			break;
		}
	}
}

void CommonMemClient::getString(int col_id,char *ret,int col_len,mysize_t nodeID)
{
	byte_t* recordP=memTable->getRowPointer(nodeID);
	tableRecord.getString( recordP,col_id,ret,col_len );
}

void CommonMemClient::getString(const string& name,char *ret,int col_len,mysize_t nodeID)
{
	byte_t* recordP=memTable->getRowPointer(nodeID);
	tableRecord.getString( recordP,name,ret,col_len );
}


void CommonMemClient::printRecordToString(mysize_t nodeId)
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
	theLog<<"RECORD IS :"<<outstring<<endi;

}

void CommonMemClient::printCommonInfo()
{
	if ( commonTable == NULL )
	{
		theLog<<"还没连到公共共享内存区"<<__FILE__<<","<<__LINE__<<endi;
		return;
	}
	m_RWLock.GetReadLock(commonTable->memlock);
	theLog<<"共创建="<<commonTable->iTotalTable<<"=个数据表存储区间"<<__FILE__<<","<<__LINE__<<endi;
	for ( int i=0;i<commonTable->iTotalTable; i++ )
	{
		theLog<<"第"<<i+1<<"个数据表["<<commonTable->m_TableInfo[i].MemName<<","<<commonTable->m_TableInfo[i].version<<"],表名["<<commonTable->m_TableInfo[i].chTableName<<"]"<<endi;
		theLog<<"    内存区域大小="<<commonTable->m_TableInfo[i].iMemSize<<",最大记录条数="<<commonTable->m_TableInfo[i].iMemRows<<",实际使用记录条数="<<commonTable->m_TableInfo[i].iRealRows<<endi;
		theLog<<"    表TableKey="<<commonTable->m_TableInfo[i].iTableKey<<",表ishmid="<<commonTable->m_TableInfo[i].ishmid<<endi;
		for ( int j=0;j<MEM_MAX_INDEX;j++)
		{
			if ( commonTable->m_TableInfo[i].iIdxshmid[j] > 0 )
			{
				theLog<<"    索引["<<j<<"]ishmid="<<commonTable->m_TableInfo[i].iIdxshmid[j];
			}
		}
		theLog<<" "<<endi;
	}
	m_RWLock.UnLock(commonTable->memlock);
}

bool CommonMemClient::isTableChange()
{
	m_RWLock.GetReadLock(commonTable->memlock);
	if (commonTable->m_TableInfo[tableLine].ishmid != m_memid) 
	{
		m_RWLock.UnLock(commonTable->memlock);
		return true;
	}
	else {
		m_RWLock.UnLock(commonTable->memlock);
		return false;
	}
	
}

void CommonMemClient::queryTableRecord(const char *queryCondition,int indexid,vector<mysize_t> &nodeID)
{
	char *tempString;
	tempString=(char *)queryCondition;
	char tempsub[1001];
	int tempid = indexid;
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
				ColumnType columnType = tempStruct->columnList[j].columnType;

				switch(columnType) {
				case INT32 :
					int buf32 ;
					buf32= atoi(tempsub);
					tableRecord.setInt32(recordBuffer, orderID, buf32);
					//tableRecord.setRealInt32(recordBuffer, j, buf32); 
					break;
				case INT64 :
					long long buf64 ;
					buf64 = atol(tempsub);
					tableRecord.setInt64(recordBuffer, orderID, buf64);
					//tableRecord.setRealInt64(recordBuffer, j, buf64);
					break;
				case CHAR :
					tableRecord.setString(recordBuffer, orderID, tempsub);
					//tableRecord.setRealString(recordBuffer, j, tempsub);
					break;
				}

			}
			
			memIndexs[i]->queryIndex(recordBuffer,nodeID);
			break;
		}
	}
	
	//theLog<<"找到"<<ret.size()<<"条记录"<<endi;
	//for ( int j=0; j< ret.size() ; j++ )
	//{
	//	printRecordToString(ret[j]);
	//}

}

int CommonMemClient::getStartCol() const
{
	return tableRecord.getStartCol();
}

int CommonMemClient::getEndCol() const
{
	return tableRecord.getEndCol();
}

