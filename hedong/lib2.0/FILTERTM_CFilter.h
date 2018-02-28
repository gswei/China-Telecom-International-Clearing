/******************************************************************************
*FILTERTM_CFilter.h
*craated by tanj 2005.4.8
*description: the FILTERTM_CFilter class designed for picking the duplicated 
*             CDRs, whose pick keys contain cdr_time
******************************************************************************/
#ifndef FILTERTM_CFILTER_H_
#define FILTERTM_CFILTER_H_
#include "FILTERTM_ERRCODE_DEFINE.h"
#include <fstream.h>
#include <iostream.h>
#include <vector>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "Common.h"
#include "CF_CError.h"


//ERROR CODE DEFINE

// CONST VALUE DEFINE
const char  INDEX_TABLE_TEMP[]  = "indextable.tmp";
const char  BLOCK_INDEX_TEMP[] = "blockindex.tmp";
const char  BLOCK_DATA_TEMP[]  = "blockdata.tmp";
const char  INDEX_TABLE_BAK[]  = "indextable.bak";
const char  BLOCK_DATA_BAK[]   = "blockdata.bak";
const char  COMMIT_FLAG[]      = "commit.flg";        //added by tanj 增加标志文件

const long  FILTERTM_INDEX_LEN          =  3000;
const long  FILTERFM_TIME_LEN           =  15;
const long  FILTERTM_FILENAME_LEN       =  25;   //索引文件名的大小
const long  FILTERTM_FILEPATH_LEN       =  256;
const long  G_lDirSep = 6;
const long  G_lFileSep = 10;


enum Modify
{
	InFile,InMemory,InTemp
};

enum CommitFlag
{
	CommitEnd,CommitBegin 
};

//**********************DEFINITION FOR INDEX_TABLE ************************

const long INDEX_TABLE_SIZE = 3600;

/***************************************************************
*本结构是存储在文件中的数据结构
***************************************************************/
struct FILTERTM_SIndexTableInFile
{
	char fileName[FILTERTM_FILENAME_LEN];   //索引文件的文件名
	long indexSize;                //added by tanj 20060213去重索引的长度，包括前面的long整数和后面的bool数
	long indexInBlock;             //added by tanj 20060213每个数据块中的记录的个数	
	long blockInFile;              //索引文件中所存储的数据块数
	long emptyHead;                //空块链的头，用于从索引文件中删除索引

	long tableItem[INDEX_TABLE_SIZE];          //索引分配表
	FILTERTM_SIndexTableInFile()
	{
		fileName[0] = 0;
		blockInFile = 0;
		emptyHead = -1;
		for (long i = 0; i < INDEX_TABLE_SIZE; i++)
		{
			tableItem[i] = 0;
		}
	}
};
/***************************************************************
*索引分配表结构，包括索引文件的文件头结构和一个unsigned long数组
*索引文件的文件头，包括索引文件名和文件中的块数。
*unsigned long 数组存储了每一秒的话单的索引在文件中的块号,包括一
*个长度为60*60的long型的数组，一天中每一秒钟的话单索引对应一个长
*整型变量，此变量表示该话单索引所在的块的块号
****************************************************************/
struct FILTERTM_SIndexTable
{
	Modify modified;
	long forward;         //双向链表的头指针
	long backward;        //双向链表的尾指针，指向最久未使用的索引分配表
	char m_szSourceId[10];     //当前数据源  added by tanj 20051114
	FILTERTM_SIndexTableInFile tableData; 
};

/****************************************************************
*索引分配表管理类，管理若干个FILTERTM_SIndexTable结构，并在必要的时候
*负责将最久未使用的结构置换出内存
*****************************************************************/
class FILTERTM_CIndexTableManager
{
	public:
	  FILTERTM_CIndexTableManager(const long _lTableNUm);
	  ~FILTERTM_CIndexTableManager();
	  
	public:
	  void loadIndexTable(const char *szSourceId, const char * fileName);
	  void setTableItem(long sec,long blockNo);	  
	  void modifyTable(Modify modify);
	  void commit();	  
	  void backup();
	  void restore(const char *szSourceId);
	  void disChain();
	  void disTable();
    //void init(long table_Num);
	  long getTableItem(long sec);
	  long getBlockInFile();
	  long getEmptyHead();
	  void setEmptyHead(long emptyHead);
	  void setBlockInFile(long blockInFile);
	  bool findInTemp(const char *file_Name, long &location);
	  void clear();            //added by tanj 20051121
	private:
	  std::vector<FILTERTM_SIndexTable> table;
	  long head;    //头指针指向当前的索引分配表
	  long tail;
	  long tableNum;
};

