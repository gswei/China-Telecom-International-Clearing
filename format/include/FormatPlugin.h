/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		FormatPlugin.h
	Description:	��������ϵͳ�ĸ�ʽ���ļ���
					��ʽ���ļ���д������

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2012/3/22	       ��ɳ������
		v2.0		hed		 2013/3/31	
	</table>
*******************************************************************/
//CALLNO_HEADER
//BILLTIME_BETWEEN_FILE
//FAKEHEADER

#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include <dirent.h> //_chdir() _getcwd() ��ȡ�ļ�Ŀ¼�ȣ�����ļ�����Ϣ

#include "process.h"
#include "Ipc.h"
#include "RTInfo.h"

#include "CF_CFscan.h"
#include "CF_CMemFileIO.h"
#include "CF_CFmtChange.h"
#include "CF_CRecordChange.h"
//#include "psutil.h"
#include "CF_CPlugin.h"
#include "voiceformat.h"
#include "bill_statistic.h"
#include "bill_statistic_route.h"
#include "bill_statistic_filenametime.h"
#include "CF_Comp_Exp.h"
#include "formatcheck.h"
#include "CF_Common.h"
#include "CF_CFmtChange.h"
#include "CF_CLogger.h"
#include "CF_CHash.h"

//#include "dr_api.h"
#include "dr_deal.h"

#define	INROUTE_NAME				"InRoute"
#define	OUTROUTE_NAME				"OutRoute"
#define	RECORD_NAME				    "Record"
#define	ORARECORD_NAME				"OraRecord"
#define	ERR_SELECT_NONE				4007	//���ݿ����Ҳ�����¼
#define	ERR_GET_RECORDFMT			4008	//��ȡ��ǰ��¼��ʽ����
#define  ERR_FILEFMT               4009   //�ļ������Ҳ���
#define  ERR_ANNFILE               4010   //ANN_commit����
#define	ERR_UPDATE_STATICS			4004	//ͳ�ƻ�����������
#define	ERR_GET_MsgDirection		4005	//��ȡ�����������

#define CHECK_SUCCESS     1
#define CHECK_FAIL		  0

//using namespace ipc;
//using namespace tpss;

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

struct SOURCECFG
{
  char szSourceId[6];
  char szFile_Fmt[6];
  char szInFileFmt[6];
  char szSourcePath[256];
  char szTollCode[16];
  int FieldIndex[20];		//ѹ���ֶ�
  char Fmt_Time_fmt[30];
  char szZDate_Fmt[16];
  char szzTime_Fmt[16];

  char serverCatID[6];		//2013-09-09

  char filterRule[50] ;  //���˹���add by hed  2013-03-12
  int  file_begin;		  //��ȡ�ļ��������ʱ��
  int  file_length;

  int  iMsgDirBegin;
  char  FnSep;
  int   FnIndex;
  int   FnBegin;
  int   FnLen;

  char FntSep;
  int   FntIndex;
  int   FntBegin;
  int   FntLen;
  
  char chIs_Bill_Statics;
  char chIs_Bill_Statics_Route;
  char chIs_Bill_Statics_fnTime;
  char chIs_TimeFile;
  
  int iRcd_Arr_Dur;
  int iERcd_Arr_Dur;
};

//����ˮ��Ϊ��λ
struct SParameter
{
	char szSrcGrpID[6];							//����Դ��
	char szService[6];							//serviceID
	char  iWorkflowId[10];						//������ģ��ID  
	int  iflowID;								//��ˮ��ID
	int  iModuleId;								//ģ���ʶ
	char  iInputId[10];							//����ӿ�ID
	char  iOutputId[10];						//����ӿ�ID

	char szOutputFiletypeId[6];					//����ļ�����
	char szSchCtlTabname[32+1];					//���ȱ��

	char szInPath[JS_MAX_FILEPATH_LEN];			//����·��(���·����	
	char szOutPath[JS_MAX_FILEPATH_LEN];		//���·��(���·����
	char szBakPath[JS_MAX_FILEPATH_LEN];		//����·��
	char szErroPath[JS_MAX_FILEPATH_LEN];		//�����ļ�·��
	char szTimeoutPath[JS_MAX_FILEPATH_LEN];	//����·��(���·����
	char szOtherPath[JS_MAX_FILEPATH_LEN];		//����·��(���·����
	char bak_flag ;
		
