#ifndef _MEMFILEIO_
#define _MEMFILEIO_ 10000

#include "COracleDB.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/mman.h>
#include <math.h>


#include "CFmt_Change.h"
#include "CF_CError.h"



#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#define FAIL							-1		/*ʧ��*/
#define SUCCESS							0		/*�ɹ�*/
#define READ_AT_END		608  		//�ļ���ȡ����β��
#define ERR_FOR_READ_END 608  		//�ļ���ȡ����β��

#define R 	101		//������
#define W 	102		//д����
#define RECLEN 5000	//CFmt_Change::Get_record(char*,RECLEN);
#define FILELEN 256
#define ERR_FILE_INIT 10000
#define EXECUTE_NUM  30


typedef struct CDRFileInfo
{
   char sz_sourceId[5+1];        /* ����ԴID, e.g. "GZ101" */ 
   char sz_begBillPeriod [8+1];  /* ���ļ���ʼ�Ʒ����� e.g. "20010701" */
   char sz_endBillPeriod[8+1];  /* ���ļ������Ʒ����� e.g. "20010701" */
   char sz_beginDate[14+1];      /* earliest date in this CDR file e.g. "20010701123030"*/
   char sz_endDate[14+1];       /* latest date in this CDR file e.g. "20010701123130" */
   long  iFileId;                /* �ļ���ţ�ռ8λ*/
   long  iFilesize;              /* �ļ��Ĵ�С��ռ10λ������Ҫ����*/
   int   iHotbill;                /* �Ƿ�Ϊ�ȼƷѡ���˵ĳЩ��ʽ����. 1 is yes, 0 is no��ռ1λ */
   long  iTotalCDR;              /*�ܹ���¼����ռ8λ*/
   long  iAvailCDR;              /*��Ч��¼����ռ8λ*/
   long  iInvalidCDR;            /* ��Ч��¼����ռ8λ*/
} CDRFileInfo;

typedef struct  MEMIO_FILE_FIELD_INFO
{
	char sz_fieldBuff[100];
	char sz_fieldValue[100]; 
	int  iFieldOffset;
	int  iFieldByte;
	int  iFieldInx;
	char cFieldFlag,cFieldCompare,cFieldMode,cHeAvail;
  
  MEMIO_FILE_FIELD_INFO()
  {
  	sz_fieldBuff[0]='\0';
  }
};

typedef struct TXT_FILE_FMT_INFO
{
	char sz_colName[50];
};

class CF_MemFile
{ 
  protected:
  	int file;
  	int RecCount;		//������¼ʱ�����ܼ�¼������д��¼ʱ����д��¼��
  	int tellCount;		//tell()
  	int iFileSize;
  	int iDBFlag;
  	char* smmap;		//smmap = mmap();
  	int iPos;//ָ���ƫ��λ��
  	char sch[RECLEN];
    int nd_num;
  	int iFieldCount;
  	int iFmtInitFlag;//0-����ҪInit;1-��ҪInit;2-�Ѿ�Init
  	int iHEcopyFlag;
  	int iTxtNum;//��¼�ֶ���
  	char sz_tableName[30];//��¼��ı���;
  	char sz_orderByBuff[100];
  	char sz_fileType[10];
    struct MEMIO_FILE_FIELD_INFO  *fieldList;
    struct TXT_FILE_FMT_INFO  *txtfmtList;
    CFmt_Change  classRecordType;
    CBindSQL Ds01;
    CBindSQL Ds02;
    
  protected:
    int iHeadTotalSize,iEndTotalSize;
    char sz_HeadPrefix[20],sz_EndPrefix[20];
    char c_Sizepos,c_Recordpos;
    int iSizeOffset,iRecordOffset;
    int iSizeByte,iRecordByte;
    char c_SizeFillsymbol,c_RecordFillsymbol;
    char c_Fillsymbol;
    char sz_fileName[255];
    char sz_errMsg[255];
  public:
    char sz_headBuff[400];
    char sz_endBuff[400];
    
  protected:
  	//int CheckFile();
  	int AllocMap(int flag);
  	int FreeMap();
  	int CheckFile(int& d,int& rec);
  	int delChar(char *,char,int);
  	int addChar(char *,char,int);
  	void Clear();

  public:
  	CF_MemFile();
    int Init(int,char *,int,char *,char,int,int,char,char ,int ,int ,char ,char);
  	int Init(const char *);
    int Init(const char *,char *pch_TableName,char* pch_OrderByBuff="\0");
    int GetPos();
    int GetFirstRecPos();
    int Get_recCount();
	  void setpos(int i);
	  int GetHead(char *,int );
	  int GetEnd(char *,int );
  	int get_num();
  	int CheckTotalNum(int iMaxTotalNum);
  	virtual ~CF_MemFile();
  	//virtual int Open(char* filename) = 0;
  	virtual int Close() = 0;
};


class CF_MemFileI : public CF_MemFile
{
  private:
    int iReadNum;
    int iRecordNo,iDealFlag;
  
  private:
	  //int CheckFile();
  public:
	  CF_MemFileI();
	  ~CF_MemFileI();
	  int Open(char* filename,int mode =0);
	  int Close();
	  int readRec(CFmt_Change& change);
	  int readRec(char *,int);
	  int operator>>(CFmt_Change& change);
	  

    
  private://���ݿ��д��֧��
    char sz_fieldListBuff[4000];
    int  iCheckFlag,iDelFlag;
    
	public:
	  int DbOpen(char *);
	  int DbReadRec(CFmt_Change&);
	  int DbClose();
	  char* getRecordNo();
	  
  public://�����ݿ��д���̵߳�֧��
	  int Open(char* filename,CBindSQL &,int mode =0);
    int readRec(CFmt_Change& change,CBindSQL &);
    int Close(CBindSQL &);
    int DbOpen(char *,CBindSQL &);
	  int DbReadRec(CFmt_Change&,CBindSQL &);
	  int DbClose(CBindSQL &);

};

class CF_MemFileO : public CF_MemFile
{
  private:
  	int process_header();
  	int process_end();
  	int InitFile();

  protected:
    int process_column(char* str);
    int CheckPos(int i);
  public:
  	CF_MemFileO();
  	~CF_MemFileO();
  	int Open(char* filename,int mode = 'W');
  	int Open(char*,int,int);
  	int Close();
  	int Close(int);
  	int writeRec(CFmt_Change& change,char *pch_cmpareBuff="\0");
  	int writeRec(char *);
  	int operator<<(CFmt_Change& change);
  	int setHeadEnd(CF_MemFileI &);
  	
  private://���ݿ��д��֧��
    char sz_fieldListBuff[1000];
    int  iCheckFlag;

	public:
	  int DbOpen(char *);
	  int DbWriteRec(CFmt_Change&,char *pch_cmpareBuff="\0");
	  int DbDeleteRec(char *pch_cmpareBuff);
	  int DbClose();
	  
  public://�����ݿ��д���̵߳�֧��
  	int writeRec(CFmt_Change&,CBindSQL &,char *pch_cmpareBuff="\0");
  	int Close(CBindSQL &);
	  int DbWriteRec(CFmt_Change&,CBindSQL &,char *pch_cmpareBuff="\0");
	  int DbDeleteRec(char *pch_cmpareBuff,CBindSQL &);
	  int DbClose(CBindSQL &);
};

class CF_MemFileLO:public CF_MemFileO
{
	private:

	public:
	  int Open(char *);
	  int setFileInfo(struct CDRFileInfo &);
	  int Close();
};

#endif
