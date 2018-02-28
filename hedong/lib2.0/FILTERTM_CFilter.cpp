/******************************************************************************
*FILTERTM_CFilter.cpp
*craated by tanj 2005.4.8
*description: the FILTERTM_CFilter class designed for picking the duplicated 
*             CDRs, whose pick keys contain cdr_time
* 20060209    Ϊ�����ӳ����ͨ���ԣ��������ļ���ӱ������Ƶ����棨ԭ���Ǵ�ʱ���ֶ�
*             �л�÷�����Ϣ��ת����0��3599������ֵ��
******************************************************************************/
#include "FILTERTM_CFilter.h"



char G_szPickIndexPath[FILTERTM_FILEPATH_LEN];

//added by tanj 20060213
long G_lIndexSize;
long G_lIndexInBlock;

FILTERTM_CIndexTableManager::FILTERTM_CIndexTableManager(const long lTableNum)
{
	tableNum = lTableNum;
	table.resize(tableNum);
	for (int i = 0; i < lTableNum; i++)
	{
		table[i].modified = InFile;
		table[i].backward = i + 1;  //��������
		table[i].forward = i - 1;
	}
	table[0].forward = -1;      //����ͷ�ı�־
	table[lTableNum - 1].backward = -1;    //����β�ı�־
	head = 0;
	tail = lTableNum - 1;
}



void FILTERTM_CIndexTableManager::clear()
{
	for (int i = 0; i < tableNum; i++)
	{
		table[i].modified = InFile;
		table[i].m_szSourceId[0] = NULL;
		table[i].tableData.fileName[0] = NULL;
		table[i].tableData.blockInFile = 0;
		table[i].tableData.emptyHead = -1;
		for (long j = 0; j < INDEX_TABLE_SIZE; j++)
		{
			table[i].tableData.tableItem[j] = 0;
		}
		table[i].backward = i + 1;  //��������
		table[i].forward = i - 1;
	}
	table[0].forward = -1;      //����ͷ�ı�־
	table[tableNum - 1].backward = -1;    //����β�ı�־
	head = 0;
	tail = tableNum - 1;
}

FILTERTM_CIndexTableManager::~FILTERTM_CIndexTableManager()
{ 
}

//����ʱ�ļ��в���file_Name���������������ҵ����򷵻�true��location��ʾ������������ڵڼ���λ��
//���û���ҵ����򷵻�false��location��ʾ���һ��λ���ټ�1
bool FILTERTM_CIndexTableManager::findInTemp(const char *file_Name, long &location )
{
	location = -1;
	FILTERTM_SIndexTableInFile tempTable;
	fstream tempStream;
	openTempFile(tempStream, INDEX_TABLE_TEMP);
	for ( ; ; )
	{
		tempStream.read((char *)&tempTable, sizeof(FILTERTM_SIndexTableInFile));
		location++;
		if (strcmp(tempTable.fileName, file_Name) == 0)
		{
			return true;
		}
		if (tempStream.gcount() < sizeof(FILTERTM_SIndexTableInFile))
		{
			break;
		}
	}
	tempStream.close();
	return false;
}

	


/*******************************************************************************************************
*������  char *file_Name [in]  ��file_Name�ļ������������load��table�����У���ʹheadָ������table�е����
*������  ���Ȳ鿴����fileName�ļ�������������ǲ��ǵ�ǰ�ģ�headָ��ģ�����������������ֱ�ӷ��أ�
         ���������鿴������������ǲ����Ѿ���table�������棬�������������֯����ʹheadָ��ÿ飻
         ����������Ƚ����δʹ�õ����������tailָ��ģ��û�����ʱ�ļ��У�Ȼ�󽫲���fileName�ļ���
         ������������table�����tail���У��Ȳ鿴�Ƿ�����ʱ�ļ��У�����������ʱ�ļ��ж�ȡ��������
         ������֯����ʹԭ����tail��Ϊhead��������ļ������ڣ�������ͬ���Ŀ��ļ�
*����ֵ����
*
**********************************************************************/
void FILTERTM_CIndexTableManager::loadIndexTable(const char *szSourceId,const char *file_Name)
{
	for (int current = head; current != -1; current = table[current].backward)
	{
		 if (strcmp(table[current].tableData.fileName, file_Name) == 0 && strcmp(table[current].m_szSourceId,szSourceId) == 0)  //���ڴ���
		 {
		 	 if (table[current].forward >= 0)                   //�Ҳ���ͷ
		 	 {
		 	 	 table[table[current].forward].backward = table[current].backward;
		 	 	 
		 	   if (table[current].backward >= 0)                 //�Ҳ���β
		 	   {
		 	 	   table[table[current].backward].forward = table[current].forward;
		 	   }
		 	   else                                             //��β
		 	   {
		 	   	 tail = table[current].forward;
		 	   }
		 	   table[current].forward = -1;                    //��current�ó�ͷ
		 	   table[current].backward = head;
		 	   table[head].forward = current;
		 	   head = current;
		 	 }
		 	 return;
		 }
	}

	//��tailָ������������д�ص���ʱ�ļ���
	if (InMemory == table[tail].modified)
	{
		long tableLoc;
		findInTemp(table[tail].tableData.fileName, tableLoc);

   	fstream fileStream;
   	openTempFile(fileStream, INDEX_TABLE_TEMP);
   	fileStream.seekp(tableLoc*sizeof(FILTERTM_SIndexTableInFile), ios::beg);
   	fileStream.write((char *)&(table[tail].tableData), sizeof(FILTERTM_SIndexTableInFile));
//*************added by tanj 2004.12.2   ����ļ�����д��״̬   	
		if ( !(fileStream.good()) )
		{
		  char msg[256];
		  sprintf(msg, "write temp file fail!:FILTERTM_CIndexTableManager::loadIndexTable(char *) at %s file ", table[tail].tableData.fileName);
      throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		}
//*************end of addition
		fileStream.close();

		table[tail].modified = InTemp;
	}
	
	//����Ҫ��ȡ������������Ƿ�����ʱ�ļ��У�����ڣ������ʱ�ļ��ж�ȡ���������
	//������ڣ���������ļ��ж�ȡ���������
	long loc;
	if (findInTemp(file_Name, loc) == true )
	{
		fstream fileStream;
		openTempFile(fileStream,INDEX_TABLE_TEMP);
		fileStream.seekg(loc*sizeof(FILTERTM_SIndexTableInFile), ios::beg);
		fileStream.read((char *)(&(table[tail].tableData)), sizeof(FILTERTM_SIndexTableInFile));
		fileStream.close();
		table[tail].modified = InTemp;
	}
	else
	{
		fstream fileStream;
		openIndexFile(fileStream, file_Name);
		fileStream.read((char *)(&(table[tail].tableData)), sizeof(FILTERTM_SIndexTableInFile));
		table[tail].modified = InFile;
		//added by tanj 20060213   delete by tanj 20060307
		//if (fileStream.gcount() == sizeof(FILTERTM_SIndexTableInFile))
		//{
		//	if (table[tail].tableData.indexSize != G_lIndexSize)
		//	{
		//		char msg[256];
		//		sprintf(msg, "the indexsize(%d) in %s not match the init indexsize(%d) ",table[tail].tableData.indexSize,file_Name,G_lIndexSize);
  	//	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		//		return ;
		//	}
		//	if (table[tail].tableData.indexInBlock != G_lIndexInBlock)
		//	{
		//		char msg[256];
		//		sprintf(msg, "the indexinblock(%d) in %s not match the init indexinblock(%d) ",table[tail].tableData.indexInBlock,file_Name,G_lIndexInBlock);
  	//	  throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		//		return ;
		//	}
		//}
		//else
		//{
		//	table[tail].tableData.indexSize = G_lIndexSize;
		//	table[tail].tableData.indexInBlock = G_lIndexInBlock;
		//}
	}

	//�򿪵���������ļ�������ļ�����Ҫ���
	if (strcmp(table[tail].tableData.fileName, file_Name) != 0)
	{
//modified by tanj 2004.12.30
		//fstream fileStream;
		//char tempFullFileName[FILEPATH_LEN+FILENAME_LEN];
	  //memset(tempFullFileName, 0, sizeof(tempFullFileName));
	  //strcpy(tempFullFileName, G_szPickIndexPath);
	  //strcat(tempFullFileName, file_Name);
		//fileStream.open(tempFullFileName, ios::trunc);  //����ļ�
		truncIndexFile(file_Name);
//end of modification
		strcpy(table[tail].m_szSourceId,szSourceId);
		strcpy(table[tail].tableData.fileName, file_Name);
		table[tail].tableData.blockInFile = 1; 
		table[tail].tableData.emptyHead = -1;
		for (int i = 0; i < INDEX_TABLE_SIZE; i++)
		{
			table[tail].tableData.tableItem[i] = 0;
		}
		table[tail].modified = InMemory;

	}
	//added by tanj 20060307
	table[tail].tableData.indexSize = G_lIndexSize;
	table[tail].tableData.indexInBlock = G_lIndexInBlock;
	//added by tanj 20051114
	strcpy(table[tail].m_szSourceId, szSourceId);
	
	
	//���°�������
	long temp;
	temp = tail;
	tail = table[tail].forward;
	table[tail].backward = -1;
	table[head].forward = temp;
	table[temp].backward = head;
	head = temp;
	table[head].forward = -1;

}


