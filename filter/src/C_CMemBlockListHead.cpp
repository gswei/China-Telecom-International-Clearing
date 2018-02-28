
#include "C_CMemBlockListHead.h"

//�ӹ����ڴ��л�ȡ��Դ
void CMemBlockListHead::AttachReSource(MemIndexFileHead* memIndexFileHead, SIZE_TYPE index)
{
	m_pdata = memIndexFileHead;
	m_index = index;
}
//�ͷ���Դ
void CMemBlockListHead::DetachReSource()
{
	m_pdata = NULL;
	m_index = -1;
}

inline bool CMemBlockListHead::IsAttached()
{
	return (m_pdata==NULL) ? false:true;
}

//����Դ�������
void CMemBlockListHead::Clear()
{
	memset((char*)&m_pdata[m_index].indexFileHead, 0, sizeof(IndexFileHead));
	//m_pdata[m_index].iProcessIndex = -1;
	//m_pdata[m_index].memDataListHead = -1;
	//ChangeFlag(INFILE);
}

bool CMemBlockListHead::FindFileInTemp(const char* sourceId, const char *tmpfile, const char* indexfile, SIZE_TYPE& location )
{
	char szTmpFile[FILTER_FILESIZE];
	sprintf(szTmpFile, tmpfile);
	FILE *filestream = openfile(szTmpFile, "rb+");

	location = -1;
	IndexFileHead tmpfilehead;
	bool bRetValue = false;
	int get = 0;
	for ( ; ; )
	{
		get = fread(&tmpfilehead, sizeof(IndexFileHead), 1, filestream);
		location++;
		if (get != 1)
		{
			bRetValue = false;
			break;
		}
		if (strcmp(tmpfilehead.szSourceId, sourceId)==0 && strcmp(tmpfilehead.szFileName, indexfile)==0)
		{
			bRetValue = true;
			break;
		}
	}
	fclose(filestream);
	
	/*
	fstream filestream;
	char szTmpFile[FILTER_FILESIZE];
	sprintf(szTmpFile, tmpfile);
	openFile(filestream, szTmpFile);

	location = -1;
	IndexFileHead tmpfilehead;
	bool bRetValue = false;
	//����ʱ�ļ��в���
	for ( ; ; )
	{
		filestream.read((char *)&tmpfilehead, sizeof(IndexFileHead));
		location++;
		if (filestream.gcount() < sizeof(IndexFileHead))
		{
			bRetValue = false;
			break;
		}
		if (strcmp(tmpfilehead.szSourceId, sourceId)==0 && strcmp(tmpfilehead.szFileName, indexfile)==0)
		{
			bRetValue = true;
			break;
		}
	}
	
	filestream.close();
	*/
	return bRetValue;
}

//*****************���ļ��л�ȡ����******************
//***********���޸����ӹ�ϵ,ֻ�޸�����***********
bool CMemBlockListHead::LoadData(const char* SourceId, const char* szPath, const char* szTime, const char* processid)
//bool CMemBlockListHead::LoadData(char* SourceId, char* szPath, char* szTime, char* processid)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ��ļ���");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	Clear();
	
	//�������ʱ�ļ����ҵ���ֱ�Ӵ���ʱ�ļ��ж�ȡ
	char szTmpFileInfoFile[FILTER_FILESIZE];		//��ʱ���ݿ��ͷ�ļ�
	sprintf(szTmpFileInfoFile, "%s%s/%s/%s_%s", szPath, SourceId, FILTER_WORK_PATH, processid, TMP_FILEINFO_NAME);
	
	char szIndexFileName[FILTER_FILESIZE];	//��ʽ�����ļ�
	memset(szIndexFileName, 0, sizeof(szIndexFileName));
	strncpy(szIndexFileName, szTime, FILTER_INDEX_FILE_LEN);
	strcat(szIndexFileName, ".idx");

	//*****************�˴�ƴ��ȫ·��******************************
	char szIndexFile[FILTER_FILESIZE];	//��ʽ�����ļ�(ȫ·��)
	char szYMonth[7];
	memset(szYMonth, 0, sizeof(szYMonth));
	strncpy(szYMonth, szTime, 6);
	char szDay[3];
	memset(szDay, 0, sizeof(szDay));
	strncpy(szDay, szTime+6, 2);
	
	sprintf(szIndexFile, "%s%s/%s/%s/%s", szPath, SourceId, szYMonth, szDay, szIndexFileName);
	//*********************************************************************

	SIZE_TYPE location;
	if(FindFileInTemp(SourceId, szTmpFileInfoFile, szIndexFileName, location))
	{
		FILE *tmpfileStream = openfile(szTmpFileInfoFile, "rb+");
		fseek(tmpfileStream, location*sizeof(IndexFileHead), SEEK_SET);
		int get = fread(&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead), 1, tmpfileStream);
		if(get != 1)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "��ȡ�ļ�ʧ��=%s=", szTmpFileInfoFile);
			throw CException(FILTER_ERR_IN_READ_FILE, szMsg, __FILE__, __LINE__);
		}
		fclose(tmpfileStream);
		/*
		fstream tmpfileStream;
		openFile(tmpfileStream, szTmpFileInfoFile);
		tmpfileStream.seekp(location*sizeof(IndexFileHead), ios::beg);
		tmpfileStream.read((char*)&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead));

		if (!(tmpfileStream.good()))
		{
		 	//throw here
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "��ȡ�ļ�ʧ��=%s=", szTmpFileInfoFile);
			throw CException(FILTER_ERR_IN_READ_FILE, szMsg, __FILE__, __LINE__);
		 }
		
		tmpfileStream.close();
		*/
		ChangeFlag(INTEMP);
	}
	//�������ʽ�ļ��ж�ȡ
	else
	{
		ChangeFlag(INFILE);
		FILE * fileStream = openfile(szIndexFile, "rb+");
		int rget = fread(&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead), 1, fileStream);
		if(rget != 1)
		{
			Clear();
			sprintf(m_pdata[m_index].indexFileHead.szFileName, "%s", szIndexFileName);
			sprintf(m_pdata[m_index].indexFileHead.szSourceId, "%s", SourceId);
			memset(m_pdata[m_index].indexFileHead.szTime, 0, sizeof(m_pdata[m_index].indexFileHead.szTime));
			strncpy(m_pdata[m_index].indexFileHead.szTime, szTime, FILTER_INDEX_FILE_LEN);
			m_pdata[m_index].indexFileHead.blockInFile = 1;
			for(int i=0; i<FILTER_MAXSECONDINFILE; i++)
			{
				m_pdata[m_index].indexFileHead.blockItem[i] = 0;
			}
			ChangeFlag(INMEM);
		}
		fclose(fileStream);
	}
	return true;
}
//************************************************************

