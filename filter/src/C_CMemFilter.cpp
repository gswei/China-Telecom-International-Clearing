
#include "C_CMemFilter.h"

C_CMemFilter::C_CMemFilter()
{
	pMemID = NULL;
	pMemIFH = NULL;
	pMemSH = NULL;
	memset(m_szCommemFile, 0, sizeof(m_szCommemFile));
	memset(m_szIndexPath, 0, sizeof(m_szIndexPath));
	memset(m_szSourceid, 0, sizeof(m_szSourceid));
	m_sourceindex = -1;
	m_fileindex = -1;
}

C_CMemFilter::~C_CMemFilter()
{
	
}

//打印 测试用
void C_CMemFilter::DisplayFile(SIZE_TYPE fileIndex)
{
	if(fileIndex > FILTER_MAXLOADFILE|| fileIndex < 0)
	{
		cout << "out of range" <<endl;
		return;
	}
	cout << "\t sourceid:" << pMemIFH[fileIndex].indexFileHead.szSourceId<<endl;
	cout << "\t time:" << pMemIFH[fileIndex].indexFileHead.szTime<<endl;
	cout << "\t file:" << pMemIFH[fileIndex].indexFileHead.szFileName<<endl;
	cout << "\t blockcount:" << pMemIFH[fileIndex].indexFileHead.blockInFile<<endl;
	cout << "\t forward:" << pMemIFH[fileIndex].forward<<endl;
	cout << "\t backward:" << pMemIFH[fileIndex].backward<<endl;
	cout << "\t head:" << pMemIFH[fileIndex].memDataListHead<<endl;
	cout << "\t end:" << pMemIFH[fileIndex].memDataListEnd<<endl;
	
}

//打印 测试用
void C_CMemFilter::DisplayBlock(SIZE_TYPE Index)
{
	if(Index > m_maxsource*FILTER_MAXLOADBLOCK*FILTER_MAXLOADFILE|| Index < 0)
	{
		cout << "out of range" <<endl;
		return;
	}
	switch(pMemID[Index].type)
	{
			case IDLE:     cout << "data flag:IDLE"<<endl; break;
			case INTEMP: cout << "data flag:INTEMP"<<endl; break; 
			case INFILE:  cout << "data flag:INFILE"<<endl; break;
			case INMEM:  cout << "data flag:INMEM"<<endl; break;
			default: cout << "data flag:ERROR"<<endl;break;
	}
	cout << "\t sourceid:" << pMemID[Index].indexData.dataHead.szSourceId<<endl;
	cout << "\t time:" << pMemID[Index].indexData.dataHead.second<<endl;
	cout << "\t file:" << pMemID[Index].indexData.dataHead.szFileName<<endl;
	cout << "\t valuecount:" << pMemID[Index].indexData.dataHead.indexInBlock<<endl;
	cout << "\t forward:" << pMemID[Index].forward<<endl;
	cout << "\t backward:" << pMemID[Index].backward<<endl;
	cout << "\t blockNoInFile:" << pMemID[Index].indexData.dataHead.blockNo<<endl;
	cout << "\t nextBlock:" << pMemID[Index].indexData.dataHead.nextBlock<<endl;

	for(int i=0; i<pMemID[Index].indexData.dataHead.indexInBlock; i++)
	{
		cout << pMemID[Index].indexData.data[i].second <<":"<<pMemID[Index].indexData.data[i].szIndexValue<<endl;
	}
}

//打印共享内存链接关系
void C_CMemFilter::DisplayList()
{
	long j=0;
	/*for(j=0; j<m_maxsource*FILTER_MAXLOADFILE; j++)
	{
		cout << "file index:" << j<<endl;
		switch(pMemIFH[j].type)
		{
			case IDLE:     cout << "file flag:IDLE"<<endl; break;
			case INTEMP: cout << "file flag:INTEMP"<<endl; break;
			case INFILE:  cout << "file flag:INFILE"<<endl; break;
			case INMEM:  cout << "file flag:INMEM"<<endl;break;
			default: cout << "file flag:ERROR"<<endl;break;
		}
		cout << "file time:" << pMemIFH[j].indexFileHead.szTime<<endl;
		cout << "file file:" << pMemIFH[j].indexFileHead.szFileName<<endl;
		cout << "file forward:" << pMemIFH[j].forward<<endl;
		cout << "file backward:" << pMemIFH[j].backward<<endl;
		cout << "file head:" << pMemIFH[j].memDataListHead<<endl;
		cout << "file end:" << pMemIFH[j].memDataListEnd<<endl;
	}
	for(j=0; j<m_maxsource*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK; j++)
	{
		cout << "data index:" << j << endl;
		switch(pMemID[j].type)
		{
			case IDLE:     cout << "data flag:IDLE"<<endl; break;
			case INTEMP: cout << "data flag:INTEMP"<<endl; break; 
			case INFILE:  cout << "data flag:INFILE"<<endl; break;
			case INMEM:  cout << "data flag:INMEM"<<endl; break;
			default: cout << "data flag:ERROR"<<endl;break;
		}
		cout << "data source:" << pMemID[j].indexData.dataHead.szSourceId<<endl;
		cout << "data indexCount:" << pMemID[j].indexData.dataHead.indexInBlock<<endl;
		cout << "data file:" << pMemID[j].indexData.dataHead.szFileName<<endl;
		cout << "data time:" << pMemID[j].szTime<<endl;
		cout << "data forward:" << pMemID[j].forward<<endl;
		cout << "data backward:" << pMemID[j].backward<<endl;
	}
	*/

	cout << "sizeof(pMemID[0].indexData)="<<sizeof(pMemID[0].indexData)<<endl;
	cout << "sizeof(IndexData)="<<sizeof(IndexData)<<endl;


	cout << "sizeof(m_pdata[m_index].indexFileHead)="<<sizeof(pMemIFH[0].indexFileHead)<<endl;
	cout << "sizeof(IndexFileHead)="<<sizeof(IndexFileHead)<<endl;
	for(j=0; j<m_maxsource; j++)
	{
		cout << "source index:" << j<<endl;
		cout << "source sourceid:" << pMemSH[j].szSourceId<<endl;
		cout << "source flag:" << pMemSH[j].type<<endl;
		cout << "source filename:" << pMemSH[j].szDealFileName<<endl;
		cout << "source begin:" << pMemSH[j].beginIndex<<endl;
		cout << "source end:" << pMemSH[j].endIndex<<endl;
		cout << "source type:" << pMemSH[j].type<<endl;
		cout << "source idle head:" << pMemSH[j].idleBlockHead<<endl;
		cout << "source idle end:" << pMemSH[j].idleBlockEnd<<endl;
		int k = pMemSH[j].idleBlockHead;
		cout << "IDLE list = " ;
		for(; k!=-1; k=pMemID[k].backward)
		{
			cout << "->[" <<pMemID[k].forward<<"]" <<k << "["<<pMemID[k].backward<<"]";
		}
		cout << endl;
		
		k = pMemSH[j].beginIndex;
		for(; k!=-1; k=pMemIFH[k].backward)
		{
			cout <<"\tFILE_LIST: " << k << " datacount =" << pMemIFH[k].memDataCount<<endl;
			cout <<"\tFILE_LIST: " << k << " beginDataIndex =" << pMemIFH[k].memDataListHead<<endl;
			cout <<"\tFILE_LIST: " << k << " endDataIndex =" << pMemIFH[k].memDataListEnd<<endl;
			int m = pMemIFH[k].memDataListHead;
			cout << "\t\t" ;
			for(; m!=-1; m=pMemID[m].backward)
			{
				cout << "->[" <<pMemID[m].forward << "]" << m<<"["<<pMemID[m].backward<<"]";
			}
			cout << endl;
		}
		
	}
	
}

//删除共享内存
void C_CMemFilter::Destroy(char *szMemFile)
{
	key_t ikey = ftok(szMemFile, FILTER_SOURCE_INDEX); 
	int ishmid = shmget(ikey, 0, 0666);
	if(ishmid != -1)
	{
		theJSLog << "source shmid =" << ishmid <<endi;
		shmctl(ishmid,IPC_RMID,NULL) ;
	}

	ikey = ftok(szMemFile, FILTER_FILE_INDEX); 
	ishmid = shmget(ikey, 0, 0666);
	if(ishmid != -1)
	{
		theJSLog << "file shmid =" << ishmid <<endi;
		shmctl(ishmid,IPC_RMID,NULL) ;
	}


	ikey = ftok(szMemFile, FILTER_BLOCK_INDEX); 
	ishmid = shmget(ikey, 0, 0666);
	if(ishmid != -1)
	{
		theJSLog << "block shmid =" << ishmid <<endi;
		shmctl(ishmid,IPC_RMID,NULL) ;
	}
}

