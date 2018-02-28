
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
	IDLE = 0,	//����
	INFILE ,		//�����ļ�����һ��
	INTEMP,		//������ʱ�ļ�����һ��
	INMEM		//��ֻ���ڴ��б�����������
};

enum ProcessPeriod
{
	INIT = 0,
	RECORD
};


//�ļ����ݿ�ͷ
struct IndexDataHead
{
	char szSourceId[8];	//����Դid
	char szFileName[FILTER_FILESIZE+1];		//��ʽ�����ļ���
	SIZE_TYPE second;			//����
	SIZE_TYPE nextBlock;			//��ʽ�����ļ����ݿ��nextָ��
	SIZE_TYPE blockNo;			//��ʽ�����ļ����ݿ���
	SIZE_TYPE indexInBlock;		//��ʽ�����ļ����ݿ���ʵ�ʵ���������
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

//���ݿ�һ����¼������ֵ
struct IndexDataValue
{
	SIZE_TYPE second;	//����
	char szIndexValue[FILTER_VALUESIZE+1];		//keyֵ
	bool isDelete;						//�Ƿ�delete��־
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



//�ļ��е��������ݿ�
struct IndexData
{
	IndexDataHead dataHead;	//���ݿ�Ŀ�ͷ��Ϣ
	IndexDataValue data[FILTER_MAXINDEXNO+1];	//���ݿ��������
};

//�ڴ��е����ݿ�
struct MemIndexData
{
	char szTime[15];	//ȥ��ʱ��
	SIZE_TYPE forward;	//ǰ��ָ��
	SIZE_TYPE backward;	//����ָ��
	DataType type;	//�������ݸ��±�־
	IndexData indexData;	//�ļ��е����ݿ�
};

//�ļ�ͷ��Ϣ
struct IndexFileHead
{
	SIZE_TYPE blockInFile;		//�ļ��е��ܿ���
	char szSourceId[8];			//����Դ
	char szFileName[FILTER_FILESIZE+1];	//�ļ���
	char szTime[15];				//ʱ���
	SIZE_TYPE blockItem[FILTER_MAXSECONDINFILE];	//�����ŵĶ�Ӧ��ϵ
};

//�ڴ��е��ļ�ͷ;��ʱ���
struct MemIndexFileHead
{
	SIZE_TYPE forward;	//ǰ��ָ��
	SIZE_TYPE backward;	//����ָ��
	DataType type;	//���ݸ��±�־
	sem_t semid;	//�ڴ���
	//char szProcessIndex[FILTER_PROCESS_INFO_LEN+1];	//��ʱû�õ�
	SIZE_TYPE memDataListHead;	//ָ�����ݿ��б�ͷ
	SIZE_TYPE memDataListEnd;	//ָ�����ݿ��б�β
	int memDataCount;
	IndexFileHead indexFileHead;	//�ļ�ͷ��Ϣ
};

//�������ļ��б�ͷ�Ķ�Ӧ��ϵ (1��1)
struct ProcessFileInfo
{
	char szProcessIndex[FILTER_PROCESS_INFO_LEN+1];
	SIZE_TYPE memIndexFileHead;	//ָ�������ļ��б�ͷ
};
//�ڴ��е�����Դ
struct MemSourceHead
{

	SIZE_TYPE idleBlockHead;	//����Դ�������ݿ��ͷָ��
	SIZE_TYPE idleBlockEnd;	//����Դ�������ݿ��βָ��
	
	SIZE_TYPE forward;	//ǰ��ָ��
	SIZE_TYPE backward;	//����ָ��
	DataType type;	//�������ݸ��±�־
	sem_t semid;	//�ڴ���
	char szSourceId[8];	//����Դid
	char szDealFileName[FILTER_FILESIZE];	//���ڴ�����ļ���(����У���ϴ�ִ���Ƿ�ɹ�)
	SIZE_TYPE beginIndex;	//�ļ��б�ͷ
	SIZE_TYPE endIndex;		//�ļ��б�β

	//����ռ�б�����Դ�Ľ���id
	int iprocessId;
	
	//SIZE_TYPE length;		//����(û���õ�)
	//ProcessFileInfo processFileInfo[FILTER_SOURCE_PROCESS];	//û���õ�
};

#endif

