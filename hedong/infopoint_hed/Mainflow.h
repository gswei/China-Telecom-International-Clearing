#include <fstream>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
#include <map>
#include "Log.h"
#include "wrlog.h"
#include "COracleDB.h"
#include "encrypt.h"
#include "CF_MemFileIO.h"
#include "Common.h"
#include "CTimeCheck.h"
#include "Infodeal.h"
#include "Infocom.h"

struct SInfo
{
	char szInfoPoint[20];
	char szTypeId[20];
	char szInfoDesc[150];
	char szDealType[5];
	char szStartTime[16];
	char szRate[6];
	int  iDelay; 
	CTimeCheck cTC;	
	
	SInfo()
	{
		memset(szInfoPoint, 0, 20); 
		memset(szTypeId, 0, 20); 
		memset(szInfoDesc, 0, 150);
		memset(szDealType, 0, 5);
		memset(szStartTime, 0, 16);
		memset(szRate, 0, 6);
	}
};

vector<SInfo> g_sInfo;
//int DealInfo(CInfoDeal &cInfoDeal,char *szDealMode,char *szKPI,int kk);
char* GetSubStr(const char* szSource, int nIndex, char cSeparator, char* szDest);
int CheckLock(char* szChTime,int iChDelay,char *szChCurDatetime,char*sFrom,char *sTo);
char* addmin(char* szDatetime, int iMin,char *szDeDatetime);
int GetRunFlag(int &iRunFlag, char *serv);
//CDatabase dbconn_2;
