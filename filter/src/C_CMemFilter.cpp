
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

//��ӡ ������
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

//��ӡ ������
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

//��ӡ�����ڴ����ӹ�ϵ
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

//ɾ�������ڴ�
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
	//��ʼ����������;��ʽ�༭����
	//��ȡ���������ڴ���ļ�·���������洢Ŀ¼
	strcpy(m_szIndexPath, szIndexPath);
	strcpy(m_szCommemFile, szComFile);
	cout<<"m_szIndexPath:"<<m_szIndexPath<<endl;
	cout<<"m_szCommemFile:"<<m_szCommemFile<<endl;
	m_maxsource = (maxSource > 0)?maxSource:FILTER_MAXSOURCE ;
	//strcpy(m_szProcessId, "filter");
	filelock.Init(m_szCommemFile);
	InitConfig();
	//��ȫ����
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
		// ����
		//��������Դ
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
		//�����ļ�ͷ
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

		//�������ݿ�
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
		// ������ȫ������������ʼֵ
		theJSLog << "creating source.. " <<endi;
		long count = m_maxsource;
		pMemSH = (MemSourceHead*)shmat(ishmid, NULL, 0);

		//�����ļ�ͷ
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

		//�������ݿ�
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

// ����˵��: ����һ���ļ���¼��ȥ�ش���
// ����:
// ����ֵint:
// 		�������ģʽʱ	:0��ʾ��ӳɹ�; 1��ʾ������ͬ����Ч��¼
//		ȥ������ģʽʱ	:0��ʾɾ���ɹ�; 1��ʾԭ��û����ͬ����Ч��¼
int C_CMemFilter::ProcessRecord(/*PacketParser& pps, ResParser& retValue*/)
{
	// ��ȡʱ���keyֵ
	// 1����ʱ���
	// 2��ȡ���ݿ�
	// 3���ڲ��ҹؼ���
	
	// szDealType ����ģʽ 2��ʾȥ������;1��ʾ�������;
/*
	//��һ���Ǵ���ʽ
	char szDealType[ATTRINUMBER];
	memset(szDealType, 0, sizeof(szDealType));
	pps.getFieldValue(1, szDealType);
	if(strcmp(szDealType, "1")!=0 && strcmp(szDealType, "2")!=0)
	{
		//throw here
	}
	//�ڶ�����ʱ��
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

	//�������
	if(strcmp(szDealType, "1")==0)
	{
		
	}
	//ɾ������
	else if(strcmp(szDealType, "2")==0)
	{
		
	} 
*/
	return 0;
}