long FILTERTM_CIndexTableManager::getTableItem(long sec)
{
	return table[head].tableData.tableItem[sec];
}

void FILTERTM_CIndexTableManager::setTableItem(long sec, long block_No)
{
	table[head].tableData.tableItem[sec] = block_No;
	modifyTable(InMemory);
}

long FILTERTM_CIndexTableManager::getBlockInFile()
{
	return table[head].tableData.blockInFile;
}

void FILTERTM_CIndexTableManager::setBlockInFile(long blockInFile)
{
	if (blockInFile < 0)
	{
		char msg[256];
		sprintf(msg, "blockInFile must > 0 at %s file ", table[head].tableData.fileName);
    throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		return ;
	}
	table[head].tableData.blockInFile = blockInFile;
	modifyTable(InMemory);
	return;
}

//add by tanj 2004.11.29 for remove index from index file
long FILTERTM_CIndexTableManager::getEmptyHead()
{
	return table[head].tableData.emptyHead;
}
void FILTERTM_CIndexTableManager::setEmptyHead(long emptyHead)
{
	if (emptyHead < -1)
	{
		char msg[256];
		sprintf(msg, "emptyHead must > 0 at %s file ", table[head].tableData.fileName);
    throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		return ;
	}
	table[head].tableData.emptyHead = emptyHead;
	modifyTable(InMemory);
	return;
}
//end of addition

void FILTERTM_CIndexTableManager::backup()
{
	fstream fileStream;
	openTempFile(fileStream, INDEX_TABLE_BAK);
	for (long current = head; current != -1; current = table[current].backward)
	{
		if (table[current].modified != InFile)
		{
			fileStream.write((char *)(&(table[current].tableData)), sizeof(FILTERTM_SIndexTableInFile));
//*************added by tanj 2004.12.2   ����ļ�����д��״̬   	
		  if ( !(fileStream.good()) )
		  {
		    char msg[256];
		    sprintf(msg, "write backup file fail!:FILTERTM_CIndexTableManager::backup() at %s file ", table[current].tableData.fileName);
        throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		  }
//*************end of addition			
		}
	}
	fileStream.close();
}
//**********************************************************
//this function is for restore the data using the bak data
//**********************************************************
void FILTERTM_CIndexTableManager::restore(const char *szSourceId)
{
	fstream fileStream;
	openTempFile(fileStream, INDEX_TABLE_BAK);
	
  for (long current = head; current != -1; current = table[current].backward)
  {
  	fileStream.read((char *)(&(table[current].tableData)), sizeof(FILTERTM_SIndexTableInFile));
  	if (fileStream.gcount() < sizeof(FILTERTM_SIndexTableInFile))
  	{
  		break;
  	}
  	strcpy(table[current].m_szSourceId, szSourceId);
  	table[current].modified = InMemory;
  }
  fileStream.close();
}


void FILTERTM_CIndexTableManager::commit()
{
	//���ڴ��е����������д�������ļ�
	for (long current = head; current != -1; current = table[current].backward)
	{
		fstream fileStream;
		if (table[current].modified != InFile)
		{
			//strncpy(G_Index, table[current].tableData.fileName, G_Dir_Sep);
			openIndexFile(fileStream, table[current].tableData.fileName);
			fileStream.write((char *)(&(table[current].tableData)), sizeof(FILTERTM_SIndexTableInFile));
//*************added by tanj 2004.12.2   ����ļ�����д��״̬   	
		  if ( !(fileStream.good()) )
		  {
		    char msg[256];
		    sprintf(msg, "commit fail! Cannot write index file:FILTERTM_CIndexTableManager::commit() at %s file ", table[current].tableData.fileName);
        throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		  }
//*************end of addition		
			fileStream.close();
			table[current].modified = InFile;
		}
	}
	//����ʱ�ļ��в����ڴ��е����������д�������ļ���
	FILTERTM_SIndexTableInFile tempTable;
	fstream tempStream;
	openTempFile(tempStream, INDEX_TABLE_TEMP);
	long current;
	for (; ;)
	{
		tempStream.read((char *)&tempTable, sizeof(FILTERTM_SIndexTableInFile));
		if (tempStream.gcount() < sizeof(FILTERTM_SIndexTableInFile))
		{
			break;
		}
		for (current = head; current != -1; current = table[current].backward)
		{
			if (strcmp(tempTable.fileName, table[current].tableData.fileName) == 0)
			{
				break;
			}
		}
		if (current == -1)
		{
			fstream fileStream;
			//strncpy(G_Index, tempTable.fileName, G_Dir_Sep);
			openIndexFile(fileStream, tempTable.fileName);
			fileStream.write((char *)&tempTable, sizeof(FILTERTM_SIndexTableInFile));
//*************added by tanj 2004.12.2   ����ļ�����д��״̬   	
		  if ( !(fileStream.good()) )
		  {
		    char msg[256];
		    sprintf(msg, "commit fail! Cannot write index file:FILTERTM_CIndexTableManager::commit() at %s file ", tempTable.fileName);
        throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		  }
//*************end of addition		
			fileStream.close();
		}
	}
}


//*********************************************************************
//*�����ڴ��е����ݿ�����ʣ�InFile��or InTemp or InMemory
//**********************************************************************
void FILTERTM_CIndexTableManager::modifyTable(Modify modify)
{
	table[head].modified = modify;
}






/**/
//for debug
void FILTERTM_CIndexTableManager::disChain()
{
	cout<<"head="<<head<<"	"<<"tail="<<tail<<endl;
	for (int current = 0; current < tableNum; current++)
	{
		cout<<current<<"	"<<table[current].forward<<"	"<<table[current].backward<<"	"<<table[current].modified<<"	"<<table[current].tableData.fileName<<endl;
	}
}

void FILTERTM_CIndexTableManager::disTable()
{
	cout<<endl;

	long blockno = 0;
	long begin, end;
	begin = 0;
	for (int i = 0; i <INDEX_TABLE_SIZE; i++)
	{
		if (table[head].tableData.tableItem[i] == blockno)
		{
			continue;
		}
		end = i - 1;
		cout<<"from "<<begin<<" to "<<end<<" : "<<blockno<<endl;
		blockno = table[head].tableData.tableItem[i];
		begin = i;
	}
	end = INDEX_TABLE_SIZE - 1;
	cout<<"from "<<begin<<" to "<<end<<" : "<<blockno<<endl;

		
	/*
	for (int i = 0; i < 0x10000; i++)
	{
		cout<<i<<":"<<table[head].tableData.tableItem[i]<<" ";
	}
	*/
	cout<<endl;
}

//***********************BLOCK***************************************

FILTERTM_CBlock::FILTERTM_CBlock()
{
	blockData = NULL;
}

FILTERTM_CBlock::~FILTERTM_CBlock()
{
	if (blockData != 0)
	{
		free(blockData);
	  blockData = NULL;
	}

}

/**************************************************************************************************
*��������void FILTERTM_CBlock::init(long max_Index,long index_Size)
*������  long max_Index [in] ���п��Դ�ŵ����ȥ��������
*        long index_Size [in]  ȥ�������ĳ���
*������  ��ʼ��
*����ֵ�� ��
***************************************************************************************************/
void FILTERTM_CBlock::init(long max_Index, long index_Size)
{
	blockData = 0;
	indexSize = index_Size;
	maxIndex = max_Index;
	
	blockSize = sizeof(FILTERTM_SBlockHead) + indexSize*maxIndex;

	setNextBlock(-1);
	setIndexInBlock(0);
	
	blockData = (char *)malloc(maxIndex*indexSize);
	if (NULL == blockData)
	{
		throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, FILTERTM_NOT_ENOUGH_MEMORY, errno, (char *)"Out Of Memory!", __FILE__, __LINE__);
	}
	
	fileName[0] = 0;
	blockNo = -1;   //��û�д�����ݣ�����blockNo����-1
	modified = InFile;
	forward = backward = -1;
}


