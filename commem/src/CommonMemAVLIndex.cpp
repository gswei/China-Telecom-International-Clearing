#include <algorithm>
#include "CommonMemAVLIndex.h"




/*
mysize_t CommonMemAVLIndex::updateIndex(const byte_t* record, mysize_t record_pos) {
	// TODO tobe implement
	return -1;
}
*/

const BlockManager* CommonMemAVLIndex::getBlockManager() const {
	return &blockManager;
}

CommonMemAVLIndex::CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key,int idxid) 
{
	indexId = idxid;
	indexStruct = index_struct;
	recordStruct = record_struct;
	int nodeSize = indexStruct->getRecordSize() + sizeof(AVLIndexHeader);
	blockManager.create(allocate_size, nodeSize, shm_key);
	keyBuffer = (byte_t*) malloc(indexStruct->getRecordSize()+1);
	memset( keyBuffer,0,indexStruct->getRecordSize()+1);
}

CommonMemAVLIndex::CommonMemAVLIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, int shid, int idxid,int kk) 
{
	indexId = idxid;
	indexStruct = index_struct;
	recordStruct = record_struct;
	int nodeSize = indexStruct->getRecordSize() + sizeof(AVLIndexHeader);
	blockManager.attach(allocate_size,nodeSize,shid);
	keyBuffer = (byte_t*) malloc(indexStruct->getRecordSize()+1);
	memset( keyBuffer,0,indexStruct->getRecordSize()+1);
}

void CommonMemAVLIndex::attach(mysize_t allocate_size, int shid)
{
	int nodeSize = indexStruct->getRecordSize() + sizeof(AVLIndexHeader);
	blockManager.attach(allocate_size,nodeSize,shid);	
}

void CommonMemAVLIndex::create(mysize_t allocate_size, key_t shm_key) {
	int nodeSize = indexStruct->getRecordSize() + sizeof(AVLIndexHeader);
	blockManager.create(allocate_size, nodeSize, shm_key);
	keyBuffer = (byte_t*) malloc(indexStruct->getRecordSize()+1);
	memset( keyBuffer,0,indexStruct->getRecordSize()+1);
}

CommonMemAVLIndex::~CommonMemAVLIndex() {
	free(keyBuffer);
}

/**
 * 根据节点左右子树高度，计算节点高度
 */
int CommonMemAVLIndex::treeNodeHigh(mysize_t node_id) {
	if (node_id == -1) {
		// 非节点，高度为-1
		return -1;
	}
	
	AVLIndexHeader* p = getHeaderPointer(node_id);
	if ( p->istreeNode == 0)
		{
			return -1 ;
		}
		
	int leftTreeHigh = -1;
	int rightTreeHigh = -1;
	if (p->leftChild != -1) {
		AVLIndexHeader* lChild = getHeaderPointer(p->leftChild);
		leftTreeHigh = lChild->treeHigh;
	}
	if (p->rightChild != -1) {
		AVLIndexHeader* rChild = getHeaderPointer(p->rightChild);
		rightTreeHigh = rChild->treeHigh;
	}
	int maxtree;
	if (leftTreeHigh > rightTreeHigh) maxtree=leftTreeHigh;
	else  maxtree=rightTreeHigh;
	return maxtree + 1;
}


CommonMemAVLIndex::AVLIndexHeader* CommonMemAVLIndex::getHeaderPointer(mysize_t node_id) {
	if (node_id < 0) {
		return NULL;
	}
	AVLIndexHeader* headerP = (AVLIndexHeader*)blockManager.getBlockPointer(node_id);
	// TODO throw exception when node is not found
	return headerP;

}
byte_t* CommonMemAVLIndex::getKeyPointer(mysize_t node_id) {
	byte_t* nodeP = blockManager.getBlockPointer(node_id);
	// TODO throw exception when node is not found
	return nodeP + sizeof(AVLIndexHeader);
}

int CommonMemAVLIndex::initNode(mysize_t node_id, mysize_t record_idx, const byte_t* key) {
	AVLIndexHeader* header = getHeaderPointer(node_id);
	header->parent = -1; //无父
	header->leftChild = -1; // 无子
	header->rightChild = -1;
	header->nextlink = -1;//无链表下一个节点
	header->istreeNode = 1;
	//header->rwLock.init(); //初始化锁
	header->treeHigh = -1; //叶子节点
	header->recordIndex = record_idx;
	byte_t* keyP = getKeyPointer(node_id); // 拷贝key
	memcpy(keyP, key, indexStruct->getRecordSize());
	return node_id;
}

