//#inculde "Filter_Common.h"

#ifndef _C_MEMBLOCK_H_
#define _C_MEMBLOCK_H_

#include "FilterCommonStruct.h"
#include "FilterCommonFunction.h"

//一块只放同一秒的数据
class CMemBlock
{
	public:
		CMemBlock()
		{
			m_pdata = NULL;
			m_index = -1;
		}

		//获取共享内存的资源
		void AttachReSource(MemIndexData* memIndexdata, SIZE_TYPE index);
		//释放资源
		void DetachReSource();

		bool IsAttached();
		
		//将数据清空
		void ClearData();

		bool FindHeaderInTemp(const char* sourceId, const char *tmpfile, const char* indexfileName, SIZE_TYPE blockno, SIZE_TYPE &location );

		//*****************从文件中获取数据*************************
		//***********不修改链接关系,只修改内容***********
		bool LoadData(const char* SourceId, const char* szPath, SIZE_TYPE blockNo, const char* szTime, const char*process_id);
		//************************************************************
		
		//将数据回写到临时文件
		void Write2Temp(const char* TmpPath, const char*processid);
		
		//将内存中的数据回写到备份文件
		//void Backup(fstream &backupStream);
		void Backup(FILE *backupStream);
		
		//修改标志
		void ChangeFlag(DataType dataType);

		void SetFlag(DataType flag);
		DataType GetFlag();
		
		//设置前导指针号
		void SetForward(SIZE_TYPE forward);
		//获取前导指针号
		SIZE_TYPE GetForward();
		
		//设置后续指针号
		void SetBackward(SIZE_TYPE backward);
		//获取后续指针号
		SIZE_TYPE GetBackward();

		//获取块号
		SIZE_TYPE GetBlockNo();
		//设置块号
		void SetBlockNo(SIZE_TYPE blockno);
		char* GetBlockFileName();
		void SetBlockFileName(const char* filename);

		char* GetSourceID();
		void SetSourceID(const char* filename);

		//是否填满
		bool IsFull();
		//查找关键字
		bool FindIndex(SIZE_TYPE second, const char* Key, SIZE_TYPE &index);
		//获取最大值
		bool GetMaxValue(IndexDataValue& MValue);
		//插入关键字
		bool InsertIndex(SIZE_TYPE second, const char* Key);
		//删除关键字
		bool DeleteIndex(SIZE_TYPE second, const char* Key);
		
		~CMemBlock()
		{
			m_pdata = NULL;
			m_index = -1;
		}

		//获取块内索引数目
		SIZE_TYPE GetIndexCount();
		//设置块内索引数目
		void SetIndexCount(SIZE_TYPE count);

		//获取文件块的下一个块号
		SIZE_TYPE GetNextBlock();
		//设置文件块的下一个块号
		void SetNextBlock(SIZE_TYPE count);
		
		//获取第index个关健值指针(从０开始计数)
		void GetKey(SIZE_TYPE index, IndexDataValue& value);
		////获取第index个索引的秒数(从０开始计数)
		SIZE_TYPE GetSecond(SIZE_TYPE index);
		//设置第index个关健值指针(从０开始计数)
		void SetKey(SIZE_TYPE index, SIZE_TYPE second, const char *key, bool isdeleteFlag);
		void SetKey(SIZE_TYPE index, const IndexDataValue &value);
		
		//获取第index个关健值的删除标志(从０开始计数)
		bool GetDeleteFlag(SIZE_TYPE index);
		//设置第index个关健值的删除标志(从０开始计数)
		bool SetDeleteFlag(SIZE_TYPE index, bool isDelete);
		//二分查找
		int BinarySearch(SIZE_TYPE second, const char* Key, SIZE_TYPE &location);
		
	private:	
		MemIndexData *m_pdata;
		SIZE_TYPE m_index;
};

#endif