//************************************************************************
//*******************DEFINITION FOR BLOCK ********************************
//************************************************************************

struct FILTERTM_SBlockHead
{
	long nextBlock;
	long indexInBlock;
};
//*****************************************************************************
//临时文件中数据块的索引，在blockindex.tmp中，由于它占用的空间比较小，所以被用来
//查找在临时文件中的数据块
//*****************************************************************************
struct FILTERTM_SBlockIndex
{
	long blockNo;
	char fileName[FILTERTM_FILENAME_LEN];
	FILTERTM_SBlockIndex()
	{
		memset(fileName, 0, sizeof(fileName));
	}
};

class FILTERTM_CBlock
{
	public:
	  FILTERTM_CBlock();
	  ~FILTERTM_CBlock();
	public:
	  void init(long max_Index,long index_Size);
	  void insertIndex(char *index,long location);	
	  void deleteIndex(long location);
	  void modifyBlock(Modify modify);	    
	  void write2Temp();	  
	  void setBlockNo(long block_No);
	  void setNextBlock(long next_Block);
	  void setIndexInBlock(long index_In_Block);
	  void loadBlock(const char *szSourceId, const char *fileName, const long blockNo);	  
	  void disBlock();
	  void disIndex(long index_No);	  
	  bool isFull();
	  bool binarySearch(char *index,long &location);	  
    bool findInTemp(const char *file_Name, const long block_No, long &location);
    bool loadBak(const char *szSourceId, fstream &fileStream);
    int  lastIndexCmp(char *index);
	  int  indexCmp(char *index, long index_No);
	  long getBlockNo();
	  long getNextBlock();
	  long getIndexInBlock();
	  bool getDeleteFlag(const long _lLocation);
	  void setDeleteFlag(const bool _bDeleteFlag, const long lLocation);
	  long forward;
	  long backward;
	
	  const char *getFileName();
	  const char *getSourceId();
	  void setSourceId(const char *szSourceId);
	  char *getIndex(long index_No);
    char *getBlockData();
    
    //added by tanj 20061020  增加自定义的比对函数对index进行比较，避免little-endian的CPU处理出错
    int CompareIndex(const char *index1,const char *index2);
    
    FILTERTM_SBlockHead *getBlockHead();
	  Modify modified;
		
		void clear();                //added by tanjie 20051121
	protected:
	  //blockHead 和 blockData所指向的数据是将要写到外存中的数据
	  FILTERTM_SBlockHead blockHead;	
	  char *blockData;
	  char fileName[FILTERTM_FILENAME_LEN];
	  long blockNo;        //本块数据在文件中的块号
	  long indexSize;
	  long maxIndex;
	  long blockSize;
	  char m_szSourceId[10];      //本数据块所属的数据源表示 added by tanj 改为按数据源
};





//****************************************************************************
//*********************DEFINITION FOR FILTER**********************************
//****************************************************************************


class FILTERTM_CFilter
{
	public:
	   FILTERTM_CFilter(long index_Table_Num, long block_Num, long index_Total_Size, long max_Index_In_Block);
	  ~FILTERTM_CFilter();
	protected:
	  long readBlock(const char *szSourceId, const char *fileName, const long blockNo);
	  void moveIndex(const long from, const long to);	 
	public:
	  bool addIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *_szPickIndex);
    bool removeIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *index_);
    bool findIndex(const char *_szCdrTime,unsigned long lIndexValue, const char *_szPickIndex);
	  void commit();
	  void backup();
	  void restore();
	  void display();
    void rollBack();
    void setSource(const char *szSourceId, const char *index_Path);
    void clear();     //added by tanj 20051121   清除内存,临时文件中的所有数据
	protected:
	  long head;
	  long tail;
	  std::vector<FILTERTM_CBlock> blockList;
	  FILTERTM_CIndexTableManager indexTableManager;
	  long indexSize;              //indexSize = pick keys length + sizeof(long) + 1  (1 byte for delete flag)
	  long maxIndexInBlock;
	  long indexTableNum;
	  long blockNum;
	  long blockSize;             //sizeof(FILTERTM_SBlockHead) + indexSize*maxIndexInBlock
	  char m_szSourceId[10];       //当前涉及的数据源 added by tanj 20051114
	  //map<string,string>  m_AllSource    //所有设计到的数据源 added by tanj 20051114
};


void openIndexFile(fstream &file_Stream, const char *file_Name);
void openTempFile(fstream &file_Stream, const char *file_Name);
//int  chkDir( char *_dir );
void truncFile(const char file_Name[FILTERTM_FILENAME_LEN]);
void truncIndexFile(const char *file_Name);
bool findIndexFile(const char *file_Name);




#endif