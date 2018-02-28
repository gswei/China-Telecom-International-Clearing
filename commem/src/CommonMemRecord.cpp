#include "CommonMemRecord.h"
#include <string.h>
#include <algorithm>


using std::min;
size_t RecordStruct::getRecordSize() const {
	return recordSize;
}


int RecordStruct::getColIndexByName(const string& name) const 
{
	int ret = -1;
	char temp[512];
	strcpy(temp,name.c_str());
	toUpper(temp);
	string Uname = temp;
	map<string, int>::const_iterator it = columnNameIndex.find(Uname);
	if (it != columnNameIndex.end()) {
		ret = it->second;
	}
	//theLog<<"Searching string="<<Uname<<",return="<<ret<<__FILE__<<","<<__LINE__<<endd;
	return ret;
}

int RecordStruct::getColIDByName(const string& name) const 
{
	int ret = -1;
	char temp[512];
	strcpy(temp,name.c_str());
	toUpper(temp);
	string Uname = temp;
	map<string, int>::const_iterator it = columnNameID.find(Uname);
	if (it != columnNameID.end()) {
		ret = it->second;
	}
	//theLog<<"Searching string="<<Uname<<",return="<<ret<<__FILE__<<","<<__LINE__<<endd;
	return ret;
}

int RecordStruct::getColIndexByID(const int index_id) const
{
	int ret = -1;
	map<int, int>::const_iterator it = columnIDIndex.find(index_id);
	if (it != columnIDIndex.end()) {
		ret = it->second;
	}
	return ret;
}

const vector<ColumnStruct>* RecordStruct::getColumnList() const {
	return &columnList;
}

void RecordStruct::addColumn(const string& name, ColumnTypeJS type,int ColId,const string &timeC, int size) {
	ColumnStruct column;
	char temp[512];
	strcpy(temp,name.c_str());
	toUpper(temp);
	string Uname = temp;
	column.columnName = Uname;
	column.columnType = type;
	column.timeCol = timeC;
	switch (type) {
		case INT32 :
			column.columnSize = 4;
			break;
		case INT64 :
			column.columnSize = 8;
			break;
		default:
			column.columnSize = size;
			break;
	}
	//column.columnSize = size;
	recordSize += column.columnSize;
	column.orderID=ColId;
	column.columnId = columnList.size();
	column.offset = lastOffset;
	lastOffset += column.columnSize;
	columnNameIndex[Uname] = column.columnId;
	columnNameID[Uname] = ColId;
	columnIDIndex[ColId] = column.columnId;
	columnList.push_back(column);
}

int RecordStruct::getInt32(const byte_t* record_p, int column_index) const 
{
	int order_Index = getColIndexByID(column_index);
	if ( order_Index == -1 ) return order_Index;
	return getRealInt32(record_p, order_Index);
}

int RecordStruct::getRealInt32(const byte_t* record_p, int order_index) const {
	int ret;
	int offset = columnList[order_index].offset;
	const byte_t* columnP = record_p + offset;
	memcpy(&ret, columnP, columnList[order_index].columnSize);
	return ret;
}

long long RecordStruct::getInt64(const byte_t* record_p, int column_index) const
{
	int order_Index = getColIndexByID(column_index);
	if ( order_Index == -1 ) return (long long)order_Index;
	return getRealInt64(record_p, order_Index);
}

long long RecordStruct::getRealInt64(const byte_t* record_p, int order_index) const {
	long long ret;
	int offset = columnList[order_index].offset;
	const byte_t* columnP = record_p + offset;
	memcpy(&ret, columnP, columnList[order_index].columnSize);
	return ret;
}

void RecordStruct::getString(const byte_t* record_p, int column_index, char* buffer, size_t buffer_size) const
{
	int order_Index = getColIndexByID(column_index);
	if ( order_Index == -1 ) {
		sprintf( buffer,"%d",order_Index );
		return ;
	}
	getRealString(record_p, order_Index, buffer, buffer_size);
	return ;
}

void RecordStruct::getRealString(const byte_t* record_p, int order_Index, char* buffer, size_t buffer_size) const {
	int ret;
	int offset = columnList[order_Index].offset;
	const byte_t* columnP;
	switch(columnList[order_Index].columnType) {
		case INT32 :
			columnP = record_p + offset;
			memcpy(&ret, columnP, columnList[order_Index].columnSize);
			sprintf( buffer,"%ld", ret  );
			break;
		case INT64 :
			long long ret1;
			columnP = record_p + offset;
			memcpy(&ret, columnP, columnList[order_Index].columnSize);
			sprintf( buffer,"%ld", ret  );
			break;
		case CHAR :			
			int copySize = min(buffer_size-1, (size_t)columnList[order_Index].columnSize) ;
			columnP = record_p + offset;
			strncpy(buffer,(char *)columnP,copySize);
			buffer[copySize] = '\0';
			break;
	}	
	return ;	
}

int RecordStruct::getInt32(const byte_t* record_p, const string& name) const {
	char temp[512];
	strcpy(temp,name.c_str());
	toUpper(temp);
	string Uname = temp;

	int order_Index = getColIndexByName(Uname);
	if ( order_Index == -1 ) return order_Index;
	return getInt32(record_p, order_Index);
}

