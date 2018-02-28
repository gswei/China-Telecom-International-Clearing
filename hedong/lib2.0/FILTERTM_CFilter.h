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
const char  COMMIT_FLAG[]      = "commit.flg";        //added by tanj ���ӱ�־�ļ�

const long  FILTERTM_INDEX_LEN          =  3000;
const long  FILTERFM_TIME_LEN           =  15;
const long  FILTERTM_FILENAME_LEN       =  25;   //�����ļ����Ĵ�С
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
*���ṹ�Ǵ洢���ļ��е����ݽṹ
***************************************************************/
struct FILTERTM_SIndexTableInFile
{
	char fileName[FILTERTM_FILENAME_LEN];   //�����ļ����ļ���
	long indexSize;                //added by tanj 20060213ȥ�������ĳ��ȣ�����ǰ���long�����ͺ����bool��
	long indexInBlock;             //added by tanj 20060213ÿ�����ݿ��еļ�¼�ĸ���	
	long blockInFile;              //�����ļ������洢�����ݿ���
	long emptyHead;                //�տ�����ͷ�����ڴ������ļ���ɾ������

	long tableItem[INDEX_TABLE_SIZE];          //���������
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
*���������ṹ�����������ļ����ļ�ͷ�ṹ��һ��unsigned long����
*�����ļ����ļ�ͷ�����������ļ������ļ��еĿ�����
*unsigned long ����洢��ÿһ��Ļ������������ļ��еĿ��,����һ
*������Ϊ60*60��long�͵����飬һ����ÿһ���ӵĻ���������Ӧһ����
*���ͱ������˱�����ʾ�û����������ڵĿ�Ŀ��
****************************************************************/
struct FILTERTM_SIndexTable
{
	Modify modified;
	long forward;         //˫�������ͷָ��
	long backward;        //˫�������βָ�룬ָ�����δʹ�õ����������
	char m_szSourceId[10];     //��ǰ����Դ  added by tanj 20051114
	FILTERTM_SIndexTableInFile tableData; 
};

/****************************************************************
*�������������࣬�������ɸ�FILTERTM_SIndexTable�ṹ�����ڱ�Ҫ��ʱ��
*�������δʹ�õĽṹ�û����ڴ�
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
	  long head;    //ͷָ��ָ��ǰ�����������
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
//��ʱ�ļ������ݿ����������blockindex.tmp�У�������ռ�õĿռ�Ƚ�С�����Ա�����
//��������ʱ�ļ��е����ݿ�
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
    
    //added by tanj 20061020  �����Զ���ıȶԺ�����index���бȽϣ�����little-endian��CPU�������
    int CompareIndex(const char *index1,const char *index2);
    
    FILTERTM_SBlockHead *getBlockHead();
	  Modify modified;
		
		void clear();                //added by tanjie 20051121
	protected:
	  //blockHead �� blockData��ָ��������ǽ�Ҫд������е�����
	  FILTERTM_SBlockHead blockHead;	
	  char *blockData;
	  char fileName[FILTERTM_FILENAME_LEN];
	  long blockNo;        //�����������ļ��еĿ��
	  long indexSize;
	  long maxIndex;
	  long blockSize;
	  char m_szSourceId[10];      //�����ݿ�����������Դ��ʾ added by tanj ��Ϊ������Դ
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
    void clear();     //added by tanj 20051121   ����ڴ�,��ʱ�ļ��е���������
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
	  char m_szSourceId[10];       //��ǰ�漰������Դ added by tanj 20051114
	  //map<string,string>  m_AllSource    //������Ƶ�������Դ added by tanj 20051114
};


void openIndexFile(fstream &file_Stream, const char *file_Name);
void openTempFile(fstream &file_Stream, const char *file_Name);
//int  chkDir( char *_dir );
void truncFile(const char file_Name[FILTERTM_FILENAME_LEN]);
void truncIndexFile(const char *file_Name);
bool findIndexFile(const char *file_Name);




#endif