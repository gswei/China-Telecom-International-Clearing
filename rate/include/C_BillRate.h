/****************************************************************
  Project	
  Copyright (c)	2010-2011. All Rights Reserved.		�㶫��Ѹ�Ƽ����޹�˾ 
  FUNCTION:	һ������
  FILE:		C_BillRate.h
  AUTHOR:	liuw
  Create Time: 2010-05-10
==================================================================
  Description:  
		����ۺϼƷѽ���3.0ϵͳ���¿����ƣ�����ģ���ò����ʽʵ��
  UpdateRecord: 
==================================================================

 *****************************************************************/

#ifndef _C_BILLRATE_H_
#define _C_BILLRATE_H_	1
 
#include "CF_CPlugin.h" //��������ļ�

#include "CF_CInterpreter.h"
//#include "CF_COracleDB.h"
#include "CF_CPluginPacket.h"
#include "CF_Config.h"

//#include "CF_CPluginengine.h"
#include "CF_CPlugin.h"
#include "CF_CMessage.h"

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

#include "settlerule.h"
#include "psutil.h"



const int  CALCFEE_ERR_NOT_ENOUGH_MEMORY =   6001;  //��̬�����ڴ�ʧ��
const int  CALCFEE_ERR_IN_READ_ENV_VAR   =   6002;  //��ȡ������������
const int  CALCFEE_ERR_IN_SELECT_DB      =   6003;  //��ѯ���ݳ���
const int  CALCFEE_ERR_IN_CONNECT_DB     =   6004;  //�������ݿ����
const int  CALCFEE_ERR_IN_CHECK_RULE     =   6005;  //������۹���ʵ����ʧ��
const int  CALCFEE_ERR_IN_LOAD_TABLE     =   6006;  //�������ݱ��ڴ�ʧ��
const int  CALCFEE_ERR_IN_SORT_TABLE     =   6007;  //�ڴ����ݱ�����ʧ��
const int  CALCFEE_ERR_UNKNOWN_CATCH     =   6008;  //��׽���޷�ʶ��Ĵ�����
const int  CALCFEE_ERR_NEED_RESTART      =   6009;  //���ش��󣬽�������������
const int  CALCFEE_ERR_PK_DUPLICATE      =   6010;  //���ݱ������ظ�
const int  CALCFEE_ERR_SLFILE_NOT_EXIT   =   6011;  //����ļ�������
const int  CALCFEE_ERR_ACCESS_PROC_MEM   =   6012;  //��ȡ���̼�ع����ڴ����
const int  CALCFEE_ERR_IN_OPEN_FILE			 =	 6013;  //���ļ����� --add by liuw 20070214

class	BillRate: public BasePlugin
{
	
private:
    int initflag;
    CFmt_Change inrcd,outrcd;
    
  	char szErrMsg[ERROR_MSG_LEN+1];						//������Ϣ
  	char szCurDatetime[DATETIME_LEN+1];				//��ǰʱ��
  	char szStartupTime[DATETIME_LEN+1];				//ģ������ʱ��
		char szLogStr[LOG_MSG_LEN+1];							//��־��Ϣ
		char szSqlTmp[10*SQL_LEN+1];							//�洢��ʱsql���ı���
		int  iRuleSavingInterval;	//�����Ӷ�������ȷ����󱣴�һ�����ݵ��ļ�
		CF_MemFileO pAuditFile;   //Ԥ����ˡ�����ӿ�
		CF_MemFileO *pDispFiles;
		//DBConnection conn;//���ݿ�����
		char szTxtType[20]; //̯��ʱ��̯�ָ�ʽ


public:
		BillRate();
		~BillRate();
		int dealFlag;
		void init(char *szSourceGroupID,char * serviceID,int index);
		void init(char *jobID, char *ratecycle);// �ع���ʼ��������Ϊ����̯����

        //Ϊ��������ʱʹ�ô˺������г�ʼ��
		void m_init(char * sourceid,STparam &szTparam,char* job_id);
		
 		void execute(PacketParser& ps,ResParser& retValue);
 		
		void message(MessageParser&  pMessage);
		
		void printMe(){
			//��ӡ�汾��
			printf( "\t�������:����,����޸����ڣ�2010-06-19 liuw,�汾�ţ�1.0.0 \n");
			return;
		};
		
		std::string getPluginName()
		{
			return "BillRate";
		}
		std::string getPluginVersion(){
			return "3.0.0";
		}


};

#endif


