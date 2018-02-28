
#ifndef _C_CMEMFILTER_H_
#define _C_CMEMFILTER_H_

#include <fcntl.h>

#include "C_CFileLock.h"
#include "C_CMemBlockListHead.h"
#include "C_CMemBlock.h"

const int filterSourceKey = 87692000;
const int filterFileKey = 87693000;
const int filterBlockKey = 87694000;

class C_CMemFilter
{
	public:

		void Init(const char* szIndexPath, const char *szComFile, SIZE_TYPE maxSource);
		int ProcessRecord(/*PacketParser& pps, ResParser& retValue*/);
		SIZE_TYPE GetBlock(SIZE_TYPE fileindex, SIZE_TYPE intendBlockNo, const char* szTime);
		bool AddIndex(const char*szTime, const char* key);
		bool ReMoveIndex(const char* szTime, const char* key);
		void MoveIndex(SIZE_TYPE fileindex, SIZE_TYPE fromindex, SIZE_TYPE toindex);
		SIZE_TYPE GetFileIndex(const char* szTime);
		void DealBatch(const char* sourceid);
		void DealFile(const char* sourceid, const char* filename);
		void CommitBatch();
		void CommitFile();
		void Commit();
		void HandleLastError();
		//***********************测试及异常用途****************************
		void Destroy(/*CDatabase *m_dbconn, CLogger *log, */char *szSourceGroupID);
		void DisplayList();
		void DisplayBlock(SIZE_TYPE Index);
		void DisplayFile(SIZE_TYPE Index);
		void ClearSemLock(const char* szsourceGroup, const char* sourceid);
		void SemLock(const char* comfile, const char* sourceid);

		//*************************************************************************
		C_CMemFilter();
		~C_CMemFilter();

		//是否共享内存有异常
		int IsMemError();

	private:
		//
		void CheckFileList(int i);
		//获取空闲块
		SIZE_TYPE GetIdlBlock();
		//重新处理列表
		void ResetList();
		//非阻塞方式获取进程锁并恢复
		bool GetSourceLock(SIZE_TYPE source_idx, const char *szFileName);
		//程序第一次运行时，初始化共享内存
		void InitMem();
		//获取数据源配置
		void InitConfig();

		//是否回写文件写到一半
		bool IsWriteFileError();
		
		//截取时间len长度后面位数所表示的秒数
		//如szTime=20091010030304 len=12 则截取的位数为后面的0304 返回结果184(3*60+4)
		SIZE_TYPE GetSecond(const char* szTime, int len);
		inline void  GetIndexFile(const char* szTime, const char* szFilename, char* resultFile);

		//创建索引的文件(全路径)
		char m_szCommemFile[FILTER_FILESIZE];
		//一级数据源列表
		MemSourceHead *pMemSH;
		//二级时间段列表
		MemIndexFileHead *pMemIFH;
		//三级时间块列表
		MemIndexData *pMemID;

		char m_szDealFileName[FILTER_FILESIZE];	//正在处理的文件名
		char m_szSourceid[10];					//数据源
		char m_szProcessId[20];					//进程id
		char m_szIndexPath[FILTER_FILESIZE];		//索引路径
		SIZE_TYPE m_maxsource;				//内存中数据源最大个数,add by chenyf at 20111125 for limit from process_env set
		SIZE_TYPE m_sourceindex;				//内存中数据源的index
		SIZE_TYPE m_fileindex;					//内存中文件的index

		C_CFileLock filelock;						//文件锁(总控)
		//CF_CHash hash;

		
};



#endif

