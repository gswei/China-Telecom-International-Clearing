#ifndef _CFRECEIVE_H_
#define _CFRECEIVE_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "Common.h"
#include "Log.h"
#include "CF_CFscan.h"
#include "wrlog.h"
#include "COracleDB.h"
#include "CFmt_Change.h"
#include "CF_CError.h"
#include "interrupt.h"
#include "CF_InfoPoint.h"
#include "ProcessMonitor.h"
#include "CF_CDealLog.h"
#include "infolog.h"

#define a 100
#define FILEREC_FILE_REPEAT          10
#define FILEREC_SCH_REPEAT           20
#define FILEREC_MAX_COLNUM           20
#define FILEREC_FILEPATH_LEN         255
#define FILEREC_FILENAME_LEN         255
#define FILEREC_WAITING_TIME_ERR     10//连接数据库失败时，系统休眠的时间；
//#define FILEREC_REC_WAITING_TIME   60//判断文件是否传输完全等待的时间；
#define FILEREC_DATE_TIME_SIZE       15
#define FILEREC_INIT_FILE_COUNT      10000//一次扫描的最大文件数；
#define FILEREC_ERR_GET_ENV          3001//获取环境变量出错
#define FILEREC_ERR_SCAN_RECDIR      3002//扫描接收目录出错
#define FILEREC_ERR_APPLY_MEMORY     3003//申请共享内存出错
#define FILEREC_ERR_OPEN_FILE        3004//打开文件出错
#define FILEREC_ERR_COMMAND          3005//系统调用SH命令出错
#define FILEREC_ERR_RACING           3006//系统空转告警
#define FILEREC_ERR_LARGE_SMALL_FILE 3007//超大超小文件告警
#define FILEREC_ERR_RENAME           3008//移动文件出错
#define FILEREC_ERR_COPY             3009//拷贝文件出错
#define FILEREC_ERR_INSERT_SCH       3010
#define FILEREC_ERR_SYSTEM           3011//系统未知错误
#define FILEREC_ERR_CHANGENAME       3012//文件改名出错
#define FILEREC_ERR_GETCOL           3013//获取原始文件名上的信息出错
#define FILEREC_ERR_QC_INIT          3014//qc格式错误
#define FILEREC_ERR_ORGNAME          3015//输入原始文件名错误
#define FILEREC_ERR_UNLINK           3016//删除文件出错
#define FILEREC_ERR_CONNECT_DB       3017
#define FILEREC_CHECK_QCTYPE         3018
#define FILEREC_ERR_CYCLEOUT         3019
#define FILEREC_REPEAT_FILE          101
#define FILEREC_NORMAL_FILE          102
#define FILEREC_DROP_FILE            103
#define FILEREC_MANY_QFILE           104

#define REC_FILE_REPEAT              601
#define REC_FILE_ERR                 602
#define REC_FILE_DROP                603
#define REC_TIME_NOT_ENOUGH          604

#define SUCC 0
#define FAIL -1


typedef struct COL_INFO
{
	int iType;
	char sz_colBuff[50];
	char c_colFlag;
	int iStartPos;
	int iEndPos;
	int iColLen;
	char c_separate;
	COL_INFO()
	{
		iType=-1;
		sz_colBuff[0]=0;
		c_colFlag=' ';
		iStartPos=-1;
		iEndPos=-1;
		c_separate=' ';
		iColLen=0;
		
	}
};

typedef struct FILE_INFO
{
  char  sz_orgFname[FILEREC_FILENAME_LEN];
  long  lCreateTime;
  int   iFileSize;
  int iFlag;
  FILE_INFO() 
  {
    memset(sz_orgFname,0,FILEREC_FILENAME_LEN);
    lCreateTime=0;
    iFlag=0;
  }
};

typedef struct SOURCE_INFO
{
    char  sz_sourceId[5+1];
    char  sz_sourcePath[FILEREC_FILEPATH_LEN];
    char  sz_fileFiter[200];
    char  c_QcFlag,c_SerialFlag;
    char  sz_QcType[20];
    char  sz_QCFiter[200];
    int  iSerial;
    char sz_nowday[8+1];
    int  iRereceiveDay;
    int  iLargeFsize;
    int  iSmallFsize;
    int  iRacing;
    int iColNum;
    int iMaxSerialValue;
    struct COL_INFO  *sFileNameList;

    SOURCE_INFO()
    {
    	memset(sz_sourceId,0,6);
    	memset(sz_sourcePath,0,FILEREC_FILEPATH_LEN);
      iSerial=0;
      iRacing=0;
      c_QcFlag='N';
      c_SerialFlag='N';
      sFileNameList=new struct COL_INFO[FILEREC_MAX_COLNUM];
    }
};

typedef struct QC_INFO
{
	char sz_QcType[20];
	char c_messA;
	int iNum[5];
	int iRecord[5][20];
	QC_INFO()
	{
		for(int i=0;i<5;i++)
		 iNum[i]=0;
	}
};

