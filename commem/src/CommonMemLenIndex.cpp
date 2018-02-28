#include <algorithm>
#include <string.h>
#include "CommonMemLenIndex.h"



const BlockManager* CommonMemLenIndex::getBlockManager() const {
	return &blockManager;
}

CommonMemLenIndex::CommonMemLenIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, key_t shm_key,int idxid) 
{
	indexId = idxid;
	indexStruct = index_struct;
	recordStruct = record_struct;
	int nodeSize = indexStruct->getRecordSize() + sizeof(LenIndexHeader);
	blockManager.create(allocate_size, nodeSize, shm_key);
	keyBuffer = (byte_t*) malloc(indexStruct->getRecordSize()+1);
	memset( keyBuffer,0,indexStruct->getRecordSize()+1);
}

CommonMemLenIndex::CommonMemLenIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t allocate_size, int shid, int idxid,int kk) 
{
	indexId = idxid;
	indexStruct = index_struct;
	recordStruct = record_struct;
	int nodeSize = indexStruct->getRecordSize() + sizeof(LenIndexHeader);
	blockManager.attach(allocate_size,nodeSize,shid);
	keyBuffer = (byte_t*) malloc(indexStruct->getRecordSize()+1);
	memset( keyBuffer,0,indexStruct->getRecordSize()+1);
}

void CommonMemLenIndex::attach(mysize_t allocate_size, int shid)
{
	int nodeSize = indexStruct->getRecordSize() + sizeof(LenIndexHeader);
	blockManager.attach(allocate_size,nodeSize,shid);	
}

void CommonMemLenIndex::create(mysize_t allocate_size, key_t shm_key)
{
	int nodeSize = indexStruct->getRecordSize() + sizeof(LenIndexHeader);
	blockManager.create(allocate_size, nodeSize, shm_key);
	return ;
}

CommonMemLenIndex::~CommonMemLenIndex() {
	free(keyBuffer);
}

CommonMemLenIndex::LenIndexHeader* CommonMemLenIndex::getHeaderPointer(mysize_t node_id) {
	if (node_id < 0) {
		return NULL;
	}
	LenIndexHeader* headerP = (LenIndexHeader*)blockManager.getBlockPointer(node_id);
	// TODO throw exception when node is not found
	return headerP;

}

byte_t* CommonMemLenIndex::getKeyPointer(mysize_t node_id) {
	byte_t* nodeP = blockManager.getBlockPointer(node_id);
	// TODO throw exception when node is not found
	return nodeP + sizeof(LenIndexHeader);
}

int CommonMemLenIndex::initNode(mysize_t node_id, mysize_t record_idx, const byte_t* key) {
	LenIndexHeader* header = getHeaderPointer(node_id);
	for ( int i=0;i <62;i++ )
	{
		header->p_num[i]=-1;
	}
	header->nextlink = -1; // 无子
	//header->rwLock.init(); //初始化锁
	header->recordIndex = record_idx;
	byte_t* keyP = getKeyPointer(node_id); // 拷贝key
	memcpy(keyP, key, indexStruct->getRecordSize());
	return node_id;
}

int CommonMemLenIndex::initNullNode(mysize_t node_id ) 
{
	LenIndexHeader* header = getHeaderPointer(node_id);
	for ( int i=0;i <62;i++ )
	{
		header->p_num[i]=-1;
	}
	header->nextlink = -1; // 无子
	header->recordIndex = -1;
	//header->rwLock.init(); //初始化锁
	byte_t* keyP = getKeyPointer(node_id); // 拷贝key
	memset(keyP, 0, indexStruct->getRecordSize());
	return node_id;
}

mysize_t CommonMemLenIndex::insertIndex(const byte_t* record, mysize_t record_pos)
{
	char eMessage[512];
	recordStruct->copyTo(keyBuffer, indexStruct, record);
	mysize_t x = blockManager.getDataHead();
	if ( x == -1)
	{
		mysize_t y = blockManager.allocateBlock();
		initNullNode( y );
		blockManager.setDataHead( y );
		x = y;
	}
	for (int i=0;i<strlen((const char *)keyBuffer);i++)
	{
		int ikey;
		LenIndexHeader* x0 = (LenIndexHeader*)blockManager.getBlockPointer(x);
		if ( (keyBuffer[i] > 47) && (keyBuffer[i] < 58) ) ikey=keyBuffer[i]-48;
		else if ( (keyBuffer[i] > 64) && (keyBuffer[i] < 91) ) ikey=keyBuffer[i]-55;
		//else if ( (keyBuffer[i] > 96) && (keyBuffer[i] < 123) ) ikey=keyBuffer[i]-61; 
		//modi by sunhua at 20110218
		else if ( (keyBuffer[i] > 96) && (keyBuffer[i] < 123) ) ikey=keyBuffer[i]-87;
		else 
		{
			sprintf(eMessage,"索引地址下标越界,key=%s,i=%d",keyBuffer,i+1);
			throw CException(10010,eMessage,__FILE__,__LINE__);	
		}

		if(x0->p_num[ikey] <= 0)
		{
			mysize_t yy = blockManager.allocateBlock();
			initNullNode( yy );
			x0->p_num[ikey] = yy;
		} 
		x = x0->p_num[ikey] ;
	}
	LenIndexHeader* x1 = (LenIndexHeader*)blockManager.getBlockPointer(x);
	while ( x1->nextlink != -1 )
	{
		x=x1->nextlink;
		x1 = (LenIndexHeader*)blockManager.getBlockPointer(x);
	}
	if ( x1->recordIndex > -1 )
	{
		mysize_t yyy = blockManager.allocateBlock();
		initNode(yyy, record_pos, keyBuffer );
		x1->nextlink = yyy;
		theJSLog<<"Lenindex node ="<<yyy<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	}
	else
	{
		x1->recordIndex = record_pos ;
		byte_t* keyP = getKeyPointer(x);
		memcpy(keyP,keyBuffer,indexStruct->getRecordSize());
		theJSLog<<"Lenindex node ="<<x<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	}
	
	return 0;
}

