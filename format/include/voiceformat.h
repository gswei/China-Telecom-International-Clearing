#ifndef _voiceformat_H_
#define _voiceformat_H_ 1

//#include "CF_COracleDB.h"
#include "CF_CFmtChange.h"
#include "CF_Common.h"


#define   LEAF_FLAG                       'L'
#define   OPRATION_FLAG                  'O'
#define   FIRSTNODE                       "11"
#define EXP_SPLIT    ';'       //多个表达式时的分隔符
#define EXP_ERR_SPLIT    ':'       //表达式与错误类型之间的分隔符

#define EXP_STR_LEN    512       //多个表达式时的分隔符

void expTrace(const char *szDebugFlag, const char *szFileName,
  int lineno, const char *msg, ...);
void delSpace(char *ss, int ss_len);


struct BILLSFC_GROUP
{
  int iIndex;
  char szValue[64];
};
struct BILLSFC_RCDFmt
{
  int iGroup;
  vector<BILLSFC_GROUP> mBillSFC_Group;
};
struct BILLSTA_FILT_COND
{
  char szinRcdFmt[6];
  vector<BILLSFC_RCDFmt> mBillSFC_RcdFmt;
};
struct STxtSign_node
{
  char szPreSignValue[11];
  char szNodeId[6];
  int   iNodePri;
  char chNodeType;
  int   iNodeLen;

  char chRecordType;
  int   iSign_Index;
  int   iSign_Mode;
  int   iSign_Begin;
  int   iSign_Len;
  char szSeperator[101];

  char szExpression[EXP_STR_LEN];
  int   iNextSign_Num;
  STxtSign_node *pNext_Node;
};
struct STxtRecordFmtDefine
{
  int   iRecord_Len;
  char szRecord_Name[6];
  char szExpression[EXP_STR_LEN];
  char szExp[10][EXP_STR_LEN];
  int iExp_Errtype_Index[10];
  int   szExp_Num;

};
struct STxtFileFmtDefine
{
  char szFile_Fmt_Name[6];
  char chRecordType;
  int   iSign_Index;
  int   iSign_Mode;
  int   iSign_Begin;
  int   iSign_Len;
  char szSeperator[101];
  int   iTxtRecordFmt_Num;
  struct STxtRecordFmtDefine *pCur_TxtRecordFmt;
  struct STxtRecordFmtDefine *pTxtRecordFmt;
  int   iNextSign_Num;
  STxtSign_node *pNext_Node;
  int   iRoute_Num;
  struct SCheckRoute *pCheckRoute;
};

struct SInput2Output
{
char szInputType[6];
char szOutputType[6];
int iCount;
int *pInputIndex;
int *pOutputIndex;
int *pOutIdx_Index;
BILLSFC_GROUP *pszInputValue;
int *psubstrInputlen;
};
struct STxtFmt
{
char szFileTypeID[6];
char chRecordType;
};

struct SCheckRoute
{
char szCalledNo_Header[11];
char szRoute[6];
char szOutputRoute[6];
};


class CFormat
{
public:
  CFormat();
  ~CFormat();
  //int LoadFmtDataToMem(CDatabase *,char *,char *,char,char *SourceGrp);
  int LoadFmtDataToMem(DBConnection,char *,char *,char,char *SourceGrp);
  char Get_Txtfmt_RecordType(char *);
  int Set_OutRcd(CFmt_Change &,CFmt_Change &,vector<string> file_head,vector<string> file_tail);
  SInput2Output* GetCurInput2Output(char *szInType,char *szOutType);
  STxtFileFmtDefine* Get_CurTxtFileFmt(char *);
  STxtRecordFmtDefine* Get_CurTxtRecordFmt(char* szRecord);

  int CheckBillSFC(CFmt_Change &inrcd);

  int  iSign2_Begin;
  int  iSign2_Len;
  int  iSign2_Value;
  char SHQQRcdType;
  int SHQQCalledNoIdx;

private:
  void DelNodeSpace(STxtSign_node *&NNode,int Num);
  int LoadTxtFileFmt(char *,char *SourceGrp);
 int LoadTxtSignNode(char *File_Fmt,char *szNodeId,int &iSonNode_Num,STxtSign_node *&pSonNode);
 int LoadTxtRoute(int iSeq);
 int LoadTxtRecordFmt(int );
  int LoadTxt_Fmt(char *SourceGrp);
  int LoadInput2Output();
  int GetBillCond();
  int GetBillSFC_RcdFmt(BILLSTA_FILT_COND &tmpBCond);
  STxtSign_node* Get_TxtNode(char *szRecord,int iSign_Num,STxtSign_node *pNode,char *iSign);


  STxtFileFmtDefine *pTxtFileFmtDefine;
  STxtFileFmtDefine *pCur_TxtFileFmtDefine;
  int            iTxtFileFmtFefine_Num;

  STxtFmt       *pTxtFmt;
  STxtFmt       *pCur_TxtFmt;
  int            iTxtFmt_Num;
  SInput2Output  *pInput2Output;
  SInput2Output  *pCur_Input2Output;
  int            iInput2Output_Num;
  char          szDebug_Flag[50];
  char          szLogStr[400];
  int            iFileFmt_Index;
  //CDatabase *m_DataBase;
  DBConnection conn;//数据库连接

  vector<BILLSTA_FILT_COND> mbCond;
  int iBCondNum;
  int iBCond_CurIndex;
};
static void Get_TxtSign_Value(char *szRecord,char chRecordType,int iSign_Index,int iSign_Mode,int iSign_Begin,
	int iSign_Len,char *szSeperator,char *szTxtSign_Value);

static void Sign_value(char *block_buff, char *sign_value,
                       int start_pos, int offset, int mode);
#endif
