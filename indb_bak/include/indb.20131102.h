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

#include "CF_CFmtChange.h"
#include "new_table.h"
//#include "CF_CError_Table.h"

using namespace std;
using namespace tpss;  //��psutil.h��Ӧ

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		=	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	2048;
const int JS_MAX_RECORD_LEN			=   1024;

const char FILE_ID[20]="FileID";

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
	 bool IsAuditSuccess(const char* dealresult);

   private:
     
 //********************************************************************************************
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //�������Դ��������Ϣ
	 map<string,SOURCECFG>::const_iterator it ;

	 map<string,int> mapConfig;   //������ı��־
	 CF_CNewError_Table	*TabConf;
	 map< string,int > mapTabConf;

	 char m_szSrcGrpID[8];  //����Դ��
	 char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //����ԴID
	 char mServCatId[6];

	 char input_path[JS_MAX_FILEPATH_LEN];  //�ļ��������·��
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];		//������Ϣ
	 char bak_path[JS_MAX_FILEPATH_LEN];	//����·��
	 char bak_flag ;						//���ݱ�־
	 char erro_path[JS_MAX_FILEPATH_LEN];   //�����ļ�·��
	 char currTime[15]; 
	
	 char m_szFileName[JS_MAX_FILENAME_LEN];  //ԭʼ�ļ���
	 char fileName[JS_MAX_FILEFULLPATH_LEN];  //ȫ·���ļ���

	 char m_szOutTypeId[8];	  //�ļ���¼��ʽ
	 CFmt_Change outrcd;	  //�����ļ��ļ�¼����ȡ���е�ĳЩ�ֶ� add by hed 2013-07-31 
	 CF_CFscan scan;	//�ļ�ɨ��ӿ�
 
	 int record_num;  //�ļ���¼��	
	 int file_num ;   //ÿ������Դ��ÿ��ɨ�������ļ����� �������¸�����Դ
	 int source_file_num ;		//ɨ������Դ��������
	 
	 long file_id;
	 char file_time[8+1];		//���ļ��������ȡ
	 short petri_status;	   //��ȡpetri��״̬
	
	 bool m_enable ;	//����״̬
	 int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	 char m_SerialString[4096];
	 char m_AuditMsg[4096];	
};