//����ʱ���ȡ����
SIZE_TYPE C_CMemFilter::GetSecond(const char* szTime, int len)
{
	if(szTime==NULL || strlen(szTime)!=14 || len > 14 || len < 10)
	{
		//TODO:throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "time=%s= ���ǺϷ�ʱ��", szTime);
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

//��ȡ�ڴ���ָ�������ݿ�
//����: 	fileindex �ڴ����ļ���Ϣ��indexֵ
//			intendBlockNo �ļ��еĵڼ������ݿ�
//			szTime ȥ��ʱ��
//����:	SIZE_TYPE ��Ѱ�ҿ����ڴ��е�indexֵ
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
	//�����溬����ͬ
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
		//��ȡ�¿�
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

//���ýӿ� ��������
//����: 	szTime ȥ��ʱ��
//			key	ȥ�عؼ����ִ�
//����:	bool �Ƿ������ɹ�
bool C_CMemFilter::AddIndex(const char*szTime, const char* key)
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�����Դ��");
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
		//���� �ƶ�
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

//�����ݿ�[fromindex]�еĲ��������ƶ������ݿ�[toindex]
//���ݿ�[index]ԭ�н������

//����: 	fileindex �ļ���Ϣ���ڴ��е�indexֵ
//			fromindex Դ���ݿ����ڴ��е�indexֵ
//			toindex Ŀ�����ݿ����ڴ��е�indexֵ
//����:	��
void C_CMemFilter::MoveIndex(SIZE_TYPE fileindex, SIZE_TYPE fromindex, SIZE_TYPE toindex)
{	
	CMemBlock fromblock, toblock;
	fromblock.AttachReSource(pMemID, fromindex);
	toblock.AttachReSource(pMemID, toindex);

	//CheckFileList(20);
	toblock.SetIndexCount(fromblock.GetIndexCount()/5);
	SIZE_TYPE fromEnd = fromblock.GetIndexCount()-1;
	SIZE_TYPE toEnd = toblock.GetIndexCount()-1;

	//�������֮һ��ȥ�¿�
	IndexDataValue value;
	for(; toEnd >=0; toEnd--, fromEnd--)
	{
		fromblock.GetKey(fromEnd, value);
		toblock.SetKey(toEnd, value);
	}
	//CheckFileList(21);
	//���þͿ�������Ŀ
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

//����ʱ�� ��ȡ��Ӧ���ڴ��ļ������
//����: 	szTime ȥ��ʱ��
//����:	SIZE_TYPE �ļ���Ϣ���ڴ��е�indexֵ

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
			//�ҵ���
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
	else		//�����溬����ͬ
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

//�Ǽ�����Դ
//����ʱ�� ��ȡ��Ӧ���ڴ��ļ������
//����: 	sourceid ����Դ
//����:	��
//�쳣:	����Դ�����������ֵ
void C_CMemFilter::DealBatch(const char* sourceid)
{
	//��ȡ����Դ��Ϣ���ϴδ���״̬
	//û�о��������Դ
	//�ϴδ���ʧ�� ��������
	
	//��������Դid��д����id
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
		sprintf(szMsg, "����Դ����������Χ");
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	
}

//��ʼ�����ļ�;ȡ������Դ��
//����: 	sourceid ����Դ
//			filename ���ڴ����ļ���
//����:	��
//�쳣:	�޷������Ϲ����ڴ�
void C_CMemFilter::DealFile(const char* sourceid, const char* filename)
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�����Դ��");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return;
	}

	char szTempDir[FILTER_FILESIZE];
	sprintf(szTempDir, "%s%s/%s/", m_szIndexPath, m_szSourceid, FILTER_WORK_PATH);
	chkAllDir(szTempDir);
	
	//������Դ����
	int ret ;
	theJSLog << "�������Դ��.... " <<endi;
	if(GetSourceLock(m_sourceindex, filename) == false)
	{
	  theJSLog << "������Դ����.... " <<endi;
		while((ret=sem_wait(&(pMemSH[m_sourceindex].semid))) != 0)
		{
			//����������ļ���
			theJSLog << "���ļ�����.... " <<endi;
			filelock.Lock();
			if(-1 == kill(pMemSH[m_sourceindex].iprocessId, 0))
			{
				theJSLog << "�����Ľ���pid(" << pMemSH[m_sourceindex].iprocessId << ")������" <<endi;
				if(IsWriteFileError())
				{
					theJSLog <<"�ļ���д��һ��" << endi;
					//�Ѿ�д��һ����,ֻ�����������ļ�
					if(strcmp(pMemSH[m_sourceindex].szDealFileName, filename) == 0)
					{
						theJSLog << "�����ϴδ���"<<endi;
						HandleLastError();
						//�ǼǶ�ռ������Դ�Ľ���id
						pMemSH[m_sourceindex].iprocessId = getpid();
						//����������ļ���
						filelock.UnLock();
						break;
					}
					else
					{
						theJSLog << pMemSH[m_sourceindex].szDealFileName
						<<"�ļ��ύ��һ��,ֻ�ܵȴ����ļ����´�����ɺ��ֹͣ����!!!!"<<endw;
					}
				}
				else
				{
					theJSLog <<"�ļ���û�л�д" << endi;
					theJSLog << "�����ϴδ���"<<endi;
					HandleLastError();
					//�ǼǶ�ռ������Դ�Ľ���id
					pMemSH[m_sourceindex].iprocessId = getpid();
					//����������ļ���
					filelock.UnLock();
					break;
				}
			}
			else
			{
				theJSLog << "����pid(" << pMemSH[m_sourceindex].iprocessId << ")����" <<endi;
				//���̴�����һֱ����
			}
			//����������ļ���
			filelock.UnLock();
			theJSLog << "�ȴ���������...."<<endi;
			//sleep(10);
		}
	}

  //�������Դ�Ĺ����ڴ�Ϸ���
//	int err___ = IsMemError();
//  if (err___)
//	{
//	  char szMsg[FILTER_ERRMSG_LEN];
//	  		sprintf(szMsg, "�����ڴ��쳣��123 jiangjz:%d", err___);
//	  		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
//	  		return;
//		theJSLog << "�����ڴ��쳣,���������ڴ�..." << endi;
//		HandleLastError();
//		theJSLog << "���������ڴ����" << endi;
//	}
	//�˴��Ѿ��������Դ��
	//�ǼǶ�ռ������Դ�Ľ���id
	pMemSH[m_sourceindex].iprocessId = getpid();
	strcpy(pMemSH[m_sourceindex].szDealFileName, filename);
	strcpy(m_szDealFileName, filename);
	theJSLog << "�ѻ������Դ�����Ǽǽ���ID:" << pMemSH[m_sourceindex].iprocessId << ",��ʼ�����ļ���" << filename << endi;
	
}

//�����ύ
void C_CMemFilter::CommitBatch()
{
	
}

//�ύ�ļ�
//�����ڴ��е�����
//����ʱ�ļ�,�ڴ��е�ȥ������д����ʽ�ļ���
//�����ڴ��е��������;
//�ͷ�����Դ��

//����: 	��
//����:	��
//�쳣:	�޷������Ϲ����ڴ���ļ���дʧ�ܵ�
void C_CMemFilter::CommitFile()
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�����Դ��");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return;
	}
	
	//�������Դ�Ĺ����ڴ�Ϸ���
