/*
 * =====================================================================================
 *
 *       Filename:  CommonMemTableDefine.h
 *
 *    Description:  管理与同步共享内存
 *
 *        Version:  1.0
 *        Created:  2010年04月03日 08时40分48秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhengxx,
 *        Company:  Poson
 *
 * =====================================================================================
 */

#ifndef __COMMON_MEM_TABLE_DEFINE_H__
#define __COMMON_MEM_TABLE_DEFINE_H__
#include <vector>
#include <map>
#include "CommonMemRecord.h"
#include "CommonMemTable.h"
#include "CommonMemIndex.h"
#include "CommonMemLenIndex.h"
#include "CommonMemAVLIndex.h"
#include "C_NRWLock.h"
#include "S_CommonTable.h"
#include "es/util/StringUtil.h"
//#include "CommonKeyValue.h"

#include "psutil.h"


using std::vector;
using std::map;

struct tableinfo
{
	int mem_define_id;
	string table_name;
	string  update_table_name; 
	string where_condition ; 			
	tableinfo():mem_define_id(0),table_name(""),update_table_name(""),where_condition(""){}
};



	class CommonMemTableDefine {
	public:
		int getTableInfo(const string& SERVER_ID,const string& mem_name, int version);
		CommonMemTableDefine();
		CommonMemTableDefine(const string &server_id, const char *table_name, int version_id,const char *key_filename,int site_id,S_MemManager *commonTable,C_NRWLock *m_RWLock); 
		void AttachTable(const string& SERVER_ID,const char *mem_name, int version_id ,const char *iniPath, int tLine, S_MemManager *commonTable,C_NRWLock *m_RWLock);
		void reloadTable( const string &machine_id, const string& mem_name, int version, const int shmid_key,int site_id,S_MemManager *commonTable,C_NRWLock *m_RWLock );
		//void scanTable();
		void destory();
		void detach();
		~CommonMemTableDefine();
		string getMemName() const;
		int getVersion() const;
		void queryTableRecord(const char *queryCondition,int id);
		int getStartCol() const;
		int getEndCol() const;

	protected:
		int getTableConfigIdFromDB(const string& SERVER_ID,const string& table_name, int version);
	//	string getRealTableNameFromDB(int tableConfigId);
	//	string getUpdateTableNameFromDB(int tableConfigId);
	//	string getWhereConditionFromDB(int tableConfigId);
		void readTableStructFromDB(int tableConfigId);
		void readIndexStructFromDB(int tableConfigId);
		mysize_t getCapacityFromDB(const string& table_name, int version);
		void getSHMkeys(const string &server_id,const string& tablename, int version, const string& key_filename);
		vector<string> getStrings(const string &str,int isUpper);
		string makeSelectAllSQL();
		string makeSelectCountSQL();
		void insertRecord(const byte_t* record);
		//void deleteRecord(const byte_t* record);
		//void updateRecord(const byte_t* newRecord, const byte_t* oldRecord);
		//string makeSelectUpdateSQL();
		void printRecordToString(mysize_t nodeId);
		
		int loadData();

		const static int MAX_INDEX_NUM = 9;
		byte_t* recordBuffer;
		int bufferLen;
		byte_t* recordBuffer2;
		string sqlWhereString;
		string realTableName;
		string updateTableName;
		string whereCondition;
		int tableConfigId;
		//表记录数容量
		mysize_t capacity;
		/*
		 * tablename and version
		 */
		string tableName;
		string memName;
		int version;
		/*
		 * Table
		 */
		RecordStruct tableRecord;
		key_t tableSHMKey;
		CommonMemTable* memTable;
		/*
		 * Indexs
		 */
		vector<IndexType> indexTypes;
		vector<int> indexID;
		vector<RecordStruct*> indexRecord;
		vector<key_t> indexSHMKeys;
		vector<CommonMemIndex*> memIndexs;
		DBConnection conn;//数据库连接
		//map<string, int> indexChoser;
		/*
		 * Functions
		 */
		//void makeRecordBuffer(CBindSQL* bs, byte_t* buffer);
		//string makeSelectUpdateIDSQL();
		//string getUpdateType(long long id);
	};



#endif
