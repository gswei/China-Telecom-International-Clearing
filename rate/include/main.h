/****************************************************************
filename: main.h
module:
created by: lij
create date: 
version: 3.0.0
description:
   ���۳���
*****************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_ 1
#include "C_BillRate.h"
#include "CF_CInterpreter.h"
//#include "CF_COracleDB.h"
#include "CF_CPluginPacket.h"
#include "CF_Config.h"

//#include "CF_CPluginengine.h"
#include "CF_CPlugin.h"
#include "CF_CMessage.h"
#include "CF_CMemFileIO.h"
#include "CF_CFmtChange.h"

#include "psutil.h"
#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>


////////////////////////////////////////
#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h> 
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <map>
#include <signal.h>

#include "CF_CMemFileIO.h"
#include "CF_Cerrcode.h"
#include "CF_CLogger.h"
#include "CF_Common.h"
#include "CF_CProcessMonitor.h"
#include "CF_CInterrupt.h"
#include "CF_CMessagePPQ.h"
//#include "DealFile.h"
//#include "PrepCommon.h"
//#include "VersionInfo.h"
//#include "AccessMem.h"

#include "process.h"
#include "CF_CPkgBlock.h"
#include "psutil.h"
#include "settlerule.h"
//#include "dr_api.h"
#include "RTInfo.h"
//#include "MdrNodeApi.h"
//#include "PrepStructDef.h"
//add by weixy 20080530
//#include "C_SourcePriority.h"
//int grepAndSortFile(map<string, int> &sourcePriority, SFileStruct * sfile, int &fileNum);
//end add by weixy 20080530

class C_MainFlow: public PS_Process
{
private:
	char m_szExeName[PATH_NAME_LEN+1];
	char m_szExePath[PATH_NAME_LEN+1];
	char m_szEnvPath[PATH_NAME_LEN+1];
	char m_szEnvFile[FILE_NAME_LEN+1];
	char m_szLastSource[MAXLENGTH+1];
	char m_szLastDay[8+1];
	
	/* ��Ҫ�󶨵����ʽ�ϵı��� */
	char m_szSourceId[MAXLENGTH+1];
	char m_szLocalTollcode[MAXLENGTH+1];
	char m_szSysTime[MAXLENGTH+1];
	char m_szFileName[MAXLENGTH+1];
	char m_szNewName[MAXLENGTH+1];
	char m_szReceiveTime[14+1];

    char m_szOutFileName[MAXLENGTH+1];
	char m_szinFilePath[PATH_NAME_LEN+1];
	char m_szoutFilePath[PATH_NAME_LEN+1];
	char m_szinTableName[PATH_NAME_LEN+1];
	char m_szoutTableName[PATH_NAME_LEN+1];

	CF_CFscan scan;   //ɨ���ļ�Ŀ¼
	SParameter Param;			//�洢������dealfile���������Ľṹ��
	//CMessagePPQ respMsgHandle;
	//vector<SFileStruct> vecFile;
	//vector<SFileStruct> nextBatch;
	//map<string, SSourceStruct> mapSource;
	//Interpreter Compile;			//���ʽ�������ļ������
	BaseAccessMem *memManager;	//�����ڴ����
	//C_AccessMem *memManager;	//�����ڴ����
	//filterchain::FilterChain *chain;
	PacketParser *pps;
	ResParser *res;
	SListSql* pListSql;			//�洢list_sql��ĸ��±�־
	int iListCount;
	int iUndoFlag;
	int iRad;
	int iRunTime;
	bool m_bDBLinkError;
    int process_index ;  
    long billing_line_id;
    int  procID;
    char sourceID[20];
           
    char ext_param[50];
    char servCat[10];   
    //char jobid[3];
    char billmonth[7];
    

    //SSourceStruct source;//�Ӻ���getSourceInfo�Ƶ�ȫ��
    int path_num; //����Դ����
    ServCatCol *source_path;
    map<string,string> sourcemes;    

    char szFileName[MAXLENGTH+1];
    char szSourceId[MAXLENGTH+1];
    //char szFileName[MAXLENGTH+1];
    //char szSourceId[MAXLENGTH+1];
    void *block_addr; //�������׵�ַ
    //vector<char *> Classify1; //�ּ𻰵�
    //vector<char *> Classify3; //�ּ𻰵�
    //C_DealCalculator m_dealRcdNum;
    pid_t _iMainPid;    
    char m_szBeginTime[14+1];// �ļ���ʼ����ʱ��
    int tmp_childnum; //�ӽ��̵���Ŀ
    int record_num;  //��¼����

    int ruleid;
	std::vector<string> filenames;    
    
public:
	C_MainFlow();
	~C_MainFlow();
	void printVersion();
	bool checkArg(int argc, char** argv);
	bool init(int argc, char **argv);
	bool dealFile(STparam &szstparam);
	bool dealTable(STparam &szstparam);
	//bool dealTable_old(STparam &szstparam);
	bool dealRecord(STparam &szstparam);
	void getSourceInfo();
	bool insertTableData(char *sql);
	void getSql(char *sqltmp,char *tableitem,char * result,char *tablename);
	//char *getSql(char **sqltmp);
	void string_replace(string & strBig, const string & strsrc, const string &strdst) ;
	int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr);
	void insertLog(char *source_id,char *dealflag);
	bool dealAll();

	DBConnection conn;//���ݿ�����
	Statement m_stmt ;
	//DBConnection conn2;//���ݿ�����
	//DBConnection conn3;//���ݿ�����
	STparam stparam[20];
	char source_group[10];
	int source_count; //��Ҫ������source����
	char jobid[20];
    //CDatabase _DBConn[10]; //�ӽ���  
};

void dealSignal(int sig);
char* rtrim_lc(char* s) ;

#endif