//	int err___ = IsMemError();
//  if (err___)
//	{
//		char szMsg[FILTER_ERRMSG_LEN];
//		sprintf(szMsg, "�����ڴ��쳣���ļ���Ҫ�ش���:jiangjz:code=%d", err___);
//		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
//		return;
//	}
	
	//д�����ļ�
	//*******************�ļ�ͷ��Ϣ*******************************
	char backupFile[FILTER_FILESIZE];
	sprintf(backupFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_FILEINFO_NAME);
	truncFile(backupFile);
	FILE *backupFileStream =  openfile(backupFile, "rb+");
	//******************************************************************

	//*******************���ݿ���Ϣ*******************************
	char backupDataFile[FILTER_FILESIZE];
	sprintf(backupDataFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, BACKUP_DATA_NAME);
	truncFile(backupDataFile);
	FILE *backupdataStream = openfile(backupDataFile, "rb+");
	CMemBlock block;
	//******************************************************************

	//�������ļ�ͷ
	SIZE_TYPE curr = pMemSH[m_sourceindex].beginIndex;
	CMemBlockListHead listhead;	
	for(; curr!=(SIZE_TYPE)-1; curr=pMemIFH[curr].backward)
	{
		listhead.AttachReSource(pMemIFH, curr);
		listhead.Backup(backupFileStream);
		//���������ݿ�
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


//�ύ
//����ʱ�ļ�,�ڴ��е�ȥ������д����ʽ�ļ���
//�����ڴ��е��������;

//����: 	��
//����:	��
//�쳣:	�޷������Ϲ����ڴ���ļ���дʧ�ܵ�
void C_CMemFilter::Commit()
{
	//******************��־λ�ļ�*****************************
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
		sprintf(szMsg, "д��־�ļ�=%s=����", commitFile);
		throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
	}
	fclose(CommitFlagStream);
	//***************************************************************

	//****************�����ļ�ͷ*************************
	char tmpFile[FILTER_FILESIZE];
	sprintf(tmpFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_FILEINFO_NAME);
	FILE *tmpFileStream = openfile(tmpFile, "rb+");
	char szIndexFile[FILTER_FILESIZE];
	IndexFileHead tmpfilehead;
	//������ʱ�ļ��ļ�ͷ��Ϣ
	for (; ;)
	{
		int rget = fread(&tmpfilehead, sizeof(IndexFileHead), 1, tmpFileStream);
		if(rget != 1)
		{
			break;
		}
		//********************��ʽ�����ļ�*********************
		GetIndexFile(tmpfilehead.szTime, tmpfilehead.szFileName, szIndexFile);
		theJSLog<< "д��ʽ�����ļ�:" << szIndexFile << endi;
		//************************************************************

		FILE * indexStream = openfile(szIndexFile, "rb+");
		errno = 0;
		int iwrite = fwrite(&tmpfilehead, sizeof(IndexFileHead), 1, indexStream);
		if(iwrite != 1)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		}
		fclose(indexStream);
	}
	fclose(tmpFileStream);

	//�����ڴ���ļ�ͷ��Ϣ
	SIZE_TYPE curr = pMemSH[m_sourceindex].beginIndex;
	for(; curr!=(SIZE_TYPE)-1; curr=pMemIFH[curr].backward)
	{
		if(pMemIFH[curr].type == INMEM)
		{
			//********************��ʽ�����ļ�*********************
			GetIndexFile(pMemIFH[curr].indexFileHead.szTime, pMemIFH[curr].indexFileHead.szFileName, szIndexFile);
			theJSLog<< "д��ʽ�����ļ�2��:" << szIndexFile << endi;
			//************************************************************

			FILE *indexStream = openfile(szIndexFile, "rb+");
			errno = 0;
			int iwrite = fwrite(&pMemIFH[curr].indexFileHead, sizeof(IndexFileHead), 1, indexStream);
			if(iwrite != 1)
			{
				char szMsg[FILTER_ERRMSG_LEN];
				sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			}
			fclose(indexStream);
		}
		pMemIFH[curr].type = IDLE;
	}
	
	//***************************************************************
	//**********************�������ݿ�*************************
	char tmpDataFile[FILTER_FILESIZE];
	sprintf(tmpDataFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, TMP_DATA_NAME);
	FILE *tmpDataFileStream = openfile(tmpDataFile, "rb+");
	IndexData tempIndexdata;
	//������ʱ�����ļ�
	for(;;)
	{
		int rget = fread(&tempIndexdata, sizeof(IndexData), 1, tmpDataFileStream);
		if(rget != 1)
		{
			break;
		}
		//********************��ʽ�����ļ�*********************
		//�ļ�������ʱ�����Ϣ
		GetIndexFile(tempIndexdata.dataHead.szFileName, tempIndexdata.dataHead.szFileName, szIndexFile);
		theJSLog<< "д��ʽ�����ļ�3��:" << szIndexFile << endi;
		//************************************************************
		FILE * indexStream = openfile(szIndexFile, "rb+");
		fseek(indexStream, sizeof(IndexFileHead) + (tempIndexdata.dataHead.blockNo)*sizeof(IndexData), SEEK_SET);
		errno = 0;
		int iwrite = fwrite(&tempIndexdata, sizeof(IndexData), 1, indexStream);
		//if ( !(indexStream.good()) )
		if(iwrite != 1)
		{
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
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
				//********************��ʽ�����ļ�*********************
				GetIndexFile(block.GetBlockFileName(), block.GetBlockFileName(), szIndexFile);
				theJSLog<< "д��ʽ�����ļ�4��:" << szIndexFile << endi;
				//************************************************************
				FILE *indexStream = openfile(szIndexFile, "rb+");
				fseek(indexStream, sizeof(IndexFileHead) + block.GetBlockNo()*sizeof(IndexData), SEEK_SET);
				errno = 0;
				int iwrite = fwrite(&(pMemID[datacurr].indexData), sizeof(IndexData), 1, indexStream);
				if(iwrite != 1)
				{
					char szMsg[FILTER_ERRMSG_LEN];
					sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
					throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
				}
				fclose(indexStream);
			}
			block.SetFlag(IDLE);
		}
	}
	//***************************************************************
	ResetList();

	//******************��־λ�ļ�*****************************
	CommitFlagStream = openfile(commitFile, "rb+");
	flag = '0';
	errno = 0;
	iwrite = fwrite(&flag, sizeof(char), 1, CommitFlagStream);
	if(iwrite != 1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "д״̬�ļ�=%s=����", commitFile);
		throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
	}
	fclose(CommitFlagStream);
	//CommitFlagStream.close();
	//***************************************************************
	//������ʱ����
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

