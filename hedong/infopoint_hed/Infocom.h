#include <fstream>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
#include <map>
#include "Log.h"
#include "wrlog.h"
#include "COracleDB.h"
#include "CF_MemFileIO.h"
#include "Common.h"
#include "CF_CFscan.h"

// express by hlp, ��Ϣ���ۺ��࣬�����ۺ��ϴ�������b���ϵ��������ݺ������������ݡ�
#ifndef _INFOCOM_
#define _INFOCOM_ 1

#define INFO_FILE_REC          100


typedef struct FILE_INFO
{
  char  sz_orgFname[255];
  long  lCreateTime;
  int   iFileSize;
  int iFlag;
  FILE_INFO() 
  {
    memset(sz_orgFname,0,255);
    lCreateTime=0;
    iFlag=0;
  }
};

class CInfoCom
{
public:
	int Infocom();	//�ۺϸ���Ϣ����־
	int Check();  //�ж�ͬһ���ε���Ϣ���ļ��Ƿ���
	int Init(int iMax);   //��ʼ����
	char* GetSubStr(char* szSrcStr, int nIndex, char cSeparator, char* szDest);
	int GetSeq(char *kpid,char *sType,char *szRate,char *szDate);
	int GetInfoEnv();
	int ScanFiles(char *pch_sourcePath,char *pch_fileFilter);

		

private:
	
	
  char m_outfile[200]; 
	char m_kpid[100];
	char m_type[10];
	char m_rate[10];
	char m_szNetId[10];
	char m_szDrId[3];
	int  m_iSeqID;
	int  m_iOrder;
	char m_szToday[9];	
	char m_szOutPath[300];
	char m_szCheckPath[300];
	char m_szEndTime[18];

	
	char sz_errtime[16];
	char sz_errmsg[100];
	
	int iCountMax;	
	int iMaxRec;
	
	
	struct FILE_INFO  *filelist;
	
	vector<string> m_vecEndTime;


};

#endif