void C_CMemFilter::Init(const char* szIndexPath, const char *szComFile, SIZE_TYPE maxSource)
{
	//初始化配置容器;公式编辑器等
	//获取创建共享内存的文件路径和索引存储目录
	strcpy(m_szIndexPath, szIndexPath);
	strcpy(m_szCommemFile, szComFile);
	cout<<"m_szIndexPath:"<<m_szIndexPath<<endl;
	cout<<"m_szCommemFile:"<<m_szCommemFile<<endl;
	m_maxsource = (maxSource > 0)?maxSource:FILTER_MAXSOURCE ;
	//strcpy(m_szProcessId, "filter");
	filelock.Init(m_szCommemFile);
	InitConfig();
	//加全局锁
	filelock.Lock();
	
	errno = 0;
	
	//key_t ikey = ftok(m_szCommemFile, FILTER_SOURCE_INDEX); 
	key_t ikey = filterSourceKey;
	theJSLog << "shm ikey == " << ikey << endl;
	/*
	if (errno != 0)
	{
		perror("ftok");
		//TODO throw
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "get key_t error ! index=[%d], path=[%s]",FILTER_SOURCE_INDEX, m_szCommemFile);
		throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		
	}
	*/
	errno = 0;
	int ishmid = shmget(ikey, m_maxsource*sizeof(MemSourceHead), IPC_CREAT|IPC_EXCL|0666);
	if(errno == EEXIST)
	{
		// 存在
		//连接数据源
		long count = m_maxsource;
		theJSLog << "connecting source..." <<endi;
		errno = 0;
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		//vivi edit
		//theJSLog<<"before shmat pMemSH:"<<pMemSH<<endd;
		if(pMemSH==NULL)
			pMemSH = (MemSourceHead*)shmat(ishmid, NULL, 0);
		else
			pMemSH = (MemSourceHead*)shmat(ishmid, pMemSH, 0);
		//theJSLog<<"agter  shmat pMemSH:"<<pMemSH<<endd;
		if(errno != 0)
		{
			perror("shmget");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "shmget error ! ikey=[%d],errno=%d",ikey,errno);
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}
		//连接文件头
		theJSLog << "connecting file..." <<endi;
		errno = 0;
		//ikey = ftok(m_szCommemFile, FILTER_FILE_INDEX); 
		key_t ikey = filterFileKey;
		count =  FILTER_MAXLOADFILE*m_maxsource;
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		errno = 0;
		//vivi edit
		if(pMemIFH==NULL)
			pMemIFH= (MemIndexFileHead*)shmat(ishmid, NULL, 0);
		else
			pMemIFH= (MemIndexFileHead*)shmat(ishmid, pMemIFH, 0);
			
		if(errno != 0)
		{
			perror("shmat");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "shmat error ! ");
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}

		//连接数据块
		theJSLog << "connecting block..." <<endi;
		errno = 0;
		//ikey = ftok(m_szCommemFile, FILTER_BLOCK_INDEX); 
		ikey = filterBlockKey;
		count = FILTER_MAXLOADFILE*m_maxsource*FILTER_MAXLOADBLOCK;
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		errno = 0;
		//vivi edit
		if(pMemID==NULL)
			pMemID = (MemIndexData*)shmat(ishmid, NULL, 0);
		else
			pMemID = (MemIndexData*)shmat(ishmid, pMemID, 0);
			
		if(errno != 0)
		{
			perror("shmat");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "shmat error ! ");
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}
		
	}
	else if(errno == 0)
	{
		// 不存在全部创建并赋初始值
		theJSLog << "creating source.. " <<endi;
		long count = m_maxsource;
		pMemSH = (MemSourceHead*)shmat(ishmid, NULL, 0);

		//连接文件头
		theJSLog << "creating file.. " <<endi;
		errno = 0;
		//ikey = ftok(m_szCommemFile, FILTER_FILE_INDEX); 
		ikey = filterFileKey;
		/*
		if (errno != 0)
		{
			perror("ftok");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "get key_t error ! index=[%d], path=[%s]",FILTER_FILE_INDEX, m_szCommemFile);
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}
		*/
		count =  FILTER_MAXLOADFILE*m_maxsource;
		errno =0;
		ishmid = shmget(ikey, count*sizeof(MemIndexFileHead), IPC_CREAT|0666);
		if (errno != 0)
		{
			perror("shmget");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "shmget error ! index=[%d], path=[%s]",FILTER_FILE_INDEX, m_szCommemFile);
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}
		errno = 0;
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, NULL, 0);
		if (errno != 0)
		{
			perror("shmat");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "shmat error ! index=[%d], path=[%s]",FILTER_FILE_INDEX, m_szCommemFile);
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}

		//连接数据块
		theJSLog << "creating block.. " <<endi;
		//ikey = ftok(m_szCommemFile, FILTER_BLOCK_INDEX); 
		ikey = filterBlockKey;
		count = FILTER_MAXLOADFILE*m_maxsource*FILTER_MAXLOADBLOCK;
		errno = 0;
		ishmid = shmget(ikey, count*sizeof(MemIndexData), IPC_CREAT|0666);
		if (errno != 0)
		{
			perror("shmget");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "shmat error ! index=[%d], path=[%s]",FILTER_BLOCK_INDEX, m_szCommemFile);
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}
		errno = 0;
		pMemID = (MemIndexData*)shmat(ishmid, NULL, 0);
		if (errno != 0)
		{
			perror("shmat");
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "shmat error ! index=[%d], path=[%s]",FILTER_BLOCK_INDEX, m_szCommemFile);
			throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		}
		InitMem();
		
	}
	else if(errno != 0)
	{
		perror("shmget");
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "shmget error ! index=[%d], path=[%s]",FILTER_SOURCE_INDEX, m_szCommemFile);
		throw CException(FILTER_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
	}

	filelock.UnLock();
	
}

// 函数说明: 处理一条文件记录做去重处理
// 参数:
// 返回值int:
// 		添加索引模式时	:0表示添加成功; 1表示已有相同的有效记录
//		去除索引模式时	:0表示删除成功; 1表示原来没有相同的有效记录
int C_CMemFilter::ProcessRecord(/*PacketParser& pps, ResParser& retValue*/)
{
	// 获取时间和key值
	// 1查找时间段
	// 2获取数据块
	// 3块内查找关键字
	
	// szDealType 处理模式 2表示去除索引;1表示添加索引;
/*
	//第一个是处理方式
	char szDealType[ATTRINUMBER];
	memset(szDealType, 0, sizeof(szDealType));
	pps.getFieldValue(1, szDealType);
	if(strcmp(szDealType, "1")!=0 && strcmp(szDealType, "2")!=0)
	{
		//throw here
	}
	//第二个是时间
	char szTime[ATTRINUMBER];
	memset(szTime, 0, sizeof(szTime));
	pps.getFieldValue(2, szTime);
	
	int length = pps.getItem_num();
	char szTemp[ATTRINUMBER];
	char szOrgKey[256] = "";
	for(int i=3; i<length; i++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		pps.getFieldValue(i, szTemp);
		strcat(szOrgKey, szTemp);
	}
	hash.getHashLen(strlen(szOrgKey));
	char szKey[20];
	memset(szKey, 0, sizeof(szKey));
	hash.getHashStr(szKey, szOrgKey, strlen(szOrgKey));

	//添加索引
	if(strcmp(szDealType, "1")==0)
	{
		
	}
	//删除索引
	else if(strcmp(szDealType, "2")==0)
	{
		
	} 
*/
	return 0;
}

//根据时间获取秒数
SIZE_TYPE C_CMemFilter::GetSecond(const char* szTime, int len)
{
	if(szTime==NULL || strlen(szTime)!=14 || len > 14 || len < 10)
	{
		//TODO:throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "time=%s= 不是合法时间", szTime);
		throw CException(FILTER_ERR_IN_TIME_FIELD_FORMAT, szMsg, __FILE__, __LINE__);
	}
	char szTemp[6];
	if(len >= 12)
	{
		strcpy(szTemp, szTime+len);
		return atoi(szTemp);
	}
	else
	{
		memset(szTemp, 0, sizeof(szTemp));
		strncpy(szTemp, szTime+len, 12-len);
		SIZE_TYPE second = atoi(szTemp)*60;
		strcpy(szTemp, szTime+12);
		second = second+atoi(szTemp);
		second = second%3600;
		return second;
	}
}