//�����ʽ�����ļ���ȫ·��
//����: 	szTime
//			szFileName
//���:	resultFile
//����:	��
//�쳣:	��
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

//��ʼ�������ڴ�����ṹ
//����: 	��
//���:	��
//����:	��
//�쳣:	��
void C_CMemFilter::InitMem()
{
	theJSLog<< "init mem..." <<endi;
	long j=0;
	for(j=0; j<m_maxsource; j++)
	{
		pMemSH[j].type=IDLE;
		sem_init(&pMemSH[j].semid, 1, 1);
	}
	//�ļ��б�
	for(j=0; j<FILTER_MAXLOADFILE*m_maxsource-1; j++)
	{
		pMemIFH[j].type = IDLE;
		pMemIFH[j].forward = j-1;
		pMemIFH[j].backward = j+1;
	}
	//���һ��
	pMemIFH[j].type = IDLE;
	pMemIFH[j].forward = j-1;
	pMemIFH[j].backward = -1;

	//���ݿ��б�
	for(j=0; j<FILTER_MAXLOADBLOCK*FILTER_MAXLOADFILE*m_maxsource-1; j++)
	{
		pMemID[j].type= IDLE;
		pMemID[j].forward = j-1;
		pMemID[j].backward = j+1;
	}
	//���һ��
	pMemID[j].type = IDLE;
	pMemID[j].forward = j-1;
	pMemID[j].backward = -1;

	//�ָ��ļ��б�
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
		//����Դ��������
		pMemSH[i].idleBlockHead = (i*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK)
			+FILTER_MAXLOADFILE*FILTER_BLOCKNUM_PER_FILE;
		
		pMemSH[i].idleBlockEnd = (i+1)*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK -1 ;
		pMemID[pMemSH[i].idleBlockHead].forward = -1;
		pMemID[pMemSH[i].idleBlockEnd].backward = -1;

		//�ļ�����
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
//��ȡ����Դ����
void C_CMemFilter::InitConfig()
{
	//��ȡ���������ڴ���ļ�ȫ·��
}

//�����ϴδ���
//�˺���������������ȷ�ϴδ�������ʱ���ܵ���
//����: 	��
//���:	��
//����:	��
//�쳣:	CException ���ӹ����ڴ������ļ���д����
void C_CMemFilter::HandleLastError()
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		//TODO:throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�����Դ��");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return;
	}
	//*****************���������б�******************************
	ResetList();
	//*****************�б��������******************************
	
	//******************��־λ�ļ�*****************************
	char commitFile[FILTER_FILESIZE];
	sprintf(commitFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
		FILTER_WORK_PATH, m_szProcessId, COMMIT_FILE);

	FILE *CommitFlagStream = openfile(commitFile, "rb+");
	char flag = '0';
	fread(&flag, sizeof(char), 1, CommitFlagStream);
	fclose(CommitFlagStream);
	if('1' == flag)
	{
		//�ڻ�д��ʽ�ļ��Ĺ����г���

		//***********����ʱ�ļ��л�д�ļ�ͷ********************
		char tmpFile[FILTER_FILESIZE];
		sprintf(tmpFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
			FILTER_WORK_PATH, m_szProcessId, TMP_FILEINFO_NAME);
		//fstream tmpFileStream;
		//openFile(tmpFileStream, tmpFile);
		FILE * tmpFileStream = openfile(tmpFile, "rb+");
		char szIndexFile[FILTER_FILESIZE];
		IndexFileHead tmpfilehead;
		//������ʱ�ļ��ļ�ͷ��Ϣ
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
				sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			}
			fclose(indexStream);
			//indexStream.close();
		}
		fclose(tmpFileStream);
		//tmpFileStream.close();
		//***********�ӱ����ļ��л�д�ļ�ͷ************************
		sprintf(tmpFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
			FILTER_WORK_PATH, m_szProcessId, BACKUP_FILEINFO_NAME);
		//openFile(tmpFileStream, tmpFile);
		tmpFileStream = openfile(tmpFile, "rb+");
		//������ʱ�ļ��ļ�ͷ��Ϣ
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
				sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			}
			fclose(indexStream);
		}
		fclose(tmpFileStream);
		
		//*************����ʱ�ļ��л�д���ݿ�*****************
		char tmpDataFile[FILTER_FILESIZE];
		sprintf(tmpDataFile, "%s%s/%s/%s_%s", m_szIndexPath, m_szSourceid, 
			FILTER_WORK_PATH, m_szProcessId, TMP_DATA_NAME);
		//fstream tmpDataFileStream;
		//openFile(tmpDataFileStream, tmpDataFile);
		FILE *tmpDataFileStream = openfile(tmpDataFile, "rb+");
		IndexData tempIndexdata;
		//������ʱ�����ļ�
		for(;;)
		{
			rget = fread(&tempIndexdata, sizeof(IndexData), 1, tmpDataFileStream);
			if(rget != 1)
			{
				break;
			}
			//�ļ�������ʱ�����Ϣ
			GetIndexFile(tempIndexdata.dataHead.szFileName, tempIndexdata.dataHead.szFileName, szIndexFile);
			FILE *indexStream = openfile(szIndexFile, "rb+");
			fseek(indexStream, sizeof(IndexFileHead) + (tempIndexdata.dataHead.blockNo)*sizeof(IndexData), SEEK_SET);
			errno = 0;
			int iwrite = fwrite(&tempIndexdata, sizeof(IndexData), 1, indexStream);
			if(iwrite != 1)
			{
			 	char szMsg[FILTER_ERRMSG_LEN];
				sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			 }
			fclose(indexStream);
			//indexStream.close();
		}
		fclose(tmpDataFileStream);
		//tmpDataFileStream.close();
		
		//*************�ӱ����ļ��л�д���ݿ�***********
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
			//�ļ�������ʱ�����Ϣ
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
				sprintf(szMsg, "д�����ļ�=%s=����", szIndexFile);
				throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
			 }
			fclose(indexStream);
			//indexStream.close();
		}
		fclose(tmpDataFileStream);
		//tmpDataFileStream.close();
	}

	//������ʱ����
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
		sprintf(szMsg, "д״̬��¼�ļ�=%s=����", commitFile);
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
	//��������Դ
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
		
	//�����ļ�ͷ
	//ikey = ftok(m_szCommemFile, FILTER_FILE_INDEX); 
	ikey = filterFileKey;
	ishmid = shmget(ikey, 0, 0666);
	if(pMemIFH==NULL)
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, NULL, 0);
	else
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, pMemIFH, 0);
		
	//�������ݿ�
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
		theJSLog << "û�д�����Դ " <<sourceid<<endi;
		return ;
	}

	theJSLog << "���� " <<sourceid<<endi;
	sem_post(&(pMemSH[tempindex].semid));

}

