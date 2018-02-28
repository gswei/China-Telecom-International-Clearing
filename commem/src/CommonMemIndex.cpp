#include "CommonMemIndex.h"


/*
CommonMemIndex::CommonMemIndex(const RecordStruct* record_struct, const RecordStruct* index_struct, mysize_t size, key_t shm_key) {
	this->recordStruct = record_struct;
	this->indexStruct = index_struct;
}
*/

CommonMemIndex::CommonMemIndex()
{
}

int CommonMemIndex::keyCompare(const byte_t* a, const byte_t* b) const {
	const vector<ColumnStruct>* columns = indexStruct->getColumnList();
	int kk=columns->size();
	for (int i = 0; i < columns->size(); i++) {
		int reta, retb;
		long long a64, b64;
		switch((*columns)[i].columnType) {
			case INT32 :
				reta = indexStruct->getRealInt32(a, i);
				retb = indexStruct->getRealInt32(b, i);
				if (reta != retb) {
					return reta - retb;
				} else {
					continue;
				}
				break;
			case INT64 :
				a64 = indexStruct->getRealInt64(a, i);
				b64 = indexStruct->getRealInt64(b, i);
				if (a64 != b64) {
					return a64 - b64;
				} else {
					continue;
				}
				break;
			case CHAR :
				char aChar[STRING_MAX_SIZE];
				char bChar[STRING_MAX_SIZE];
				indexStruct->getRealString(a, i, aChar, STRING_MAX_SIZE);
				indexStruct->getRealString(b, i, bChar, STRING_MAX_SIZE);
				int cmpRet = strcmp(aChar, bChar);
				if (cmpRet != 0) {
					return cmpRet;
				} else {
					continue;
				}
				break;

		}

	}
	return 0;
}


BlockManager* CommonMemIndex::getBlockManager()
{
	return &blockManager;
}

int CommonMemIndex::getIndexId()
{
	return indexId;
}


//void CommonMemIndex::clear() {
	//blockManager.clear();
//}
