
#include "C_CMemBlock.h"

//��ȡ�����ڴ����Դ
void CMemBlock::AttachReSource(MemIndexData* memIndexdata, SIZE_TYPE index)
{
	this->m_pdata = memIndexdata;
	this->m_index = index;
}

//�ͷ���Դ
inline void CMemBlock::DetachReSource()
{
	this->m_pdata = NULL;
	this->m_index = -1;
}

//�ж��Ƿ��Ѿ�������
inline bool CMemBlock::IsAttached()
{
	return (m_pdata==NULL) ? false:true;
}

//���������
void CMemBlock::ClearData()
{
	memset((char*)&m_pdata[m_index].indexData, 0, sizeof(IndexData));
	memset(m_pdata[m_index].szTime, 0, sizeof(m_pdata[m_index].szTime));
	SetBlockNo(-1);
	SetIndexCount(0);
	SetNextBlock(-1);
	//SetFlag(INFILE);
}

//����ʱ���ݿ��ͷ�ļ���,�������ݿ��ͷ
//����:	tmpfile�洢��ʱ���ݿ��ͷ���ļ�
//			indexfile	�����ļ���(������·��)
//			blockno	�����ļ��еĿ��
//			location	indexfile.blockno����Ϣ�洢��tmpfile�е�λ��

bool CMemBlock::FindHeaderInTemp(const char* sourceId, const char *tmpfile, const char* indexfileName, const SIZE_TYPE blockno, SIZE_TYPE &location )
{
	//fstream filestream;
	char szTmpFile[FILTER_FILESIZE];
	sprintf(szTmpFile, tmpfile);
	FILE *fileid = openfile(szTmpFile, "rb+");

	location = -1;
	IndexDataHead tmpfilehead;
	bool bRetValue = false;
	//����ʱ�ļ��в���
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
	//����ʱ�ļ��в���
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


//*****************���ļ��л�ȡ����*************************
//***********���޸����ӹ�ϵ,ֻ�޸�����******************
bool CMemBlock::LoadData(const char* SourceId, const char* szPath, SIZE_TYPE blockNo, const char* szTime, const char*process_id)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	ClearData();
	
	//�������ʱ�ļ����ҵ���ֱ�Ӵ���ʱ�ļ��ж�ȡ
	char szTmpDataHeadFile[FILTER_FILESIZE];		//��ʱ���ݿ��ͷ�ļ�
	sprintf(szTmpDataHeadFile, "%s%s/%s/%s_%s", szPath, SourceId, FILTER_WORK_PATH, process_id, TMP_DATAHEAD_NAME);
	
	char szIndexFileName[FILTER_FILESIZE];	//��ʽ�����ļ���
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
	
	//���ö�ȡ�������ļ���
	//SetBlockFileName(szIndexFile);
	SIZE_TYPE location;
	if(FindHeaderInTemp(SourceId, szTmpDataHeadFile, szIndexFileName, blockNo, location))
	{
		
		char szTmpDataFile[FILTER_FILESIZE];		//��ʱ���ݿ��ļ�
		sprintf(szTmpDataFile, "%s%s/%s/%s_%s", szPath, SourceId, FILTER_WORK_PATH, process_id, TMP_DATA_NAME);
		FILE *fileid1 = openfile(szTmpDataFile, "rb+");
		fseek(fileid1, location*sizeof(IndexData), SEEK_SET);
		int get = fread(&m_pdata[m_index].indexData, sizeof(IndexData), 1, fileid1);
		if(get != 1)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "��ȡ�ļ�ʧ��=%s=", szTmpDataHeadFile);
			throw CException(FILTER_ERR_IN_READ_FILE, szMsg, __FILE__, __LINE__);
		}
		fclose(fileid1);
		ChangeFlag(INTEMP);
	}
	//�������ʽ�ļ��ж�ȡ
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

//�����ݻ�д����ʱ�ļ�
//����:	�����ʱ�ļ���·��(����'/')
//			��pathΪ��Ŀ¼
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
			sprintf(szMsg, "д�ļ�ʧ��=%s=", TmpDataHeadFileName);
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
			sprintf(szMsg, "д�ļ�ʧ��=%s=", TmpDataFileName);
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
		sprintf(szMsg, "�޷��򿪱����ļ���");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	if(INMEM == GetFlag())
	{
		backupStream.write((char*) &m_pdata[m_index].indexData, sizeof(IndexData));
		if ( !backupStream.good())
		{
		 	//TODO throw here
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "д�����ļ�ʧ��");
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
		sprintf(szMsg, "�޷��򿪱����ļ���");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	if(INMEM == GetFlag())
	{
		errno = 0;
		int iwrite = fwrite(&m_pdata[m_index].indexData, sizeof(IndexData), 1, backupStream);
		if ( iwrite != 1)
		{
		 	char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "д�����ļ�ʧ��");
			throw CException(FILTER_ERR_IN_WRITE_FILE, szMsg, __FILE__, __LINE__);
		 }
	}
}

//�޸ı�־
void CMemBlock::ChangeFlag(DataType dataType)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].type = dataType;
}

//��ȡ���ݿ��־
DataType CMemBlock::GetFlag()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
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
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].type = flag;
}


//����ǰ��ָ���
inline void CMemBlock::SetForward(SIZE_TYPE forward)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].forward = forward;
}

//��ȡǰ��ָ���
inline SIZE_TYPE CMemBlock::GetForward()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].forward;
}

//���ú���ָ���
inline void CMemBlock::SetBackward(SIZE_TYPE backward)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].backward = backward;
}

