#ifndef __COMMON_MEM_RECORD_H__
#define __COMMON_MEM_RECORD_H__
#include <string>
#include <map>
#include <vector>
#include "CommonMemTypes.h"
#include "CF_Common.h"
#include "CF_CLogger.h"

using std::string;
using std::map;
using std::vector;


#define RECORD_MAX 500
	enum ColumnTypeJS {
		INT32,INT64,CHAR
	};
	struct ColumnStruct {
		int columnId;
		int orderID;
		string columnName;
		ColumnTypeJS columnType;
		string timeCol;
		int columnSize;
		int offset;
	};

	class RecordStruct {
	public:
		vector<ColumnStruct> columnList;
		map<string, int> columnNameIndex;
		map<string, int> columnNameID;
		map<int, int> columnIDIndex;
		int lastOffset;
		int recordSize;
	public:
		RecordStruct();
		int getColIndexByName(const string& name) const;
		int getColIDByName(const string& name) const;
		int getColIndexByID(const int index_id) const;
		void addColumn(const string& name, ColumnTypeJS type,int ColId,const string &timeC, int size = 0);
		int getInt32(const byte_t* record_p, int column_index) const;
		long long getInt64(const byte_t* record_p, int column_index) const;
		void getString(
				const byte_t* record_p,
				int column_index,
				char* buffer,
				size_t buffer_size) const;
		int getInt32(const byte_t* record_p, const string& name) const;
		long long getInt64(const byte_t* record_p, const string& name) const;
		void getString(
				const byte_t* record_p,
				const string& name,
				char* buffer,
				size_t buffer_size) const;
		void setInt32(byte_t* record_p, int column_index, int value) const;
		void setInt64(byte_t* record_p, int column_index, long long value) const;
		void setString(byte_t* record_p, int column_index, const char* value) const;
		void setRealInt32(byte_t* record_p, int order_index, int value) const;
		void setRealInt64(byte_t* record_p, int order_index, long long value) const;
		void setRealString(byte_t* record_p, int order_index, const char* value) const;
		size_t getRecordSize() const;
		const vector<ColumnStruct>* getColumnList() const;
		void copyTo(byte_t* dest, const RecordStruct* dest_struct, const byte_t* source) const;
		int getRealInt32(const byte_t* record_p, int order_index) const;
		long long getRealInt64(const byte_t* record_p, int order_index) const;
		void getRealString(
				const byte_t* record_p,
				int order_index,
				char* buffer,
				size_t buffer_size) const;
		int getStartCol() const;
		int getEndCol() const;

	};


#endif