void FILTERTM_CBlock::clear()
{
	setNextBlock(-1);
	setIndexInBlock(0);
	
	fileName[0] = 0;
	blockNo = -1;   //��û�д�����ݣ�����blockNo����-1
	modified = InFile;
	forward = backward = -1;
}
const char *FILTERTM_CBlock::getFileName()
{
	return fileName;
}

const char *FILTERTM_CBlock::getSourceId()
{
	return m_szSourceId;
}

void FILTERTM_CBlock::modifyBlock(Modify modify)
{
	modified = modify;
}

bool FILTERTM_CBlock::isFull()
{
	
	if (getIndexInBlock() >= maxIndex)
	{
   	return true;
	}
	else
	{
		return false;
	}
	
	
}

long FILTERTM_CBlock::getBlockNo()
{
	return blockNo;
}


//����ʱ�ļ��в���file_Name�ĵ�block_No�飬����ҵ����򷵻�true��location��ʾ�ÿ��ڵڼ���λ��
//���û���ҵ����򷵻�false��location��ʾ���һ��λ���ټ�1
bool FILTERTM_CBlock::findInTemp(const char *file_Name, const long block_No, long &location )
{
	location = -1;
	FILTERTM_SBlockIndex tempIndex;
	fstream tempStream;
	openTempFile(tempStream, BLOCK_INDEX_TEMP);

	for ( ; ; )
	{
		tempStream.read((char *)&tempIndex, sizeof(FILTERTM_SBlockIndex));
		location++;
		if ((strcmp(tempIndex.fileName, file_Name) == 0) && (tempIndex.blockNo == block_No))
		{
			return true;
		}
		if (tempStream.gcount() < sizeof(FILTERTM_SBlockIndex))
		{
			break;
		}
	}
	tempStream.close();
	return false;
}

/************************************************************************************************
*��������void FILTERTM_CBlock::rollBack()
*������  ��
*������  ���modified == InMemory�������е����ݻ�д����ʱ�ļ���
*�����ã���
*���ߣ�  ̷��  2004.9.10
************************************************************************************************/
void FILTERTM_CBlock::write2Temp()
{
	if (InMemory == modified) 
	{
		long blockLoc;
		FILTERTM_SBlockIndex tempIndex;
		strcpy(tempIndex.fileName, fileName);
		tempIndex.blockNo = blockNo;
		findInTemp(fileName, blockNo, blockLoc) ;

		
		fstream indexStream;
		fstream dataStream;
		openTempFile(indexStream, BLOCK_INDEX_TEMP);
		openTempFile(dataStream, BLOCK_DATA_TEMP);
		indexStream.seekp(blockLoc*sizeof(FILTERTM_SBlockIndex), ios::beg);
		dataStream.seekp(blockLoc*blockSize, ios::beg);
		indexStream.write((char *)&tempIndex, sizeof(FILTERTM_SBlockIndex));
		dataStream.write((char *)&blockHead, sizeof(FILTERTM_SBlockHead));
		dataStream.write((char *)blockData, indexSize*maxIndex);
//*************added by tanj 2004.12.2 ����ļ�����д��״̬
		if ( !(indexStream.good()) ||!(dataStream.good()) )
		{
		  char msg[256];
		  sprintf(msg, "write temp file fail!:FILTERTM_CBlock::write2Temp() at %s file %d block", fileName, blockNo);
      throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		}
//**************end of addition
	  indexStream.close();
	  dataStream.close();
	  
	  modifyBlock(InTemp);
	}
	return;
}

/*************************************************************************************************
*��������void FILTERTM_CBlock::loadBlock(char *file_Name,long block_No)
*������  char *file_Name [in] ��Ҫװ����ļ����ļ���
*        long block_No [in]   ��Ҫװ��Ŀ��
*������  ��file_Name�ļ��ĵ�Block_No�����ݶ���������
*����ֵ����
*************************************************************************************************/
void FILTERTM_CBlock::loadBlock(const char *szSourceId, const char *file_Name, const long block_No)
{
	//������տ��е�ԭ������
	blockHead.nextBlock = -1;
	blockHead.indexInBlock = 0;
	memset(blockData, 0, maxIndex*indexSize);
	//����Ҫ��ȡ�Ŀ��Ƿ�����ʱ�ļ��У�����ڣ������ʱ�ļ��ж�ȡ�ÿ飻
	//������ڣ���������ļ��ж�ȡ��
	long loc;
	if (findInTemp(file_Name, block_No, loc) == true )
	{
		fstream fileStream;
		openTempFile(fileStream,BLOCK_DATA_TEMP);
		fileStream.seekg(loc*blockSize, ios::beg);
		fileStream.read((char *)(&blockHead), sizeof(FILTERTM_SBlockHead));
		fileStream.read((char *)blockData, indexSize*maxIndex);
		fileStream.close();
		if (blockHead.indexInBlock == 0)
		{
			//cout<<file_Name<<" "<<block_No<<" block indexInBlock = 0 "<<endl;
		}
		modifyBlock(InTemp);
	}
	else
	{
		fstream fileStream;
		openIndexFile(fileStream, file_Name);
		fileStream.seekg(sizeof(FILTERTM_SIndexTableInFile) + (block_No )*blockSize,ios::beg);
		fileStream.read((char *)(&blockHead), sizeof(FILTERTM_SBlockHead));
		if (fileStream.gcount() < sizeof(FILTERTM_SBlockHead))
		{
			setNextBlock(-1);
		}
		else
		{
			fileStream.read((char *)blockData, indexSize*maxIndex);
		}
	
		modifyBlock(InFile);
	}
	strcpy(m_szSourceId, szSourceId);         //added by tanj 20051114
	strcpy(fileName, file_Name);
	blockNo = block_No;
	
	return;
}
bool FILTERTM_CBlock::loadBak(const char *szSourceId, fstream &fileStream)
{
	//������տ��е�ԭ������
	blockHead.nextBlock = -1;
	blockHead.indexInBlock = 0;
	memset(blockData, 0, maxIndex*indexSize);
	
	FILTERTM_SBlockIndex blockIndex;
	fileStream.read((char *)&blockIndex, sizeof(FILTERTM_SBlockIndex));
	if (fileStream.gcount() < sizeof(FILTERTM_SBlockIndex))
	{
		return false;
	}
	strcpy(fileName, blockIndex.fileName);
	blockNo = blockIndex.blockNo;
	
	fileStream.read((char *)&blockHead, sizeof(FILTERTM_SBlockHead));
	fileStream.read((char *)blockData, indexSize*maxIndex);
	strcpy(m_szSourceId, szSourceId);
	return true;
}

//��ÿ��е�index_No������0��ʼ�ƣ�������ָ��
char * FILTERTM_CBlock::getIndex(long index_No)
{
  if ((index_No > getIndexInBlock())||(index_No < 0))
  {
  	char msg[256];
  	sprintf(msg, "illegal index_No(%d):FILTERTM_CBlock::getIndex(long) at %s file %d block", index_No, fileName, blockNo);
  	throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, (char *)__FILE__, __LINE__);
		return NULL;
  }
	
	return blockData + indexSize*index_No;
}

//��index����е�index_No����������0��ʼ�ƣ���Ƚ�
int FILTERTM_CBlock::indexCmp(char *index, long index_No)
{
	
	if ((index_No > getIndexInBlock() - 1)||(index_No < 0))
	{
		char msg[256];
		sprintf(msg, "illegal indexNo!(%d):FILTERTM_CBlock::indexCmp(char *, long) at %s file %d block", index_No, fileName, blockNo);
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, (char *)__FILE__, __LINE__);
		return -2;
	}
	
	//deleted by tanj 20061020 index��ʼ�ļ����ֽ���unsigned long�͵Ķ�����������big-endian��CPU�Ͽ�����memcmp���бȽ�
	//������little-endian��CPU����intel�������޷�ʹ��memcmp���бȽ�
	//return memcmp(index, getIndex(index_No), indexSize - 1);  //do not cmp the delete flag	
	
	//added by tanj 20061020 ʹ���Զ���ıȽϺ���������memcmp��������index���бȽϣ�����little-end��CPU�������
	return CompareIndex(index, getIndex(index_No));
	
}
//added by tanj 20061020 ���Զ���ıȽϺ�������memcmp����������big endian ��little endian
int FILTERTM_CBlock::CompareIndex(const char *index1,const char *index2)
{
	//deleted by tanj 20061101 *(unsigned long *)(char *) ת����unsigned long ��64λ
	//�Ļ���������ܻ������⣬��charָ���ַ���ǰ�sizeof(unsigned long)����ʱ��cpu�ᱨ��
	//unsigned long lIndexValue1 = *((unsigned long *)index1);
	//unsigned long lIndexValue2 = *((unsigned long *)index2);
	//end of delete by tanj 20061101
	
	//added by tanj 20061101 ʹ��memcpy�����ַ�����unsigned long��ת���������ַ���ַ�����������
	unsigned long lIndexValue1, lIndexValue2;	
	memcpy(&lIndexValue1, index1, sizeof(unsigned long));
	memcpy(&lIndexValue2, index2, sizeof(unsigned long));
	//end of addition by tanj 20061101
	
	if (lIndexValue1 < lIndexValue2)
	{
		return -1;
	}
	else if (lIndexValue1 > lIndexValue2)
	{
		return 1;
	}
	return memcmp(index1+sizeof(unsigned long),index2+sizeof(unsigned long), indexSize - sizeof(bool) -sizeof(unsigned long));
}


