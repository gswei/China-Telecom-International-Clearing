
#ifndef _C_FILTER_COMMON_H_
#define _C_FILTER_COMMON_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <iostream.h>
#include <sys/types.h>
#include <string.h>

#include "FilterCommonFunction.h"


enum DataType 
{
	IDLE = 0,	//空闲
	INFILE ,		//块与文件内容一致
	INTEMP,		//块与临时文件内容一致
	INMEM		//块只在内存中保留最新内容
};

enum ProcessPeriod
{
	INIT = 0,
	RECORD
};


//文件数据块头
struct IndexDataHead
{
	char szSourceId[8];	//数据源id
	char szFileName[FILTER_FILESIZE+1];		//正式索引文件名
	SIZE_TYPE second;			//秒数
	SIZE_TYPE nextBlock;			//正式索引文件数据块的next指针
	SIZE_TYPE blockNo;			//正式索引文件数据块块号
	SIZE_TYPE indexInBlock;		//正式索引文件数据块中实际的索引条数
	IndexDataHead()
	{
		memset(szSourceId, 0, sizeof(szSourceId));
		memset(szFileName, 0, sizeof(szFileName));
		second = -1;
		nextBlock = -1;
		blockNo = -1;
		indexInBlock = -1;
	};
};

//数据块一条记录的索引值
struct IndexDataValue
{
	SIZE_TYPE second;	//秒数
	char szIndexValue[FILTER_VALUESIZE+1];		//key值
	bool isDelete;						//是否delete标志
	IndexDataValue()
	{
		second = 0;
		memset(szIndexValue, 0, sizeof(szIndexValue));
		isDelete = false;
	};
	IndexDataValue& operator = (const IndexDataValue& source)
	{
		if(this==&source)
			return *this;
		
		this->isDelete = source.isDelete;
		this->second = source.second;
		memcpy(this->szIndexValue, source.szIndexValue, FILTER_VALUESIZE+1);
		return *this;
	};

	bool operator < (const IndexDataValue& source)
	{	
		if(this->second < source.second)
		{
			return true;
		}
		else if(this->second > source.second)
		{
			return false;
		}
		if(memcmp(this->szIndexValue, source.szIndexValue, FILTER_VALUESIZE+1) < 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	};
};



//文件中的完整数据块
struct IndexData
{
	IndexDataHead dataHead;	//数据块的块头信息
	IndexDataValue data[FILTER_MAXINDEXNO+1];	//数据块的数据区
};

//内存中的数据块
struct MemIndexData
{
	char szTime[15];	//去重时间
	SIZE_TYPE forward;	//前导指针
	SIZE_TYPE backward;	//后续指针
	DataType type;	//数据内容更新标志
	IndexData indexData;	//文件中的数据块
};

//文件头信息
struct IndexFileHead
{
	SIZE_TYPE blockInFile;		//文件中的总块数
	char szSourceId[8];			//数据源
	char szFileName[FILTER_FILESIZE+1];	//文件名
	char szTime[15];				//时间段
	SIZE_TYPE blockItem[FILTER_MAXSECONDINFILE];	//秒与块号的对应关系
};

//内存中的文件头;即时间段
struct MemIndexFileHead
{
	SIZE_TYPE forward;	//前导指针
	SIZE_TYPE backward;	//后续指针
	DataType type;	//内容更新标志
	sem_t semid;	//内存锁
	//char szProcessIndex[FILTER_PROCESS_INFO_LEN+1];	//暂时没用到
	SIZE_TYPE memDataListHead;	//指向数据块列表头
	SIZE_TYPE memDataListEnd;	//指向数据块列表尾
	int memDataCount;
	IndexFileHead indexFileHead;	//文件头信息
};

//进程与文件列表头的对应关系 (1对1)
struct ProcessFileInfo
{
	char szProcessIndex[FILTER_PROCESS_INFO_LEN+1];
	SIZE_TYPE memIndexFileHead;	//指向数据文件列表头
};
//内存中的数据源
struct MemSourceHead
{

	SIZE_TYPE idleBlockHead;	//数据源空闲数据块块头指针
	SIZE_TYPE idleBlockEnd;	//数据源空闲数据块块尾指针
	
	SIZE_TYPE forward;	//前导指针
	SIZE_TYPE backward;	//后续指针
	DataType type;	//数据内容更新标志
	sem_t semid;	//内存锁
	char szSourceId[8];	//数据源id
	char szDealFileName[FILTER_FILESIZE];	//正在处理的文件名(用于校验上次执行是否成功)
	SIZE_TYPE beginIndex;	//文件列表头
	SIZE_TYPE endIndex;		//文件列表尾

	//增加占有本数据源的进程id
	int iprocessId;
	
	//SIZE_TYPE length;		//长度(没有用到)
	//ProcessFileInfo processFileInfo[FILTER_SOURCE_PROCESS];	//没有用到
};

#endif

