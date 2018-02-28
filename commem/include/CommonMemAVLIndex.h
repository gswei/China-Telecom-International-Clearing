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
		// ���ٹ����ڴ�
		void destory();
		// �Ͽ������ڴ�
		void detach();
	protected:
		//BlockManager blockManager;
		//const RecordStruct* indexStruct;
		//const RecordStruct* recordStruct;
		//vector<String> indexColumnNames;
		struct AVLIndexHeader {
			mysize_t parent; // ���ڵ��±�
			mysize_t leftChild; // �����±�
			mysize_t rightChild; // �����±�
			mysize_t nextlink;//�����¸��ڵ�
			mysize_t recordIndex; // ��Ӧ�ļ�¼���±�
			int			 istreeNode; //�Ƿ���AVL���ڵ�
			//ReadWriteLock rwLock; // ��д��
			int treeHigh; // ���߶�
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
