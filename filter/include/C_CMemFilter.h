
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
		//***********************���Լ��쳣��;****************************
		void Destroy(/*CDatabase *m_dbconn, CLogger *log, */char *szSourceGroupID);
		void DisplayList();
		void DisplayBlock(SIZE_TYPE Index);
		void DisplayFile(SIZE_TYPE Index);
		void ClearSemLock(const char* szsourceGroup, const char* sourceid);
		void SemLock(const char* comfile, const char* sourceid);

		//*************************************************************************
		C_CMemFilter();
		~C_CMemFilter();

		//�Ƿ����ڴ����쳣
		int IsMemError();

	private:
		//
		void CheckFileList(int i);
		//��ȡ���п�
		SIZE_TYPE GetIdlBlock();
		//���´����б�
		void ResetList();
		//��������ʽ��ȡ���������ָ�
		bool GetSourceLock(SIZE_TYPE source_idx, const char *szFileName);
		//�����һ������ʱ����ʼ�������ڴ�
		void InitMem();
		//��ȡ����Դ����
		void InitConfig();

		//�Ƿ��д�ļ�д��һ��
		bool IsWriteFileError();
		
		//��ȡʱ��len���Ⱥ���λ������ʾ������
		//��szTime=20091010030304 len=12 ���ȡ��λ��Ϊ�����0304 ���ؽ��184(3*60+4)
		SIZE_TYPE GetSecond(const char* szTime, int len);
		inline void  GetIndexFile(const char* szTime, const char* szFilename, char* resultFile);

		//�����������ļ�(ȫ·��)
		char m_szCommemFile[FILTER_FILESIZE];
		//һ������Դ�б�
		MemSourceHead *pMemSH;
		//����ʱ����б�
		MemIndexFileHead *pMemIFH;
		//����ʱ����б�
		MemIndexData *pMemID;

		char m_szDealFileName[FILTER_FILESIZE];	//���ڴ�����ļ���
		char m_szSourceid[10];					//����Դ
		char m_szProcessId[20];					//����id
		char m_szIndexPath[FILTER_FILESIZE];		//����·��
		SIZE_TYPE m_maxsource;				//�ڴ�������Դ������,add by chenyf at 20111125 for limit from process_env set
		SIZE_TYPE m_sourceindex;				//�ڴ�������Դ��index
		SIZE_TYPE m_fileindex;					//�ڴ����ļ���index

		C_CFileLock filelock;						//�ļ���(�ܿ�)
		//CF_CHash hash;

		
};



#endif

