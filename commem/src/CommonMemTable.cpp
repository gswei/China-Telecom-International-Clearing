#include "CommonMemTable.h"
#include <assert.h>



CommonMemTable::CommonMemTable(const RecordStruct* rs, mysize_t record_number, key_t shm_key) {
	this->recordStruct = rs;
	blockManager.create(record_number, rs->getRecordSize() + sizeof(RecordHeader), shm_key);
}

CommonMemTable::CommonMemTable(const RecordStruct* rs, mysize_t record_number, int shid, int kk) {
	this->recordStruct = rs;
	blockManager.attach(record_number, rs->getRecordSize() + sizeof(RecordHeader),shid);
}

void CommonMemTable::attach(mysize_t record_number, int shid)
{
		blockManager.attach(record_number, recordStruct->getRecordSize() + sizeof(RecordHeader),shid);
}


void CommonMemTable::create( mysize_t record_number, key_t shm_key )
{
	blockManager.create(record_number, recordStruct->getRecordSize() + sizeof(RecordHeader), shm_key);
}

mysize_t CommonMemTable::insertRow(const byte_t* row) {
	mysize_t rowIndex = blockManager.allocateBlock();
	if (rowIndex < 0) {
		//throw exception
		throw CException(10011,"blockmanager分配不到空闲记录",__FILE__,__LINE__);
	} else {
		initRow(rowIndex);
		RecordHeader* header = getHeaderPointer(rowIndex);
		mysize_t nextN = blockManager.getDataHead();
		if ( nextN != -1 )
		{
			RecordHeader* preHeader = getHeaderPointer(nextN);
			preHeader->preNode = rowIndex;
		} else {
			blockManager.setLinkEnd(rowIndex);
		}
		header->nextNode = nextN;
		blockManager.setDataHead(rowIndex);

		byte_t* rowP = getRowPointer(rowIndex);
		memcpy(rowP, row, recordStruct->getRecordSize());
	}
	return rowIndex;
}

int CommonMemTable::getRecordSize() const {
	return recordStruct->getRecordSize();
}


void CommonMemTable::deleteRow(mysize_t row_index) {
	RecordHeader* header = getHeaderPointer(row_index);
	assert(header != NULL);
	RecordHeader* preHeader = getHeaderPointer(header->preNode);
	RecordHeader* nextHeader = getHeaderPointer(header->nextNode);
	//vector<ReadWriteLock*> locks;
	//if (preHeader != NULL) locks.push_back(&preHeader->rwLock);
	//if (nextHeader != NULL) locks.push_back(&nextHeader->rwLock);
	//writeLockAll(locks);
	if (header->preNode == -1) {
		blockManager.setDataHead(header->nextNode);
	} else {
		preHeader->nextNode = header->nextNode;
	}
	if (header->nextNode != -1) {
		nextHeader->preNode = header->preNode;
	} else {
		blockManager.setLinkEnd(header->preNode);
	}
	//releaseWriteLockAll(locks);
	blockManager.freeBlock(row_index);
}

/**
void CommonMemTable::updateRow(mysize_t rowIndex, const byte_t* row) {
	assert(rowIndex != -1);
	byte_t* rowP = getRowPointer(rowIndex);
	RecordHeader* header = getHeaderPointer(rowIndex);
	//header->rwLock.getWriteLock();
	memcpy(rowP, row, recordStruct->getRecordSize());
	//header->rwLock.releaseWriteLock();
}
*/
//void CommonMemTable::clear() {
	//blockManager.clear();
//}

void CommonMemTable::initRow(mysize_t row_index) {
	RecordHeader* header = getHeaderPointer(row_index);
	assert(header != NULL);

	
	header->nextNode = -1;
	header->preNode = -1;
	//header->rwLock.init();
}

void CommonMemTable::destory() {
	blockManager.destory();
}

void CommonMemTable::detach() {
	blockManager.detach();
}

RecordHeader* CommonMemTable::getHeaderPointer(mysize_t rowIndex) {
	return (RecordHeader*) blockManager.getBlockPointer(rowIndex);
}

byte_t* CommonMemTable::getRowPointer(mysize_t rowIndex) {
	return (byte_t*) blockManager.getBlockPointer(rowIndex) + sizeof(RecordHeader);
}

/*
void CommonMemTable::getReadLock(mysize_t rowIndex) {
	RecordHeader* header = getHeaderPointer(rowIndex);
	if (header) {
		header->rwLock.getReadLock();
	}
}

void CommonMemTable::releaseReadLock(mysize_t rowIndex) {
	RecordHeader* header = getHeaderPointer(rowIndex);
	if (header) {
		header->rwLock.releaseReadLock();
	}
	
}
*/

const byte_t* CommonMemTable::getRow(mysize_t rowIndex) const {
	return (byte_t*) blockManager.getBlockPointer(rowIndex) + sizeof(RecordHeader);
}

BlockManager* CommonMemTable::getBlockManager()
{
	return &blockManager;
}

