#ifndef __COMMON_MEM_INDEX_H__
#define __COMMON_MEM_INDEX_H__
#include "CommonMemRecord.h"
#include "CommonMemTypes.h"
#include "CommonMemTable.h"
#include "BlockAllocator.h"
#include "RecordList.h"



	enum IndexType {
		UNIQUE, NON_UNIQUE, LONGIST
	};
	class CommonMemIndex {
	public:
		//CommonMemIndex(const RecordStruct* recordStruct, const RecordStruct* indexStruct/*, mysize_t size, key_t shm_key*/);
		CommonMemIndex();
		// 插入索引节点，record为索引对应的记录，record_pos是该记录的下标
		virtual void create(mysize_t allocate_size, key_t shm_key) = 0;
		virtual mysize_t insertIndex(const byte_t* record, mysize_t record_pos) = 0;
		// 删除索引节点，record为索引对应的记录，record_pos是该记录的下标
		//virtual mysize_t deleteIndex(const byte_t* record, mysize_t record_pos) = 0 ;
		// 更新索引节点，record为索引对应的记录，record_pos是该记录的下标
		//virtual mysize_t updateIndex(const byte_t* record, mysize_t record_pos) = 0;
		virtual void queryIndex(const byte_t* key,vector<mysize_t> &nodeID) = 0;
		// 查询索引，key为索引值，table为共享内存表对象，recordList为输出的记录
		virtual void queryIndex(const byte_t* key, const CommonMemTable* table, RecordList* recordList) = 0;
		// 清空共享内存
		//virtual void clear();
		virtual void attach(mysize_t allocate_size, int shid)=0;
		// 销毁共享内存
		virtual void destory() = 0;
		// 断开共享内存
		virtual void detach() = 0;
		BlockManager* getBlockManager();
		int getIndexId();
		int indexId;

	protected:
		const RecordStruct* recordStruct;
		const RecordStruct* indexStruct;

		BlockManager blockManager;
		// 对两个key值进行比较，若a>b则返回值>0，a=b则返回值0，a<b返回值<0
		int keyCompare(const byte_t* a, const byte_t* b) const;

	};

#endif