//��index�ڿ������һ��������Ƚ�
int FILTERTM_CBlock::lastIndexCmp(char *index)
{
	return indexCmp(index, getIndexInBlock() - 1);
}

//��index���뵽��location������0��ʼ�ƣ�������λ�ã�
//�Ƚ���������˳�κ�Ų���ٲ���
void FILTERTM_CBlock::insertIndex(char *index, long location)
{

	if (isFull())
	{
		char msg[256];
		sprintf(msg, "the block is full! Cannot insert anymore!:FILTERTM_CBlock::insertIndex(char *,long) at %s file %d block", fileName, blockNo);
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		return ;
	}
	
	if ((location > getIndexInBlock())||(location < 0))
	{
		char msg[256];
		sprintf(msg, "illegal location!(%d):FILTERTM_CBlock::insertIndex(char *, long) at %s file %d block", location, fileName, blockNo);
    throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		return ;
	}
	
	if (location != getIndexInBlock())
	{
	  for (int count = getIndexInBlock(); count > location; count--)
	  {
		  memcpy(getIndex(count), getIndex(count - 1), indexSize);
	  }
	}
	
	memcpy(getIndex(location), index, indexSize);
	setIndexInBlock(getIndexInBlock() + 1);

	modifyBlock(InMemory);
	return;	
}

//added by tanj 2004.11.29  for remove  index from index file
void FILTERTM_CBlock::deleteIndex(long location)
{
	if ( (getIndexInBlock() <= 0) || (getIndexInBlock() <= location) )
	{
		char msg[256];
		sprintf(msg, "illegal indexInBlock!(%d):FILTERTM_CBlock::deleteIndex(long) at %s file %d block", getIndexInBlock(), fileName ,blockNo);
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		return ;
	}
	for (int indexNo = location; indexNo < getIndexInBlock() - 1; indexNo++)
	{
		memcpy(getIndex(indexNo), getIndex(indexNo +1), indexSize);
	}
	//added by tanj 2005.1.10  
	memset(getIndex(getIndexInBlock() - 1), 0, indexSize);
	//end of addition
	
	setIndexInBlock(getIndexInBlock() - 1);
	
	modifyBlock(InMemory);
	return;
}
//end  add


//�Կ��е��������ж��ֲ��ң�����ҵ�����true
//����Ҳ���������false����ͨ��location����indexӦ���ڵ�λ��
//this function do not care for delete flag
bool FILTERTM_CBlock::binarySearch(char *index, long &location)
{
	int cmpResult;
	long begin;
	long end;
	begin = 0;
	end = getIndexInBlock();
	for (; ;)
	{
		if (begin == end)
		{
			break;
		}
		long count = (begin + end)/2;
		

			cmpResult = indexCmp(index,count);

		
		if (cmpResult < 0)
		{
			end = count;
		}
		if (cmpResult > 0)
		{
			begin = count +1;
		}
		
		if (cmpResult == 0)
		{
			 location = count;
			 return true;
		}
	}
	location = begin;
	
	return false;
}

//return the delete flag
bool FILTERTM_CBlock::getDeleteFlag(const long _lLocation)
{
	return *(bool *)(getIndex(_lLocation) + indexSize - 1);  
}
//set the delete flag
void FILTERTM_CBlock::setDeleteFlag(const bool _bDeleteFlag, const long _lLocation)
{
	char *p = getIndex(_lLocation);
	p += indexSize -1;
	*(bool *)p = _bDeleteFlag;
	modifyBlock(InMemory);    //almost forget it ! ^_^
}

long FILTERTM_CBlock::getNextBlock()
{
	return blockHead.nextBlock;
}

long FILTERTM_CBlock::getIndexInBlock()
{
	return blockHead.indexInBlock;
}

void FILTERTM_CBlock::setNextBlock(long next_Block)
{
	if (next_Block < -1)
	{
		char msg[256];
		sprintf(msg, "illegal next_Block!(%d):FILTERTM_CBlock::setNextBlock(long) at %s file %d block", next_Block, fileName ,blockNo);
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		return ;
	}
	blockHead.nextBlock = next_Block;
	//added by tanj 2004.11.29
	modifyBlock(InMemory);
	//end of addition
}

void FILTERTM_CBlock::setIndexInBlock(long index_In_Block)
{
	if (index_In_Block > maxIndex)
	{
		char msg[256];
		sprintf(msg, "Too many indice(%d)in the block:FILTERTM_CBlock::setIndexInBlock(long) at %s file %d block", index_In_Block, fileName ,blockNo);
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, __FILE__, __LINE__);
		return ;
	}
	blockHead.indexInBlock = index_In_Block;
	//added by tanj 2004.11.29
	modifyBlock(InMemory);
	//end of addition;
}



FILTERTM_SBlockHead * FILTERTM_CBlock::getBlockHead()
{
	return &blockHead;
}

char * FILTERTM_CBlock::getBlockData()
{
	return blockData;
}



/**/
//for debug
void FILTERTM_CBlock::disBlock()
{
	cout<<"FileName = "<<fileName<<endl;
	cout<<"blockNo = "<<blockNo<<endl;
	cout<<"modified = "<<modified<<endl;
	cout<<"nextBlock = "<<getNextBlock()<<endl;
	cout<<"indexInBlock = "<<getIndexInBlock()<<endl;
	cout<<"maxIndex = "<<maxIndex<<endl;
	cout<<"indexSize = "<<indexSize<<endl;
	cout<<"forward = "<<forward<<endl;
	cout<<"backward = "<<backward<<endl;
	
	for (int i = 0; i < getIndexInBlock(); i++)
	{
		disIndex(i);
	}
	cout<<endl;
}

//for debug
void FILTERTM_CBlock::disIndex(long index_No)
{
	long second;
	unsigned short temp;
	char *indexp = getIndex(index_No);
	//memcpy(&second, getIndex(index_No),sizeof(long));
	//cout<<second<<"	";
	
	printf("%04d", *(long *)indexp );
	indexp += sizeof(long);
	for (int i = 0; i < indexSize - sizeof(long) - 1; i++, indexp++)
	{
		printf("%c",*indexp); 
  	fflush(stdout);		
	}
	printf("%d", *indexp);	
  fflush(stdout);
  cout<<endl;	
}




























//***********************************FILTER**********************************



































/******************************************************************************************
*���캯��
*������long block_Num [in]  ����FILTERTM_CBlock����ĸ�������Щ���󹹳�һ��vector
*      long index_Size [in] ȥ�عؼ��ֵĳ��ȣ������������ͱ�����
*      long max_Index_In_Block [in]  ÿ��FILTERTM_CBlock�����������е�ȥ�عؼ��ֵ������Ŀ
*�������������������ʼ�����󣬲�����blockList�е�����
*      ע�⣺��ʼ��indexSizeʱ����Ҫ������index_Size + sizeof(long) + 1;
*            ��ʼ��blockSizeʱ��blockSize = sizeof(FILTERTM_SBlockHead) + indexSize*maxIndexInBlock;
*            ����ͷ��forward = -1, ����β��backward = -1;
*���أ���
*���ߣ�̷��    2004.9.10
*******************************************************************************************/
FILTERTM_CFilter::FILTERTM_CFilter(long index_Table_Num, long block_Num,
                                               long index_Total_Size, long max_Index_In_Block
                                              ):indexTableManager(index_Table_Num)
     
{
	
	blockNum = block_Num;
	indexTableNum = index_Table_Num;
  indexSize = index_Total_Size + sizeof(long) + sizeof(bool);
  
  //added by tanj 20060213
  G_lIndexSize = indexSize;
  G_lIndexInBlock = max_Index_In_Block;
	
	maxIndexInBlock = max_Index_In_Block;

	blockSize = sizeof(FILTERTM_SBlockHead) + indexSize*maxIndexInBlock;

	blockList.resize(blockNum);	
	for (int current = 0; current < blockNum; current++)
	{
		blockList[current].init(maxIndexInBlock, indexSize);
    blockList[current].backward = current + 1;
    blockList[current].forward = current - 1;
	}
	blockList[0].forward = -1;
	blockList[blockNum-1].backward = -1;
	head = blockNum - blockNum;        //���д��"head = 0;" ��"core dump"
	tail = blockNum - 1;
  //cout<<"constructor "<<endl;
}

