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

using namespace tpss;  //��psutil.h��Ӧ

#include "CF_Common.h"
#include "CF_CFscan.h"
#include "CF_CLogger.h"

#include "CF_CFmtChange.h"
#include "CF_CError_Table.h"

using namespace std;

const char FILE_ID[20]="FileID";

//����Դ��Ϣ
struct SOURCECFG
{
  char szSourceId[6];		//����ԴID
  //char szFile_Fmt[6];		//����Դ��Ӧ�ĸ�ʽ
  char szInFileFmt[6];		
  char szSourcePath[256];   //����Դ����·��
  //char szTollCode[16];      //����������
  char filterRule[256] ;	//����Դ�Ĺ��˹���

  char serverCatID[5];
  int  file_begin;		  //��ȡ�ļ��������ʱ��
  int  file_length;

};


class C_Indb : public PS_Process
{
    public:
     C_Indb();
     ~C_Indb();    
	 
	 bool init(int argc,char** argv); 
	 //int getSourceFilter(char* source,char* filter);
	  int getSourceFilter(char* source,char* filter,int &index,int &length);
	 int LoadSourceCfg();
	
	 int dealFile();  //�����ļ�����������ϵͳ�ϲ�����
	 void run();

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
	 map< string,CF_CError_Table> mapTabConf;

	 char m_szSrcGrpID[8];  //����Դ��
	 char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //����ԴID

	 char mServCatId[5];

	 char input_path[256];     //�ļ��������·��
	 char sql[1024];
	 char erro_msg[1024];  //������Ϣ
	 char bak_path[256];	//����·��
	 char bak_flag ;    //���ݱ�־
	 char erro_path[256];  //�����ļ�·��
	 char currTime[15]; 
	
	 char m_szFileName[256];  //ԭʼ�ļ���
	 char fileName[512];	  //ȫ·���ļ���

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