mysize_t CommonMemLenIndex::deleteIndex(const byte_t* record, mysize_t record_pos)
{
	//
}

void CommonMemLenIndex::queryIndex(const byte_t* key,vector<mysize_t> &nodeID)
{
	nodeID.clear();
	recordStruct->copyTo(keyBuffer, indexStruct, key);
	mysize_t tmp=-1;
	mysize_t x = blockManager.getDataHead();
	int stringl;
	stringl = strlen((const char *)keyBuffer);
	LenIndexHeader* x0;
	char eMessage[512];
	int i;
	for (i=0;i<stringl;i++)
	{
		int ikey;
		if ( (keyBuffer[i] > 47) && (keyBuffer[i] < 58) ) ikey=keyBuffer[i]-48;
		else if ( (keyBuffer[i] > 64) && (keyBuffer[i] < 91) ) ikey=keyBuffer[i]-55;
		//else if ( (keyBuffer[i] > 96) && (keyBuffer[i] < 123) ) ikey=keyBuffer[i]-61;
		//modi by sunhua at 20110218
		else if ( (keyBuffer[i] > 96) && (keyBuffer[i] < 123) ) ikey=keyBuffer[i]-87;
		else 
		{
			//sprintf(eMessage,"索引地址下标越界,key=%s,i=%d",keyBuffer,i+1);
			//throw CException(10010,eMessage,__FILE__,__LINE__);	
			//modi by sunhua at 2010-10-21
			//遇到超过地址下标的字符，直接当字符串结束，跳出循环
			//break ;
			//modi by sunhua at 20110321
			ikey=-1;
		}

		x0 = (LenIndexHeader*)blockManager.getBlockPointer(x);
		if ( x0->recordIndex >=0 )  tmp = x;

		if(x0->p_num[ikey] > 0) x = x0->p_num[ikey];
		else { break ; }
			
		if ( ikey == -1 ) break;
	}
	
	if ( i==stringl )
	{
		x0 = (LenIndexHeader*)blockManager.getBlockPointer(x);
		if ( x0->recordIndex > -1 ) tmp = x;
	}	
	
	if ( tmp >=0 )
	{
		LenIndexHeader* x1 = (LenIndexHeader*)blockManager.getBlockPointer(tmp);
		while ( x1->nextlink > 0 )
		{
			nodeID.push_back(x1->recordIndex);
			tmp = x1->nextlink;
			x1 = (LenIndexHeader*)blockManager.getBlockPointer(tmp);
		}
		nodeID.push_back(x1->recordIndex);
	}

}

void CommonMemLenIndex::queryIndex(const byte_t* key, const CommonMemTable* table, RecordList* recordList)
{
	recordList->clear();
	vector<mysize_t> ret;
	queryIndex(key,ret);
	vector<mysize_t>::iterator it;
	for ( it=ret.begin(); it!= ret.end(); it ++)
	{
		recordList->add(table->getRow(*it), table->getRecordSize());
	}
}

void CommonMemLenIndex::dumpIndexToScreen() const 
{

}

void CommonMemLenIndex::printNode(mysize_t node) {
	if (node == -1) return;
	LenIndexHeader* nodeHeader = getHeaderPointer(node);
	theJSLog<<"[Node begin]"<<"Posion="<<node<<endi;
	for ( int i=0;i<62;i++ )
	{
		theJSLog<<"p_num["<<i<<"]="<<nodeHeader->p_num[i]<<endi;
	}
	theJSLog<<"nextlink="<<nodeHeader->nextlink<<",RecordIndex="<<nodeHeader->recordIndex<<"Key="<<endi;
	const vector<ColumnStruct>* columnList = indexStruct->getColumnList();
	byte_t* keyP = getKeyPointer(node);
	if ( nodeHeader->recordIndex > 0 )
	{
		for (int i = 0; i < columnList->size(); i++) {
			#ifndef _LINUX_ACC 
			switch(columnList->at(i).columnType) {
			#else
			switch((*columnList)[i].columnType) {
			#endif
			case INT32:
				#ifndef _LINUX_ACC 
				theJSLog<<columnList->at(i).columnName.c_str()<<"="<<indexStruct->getRealInt32(keyP, i)<<endi;
				#else
				theJSLog<<(*columnList)[i].columnName.c_str()<<"="<<indexStruct->getRealInt32(keyP, i)<<endi;
				#endif
				break;
			case INT64:
				#ifndef _LINUX_ACC 
				theJSLog<<columnList->at(i).columnName.c_str()<<"="<<indexStruct->getRealInt64(keyP, i)<<endi;
				#else
				theJSLog<<(*columnList)[i].columnName.c_str()<<"="<<indexStruct->getRealInt64(keyP, i)<<endi;
				#endif
				break;
			case CHAR:
				char strBuf[STRING_MAX_SIZE];
				indexStruct->getRealString(keyP, i, strBuf, STRING_MAX_SIZE);
				#ifndef _LINUX_ACC
				theJSLog<<columnList->at(i).columnName.c_str()<<"="<<strBuf<<endi;
				#else
				theJSLog<<(*columnList)[i].columnName.c_str()<<"="<<strBuf<<endi;
				#endif
				break;
			}
		}
	}
	theJSLog<<"[Node end]"<<endi;
}


void CommonMemLenIndex::destory() {
	blockManager.destory();
}

void CommonMemLenIndex::detach() {
	blockManager.detach();
}