void FILTERTM_CFilter::clear()
{
	indexTableManager.clear();
	for (int current = 0; current < blockNum; current++)
	{
		blockList[current].clear();
    blockList[current].backward = current + 1;
    blockList[current].forward = current - 1;
	}	
	blockList[0].forward = -1;
	blockList[blockNum-1].backward = -1;
	head = blockNum - blockNum;        //���д��"head = 0;" ��"core dump"
	tail = blockNum - 1;
	truncFile(INDEX_TABLE_TEMP);
	truncFile(BLOCK_INDEX_TEMP);
	truncFile(BLOCK_DATA_TEMP);
	
	truncFile(BLOCK_DATA_BAK);
	truncFile(INDEX_TABLE_BAK);
}
//******************************************************************************
//����ȥ�������ļ���Ŀ¼
//******************************************************************************
void FILTERTM_CFilter::setSource(const char *szSourceId, const char *index_Path)
{
  strcpy(m_szSourceId, szSourceId);
	memset(G_szPickIndexPath, 0, sizeof(G_szPickIndexPath));
	strcpy(G_szPickIndexPath, index_Path);
	if (G_szPickIndexPath[strlen(G_szPickIndexPath) - 1] != '/')
	{
		strcat(G_szPickIndexPath, "/");
	}
  //m_AllSource[string(szSourcdId)] = string(G_szPickIndexPath); 
}

FILTERTM_CFilter::~FILTERTM_CFilter()
{


}


/******************************************************************************************
*��������void FILTERTM_CFilter::moveIndex(long from, long to)
*������long from [in]  ��Ҫ�Ƴ����ݵ�FITLER_CBlock������blockList�е����
*      long to   [in]  ��Ҫ�������ݵ�FITLER_CBlock������blockList�е����
*��������blockList�еĵ�from����1/5������ת�Ƶ���to����,����������ʱ������
*      �ڵ��ô˺���֮ǰ��blockList[from]��nextBlockӦָΪblockList[to]���ļ��еĿ��
*���أ���
*���ߣ�̷��   2004.9.10
******************************************************************************************/
void FILTERTM_CFilter::moveIndex(long from, long to)
{
	blockList[to].setIndexInBlock(blockList[from].getIndexInBlock()/5);
	long toCount = blockList[to].getIndexInBlock() - 1;
	long fromCount = blockList[from].getIndexInBlock() - 1;

	for (; toCount >= 0; toCount--, fromCount--)
	{
		memcpy(blockList[to].getIndex(toCount), blockList[from].getIndex(fromCount), indexSize);
		memset(blockList[from].getIndex(fromCount), 0, indexSize);
	} 

	blockList[from].setIndexInBlock(blockList[from].getIndexInBlock() - blockList[to].getIndexInBlock());

	long begin,end;   //begin,endΪʱ����������Ҫ�޸ĵķ�Χ���׺�β both value should be less than 3600
	memcpy(&begin, blockList[from].getIndex(blockList[from].getIndexInBlock() - 1), sizeof(long));
	begin++;
	
	memcpy(&end, blockList[to].getIndex(blockList[to].getIndexInBlock() - 1), sizeof(long));

	if (blockList[to].getNextBlock() == -1)
	{
	   end = INDEX_TABLE_SIZE - 1;
	}

	for (int count = begin; count <= end; count++)
	{
	   indexTableManager.setTableItem(count, blockList[from].getNextBlock());
	}
	
	indexTableManager.modifyTable(InMemory);
	blockList[from].modifyBlock(InMemory);
	blockList[to].modifyBlock(InMemory);
}

/*****************************************************************************************
*��������long FILTERTM_CFilter::readBlock(char *file_Name,long block_No)
*������  char *file_Name [in]  ��Ҫ����Ŀ����ڵ������ļ����ļ���
*        long block_No   [in]  ��Ҫ����Ŀ��������ļ��еĿ��
*������  ��������ȡfileName������������ļ��еĵ�blockNo������������飨��0��ʼ����blockList
         �е�ĳһ��block�С����Ҫ��ȡ�Ŀ�ԭ�Ⱦ���blockList�У���ֱ�ӷ��ظÿ���blockList�е�
         ��ţ����Ҫ��ȡ�Ŀ鲻��blockList�У���blockList�����δʹ�ÿ飨˫�������β�飩
         д�ص���ʱ�ļ��У�������ʱ�ļ��в��ҽ�Ҫ��ȡ�Ŀ飬����ҵ������˫�������β�飬
         ����β��Ϊͷ�飬���û���ҵ��������ļ��ж�ȡҪ��ȡ�Ŀ鵽˫�������β�顣
         ���������֯����ʱԭ�ȵ�β���Ϊͷ��
*���أ�  ���ض���Ŀ���blockList�е����
*���ߣ�̷�� 2004.9.10
*********************************************************************************************/
long FILTERTM_CFilter::readBlock(const char *szSourceId, const char *file_Name,const long block_No)
{
	
	//���������˳�����ң����Ҫ����Ŀ�ԭ������blockList�У���������������λ��
	for (int current = head; current >= 0;current = blockList[current].backward)
	{ 
		if ((strcmp(blockList[current].getFileName(),file_Name) == 0)&&(blockList[current].getBlockNo() == block_No)&&(strcmp(blockList[current].getSourceId(),szSourceId)==0))
		{
			if (blockList[current].forward >= 0)
			{
				blockList[blockList[current].forward].backward = blockList[current].backward; 
				if (blockList[current].backward >= 0)
				{
					blockList[blockList[current].backward].forward = blockList[current].forward;
				}
				else
				{
					tail = blockList[current].forward;
				}
				blockList[current].forward = -1;
				blockList[current].backward = head;
				blockList[head].forward = current;
				head = current;
			}
			return head;         //headָ��ľ����ҵ��Ŀ�
		}
	}
	//��β���е����ݻ��˵������ļ���	
	blockList[tail].write2Temp();
	blockList[tail].loadBlock(szSourceId, file_Name,block_No);
	//���°�������
	long temp;
	temp = tail;
	tail = blockList[tail].forward;
	blockList[tail].backward = -1;
	blockList[head].forward = temp;
	blockList[temp].backward = head;
	head = temp;
	blockList[head].forward = -1;
	return head;
}