mysize_t CommonMemAVLIndex::insertIndex(const byte_t* record, mysize_t record_idx) {
	recordStruct->copyTo(keyBuffer, indexStruct, record);
	mysize_t node = dirtyInsert(keyBuffer, record_idx);
	balance(node);
	return node;
}

// 非平衡插入，插入后不保证平衡，返回插入节点的id
mysize_t CommonMemAVLIndex::dirtyInsert(const byte_t* key, mysize_t record_idx) {
	AVLIndexHeader* child;
	mysize_t nodeIdx = blockManager.allocateBlock();
	int isSame=0;
	if (nodeIdx < 0) {
		// TODO throw some exception
		return -1;
	}
	initNode(nodeIdx, record_idx, key);
	mysize_t x = blockManager.getDataHead();
	mysize_t p = -1;
	while (x != -1) {
		p = x;
		byte_t* keyX = getKeyPointer(x);
		int compareRet = keyCompare(key, keyX);
		if ( compareRet < 0) {
			child = getHeaderPointer(x);
			x = child->leftChild;
		} else if (compareRet == 0) {
			//TODO throw some expception
			//return -1;
			child = getHeaderPointer(nodeIdx);
			child->parent = p;
			AVLIndexHeader* father;
			father=getHeaderPointer(p);
			child->nextlink = father->nextlink;
			child->istreeNode = 0;
			child = getHeaderPointer(p);
			child->nextlink = nodeIdx;
			
			isSame=1;
			break;
		} else {
			child = getHeaderPointer(x);
			x = child->rightChild;
		}
	}
	child = getHeaderPointer(nodeIdx);
	child->parent = p;

	if (p == -1) {
		blockManager.setDataHead(nodeIdx);
	} else if (keyCompare(key, getKeyPointer(p)) < 0 && isSame==0) {
		child = getHeaderPointer(nodeIdx);
		child->parent = p;
		//getHeaderPointer(p)->rwLock.getWriteLock();
		child = getHeaderPointer(p);
		child->leftChild = nodeIdx;
		//getHeaderPointer(p)->rwLock.releaseWriteLock();

	} else if (keyCompare(key, getKeyPointer(p)) > 0 && isSame==0) {
		child = getHeaderPointer(nodeIdx);
		child->parent = p;
		//getHeaderPointer(p)->rwLock.getWriteLock();
		child = getHeaderPointer(p);
		child->rightChild = nodeIdx;
		//getHeaderPointer(p)->rwLock.releaseWriteLock();
	}
	renewHigh(nodeIdx);
	return nodeIdx;
}


void CommonMemAVLIndex::renewHigh(mysize_t node) {
	if (node == -1) {
		// 非节点
		return;
	}
	AVLIndexHeader* header = getHeaderPointer(node);
	//如果非树节点，不用计算树高
	if (header->istreeNode == 0)
		{
			return;
		}
		
	int oldHigh = header->treeHigh;
	header->treeHigh = treeNodeHigh(node);
	if (oldHigh != header->treeHigh) {
		// 本节点高度发生变化，需要更新父节点的高度
		renewHigh(header->parent);
	}
}

void CommonMemAVLIndex::balance(mysize_t node_id) {
	mysize_t currentNode = node_id;
	DEBUG_LOG<<"balance node ="<<node_id<<"= start"<<__FILE__<<","<<__LINE__<<endd;
	while (currentNode != -1) {
		AVLIndexHeader* currentHeader = getHeaderPointer(currentNode);
		int leftTreeHigh = treeNodeHigh(currentHeader->leftChild);
		int rightTreeHigh = treeNodeHigh(currentHeader->rightChild);
		int balanceFactor = leftTreeHigh - rightTreeHigh;
		if (balanceFactor == 2) {
			// 左高
			AVLIndexHeader* leftChildHeader
				= getHeaderPointer(currentHeader->leftChild);
			if (treeNodeHigh(leftChildHeader->leftChild)
				       	>= treeNodeHigh(leftChildHeader->rightChild)) {
				// 左左
				currentNode = LL(currentNode);
			} else {
				// 左右
				currentNode = LR(currentNode);
			}
		} else if (balanceFactor == -2) {
			// 右高
			AVLIndexHeader* rightChildheader = getHeaderPointer(currentHeader->rightChild);
			if (treeNodeHigh(rightChildheader->leftChild) >= treeNodeHigh(rightChildheader->rightChild)) {
				// 右左
				currentNode = RL(currentNode);
			} else {
				// 右右
				currentNode = RR(currentNode);
			}
		} else {
			// 不需要旋转
			currentNode = currentHeader->parent;
		}
	}
}