//获取内存中指定的数据块
//输入: 	fileindex 内存中文件信息的index值
//			intendBlockNo 文件中的第几块数据块
//			szTime 去重时间
//返回:	SIZE_TYPE 所寻找块在内存中的index值
SIZE_TYPE C_CMemFilter::GetBlock(SIZE_TYPE fileindex, SIZE_TYPE intendBlockNo, const char* szTime)
{
	CMemBlockListHead blocklisthead;
	blocklisthead.AttachReSource(pMemIFH, fileindex);
	SIZE_TYPE head = blocklisthead.GetDataIndexHead();
	SIZE_TYPE end = blocklisthead.GetDataIndexEnd();
	SIZE_TYPE curr = head;

	CMemBlock block;
	for(; curr!=(SIZE_TYPE)-1; curr=pMemID[curr].backward)
	{
		block.AttachReSource(pMemID, curr);
		if(block.GetFlag()!=IDLE && block.GetBlockNo() == intendBlockNo)
		{
			break;
		}
		else if(block.GetFlag() == IDLE)
		{
			block.LoadData(m_szSourceid, m_szIndexPath, intendBlockNo, szTime, m_szProcessId);
			//theJSLog << '\t' << "block.LoadData time=" << mytime.GetTime() <<"=" <<endd;
			//block.ChangeFlag(INFILE);
			break;
		}
	}
	
	if(head == curr) 
	{
		return head;
	}
	else if(curr != (SIZE_TYPE)-1 && curr != pMemIFH[fileindex].memDataListEnd)
	{
		SIZE_TYPE myforward = pMemID[curr].forward;
		SIZE_TYPE mybackward = pMemID[curr].backward;

		pMemID[myforward].backward = mybackward;
		pMemID[mybackward].forward = myforward;
		pMemID[curr].backward = head;
		pMemID[head].forward = curr;
		pMemID[curr].forward = -1;
		pMemIFH[fileindex].memDataListHead= curr;

		return curr;
	}
	//else if(curr == pMemIFH[fileindex].memDataListEnd || curr == -1)
	//与上面含义相同
	else if(curr == pMemIFH[fileindex].memDataListEnd)
	{
		SIZE_TYPE myforward = pMemID[curr].forward;
		pMemID[myforward].backward = -1;
		pMemID[curr].backward = head;
		pMemID[head].forward = curr;
		pMemID[curr].forward = -1;
		pMemIFH[fileindex].memDataListHead= curr;
		pMemIFH[fileindex].memDataListEnd= myforward;
		return curr;
	}
	else if(curr == (SIZE_TYPE)-1)
	{
		//获取新块
		curr = GetIdlBlock();
		SIZE_TYPE oldHead = pMemIFH[fileindex].memDataListHead;
		pMemID[oldHead].forward = curr;
		pMemID[curr].backward = oldHead;
		pMemID[curr].forward = -1;
		pMemIFH[fileindex].memDataListHead = curr;
		pMemIFH[fileindex].memDataCount++;
		
		CMemBlock tempblock;
		tempblock.AttachReSource(pMemID, curr);
		tempblock.LoadData(m_szSourceid, m_szIndexPath, intendBlockNo, szTime, m_szProcessId);

		return curr;
	}
	else
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "link error!");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return -1;
}

//调用接口 新增索引
//输入: 	szTime 去重时间
//			key	去重关键字字串
//返回:	bool 是否新增成功
bool C_CMemFilter::AddIndex(const char*szTime, const char* key)
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存数据源块");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		theJSLog<< "can't get source info" <<ende;
	}
	SIZE_TYPE second = GetSecond(szTime, FILTER_INDEX_FILE_LEN);
	SIZE_TYPE fileindex = GetFileIndex(szTime);
	m_fileindex = fileindex;
	CMemBlockListHead blocklisthead;
	blocklisthead.AttachReSource(pMemIFH, fileindex);
	SIZE_TYPE intendblockno = -1;
	blocklisthead.GetBlockIndexNo(szTime, second, intendblockno);
	CMemBlock block ;
	SIZE_TYPE indexposition;
	int count = 0;
	while(true)
	{
		count++;
		indexposition = GetBlock(fileindex, intendblockno, szTime);
		block.AttachReSource(pMemID, indexposition);
		if(block.GetNextBlock() == (SIZE_TYPE)-1)
		{
			break;
		}
		IndexDataValue value;
		block.GetMaxValue(value);
		if(second<value.second || (second==value.second && memcmp(key, value.szIndexValue, FILTER_VALUESIZE+1) <= 0))
		{
			break;
		}
		intendblockno = block.GetNextBlock();
	}
	if(block.IsFull())
	{
		//theJSLog << "block is full!" <<endi;
		//分裂 移动
		blocklisthead.SetTotalBlockCount(blocklisthead.GetTotalBlockCount()+1);
		SIZE_TYPE blocknoinfile = blocklisthead.GetTotalBlockCount()-1;

		SIZE_TYPE tempIndexPosition = GetBlock(fileindex, blocknoinfile, szTime);

		CMemBlock tempblock;
		tempblock.AttachReSource(pMemID, tempIndexPosition);
		tempblock.SetNextBlock(block.GetNextBlock());
		block.SetNextBlock(tempblock.GetBlockNo());
		//CheckFileList(9);
		MoveIndex(fileindex, indexposition, tempIndexPosition);
		//CheckFileList(10);
		IndexDataValue value;
		block.GetMaxValue(value);
		if(second>value.second || (second==value.second && memcmp(key, value.szIndexValue, FILTER_VALUESIZE+1)>0))
		{
			indexposition = tempIndexPosition;
		}
	}
	SIZE_TYPE location;
	block.AttachReSource(pMemID, indexposition);
	bool tempbool = block.InsertIndex(second, key);
	return tempbool;
}

//将数据块[fromindex]中的部分索引移动到数据块[toindex]
//数据块[index]原有将被清除

//输入: 	fileindex 文件信息在内存中的index值
//			fromindex 源数据块在内存中的index值
//			toindex 目标数据块在内存中的index值
//返回:	无
void C_CMemFilter::MoveIndex(SIZE_TYPE fileindex, SIZE_TYPE fromindex, SIZE_TYPE toindex)
{	
	CMemBlock fromblock, toblock;
	fromblock.AttachReSource(pMemID, fromindex);
	toblock.AttachReSource(pMemID, toindex);

	//CheckFileList(20);
	toblock.SetIndexCount(fromblock.GetIndexCount()/5);
	SIZE_TYPE fromEnd = fromblock.GetIndexCount()-1;
	SIZE_TYPE toEnd = toblock.GetIndexCount()-1;

	//拷贝五分之一过去新块
	IndexDataValue value;
	for(; toEnd >=0; toEnd--, fromEnd--)
	{
		fromblock.GetKey(fromEnd, value);
		toblock.SetKey(toEnd, value);
	}
	//CheckFileList(21);
	//设置就块索引数目
	fromblock.SetIndexCount(fromblock.GetIndexCount() - toblock.GetIndexCount());

	SIZE_TYPE sourceSecond = fromblock.GetSecond(fromblock.GetIndexCount()-1);
	sourceSecond++;
	//CheckFileList(22);
	SIZE_TYPE targetSecond = toblock.GetSecond(toblock.GetIndexCount()-1);
	if(toblock.GetNextBlock() == (SIZE_TYPE)-1)
	{
		targetSecond = FILTER_MAXSECONDINFILE-1;
	}
	//CheckFileList(23);
	for(SIZE_TYPE i=sourceSecond; i<=targetSecond; i++)
	{
		pMemIFH[fileindex].indexFileHead.blockItem[i] = toblock.GetBlockNo();
	}
	try
	{
		CheckFileList(24);
	}
	catch(CException &e)
	{
		theJSLog << "fileindex = " << fileindex<<endi;
		theJSLog << "targetSecond = " << targetSecond<<endi;
		theJSLog << "sourceSecond = " << sourceSecond<<endi;
		theJSLog << "toblock.GetBlockNo() = " << toblock.GetBlockNo()<<endi;
		throw e;
	}
	pMemIFH[fileindex].type = INMEM;
	toblock.ChangeFlag(INMEM);
	fromblock.ChangeFlag(INMEM);
	//CheckFileList(25);
}

//根据时间 获取相应的内存文件块序号
//输入: 	szTime 去重时间
//返回:	SIZE_TYPE 文件信息在内存中的index值