long long RecordStruct::getInt64(const byte_t* record_p, const string& name) const {
	char temp[512];
	strcpy(temp,name.c_str());
	toUpper(temp);
	string Uname = temp;
	int order_Index = getColIndexByName(Uname);
	if ( order_Index == -1 ) return (long long)order_Index;
	return getInt64(record_p, order_Index);
}

void RecordStruct::getString(const byte_t* record_p, const string& name, char* buffer, size_t buffer_size) const {
	char temp[512];
	strcpy(temp,name.c_str());
	toUpper(temp);
	string Uname = temp;
	int colIndex = getColIndexByName(Uname);
	if ( colIndex == -1 )
	{
		sprintf( buffer,"%d",colIndex );
		return ;
	}
	int order_Index = getColIndexByID(colIndex);
	getRealString(record_p, order_Index, buffer, buffer_size);
}

void RecordStruct::copyTo(byte_t* dest, const RecordStruct* dest_struct, const byte_t* source) const {
	const vector<ColumnStruct>* destColumns = dest_struct->getColumnList();
	for (int i = 0; i < destColumns->size(); i++) {
		#ifndef _LINUX_ACC
		string columnName = destColumns->at(i).columnName;
		ColumnTypeJS columnType = destColumns->at(i).columnType;
		int columnSize = destColumns->at(i).columnSize;
		int orderID = destColumns->at(i).orderID;
		#else
		string columnName = (*destColumns)[i].columnName;
		ColumnTypeJS columnType = (*destColumns)[i].columnType;
		int columnSize = (*destColumns)[i].columnSize;
		int orderID = (*destColumns)[i].orderID;
		#endif
		switch(columnType) {
			case INT32 :
				dest_struct->setRealInt32(dest, i, getInt32(source, orderID));
				break;
			case INT64 :
				dest_struct->setRealInt64(dest, i, getInt64(source, orderID));
				break;
			case CHAR :
				char buffer[STRING_MAX_SIZE];
				getString(source, orderID, buffer, STRING_MAX_SIZE);
				dest_struct->setRealString(dest, i, buffer);
				break;
		}
	}
}

void RecordStruct::setInt32(byte_t* record_p, int column_index, int value) const {
	int order_Index = getColIndexByID(column_index);
	if ( order_Index == -1 ) return ;

	int offset = columnList[order_Index].offset;
	byte_t* columnP = record_p + offset;
	memcpy(columnP, &value, columnList[order_Index].columnSize);
}

void RecordStruct::setRealInt32(byte_t* record_p, int order_index, int value) const {
	int offset = columnList[order_index].offset;
	byte_t* columnP = record_p + offset;
	memcpy(columnP, &value, columnList[order_index].columnSize);
}

void RecordStruct::setInt64(byte_t* record_p, int column_index, long long value) const {
	int order_Index = getColIndexByID(column_index);
	if ( order_Index == -1 ) return ;

	int offset = columnList[order_Index].offset;
	byte_t* columnP = record_p + offset;
	memcpy(columnP, &value, columnList[order_Index].columnSize);
}

void RecordStruct::setRealInt64(byte_t* record_p, int order_index, long long value) const {
	int offset = columnList[order_index].offset;
	byte_t* columnP = record_p + offset;
	memcpy(columnP, &value, columnList[order_index].columnSize);
}

void RecordStruct::setString(byte_t* record_p, int column_index, const char* value) const {
	int order_index = getColIndexByID(column_index);
	if ( order_index == -1 ) return ;

	int offset = columnList[order_index].offset;
	int strsize = columnList[order_index].columnSize;
	byte_t* columnP = record_p + offset;
	strncpy((char*)columnP, value, strsize );
}

void RecordStruct::setRealString(byte_t* record_p, int order_index, const char* value) const {
	int offset = columnList[order_index].offset;
	int strsize = columnList[order_index].columnSize;
	byte_t* columnP = record_p + offset;
	strncpy((char*)columnP, value, strsize );
}

RecordStruct::RecordStruct() : lastOffset(0), recordSize(0) 
{
}

int RecordStruct::getStartCol() const
{
	int Colid=-1;
	string cmpString;
	cmpString="S";
	for ( int i=0;i<columnList.size();i++ )
	{
		if (columnList[i].timeCol == cmpString) 
		{
			for (map<int, int>::const_iterator pointer = columnIDIndex.begin();pointer != columnIDIndex.end();pointer++)
			{
				if (pointer->second == columnList[i].columnId ) return pointer->first ;
			}
		}
	}
	return Colid;
}

int RecordStruct::getEndCol() const
{
	int Colid=-1;
	string cmpString;
	cmpString="E";
	for ( int i=0;i<columnList.size();i++ )
	{
		if (columnList[i].timeCol == cmpString) 
		{
			//map<int, int>::iterator pointer;
			for (map<int, int>::const_iterator pointer = columnIDIndex.begin();pointer != columnIDIndex.end();pointer++)
			{
				if (pointer->second == columnList[i].columnId ) return pointer->first ;
			}
		}
	}
	return Colid;
}
