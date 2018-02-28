/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		FileLoad.h
	Description:	��������ϵͳ���ļ�������
					
	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2012/6/21	       ��ɳ������
	
	</table>
*******************************************************************/

#include<iostream>
#include <vector>

#include "bill_process.h"
#include "CF_CPkgBlock.h"
#include "psutil.h"
#include "tp_log_code.h"
#include "RTInfo.h"

//#include "tp_log_code.h"
#include "CF_Common.h"
#include "CF_CFscan.h"
#include "CF_CLogger.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"

#include "dr_api.h"

using namespace std;
using namespace tpss;  //��psutil.h��Ӧ

/*
#define JS_MAX_SQL_LEN					1024
#define JS_MAX_FILENAME_LEN				256
#define JS_MAX_FILEPATH_LEN				256
#define JS_MAX_FILEFULLPATH_LEN			512
#define JS_MAX_ERRMSG_LEN				1024
#define JS_MAX_RECORD_LEN			    1024
*/

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

const char FILE_ID[20]="FileID";

//����Դ��Ϣ
struct SOURCECFG
{
	char szSourceId[6];							//����ԴID
	char szInFileFmt[6];						//�ļ���ʽ
	char szSourcePath[JS_MAX_FILEPATH_LEN];		//����Դ����·��
	char filterRule[50] ;						//����Դ�Ĺ��˹���
	char serverCatID[6];

	SOURCECFG()
	{
		memset(szSourceId,0,sizeof(szSourceId));
		memset(szInFileFmt,0,sizeof(szInFileFmt));
		memset(szSourcePath,0,sizeof(szSourcePath));
		memset(filterRule,0,sizeof(filterRule));
		memset(serverCatID,0,sizeof(serverCatID));
	}
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
	char szBakPath[JS_MAX_FILEPATH_LEN];		//����·��
	char szErroPath[JS_MAX_FILEPATH_LEN];		//�����ļ�·��
	char bak_flag ;
		
	int maxRecord_num;							//ÿ�������������¼��
	int source_file_num ;						//ɨ������Դ��������	 
	int szSleepTime;							//ÿ������ʱ��

	//char szDebugFlag[50];						//�Ƿ����������־

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;
		source_file_num = 0;
		maxRecord_num = 0;
		szSleepTime = 5;
		bak_flag = 'N';							//Ĭ�ϲ�����

		memset(iWorkflowId,0,sizeof(iWorkflowId));
		memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));
		
		memset(iInputId,0,sizeof(iInputId));
		memset(iOutputId,0,sizeof(iOutputId));

		memset(szInPath,0,sizeof(szInPath));
		memset(szErroPath,0,sizeof(szErroPath));
		memset(szBakPath,0,sizeof(szBakPath));	
	}
};

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


class FileLoad : public PS_BillProcess
{
   public:

     FileLoad();
     ~FileLoad();    
	 
	 bool init(int argc,char** argv);
	 int getSourceFilter(char* source,char* filter);
	 int LoadSourceCfg();	

	 int dealFile();  //�����ļ�����������ϵͳ�ϲ�����
	 void prcExit();

	 //����ƽ̨
	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

   protected:
    //�ӽ�����Ҫʵ�ֵĺ��� �̳�PS_BillProcess
	int onBeforeTask();
	int onTaskBegin(void *task_addr);
	bool onChildInit();
	int onTask(void *task_addr, int offset, int ticket_num);
	//int onTask(int event_sn, int event_type, long param1, long param2, long src_id);
	//bool onChildTask(int event_sn, int event_type, long param1, long param2, long src_id);
	void onChildExit();
	int onTaskOver(int child_ret);

   private:
   
 //********************************************************************************************
	 DBConnection conn;
	 Statement stmt;

	 map<string,SOURCECFG> m_SourceCfg;					//�������Դ��������Ϣ
	 map<string,SOURCECFG>::const_iterator it ;
	 
	 SParameter mConfParam;
	 DRparameter mdrParam;
	 
	 CFmt_Change outrcd;								//�����ļ��ļ�¼����ȡ���е�ĳЩ�ֶ� add by hed 2013-07-31		
	 //vector<PkgFmt> m_record ;						//˽���ڴ��¼
	 PkgFmt*  m_record;

	 CF_CFscan scan;									//�ļ�ɨ��ӿ�

	 char m_szSourceID[6];								//����ԴID
	 char mServCatId[6];
	 char m_szFileName[JS_MAX_FILENAME_LEN];			//ԭʼ�ļ���
	 char fileName[JS_MAX_FILEFULLPATH_LEN];			//ȫ·���ļ���
	 char file_name[JS_MAX_FILENAME_LEN];				//�ָ����ļ���

	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];					//������Ϣ
	 char currTime[14+1];
	 
	 long file_id;
	 int curr_record_num;
	 int record_num;									//�ļ���¼��	
	 int split_num ;									//�ļ�����ʱ���з����Ĵ���
	 int file_num ;										//ÿ������Դ��ÿ��ɨ�������ļ����� �������¸�����Դ	 
	 short petri_status;								//��ȡpetri��״̬
	 short petri_status_tmp;
};