SIZE_TYPE C_CMemFilter::GetFileIndex(const char* szTime)
{
	SIZE_TYPE head = pMemSH[m_sourceindex].beginIndex;
	SIZE_TYPE curr = head;
	CMemBlockListHead listhead;
	for(; curr!=(SIZE_TYPE)-1; curr=pMemIFH[curr].backward)
	{
		listhead.AttachReSource(pMemIFH, curr);
 		if(listhead.GetFlag()!=IDLE && listhead.IsContain(szTime))
		{
			//找到了
			break;
		}
		else if(listhead.GetFlag() == IDLE)
		{
			listhead.LoadData(m_szSourceid, m_szIndexPath, szTime, m_szProcessId);
			//listhead.ChangeFlag(INFILE);
			break;
		}
	}

	if(head == curr) 
	{
		return head;
	}
	else if(curr != (SIZE_TYPE)-1 && curr != pMemSH[m_sourceindex].endIndex)
	{
		SIZE_TYPE myforward = pMemIFH[curr].forward;
		SIZE_TYPE mybackward = pMemIFH[curr].backward;

		pMemIFH[myforward].backward = mybackward;
		pMemIFH[mybackward].forward = myforward;
		pMemIFH[curr].backward = head;
		pMemIFH[head].forward = curr;
		pMemIFH[curr].forward = -1;
		pMemSH[m_sourceindex].beginIndex = curr;

		return curr;
	}
	//else if(curr == pMemSH[sourceindex].endIndex || curr == -1)
	else		//与上面含义相同
	{
		if(curr == (SIZE_TYPE)-1)
		{
			CMemBlockListHead templisthead;
			templisthead.AttachReSource(pMemIFH, pMemSH[m_sourceindex].endIndex);
			templisthead.Write2Temp(m_szIndexPath, m_szProcessId);
		
			SIZE_TYPE currblock = templisthead.GetDataIndexHead();
			CMemBlock  block;
			for(; currblock!=(SIZE_TYPE)-1; currblock=pMemID[currblock].backward)
			{
				block.AttachReSource(pMemID, currblock);
				block.Write2Temp(m_szIndexPath, m_szProcessId);
				block.SetFlag(IDLE);
			}
			templisthead.LoadData(m_szSourceid, m_szIndexPath, szTime, m_szProcessId);
			curr = pMemSH[m_sourceindex].endIndex;
		}
		SIZE_TYPE myforward = pMemIFH[curr].forward;
		pMemIFH[myforward].backward = -1;
		pMemIFH[curr].backward = head;
		pMemIFH[head].forward = curr;
		pMemIFH[curr].forward = -1;
		pMemSH[m_sourceindex].beginIndex = curr;
		pMemSH[m_sourceindex].endIndex = myforward;
		return curr;
	}

	
	
}

//登记数据源
//根据时间 获取相应的内存文件块序号
//输入: 	sourceid 数据源
//返回:	无
//异常:	数据源个数超过最大值
void C_CMemFilter::DealBatch(const char* sourceid)
{
	//获取数据源信息和上次处理状态
	//没有就添加数据源
	//上次处理失败 清理数据
	
	//按照数据源id来写进程id
	strcpy(m_szProcessId, sourceid);
	strcpy(m_szSourceid, sourceid);
	filelock.Lock();
	int i=0;
	for(i=0; i<m_maxsource; i++)
	{
		if(pMemSH[i].type == IDLE)
		{
			strcpy(pMemSH[i].szSourceId, m_szSourceid);
			pMemSH[i].type = INMEM;
			break;
		}
		if(strcmp(pMemSH[i].szSourceId, m_szSourceid) == 0)
		{
			break;
		}
	}
	m_sourceindex = i;
	filelock.UnLock();
	if(m_sourceindex >= m_maxsource)
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "数据源个数超出范围");
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	
}

//开始处理文件;取得数据源锁
//输入: 	sourceid 数据源
//			filename 正在处理文件名
//返回:	无
//异常:	无法连接上共享内存
void C_CMemFilter::DealFile(const char* sourceid, const char* filename)
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存数据源块");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return;
	}

	char szTempDir[FILTER_FILESIZE];
	sprintf(szTempDir, "%s%s/%s/", m_szIndexPath, m_szSourceid, FILTER_WORK_PATH);
	chkAllDir(szTempDir);
	
	//对数据源加锁
	int ret ;
	theJSLog << "检查数据源锁.... " <<endi;
	if(GetSourceLock(m_sourceindex, filename) == false)
	{
	  theJSLog << "给数据源加锁.... " <<endi;
		while((ret=sem_wait(&(pMemSH[m_sourceindex].semid))) != 0)
		{
			//加锁另外的文件锁
			theJSLog << "给文件加锁.... " <<endi;
			filelock.Lock();
			if(-1 == kill(pMemSH[m_sourceindex].iprocessId, 0))
			{
				theJSLog << "加锁的进程pid(" << pMemSH[m_sourceindex].iprocessId << ")不存在" <<endi;
				if(IsWriteFileError())
				{
					theJSLog <<"文件回写到一半" << endi;
					//已经写到一半了,只能阻塞到该文件
					if(strcmp(pMemSH[m_sourceindex].szDealFileName, filename) == 0)
					{
						theJSLog << "处理上次错误"<<endi;
						HandleLastError();
						//登记独占此数据源的进程id
						pMemSH[m_sourceindex].iprocessId = getpid();
						//解锁另外的文件锁
						filelock.UnLock();
						break;
					}
					else
					{
						theJSLog << pMemSH[m_sourceindex].szDealFileName
						<<"文件提交到一半,只能等待该文件重新处理完成后才停止阻塞!!!!"<<endw;
					}
				}
				else
				{
					theJSLog <<"文件还没有回写" << endi;
					theJSLog << "处理上次错误"<<endi;
					HandleLastError();
					//登记独占此数据源的进程id
					pMemSH[m_sourceindex].iprocessId = getpid();
					//解锁另外的文件锁
					filelock.UnLock();
					break;
				}
			}
			else
			{
				theJSLog << "进程pid(" << pMemSH[m_sourceindex].iprocessId << ")存在" <<endi;
				//进程存在则一直阻塞
			}
			//解锁另外的文件锁
			filelock.UnLock();
			theJSLog << "等待其他进程...."<<endi;
			//sleep(10);
		}
	}

  //检查数据源的共享内存合法性
//	int err___ = IsMemError();
//  if (err___)
//	{
//	  char szMsg[FILTER_ERRMSG_LEN];
//	  		sprintf(szMsg, "共享内存异常，123 jiangjz:%d", err___);
//	  		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
//	  		return;
//		theJSLog << "共享内存异常,重整共享内存..." << endi;
//		HandleLastError();
//		theJSLog << "重新梳理内存完毕" << endi;
//	}
	//此处已经获得数据源锁
	//登记独占此数据源的进程id
	pMemSH[m_sourceindex].iprocessId = getpid();
	strcpy(pMemSH[m_sourceindex].szDealFileName, filename);
	strcpy(m_szDealFileName, filename);
	theJSLog << "已获得数据源锁，登记进程ID:" << pMemSH[m_sourceindex].iprocessId << ",开始处理文件：" << filename << endi;
	
}

//批次提交
void C_CMemFilter::CommitBatch()
{
	
}

//提交文件
//备份内存中的索引
//将临时文件,内存中的去重索引写入正式文件中
//并将内存中的索引清除;
//释放数据源锁

//输入: 	无
//返回:	无
//异常:	无法连接上共享内存或文件读写失败等
void C_CMemFilter::CommitFile()
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存数据源块");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return;
	}
	
	//检查数据源的共享内存合法性
//	int err___ = IsMemError();
//  if (err___)
//	{
//		char szMsg[FILTER_ERRMSG_LEN];
//		sprintf(szMsg, "共享内存异常，文件需要重处理:jiangjz:code=%d", err___);
//		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
//		return;
//	}
	
	//写备份文件
	//*******************文件头信息*******************************
	char backupFile[FILTER_FILESIZE];
	sprintf(backupFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_FILEINFO_NAME);
	truncFile(backupFile);
	FILE *backupFileStream =  openfile(backupFile, "rb+");
	//******************************************************************

	//*******************数据块信息*******************************
	char backupDataFile[FILTER_FILESIZE];
	sprintf(backupDataFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_DATA_NAME);
	truncFile(backupDataFile);
	FILE *backupdataStream = openfile(backupDataFile, "rb+");
	CMemBlock block;
	//******************************************************************

	//遍历各文件头
	SIZE_TYPE curr = pMemSH[m_sourceindex].beginIndex;
	CMemBlockListHead listhead;	
	for(; curr!=(SIZE_TYPE)-1; curr=pMemIFH[curr].backward)
	{
		listhead.AttachReSource(pMemIFH, curr);
		listhead.Backup(backupFileStream);
		//遍历各数据块
		SIZE_TYPE datacurr = listhead.GetDataIndexHead();
		for(; datacurr!=(SIZE_TYPE)-1; datacurr=pMemID[datacurr].backward)
		{
			block.AttachReSource(pMemID, datacurr);
			block.Backup(backupdataStream);
		}
	}
	fclose(backupFileStream);
	fclose(backupdataStream);

	Commit();
	sem_post(&(pMemSH[m_sourceindex].semid));
	
	//vivi edit 
	shmdt(pMemSH);
	pMemSH = NULL;
	shmdt(pMemIFH);
	pMemIFH = NULL;
	shmdt(pMemID);
	pMemID = NULL;
	
}


