#include "BlockAllocator.h"

//���캯��
BlockManager::BlockManager()
{
	shareMemory=NULL;
	header=NULL;
	dataSegement=NULL;
	shmid=-1;
}

//��������
BlockManager::~BlockManager()
{
	//������
	//destory();
	shareMemory=NULL;
	header=NULL;
	dataSegement=NULL;
	shmid=-1;
}

//����һ��block�����û��յģ�û���ٷ����µ�
mysize_t BlockManager::allocateBlock() 
{
	mysize_t ret = -1;

	mysize_t freeBlockLinkList = header->freeHead;

	if (freeBlockLinkList != -1) {
		ret = freeBlockLinkList;
		header->freeHead = getFreeBlockNextNode(ret);
		header->usingBlocks++;
	} else if (header->usedBlocks < header->totalNum) {
		ret = header->usedBlocks;
		header->usedBlocks++;
		header->usingBlocks++;
	}
	
	return ret;
}

//ճ���������ڴ���
int BlockManager::attach() 
{
	char eMessage[512];
	if ( shareMemory == NULL )
	{
		errno = 0;
		shareMemory = shmat(shmid, NULL, 0);	
		if (shareMemory == (void *) -1)
		{
			sprintf(eMessage,"blockmanager attach �������ڴ�ʱ����errno=%d=",errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}	
	}

	mapVariable();
	return 0;
}

int BlockManager::attach(mysize_t block_number, long block_size ,int shid)
{
	shmid=shid;
	char eMessage[512];
	theJSLog<<"block_number="<<block_number<<",block_size="<<block_size<<",ishmid="<<shid<<","<<__FILE__<<","<<__LINE__<<endi;
	if ( shareMemory == NULL )
	{
		errno = 0;
		shareMemory = shmat(shmid, NULL, 0);	
		if (shareMemory == (void *) -1)
		{
			sprintf(eMessage,"blockmanager attach �������ڴ�ʱ����errno=%d=",errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}	
	}

	mapVariable();
	memSize=caculateTotalSize(block_number, block_size);
	//init(block_number, memSize, block_size, 0);
	return 0;
}	


//�������������ڴ��С
size_t BlockManager::caculateTotalSize(mysize_t block_number, long block_size) 
{
	return block_number * block_size + sizeof(Header);
}

/*
//���ع����ڴ��С
long BlockManager::checkShareMemory(key_t shm_key) 
{
	int shmid = shmget(shm_key, 0, 0);
	if (shmid < 0) {
		return shmid;
	}
	struct shmid_ds shmDs;
	shmctl(shmid, IPC_STAT, &shmDs);
	return shmDs.shm_segsz;

}
*/

//��չ����ڴ�
//void BlockManager::clear() 
//{
	//������
	//init(header->totalNum, header->memorySize, header->blockSize, header->shmKey);
//}

//���������ڴ棬������ڣ��׳��쳣������������򴴽������ع����ڴ��С
long BlockManager::create(mysize_t block_number, long block_size, key_t shm_key) 
{
	char eMessage[512];
	memSize=caculateTotalSize(block_number, block_size);
	shmKey = shm_key;
	errno = 0;
	char tempStr[255];
	sprintf(tempStr,"%x",shm_key);		
	theJSLog<<"�����ڴ��ռ俪ʼ.key=0x"<<tempStr<<",size="<<memSize<<endi;
	shmid = shmget(shm_key, memSize, IPC_CREAT  |0666);
	if ( shmid == -1 ) 
	{
		sprintf(eMessage,"blockmanager shmget���������ڴ�ʱ����errno=%d=\n",errno);
		throw CException(10004,eMessage,__FILE__,__LINE__);	
	}
	theJSLog<<"blockmanager �����ڴ��ռ�ɹ���shm_key=0x"<<tempStr<<",shmid="<<shmid<<","<<__FILE__<<","<<__LINE__<<endi;
	attach();
	init(block_number, memSize, block_size, shm_key);
	return memSize;
}

//ɾ�������ڴ�
int BlockManager::destory() 
{
	char eMessage[512];
	int ret;
	if ( shareMemory != NULL ) 
	{
		errno=0;	
		ret=shmdt(shareMemory);
		if (ret==-1) 
		{
			sprintf(eMessage,"blockmanager shmdt�����ڴ�ʱ����errno=%d=",errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}
		shareMemory=NULL;
	}
	
  if ( shmid >0 )
  {
 		errno=0;
 		ret=shmctl(shmid, IPC_RMID, NULL);
		if (ret==-1) 
		{
			sprintf(eMessage,"blockmanager shmctlɾ�������ڴ�ʱ����errno=%d=",errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}  	
		shmid=0;
  }
	
	return 0;

}

//�ͷŹ����ڴ�
int BlockManager::detach() 
{
	char eMessage[512];
	int ret;
	if ( shareMemory != NULL ) 
	{
		errno=0;
		ret=shmdt(shareMemory);
		if (ret==-1) 
		{
			sprintf(eMessage,"blockmanager shmdt�����ڴ�ʱ����errno=%d=",errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}
		shareMemory=NULL;
		header=NULL;
		dataSegement=NULL;
	}

	
	return 0;
	
}

//�ͷ�һ�����ڿ�
int BlockManager::freeBlock(mysize_t block_index) {
	if (header->freeHead == -1) {
		header->freeHead = block_index;
		setFreeBlockNextNode(block_index, -1);
	} else {
		setFreeBlockNextNode(block_index, header->freeHead);
		header->freeHead = block_index;
	}
	header->usingBlocks--;
	return 0;
}

//�õ���n���ͷ��ַ
byte_t* BlockManager::getBlockPointer(mysize_t block_index) const 
{
	byte_t* ret = dataSegement + block_index * header->blockSize;
	return ret;
}

mysize_t BlockManager::getDataHead() 
{
	return header->dataHead;
}

size_t BlockManager::getMemorySize() const
{
	return memSize;
}

//��ȡ��һ�����нڵ�
mysize_t BlockManager::getFreeBlockNextNode(mysize_t block_index) 
{
	mysize_t ret;
	byte_t* pBlock = getBlockPointer(block_index);
	memcpy(&ret, pBlock, sizeof(mysize_t));
	return ret;
}



mysize_t BlockManager::getFreeBlockNum() const {
	return header->totalNum - header->usingBlocks;
}

/*
ReadWriteLock* BlockManager::getReadWriteLock() {
	return &header->rwLock;
}
*/


mysize_t BlockManager::getTotalBlockNum() const {
	return header->totalNum;
}

void BlockManager::init(mysize_t block_number, size_t mem_size, long block_size, key_t shm_key) {
	header->dataHead = -1;
	header->freeHead = -1;
	header->linkEnd = -1;
	header->totalNum = block_number;
	header->usedBlocks = 0;
	header->usingBlocks = 0;
	header->memorySize = mem_size;
	header->blockSize = block_size;
	header->shmKey = shm_key;
	//header->rwLock.init();
}

void BlockManager::mapVariable () {
	header = (Header*) shareMemory;
	dataSegement = ((byte_t *) shareMemory) + sizeof(Header);
	char iMessage[512];
	sprintf(iMessage,"Header address=%x,dataSegement address=%x,",header,dataSegement);
	DEBUG_LOG<<iMessage<<__FILE__<<","<<__LINE__<<endd;
	//theJSLog<<"mysize_t="<<sizeof(mysize_t)<<",byte_t="<<sizeof(byte_t)<<",size_t="<<sizeof(size_t)<<",key_t="<<sizeof(key_t)<<",header size="<<sizeof(Header)<<__FILE__<<","<<__LINE__<<endd;
	//mysize_t=8,byte_t=1,size_t=8,key_t=4,header size=64
}


void BlockManager::setDataHead(mysize_t data_head_idx) {
	header->dataHead = data_head_idx;
}




void BlockManager::setFreeBlockNextNode(mysize_t block_index, mysize_t next_index) {
	byte_t* pBlock = getBlockPointer(block_index);
	memcpy(pBlock, &next_index, sizeof(mysize_t));
}

int	BlockManager::getShmid() const
{
	return shmid;
}

key_t BlockManager::getShmKey() const
{
	return shmKey;
}

mysize_t BlockManager::getLinkEnd()
{
	return header->linkEnd ;
}

void BlockManager::setLinkEnd(mysize_t block_index)
{
	header->linkEnd = block_index ;
}

size_t BlockManager::getUsingBlocksNum() const
{
	return header->usingBlocks;
}

size_t BlockManager::getTotalBlocksNum() const
{
	return header->totalNum;
}

