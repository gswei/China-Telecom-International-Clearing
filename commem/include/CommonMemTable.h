#ifndef __COMMON_MEM_TABLE_H__
#define __COMMON_MEM_TABLE_H__
#include "CommonMemRecord.h"
#include "CommonMemTypes.h"
#include "BlockAllocator.h"
//#include "ReadWriteLock.h"

struct RecordHeader {
	mysize_t nextNode; // 下一结点的下标
	mysize_t preNode; // 前一节点的下标
	//ReadWriteLock rwLock; // 读写锁
};


	class CommonMemTable {
	private:
		const RecordStruct* recordStruct;
		BlockManager blockManager;
		

		int commonTable_rowid;
		void initRow(mysize_t row_index);

	public:
		CommonMemTable(const RecordStruct* recordStruct, mysize_t record_number, key_t shm_key);
		CommonMemTable(const RecordStruct* rs, mysize_t record_number, int shid,int kk);
		void attach(mysize_t record_number, int shid);
		mysize_t insertRow(const byte_t* row);
		void deleteRow(mysize_t rowIndex);
		//void updateRow(mysize_t rowIndex, const byte_t* row);
		//void clear();
		void destory();
		void detach();
		void create(mysize_t record_number, key_t shm_key);
		//void getReadLock(mysize_t rowIndex);
		//void releaseReadLock(mysize_t rowIndex);
		RecordHeader* getHeaderPointer(mysize_t rowIndex);
		byte_t* getRowPointer(mysize_t rowIndex);
		const byte_t* getRow(mysize_t rowIndex) const;
		int getRecordSize() const;
		BlockManager* getBlockManager();
	};


#endif