//提交
//将临时文件,内存中的去重索引写入正式文件中
//并将内存中的索引清除;

//输入: 	无
//返回:	无
//异常:	无法连接上共享内存或文件读写失败等
void C_CMemFilter::Commit()
{
	//******************标志位文件*****************************
	char commitFile[FILTER_FILESIZE];
	sprintf(commitFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, COMMIT_FILE);
	FILE * CommitFlagStream = openfile(commitFile, "rb+");
	char flag = '1';
	errno = 0;
	int iwrite = fwrite(&flag, sizeof(char), 1, CommitFlagStream);
	if(iwrite != 1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "写标志文件=%s=出错", commitFile);
		throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
	}
	fclose(CommitFlagStream);
	//***************************************************************

	//****************处理文件头*************************
	char tmpFile[FILTER_FILESIZE];
	sprintf(tmpFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_FILEINFO_NAME);
	FILE *tmpFileStream = openfile(tmpFile, "rb+");
	char szIndexFile[FILTER_FILESIZE];
	IndexFileHead tmpfilehead;
	//遍历临时文件文件头信息
	for (; ;)
	{
		int rget = fread(&tmpfilehead, sizeof(IndexFileHead), 1, tmpFileStream);
		if(rget != 1)
		{
			break;
		}
		//********************正式索引文件*********************
		GetIndexFile(tmpfilehead.szTime, tmpfilehead.szFileName, szIndexFile);
		theJSLog<< "写正式索引文件:" << szIndexFile << endi;
		//************************************************************

		FILE * indexStream = openfile(szIndexFile, "rb+");
		errno = 0;
		int iwrite = fwrite(&tmpfilehead, sizeof(IndexFileHead), 1, indexStream);
		if(iwrite != 1)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		}
		fclose(indexStream);
	}
	fclose(tmpFileStream);

	//遍历内存各文件头信息
	SIZE_TYPE curr = pMemSH[m_sourceindex].beginIndex;
	for(; curr!=(SIZE_TYPE)-1; curr=pMemIFH[curr].backward)
	{
		if(pMemIFH[curr].type == INMEM)
		{
			//********************正式索引文件*********************
			GetIndexFile(pMemIFH[curr].indexFileHead.szTime, pMemIFH[curr].indexFileHead.szFileName, szIndexFile);
			theJSLog<< "写正式索引文件2，:" << szIndexFile << endi;
			//************************************************************

			FILE *indexStream = openfile(szIndexFile, "rb+");
			errno = 0;
			int iwrite = fwrite(&pMemIFH[curr].indexFileHead, sizeof(IndexFileHead), 1, indexStream);
			if(iwrite != 1)
			{
				char szMsg[FILTER_ERRMSG_LEN];
				sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			}
			fclose(indexStream);
		}
		pMemIFH[curr].type = IDLE;
	}
	
	//***************************************************************
	//**********************处理数据块*************************
	char tmpDataFile[FILTER_FILESIZE];
	sprintf(tmpDataFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_DATA_NAME);
	FILE *tmpDataFileStream = openfile(tmpDataFile, "rb+");
	IndexData tempIndexdata;
	//遍历临时数据文件
	for(;;)
	{
		int rget = fread(&tempIndexdata, sizeof(IndexData), 1, tmpDataFileStream);
		if(rget != 1)
		{
			break;
		}
		//********************正式索引文件*********************
		//文件名中有时间的信息
		GetIndexFile(tempIndexdata.dataHead.szFileName, tempIndexdata.dataHead.szFileName, szIndexFile);
		theJSLog<< "写正式索引文件3，:" << szIndexFile << endi;
		//************************************************************
		FILE * indexStream = openfile(szIndexFile, "rb+");
		fseek(indexStream, sizeof(IndexFileHead) + (tempIndexdata.dataHead.blockNo)*sizeof(IndexData), SEEK_SET);
		errno = 0;
		int iwrite = fwrite(&tempIndexdata, sizeof(IndexData), 1, indexStream);
		//if ( !(indexStream.good()) )
		if(iwrite != 1)
		{
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
		fclose(indexStream);
	}
	fclose(tmpDataFileStream);

	curr = pMemSH[m_sourceindex].beginIndex;
	CMemBlock block;
	for(; curr!=(SIZE_TYPE)-1; curr=pMemIFH[curr].backward)
	{
		SIZE_TYPE datacurr = pMemIFH[curr].memDataListHead;
		for(; datacurr!=(SIZE_TYPE)-1; datacurr=pMemID[datacurr].backward)
		{
			block.AttachReSource(pMemID, datacurr);
			if(block.GetFlag() == INMEM)
			{
				//********************正式索引文件*********************
				GetIndexFile(block.GetBlockFileName(), block.GetBlockFileName(), szIndexFile);
				theJSLog<< "写正式索引文件4，:" << szIndexFile << endi;
				//************************************************************
				FILE *indexStream = openfile(szIndexFile, "rb+");
				fseek(indexStream, sizeof(IndexFileHead) + block.GetBlockNo()*sizeof(IndexData), SEEK_SET);
				errno = 0;
				int iwrite = fwrite(&(pMemID[datacurr].indexData), sizeof(IndexData), 1, indexStream);
				if(iwrite != 1)
				{
					char szMsg[FILTER_ERRMSG_LEN];
					sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
					throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
				}
				fclose(indexStream);
			}
			block.SetFlag(IDLE);
		}
	}
	//***************************************************************
	ResetList();

	//******************标志位文件*****************************
	CommitFlagStream = openfile(commitFile, "rb+");
	flag = '0';
	errno = 0;
	iwrite = fwrite(&flag, sizeof(char), 1, CommitFlagStream);
	if(iwrite != 1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "写状态文件=%s=出错", commitFile);
		throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
	}
	fclose(CommitFlagStream);
	//CommitFlagStream.close();
	//***************************************************************
	//清理临时数据
	char truncaterFile[FILTER_FILESIZE];
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_DATA_NAME);
	truncFile(truncaterFile);

	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_DATAHEAD_NAME);
	truncFile(truncaterFile);
	
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_FILEINFO_NAME);
	truncFile(truncaterFile);
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_FILEINFO_NAME);
	truncFile(truncaterFile);
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_DATA_NAME);
	truncFile(truncaterFile);

}

//组合正式索引文件名全路径
//输入: 	szTime
//			szFileName
//输出:	resultFile
//返回:	无
//异常:	无
inline void  C_CMemFilter::GetIndexFile(const char* szTime, const char* szFileName, char* resultFile)
{
	if(resultFile == NULL)
	{
		//throw here
	}
	char szYMonth[7];
	char szDay[3];
	memset(szYMonth, 0, sizeof(szYMonth));
	strncpy(szYMonth, szTime, 6);
	memset(szDay, 0, sizeof(szDay));
	strncpy(szDay, szTime+6, 2);
	sprintf(resultFile, "%s%s/%s/%s/%s", m_szIndexPath, m_szSourceid, szYMonth, szDay, szFileName);
}

