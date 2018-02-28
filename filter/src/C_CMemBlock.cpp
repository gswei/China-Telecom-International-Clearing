
#include "C_CMemBlock.h"

//获取共享内存的资源
void CMemBlock::AttachReSource(MemIndexData* memIndexdata, SIZE_TYPE index)
{
	this->m_pdata = memIndexdata;
	this->m_index = index;
}

//释放资源
inline void CMemBlock::DetachReSource()
{
	this->m_pdata = NULL;
	this->m_index = -1;
}

//判断是否已经连接着
inline bool CMemBlock::IsAttached()
{
	return (m_pdata==NULL) ? false:true;
}

//将数据清空
void CMemBlock::ClearData()
{
	memset((char*)&m_pdata[m_index].indexData, 0, sizeof(IndexData));
	memset(m_pdata[m_index].szTime, 0, sizeof(m_pdata[m_index].szTime));
	SetBlockNo(-1);
	SetIndexCount(0);
	SetNextBlock(-1);
	//SetFlag(INFILE);
}

//在临时数据块块头文件中,查找数据块块头
//参数:	tmpfile存储临时数据块块头的文件
//			indexfile	索引文件名(不包含路径)
//			blockno	索引文件中的块号
//			location	indexfile.blockno的信息存储在tmpfile中的位置

bool CMemBlock::FindHeaderInTemp(const char* sourceId, const char *tmpfile, const char* indexfileName, const SIZE_TYPE blockno, SIZE_TYPE &location )
{
	//fstream filestream;
	char szTmpFile[FILTER_FILESIZE];
	sprintf(szTmpFile, tmpfile);
	FILE *fileid = openfile(szTmpFile, "rb+");

	location = -1;
	IndexDataHead tmpfilehead;
	bool bRetValue = false;
	//在临时文件中查找
	for ( ; ; )
	{
		int get = fread(&tmpfilehead, sizeof(IndexDataHead), 1, fileid);
		location++;
		if (get != 1)
		{
			bRetValue = false;
			break;
		}
		if ((strcmp(tmpfilehead.szFileName, indexfileName) == 0) 
			&& (strcmp(tmpfilehead.szSourceId, sourceId) == 0) && (tmpfilehead.blockNo == blockno))
		{
			bRetValue = true;
			break;
		}
	}
	fclose(fileid);

	
	/*
	openFile(filestream, szTmpFile);

	location = -1;
	IndexDataHead tmpfilehead;
	bool bRetValue = false;
	//在临时文件中查找
	for ( ; ; )
	{
		filestream.read((char *)&tmpfilehead, sizeof(IndexDataHead));
		location++;
		if ((strcmp(tmpfilehead.szFileName, indexfileName) == 0) 
			&& (strcmp(tmpfilehead.szSourceId, sourceId) == 0) && (tmpfilehead.blockNo == blockno))
		{
			bRetValue = true;
			break;
		}
		if (filestream.gcount() < sizeof(IndexDataHead))
		{
			bRetValue = false;
			break;
		}
	}
	
	filestream.close();
	*/
	return bRetValue;
	
}


