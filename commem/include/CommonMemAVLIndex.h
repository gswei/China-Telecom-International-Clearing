#ifndef __COMMON_MEM_AVL_INDEX_H__
#define __COMMON_MEM_AVL_INDEX_H__
#include "CommonMemTypes.h"
#include "BlockAllocator.h"
#include "CommonMemRecord.h"
#include "CommonMemIndex.h"


	class CommonMemAVLIndex : public CommonMemIndex {
	public:
		CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key,int idxid);
		CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, int shid,int idxid,int kk);
		mysize_t insertIndex(const byte_t* record, mysize_t record_pos);
		mysize_t deleteIndex(const byte_t* record, mysize_t record_pos);
		//mysize_t updateIndex(const byte_t* record, mysize_t record_pos);
		void queryIndex(const byte_t* key,vector<mysize_t> &nodeID);
		void queryIndex(const byte_t* key, const CommonMemTable* table, RecordList* recordList);
		const BlockManager* getBlockManager() const;
		void dumpIndexToScreen() const;
		void create(mysize_t allocate_size, key_t shm_key);
		//mysize_t queryUnique(const byte_t* record);
		~CommonMemAVLIndex();
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
		struct AVLIndexHeader {
			mysize_t parent; // 父节点下标
			mysize_t leftChild; // 左子下标
			mysize_t rightChild; // 右子下标
			mysize_t nextlink;//链表下个节点
			mysize_t recordIndex; // 对应的记录的下标
			int			 istreeNode; //是否是AVL树节点
			//ReadWriteLock rwLock; // 读写锁
			int treeHigh; // 树高度
		};

		int initNode(mysize_t node_id, mysize_t record_idx, const byte_t* key);
		int treeNodeHigh(mysize_t node_id);
		mysize_t dirtyInsert(const byte_t* key, mysize_t record_idx);
		void balance(mysize_t leafNode);
		void renewHigh(mysize_t leafNode);
		mysize_t getSuccessorNode(mysize_t record_pos);
		void printNode(mysize_t node);
	private:
		mysize_t findIndexPos(const byte_t* key);
		AVLIndexHeader* getHeaderPointer(mysize_t node_id);
		byte_t* getKeyPointer(mysize_t node_id);
		mysize_t LL(mysize_t record_pos);
		mysize_t LR(mysize_t record_pos);
		mysize_t RR(mysize_t record_pos);
		mysize_t RL(mysize_t record_pos);
		void setLeftChild(mysize_t parent, mysize_t child);
		void setRightChild(mysize_t parent, mysize_t child);
		void dumpNodeInMid(mysize_t node);
		byte_t* keyBuffer;
	};


#endif
