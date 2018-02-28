
#ifndef _C_MEMBLOCKLISTHEAD_H_
#define _C_MEMBLOCKLISTHEAD_H_

#include "FilterCommonStruct.h"
#include "FilterCommonFunction.h"

//һ��ֻ��ͬһ�ļ�������
class CMemBlockListHead
{
	public:
		CMemBlockListHead()
		{
			m_pdata = NULL;
			m_index = -1;
		};
		//�ӹ����ڴ��л�ȡ��Դ
		void AttachReSource(MemIndexFileHead* memIndexFileHead, SIZE_TYPE index);
		//�ͷ���Դ
		void DetachReSource();

		bool IsAttached();
		//����Դ�������
		void Clear();

		bool FindFileInTemp(const char* sourceId, const char *tmpfile, const char* indexfile, SIZE_TYPE& location );

		//*****************���ļ��л�ȡ����******************
		//***********���޸����ӹ�ϵ,ֻ�޸�����***********
		bool LoadData(const char* SourceId, const char* szPath, const char* szTime, const char* processid);
		//************************************************************
		
		//��д����ʱ�ļ�
		void Write2Temp(const char* TmpPath, const char* processid);
	
		//���ڴ��е��ļ�ͷ��д�������ļ�
		void Backup(FILE* backupStream);
		
		//�޸ı�־
		void ChangeFlag(DataType dataType);
		
		//����ָ������ݿ��б��ͷָ��
		void SetDataIndexHead(SIZE_TYPE indexNo);
		//��ȡָ������ݿ��б��ͷָ��
		SIZE_TYPE GetDataIndexHead();

		//����ָ������ݿ��б��βָ��
		void SetDataIndexEnd(SIZE_TYPE indexNo);
		//��ȡָ������ݿ��б��βָ��
		SIZE_TYPE GetDataIndexEnd();
		
		//����ǰ��ָ��
		void SetForward(SIZE_TYPE indexNo);
		//��ȡǰ��ָ��
		SIZE_TYPE GetForward();
		
		//���ú���ָ��
		void SetBackward(SIZE_TYPE indexNo);
		//��ȡ����ָ��
		SIZE_TYPE GetBackward();

		//У��pTime�Ƿ��ڴ��б���
		bool IsContain(const char* pTime);
		//��ȡpTime���ļ��е��׿��
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

