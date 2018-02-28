/******************************************************************************
*FILTERTM_CFilter.cpp
*craated by tanj 2005.4.8
*description: the FILTERTM_CFilter class designed for picking the duplicated 
*             CDRs, whose pick keys contain cdr_time
* 20060209    为了增加程序的通用性，将索引的计算从本类中移到外面（原先是从时间字段
*             中获得分秒信息在转换成0至3599的整型值）
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
		table[i].backward = i + 1;  //建立链表
		table[i].forward = i - 1;
	}
	table[0].forward = -1;      //链表头的标志
	table[lTableNum - 1].backward = -1;    //链表尾的标志
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
		table[i].backward = i + 1;  //建立链表
		table[i].forward = i - 1;
	}
	table[0].forward = -1;      //链表头的标志
	table[tableNum - 1].backward = -1;    //链表尾的标志
	head = 0;
	tail = tableNum - 1;
}

FILTERTM_CIndexTableManager::~FILTERTM_CIndexTableManager()
{ 
}

//在临时文件中查找file_Name的索引分配表，如果找到，则返回true，location表示该索引分配表在第几个位置
//如果没有找到，则返回false，location表示最后一个位置再加1
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
*参数：  char *file_Name [in]  将file_Name文件的索引分配表load到table数组中，并使head指向它在table中的序号
*描述：  首先查看参数fileName文件的索引分配表是不是当前的（head指向的）索引分配表，如果是则直接返回，
         如果不是则查看该索引分配表是不是已经在table数组里面，如果在则重新组织链表使head指向该块；
         如果不在则先将最久未使用的索引分配表（tail指向的）置换到临时文件中，然后将参数fileName文件的
         索引分配表读到table数组的tail块中（先查看是否在临时文件中，如果在则从临时文件中读取），并将
         重新组织链表使原来的tail成为head。如果该文件不存在，则生成同名的空文件
*返回值：无
*
**********************************************************************/
void FILTERTM_CIndexTableManager::loadIndexTable(const char *szSourceId,const char *file_Name)
{
	for (int current = head; current != -1; current = table[current].backward)
	{
		 if (strcmp(table[current].tableData.fileName, file_Name) == 0 && strcmp(table[current].m_szSourceId,szSourceId) == 0)  //在内存中
		 {
		 	 if (table[current].forward >= 0)                   //且不是头
		 	 {
		 	 	 table[table[current].forward].backward = table[current].backward;
		 	 	 
		 	   if (table[current].backward >= 0)                 //且不是尾
		 	   {
		 	 	   table[table[current].backward].forward = table[current].forward;
		 	   }
		 	   else                                             //是尾
		 	   {
		 	   	 tail = table[current].forward;
		 	   }
		 	   table[current].forward = -1;                    //将current置成头
		 	   table[current].backward = head;
		 	   table[head].forward = current;
		 	   head = current;
		 	 }
		 	 return;
		 }
	}

	//将tail指向的索引分配表写回到临时文件中
	if (InMemory == table[tail].modified)
	{
		long tableLoc;
		findInTemp(table[tail].tableData.fileName, tableLoc);

   	fstream fileStream;
   	openTempFile(fileStream, INDEX_TABLE_TEMP);
   	fileStream.seekp(tableLoc*sizeof(FILTERTM_SIndexTableInFile), ios::beg);
   	fileStream.write((char *)&(table[tail].tableData), sizeof(FILTERTM_SIndexTableInFile));
//*************added by tanj 2004.12.2   检查文件流的写入状态   	
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
	
	//查找要读取的索引分配表是否在临时文件中，如果在，则从临时文件中读取索引分配表；
	//如果不在，则从索引文件中读取索引分配表
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

	//打开的如果是新文件或错误文件，需要清空
	if (strcmp(table[tail].tableData.fileName, file_Name) != 0)
	{
//modified by tanj 2004.12.30
		//fstream fileStream;
		//char tempFullFileName[FILEPATH_LEN+FILENAME_LEN];
	  //memset(tempFullFileName, 0, sizeof(tempFullFileName));
	  //strcpy(tempFullFileName, G_szPickIndexPath);
	  //strcat(tempFullFileName, file_Name);
		//fileStream.open(tempFullFileName, ios::trunc);  //清空文件
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
	
	
	//重新安排链表
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
//*************added by tanj 2004.12.2   检查文件流的写入状态   	
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
	//将内存中的索引分配表写入索引文件
	for (long current = head; current != -1; current = table[current].backward)
	{
		fstream fileStream;
		if (table[current].modified != InFile)
		{
			//strncpy(G_Index, table[current].tableData.fileName, G_Dir_Sep);
			openIndexFile(fileStream, table[current].tableData.fileName);
			fileStream.write((char *)(&(table[current].tableData)), sizeof(FILTERTM_SIndexTableInFile));
//*************added by tanj 2004.12.2   检查文件流的写入状态   	
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
	//将临时文件中不在内存中的索引分配表写入索引文件中
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
//*************added by tanj 2004.12.2   检查文件流的写入状态   	
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
//*设置内存中的数据块的性质，InFile，or InTemp or InMemory
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
*函数名：void FILTERTM_CBlock::init(long max_Index,long index_Size)
*参数：  long max_Index [in] 块中可以存放的最大去重索引数
*        long index_Size [in]  去重索引的长度
*描述：  初始化
*返回值： 无
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
	blockNo = -1;   //还没有存放数据，所以blockNo等于-1
	modified = InFile;
	forward = backward = -1;
}


void FILTERTM_CBlock::clear()
{
	setNextBlock(-1);
	setIndexInBlock(0);
	
	fileName[0] = 0;
	blockNo = -1;   //还没有存放数据，所以blockNo等于-1
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


//在临时文件中查找file_Name的第block_No块，如果找到，则返回true，location表示该块在第几个位置
//如果没有找到，则返回false，location表示最后一个位置再加1
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
*函数名：void FILTERTM_CBlock::rollBack()
*参数：  无
*描述：  如果modified == InMemory，将块中的数据回写到临时文件中
*返回置：无
*作者：  谭杰  2004.9.10
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
//*************added by tanj 2004.12.2 检查文件流的写入状态
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
*函数名：void FILTERTM_CBlock::loadBlock(char *file_Name,long block_No)
*参数：  char *file_Name [in] 需要装入的文件的文件名
*        long block_No [in]   需要装入的块号
*描述：  将file_Name文件的第Block_No块数据读到本块中
*返回值：无
*************************************************************************************************/
void FILTERTM_CBlock::loadBlock(const char *szSourceId, const char *file_Name, const long block_No)
{
	//首先清空块中的原有内容
	blockHead.nextBlock = -1;
	blockHead.indexInBlock = 0;
	memset(blockData, 0, maxIndex*indexSize);
	//查找要读取的块是否在临时文件中，如果在，则从临时文件中读取该块；
	//如果不在，则从索引文件中读取块
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
	//首先清空块中的原有内容
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

//获得块中第index_No个（从0开始计）索引的指针
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

//将index与块中第index_No个索引（从0开始计）相比较
int FILTERTM_CBlock::indexCmp(char *index, long index_No)
{
	
	if ((index_No > getIndexInBlock() - 1)||(index_No < 0))
	{
		char msg[256];
		sprintf(msg, "illegal indexNo!(%d):FILTERTM_CBlock::indexCmp(char *, long) at %s file %d block", index_No, fileName, blockNo);
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, FILTERTM_LOGIC_ERR_IN_PROGRAM, errno, msg, (char *)__FILE__, __LINE__);
		return -2;
	}
	
	//deleted by tanj 20061020 index开始的几个字节是unsigned long型的二进制数，在big-endian的CPU上可以用memcmp进行比较
	//但是在little-endian的CPU（如intel）上面无法使用memcmp进行比较
	//return memcmp(index, getIndex(index_No), indexSize - 1);  //do not cmp the delete flag	
	
	//added by tanj 20061020 使用自定义的比较函数（而非memcmp函数）对index进行比较，避免little-end的CPU处理出错
	return CompareIndex(index, getIndex(index_No));
	
}
//added by tanj 20061020 用自定义的比较函数代替memcmp函数，避免big endian 和little endian
int FILTERTM_CBlock::CompareIndex(const char *index1,const char *index2)
{
	//deleted by tanj 20061101 *(unsigned long *)(char *) 转化成unsigned long 在64位
	//的机器上面可能会有问题，当char指针地址不是按sizeof(unsigned long)对齐时，cpu会报错
	//unsigned long lIndexValue1 = *((unsigned long *)index1);
	//unsigned long lIndexValue2 = *((unsigned long *)index2);
	//end of delete by tanj 20061101
	
	//added by tanj 20061101 使用memcpy进行字符串到unsigned long的转话，避免字符地址不对齐的问题
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


//将index于块中最后一个索引相比较
int FILTERTM_CBlock::lastIndexCmp(char *index)
{
	return indexCmp(index, getIndexInBlock() - 1);
}

//将index插入到第location个（从0开始计）索引的位置，
//先将其后的索引顺次后挪，再插入
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


//对块中的索引进行二分查找，如果找到返回true
//如果找不到，返回false，并通过location返回index应该在的位置
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
*构造函数
*参数：long block_Num [in]  所含FILTERTM_CBlock对象的个数，这些对象构成一个vector
*      long index_Size [in] 去重关键字的长度（不包括长整型变量）
*      long max_Index_In_Block [in]  每个FILTERTM_CBlock对象中所含有的去重关键字的最大数目
*描述：利用输入参数初始化对象，并构造blockList中的链表
*      注意：初始化indexSize时，需要将参数index_Size + sizeof(long) + 1;
*            初始化blockSize时，blockSize = sizeof(FILTERTM_SBlockHead) + indexSize*maxIndexInBlock;
*            链表头的forward = -1, 链表尾的backward = -1;
*返回：无
*作者：谭杰    2004.9.10
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
	head = blockNum - blockNum;        //如果写成"head = 0;" 则"core dump"
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
	head = blockNum - blockNum;        //如果写成"head = 0;" 则"core dump"
	tail = blockNum - 1;
	truncFile(INDEX_TABLE_TEMP);
	truncFile(BLOCK_INDEX_TEMP);
	truncFile(BLOCK_DATA_TEMP);
	
	truncFile(BLOCK_DATA_BAK);
	truncFile(INDEX_TABLE_BAK);
}
//******************************************************************************
//设置去重索引文件的目录
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
*函数名：void FILTERTM_CFilter::moveIndex(long from, long to)
*参数：long from [in]  需要移出数据的FITLER_CBlock对象在blockList中的序号
*      long to   [in]  需要移入数据的FITLER_CBlock对象在blockList中的序号
*描述：将blockList中的第from块中1/5的数据转移到第to块中,并重新设置时间分配表
*      在调用此函数之前，blockList[from]的nextBlock应指为blockList[to]在文件中的块号
*返回：无
*作者：谭杰   2004.9.10
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

	long begin,end;   //begin,end为时间分配表中需要修改的范围的首和尾 both value should be less than 3600
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
*函数名：long FILTERTM_CFilter::readBlock(char *file_Name,long block_No)
*参数：  char *file_Name [in]  需要读入的块所在的索引文件的文件名
*        long block_No   [in]  需要读入的块在索引文件中的块号
*描述：  本函数读取fileName（输入参数）文件中的第blockNo（输入参数）块（从0开始）到blockList
         中的某一个block中。如果要读取的块原先就在blockList中，则直接返回该块在blockList中的
         序号，如果要读取的块不在blockList中，则将blockList中最久未使用块（双向链表的尾块）
         写回到临时文件中；再在临时文件中查找将要读取的块，如果找到则读到双向链表的尾块，
         并置尾块为头块，如果没有找到则到索引文件中读取要读取的块到双向链表的尾块。
         最后重新组织链表时原先的尾块成为头块
*返回：  返回读入的块在blockList中的序号
*作者：谭杰 2004.9.10
*********************************************************************************************/
long FILTERTM_CFilter::readBlock(const char *szSourceId, const char *file_Name,const long block_No)
{
	
	//按照链表的顺序查查找，如果要读入的块原来就在blockList中，则重置链表并返回位置
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
			return head;         //head指向的就是找到的块
		}
	}
	//将尾块中的数据回退到索引文件中	
	blockList[tail].write2Temp();
	blockList[tail].loadBlock(szSourceId, file_Name,block_No);
	//重新安排链表
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
*函数名：bool FILTERTM_CFilter::findIndex(const char *_szCdrTime,long lIndexValue, const char *_szPickIndex)
*参数：  char *index_  [in]   去重话单的去重关键字（不包括长整型变量）
*描述：  将话单去重关键字插入到去重模块中，如果是重单，则不插入返回false
*        如果不是重单，则将其插入模块中，返回true
*返回：  如果index_是重单，则返回false；如果不是重单，则返回true
*作者：  谭杰 2006.2.9
****************************************************************************************************/
bool FILTERTM_CFilter::findIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *_szPickIndex)
{
	//memset(indexTime, 0, 7);
  //strncpy(indexTime, time, 6);
  //把index_分拣开
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
		lSecond = 0;          //无此句则会在*((long *)szCurIndex) = lSecond;时core dump
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
			//找到应该插入的位置，tempBlock是blockList中的位置，count是在文件中的位置
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
*函数名：bool FILTERTM_CFilter::addIndex(char *index_)
*参数：  char *index_  [in]   去重话单的去重关键字（不包括长整型变量）
*描述：  将话单去重关键字插入到去重模块中，如果是重单，则不插入返回false
*        如果不是重单，则将其插入模块中，返回true
*返回：  如果index_是重单，则返回false；如果不是重单，则返回true
*作者：  谭杰 2004.9.10
****************************************************************************************************/
bool FILTERTM_CFilter::addIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *_szPickIndex)
{
	//memset(indexTime, 0, 7);
  //strncpy(indexTime, time, 6);
  //把index_分拣开

	char indexFileName[FILTERTM_FILENAME_LEN];
	memset(indexFileName, 0, sizeof(indexFileName));
	strncpy(indexFileName, _szCdrTime, G_lFileSep);
	strncpy(indexFileName + G_lFileSep, ".idx", 5);
	//20050316 by tanj 增加捕获异常
	try
	{
		unsigned long lSecond;
		//lSecond = ((_szCdrTime[10] - '0') * 10 + (_szCdrTime[11] - '0')) * 60 + (_szCdrTime[12] - '0') * 10 + (_szCdrTime[13] - '0');
		lSecond = 0;          //无此句则会在*((long *)szCurIndex) = lSecond;时core dump
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
			//找到应该插入的位置，tempBlock是blockList中的位置，count是在文件中的位置
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
	
		//此块已经满，需要分裂块
		if (blockList[tempBlock].isFull())
		{
	
		  long newBlockInFile; 
		  long newBlockNo;		
	
	//  old code
			//indexTableManager.setBlockInFile(indexTableManager.getBlockInFile() + 1);
			
	
			//newBlockInFile = indexTableManager.getBlockInFile() - 1;
	//  end of old code
	
	//**************modified by tanj 2004.11.30 可以使用删除索引时产生的空块
	   	indexTableManager.setBlockInFile(indexTableManager.getBlockInFile() + 1);
	   	newBlockInFile = indexTableManager.getBlockInFile() -1;   //之前blockInFile已经加1，所以现在要减1
	
	//***************end of modify******************************************
	
			
			newBlockNo = readBlock(m_szSourceId, indexFileName, newBlockInFile);
			
			blockList[newBlockNo].setNextBlock(blockList[tempBlock].getNextBlock());
			blockList[tempBlock].setNextBlock(newBlockInFile);
			
			
			//将tempBlock中1/5的idnex转移到newBlockNo中
			moveIndex(tempBlock,newBlockNo);
	    
		  if (blockList[tempBlock].lastIndexCmp(szCurIndex) > 0)
		  {   //要插入的index应该在blockList[newBlockNo]中
		  	  tempBlock = newBlockNo;
		  }
		}
		//tempBlock依然是blockList中要插入的位置
		long location;
		if (blockList[tempBlock].binarySearch(szCurIndex, location))
		{
			//在blockList[tempBlock]中查找到index
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
	//20060316 by tanj 增加捕获异常
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
			//找到应该插入的位置，tempBlock是blockList中的位置，count是在文件中的位置
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
			//在blockList[tempblock]中查找到index
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
//*************added by tanj 2004.12.2   检查文件流的写入状态 ***************
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
	//added by tanj 20060117 恢复之前读标志文件，决定是否要恢复
	fstream CommitFlagStream;
	openTempFile(CommitFlagStream,COMMIT_FLAG);
	char szCommitFlag[5];
	CommitFlagStream.read((char *)szCommitFlag,sizeof(char));
	if ('0' == szCommitFlag[0])
	{
		//added by tanj 防止程序写了临时文件而在commit之前被kill
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
*函数名：void FILTERTM_CFilter::commit()
*参数：无
*描述：将内存中的时间分配表和数据块（modified == InMemory）and 临时文件的数据回写到索引文件中
       最后清空临时文件
*返回值：无
*作者：谭杰 2004.10.15
************************************************************************************************/
void FILTERTM_CFilter::commit()
{
	//added by tanj 20060117 提交之前需要写标志文件
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
//*************added by tanj 2004.12.2   检查文件流的写入状态 ***************
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
//*************added by tanj 2004.12.2   检查文件流的写入状态 ***************
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

  //added by tanj 20060117 提交完毕，置标志文件
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
*参数：无
*描述：当处理某一个文件的过程中出错时，需要将内存和索引文件中的索引回退到处理该文件之前的状态。
       首先清空临时文件，再将内存中的数据块改回到和索引文件一致的状态
*返回值：无
*作者：谭杰 2004.10.15
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
*参数：char    *file_Name   [in]     要查找的索引文件的文件名 
*描述：从打开索引文件目录G_szPickIndexPath下的临时文件，索引文件名由
       file_Name（输入参数）指定，如果file_Name不存在，则返回false；存在
       返回 true
*返回：无
*作者：谭杰    2006.02.10
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
*参数：fstream &fileStream  [out]    用来打开索引文件的fstream对象
       char    *file_Name   [in]     将要打开的索引文件的文件名 
*描述：从打开索引文件目录G_szPickIndexPath下的临时文件，索引文件名由
       file_Name（输入参数）指定，由file_Stream（输出参数）打开
       如果file_Name不存在，则创建该文件然后再打开
*返回：无
*作者：谭杰    2004.10.15
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
   	//路径不存在且无法生成
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
*参数：fstream &fileStream  [out]    用来打开临时文件的fstream对象
       char    *file_Name   [in]     将要打开的临时文件的文件文件名 
*描述：从打开索引文件目录G_szPickIndexPath下的临时文件，临时文件名由
       file_Name（输入参数）指定，由file_Stream（输出参数）打开
       如果file_Name不存在，则创建该文件然后再打开
*返回：无
*作者：谭杰    2004.10.15
**********************************************************************/
void openTempFile(fstream &file_Stream, const char *file_Name)
{
	char tempFullFileName[FILTERTM_FILEPATH_LEN + FILTERTM_FILENAME_LEN];
	memset(tempFullFileName, 0, sizeof(tempFullFileName));
	strcpy(tempFullFileName, G_szPickIndexPath);
	strcat(tempFullFileName, file_Name);                        
	
	/*以下打开索引文件*/
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
//*参数：char *_dir  [in]    要检查或创建的路径
//*描述：检查路径_dir是否存在，如果不存在，则生成该路径 注意：一次只能生成一层目录
//*返回：如果路径已经存在或不存在但是创建成功则返回0，如果路径不存在且创建不成功则返回-1
//*作者：谭杰    2004.10.15
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
*参数：char file_Name[FILENAME_LEN]   要清空的文件名，不带路径
*描述：清空去重索引文件目录G_szPickIndexPath中的文件file_Name
*返回:无
*作者：谭杰    2004.10.15
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
 
