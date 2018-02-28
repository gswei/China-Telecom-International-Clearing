#ifndef __COMMON_MEM_LEN_INDEX_H__
#define __COMMON_MEM_LEN_INDEX_H__
#include "CommonMemTypes.h"
#include "BlockAllocator.h"
#include "CommonMemRecord.h"
#include "CommonMemIndex.h"


	class CommonMemLenIndex : public CommonMemIndex {
	public:
		CommonMemLenIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key, int idxid);
		CommonMemLenIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, int shid, int idxid, int kk);
		mysize_t insertIndex(const byte_t* record, mysize_t record_pos);
		mysize_t deleteIndex(const byte_t* record, mysize_t record_pos);
		void queryIndex(const byte_t* key,vector<mysize_t> &nodeID);
		void queryIndex(const byte_t* key, const CommonMemTable* table, RecordList* recordList);
		const BlockManager* getBlockManager() const;
		void dumpIndexToScreen() const;
		void create(mysize_t allocate_size, key_t shm_key);
		~CommonMemLenIndex();
		void attach(mysize_t allocate_size, int shid);
		// 销毁共享内存
		void destory();
		// 断开共享内存
		void detach();
	protected:
		//BlockManager blockManager;
		//const RecordStruct* indexStruct;
		//const RecordStruct* recordStruct;
		//vector<String> indexColumnNames;
		struct LenIndexHeader {
			//mysize_t p_num[62];
			//modi by sunhua at 20110218
			mysize_t p_num[36];
			mysize_t nextlink;		//下一个索引记录的下标
			mysize_t recordIndex; // 对应的记录的下标
			//ReadWriteLock rwLock; // 读写锁
		};

		int initNode(mysize_t node_id, mysize_t record_idx, const byte_t* key);
		int initNullNode(mysize_t node_id ); 

		void printNode(mysize_t node);
	private:
		//mysize_t findIndexPos(const byte_t* key);
		LenIndexHeader* getHeaderPointer(mysize_t node_id);
		byte_t* getKeyPointer(mysize_t node_id);
		//void dumpNodeInMid(mysize_t node);
		byte_t* keyBuffer;
	};

#endif