//初始化共享内存链表结构
//输入: 	无
//输出:	无
//返回:	无
//异常:	无
void C_CMemFilter::InitMem()
{
	theJSLog<< "init mem..." <<endi;
	long j=0;
	for(j=0; j<m_maxsource; j++)
	{
		pMemSH[j].type=IDLE;
		sem_init(&pMemSH[j].semid, 1, 1);
	}
	//文件列表
	for(j=0; j<FILTER_MAXLOADFILE*m_maxsource-1; j++)
	{
		pMemIFH[j].type = IDLE;
		pMemIFH[j].forward = j-1;
		pMemIFH[j].backward = j+1;
	}
	//最后一个
	pMemIFH[j].type = IDLE;
	pMemIFH[j].forward = j-1;
	pMemIFH[j].backward = -1;

	//数据块列表
	for(j=0; j<FILTER_MAXLOADBLOCK*FILTER_MAXLOADFILE*m_maxsource-1; j++)
	{
		pMemID[j].type= IDLE;
		pMemID[j].forward = j-1;
		pMemID[j].backward = j+1;
	}
	//最后一个
	pMemID[j].type = IDLE;
	pMemID[j].forward = j-1;
	pMemID[j].backward = -1;

	//分割文件列表
	int i=0;
	for(i=0; i<m_maxsource; i++)
	{
		pMemSH[i].beginIndex = i*FILTER_MAXLOADFILE;
		pMemSH[i].endIndex = (i+1)*FILTER_MAXLOADFILE-1;
		pMemIFH[i*FILTER_MAXLOADFILE].forward=-1;
		pMemIFH[(i+1)*FILTER_MAXLOADFILE-1].backward=-1;
	}

	for(i=0; i<m_maxsource; i++)
	{
		//数据源空闲链表
		pMemSH[i].idleBlockHead = (i*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK)
			+FILTER_MAXLOADFILE*FILTER_BLOCKNUM_PER_FILE;
		
		pMemSH[i].idleBlockEnd = (i+1)*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK -1 ;
		pMemID[pMemSH[i].idleBlockHead].forward = -1;
		pMemID[pMemSH[i].idleBlockEnd].backward = -1;

		//文件链表
		int k = 0;
		for(; k < FILTER_MAXLOADFILE ; k++)
		{
			int fileindex = i*FILTER_MAXLOADFILE+k;
			pMemIFH[fileindex].memDataListHead = i*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK+k*FILTER_BLOCKNUM_PER_FILE;
			pMemIFH[fileindex].memDataListEnd= i*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK+(k+1)*FILTER_BLOCKNUM_PER_FILE-1;
			pMemIFH[fileindex].memDataCount = FILTER_BLOCKNUM_PER_FILE;
			pMemID[pMemIFH[fileindex].memDataListHead].forward = -1;
			pMemID[pMemIFH[fileindex].memDataListEnd].backward= -1;			
		}
	}
	
}
//获取数据源配置
void C_CMemFilter::InitConfig()
{
	//获取创建共享内存的文件全路径
}

//处理上次错误
//此函数须在主程序明确上次处理有误时才能调用
//输入: 	无
//输出:	无
//返回:	无
//异常:	CException 连接共享内存出错或文件读写出错
void C_CMemFilter::HandleLastError()
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		//TODO:throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存数据源块");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return;
	}
	//*****************重新梳理列表******************************
	ResetList();
	//*****************列表梳理完毕******************************
	
	//******************标志位文件*****************************
	char commitFile[FILTER_FILESIZE];
	sprintf(commitFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, COMMIT_FILE);

	FILE *CommitFlagStream = openfile(commitFile, "rb+");
	char flag = '0';
	fread(&flag, sizeof(char), 1, CommitFlagStream);
	fclose(CommitFlagStream);
	if('1' == flag)
	{
		//在回写正式文件的过程中出错

		//***********从临时文件中回写文件头********************
		char tmpFile[FILTER_FILESIZE];
		sprintf(tmpFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
			FILTER_WORK_PATH, m_szProcessId, TMP_FILEINFO_NAME);
		//fstream tmpFileStream;
		//openFile(tmpFileStream, tmpFile);
		FILE * tmpFileStream = openfile(tmpFile, "rb+");
		char szIndexFile[FILTER_FILESIZE];
		IndexFileHead tmpfilehead;
		//遍历临时文件文件头信息
		int rget = 0;
		for (; ;)
		{
			rget = fread(&tmpfilehead, sizeof(IndexFileHead), 1, tmpFileStream);
			//tmpFileStream.read((char *)&tmpfilehead, sizeof(IndexFileHead));
			//if (tmpFileStream.gcount() < sizeof(IndexFileHead))
			if(rget != 1)
			{
				break;
			}
			GetIndexFile(tmpfilehead.szTime, tmpfilehead.szFileName, szIndexFile);
			//fstream indexStream;
			//openFile(indexStream, szIndexFile);
			//indexStream.write((char*)&tmpfilehead, sizeof(IndexFileHead));
			FILE *indexStream = openfile(szIndexFile, "rb+");
			errno = 0;
			int iwrite = fwrite(&tmpfilehead, sizeof(IndexFileHead), 1, indexStream);
			//if ( !(indexStream.good()) )
			if(iwrite != 1)
			{
				char szMsg[FILTER_ERRMSG_LEN];
				sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			}
			fclose(indexStream);
			//indexStream.close();
		}
		fclose(tmpFileStream);
		//tmpFileStream.close();
		//***********从备份文件中回写文件头************************
		sprintf(tmpFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
			FILTER_WORK_PATH, m_szProcessId, BACKUP_FILEINFO_NAME);
		//openFile(tmpFileStream, tmpFile);
		tmpFileStream = openfile(tmpFile, "rb+");
		//遍历临时文件文件头信息
		for (; ;)
		{
			//tmpFileStream.read((char *)&tmpfilehead, sizeof(IndexFileHead));
			//if (tmpFileStream.gcount() < sizeof(IndexFileHead))
			rget = fread(&tmpfilehead, sizeof(IndexFileHead), 1, tmpFileStream);
			if(rget != 1)
			{
				break;
			}
			GetIndexFile(tmpfilehead.szTime, tmpfilehead.szFileName, szIndexFile);
			FILE *indexStream = openfile(szIndexFile, "rb+");
			errno = 0;
			int iwrite = fwrite(&tmpfilehead, sizeof(IndexFileHead), 1, indexStream);
			if(iwrite != 1)
			{
				char szMsg[FILTER_ERRMSG_LEN];
				sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			}
			fclose(indexStream);
		}
		fclose(tmpFileStream);
		
		//*************从临时文件中回写数据块*****************
		char tmpDataFile[FILTER_FILESIZE];
		sprintf(tmpDataFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
			FILTER_WORK_PATH, m_szProcessId, TMP_DATA_NAME);
		//fstream tmpDataFileStream;
		//openFile(tmpDataFileStream, tmpDataFile);
		FILE *tmpDataFileStream = openfile(tmpDataFile, "rb+");
		IndexData tempIndexdata;
		//遍历临时数据文件
		for(;;)
		{
			rget = fread(&tempIndexdata, sizeof(IndexData), 1, tmpDataFileStream);
			if(rget != 1)
			{
				break;
			}
			//文件名中有时间的信息
			GetIndexFile(tempIndexdata.dataHead.szFileName, tempIndexdata.dataHead.szFileName, szIndexFile);
			FILE *indexStream = openfile(szIndexFile, "rb+");
			fseek(indexStream, sizeof(IndexFileHead) + (tempIndexdata.dataHead.blockNo)*sizeof(IndexData), SEEK_SET);
			errno = 0;
			int iwrite = fwrite(&tempIndexdata, sizeof(IndexData), 1, indexStream);
			if(iwrite != 1)
			{
			 	char szMsg[FILTER_ERRMSG_LEN];
				sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			 }
			fclose(indexStream);
			//indexStream.close();
		}
		fclose(tmpDataFileStream);
		//tmpDataFileStream.close();
		
		//*************从备份文件中回写数据块***********
		sprintf(tmpDataFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
			FILTER_WORK_PATH, m_szProcessId, BACKUP_DATA_NAME);
		//openFile(tmpDataFileStream, tmpDataFile);
		tmpDataFileStream = openfile(tmpDataFile, "rb+");
		for(;;)
		{
			rget = fread(&tempIndexdata, sizeof(IndexData), 1, tmpDataFileStream);
			if(rget != 1)
			{
				break;
			}
			//文件名中有时间的信息
			GetIndexFile(tempIndexdata.dataHead.szFileName, tempIndexdata.dataHead.szFileName, szIndexFile);
			//fstream indexStream;
			//openFile(indexStream, szIndexFile);
			//indexStream.seekg(sizeof(IndexFileHead) + (tempIndexdata.dataHead.blockNo)*sizeof(IndexData), ios::beg);
			//indexStream.write((char*) &tempIndexdata, sizeof(IndexData));
			//if ( !(indexStream.good()) )
			FILE *indexStream = openfile(szIndexFile, "rb+");
			fseek(indexStream, sizeof(IndexFileHead) + (tempIndexdata.dataHead.blockNo)*sizeof(IndexData), SEEK_SET);
			errno = 0;
			int iwrite = fwrite(&tempIndexdata, sizeof(IndexData), 1, indexStream);
			if(iwrite != 1)
			{
			 	char szMsg[FILTER_ERRMSG_LEN];
				sprintf(szMsg, "写索引文件=%s=出错", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			 }
			fclose(indexStream);
			//indexStream.close();
		}
		fclose(tmpDataFileStream);
		//tmpDataFileStream.close();
	}

	//清理临时数据
	char truncaterFile[FILTER_FILESIZE];
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_DATA_NAME);
	truncFile(truncaterFile);
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_DATAHEAD_NAME);
	truncFile(truncaterFile);
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_FILEINFO_NAME);
	truncFile(truncaterFile);
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_FILEINFO_NAME);
	truncFile(truncaterFile);
	sprintf(truncaterFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_DATA_NAME);
	truncFile(truncaterFile);

	//openFile(CommitFlagStream, commitFile);
	//CommitFlagStream.write((char *)"0",sizeof(char));
	//if ( !(CommitFlagStream.good()))
	CommitFlagStream = openfile(commitFile, "rb+");
	errno = 0;
	flag = '0';
	int iwrite = fwrite(&flag, sizeof(char), 1, CommitFlagStream);
	if(iwrite != 1)
	{
		//TODO throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "写状态记录文件=%s=出错", commitFile);
		throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
	}
	fclose(CommitFlagStream);
	//CommitFlagStream.close();
	//***************************************************************
}


