
#ifndef _CMEMSOURCE_H_
#define _CMEMSOURCE_H_

class CMemSource
{
	public:
		CMemSource()
		{
			m_pdata = NULL;
			m_index = -1;
		}

		//��ȡ�����ڴ����Դ
		void AttachReSource(MemSourceHead* memSourcedata, SIZE_TYPE index);
		//�ͷ���Դ
		void DetachReSource();

		inline bool IsAttached();
		
		//����Դ�������
		void Clear();

		//�޸ı�־
		inline void ChangeFlag(DataType dataType);
		inline DataType GetFlag();

		//����ָ�������Դ
		inline void SetSourceId(const char* szSourceId);
		//��ȡ����Դ
		inline const char* GetSourceId();

		//����ָ�������Դ���
		inline void SetSourceIndex(SIZE_TYPE indexNo);
		//��ȡָ�������Դ���
		inline SIZE_TYPE GetSourceIndex();
		
		//����ǰ��ָ���
		inline void SetForward(SIZE_TYPE indexNo);
		//��ȡǰ��ָ���
		inline SIZE_TYPE GetForward();
		
		//���ú���ָ���
		inline void SetBackward(SIZE_TYPE indexNo);
		//��ȡ����ָ���
		inline SIZE_TYPE GetBackward();
		
		~CMemSource()
		{
			m_pdata = NULL;
			m_index = -1;
		}
		
	private:
		
		MemSourceHead *m_pdata;
		SIZE_TYPE m_index;
};

#endif

