/*
 * =====================================================================================
 *
 *       Filename:  CommonMemClient.h
 *
 *    Description:  共享内存客户端
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

#ifndef __COMMON_MEM_CLIENT_H__
#define __COMMON_MEM_CLIENT_H__
#include <vector>
#include <map>
#include "CommonMemRecord.h"
#include "CommonMemTable.h"
#include "CommonMemIndex.h"
#include "CommonMemLenIndex.h"
#include "CommonMemAVLIndex.h"
#include "C_NRWLock.h"
#include "S_CommonTable.h"
#include "CF_CommemClient.h"
#include "es/util/StringUtil.h"

//#include "CommonKeyValue.h"


using std::vector;
using std::map;

extern S_MemManager *commonTable;
extern int m_ishmidcommon;

	class CommonMemClient : public BaseCommemClient {
	public:
		CommonMemClient();
		CommonMemClient(const string& SERVER_ID,const string& mem_name, int version_id ,char *iniPath);
		void init(const string& SERVER_ID,const string& mem_name, int version_id ,char *iniPath);
		void  reBind();
		void query(const byte_t* key,vector<mysize_t> &nodeID,int indexid);
		void printRecordToString(mysize_t nodeId);
		void detach();
		~CommonMemClient();
		void printTableRecord(int recordCount);
		void printCommonInfo();
		bool isTableChange();
		void queryTableRecord(const char *queryCondition,int indexid,vector<mysize_t> &nodeID);
		void getString(int col_id,char *ret,int col_len,mysize_t nodeID);
		void getString(const string& name,char *ret,int col_len,mysize_t nodeID);
		int getStartCol() const;
		int getEndCol() const;
		
	public:
		RecordStruct tableRecord;
		byte_t* recordBuffer;
		int bufferLen;
	protected:
		int getTableConfigIdFromDB(const string& SERVER_ID,const string& table_name, int version);
		string getRealTableNameFromDB(int tableConfigId);
		string getUpdateTableNameFromDB(int tableConfigId);
		string getWhereConditionFromDB(int tableConfigId);
		void readTableStructFromDB(int tableConfigId);
		void readIndexStructFromDB(int tableConfigId);
		mysize_t getCapacityFromDB(const string& table_name, int version);
		void getSHMkeys(const string &server_id,const string& tablename, int version, const string& key_filename);
		vector<string> getStrings(const string &str, int isUpper);
		string makeSelectAllSQL();
		void insertRecord(const byte_t* record);
		//void deleteRecord(const byte_t* record);
		//void updateRecord(const byte_t* newRecord, const byte_t* oldRecord);
		//string makeSelectUpdateSQL();
		
		void loadData();

		const static int MAX_INDEX_NUM = 9;

		string sqlWhereString;
		string realTableName;
		int tableConfigId;
		//表记录数容量
		mysize_t capacity;
		/*
		 * tablename and version
		 */
		string serverId;
		string memName;
		int version;
		string shm_path;
		string sem_path;
		/*
		 * Table
		 */
		//S_MemManager *commonTable;
		//int m_ishmidcommon;
		int tableLine;
		CommonMemTable* memTable;
		int m_memid;
		/*
		 * Indexs
		 */
		vector<IndexType> indexTypes;
		vector<int> indexID;
		vector<RecordStruct*> indexRecord;
		vector<key_t> indexSHMKeys;
		vector<CommonMemIndex*> memIndexs;
		C_NRWLock m_RWLock;
		//map<string, int> indexChoser;
		/*
		 * Functions
		 */
		//void makeRecordBuffer(CBindSQL* bs, byte_t* buffer);
		//string makeSelectUpdateIDSQL();
		//string getUpdateType(long long id);
	};



#endif