/*****************************************************************************************************
*��������bool FILTERTM_CFilter::findIndex(const char *_szCdrTime,long lIndexValue, const char *_szPickIndex)
*������  char *index_  [in]   ȥ�ػ�����ȥ�عؼ��֣������������ͱ�����
*������  ������ȥ�عؼ��ֲ��뵽ȥ��ģ���У�������ص����򲻲��뷵��false
*        ��������ص����������ģ���У�����true
*���أ�  ���index_���ص����򷵻�false����������ص����򷵻�true
*���ߣ�  ̷�� 2006.2.9
****************************************************************************************************/
bool FILTERTM_CFilter::findIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *_szPickIndex)
{
	//memset(indexTime, 0, 7);
  //strncpy(indexTime, time, 6);
  //��index_�ּ�
  //added by tanj 20060523 add the try and catch
  char indexFileName[FILTERTM_FILENAME_LEN]; 
	try
	{
		memset(indexFileName, 0, sizeof(indexFileName));
		strncpy(indexFileName, _szCdrTime, G_lFileSep);
		strncpy(indexFileName + G_lFileSep, ".idx", 5);
		if (!findIndexFile(indexFileName))
		{
			return false;
		}
		unsigned long lSecond;
		//lSecond = ((_szCdrTime[10] - '0') * 10 + (_szCdrTime[11] - '0')) * 60 + (_szCdrTime[12] - '0') * 10 + (_szCdrTime[13] - '0');
		lSecond = 0;          //�޴˾������*((long *)szCurIndex) = lSecond;ʱcore dump
		char szCurIndex[FILTERTM_INDEX_LEN];
		memset(szCurIndex, 0, sizeof(szCurIndex));
		if (lIndexValue > 3599 || lIndexValue < 0)
		{
			lIndexValue = lIndexValue%3600;
		}
		lSecond = lIndexValue;
		
		*((unsigned long *)szCurIndex) = lSecond;	       // lSecond is file index of the pick keys
		memcpy(szCurIndex + sizeof(unsigned long), _szPickIndex, indexSize - sizeof(unsigned long) - 1);
		indexTableManager.loadIndexTable(m_szSourceId, indexFileName);
		
		long tempBlock;
		long count = indexTableManager.getTableItem(lSecond);
		while(true)
		{
			//�ҵ�Ӧ�ò����λ�ã�tempBlock��blockList�е�λ�ã�count�����ļ��е�λ��
			tempBlock = readBlock(m_szSourceId, indexFileName,count);
			if (blockList[tempBlock].getNextBlock() == -1)
			{
				break;
			}
			if (blockList[tempBlock].lastIndexCmp(szCurIndex) <= 0)
			{
				break;
			}
	
			count = blockList[tempBlock].getNextBlock();
		}
		long location = 0;
		if (blockList[tempBlock].binarySearch(szCurIndex, location))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (CF_CError &error)
	{
		commit();
		truncIndexFile(indexFileName);
		return false;
	}
}



/*****************************************************************************************************
*��������bool FILTERTM_CFilter::addIndex(char *index_)
*������  char *index_  [in]   ȥ�ػ�����ȥ�عؼ��֣������������ͱ�����
*������  ������ȥ�عؼ��ֲ��뵽ȥ��ģ���У�������ص����򲻲��뷵��false
*        ��������ص����������ģ���У�����true
*���أ�  ���index_���ص����򷵻�false����������ص����򷵻�true
*���ߣ�  ̷�� 2004.9.10
****************************************************************************************************/
bool FILTERTM_CFilter::addIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *_szPickIndex)
{
	//memset(indexTime, 0, 7);
  //strncpy(indexTime, time, 6);
  //��index_�ּ�

	char indexFileName[FILTERTM_FILENAME_LEN];
	memset(indexFileName, 0, sizeof(indexFileName));
	strncpy(indexFileName, _szCdrTime, G_lFileSep);
	strncpy(indexFileName + G_lFileSep, ".idx", 5);
	//20050316 by tanj ���Ӳ����쳣
	try
	{
		unsigned long lSecond;
		//lSecond = ((_szCdrTime[10] - '0') * 10 + (_szCdrTime[11] - '0')) * 60 + (_szCdrTime[12] - '0') * 10 + (_szCdrTime[13] - '0');
		lSecond = 0;          //�޴˾������*((long *)szCurIndex) = lSecond;ʱcore dump
		char szCurIndex[FILTERTM_INDEX_LEN];
		memset(szCurIndex, 0, sizeof(szCurIndex));
		if (lIndexValue > 3599 || lIndexValue < 0)
		{
			lIndexValue = lIndexValue%3600;
		}
		lSecond = lIndexValue;
		
		*((unsigned long *)szCurIndex) = lSecond;	       // lSecond is file index of the pick keys
		memcpy(szCurIndex + sizeof(unsigned long), _szPickIndex, indexSize - sizeof(unsigned long) - 1);
		indexTableManager.loadIndexTable(m_szSourceId, indexFileName);
		
		long tempBlock;
		long count = indexTableManager.getTableItem(lSecond);
		while(true)
		{
			//�ҵ�Ӧ�ò����λ�ã�tempBlock��blockList�е�λ�ã�count�����ļ��е�λ��
			tempBlock = readBlock(m_szSourceId, indexFileName,count);
			if (blockList[tempBlock].getNextBlock() == -1)
			{
				break;
			}
			if (blockList[tempBlock].lastIndexCmp(szCurIndex) <= 0)
			{
				break;
			}
	
			count = blockList[tempBlock].getNextBlock();
		}
	
		//�˿��Ѿ�������Ҫ���ѿ�
		if (blockList[tempBlock].isFull())
		{
	
		  long newBlockInFile; 
		  long newBlockNo;		
	
	//  old code
			//indexTableManager.setBlockInFile(indexTableManager.getBlockInFile() + 1);
			
	
			//newBlockInFile = indexTableManager.getBlockInFile() - 1;
	//  end of old code
	
	//**************modified by tanj 2004.11.30 ����ʹ��ɾ������ʱ�����Ŀտ�
	   	indexTableManager.setBlockInFile(indexTableManager.getBlockInFile() + 1);
	   	newBlockInFile = indexTableManager.getBlockInFile() -1;   //֮ǰblockInFile�Ѿ���1����������Ҫ��1
	
	//***************end of modify******************************************
	
			
			newBlockNo = readBlock(m_szSourceId, indexFileName, newBlockInFile);
			
			blockList[newBlockNo].setNextBlock(blockList[tempBlock].getNextBlock());
			blockList[tempBlock].setNextBlock(newBlockInFile);
			
			
			//��tempBlock��1/5��idnexת�Ƶ�newBlockNo��
			moveIndex(tempBlock,newBlockNo);
	    
		  if (blockList[tempBlock].lastIndexCmp(szCurIndex) > 0)
		  {   //Ҫ�����indexӦ����blockList[newBlockNo]��
		  	  tempBlock = newBlockNo;
		  }
		}
		//tempBlock��Ȼ��blockList��Ҫ�����λ��
		long location;
		if (blockList[tempBlock].binarySearch(szCurIndex, location))
		{
			//��blockList[tempBlock]�в��ҵ�index
			if (blockList[tempBlock].getDeleteFlag(location) == true)
			{
				//the pick key exist, but was deleted before, we added it again
				blockList[tempBlock].setDeleteFlag(false, location);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			blockList[tempBlock].insertIndex(szCurIndex,location);
		  return true;
		}
	}
	catch (CF_CError &error)
	{
		commit();
		truncIndexFile(indexFileName);
		return true;
	}
}

//added by tanj 2004.11.29  for remove index from index file
bool FILTERTM_CFilter::removeIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *_szPickIndex)
{
	char indexFileName[FILTERTM_FILENAME_LEN];
	memset(indexFileName, 0, sizeof(indexFileName));
	strncpy(indexFileName, _szCdrTime, G_lFileSep);
	strncpy(indexFileName + G_lFileSep, ".idx", 5);
	//20060316 by tanj ���Ӳ����쳣
	try
	{
		unsigned long lSecond;
		//lSecond = ((_szCdrTime[10] - '0') * 10 + (_szCdrTime[11] - '0')) * 60 + (_szCdrTime[12] - '0') * 10 + (_szCdrTime[13] - '0');
		lSecond = 0;
		char szCurIndex[FILTERTM_INDEX_LEN];
		memset(szCurIndex, 0, sizeof(szCurIndex));
		if (lIndexValue > 3599 || lIndexValue < 0)
		{
			lIndexValue = lIndexValue%3600;
		}
		lSecond = lIndexValue;
		
		*(unsigned long *)szCurIndex = lSecond;	       // lSecond is file index of the pick keys
		memcpy(szCurIndex + sizeof(unsigned long), _szPickIndex, indexSize - sizeof(unsigned long) - 1);	
		indexTableManager.loadIndexTable(m_szSourceId, indexFileName);
		
		long tempBlock;
		long count = indexTableManager.getTableItem(lSecond);
		while(true)
		{
			//�ҵ�Ӧ�ò����λ�ã�tempBlock��blockList�е�λ�ã�count�����ļ��е�λ��
			tempBlock = readBlock(m_szSourceId, indexFileName,count);
			if (blockList[tempBlock].getNextBlock() == -1)
			{
				break;
			}
			if (blockList[tempBlock].lastIndexCmp(szCurIndex) <= 0)
			{
				break;
			}
	
			count = blockList[tempBlock].getNextBlock();
		}
	
	
		long location;
		if (blockList[tempBlock].binarySearch(szCurIndex, location))
		{
			//��blockList[tempblock]�в��ҵ�index
			if (blockList[tempBlock].getDeleteFlag(location) == false)
			{
				blockList[tempBlock].setDeleteFlag(true, location);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
		   return false;
		}
	}
	catch(CF_CError &error)
	{
		commit();
		truncIndexFile(indexFileName);
		return true;
	}
}
//end of addition
	


void FILTERTM_CFilter::backup()
{
	truncFile(BLOCK_DATA_BAK);
	truncFile(INDEX_TABLE_BAK);
	
	indexTableManager.backup();
	
  fstream fileStream;
	openTempFile(fileStream, BLOCK_DATA_BAK);	
	FILTERTM_SBlockIndex blockIndex;
	for (long current = head; current != -1; current = blockList[current].backward)
	{
		if (InFile != blockList[current].modified)
		{
			 strcpy(blockIndex.fileName, blockList[current].getFileName());
			 blockIndex.blockNo = blockList[current].getBlockNo();
			 fileStream.write((char *)&blockIndex, sizeof(FILTERTM_SBlockIndex));
       fileStream.write((char *)blockList[current].getBlockHead(), sizeof(FILTERTM_SBlockHead));
       fileStream.write((char *)blockList[current].getBlockData(), indexSize*maxIndexInBlock);
//*************added by tanj 2004.12.2   ����ļ�����д��״̬ ***************
		   if ( !(fileStream.good()) )
		   {
		     char msg[256];
		     sprintf(msg, "write backup file fail!:FILTERTM_CFilter::backup() at %s file %d block", blockIndex.fileName, blockIndex.blockNo);
         throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		   }
//*************end of addition ******************************
    }
  }
  fileStream.close();
}
			
void FILTERTM_CFilter::restore()
{
	//added by tanj 20060117 �ָ�֮ǰ����־�ļ��������Ƿ�Ҫ�ָ�
	fstream CommitFlagStream;
	openTempFile(CommitFlagStream,COMMIT_FLAG);
	char szCommitFlag[5];
	CommitFlagStream.read((char *)szCommitFlag,sizeof(char));
	if ('0' == szCommitFlag[0])
	{
		//added by tanj ��ֹ����д����ʱ�ļ�����commit֮ǰ��kill
		truncFile(INDEX_TABLE_TEMP);
		truncFile(BLOCK_INDEX_TEMP);
		truncFile(BLOCK_DATA_TEMP);
		truncFile(BLOCK_DATA_BAK);
		truncFile(INDEX_TABLE_BAK);
		return;
	}
	
	indexTableManager.restore(m_szSourceId);
	
	fstream fileStream;
	openTempFile(fileStream, BLOCK_DATA_BAK);
	
	for (long current = head; current != -1; current = blockList[current].backward)
	{
		if (!blockList[current].loadBak(m_szSourceId, fileStream))
		{
			break;
		}
		blockList[current].modified = InMemory;
	}

	commit();
}
/************************************************************************************************
*��������void FILTERTM_CFilter::commit()
*��������
*���������ڴ��е�ʱ����������ݿ飨modified == InMemory��and ��ʱ�ļ������ݻ�д�������ļ���
       ��������ʱ�ļ�
*����ֵ����
*���ߣ�̷�� 2004.10.15
************************************************************************************************/
void FILTERTM_CFilter::commit()
{
	//added by tanj 20060117 �ύ֮ǰ��Ҫд��־�ļ�
	fstream CommitFlagStream;
	openTempFile(CommitFlagStream,COMMIT_FLAG);
	CommitFlagStream.write((char *)"1",sizeof(char));
	if ( !(CommitFlagStream.good()) )
	{
	  char msg[256];
	  sprintf(msg, "Can not write %s file before commit!",COMMIT_FLAG);
    throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
	}
	CommitFlagStream.close();

  indexTableManager.commit();	
	fstream fileStream;
	for (long current = head; current != -1; current = blockList[current].backward)
	{
		if (InFile != blockList[current].modified)
		{
			//strncpy(G_Index, blockList[current].getFileName(), G_Dir_Sep);
			openIndexFile(fileStream, blockList[current].getFileName());
			fileStream.seekp(sizeof(FILTERTM_SIndexTableInFile) + blockList[current].getBlockNo()*blockSize, ios::beg);
			fileStream.write((char *)blockList[current].getBlockHead(), sizeof(FILTERTM_SBlockHead));
			fileStream.write((char *)blockList[current].getBlockData(), indexSize*maxIndexInBlock);
//*************added by tanj 2004.12.2   ����ļ�����д��״̬ ***************
		  if ( !(fileStream.good()) )
		  {
		    char msg[256];
		    sprintf(msg, "commit fail!:FILTERTM_CFilter::commit() at %s file %d block",blockList[current].getFileName(),blockList[current].getBlockNo());
        throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		  }
//*************end of addition ******************************
			fileStream.close();
			blockList[current].modifyBlock(InFile);
		}
	}
	
	FILTERTM_SBlockIndex blockIndex;
	fstream blockIndexStream;
	openTempFile(blockIndexStream, BLOCK_INDEX_TEMP);
	fstream tempDataStream;
	openTempFile(tempDataStream,BLOCK_DATA_TEMP);
	char *tempData;
	tempData = (char *)malloc(blockSize);
	if (tempData == NULL)
	{
		throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, FILTERTM_NOT_ENOUGH_MEMORY, errno,(char *)"Not Enough Memory!",__FILE__,__LINE__);
		return ;
	}
	for (long current; ;)
	{
		blockIndexStream.read((char *)&blockIndex, sizeof(FILTERTM_SBlockIndex));
		if (blockIndexStream.gcount() < sizeof(FILTERTM_SBlockIndex))
		{
			break;
		}
		
		for (current = head; current != -1; current = blockList[current].backward)
		{
			if ((strcmp(blockIndex.fileName, blockList[current].getFileName()) == 0) 
			      && (blockIndex.blockNo == blockList[current].getBlockNo()) )
			{
				break;
			}
		}

		if (current == -1)
		{
			fstream fileStream;
			//strncpy(G_Index, blockIndex.fileName, G_Dir_Sep);
			openIndexFile(fileStream, blockIndex.fileName);
			
			tempDataStream.read((char *)tempData,blockSize);
			fileStream.seekp(sizeof(FILTERTM_SIndexTableInFile) + blockIndex.blockNo*blockSize, ios::beg);
			fileStream.write((char *)tempData, blockSize);
//*************added by tanj 2004.12.2   ����ļ�����д��״̬ ***************
		   if ( !(fileStream.good()) )
		   {
		     char msg[256];
		     sprintf(msg, "commit fail!:FILTERTM_CFilter::commit() at %s file %d block",blockIndex.fileName, blockIndex.blockNo);
         throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
		   }
//*************end of addition ******************************
			fileStream.close();
		}
		else
		{
			tempDataStream.seekg(blockSize, ios::cur);
		}
	}
	truncFile(INDEX_TABLE_TEMP);
	truncFile(BLOCK_INDEX_TEMP);
	truncFile(BLOCK_DATA_TEMP);
	
	truncFile(BLOCK_DATA_BAK);
	truncFile(INDEX_TABLE_BAK);
	
	free(tempData);

  //added by tanj 20060117 �ύ��ϣ��ñ�־�ļ�
	openTempFile(CommitFlagStream,COMMIT_FLAG);
	CommitFlagStream.write((char *)"0",sizeof(char));
	if ( !(CommitFlagStream.good()) )
	{
	  char msg[256];
	  sprintf(msg, "Can not write %s file after commit!",COMMIT_FLAG);
    throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_WRITE_FILE,  msg, __FILE__, __LINE__);
	}
	CommitFlagStream.close();
}

