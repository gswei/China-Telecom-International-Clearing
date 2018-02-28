#ifndef __BLOCK_MANAGER_H__
#define __BLOCK_MANAGER_H__

#include <sys/ipc.h>
#include <sys/shm.h>
#include "CommonMemTypes.h"
#include "CF_CLogger.h"
//#include "CF_COracleDB.h"
#include "string.h"
//#include "ReadWriteLock.h"


class BlockManager {
	public:
		//���캯��
		BlockManager();
		//��������
		~BlockManager();
		//����һ��block�����û��յģ�û���ٷ����µ�
		mysize_t allocateBlock();
		//ճ���������ڴ���
		int attach();
		int attach(mysize_t block_number, long block_size ,int shid);
		//���������ڴ棬������ڣ��׳��쳣������������򴴽������ع����ڴ��С
		long create(mysize_t block_number, long block_size, key_t shm_key);
		//��չ����ڴ�
		//void clear();
		//�ͷ�һ�����ڿ�
		int freeBlock(mysize_t);		
		//�õ���n���ͷ��ַ
		byte_t* getBlockPointer(mysize_t block_index) const;
		mysize_t getDataHead();
		mysize_t getTotalBlockNum() const;
		mysize_t getFreeBlockNum() const;
		int destory();
		int detach();
		void setDataHead(mysize_t data_head_idx);
		mysize_t getLinkEnd();
		void setLinkEnd(mysize_t block_index);
		key_t getShmKey() const;
		int	getShmid() const;
		size_t getMemorySize() const;
		//ReadWriteLock* getReadWriteLock();
		size_t getUsingBlocksNum() const;
		size_t getTotalBlocksNum() const;
	private:
		struct Header {
			mysize_t dataHead; // ���ݵĸ��ڵ㣬��Ҫָ���������ĸ��ڵ�
			mysize_t freeHead; // ���п�������׽ڵ�
			mysize_t linkEnd; // ����ȫ��ɨ���ͷ��¼
			mysize_t totalNum; // �ܹ�����ʹ�õĿ�
			mysize_t	usedBlocks; // �Ѿ�������ģ�����������ֻ��յģ�������
			mysize_t usingBlocks; // ��ʹ�õĿ�ĸ���
			size_t memorySize; // �����ڴ���ܴ�С
			long blockSize; // ÿ����Ĵ�С
			key_t shmKey; // �����ڴ��key
			//ReadWriteLock rwLock; // ��д��
		};
	private:
		void* shareMemory;
		Header* header;
		//�������׵�ַ
		byte_t* dataSegement;
		int shmid;
		key_t shmKey;
		size_t memSize; // �����ڴ���ܴ�С

		//���ع����ڴ��С
		//long checkShareMemory(key_t shm_key);
		//�������������ڴ��С
		size_t caculateTotalSize(mysize_t block_number, long block_size);
		void mapVariable();
		void init(mysize_t block_number, size_t mem_size, long block_size, key_t shm_key);
		mysize_t getFreeBlockNextNode(mysize_t block_index);
		void setFreeBlockNextNode(mysize_t block_index, mysize_t next_index);

};

#endif