void C_CMemFilter::ClearSemLock(const char* comfile, const char* sourceid)
{
	//key_t ikey = ftok(comfile, FILTER_SOURCE_INDEX); 
	key_t ikey = filterSourceKey;
	//连接数据源
	int ishmid = shmget(ikey, 0, 0666);
	if(ishmid < 0)
	{
		theJSLog << "connect error !" <<ende;
		return;
	}
	errno = 0;
	//vivi edit
	if(pMemSH==NULL)
		pMemSH = (MemSourceHead*)shmat(ishmid, NULL, 0);
	else
		pMemSH = (MemSourceHead*)shmat(ishmid, pMemSH, 0);
		
	//连接文件头
	//ikey = ftok(m_szCommemFile, FILTER_FILE_INDEX); 
	ikey = filterFileKey;
	ishmid = shmget(ikey, 0, 0666);
	if(pMemIFH==NULL)
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, NULL, 0);
	else
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, pMemIFH, 0);
		
	//连接数据块
	//ikey = ftok(m_szCommemFile, FILTER_BLOCK_INDEX); 
	ikey = filterBlockKey;
	ishmid = shmget(ikey, 0, 0666);
	if(pMemID==NULL)
		pMemID = (MemIndexData*)shmat(ishmid, NULL, 0);
  else
  	pMemID = (MemIndexData*)shmat(ishmid, pMemID, 0);
  	
	int i=0;
	for(i=0; i<m_maxsource; i++)
	{
		if(strcmp(pMemSH[i].szSourceId, sourceid) == 0)
		{
			break;
		}
	}
	int tempindex = i;
	if(tempindex >= m_maxsource)
	{
		theJSLog << "没有此数据源 " <<sourceid<<endi;
		return ;
	}

	theJSLog << "解锁 " <<sourceid<<endi;
	sem_post(&(pMemSH[tempindex].semid));

}

void C_CMemFilter::SemLock(const char* comfile, const char* sourceid)
{
	//key_t ikey = ftok(comfile, FILTER_SOURCE_INDEX); 
	key_t ikey = filterSourceKey;
	//连接数据源
	int ishmid = shmget(ikey, 0, 0666);
	if(ishmid < 0)
	{
		theJSLog << "connect error !" <<endi;
		return;
	}
	errno = 0;
	if(pMemSH==NULL)
		pMemSH = (MemSourceHead*)shmat(ishmid, NULL, 0);
	else
		pMemSH = (MemSourceHead*)shmat(ishmid, pMemSH, 0);
		
	//连接文件头
	//ikey = ftok(m_szCommemFile, FILTER_FILE_INDEX); 
	ikey = filterFileKey;
	ishmid = shmget(ikey, 0, 0666);
	if(pMemIFH==NULL)
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, NULL, 0);
	else
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, pMemIFH, 0);
		
	//连接数据块
	//ikey = ftok(m_szCommemFile, FILTER_BLOCK_INDEX); 
	ikey = filterBlockKey;
	ishmid = shmget(ikey, 0, 0666);
	if(pMemID==NULL)
		pMemID = (MemIndexData*)shmat(ishmid, NULL, 0);
	else
		pMemID = (MemIndexData*)shmat(ishmid, pMemID, 0);

	int i=0;
	for(i=0; i<m_maxsource; i++)
	{
		if(strcmp(pMemSH[i].szSourceId, sourceid) == 0)
		{
			break;
		}
	}
	int tempindex = i;
	if(tempindex >= m_maxsource)
	{
		return ;
	}
	int ret = sem_trywait(&(pMemSH[tempindex].semid));	
}


//是否回写文件写到一半
bool C_CMemFilter::IsWriteFileError()
{
	//******************标志位文件*****************************
	char commitFile[FILTER_FILESIZE];
	sprintf(commitFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, COMMIT_FILE);
	FILE *CommitFlagStream = openfile(commitFile, "rb+");
	//fstream CommitFlagStream;
	//openFile(CommitFlagStream, commitFile);
	//char szFlag[3];
	//memset(szFlag, 0, sizeof(szFlag));
	char flag = '0';
	fread(&flag, sizeof(char), 1, CommitFlagStream);
	fclose(CommitFlagStream);
	//CommitFlagStream.read(szFlag, sizeof(char));
	//CommitFlagStream.close();
	//if('1' == szFlag[0])
	if('1' == flag)
	{
		return true;
	}
	return false;
}

//是否共享内存有异常
int C_CMemFilter::IsMemError()
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存数据源块");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return 1;
	}

	SIZE_TYPE f_curr = pMemSH[m_sourceindex].beginIndex;  //获得内存文件地址
	SIZE_TYPE f_cure_old=-1;
	int check_mem_err = 0;
	//theJSLog << "检查数据源的共享内存合法性 " << endi;
	for(; f_curr!=pMemSH[m_sourceindex].endIndex; f_curr=pMemIFH[f_curr].backward)
	{
		if(f_curr <(SIZE_TYPE)0){
			// 下标出错
			theJSLog << "内存异常，f_curr:" << f_curr << endi;
			return 123456;
		}

		if(f_cure_old != -1 && f_cure_old != pMemIFH[f_curr].forward ){
			// 当前节点的前导节点不等于上次访问的节点，出错
			return 123457;
		}
		f_cure_old = f_curr;

//		if(pMemIFH[f_curr].forward < (SIZE_TYPE)0 && f_curr != pMemSH[m_sourceindex].endIndex)
//			// 当前节点还不是尾节点，但forward<0，出错
//			return 123458;


		SIZE_TYPE d_curr = pMemIFH[f_curr].memDataListHead;	//获得数据块地址
		SIZE_TYPE d_curr_old = -1;
	  //theJSLog << "f_curr == " << f_curr << ",d_curr ==" << d_curr << endi;
		for(; d_curr!=(SIZE_TYPE)-1; d_curr=pMemID[d_curr].backward)
		{

			if(d_curr < (SIZE_TYPE)0){
				// 下标出错
				return 1000006;
			}

			if(d_curr_old != -1 && d_curr_old != pMemID[d_curr].forward ){
				// 当前节点的前导节点不等于上次访问的节点，出错
				return 1000007;
			}
			d_curr_old = d_curr;

//			if(pMemID[d_curr].forward < (SIZE_TYPE)0 && f_curr != pMemIFH[f_curr].memDataListEnd)
//						// 当前节点还不是尾节点，但forward<0，出错
//						return 1000008;

			if (pMemID[d_curr].type!=IDLE && pMemID[d_curr].indexData.dataHead.indexInBlock == 0)
			{	
				theJSLog << "内存异常，fileno:" << f_curr << ",blockno:" << d_curr << ",block flag:" << pMemID[d_curr].type << ",count:" << pMemID[d_curr].indexData.dataHead.indexInBlock << endi;
				check_mem_err ++;
			}
		}
	}
	return check_mem_err;;
}

bool C_CMemFilter::ReMoveIndex(const char* szTime, const char* key)
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存数据源块");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	SIZE_TYPE second = GetSecond(szTime, FILTER_INDEX_FILE_LEN);
	SIZE_TYPE fileindex = GetFileIndex(szTime);
	m_fileindex = fileindex;
	CMemBlockListHead blocklisthead;
	blocklisthead.AttachReSource(pMemIFH, fileindex);
	SIZE_TYPE intendblockno = -1;
	blocklisthead.GetBlockIndexNo(szTime, second, intendblockno);

	CMemBlock block ;
	SIZE_TYPE indexposition;
	while(true)
	{
		indexposition = GetBlock(fileindex, intendblockno, szTime);
		block.AttachReSource(pMemID, indexposition);
		if(block.GetNextBlock() == (SIZE_TYPE)-1)
		{
			break;
		}
		IndexDataValue value;
		block.GetMaxValue(value);
		if(second<value.second || (second==value.second && memcmp(key, value.szIndexValue, FILTER_VALUESIZE+1) <= 0))
		{
			break;
		}
		intendblockno = block.GetNextBlock();
	}

	SIZE_TYPE location;
	block.AttachReSource(pMemID, indexposition);
	
	return block.DeleteIndex(second, key);
}