	int source_file_num ;						//ɨ������Դ��������	 
	int szSleepTime;							//ÿ������ʱ��

	//char szDebugFlag[50];						//�Ƿ����������־

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;
		source_file_num = 0;
		szSleepTime = 5;
		bak_flag = 'Y';							//Ĭ�ϲ�����

		memset(iWorkflowId,0,sizeof(iWorkflowId));
		memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));
		
		memset(iInputId,0,sizeof(iInputId));
		memset(iOutputId,0,sizeof(iOutputId));

		memset(szInPath,0,sizeof(szInPath));
		memset(szOutPath,0,sizeof(szOutPath));
		memset(szErroPath,0,sizeof(szErroPath));
		memset(szBakPath,0,sizeof(szBakPath));	
		memset(szTimeoutPath,0,sizeof(szTimeoutPath));
		memset(szOtherPath,0,sizeof(szOtherPath));
	}
};

/*
struct DRparameter
{
	 bool m_enable ;						//����״̬
	 int  drStatus;							//ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	 char m_SerialString[4096];				//ͬ����
	 char m_AuditMsg[4096];					//�ٲô�

	 DRparameter()
	 {
		m_enable = false;
		drStatus = 2;
		memset(m_SerialString,0,sizeof(m_SerialString));
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
	 }
};
*/

//У���ļ��ĸ�ʽ
struct FileNameFmt
{
	int number;     //У�����
	int index;		//У������λ��
	int len;		//У���ֶγ���
	char check_type;  //У��ֵ����
	char check_value[50];  //У��Ĭ��ֵ
	char err_code[6];
};

//У���ļ���¼��ͷβ
struct RecordHTFmt
{
	int  number;
	char name[50];
	int index;
	int len;
	char seperator[2];			//�ֶεķָ���
	char check_type;			//У��ֵ����
	char check_value[50];		//У��Ĭ��ֵ
	char ht_flag;				//ͷβ��־
	char spec_flag[20];			//2013-07-26,����ȥ��������
	char err_code[6];
};

//2014-07-01
struct C_ADJ_CYCEL_DEF
{
	int monthnum;				//�ɵ��������ڼ�� Ĭ��Ϊ 1
};

class FormatPlugin:public PS_Process,public BasePlugin
{
	//CDatabase *m_DBConn;
	DBConnection conn;//���ݿ�����
	Statement stmt;
//	CLogger *m_Logger;

	char m_szSrcGrpID[8];
	char m_szService[8];
	char m_szSourceID[8];
	char mServCatId[6];
	int iProcIndex;
	char m_szFileName[256];		//ԭʼ�ļ���
	char m_szOutTypeId[8];
	char szDebugFlag[2];
	char fileName[512] ;		//ȫ·���ļ���
	char outFileName[512];		//����ļ���

	int Redo_Begin;
	char Redo_What[8];
	int iErrorBack_Flag;

  	C_BillStat_Route Bill_Route;
  	C_BillStat_fnTime Bill_fnTime;
  	C_BillStat Bill;

	CFormat m_txtformat;
	CFormatCheck m_formatcheck;
	STxtFileFmtDefine *pCur_TxtFmtDefine;
	SInput2Output  *pCur_Input2Output;

	map<string,SOURCECFG> m_SourceCfg;
	map<string,SOURCECFG>::iterator mCur_SourceCfg;
	
	char m_chBillSta_Filter_Cond;
	char m_chIs_Ann;
	char szAnn_Dir[32];
	char szLastAnn_Dir[256];
	char szLastAnn_Abbr[8];
	

	CFmt_Change outrcd;
	int iRecord_Index;
	int iOraRcdIdx;

	Txt_Fmt OTxt_Fmt;
	vector<Comp_Exp> oComp_Exp;
  	int iComp_Exp_Num;

  	char szEarliestTime[15];
	char szLatestTime[15];

	int iTotalNum;
	int iRightNum;
	int iLackNum;
	int iErrorNum;
	int iPickNum;
	int iOtherNum;
	char trans_table[256][2]; /*BCD-ASCII table*/
	char trans_tablex[256][2];/*HEX-ASCII table*/
	