void C_CMemFilter::SemLock(const char* comfile, const char* sourceid)
{
	//key_t ikey = ftok(comfile, FILTER_SOURCE_INDEX); 
	key_t ikey = filterSourceKey;
	//��������Դ
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
		
	//�����ļ�ͷ
	//ikey = ftok(m_szCommemFile, FILTER_FILE_INDEX); 
	ikey = filterFileKey;
	ishmid = shmget(ikey, 0, 0666);
	if(pMemIFH==NULL)
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, NULL, 0);
	else
		pMemIFH= (MemIndexFileHead*)shmat(ishmid, pMemIFH, 0);
		
	//�������ݿ�
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


//�Ƿ��д�ļ�д��һ��
bool C_CMemFilter::IsWriteFileError()
{
	//******************��־λ�ļ�*****************************
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

//�Ƿ����ڴ����쳣
int C_CMemFilter::IsMemError()
{
	if(m_sourceindex == (SIZE_TYPE)-1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�����Դ��");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
		return 1;
	}

	SIZE_TYPE f_curr = pMemSH[m_sourceindex].beginIndex;  //����ڴ��ļ���ַ
	SIZE_TYPE f_cure_old=-1;
	int check_mem_err = 0;
	//theJSLog << "�������Դ�Ĺ����ڴ�Ϸ��� " << endi;
	for(; f_curr!=pMemSH[m_sourceindex].endIndex; f_curr=pMemIFH[f_curr].backward)
	{
		if(f_curr <(SIZE_TYPE)0){
			// �±����
			theJSLog << "�ڴ��쳣��f_curr:" << f_curr << endi;
			return 123456;
		}

		if(f_cure_old != -1 && f_cure_old != pMemIFH[f_curr].forward ){
			// ��ǰ�ڵ��ǰ���ڵ㲻�����ϴη��ʵĽڵ㣬����
			return 123457;
		}
		f_cure_old = f_curr;

//		if(pMemIFH[f_curr].forward < (SIZE_TYPE)0 && f_curr != pMemSH[m_sourceindex].endIndex)
//			// ��ǰ�ڵ㻹����β�ڵ㣬��forward<0������
//			return 123458;


		SIZE_TYPE d_curr = pMemIFH[f_curr].memDataListHead;	//������ݿ��ַ
		SIZE_TYPE d_curr_old = -1;
	  //theJSLog << "f_curr == " << f_curr << ",d_curr ==" << d_curr << endi;
		for(; d_curr!=(SIZE_TYPE)-1; d_curr=pMemID[d_curr].backward)
		{

			if(d_curr < (SIZE_TYPE)0){
				// �±����
				return 1000006;
			}

			if(d_curr_old != -1 && d_curr_old != pMemID[d_curr].forward ){
				// ��ǰ�ڵ��ǰ���ڵ㲻�����ϴη��ʵĽڵ㣬����
				return 1000007;
			}
			d_curr_old = d_curr;

//			if(pMemID[d_curr].forward < (SIZE_TYPE)0 && f_curr != pMemIFH[f_curr].memDataListEnd)
//						// ��ǰ�ڵ㻹����β�ڵ㣬��forward<0������
//						return 1000008;

			if (pMemID[d_curr].type!=IDLE && pMemID[d_curr].indexData.dataHead.indexInBlock == 0)
			{	
				theJSLog << "�ڴ��쳣��fileno:" << f_curr << ",blockno:" << d_curr << ",block flag:" << pMemID[d_curr].type << ",count:" << pMemID[d_curr].indexData.dataHead.indexInBlock << endi;
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
		sprintf(szMsg, "��δ���ӹ����ڴ�����Դ��");
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
		//����������ļ���
		filelock.Lock();
		if(-1 == kill(pMemSH[source_idx].iprocessId, 0))
		{
			theJSLog << "�����Ľ���pid(" << pMemSH[source_idx].iprocessId << ")������" <<endi;
			if(IsWriteFileError())
			{
				theJSLog <<"�ļ���д��һ��" << endi;
				//�Ѿ�д��һ����,ֻ�����������ļ�
				if(strcmp(pMemSH[source_idx].szDealFileName, filename) == 0)
				{
					theJSLog << "�����ϴδ���"<<endi;
					HandleLastError();
					//�ǼǶ�ռ������Դ�Ľ���id
					pMemSH[source_idx].iprocessId = getpid();
					//����������ļ���
					filelock.UnLock();
					return true;
				}
				else
				{
					theJSLog << pMemSH[source_idx].szDealFileName
					<<"�ļ��ύ��һ��,ֻ�ܵȴ����ļ����´�����ɺ��ֹͣ����!!!!"<<endw;
				}
			}
			else
			{
				theJSLog <<"�ļ���û�л�д" << endi;
				theJSLog << "�����ϴδ���"<<endi;
				HandleLastError();
				//�ǼǶ�ռ������Դ�Ľ���id
				pMemSH[source_idx].iprocessId = getpid();
				//����������ļ���
				filelock.UnLock();
				return true;
			}
		}
		else
		{
			theJSLog << "�����Ľ���pid(" << pMemSH[source_idx].iprocessId << ")����" <<endi;
			//���̴�����һֱ����
		}
		//����������ļ���
		filelock.UnLock();
		theJSLog << "�ȴ���������...."<<endi;
		//sleep(10);
	}
	else
	{
		return true;
	}

	return false;
}

//��ȡ���п�
SIZE_TYPE C_CMemFilter::GetIdlBlock()
{
	//���з�ʽ
	SIZE_TYPE oldHead;
	if(pMemSH[m_sourceindex].idleBlockHead == (SIZE_TYPE)-1)
	{
		//���ж����޿��п�
		//�����δʹ�õ��ļ��б�ʼ
		//�����ļ��б��п�������FILTER_BLOCKNUM_PER_FILE���ļ�����
		//ȡ������տ����,�ŵ����п���
		int getCount=0;	//��ʶ�Ѿ�ȡ�����ٸ��տ����
		
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
			dataListId = pMemIFH[fileListId].memDataListEnd;	//��ȡҪȡ�����Ŀ�
			dataListQ = pMemID[dataListId].forward;	//��ȡĿ����ǰ��һ��
			pMemID[dataListId].forward = -1;		//��Ҫȡ�����Ŀ����
			pMemID[dataListQ].backward = -1;				//����ǰ��һ����dataListQΪβ��
			pMemIFH[fileListId].memDataListEnd = dataListQ;	//����ǰ��һ����dataListQΪβ��
			pMemIFH[fileListId].memDataCount--;			//����������һ
			
			//��ȡ�������ݿ�ԭʼ��Ϣд����ʱ�ļ�����
			memblock.AttachReSource(pMemID, dataListId);
			memblock.Write2Temp(m_szIndexPath, m_szProcessId);
			memblock.SetFlag(IDLE);

			//��ȡ�����Ŀ������ж�����
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
			//���п�����1
			getCount++;
			if(getCount >= 2)
			{
				break;
			}	
		}	
	}
	
	//���ж������п��п�
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
	//*****************���������б�******************************
	pMemSH[m_sourceindex].beginIndex = m_sourceindex*FILTER_MAXLOADFILE;
	pMemSH[m_sourceindex].endIndex = (m_sourceindex+1)*FILTER_MAXLOADFILE-1;
	SIZE_TYPE i=m_sourceindex*FILTER_MAXLOADFILE;
	for(; i<(m_sourceindex+1)*FILTER_MAXLOADFILE; i++)
	{
		pMemIFH[i].type = IDLE;
		pMemIFH[i].forward = i-1;
		pMemIFH[i].backward = i+1;
	}
	//��һ���ļ��б�
	pMemIFH[m_sourceindex*FILTER_MAXLOADFILE].forward=-1;
	//���һ���ļ��б�
	pMemIFH[(m_sourceindex+1)*FILTER_MAXLOADFILE-1].backward=-1;
	SIZE_TYPE j = m_sourceindex*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK;
	//��һ�����ݿ�
	pMemID[j].forward = -1;
	for(; j<(m_sourceindex+1)*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK; j++)
	{
		pMemID[j].type = IDLE;
		pMemID[j].forward = j-1;
		pMemID[j].backward = j+1;
	}
	//���һ�����ݿ�
	pMemID[j-1].backward = -1;
	
	pMemSH[m_sourceindex].idleBlockHead = (m_sourceindex*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK)
		+FILTER_MAXLOADFILE*FILTER_BLOCKNUM_PER_FILE;
	
	pMemSH[m_sourceindex].idleBlockEnd = (m_sourceindex+1)*FILTER_MAXLOADFILE*FILTER_MAXLOADBLOCK-1;
	pMemID[pMemSH[m_sourceindex].idleBlockHead].forward = -1;
	pMemID[pMemSH[m_sourceindex].idleBlockEnd].backward = -1;

	//�����ļ����������ݿ�����Ĺ�ϵ
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

	//*****************�б��������******************************

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
			sprintf(szMsg, "%d �ظ����ļ��б�=%d", i, k);
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