mysize_t CommonMemAVLIndex::LL(mysize_t curr_node) {
	DEBUG_LOG<<"LL("<<curr_node<<")"<<__FILE__<<","<<__LINE__<<endd;
	AVLIndexHeader* currHeader = getHeaderPointer(curr_node);
	mysize_t parent = currHeader->parent;
	mysize_t leftChild = currHeader->leftChild;
	AVLIndexHeader* parentHeader = getHeaderPointer(parent);
	AVLIndexHeader* leftHeader = getHeaderPointer(leftChild);
	mysize_t lrChild = leftHeader->rightChild;
	AVLIndexHeader* lrHeader = getHeaderPointer(lrChild);


	//vector<ReadWriteLock*> locks;
	//locks.push_back(&(currHeader->rwLock));
	//if (parent != -1) locks.push_back(&(parentHeader->rwLock));
	//if (leftChild != -1) locks.push_back(&(leftHeader->rwLock));
	//if (lrChild != -1) locks.push_back(&(lrHeader->rwLock));
	//writeLockAll(locks);
	if (parent != -1) {
		if (parentHeader->leftChild == curr_node) {
			//parentHeader->leftChild = leftChild;
			setLeftChild(parent, leftChild);
		} else {
			//parentHeader->rightChild = leftChild;
			setRightChild(parent, leftChild);
		}
		//leftHeader->parent = parent;
	} else {
		blockManager.setDataHead(leftChild);
		leftHeader->parent = -1;
	}

	//currHeader->leftChild = lrChild;
	//lrHeader->parent = curr_node;
	setLeftChild(curr_node, lrChild);

	//leftHeader->rightChild = curr_node;
	//currHeader->parent = leftChild;
	setRightChild(leftChild, curr_node);

	//currHeader->treeHigh = treeNodeHigh(curr_node);
	//leftChildHeader->treeHigh = treeNodeHigh(leftChild);
	//parentHeader->treeHigh = treeNodeHigh(parent);
	renewHigh(curr_node);

	//releaseWriteLockAll(locks);
	return parent;
}

mysize_t CommonMemAVLIndex::getSuccessorNode(mysize_t record_pos) {
	AVLIndexHeader* recordHeader = getHeaderPointer(record_pos);
	if (NULL == recordHeader) {
		return -1;
	}
	mysize_t p = -1;
	mysize_t currentNode = recordHeader->rightChild;
	while (currentNode != -1) {
		p = currentNode;
		AVLIndexHeader* nodeHeader = getHeaderPointer(currentNode);
		currentNode = nodeHeader->leftChild;
	}
	return p;
}

void CommonMemAVLIndex::queryIndex(const byte_t* key,vector<mysize_t> &nodeID) {
	nodeID.clear();
	recordStruct->copyTo(keyBuffer, indexStruct, key);
	AVLIndexHeader* header;
	mysize_t indexNode = findIndexPos(keyBuffer);
	if (indexNode != -1) {
		header = getHeaderPointer(indexNode);
		nodeID.push_back(header->recordIndex);
		while( header->nextlink != -1 )
		{
			indexNode = header->nextlink;
			header = getHeaderPointer(indexNode);
			nodeID.push_back(header->recordIndex);
		}
	}
}

void CommonMemAVLIndex::queryIndex(const byte_t* key, const CommonMemTable* table, RecordList* record_list) {
	recordStruct->copyTo(keyBuffer, indexStruct, key);
	mysize_t indexNode = findIndexPos(keyBuffer);
	AVLIndexHeader* header ;
	record_list->clear();
	mysize_t tableNode;
	if (indexNode != -1) {
		header = getHeaderPointer(indexNode);
		tableNode=header->recordIndex;
		record_list->add(table->getRow(tableNode), table->getRecordSize());
	}
	while( header->nextlink != -1 )
	{
		indexNode = header->nextlink;
		header = getHeaderPointer(indexNode);
		tableNode=header->recordIndex;
		record_list->add(table->getRow(tableNode), table->getRecordSize());
	}
}


