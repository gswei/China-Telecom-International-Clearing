/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-16
File:			 CSql.h
Description:	 ʵʱSQL���ģ��
	��petri״̬���л�ȡ���ж��Ƿ�Ϊ��д��
	��ȡĿ¼�µ�sql�ļ��������

	����ָ��Ŀ¼�µ�SQL�ļ� C_GLOBAL_ENV
    ���SQL�ļ��ǼǱ� D_SQL_FILEREG

    �ӿ�˵��  �������jsextSQL 
**************************************************************************/
//2013-11-22 sql�ļ��ٲ��������
//2013-12-05 �ٲ�ʧ��ʱ,��Ҫ��С��Χ��ѯԭ��,��������(�ٲ��ļ�����,��С�ٲõ�λ����)���õ����ݿ���

#ifndef CSQL_H
#define CSQL_H

#include <iostream>
#include <vector>
#include <dirent.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include <unistd.h>     //��ȡ��ǰ��������Ŀ¼
#include <fstream>

//#include "bill_process.h"
#include "process.h"

#include "psutil.h"
#include "tp_log_code.h"
#include "RTInfo.h"   //��ȡpetri��״̬
#include "dr_api.h"

using namespace tpss;  //��psutil.h��Ӧ

#include "CF_CFscan.h"
#include "CF_Common.h"
#include "CF_CLogger.h"

using namespace std;

#define SQL_COMMIT_COUNT	1000	

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	2048;
const int JS_MAX_RECORD_LEN			=	2048;
//const int JS_MINI_AUDIT_NUM			=	1;

//����ˮ��Ϊ��λ
struct SParameter
{
	//char szSrcGrpID[6];							//����Դ��
	char szService[6];							//serviceID
	//char  iWorkflowId[10];						//������ģ��ID  
	int  iflowID;								//��ˮ��ID
	int  iModuleId;								//ģ���ʶ
	//char  iInputId[10];						//����ӿ�ID
	//char  iOutputId[10];						//����ӿ�ID
	//char szSchCtlTabname[32+1];				//���ȱ��

	char szInPath[JS_MAX_FILEFULLPATH_LEN];		//����·��	
	char szBakPath[JS_MAX_FILEFULLPATH_LEN];	//�����ļ�·��
	char szErrPath[JS_MAX_FILEFULLPATH_LEN];	//����·��
	char szFailPath[JS_MAX_FILEFULLPATH_LEN];	//�ٲ�ʧ��·��

	int szSleepTime;							//ÿ������ʱ��
	//char szDebugFlag[50];						//�Ƿ����������־

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;		
		szSleepTime = 5;
		//memset(iWorkflowId,0,sizeof(iWorkflowId));
		//memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		memset(szInPath,0,sizeof(szInPath));
		memset(szErrPath,0,sizeof(szErrPath));
		memset(szFailPath,0,sizeof(szFailPath));
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


class CSql: public PS_Process
{

private:
    
	char  m_Filename[JS_MAX_FILENAME_LEN];		//�ļ���
	char  filenames[JS_MAX_FILEFULLPATH_LEN];   //ȫ·���ļ���
	//char  input_path[512];					//�ļ�����·��
	//char  output_path[512];					//�ļ����·��
	//char  erro_path[512];						//�ļ�ʧ��·��
	//char  fail_path[512];						//�ٲ�ʧ��·��
	char  erro_msg[JS_MAX_ERRMSG_LEN];			//������Ϣ	
	char  currTime[15];
	CF_CFscan scan;	//�ļ�ɨ��ӿ�
    DBConnection conn;//���ݿ�����

	SParameter mConfParam;
	DRparameter mdrParam;
	
	bool mini_flag			 ;		//2013-12-05
	vector<string> mMiniVfile;

	vector<string> mVfile;		//�����ļ�
	vector<char> vDealFlag;
	short petri_status ;
	int	  audit_file_num ;
	int   audit_fail_mini_num;
    
public:
	CSql();
	~CSql();
	bool init(int argc,char** argv);   
	void run();
	int getFileExist();		//��ȡ�ļ���

	bool doAllSQL();//���ĳ���ļ�����ȡ����SQL��䣬��ִ��
	void saveLog();  //ÿ����һ���ļ������浽D_SQL_FILEREG����
	bool moveFiles(int flag);//���Ѿ��������ļ��ƶ���ָ������Ŀ¼
	void prcExit();

	//����ƽ̨
    bool drInit();
	bool CheckTriggerFile();
	int  drVarGetSet(char* m_SerialString);
	int IsAuditSuccess(const char* dealresult);
};

#endif 





























