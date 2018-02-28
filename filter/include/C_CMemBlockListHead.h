
#ifndef _C_MEMBLOCKLISTHEAD_H_
#define _C_MEMBLOCKLISTHEAD_H_

#include "FilterCommonStruct.h"
#include "FilterCommonFunction.h"

//一块只放同一文件的数据
class CMemBlockListHead
{
	public:
		CMemBlockListHead()
		{
			m_pdata = NULL;
			m_index = -1;
		};
		//从共享内存中获取资源
		void AttachReSource(MemIndexFileHead* memIndexFileHead, SIZE_TYPE index);
		//释放资源
		void DetachReSource();

		bool IsAttached();
		//将资源数据清空
		void Clear();

		bool FindFileInTemp(const char* sourceId, const char *tmpfile, const char* indexfile, SIZE_TYPE& location );

		//*****************从文件中获取数据******************
		//***********不修改链接关系,只修改内容***********
		bool LoadData(const char* SourceId, const char* szPath, const char* szTime, const char* processid);
		//************************************************************
		
		//回写到临时文件
		void Write2Temp(const char* TmpPath, const char* processid);
	
		//将内存中的文件头回写到备份文件
		void Backup(FILE* backupStream);
		
		//修改标志
		void ChangeFlag(DataType dataType);
		
		//设置指向的数据块列表的头指针
		void SetDataIndexHead(SIZE_TYPE indexNo);
		//获取指向的数据块列表的头指针
		SIZE_TYPE GetDataIndexHead();

		//设置指向的数据块列表的尾指针
		void SetDataIndexEnd(SIZE_TYPE indexNo);
		//获取指向的数据块列表的尾指针
		SIZE_TYPE GetDataIndexEnd();
		
		//设置前导指针
		void SetForward(SIZE_TYPE indexNo);
		//获取前导指针
		SIZE_TYPE GetForward();
		
		//设置后续指针
		void SetBackward(SIZE_TYPE indexNo);
		//获取后续指针
		SIZE_TYPE GetBackward();

		//校验pTime是否在此列表中
		bool IsContain(const char* pTime);
		//获取pTime在文件中的首块号
		bool GetBlockIndexNo(const char*pTime, SIZE_TYPE second, SIZE_TYPE &blockNo);
		
		DataType GetFlag();

		SIZE_TYPE GetTotalBlockCount();
		void SetTotalBlockCount(SIZE_TYPE blockcount);
		
		~CMemBlockListHead()
		{
			m_pdata = NULL;
			m_index = -1;
		};

	private:
		
		MemIndexFileHead *m_pdata;
		SIZE_TYPE m_index;
};

#endif