//��ȡ����ָ���
inline SIZE_TYPE CMemBlock::GetBackward()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].backward;
}

//��ȡ���
SIZE_TYPE CMemBlock::GetBlockNo()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.blockNo;
}
//���ÿ��
void CMemBlock::SetBlockNo(SIZE_TYPE blockno)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.dataHead.blockNo = blockno;
}

//��ȡ�����ļ���
char* CMemBlock::GetBlockFileName()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.szFileName;
}
//���������ļ���
inline void CMemBlock::SetBlockFileName(const char* filename)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	sprintf(m_pdata[m_index].indexData.dataHead.szFileName, filename);
}

//��ȡ���
inline char* CMemBlock::GetSourceID()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.szSourceId;
}
//���ÿ��
inline void CMemBlock::SetSourceID(const char* sourceid)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	sprintf(m_pdata[m_index].indexData.dataHead.szSourceId, sourceid);
}


//��ȡ�ļ������һ�����
SIZE_TYPE CMemBlock::GetNextBlock()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	
	return m_pdata[m_index].indexData.dataHead.nextBlock;
}
//�����ļ������һ�����
void CMemBlock::SetNextBlock(SIZE_TYPE count)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	
	m_pdata[m_index].indexData.dataHead.nextBlock = count;
}


//��ȡ����������Ŀ
SIZE_TYPE CMemBlock::GetIndexCount()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.dataHead.indexInBlock;
}

//���ÿ���������Ŀ
void CMemBlock::SetIndexCount(SIZE_TYPE count)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.dataHead.indexInBlock = count;
}


//�Ƿ�����
bool CMemBlock::IsFull()
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(GetIndexCount() >= (long)FILTER_MAXINDEXNO)
	{
		return true;
	}

	return false;
}

//��0��ʼ����
void CMemBlock::GetKey(SIZE_TYPE index, IndexDataValue& value)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index>GetIndexCount() ||index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��ȡ����ֵʧ��,index=%d ����������Χ,GetIndexCount()=%d;", index,GetIndexCount());
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	value = m_pdata[m_index].indexData.data[index];
}

//��0��ʼ����
SIZE_TYPE CMemBlock::GetSecond(SIZE_TYPE index)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index>GetIndexCount() ||index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "������Χ index=%d", index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.data[index].second;
}


//��0��ʼ����
void CMemBlock::SetKey(SIZE_TYPE index, SIZE_TYPE second, const char *key, bool isdeleteFlag)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index > GetIndexCount() ||index < 0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "������Χ, index=%d,GetIndexCount()=%d;", index,GetIndexCount());
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
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index > GetIndexCount() ||index < 0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "������Χindex=%d=,GetIndexCount()=%d;", index,GetIndexCount());
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.data[index] = value;
}

//��ȡɾ����־
bool CMemBlock::GetDeleteFlag(SIZE_TYPE index)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index > GetIndexCount() || index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "index=%d= ������Ч��Χ", index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	return m_pdata[m_index].indexData.data[index].isDelete;
}

//����ɾ����־
bool CMemBlock::SetDeleteFlag(SIZE_TYPE index, bool isDelete)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}
	if(index>GetIndexCount() || index<0)
	{
		//TODO: throw here (out of range)
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "index=%d= ������Ч��Χ", index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	m_pdata[m_index].indexData.data[index].isDelete = isDelete;
	return true;
}

//TODO:���ҹؼ��ֲ���ȡ����λ��
//����: 
//		����:key �ؼ�ֵ
//		���:index�ùؼ�ֵ��������(��0��ʼ����)
bool CMemBlock::FindIndex(SIZE_TYPE second, const char* Key, SIZE_TYPE &index)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
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

//��ȡ���keyֵ
bool CMemBlock::GetMaxValue(IndexDataValue& MValue)
{
	if(!IsAttached())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "��δ���ӹ����ڴ�");
		throw CException(FILTER_ERR_CONNECT_MEMORY, szMsg, __FILE__, __LINE__);
	}

	int iMaxExsitIndex = GetIndexCount() -1;

	GetKey(iMaxExsitIndex, MValue);
	return true;
}

//����ؼ���
//true ��ʾ����ɹ�(�����ص�)
//false��ʾ����ʧ��(�����ص�)
bool CMemBlock::InsertIndex(SIZE_TYPE second, const char* Key)
{
	if(IsFull())
	{
		//TODO: throw here
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "���ڴ���=%d=�Ѿ���", m_index);
		throw CException(FILTER_ERR_OUT_OF_RANGE, szMsg, __FILE__, __LINE__);
	}
	SIZE_TYPE   location;
	//����Ҫ�����λ��
	if(BinarySearch(second, Key, location))
	{
		//���������Ѿ���ɾ����,������Ϊ��ɾ��״̬(��ʹ������Ч)
		if(GetDeleteFlag(location) == true)
		{
			SetFlag(INMEM);
			SetDeleteFlag(location, false);
			//��ʾ����ɹ�
			return true;
		}
		//��ʾ�Ѿ����ڼ�ֵһ������Ч������
		return false;
	}
	else
	{
		SetIndexCount(GetIndexCount()+1);
		
		SetFlag(INMEM);
		if(location != GetIndexCount())
		{
			IndexDataValue tempvalue;
			//˳������Ųһλ
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

//ɾ���ؼ���
//����ֵ: 	true ����������ΪKey�ļ�¼
//				false��������û��ΪKey�ļ�¼
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

//���ֲ���
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