//*****************从文件中获取数据*************************
//***********不修改链接关系,只修改内容******************
bool CMemBlock::LoadData(const char* SourceId, const char* szPath, SIZE_TYPE blockNo, const char* szTime, const char*process_id)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	ClearData();
	
	//如果在临时文件中找到就直接从临时文件中读取
	char szTmpDataHeadFile[FILTER_FILESIZE];		//临时数据块块头文件
	sprintf(szTmpDataHeadFile, "%s%s/%s/%s_%s", szPath, SourceId, FILTER_WORK_PATH, process_id, TMP_DATAHEAD_NAME);
	
	char szIndexFileName[FILTER_FILESIZE];	//正式索引文件名
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
	
	//设置读取的索引文件名
	//SetBlockFileName(szIndexFile);
	SIZE_TYPE location;
	if(FindHeaderInTemp(SourceId, szTmpDataHeadFile, szIndexFileName, blockNo, location))
	{
		
		char szTmpDataFile[FILTER_FILESIZE];		//临时数据块文件
		sprintf(szTmpDataFile, "%s%s/%s/%s_%s", szPath, SourceId, FILTER_WORK_PATH, process_id, TMP_DATA_NAME);
		FILE *fileid1 = openfile(szTmpDataFile, "rb+");
		fseek(fileid1, location*sizeof(IndexData), SEEK_SET);
		int get = fread(&m_pdata[m_index].indexData, sizeof(IndexData), 1, fileid1);
		if(get != 1)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "读取文件失败=%s=", szTmpDataHeadFile);
			throw CException(FILTER_ERR_IN_READ_FILE, szMsg, __FILE__, __LINE__);
		}
		fclose(fileid1);
		ChangeFlag(INTEMP);
	}
	//否则从正式文件中读取
	else
	{
		ChangeFlag(INFILE);
		FILE *fileid = openfile(szIndexFile, "rb+");

		fseek(fileid, sizeof(IndexFileHead) + blockNo*sizeof(IndexData), SEEK_SET);
		int get = fread(&m_pdata[m_index].indexData, sizeof(IndexData), 1, fileid);
		if(get != 1)
		{
			ClearData();
			SetBlockFileName(szIndexFileName);
			SetSourceID(SourceId);
			SetIndexCount(0);
			sprintf(m_pdata[m_index].szTime, szTime);
			SetBlockNo(blockNo);
			ChangeFlag(INMEM);
		}
		fclose(fileid);
	}
	return true;
}
//************************************************************

//将数据回写到临时文件
//输入:	存放临时文件的路径(最后带'/')
//			此path为根目录
void CMemBlock::Write2Temp(const char* TmpPath, const char*processid)
{
	char TmpDataHeadFileName[FILTER_FILESIZE];
	char TmpDataFileName[FILTER_FILESIZE];
	sprintf(TmpDataHeadFileName, "%s%s/%s/%s_%s", TmpPath, m_pdata[m_index].indexData.dataHead.szSourceId
		, FILTER_WORK_PATH, processid, TMP_DATAHEAD_NAME);
	
	sprintf(TmpDataFileName, "%s%s/%s/%s_%s", TmpPath, m_pdata[m_index].indexData.dataHead.szSourceId
		, FILTER_WORK_PATH, processid, TMP_DATA_NAME);
	
	if(INMEM == GetFlag())
	{
		SIZE_TYPE location;
		FindHeaderInTemp(GetSourceID(), TmpDataHeadFileName, GetBlockFileName(), GetBlockNo(), location);
		FILE *dataHeadStream = openfile(TmpDataHeadFileName, "rb+");
		FILE *dataStream = openfile(TmpDataFileName, "rb+");
		errno =0;
		fseek(dataHeadStream, location*sizeof(IndexDataHead), SEEK_SET);
		int iwrite = fwrite(&m_pdata[m_index].indexData.dataHead, sizeof(IndexDataHead), 1, dataHeadStream);
		if(iwrite != 1)
		{
			fclose(dataHeadStream);
			fclose(dataStream);
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写文件失败=%s=", TmpDataHeadFileName);
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		}
		errno = 0;
		fseek(dataStream, location*sizeof(IndexData), SEEK_SET);
		iwrite = fwrite(&m_pdata[m_index].indexData, sizeof(IndexData), 1, dataStream);
		if(iwrite != 1)
		{
			fclose(dataHeadStream);
			fclose(dataStream);
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写文件失败=%s=", TmpDataFileName);
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		}
		fclose(dataHeadStream);
		fclose(dataStream);
		ChangeFlag(INTEMP);
	}
}

/*
void CMemBlock::Backup(fstream &backupStream)
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
		backupStream.write((char*) &m_pdata[m_index].indexData, sizeof(IndexData));
		if ( !backupStream.good())
		{
		 	//TODO throw here
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写备份文件失败");
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
	}
}
*/