/**
 * 查找符合key的索引，并加读锁
 */
mysize_t CommonMemAVLIndex::findIndexPos(const byte_t* key) {
	mysize_t currNode = blockManager.getDataHead();
	while (currNode != -1) {
		AVLIndexHeader* header = getHeaderPointer(currNode);
		//header->rwLock.getReadLock();
		const byte_t* currKey = getKeyPointer(currNode);
		int cmpResult = keyCompare(key, currKey);
		if (cmpResult > 0) {
			currNode = header->rightChild;
			//header->rwLock.releaseReadLock();
		} else if (cmpResult < 0) {
			currNode = header->leftChild;
			//header->rwLock.releaseReadLock();
		} else {
			//header->rwLock.releaseReadLock();
			break;
		}
	}
	return currNode;
}

mysize_t CommonMemAVLIndex::deleteIndex(const byte_t* key, mysize_t record_pos) {
	mysize_t indexPos = findIndexPos(key);
	AVLIndexHeader* deleteHeader = getHeaderPointer(indexPos);
	if (deleteHeader == NULL) {
		return -1;
	}
	//deleteHeader->rwLock.releaseReadLock();
	if (-1 == deleteHeader->leftChild && -1 == deleteHeader->rightChild) {
		// 叶节点，直接删除
		mysize_t parent = deleteHeader->parent;
		if (parent != -1) {
			AVLIndexHeader* parentHeader = getHeaderPointer(parent);
			//vector<ReadWriteLock*> locks;
			//locks.push_back(&parentHeader->rwLock);
			//locks.push_back(&deleteHeader->rwLock);
			//writeLockAll(locks);
			if (parentHeader->leftChild == indexPos) {
				parentHeader->leftChild = -1;
			} else {
				parentHeader->rightChild = -1;
			}
			renewHigh(parent);
			//releaseWriteLockAll(locks);
			balance(parent);
		} else {
			//deleteHeader->rwLock.getWriteLock();
			blockManager.setDataHead(-1);
			//deleteHeader->rwLock.releaseWriteLock();
		}
		blockManager.freeBlock(indexPos);
		return 0;
	} else if (-1 == deleteHeader->leftChild || -1 == deleteHeader->rightChild) {
		// 只有一个子节点
		mysize_t parent = deleteHeader->parent;
		if (parent != -1) {
			AVLIndexHeader* parentHeader = getHeaderPointer(parent);
			mysize_t onlyChild;
			if (deleteHeader->leftChild != -1) {
				onlyChild = deleteHeader->leftChild;
			} else {
				onlyChild = deleteHeader->rightChild;
			}

			AVLIndexHeader* onlyChildHeader = getHeaderPointer(onlyChild);
			//vector<ReadWriteLock*> locks;
			//locks.push_back(&parentHeader->rwLock);
			//locks.push_back(&deleteHeader->rwLock);
			//locks.push_back(&onlyChildHeader->rwLock);
			//writeLockAll(locks);

			if (parentHeader->leftChild == indexPos) {
				parentHeader->leftChild = onlyChild;
			} else {
				parentHeader->rightChild = onlyChild;
			}
			onlyChildHeader->parent = parent;
			renewHigh(onlyChild);
			//releaseWriteLockAll(locks);
			balance(parent);
		} else {
			//deleteHeader->rwLock.getWriteLock();
			mysize_t onlyChild;
			if (deleteHeader->leftChild != -1) {
				//blockManager.setDataHead(deleteHeader->leftChild);
				onlyChild = deleteHeader->leftChild;
			} else {
				//blockManager.setDataHead(deleteHeader->rightChild);
				onlyChild = deleteHeader->rightChild;
			}
			AVLIndexHeader* onlyChildHeader = getHeaderPointer(onlyChild);
			//vector<ReadWriteLock*> locks;
			//locks.push_back(&(deleteHeader->rwLock));
			//locks.push_back(&(onlyChildHeader->rwLock));
			//writeLockAll(locks);
			blockManager.setDataHead(onlyChild);
			onlyChildHeader->parent = -1;
			//releaseWriteLockAll(locks);

			//deleteHeader->rwLock.releaseWriteLock();
		}
		blockManager.freeBlock(indexPos);
		return 0;

	} else {
		// 双子节点
		mysize_t successor = getSuccessorNode(indexPos);
		AVLIndexHeader* successorHeader = getHeaderPointer(successor);
		if (successor == deleteHeader->rightChild) {
			//vector<ReadWriteLock*> locks;
			//locks.push_back(&deleteHeader->rwLock);
			//locks.push_back(&successorHeader->rwLock);
			//writeLockAll(locks);
			deleteHeader->recordIndex = successorHeader->recordIndex;
			byte_t* delKeyP = getKeyPointer(indexPos);
			byte_t* successorKeyP = getKeyPointer(successor);
			memcpy(delKeyP, successorKeyP, recordStruct->getRecordSize());
			deleteHeader->rightChild = -1;
			renewHigh(indexPos);
			//releaseWriteLockAll(locks);
			blockManager.freeBlock(successor);
			return 0;
		} else {
			const mysize_t sp = successorHeader->parent;
			const mysize_t sr = successorHeader->rightChild;
			AVLIndexHeader* spHeader = getHeaderPointer(sp);
			AVLIndexHeader* srHeader = getHeaderPointer(sr);
			deleteHeader->recordIndex = successorHeader->recordIndex;
			//vector<ReadWriteLock*> locks;
			//locks.push_back(&deleteHeader->rwLock);
			//locks.push_back(&spHeader->rwLock);
			//locks.push_back(&successorHeader->rwLock);
			//if (srHeader != NULL) locks.push_back(&srHeader->rwLock);
			//writeLockAll(locks);

			byte_t* delKeyP = getKeyPointer(indexPos);
			byte_t* successorKeyP = getKeyPointer(successor);
			memcpy(delKeyP, successorKeyP, recordStruct->getRecordSize());
			if (sr == -1) {
				spHeader->leftChild = successorHeader->rightChild;
			}
			renewHigh(sp);
			//releaseWriteLockAll(locks);
			blockManager.freeBlock(successor);
		}
	}
	return 1;

}


