
#ifndef _formatcheck_H_
#define _formatcheck_H_ 1

//#include "CF_COracleDB.h"
#include "CF_Common.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"


#define YEARSTRING    "YYYY"
#define YEARSTRING2    "YY"
#define MONTHSTRING  "MM"
#define DAYSTRING     "DD"
#define HOURSTRING    "hh"
#define MINUSTRING    "mm"
#define SECSTRING      "ss"
#define SECSTRING2      "s"  //6秒

const char CHECK_VALUE_TYPE_CH='V';
const char CHECK_LENTH_TYPE_CH='L';










const char CALLINGCHECK_CH='A';
const char CALLINGCHECKa_CH='a';  //右对齐

const char CALLEDCHECK_CH='B';  
const char CALLEDCHECKb_CH='b';  //右对齐
const char CALLED30CHECK_CH='J';  
const char CALLED30CHECKb_CH='j';  //右对齐


const char CTIMECHECK_CH='C';
const char cTIMECHECK_CH='c';
const char DTIMECHECK_CH='D';
const char dTIMECHECK_CH='t';
const char sTIMECHECK_CH='k';

const char DURCHECK_CH='d';
const char ENUMCHECK_CH='e';

const char FREECHECK_CH='f';
const char HCHECK_CH='H';
const char INTCHECK_CH='I';
const char LENCHECK_CH='L';
const char FEECHECK_CH='M';
const char MDURCHECK_CH='m'; //毫秒时长校验，并转换为秒，采用进1方式

const char NUMCHECK_CH='N';
const char ROUTECHECK_CH='R';
const char NONCHECK_STATITC_CH='s';
const char NONCHECK_CH='S';
const char TELNUMCHECK_CH='T';
const char NULLCHECK_CH='U';

const char YCallIdxCHECK_CH='Y';//市化亲情被叫
const char yRcdTypeCHECK_CH='y';//市化亲情记录类型
const char ZDATECHECK_CH='Z';//校验日期YYYYMMDD
const char zTIMECHECK_CH='z';//校验时间(不包含日期) hhmmss
const char xCTRLCHECK_CH='x';//不可见字符切掉


void expTrace(const char *szDebugFlag, const char *szFileName,
  int lineno, const char *msg, ...);
void delSpace(char *ss, int ss_len);
int getCurDate(char *pchDate);
int chkDir( char *_dir );



struct SFile
{
char szBusi_Abbr[3];
char szFileName[256];
char TmpFileName[256];
int   iIs_Open;
CF_MemFileO Outfp;
char szdate[9];
};


struct SIpNo
{
char szIpNo[11];
char szBusi_Abbr[3];
};



struct SENUMCHK
{
  char szRcdFmt[6];
  int iFieldIndex;
  char szEnumValue[100][20];
  int iEnumValue_Num;
};




struct SCheckRule
{
char szSource_ID[6];
char chCheckFmt;
char chCheckType;
char szUp_Limit[21];
char szLow_Limit[21];
};

struct SFakeHeader
{
char szSource_ID[6];
char szFakeHeader[22];
char chDelFlag;
};

struct SCallnoHeader
{
char szCallnoHeader[7];
};





class CFormatCheck
{
public:
  CFormatCheck();
  ~CFormatCheck();
  //int LoadCheckDataToMem(CDatabase *,char *,char *,char );
  int LoadCheckDataToMem(DBConnection,char *,char *,char );
  int CheckField(char ,char *,char * ,int,char *szFile_Fmt,int iCheckLen);
  int Init_Ann_Dir(char *szAnn_Dir,char *szlocalnet,int iProcIndex);
  int Check_Ann_Flag(CFmt_Change &outrcd);
  int CommitAnnFile();
  int RollbackAnnFile();


	char szPreCalledNo[50];
	char szAftCalledNo[50];
	char szTmpCalledNo[50];
	int  iCheckRouteIndex;
	int iAnn_Index;
	int iCalledNo_Index;
	int iFake_Header_Flag;
	
	char szTime_Fmt[30];
	int iTime_FmtLen;
	char szzTime_Fmt[15];
	int izTime_FmtLen;
	char szZDate_Fmt[15];
	int iZDate_FmtLen;
    char SHQQRcdType;
	int SHQQCalledNoIdx;
	int iRcd_Arr_Dur;
	int iERcd_Arr_Dur;
  
private:
  int FHCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FCTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FcTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FsTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FDTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FdTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FNUMCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FTELNUMCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FLENCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FNULLCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FCALLINGCHECK_CH(char *str,char *szSourceID,int iCheckLen,char checkType=CALLINGCHECK_CH);
  int FCALLEDCHECK_CH(char *str,char *szSourceID,int iCheckLen,char checkType=CALLEDCHECK_CH);
  int FCALLED30CHECK_CH(char *str,char *szSourceID,int iCheckLen,char checkType=CALLED30CHECK_CH);
  int FDURCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FMDURCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FENUMCHECK_CH(char *str,char *szSourceID,int,char *szFile_Fmt);

  int FFEECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FFREECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FROUTECHECK_CH(char *str,int index);
  int FzTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FZDATECHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int FINTCHECK_CH(char *str,char *szSourceID,int iCheckLen);
  int Check_Rcd_Arr_Dur(char *nowertime, char *timeStr );




  bool CheckDate(const char *pchString);
  bool CheckTime(const char *pchString);
  int LoadCheckRule(char *);
  int splEnumValue( char *ss,SENUMCHK &TmpOut );
  int LoadCheckEnum();
  int LoadFakeHeader(char *);
  int LoadCallnoHeader();
  int LoadIpNo();
  
  SCheckRule *pCheckRule;
  int          iCheckRule_Num;
  SCheckRule *pCur_CheckRule;

  SFakeHeader *pFakeHeader;
  int          iFakeHeader_Num;
  SFakeHeader *pCur_FakeHeader;
  
  SCallnoHeader *pCallnoHeader;
  int          iCallnoHeader_Num;

//add by zhoulh 20070720
  SENUMCHK   *pENumChk;
  int iENumChk_num;
  SENUMCHK   *pCur_ENumChk;
//add by 20050627//Ann
  SFile         *pFile;
  int           iFile_Num;
  SIpNo        *pIpNo;
  int           iIpNo_Num;

  char          szDebug_Flag[50];
  char          szLogStr[400];
  //CDatabase *m_DataBase;
  DBConnection conn;//数据库连接

};



static void Remove_connector(char *str);


#endif