/************************************************************************************************
*��������
*������������ĳһ���ļ��Ĺ����г���ʱ����Ҫ���ڴ�������ļ��е��������˵�������ļ�֮ǰ��״̬��
       ���������ʱ�ļ����ٽ��ڴ��е����ݿ�Ļص��������ļ�һ�µ�״̬
*����ֵ����
*���ߣ�̷�� 2004.10.15
************************************************************************************************/
void FILTERTM_CFilter::rollBack()
{
	truncFile(INDEX_TABLE_TEMP);
	truncFile(BLOCK_INDEX_TEMP);
	truncFile(BLOCK_DATA_TEMP);
	//added by tanj 2004.12.16
	truncFile(INDEX_TABLE_BAK);
	truncFile(BLOCK_DATA_BAK);
	//end of addition

	for (long current = head; current != -1; current = blockList[current].backward)
	{
		if (InFile != blockList[current].modified)
		{
			fstream fileStream;
			openIndexFile(fileStream, blockList[current].getFileName());
			fileStream.seekg(sizeof(FILTERTM_SIndexTableInFile) + blockList[current].getBlockNo()*blockSize, ios::beg);
			fileStream.read((char *)blockList[current].getBlockHead(), sizeof(FILTERTM_SBlockHead));
		  if (fileStream.gcount() < sizeof(FILTERTM_SBlockHead))
		  {
			  blockList[current].setNextBlock(-1);
		  }
		  else
		  {
		   	fileStream.read((char *)blockList[current].getBlockData(), indexSize*maxIndexInBlock);
		  }
		  blockList[current].modifyBlock(InFile);
		}
  }
}





/**/
/////////////////////////////////////////////////////////////////////////
//for debuging
/////////////////////////////////////////////////////////////////////////
void FILTERTM_CFilter::display()
{
	//cout<<endl<<"FILTERTM_CFilter:"<<endl;
	//cout<<"head = "<<head<<"	"<<"tail = "<<tail<<"	"<<"blockNum = "<<blockNum<<endl;
	//cout<<"display indexTableManager*******************************************"<<endl;
	

	cout<<"display blockList*******************************************"<<endl;
	cout<<"head = "<<head<<"tail = "<<tail<<endl;
	for (long current = 0; current < blockNum; current++)
	{
		cout<<current<<"	"<<blockList[current].forward<<"	"<<blockList[current].backward<<"	"<<blockList[current].modified<<endl;
	}
	
	for (long current = 0; current < blockNum; current++)
	{
		blockList[current].disBlock();
	}
	indexTableManager.disChain();
	indexTableManager.disTable();
}