mysize_t CommonMemAVLIndex::RR(mysize_t curr_node) {
	DEBUG_LOG<<"RR("<<curr_node<<")"<<__FILE__<<","<<__LINE__<<endd;
	AVLIndexHeader* currHeader = getHeaderPointer(curr_node);
	mysize_t root = currHeader->parent;
	mysize_t rChild = currHeader->rightChild;
	AVLIndexHeader* rootHeader = getHeaderPointer(root);
	AVLIndexHeader* rHeader = getHeaderPointer(rChild);
	mysize_t rlChild = rHeader->leftChild;
	AVLIndexHeader* rlHeader = getHeaderPointer(rlChild);

	//vector<ReadWriteLock*> locks;
	//locks.push_back(&(currHeader->rwLock));
	//if (root != -1) locks.push_back(&(rootHeader->rwLock));
	//if (rChild != -1) locks.push_back(&(rHeader->rwLock));
	//if (rlChild != -1) locks.push_back(&(rlHeader->rwLock));
	//writeLockAll(locks);

	if (root != -1) {
		if (rootHeader->leftChild == curr_node) {
			//rootHeader->leftChild = rChild;
			setLeftChild(root, rChild);
		} else {
			//rootHeader->rightChild = rChild;
			setRightChild(root, rChild);
		}
		//rHeader->parent = root;
	} else {
		blockManager.setDataHead(rChild);
		rHeader->parent = -1;
	}

	//currHeader->rightChild = rlChild;
	//rlHeader->parent = curr_node;
	setRightChild(curr_node, rlChild);

	//rHeader->leftChild = curr_node;
	//currHeader->parent = rChild;
	setLeftChild(rChild, curr_node);

	renewHigh(curr_node);
	//releaseWriteLockAll(locks);

	return root;

}

