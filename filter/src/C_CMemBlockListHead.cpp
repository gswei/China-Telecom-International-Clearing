
#include "C_CMemBlockListHead.h"

//从共享内存中获取资源
void CMemBlockListHead::AttachReSource(MemIndexFileHead* memIndexFileHead, SIZE_TYPE index)
{
	m_pdata = memIndexFileHead;
	m_index = index;
}
//释放资源
void CMemBlockListHead::DetachReSource()
{
	m_pdata = NULL;
	m_index = -1;
}

inline bool CMemBlockListHead::IsAttached()
{
	return (m_pdata==NULL) ? false:true;
}

//将资源数据清空
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
	//在临时文件中查找
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

//*****************从文件中获取数据******************
//***********不修改链接关系,只修改内容***********
bool CMemBlockListHead::LoadData(const char* SourceId, const char* szPath, const char* szTime, const char* processid)
//bool CMemBlockListHead::LoadData(char* SourceId, char* szPath, char* szTime, char* processid)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存文件块");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	Clear();
	
	//如果在临时文件中找到就直接从临时文件中读取
	char szTmpFileInfoFile[FILTER_FILESIZE];		//临时数据块块头文件
	sprintf(szTmpFileInfoFile, "%s%s/%s/%s_%s", szPath, SourceId, FILTER_WORK_PATH, processid, TMP_FILEINFO_NAME);
	
	char szIndexFileName[FILTER_FILESIZE];	//正式索引文件
	memset(szIndexFileName, 0, sizeof(szIndexFileName));
	strncpy(szIndexFileName, szTime, FILTER_INDEX_FILE_LEN);
	strcat(szIndexFileName, ".idx");

	//*****************此处拼接全路径******************************
	char szIndexFile[FILTER_FILESIZE];	//正式索引文件(全路径)
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
			sprintf(szMsg, "读取文件失败=%s=", szTmpFileInfoFile);
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
			sprintf(szMsg, "读取文件失败=%s=", szTmpFileInfoFile);
			throw CException(FILTER_ERR_IN_READ_FILE, szMsg, __FILE__, __LINE__);
		 }
		
		tmpfileStream.close();
		*/
		ChangeFlag(INTEMP);
	}
	//否则从正式文件中读取
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

//回写到临时文件
//参数: 	TmpPath	根目录(带'/')
//			processid	进程标识
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
			sprintf(szMsg, "写文件失败=%s=", TmpFileHeadName);
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
		fclose(fileHeadStream);
		//fileHeadStream.close();
		ChangeFlag(INTEMP);
	}
}

//将内存中的文件头回写到备份文件
void CMemBlockListHead::Backup(FILE * backupStream)
{
	if(backupStream == NULL)
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "无法打开备份文件流");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	if(INMEM == GetFlag())
	{
		errno = 0;
		int iwrite = fwrite(&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead), 1, backupStream);
		if (iwrite != 1)
		{
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写备份文件失败");
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
		sprintf(szMsg, "无法打开备份文件流");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	if(INMEM == GetFlag())
	{
		backupStream.write((char*)&m_pdata[m_index].indexFileHead, sizeof(IndexFileHead));
		if ( !backupStream.good())
		{
		 	//throw here
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写备份文件失败");
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
	}
}
*/


//修改标志
inline void CMemBlockListHead::ChangeFlag(DataType dataType)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].type = dataType;
}

//设置指向的数据块列表的头指针
inline void CMemBlockListHead::SetDataIndexHead(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].memDataListHead = indexNo;
}
//获取指向的数据块列表的头指针
SIZE_TYPE CMemBlockListHead::GetDataIndexHead()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].memDataListHead;
}

//设置指向的数据块列表的尾指针
inline void CMemBlockListHead::SetDataIndexEnd(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].memDataListEnd= indexNo;
}
//获取指向的数据块列表的尾指针
SIZE_TYPE CMemBlockListHead::GetDataIndexEnd()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].memDataListEnd;
}


//设置前导指针
inline void CMemBlockListHead::SetForward(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].forward = indexNo;
}
//获取前导指针
inline SIZE_TYPE CMemBlockListHead::GetForward()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].forward;
}

//设置后续指针
inline void CMemBlockListHead::SetBackward(SIZE_TYPE indexNo)
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].backward = indexNo;
}
//获取后续指针
inline SIZE_TYPE CMemBlockListHead::GetBackward()
{
	if(!IsAttached())
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].backward;
}

//校验pTime是否在此列表中
bool CMemBlockListHead::IsContain(const char* pTime)
{
	if(!IsAttached() ||pTime==NULL)
	{
		//throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存或时间pTime为空指针");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(strncmp(m_pdata[m_index].indexFileHead.szTime, pTime, FILTER_INDEX_FILE_LEN)==0)
		return true;

	return false;
}

//获取pTime在文件中的首块号
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
		sprintf(szMsg, "尚未连接共享内存");
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
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexFileHead.blockInFile = blockcount;
	ChangeFlag(INMEM);
}


		
