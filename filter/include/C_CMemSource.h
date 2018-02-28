
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

		//获取共享内存的资源
		void AttachReSource(MemSourceHead* memSourcedata, SIZE_TYPE index);
		//释放资源
		void DetachReSource();

		inline bool IsAttached();
		
		//将资源数据清空
		void Clear();

		//修改标志
		inline void ChangeFlag(DataType dataType);
		inline DataType GetFlag();

		//设置指向的数据源
		inline void SetSourceId(const char* szSourceId);
		//获取数据源
		inline const char* GetSourceId();

		//设置指向的数据源块号
		inline void SetSourceIndex(SIZE_TYPE indexNo);
		//获取指向的数据源块号
		inline SIZE_TYPE GetSourceIndex();
		
		//设置前导指针号
		inline void SetForward(SIZE_TYPE indexNo);
		//获取前导指针号
		inline SIZE_TYPE GetForward();
		
		//设置后续指针号
		inline void SetBackward(SIZE_TYPE indexNo);
		//获取后续指针号
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

