/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		C_Indb.h
	Description:	��������ϵͳ���ļ������
					
	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2012/9/2	       ��ɳ������
	
	</table>
*******************************************************************/
//2013-09-01��д�ļ�ģ��������ļ��������,������,��������ˮ���ϵ����������ݿ�����,������ˮ���ϵ������ں��Ĳ�������
//2013-10-24 �޸��ļ��������������ٲ�ʧ�ܲ�д���ȱ������,���ݰ���ʱ��Ŀ¼

#include<iostream>
#include <vector>

#include "process.h"
#include "psutil.h"
#include "tp_log_code.h"

#include "RTInfo.h"
#include "dr_api.h"


#include "CF_Common.h"
#include "CF_CFscan.h"
#include "CF_CLogger.h"

#include "CF_CMemFileIO.h"
#include "CF_CFmtChange.h"
#include "new_table.h"
//#include "CF_CError_Table.h"

using namespace std;
using namespace tpss;  //��psutil.h��Ӧ

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

const char FILE_ID[20]="FileID";
const char RATE_CYCLE[20]="RateCycle";

//����Դ��Ϣ
struct SOURCECFG
{
	char szSourceId[6];		//����ԴID
	char szInFileFmt[6];		
	char szSourcePath[256];   //����Դ����·��
	char filterRule[50] ;	//����Դ�Ĺ��˹���
	char serverCatID[6];
	int  file_begin;		  //��ȡ�ļ��������ʱ��
	int  file_length;
	
	SOURCECFG()
	{
		file_begin = -1;
		file_length = -1;
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
		
	int source_file_num ;						//ɨ������Դ��������	 
	int szSleepTime;							//ÿ������ʱ��

	//char szDebugFlag[50];						//�Ƿ����������־

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;
		source_file_num = 0;
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


class C_Indb : public PS_Process
{
    public:
     C_Indb();
     ~C_Indb();    
	 
	 bool init(int argc,char** argv); 
	 int getSourceFilter(char* source,char* filter,int &index,int &length);
	 int LoadSourceCfg();
	 int dealFile();  //�����ļ�����������ϵͳ�ϲ�����
	 void run();
	 void prcExit();

	  //����ƽ̨
	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

   private:   

 //********************************************************************************************
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //�������Դ��������Ϣ
	 map<string,SOURCECFG>::const_iterator it ;
	 map<string,int> mapConfig;   //������ı��־
	 CF_CNewError_Table	*TabConf;
	 map< string,int > mapTabConf;
	 SParameter mConfParam;
	 DRparameter mdrParam;

	 char m_szSourceID[8];		//����ԴID
	 char mServCatId[6];
	 char m_szFileName[256];	//ԭʼ�ļ���
	 char fileName[512];		 //ȫ·���ļ���
	 char currTime[15]; 
	 char sql[1024];
	 char erro_msg[2048];		//������Ϣ 	 
	 
	 CFmt_Change outrcd;	    //�����ļ��ļ�¼����ȡ���е�ĳЩ�ֶ� add by hed 2013-07-31
	 CF_CFscan scan;		    //�ļ�ɨ��ӿ�
 
	 int record_num;            //�ļ���¼��		
	 int file_num ;             //ÿ������Դ��ÿ��ɨ�������ļ����� �������¸�����Դ
	 long file_id;
	 char file_time[8+1];		//���ļ��������ȡ
	 short petri_status;	    //��ȡpetri��״̬
	 short petri_status_tmp;	
};
