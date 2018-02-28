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
		// ���������ڵ㣬recordΪ������Ӧ�ļ�¼��record_pos�Ǹü�¼���±�
		virtual void create(mysize_t allocate_size, key_t shm_key) = 0;
		virtual mysize_t insertIndex(const byte_t* record, mysize_t record_pos) = 0;
		// ɾ�������ڵ㣬recordΪ������Ӧ�ļ�¼��record_pos�Ǹü�¼���±�
		//virtual mysize_t deleteIndex(const byte_t* record, mysize_t record_pos) = 0 ;
		// ���������ڵ㣬recordΪ������Ӧ�ļ�¼��record_pos�Ǹü�¼���±�
		//virtual mysize_t updateIndex(const byte_t* record, mysize_t record_pos) = 0;
		virtual void queryIndex(const byte_t* key,vector<mysize_t> &nodeID) = 0;
		// ��ѯ������keyΪ����ֵ��tableΪ�����ڴ�����recordListΪ����ļ�¼
		virtual void queryIndex(const byte_t* key, const CommonMemTable* table, RecordList* recordList) = 0;
		// ��չ����ڴ�
		//virtual void clear();
		virtual void attach(mysize_t allocate_size, int shid)=0;
		// ���ٹ����ڴ�
		virtual void destory() = 0;
		// �Ͽ������ڴ�
		virtual void detach() = 0;
		BlockManager* getBlockManager();
		int getIndexId();
		int indexId;

	protected:
		const RecordStruct* recordStruct;
		const RecordStruct* indexStruct;

		BlockManager blockManager;
		// ������keyֵ���бȽϣ���a>b�򷵻�ֵ>0��a=b�򷵻�ֵ0��a<b����ֵ<0
		int keyCompare(const byte_t* a, const byte_t* b) const;

	};

#endif