mysize_t CommonMemAVLIndex::LR(mysize_t curr_node) {
	DEBUG_LOG<<"LR("<<curr_node<<")"<<__FILE__<<","<<__LINE__<<endd;
	AVLIndexHeader* currHeader = getHeaderPointer(curr_node);
	mysize_t root = currHeader->parent;
	mysize_t lChild = currHeader->leftChild;
	AVLIndexHeader* rootHeader = getHeaderPointer(root);
	AVLIndexHeader* lHeader = getHeaderPointer(lChild);
	mysize_t lrChild = lHeader->rightChild;
	AVLIndexHeader* lrHeader = getHeaderPointer(lrChild);
	mysize_t lrlChild = lrHeader->leftChild;
	mysize_t lrrChild = lrHeader->rightChild;
	AVLIndexHeader* lrlHeader = getHeaderPointer(lrlChild);
	AVLIndexHeader* lrrHeader = getHeaderPointer(lrrChild);

	//vector<ReadWriteLock*> locks;
	//locks.push_back(&(currHeader->rwLock));
	//if (root != -1) locks.push_back(&(rootHeader->rwLock));
	//if (lChild != -1) locks.push_back(&(lHeader->rwLock));
	//if (lrChild != -1) locks.push_back(&(lrHeader->rwLock));
	//if (lrlChild != -1) locks.push_back(&(lrlHeader->rwLock));
	//if (lrrChild != -1) locks.push_back(&(lrrHeader->rwLock));
	//writeLockAll(locks);

	//currHeader->leftChild = lrChild;
	//lrHeader->parent = curr_node;
	setLeftChild(curr_node, lrChild);

	//lHeader->rightChild = lrlChild;
	//lrlHeader->parent = lChild;
	setRightChild(lChild, lrlChild);


	//lrHeader->leftChild = lChild;
	//lHeader->parent = lrChild;
	setLeftChild(lrChild, lChild);

	if (root != -1) {
		if (rootHeader->leftChild == curr_node) {
			//rootHeader->leftChild = lrChild;
			setLeftChild(root, lrChild);
		} else {
			//rootHeader->rightChild = lrChild;
			setRightChild(root, lrChild);
		}
		//lrHeader->parent = root;
	} else {
		blockManager.setDataHead(lrChild);
		lrHeader->parent = -1;
	}

	//currHeader->leftChild = lrrChild;
	//lrrHeader->parent = curr_node;
	setLeftChild(curr_node, lrrChild);

	//lrHeader->rightChild = curr_node;
	//currHeader->parent = lrChild;
	setRightChild(lrChild, curr_node);

	renewHigh(lChild);
	renewHigh(curr_node);

	//releaseWriteLockAll(locks);
	return root;

}


mysize_t CommonMemAVLIndex::RL(mysize_t curr_node) {
	DEBUG_LOG<<"RL("<<curr_node<<")"<<__FILE__<<","<<__LINE__<<endd;
	AVLIndexHeader* currHeader = getHeaderPointer(curr_node);
	mysize_t root = currHeader->parent;
	mysize_t rChild = currHeader->rightChild;
	AVLIndexHeader* rootHeader = getHeaderPointer(root);
	AVLIndexHeader* rHeader = getHeaderPointer(rChild);
	mysize_t rlChild = rHeader->leftChild;
	AVLIndexHeader* rlHeader = getHeaderPointer(rlChild);
	mysize_t rllChild = rlHeader->leftChild;
	mysize_t rlrChild = rlHeader->rightChild;
	AVLIndexHeader* rllHeader = getHeaderPointer(rllChild);
	AVLIndexHeader* rlrHeader = getHeaderPointer(rlrChild);

	//vector<ReadWriteLock*> locks;
	//locks.push_back(&(currHeader->rwLock));
	//if (root != -1) locks.push_back(&(rootHeader->rwLock));
	//if (rChild != -1) locks.push_back(&(rHeader->rwLock));
	//if (rlChild != -1) locks.push_back(&(rlHeader->rwLock));
	//if (rlrChild != -1) locks.push_back(&(rlrHeader->rwLock));
	//if (rllChild != -1) locks.push_back(&(rllHeader->rwLock));
	//writeLockAll(locks);

	//currHeader->rightChild = rlChild;
	//rlHeader->parent = curr_node;
	setRightChild(curr_node, rlChild);

	//rHeader->leftChild = rlrChild;
	//rlrHeader->parent = rChild;
	setLeftChild(rChild, rlrChild);

	//rlHeader->rightChild = rChild;
	//rHeader->parent = rlChild;
	setRightChild(rlChild, rChild);

	if (root != -1) {
		if (rootHeader->leftChild == curr_node) {
			//rootHeader->leftChild = rlChild;
			setLeftChild(root, rlChild);
		} else {
			//rootHeader->rightChild = rlChild;
			setRightChild(root, rlChild);
		}
		//rlHeader->parent = root;
	} else {
		blockManager.setDataHead(rlChild);
		rlHeader->parent = -1;
	}

	//currHeader->rightChild = rllChild;
	//rllHeader->parent = curr_node;
	setRightChild(curr_node, rllChild);

	//rlHeader->leftChild = curr_node;
	//currHeader->parent = rlChild;
	setLeftChild(rlChild, curr_node);

	renewHigh(rChild);
	renewHigh(curr_node);

	//releaseWriteLockAll(locks);
	return root;
}