bool C_CMemFilter::GetSourceLock(SIZE_TYPE source_idx, const char * filename)
{
	int ret ;
	if((ret=sem_trywait(&(pMemSH[source_idx].semid))) != 0)
	{
		//加锁另外的文件锁
		filelock.Lock();
		if(-1 == kill(pMemSH[source_idx].iprocessId, 0))
		{
			theJSLog << "加锁的进程pid(" << pMemSH[source_idx].iprocessId << ")不存在" <<endi;
			if(IsWriteFileError())
			{
				theJSLog <<"文件回写到一半" << endi;
				//已经写到一半了,只能阻塞到该文件
				if(strcmp(pMemSH[source_idx].szDealFileName, filename) == 0)
				{
					theJSLog << "处理上次错误"<<endi;
					HandleLastError();
					//登记独占此数据源的进程id
					pMemSH[source_idx].iprocessId = getpid();
					//解锁另外的文件锁
					filelock.UnLock();
					return true;
				}
				else
				{
					theJSLog << pMemSH[source_idx].szDealFileName
					<<"文件提交到一半,只能等待该文件重新处理完成后才停止阻塞!!!!"<<endw;
				}
			}
			else
			{
				theJSLog <<"文件还没有回写" << endi;
				theJSLog << "处理上次错误"<<endi;
				HandleLastError();
				//登记独占此数据源的进程id
				pMemSH[source_idx].iprocessId = getpid();
				//解锁另外的文件锁
				filelock.UnLock();
				return true;
			}
		}
		else
		{
			theJSLog << "加锁的进程pid(" << pMemSH[source_idx].iprocessId << ")存在" <<endi;
			//进程存在则一直阻塞
		}
		//解锁另外的文件锁
		filelock.UnLock();
		theJSLog << "等待其他进程...."<<endi;
		//sleep(10);
	}
	else
	{
		return true;
	}

	return false;
}

//获取空闲块
SIZE_TYPE C_CMemFilter::GetIdlBlock()
{
	//队列方式
	SIZE_TYPE oldHead;
	if(pMemSH[m_sourceindex].idleBlockHead == (SIZE_TYPE)-1)
	{
		//空闲队列无空闲块
		//从最近未使用的文件列表开始
		//查找文件列表中块数大于FILTER_BLOCKNUM_PER_FILE的文件链表
		//取出五个空块出来,放到空闲块中
		int getCount=0;	//标识已经取出多少个空块出来
		
		SIZE_TYPE fileListId = pMemSH[m_sourceindex].endIndex;
		SIZE_TYPE dataListId;
		SIZE_TYPE dataListQ;
		CMemBlock memblock;
		
		while(fileListId != (SIZE_TYPE)-1)
		{
			if(pMemIFH[fileListId].memDataCount <= FILTER_BLOCKNUM_PER_FILE)
			{
				fileListId = pMemIFH[fileListId].forward;
				continue;
			}
			dataListId = pMemIFH[fileListId].memDataListEnd;	//获取要取出来的块
			dataListQ = pMemID[dataListId].forward;	//获取目标块的前面一块
			pMemID[dataListId].forward = -1;		//将要取出来的块孤立
			pMemID[dataListQ].backward = -1;				//设置前面一个块dataListQ为尾块
			pMemIFH[fileListId].memDataListEnd = dataListQ;	//设置前面一个块dataListQ为尾块
			pMemIFH[fileListId].memDataCount--;			//块数计数减一
			
			//将取出来数据块原始信息写入临时文件保存
			memblock.AttachReSource(pMemID, dataListId);
			memblock.Write2Temp(m_szIndexPath, m_szProcessId);
			memblock.SetFlag(IDLE);

			//将取出来的块放入空闲队列中
			if(pMemSH[m_sourceindex].idleBlockHead == (SIZE_TYPE)-1)
			{
				pMemSH[m_sourceindex].idleBlockHead = dataListId;
				pMemSH[m_sourceindex].idleBlockEnd = dataListId;
			}
			else
			{
				pMemID[pMemSH[m_sourceindex].idleBlockEnd].backward = dataListId;
				pMemID[dataListId].forward = pMemSH[m_sourceindex].idleBlockEnd;
				pMemSH[m_sourceindex].idleBlockEnd = dataListId;
			}
			//空闲块自增1
			getCount++;
			if(getCount >= 2)
			{
				break;
			}	
		}	
	}
	
	//空闲队列中有空闲块
	oldHead = pMemSH[m_sourceindex].idleBlockHead;
	pMemSH[m_sourceindex].idleBlockHead = pMemID[oldHead].backward;
	if(pMemSH[m_sourceindex].idleBlockHead != (SIZE_TYPE)-1)
	{
		pMemID[pMemSH[m_sourceindex].idleBlockHead].forward = -1;
	}
	else
	{
		pMemSH[m_sourceindex].idleBlockEnd = -1;
	}
	pMemID[oldHead].backward = -1;
	pMemID[oldHead].forward = -1;
	return oldHead;

}


void C_CMemFilter::ResetList()
{
	//*****************重新梳理列表******************************
	pMemSH[m_sourceindex].beginIndex = m_sourceindex*FILTER_MAXLOADFILE;
	pMemSH[m_sourceindex].endIndex = (m_sourceindex+1)*FILTER_MAXLOADFILE-1;
	SIZE_TYPE i=m_sourceindex*FILTER_MAXLOADFILE;
	for(; i<(m_sourceindex+1)*FILTER_MAXLOADFILE; i++)
	{
		pMemIFH[i].type = IDLE;
		pMemIFH[i].forward = i-1;
		pMemIFH[i].backward = i+1;
	}
	//第一个文件列表
	pMemIFH[m_sourceindex*FILTER_MAXLOADFILE].forward=-1;
	//最后一个文件列表
	pMemIFH[(m_sourceindex+1)*FILTER_MAXLOADFILE-1].backward=-1;
	SIZE_TYPE j = m_sourceindex*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK;
	//第一个数据块
	pMemID[j].forward = -1;
	for(; j<(m_sourceindex+1)*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK; j++)
	{
		pMemID[j].type = IDLE;
		pMemID[j].forward = j-1;
		pMemID[j].backward = j+1;
	}
	//最后一个数据块
	pMemID[j-1].backward = -1;
	
	pMemSH[m_sourceindex].idleBlockHead = (m_sourceindex*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK)
		+FILTER_MAXLOADFILE*FILTER_BLOCKNUM_PER_FILE;
	
	pMemSH[m_sourceindex].idleBlockEnd = (m_sourceindex+1)*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK-1;
	pMemID[pMemSH[m_sourceindex].idleBlockHead].forward = -1;
	pMemID[pMemSH[m_sourceindex].idleBlockEnd].backward = -1;

	//处理文件链表与数据块链表的关系
	int k = 0;
	for(; k < FILTER_MAXLOADFILE ; k++)
	{
		int fileindex = m_sourceindex*FILTER_MAXLOADFILE+k;
		pMemIFH[fileindex].memDataListHead = m_sourceindex*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK
			+k*FILTER_BLOCKNUM_PER_FILE;
		pMemIFH[fileindex].memDataListEnd= m_sourceindex*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK
			+(k+1)*FILTER_BLOCKNUM_PER_FILE-1;

		pMemIFH[fileindex].memDataCount = FILTER_BLOCKNUM_PER_FILE;
		
		pMemID[pMemIFH[fileindex].memDataListHead].forward = -1;
		pMemID[pMemIFH[fileindex].memDataListEnd].backward= -1;			
	}

	//*****************列表梳理完毕******************************

}

void C_CMemFilter::CheckFileList(int i)
{
	map<SIZE_TYPE, int> myMap;
	myMap.clear();
	
	SIZE_TYPE k = pMemSH[m_sourceindex].beginIndex;
	for(; k!=-1; k=pMemIFH[k].backward)
	{
		map<SIZE_TYPE, int>::iterator ite =  myMap.find(k) ;
		if(ite != myMap.end())
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "%d 重复的文件列表=%d", i, k);
			throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		}
		myMap.insert(map<SIZE_TYPE, int>::value_type(k, 1));
		
		if(pMemIFH[k].memDataCount == 0)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "%d check memDataCount = 0", i);
			throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		}
	}
}