extern int _CmpCreateTime(const void *p1, const void *p2);
extern int _CmpFileSize(const void *p1, const void *p2);
extern int _CmpFileName(const void *p1, const void *p2);


class CFreceive
{
	private:
	  int iWorkFlow,iProcessId,iCountMax,iSourceNum,iSleepTime,iBatchTime,iBatchVal;
	  //int iLargeFile,iSmallFile,iRereceiveDay;
	  int iRacingTime;
	  char c_CheckFile;
	  char sz_pipeId[5+1],sz_sourcePath[FILEREC_FILEPATH_LEN],sz_DebugFlag[10];
	  char sz_outControlTable[30];
	  char sz_infoPoint[20];
	  char sz_fileInPath[FILEREC_FILEPATH_LEN],sz_fileOutPath[FILEREC_FILEPATH_LEN];
	  char sz_fileErrPath[FILEREC_FILEPATH_LEN],sz_fileBakPath[FILEREC_FILEPATH_LEN];
	  char sz_shFile[255];
	  char sz_logFile[FILEREC_FILENAME_LEN];
	  char sz_errtime[14+1],sz_errmsg[400];
	  char c_FileType,c_BatchFlag,c_QCFlag,c_UzipFlag,c_BakFlag,c_SerialFlag;
	  char c_OrgBatchFlag,c_OrgSourceFlag,c_OrgColDateFlag,c_OrgNameFlag,c_BillCycleFlag,c_DropTimeFlag,cSortFlag;
	  char c_CompressTimeFlag,c_LargeFlag,c_SmallFlag,c_ReceiveFlag,c_QcDeleteFlag,c_bakFrontFlag,c_copyFlag,c_serialTimeFlag;
	  char c_SourceEnvFlag;
	  char sz_bakType[100],sz_fileNameBuff[255];
	  char cQcBakOnlyFlag;
	  char sz_cycleTime[14+1],sz_dropTime[14+1];
	  char c_QCtableFlag;
	  char c_renameFlag;
	  int iQCDropTime,iBillCycle;
	  int iColNum,iSourceNo,iQcTypeNum,iSerialLong,iMaxSerialValue,iSerialPos;
	  int iFileId;
	  int FILEREC_REC_WAITING_TIME;

	  CProcessMonitor monitor;
	  //CF_MemFileI FileReed;
	  CFmt_Change QCRecord;
	  CFmt_Change Filefmt;
	  CF_CDealLog DealLog;

	  struct FILE_INFO  *filelist;
	  struct SOURCE_INFO *sourcelist;
	  struct QC_INFO  *QCRecordlist;
    struct COL_INFO  *sourcecol;
    struct COL_INFO  *batchcol;
    struct COL_INFO  *datecol;
    struct COL_INFO  *bakdatecol;
    struct COL_INFO  *orgnamecol;
    struct COL_INFO  *billcyclecol;
    struct COL_INFO  *serialtimecol;

	protected:
	  int GetAllEnv();
	  int GetOraData();
	  int ScanFiles(char *,char *);
	  int FileTmpCheck(char *);
	  int GetBatchId(char *,int &);
	  int GetFileSize(char *);  
	  int GetCol(char const *,struct COL_INFO e,char *);
	  int GetFileName(char *,char *,char *,int ,int,char *,SOURCE_INFO);
	  int ParseFileName(char *,struct COL_INFO *,int &,int);
	  int GetRunFlag(int &);
    int SplitBuf(char *,char ,char pch_Var[20][100]);
    void MoveToErrPath(struct SOURCE_INFO &,char  *,char *);

	private:
	  int InsertRepeatFile(char *,char *,char *);
	  int InsertQcReceived(char *,char *);
	  int InsertFileReceive(char*,char*,char *,int,long,char *,char *,char *,char);
	  int InsertFileReceive(char *,char *,char *,int,long ,char *,char *,char *,char *,char ,char );
	  int UpdateErrFileReceive(char *,char *);
	  int UpdateSerialInfo(char *,char *,int);
	  int GetMaxSerial(char *,char *);

	  int InsertSchFormat(char *,char *);
	  int CheckFileInFileReceive(char *,char *);
	  int LDTCheckFileInFileReceive(char *,char *,char *,char *,char);
    int SingleCheckFileInFileReceive(char *,char *);
    int InsertRecollectInfo(char *,char *,char *,char *);
    int CheckRecollectInfo(char *,char *,char *);
    int UpdateRecollectInfo(char *,char *,char *,char *);
	  int UpdateFileReceive(char *,char *,char *,int,char *,char,int);
    int SelectBillCycle(char *,int &);

	  int ProcFile(struct SOURCE_INFO &,char *);
	  int ProcFileQC(struct SOURCE_INFO &,char *);
	  int InsertQCToFileRec(struct SOURCE_INFO &,char *,int);

	public:
	  CFreceive();
	  ~CFreceive();
    int Init(char *,char *,char *,char *);
	  int Proc();
	  void SleepSender();
	  int AAA();
};

#endif;