void CommonMemAVLIndex::setLeftChild(mysize_t parent, mysize_t child) {
	AVLIndexHeader* parentHeader = getHeaderPointer(parent);
	AVLIndexHeader* childHeader = getHeaderPointer(child);
	if (-1 != parent) {
		parentHeader->leftChild = child;

	}
	if (-1 != child) {
		childHeader->parent = parent;
	}
}

void CommonMemAVLIndex::setRightChild(mysize_t parent, mysize_t child) {
	AVLIndexHeader* parentHeader = getHeaderPointer(parent);
	AVLIndexHeader* childHeader = getHeaderPointer(child);
	if (-1 != parent) {
		parentHeader->rightChild = child;

	}
	if (-1 != child) {
		childHeader->parent = parent;
	}

}

void CommonMemAVLIndex::dumpIndexToScreen() const {

}

void CommonMemAVLIndex::printNode(mysize_t node) {
	if (node == -1) return;
	AVLIndexHeader* nodeHeader = getHeaderPointer(node);
	theJSLog<<"[Node begin]"<<endi;
	theJSLog<<"Posion="<<node<<",Parent="<<nodeHeader->parent<<",LeftChild="<<nodeHeader->leftChild<<",RightChild="<<nodeHeader->rightChild<<",NextLink="<<nodeHeader->nextlink<<",RecordIndex"<<nodeHeader->recordIndex<<",istreeNode="<<nodeHeader->istreeNode<<",key="<<endi;
	
	const vector<ColumnStruct>* columnList = indexStruct->getColumnList();
	byte_t* keyP = getKeyPointer(node);
	for (int i = 0; i < columnList->size(); i++) {
		#ifndef _LINUX_ACC 
		switch(columnList->at(i).columnType) {
		#else
		switch((*columnList)[i].columnType) {
		#endif

		case INT32:
			#ifndef _LINUX_ACC
			theJSLog<<columnList->at(i).columnName.c_str()<<":"<<indexStruct->getRealInt32(keyP, i)<<endi;
			#else
			theJSLog<<(*columnList)[i].columnName.c_str()<<":"<<indexStruct->getRealInt32(keyP, i)<<endi;
			#endif
			break;
		case INT64:
			#ifndef _LINUX_ACC
			theJSLog<<columnList->at(i).columnName.c_str()<<":"<<indexStruct->getRealInt64(keyP, i)<<endi;
			#else
			theJSLog<<(*columnList)[i].columnName.c_str()<<":"<<indexStruct->getRealInt64(keyP, i)<<endi;
			#endif
			break;
		case CHAR:
			char strBuf[STRING_MAX_SIZE];
			indexStruct->getRealString(keyP, i, strBuf, STRING_MAX_SIZE);
			#ifndef _LINUX_ACC
			theJSLog<<columnList->at(i).columnName.c_str()<<":"<<strBuf<<endi;
			#else
			theJSLog<<(*columnList)[i].columnName.c_str()<<":"<<strBuf<<endi;
			#endif
			break;
		}
	}
	theJSLog<<"[Node end]"<<endi;
}

void CommonMemAVLIndex::dumpNodeInMid(mysize_t node) {
	if (node == -1) return;
	AVLIndexHeader* nodeHeader = getHeaderPointer(node);
	//nodeHeader->rwLock.getReadLock();
	dumpNodeInMid(nodeHeader->leftChild);
	// Print current node
	printNode(node);
	dumpNodeInMid(nodeHeader->rightChild);
	//nodeHeader->rwLock.releaseReadLock();
}

void CommonMemAVLIndex::destory() {
	blockManager.destory();
}

void CommonMemAVLIndex::detach() {
	blockManager.detach();
}