void CMemBlock::Backup(FILE *backupStream)
{
	if(backupStream == NULL)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "无法打开备份文件流");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	if(INMEM == GetFlag())
	{
		errno = 0;
		int iwrite = fwrite(&m_pdata[m_index].indexData, sizeof(IndexData), 1, backupStream);
		if ( iwrite != 1)
		{
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "写备份文件失败");
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
	}
}

//修改标志
void CMemBlock::ChangeFlag(DataType dataType)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].type = dataType;
}

//获取数据块标志
DataType CMemBlock::GetFlag()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].type;
}

void CMemBlock::SetFlag(DataType flag)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].type = flag;
}


//设置前导指针号
inline void CMemBlock::SetForward(SIZE_TYPE forward)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].forward = forward;
}

//获取前导指针号
inline SIZE_TYPE CMemBlock::GetForward()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].forward;
}

//设置后续指针号
inline void CMemBlock::SetBackward(SIZE_TYPE backward)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].backward = backward;
}

//获取后续指针号
inline SIZE_TYPE CMemBlock::GetBackward()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].backward;
}

//获取块号
SIZE_TYPE CMemBlock::GetBlockNo()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.blockNo;
}
//设置块号
void CMemBlock::SetBlockNo(SIZE_TYPE blockno)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.dataHead.blockNo = blockno;
}

//获取索引文件名
char* CMemBlock::GetBlockFileName()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.szFileName;
}
//设置索引文件名
inline void CMemBlock::SetBlockFileName(const char* filename)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	sprintf(m_pdata[m_index].indexData.dataHead.szFileName, filename);
}

//获取块号
inline char* CMemBlock::GetSourceID()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.szSourceId;
}
//设置块号
inline void CMemBlock::SetSourceID(const char* sourceid)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	sprintf(m_pdata[m_index].indexData.dataHead.szSourceId, sourceid);
}


//获取文件块的下一个块号
SIZE_TYPE CMemBlock::GetNextBlock()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	
	return m_pdata[m_index].indexData.dataHead.nextBlock;
}
//设置文件块的下一个块号
void CMemBlock::SetNextBlock(SIZE_TYPE count)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	
	m_pdata[m_index].indexData.dataHead.nextBlock = count;
}


//获取块内索引数目
SIZE_TYPE CMemBlock::GetIndexCount()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.indexInBlock;
}

//设置块内索引数目
void CMemBlock::SetIndexCount(SIZE_TYPE count)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.dataHead.indexInBlock = count;
}


//是否填满
bool CMemBlock::IsFull()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(GetIndexCount() >= (long)FILTER_MAXINDEXNO)
	{
		return true;
	}

	return false;
}

//从0开始计数
void CMemBlock::GetKey(SIZE_TYPE index, IndexDataValue& value)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index>GetIndexCount() ||index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "获取索引值失败,index=%d 不在正常范围,GetIndexCount()=%d;", index,GetIndexCount());
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	value = m_pdata[m_index].indexData.data[index];
}

//从0开始计数
SIZE_TYPE CMemBlock::GetSecond(SIZE_TYPE index)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index>GetIndexCount() ||index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "超过范围 index=%d", index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.data[index].second;
}


//从0开始计数
void CMemBlock::SetKey(SIZE_TYPE index, SIZE_TYPE second, const char *key, bool isdeleteFlag)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index > GetIndexCount() ||index < 0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "超过范围, index=%d,GetIndexCount()=%d;", index,GetIndexCount());
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.data[index].second = second;
	m_pdata[m_index].indexData.data[index].isDelete = isdeleteFlag;
	//strcpy(m_pdata[m_index].indexData.data[index].szIndexValue, key);
	memcpy(m_pdata[m_index].indexData.data[index].szIndexValue, key, FILTER_VALUESIZE+1);
}