//��д����ʱ�ļ�
//����: 	TmpPath	��Ŀ¼(��'/')
//			processid	���̱�ʶ
void CMemBlockListHead::Write2Temp(const char* TmpPath, const char* processid)
{
	char TmpFileHeadName[FILTER_FILESIZE];
	sprintf(TmpFileHeadName, "%s%s/%s/%s_%s", TmpPath, m_pdata[m_index].indexFileHead.szSourceId, 
		FILTER_WORK_PATH, processid, TMP_FILEINFO_NAME);
	
	if(INMEM == GetFlag())
	{
		SIZE_TYPE location;
		FindFileInTemp(m_pdata[m_index].indexFileHead.szSourceId, TmpFileHeadName, 
			m_pdata[m_index].indexFileHead.szFileName, location);

		//fstream fileHeadStream;
		//openFile(fileHeadStream, TmpFileHeadName);
		//fileHeadStream.seekp(location*sizeof(IndexFileHead), ios::beg);
		//fileHeadStream.write((char*)&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead));
		//if ( !fileHeadStream.good())
		FILE* fileHeadStream = openfile(TmpFileHeadName, "rb+");
		fseek(fileHeadStream, location*sizeof(IndexFileHead), SEEK_SET);
		errno = 0;
		int iwrite = fwrite(&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead), 1, fileHeadStream);
		if(iwrite != 1)
		{
		 	//throw here
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "д�ļ�ʧ��=%s=", TmpFileHeadName);
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
		fclose(fileHeadStream);
		//fileHeadStream.close();
		ChangeFlag(INTEMP);
	}
}

//���ڴ��е��ļ�ͷ��д�������ļ�
void CMemBlockListHead::Backup(FILE * backupStream)
{
	if(backupStream == NULL)
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "�޷��򿪱����ļ���");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	if(INMEM == GetFlag())
	{
		errno = 0;
		int iwrite = fwrite(&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead), 1, backupStream);
		if (iwrite != 1)
		{
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "д�����ļ�ʧ��");
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
	}
}

/*
void CMemBlockListHead::Backup(fstream& backupStream)
{
	if(backupStream == NULL)
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "�޷��򿪱����ļ���");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	if(INMEM == GetFlag())
	{
		backupStream.write((char*)&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead));
		if ( !backupStream.good())
		{
		 	//throw here
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "д�����ļ�ʧ��");
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
	}
}
*/


//�޸ı�־
inline void CMemBlockListHead::ChangeFlag(DataType dataType)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].type = dataType;
}

//����ָ������ݿ��б��ͷָ��
inline void CMemBlockListHead::SetDataIndexHead(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].memDataListHead = indexNo;
}
//��ȡָ������ݿ��б��ͷָ��
SIZE_TYPE CMemBlockListHead::GetDataIndexHead()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].memDataListHead;
}

//����ָ������ݿ��б��βָ��
inline void CMemBlockListHead::SetDataIndexEnd(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].memDataListEnd= indexNo;
}
//��ȡָ������ݿ��б��βָ��
SIZE_TYPE CMemBlockListHead::GetDataIndexEnd()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].memDataListEnd;
}


//����ǰ��ָ��
inline void CMemBlockListHead::SetForward(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].forward = indexNo;
}
//��ȡǰ��ָ��
inline SIZE_TYPE CMemBlockListHead::GetForward()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].forward;
}

//���ú���ָ��
inline void CMemBlockListHead::SetBackward(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].backward = indexNo;
}
//��ȡ����ָ��
inline SIZE_TYPE CMemBlockListHead::GetBackward()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].backward;
}

//У��pTime�Ƿ��ڴ��б���
bool CMemBlockListHead::IsContain(const char* pTime)
{
	if(!IsAttached() ||pTime==NULL)
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ��ʱ��pTimeΪ��ָ��");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(strncmp(m_pdata[m_index].indexFileHead.szTime, pTime, FILTER_INDEX_FILE_LEN)==0)
		return true;

	return false;
}

//��ȡpTime���ļ��е��׿��
bool CMemBlockListHead::GetBlockIndexNo(const char*pTime, SIZE_TYPE second, SIZE_TYPE &blockNo)
{
	if(!IsContain(pTime))
		return false;

	SIZE_TYPE sec = second%3600;
	blockNo  = m_pdata[m_index].indexFileHead.blockItem[sec];
	return true;
}

DataType CMemBlockListHead::GetFlag()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].type;
}


SIZE_TYPE CMemBlockListHead::GetTotalBlockCount()
{
	return m_pdata[m_index].indexFileHead.blockInFile;
}

void CMemBlockListHead::SetTotalBlockCount(SIZE_TYPE blockcount)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexFileHead.blockInFile = blockcount;
	ChangeFlag(INMEM);
}


		
