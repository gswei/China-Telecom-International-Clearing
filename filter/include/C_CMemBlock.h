//#inculde "Filter_Common.h"

#ifndef _C_MEMBLOCK_H_
#define _C_MEMBLOCK_H_

#include "FilterCommonStruct.h"
#include "FilterCommonFunction.h"

//һ��ֻ��ͬһ�������
class CMemBlock
{
	public:
		CMemBlock()
		{
			m_pdata = NULL;
			m_index = -1;
		}

		//��ȡ�����ڴ����Դ
		void AttachReSource(MemIndexData* memIndexdata, SIZE_TYPE index);
		//�ͷ���Դ
		void DetachReSource();

		bool IsAttached();
		
		//���������
		void ClearData();

		bool FindHeaderInTemp(const char* sourceId, const char *tmpfile, const char* indexfileName, SIZE_TYPE blockno, SIZE_TYPE &location );

		//*****************���ļ��л�ȡ����*************************
		//***********���޸����ӹ�ϵ,ֻ�޸�����***********
		bool LoadData(const char* SourceId, const char* szPath, SIZE_TYPE blockNo, const char* szTime, const char*process_id);
		//************************************************************
		
		//�����ݻ�д����ʱ�ļ�
		void Write2Temp(const char* TmpPath, const char*processid);
		
		//���ڴ��е����ݻ�д�������ļ�
		//void Backup(fstream &backupStream);
		void Backup(FILE *backupStream);
		
		//�޸ı�־
		void ChangeFlag(DataType dataType);

		void SetFlag(DataType flag);
		DataType GetFlag();
		
		//����ǰ��ָ���
		void SetForward(SIZE_TYPE forward);
		//��ȡǰ��ָ���
		SIZE_TYPE GetForward();
		
		//���ú���ָ���
		void SetBackward(SIZE_TYPE backward);
		//��ȡ����ָ���
		SIZE_TYPE GetBackward();

		//��ȡ���
		SIZE_TYPE GetBlockNo();
		//���ÿ��
		void SetBlockNo(SIZE_TYPE blockno);
		char* GetBlockFileName();
		void SetBlockFileName(const char* filename);

		char* GetSourceID();
		void SetSourceID(const char* filename);

		//�Ƿ�����
		bool IsFull();
		//���ҹؼ���
		bool FindIndex(SIZE_TYPE second, const char* Key, SIZE_TYPE &index);
		//��ȡ���ֵ
		bool GetMaxValue(IndexDataValue& MValue);
		//����ؼ���
		bool InsertIndex(SIZE_TYPE second, const char* Key);
		//ɾ���ؼ���
		bool DeleteIndex(SIZE_TYPE second, const char* Key);
		
		~CMemBlock()
		{
			m_pdata = NULL;
			m_index = -1;
		}

		//��ȡ����������Ŀ
		SIZE_TYPE GetIndexCount();
		//���ÿ���������Ŀ
		void SetIndexCount(SIZE_TYPE count);

		//��ȡ�ļ������һ�����
		SIZE_TYPE GetNextBlock();
		//�����ļ������һ�����
		void SetNextBlock(SIZE_TYPE count);
		
		//��ȡ��index���ؽ�ֵָ��(�ӣ���ʼ����)
		void GetKey(SIZE_TYPE index, IndexDataValue& value);
		////��ȡ��index������������(�ӣ���ʼ����)
		SIZE_TYPE GetSecond(SIZE_TYPE index);
		//���õ�index���ؽ�ֵָ��(�ӣ���ʼ����)
		void SetKey(SIZE_TYPE index, SIZE_TYPE second, const char *key, bool isdeleteFlag);
		void SetKey(SIZE_TYPE index, const IndexDataValue &value);
		
		//��ȡ��index���ؽ�ֵ��ɾ����־(�ӣ���ʼ����)
		bool GetDeleteFlag(SIZE_TYPE index);
		//���õ�index���ؽ�ֵ��ɾ����־(�ӣ���ʼ����)
		bool SetDeleteFlag(SIZE_TYPE index, bool isDelete);
		//���ֲ���
		int BinarySearch(SIZE_TYPE second, const char* Key, SIZE_TYPE &location);
		
	private:	
		MemIndexData *m_pdata;
		SIZE_TYPE m_index;
};

#endif



