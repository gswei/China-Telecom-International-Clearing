//filter_common.h
#ifndef _FILTER_COMMON_H_
#define _FILTER_COMMON_H_

//#define FILTERTM_USING_HASH_COMPRESS    //�Ƿ���hash�㷨ѹ��ȥ�عؼ��֣���������

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
#include "CF_CHash.h"
#include "CF_CFscan.h"
#include "CF_CError.h"
#include "Common.h"
#include "config.h"
#include "COracleDB.h"
#include "CFmt_Change.h"
#include "CF_MemFileIO.h"
#include "wrlog.h"
#include "encrypt.h"
#include "CF_InfoPoint.h"
#include "interrupt.h"
#include "FILTERTM_CFilter.h"
#include "FILTERTM_ERRCODE_DEFINE.h"
#include "ProcessMonitor.h"
#include "Log.h"
#include "CF_CRecStat.h"
#include "CF_CDealLog.h"
#include "interpreter.h"
#include "CF_CStrExp.h"


struct SFileStruct
{
  char szFileName[257];
  char szSourceId[6];
  char szSourcePath[251];
  //edit by linyb 20080430
  char szLocalNet[21];
  //end of edit by linyb 20080430
};

struct SPickKey
{
	char m_szColName[30];
	char m_cPickTimeFlag;   //added by tanj 20060301
	char m_cPickKeyType;
	char m_szPickKeyFormat[256];
	int m_iFieldLen;
	CF_CStrExp m_StrExp;
	SPickKey()
	{
		memset(m_szColName,0,sizeof(m_szColName));
		m_cPickTimeFlag = 'N';
		m_cPickKeyType = 'F';
		memset(m_szPickKeyFormat,0,sizeof(m_szPickKeyFormat));
		m_iFieldLen = 0;
	}
};

//add by linyb 20071109
struct SSourceEnv
{
	char m_szPickConfigId[20];
	//edit by linyb 20080108
	//bool m_bCompressFlag;
	int m_iCompressFlag;    // 0:��ѹ��1:ѹ��2:ת����ѹ�����ֶ�Ϊ16λ
	//end of edit by linyb 20080108
	long m_lPickLen;
	SPickKey m_PickKey[100];
	SSourceEnv()
	{
		memset(m_szPickConfigId, 0, sizeof(m_szPickConfigId));
	 	m_iCompressFlag = 0;
		m_lPickLen = 0;
	}
};
//end of add by linyb 20071109
struct SParameter
{
  char szEnvPath[FILE_NAME_LEN+1]; //������������·��
  char szEnvFile[FILE_NAME_LEN+1]; //���������ļ���
  char szPipeId[6];                 //��ˮ��ID
  int  iProcessId;                 //������̱�ʶ
  int  iWorkflowId;                //������ģ��ID
  int  iInputId;                  //����ӿ�ID
  int  iOutputId;                  //����ӿ�ID
	char szInputFiletypeId[6];          //�����ļ�����
  char szOutputFiletypeId[6];          //����ļ�����
  char szInrcdType;                 //�����ļ���¼����
  char szOutrcdType;                //����ļ���¼����
  char szInCtlTabname[33];            //������Ʊ�
  char szOutCtlTabname[33];           //������Ʊ�
  char szInPath[256];                //����·�������·����
  char szOutPath[256];               //���·�������·����
  char szDebugFlag[50];              //�Ƿ����������־
  int  iTimeIndex;                 //���������жϵ�����/ʱ���ֶ����
  char szCheckName[20];              //��Ϣ����־������
  long lBlockNum;
  long lIndexTableNum;
  char szPickConfigId[10];
  //SPickKey PickKey[100];          //100��ȥ�عؼ���
  //add by linyb 20071109
  map<string, SSourceEnv*> mapSourceKey;
  //end of add by linyb 20071109
  long lPickKeyLen;
  char szDupIndexPath[256];       //added by tanj 20050719
  int iRunFlag;
  int iSleepTime;               //added by tanj 20051121
  char szBackupFlag[256];               //added by tanj 20051012  for ctjf
  char szDupDealFlag[10];               //added by tanj 20051013  for ctjf
  char szFilterIndexMode[10];           //added by tanj 20060109  P��ȥ����������ˮ�ߴ�ţ�S��������Դ���
  char szFilterCondition[1024];        //added by tanj 20060116  ���������ľͲ�����ȥ��
  char szFileName[256];                //added by tanj 20060117 Ϊ�˱��ʽ���ļ���������ı���
  //add by linyb 20071116
  char szStartDealTime[15];
  //end of add by linyb 20071116
  //delete by linyb 20071109
 // bool bCompressFlag;                  //added by tanj 20060211 �Ƿ�ѹ��ȥ�عؼ��֣�����md5�㷨��ԭ�򣬶��Ų���ѹ��
  //end of delete by linyb 20071109
  CFmt_Change inrcd, outrcd;           //added by tanj 20060116 Ϊ����������ȥ���������ܣ����������˴�
	Interpreter theCompile;              //added by tanj 20060116 Ϊ����������ȥ���������ܣ����Ӷ�̬��������
	
	
  FILTERTM_CFilter *Filter;
  CF_CDealLog DealLog;
  CF_CRecStat RecStat;
  CF_CHash     HashCompresser;
  CProcessMonitor ProcessMonitor;