void CMemBlock::SetKey(SIZE_TYPE index, const IndexDataValue &value)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index > GetIndexCount() ||index < 0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "超出范围index=%d=,GetIndexCount()=%d;", index,GetIndexCount());
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.data[index] = value;
}

//获取删除标志
bool CMemBlock::GetDeleteFlag(SIZE_TYPE index)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index > GetIndexCount() || index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "index=%d= 超出有效范围", index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.data[index].isDelete;
}

//设置删除标志
bool CMemBlock::SetDeleteFlag(SIZE_TYPE index, bool isDelete)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index>GetIndexCount() || index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "index=%d= 超出有效范围", index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.data[index].isDelete = isDelete;
	return true;
}

//TODO:查找关键字并获取索引位置
//参数: 
//		输入:key 关键值
//		输出:index该关键值所在索引(从0开始计数)
bool CMemBlock::FindIndex(SIZE_TYPE second, const char* Key, SIZE_TYPE &index)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	SIZE_TYPE location = -1;
	if(BinarySearch(second, Key, location))
	{
		index = location;
		return true;
	}
  
	index = location;
	return false;
}

//获取最大key值
bool CMemBlock::GetMaxValue(IndexDataValue& MValue)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "尚未连接共享内存");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}

	int iMaxExsitIndex = GetIndexCount() -1;

	GetKey(iMaxExsitIndex, MValue);
	return true;
}

//插入关键字
//true 表示插入成功(即无重单)
//false表示插入失败(即有重单)
bool CMemBlock::InsertIndex(SIZE_TYPE second, const char* Key)
{
	if(IsFull())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "块内存编号=%d=已经满", m_index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	SIZE_TYPE   location;
	//查找要插入的位置
	if(BinarySearch(second, Key, location))
	{
		//该条索引已经被删除的,则设置为非删除状态(即使索引生效)
		if(GetDeleteFlag(location) == true)
		{
			SetFlag(INMEM);
			SetDeleteFlag(location, false);
			//表示插入成功
			return true;
		}
		//表示已经存在键值一样的有效索引了
		return false;
	}
	else
	{
		SetIndexCount(GetIndexCount()+1);
		
		SetFlag(INMEM);
		if(location != GetIndexCount())
		{
			IndexDataValue tempvalue;
			//顺序往后挪一位
			for(SIZE_TYPE i =GetIndexCount(); location < i; i--)
			{
				GetKey(i-1, tempvalue);
				SetKey(i, tempvalue);
			}
		}
		SetKey(location, second, Key, false);
		return true;
	}
}

//删除关键字
//返回值: 	true 索引里面有为Key的记录
//				false索引里面没有为Key的记录
bool CMemBlock::DeleteIndex(SIZE_TYPE second, const char* Key)
{
	SIZE_TYPE location;
	if(FindIndex(second, Key, location))
	{
		SetFlag(INMEM);
		SetDeleteFlag(location, true);
		return true;
	}
	return false;
}

//二分查找
int CMemBlock::BinarySearch(SIZE_TYPE second, const char* Key, SIZE_TYPE &location)
{
	SIZE_TYPE begin = 0;
	SIZE_TYPE end = GetIndexCount();
	IndexDataValue value;
	while(begin < end)
	{
		int mid = (begin+end)/2;
		GetKey(mid, value);
		if(value.second < second)
		{
			begin = mid+1;
		}
		else if(value.second > second)
		{
			end = mid;
		}
		//int iresult = strcmp(GetKey(mid), Key);
		else
		{
			//int iresult = strcmp(value.szIndexValue, Key);
			//int len= strlen(Key)>strlen(value.szIndexValue)?strlen(Key):strlen(value.szIndexValue);
			int iresult = memcmp(value.szIndexValue, Key, FILTER_VALUESIZE+1);
			if(iresult < 0)
			{
				begin = mid+1;
			}
			else if(iresult > 0)
			{
				end = mid;
			}
			else
			{
				location = mid;
				return true;
			}
		}
	}

	location = begin;
	return false;
}


