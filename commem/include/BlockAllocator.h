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
		//构造函数
		BlockManager();
		//析构函数
		~BlockManager();
		//分配一个block，先用回收的，没有再分配新的
		mysize_t allocateBlock();
		//粘帖到共享内存上
		int attach();
		int attach(mysize_t block_number, long block_size ,int shid);
		//创建共享内存，如果存在，抛除异常，如果不存在则创建，返回共享内存大小
		long create(mysize_t block_number, long block_size, key_t shm_key);
		//清空共享内存
		//void clear();
		//释放一个过期块
		int freeBlock(mysize_t);		
		//得到第n块的头地址
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
			mysize_t dataHead; // 数据的根节点，主要指向索引树的根节点
			mysize_t freeHead; // 空闲块链表的首节点
			mysize_t linkEnd; // 用于全表扫描的头记录
			mysize_t totalNum; // 总共可以使用的块
			mysize_t	usedBlocks; // 已经分配过的（包括分配后又回收的）块数量
			mysize_t usingBlocks; // 在使用的块的个数
			size_t memorySize; // 共享内存的总大小
			long blockSize; // 每个块的大小
			key_t shmKey; // 共享内存的key
			//ReadWriteLock rwLock; // 读写锁
		};
	private:
		void* shareMemory;
		Header* header;
		//数据区首地址
		byte_t* dataSegement;
		int shmid;
		key_t shmKey;
		size_t memSize; // 共享内存的总大小

		//返回共享内存大小
		//long checkShareMemory(key_t shm_key);
		//计算整个共享内存大小
		size_t caculateTotalSize(mysize_t block_number, long block_size);
		void mapVariable();
		void init(mysize_t block_number, size_t mem_size, long block_size, key_t shm_key);
		mysize_t getFreeBlockNextNode(mysize_t block_index);
		void setFreeBlockNextNode(mysize_t block_index, mysize_t next_index);

};

#endif