	map<string,SOURCECFG>::const_iterator it ;
	
	SParameter mConfParam;
	//DRparameter mdrParam;
	DR_Deal	 mdrDeal;

	//char erro_path[256];       //�����ļ����·��
	//char input_path[256];     //�ļ��������·��
	//char output_path[256];       //�ļ�������·��  2013-06-19
	//char timeout_path[256];   //��ʱ�ļ�·��
	//char other_path[256];     //��ʽ��δ�����ʽ��������otherformat�ļ�
	
	CF_CFscan scan;
	PacketParser *pps;
	ResParser *res;
	
	vector<string> file_head;
	vector<string> file_tail;
	vector<string> m_vsql;
	vector<string> record_array;
	
	map< string,vector<FileNameFmt> > mapFileNameFmt;
	map< string,vector<RecordHTFmt> > mapFileRecordHeadFmt;
	map< string,vector<RecordHTFmt> > mapFileRecordTailFmt;
	
	map<string,C_ADJ_CYCEL_DEF> mapAdjCycleBusi;

	char currTimeA[15];
	char currTimeB[15];
	char rate_cycle[15];				//2013-07-26 ȡ���ڵ���������
	char rate_cycle_org[15];				//2014-07-02 ȡ���ڵ���������
	char file_time[15];					//ȥ�ļ����ϵ�ʱ�������������ȥ
	char err_code[10];					//������ 2013-07-19
	char sql[JS_MAX_SQL_LEN]; 
	char erro_msg[JS_MAX_ERRMSG_LEN];	//������Ϣ�Ǽ���Ϣ
	
	long file_id ;
	int  record_num;
	//int  source_file_num ;		//ɨ������Դ�������� 
	short petri_status;			//��ȡpetri��״̬ 
	short petri_status_tmp;

private:
	int LoadSourceCfg();
	int InsTimeBetweenFile(char *SourceId,char *filename,char *EarlyTime,char *LastTime);
	void Build_trans_table();
	int transfer( char *block_buff, int len,char *szRtn,int flag = 1);
	void Route_Change(STxtFileFmtDefine *pTmp,CFmt_Change *inrcd);
	void Get_TxtCalledNo(CFmt_Change *inrcd);
	int GetStrFromFN(char *Res,char *Fn,char Spl,int Index,int Begin,int Len);
	int getFromSrcEnv( char *Value, char *Name, char *SourceId,char *szService);
	int SetAbnOutCdr(pluginAnaResult anaType,int iErrorType,CFmt_Change *inrcd,PacketParser& ps,ResParser& retValue);
	int initflag;

	int getSourceFilter(char* source,char* filter,int &index,int &length);  //add by hed  2013-03-12
public:
	FormatPlugin();		//{	initflag=0;m_bSuspSig=false;childNum = 0;}
	~FormatPlugin();	//{ m_Shm = NULL; m_pArgv=NULL;m_pTktBlkBase=NULL;m_DBConn=NULL;pCur_TxtFmtDefine=NULL;pCur_Input2Output=NULL; }
	void init(char *szSourceGroupID, char *szServiceID, int index);	
	void execute(PacketParser& ps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName(){return "FormatPlugin";}
	std::string getPluginVersion(){return "3.0.0";}
	void printMe(){}

	bool init(int argc,char** argv);
	void run();
	int dealFile();					  //�����ļ�
	int dealAuditResult(int result);  //�����ٲ�
	int writeFile(char* fileName);
	int updateDB();					  //���ݿ�ĸ��� ��ɾ��
	void prcExit();
	
	int getmapAdjCycleBusi(string source); 

	int getFileNameFmt(string source);
	int getRecordHTFilter(char* fileType);	
	int checkFileRepeat(char* file);      //У���ļ����Ƿ��ظ�

	int checkFileName(string source,char* fileName);		  //У���ļ����ĸ�ʽ�Ƿ���ȷ
	int checkRecorHT(char* fileType,char* head,char* tail);	  //У���ļ���¼ͷβ�Ƿ���ȷ
	int checkRule(RecordHTFmt fmt,char* column_value);
	int checkDate(char* date);

	//bool drInit();
	//bool CheckTriggerFile();
	//int  drVarGetSet(char* m_SerialString);
	//int IsAuditSuccess(const char* dealresult);

};