  //char szUpdateRecordFlag[50];
  //CF_CErrorLackRec *lack_info;         //��/�����ϵ��ӿ���
  //ProcessRec_Mgr *m_mgr;             //�����

  SParameter()
  {
 	  memset(szPipeId, 0, 6);
    iProcessId = 0;
 	  iWorkflowId = 0;
    iInputId = 0;
 	  iOutputId = 0;
 	  memset(szInputFiletypeId, 0, 6);
 	  memset(szOutputFiletypeId, 0, 6);
 	  szInrcdType = 0;
 	  szOutrcdType = 0;
 	  memset(szInCtlTabname, 0, 33);
 	  memset(szOutCtlTabname, 0, 33);
 	  memset(szInPath, 0, 256);
 	  memset(szOutPath, 0, 256);
 	  sprintf(szDebugFlag, "%s", "Y");
    iTimeIndex = -1;
    memset(szCheckName, 0, 20);
    lBlockNum = 0;
    lIndexTableNum = 0;
    memset(szPickConfigId, 0, sizeof(szPickConfigId));
	//add by linyb 20071116
	memset(szStartDealTime, 0, sizeof(szStartDealTime));
	//end of add by linyb 20071116
    lPickKeyLen = 0;
    Filter = NULL;
	//add by linyb 20071109
	mapSourceKey.clear();
	//end of add by linyb 20071109
 	  //memset(szUpdateRecordFlag, 0, 50);
 	  //lack_info = NULL;
 	  //m_mgr = NULL;
  }

};

extern SParameter Param;      //�ñ���������filter_common.cpp�У��������ļ�ʹ��


void fillStr(char *str, long len, char filter);
void HoldOn(SParameter &Param);
int selectFromPipeEnv(const char *szPipeId,const char *szEnvName, char *szEnvValue);
int selectFromPipeEnv(const char *szPipeId,const char *szEnvName, long &lEnvValue);
int selectFromPipeEnv(const char *szPipeId, int iProcessId,const  char *szEnvName, char *szEnvValue);
int selectFromPipeEnv(const char *szPipeId, int iProcessId,const  char *szEnvName, long &lEnvValue);
//add by linyb 20071109
int selectFromSourceEnv(const char *szSourceId, int iProcessId,const  char *szEnvName, char *szEnvValue);
//end of add by linyb 20071109
bool checkTimeFormat(const char *szTimeStr);
unsigned long calcIndex(const char *szIndexStr);
bool addHours(int nHours, const char *szOrgTime, char *szDestTime);
int paramInit();
//add by linyb 20080108
int TransHToD(char *instr,char *outstr,int outstrlen);
//end of add by linyb 20080108
#endif