/*********************************************************************
*������char    *file_Name   [in]     Ҫ���ҵ������ļ����ļ��� 
*�������Ӵ������ļ�Ŀ¼G_szPickIndexPath�µ���ʱ�ļ��������ļ�����
       file_Name�����������ָ�������file_Name�����ڣ��򷵻�false������
       ���� true
*���أ���
*���ߣ�̷��    2006.02.10
**********************************************************************/
bool findIndexFile(const char *file_Name)
{
	char tempFullFileName[FILTERTM_FILEPATH_LEN + FILTERTM_FILENAME_LEN];
	memset(tempFullFileName, 0, sizeof(tempFullFileName));
	strcpy(tempFullFileName, G_szPickIndexPath);
	if (tempFullFileName[strlen(tempFullFileName) - 1] != '/')
  {
   	 strcat(tempFullFileName, "/");
  }
  
  char dirName[FILTERTM_FILENAME_LEN];
  memset(dirName, 0, sizeof(dirName));
  strncpy(dirName, file_Name, G_lDirSep);
	strcat(tempFullFileName, dirName);
	strcat(tempFullFileName, "/");
  strcat(tempFullFileName, file_Name);
  if (access(tempFullFileName,F_OK) == 0)
  {
  	return true;
  }
  else
  {
  	return false;
  }
}

/*********************************************************************
*������fstream &fileStream  [out]    �����������ļ���fstream����
       char    *file_Name   [in]     ��Ҫ�򿪵������ļ����ļ��� 
*�������Ӵ������ļ�Ŀ¼G_szPickIndexPath�µ���ʱ�ļ��������ļ�����
       file_Name�����������ָ������file_Stream�������������
       ���file_Name�����ڣ��򴴽����ļ�Ȼ���ٴ�
*���أ���
*���ߣ�̷��    2004.10.15
**********************************************************************/
void openIndexFile(fstream &file_Stream, const char *file_Name)
{
	char tempFullFileName[FILTERTM_FILEPATH_LEN + FILTERTM_FILENAME_LEN];
	memset(tempFullFileName, 0, sizeof(tempFullFileName));
	strcpy(tempFullFileName, G_szPickIndexPath);
	if (tempFullFileName[strlen(tempFullFileName) - 1] != '/')
  {
   	 strcat(tempFullFileName, "/");
  }
  
  char dirName[FILTERTM_FILENAME_LEN];
  memset(dirName, 0, sizeof(dirName));
  strncpy(dirName, file_Name, G_lDirSep);
	strcat(tempFullFileName, dirName);
	strcat(tempFullFileName, "/");
	if (chkDir(tempFullFileName) == -1)
  {
   	//·�����������޷�����
		char msg[256];
		sprintf(msg, "Cannot create path %s",tempFullFileName);
		throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG,errno, FILTERTM_ERR_IN_CREATE_DIR, msg, __FILE__, __LINE__);
		return ;
  }

  strcat(tempFullFileName, file_Name);
  
  if (file_Stream)
	{
		file_Stream.close();              
	}
	file_Stream.open(tempFullFileName, ios::in|ios::out|ios::binary);
	if (!file_Stream)
	{
		fstream tempStream;
		tempStream.open(tempFullFileName,ios::out|ios::binary);
		if (!tempStream)
		{
			char msg[256];
			sprintf(msg, "Cannot create %s",tempFullFileName);
			throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, errno, FILTERTM_ERR_IN_CREATE_INDEX_FILE, msg, __FILE__, __LINE__);
		  return ;
		}
		tempStream.close();
		file_Stream.open(tempFullFileName,ios::in|ios::out|ios::binary);
		if (!file_Stream)
		{
			char msg[256];
			sprintf(msg, "Cannot open %s",tempFullFileName);
			throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, errno, FILTERTM_ERR_IN_OPEN_INDEX_FILE, msg, __FILE__, __LINE__);
		  return ;
		}
	}
}


/*********************************************************************
*������fstream &fileStream  [out]    ��������ʱ�ļ���fstream����
       char    *file_Name   [in]     ��Ҫ�򿪵���ʱ�ļ����ļ��ļ��� 
*�������Ӵ������ļ�Ŀ¼G_szPickIndexPath�µ���ʱ�ļ�����ʱ�ļ�����
       file_Name�����������ָ������file_Stream�������������
       ���file_Name�����ڣ��򴴽����ļ�Ȼ���ٴ�
*���أ���
*���ߣ�̷��    2004.10.15
**********************************************************************/
void openTempFile(fstream &file_Stream, const char *file_Name)
{
	char tempFullFileName[FILTERTM_FILEPATH_LEN + FILTERTM_FILENAME_LEN];
	memset(tempFullFileName, 0, sizeof(tempFullFileName));
	strcpy(tempFullFileName, G_szPickIndexPath);
	strcat(tempFullFileName, file_Name);                        
	
	/*���´������ļ�*/
	if (file_Stream)
	{
		file_Stream.close();              
	}
	file_Stream.open(tempFullFileName, ios::in|ios::out|ios::binary);
	if (!file_Stream)
	{
		fstream tempStream;
		tempStream.open(tempFullFileName,ios::out|ios::binary);
		if (!tempStream)
		{
			char msg[256];
			sprintf(msg, "Cannot create %s",tempFullFileName);
			throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, errno, FILTERTM_ERR_IN_CREATE_INDEX_FILE, msg, __FILE__, __LINE__);
		  return ;
		}
		tempStream.close();
		file_Stream.open(tempFullFileName,ios::in|ios::out|ios::binary);
		if (!file_Stream)
		{
			char msg[256];
			sprintf(msg, "Cannot open %s",tempFullFileName);
			throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, errno, FILTERTM_ERR_IN_OPEN_INDEX_FILE, msg, __FILE__, __LINE__);
		  return ;
		}
	}
}


///******************************************************************************************
//*������char *_dir  [in]    Ҫ���򴴽���·��
//*���������·��_dir�Ƿ���ڣ���������ڣ������ɸ�·�� ע�⣺һ��ֻ������һ��Ŀ¼
//*���أ����·���Ѿ����ڻ򲻴��ڵ��Ǵ����ɹ��򷵻�0�����·���������Ҵ������ɹ��򷵻�-1
//*���ߣ�̷��    2004.10.15
//*******************************************************************************************/
//int chkDir( char *_dir )
//{
// int i;
//     if( chdir( _dir ) ) {
//         if( mkdir( _dir,S_IRWXU|S_IRGRP|S_IXGRP ) ) 
//         {
//             return(-1);
//         }
//     }
// return(0);
//}

/******************************************************************************************
*������char file_Name[FILENAME_LEN]   Ҫ��յ��ļ���������·��
*���������ȥ�������ļ�Ŀ¼G_szPickIndexPath�е��ļ�file_Name
*����:��
*���ߣ�̷��    2004.10.15
*******************************************************************************************/
void truncFile(const char file_Name[FILTERTM_FILENAME_LEN])
{
	char fullFileName[FILTERTM_FILEPATH_LEN + FILTERTM_FILENAME_LEN];
	memset(fullFileName,0,sizeof(fullFileName));
	strcpy(fullFileName, G_szPickIndexPath);
//added by tanj 2004.12.30
	if (fullFileName[strlen(fullFileName) - 1] != '/')
  {
   	 strcat(fullFileName, "/");
  }
//end of addition
	strcat(fullFileName, file_Name);
	fstream outStream;
	outStream.open(fullFileName, ios::out|ios::trunc);
	if (!outStream)
	{
		char msg[256];
		sprintf(msg, "Cannot truncate %s",fullFileName);
		throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, errno, FILTERTM_ERR_IN_OPEN_INDEX_FILE, msg, __FILE__, __LINE__);
		return;
	}
	outStream.close();
	return;
}

//added by tanj 2004.12.30
void truncIndexFile(const char *file_Name)
{
	char tempFullFileName[FILTERTM_FILEPATH_LEN + FILTERTM_FILENAME_LEN];
	memset(tempFullFileName, 0, sizeof(tempFullFileName));
	strcpy(tempFullFileName, G_szPickIndexPath);
	if (tempFullFileName[strlen(tempFullFileName) - 1] != '/')
  {
   	 strcat(tempFullFileName, "/");
  }
  
  char dirName[FILTERTM_FILENAME_LEN];
  memset(dirName, 0, sizeof(dirName));
  strncpy(dirName, file_Name, G_lDirSep);
	strcat(tempFullFileName, dirName);
	strcat(tempFullFileName, "/");
	chkDir(tempFullFileName);
  strcat(tempFullFileName, file_Name);
	fstream outStream;
	outStream.open(tempFullFileName, ios::out|ios::trunc);
	outStream.close();
}
 
