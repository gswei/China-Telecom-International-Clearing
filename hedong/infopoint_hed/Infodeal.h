/****************************************************************
filename: Info_deal.h
module: CF - Common Function
created by: lij
create date: 2012-02-14
update list: 
version: 1.0.0
description:
    the header file of the classes for Info_deal
*****************************************************************/ 
#ifndef _INFO_DEAL_H_
#define _INFO_DEAL_H_ 1

#include <time.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <map>
#include "Log.h"
#include "wrlog.h"
#include "COracleDB.h"
#include "CF_MemFileIO.h"
#include "Common.h"

//#include "CF_Common.h"
//#include "CF_CFmtChange.h"
//#include "CF_CException.h"
//#include "CF_COracleDB.h"
//#include "CF_CMemFileIO.h"
//#include "CF_CInterpreter.h"
//#include "CF_CLogger.h"
#include "CF_CStat.h"
#include "interpreter.h"

//#define	STAT_RECORD_COUNT              3000  
//#define  STAT_INSERT_CONNT             500 //更新库表时一次插入的记录书
#define	STAT_ITEM_COUNT                100    //表项个数
#define	STAT_ITEM_NAME_LEN             50    //表项名长度
#define	STAT_ITEM_CONTAINER_LEN        251    //表项内容长度
#define  STAT_VALUE_CONTAINER_LEN       100   //字段值内容
//#define  STAT_GROUP_COUNT               10    //统计标识组个数
//#define  MAX_STATID_NUM                 50    //一条话单允许的最大统标识个数
#define  SQL_LENGTH               2000
#define  Value_Max_Num           50

#define	STAT_SZ_TYPE                   2     //表项内容为字符串型的类型标志
#define	STAT_N_TYPE                    1     //表项内容为整型的类型标志
#define	STAT_N_COUNT_TYPE              0     //表项内容为整型且为累计项的类型标志

struct INFO_RECORD//统计记录
{
  //char Info_Value[Value_Max_Num][STAT_ITEM_CONTAINER_LEN];
  vector<string> v_row;
  Reset()
		{
			v_row.clear();
			return 1;
		}
};

//2014-03-13 
struct D_STAT_TABLE								//统计表
{
  char szInfoPoint[20];
  //char szRate[6];
  char SzSrcT_Name[32];							//原始表表名
  char SzDesT_Name[32];							//目标表表名
  char SzSrc_Item_Name[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];	    //话单数统计项名
  char SzDes_Item_Name[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
  int  NItem_Type[STAT_ITEM_COUNT];				//统计项类型,0统计项 1统计维度
  int  NStat_Item_Count;						//统计字段个数

  D_STAT_TABLE()
  {
	NStat_Item_Count=0;
	//memset(szRate,0,sizeof(szRate));
	memset(SzSrcT_Name,0,sizeof(SzSrcT_Name));
	memset(SzDesT_Name,0,sizeof(SzDesT_Name));
	memset(NItem_Type,0,sizeof(NItem_Type));
	///memset(SzSrcT_Name,0,sizeof(SzSrcT_Name));
	//memset(SzSrcT_Name,0,sizeof(SzSrcT_Name));

  }
};

struct INFO_FILE//信息表
  {
  char SzTable_Name[STAT_ITEM_CONTAINER_LEN];//表名
  char ChStat_Type;//表类型
  char Server_id; //服务类型
  char Info_netid[4]; //NETID
  char Info_outpath[256];
  char Info_srate[5];
  char Info_auditid[20];
  char Info_typeid[20];
  char Info_tablename[50];
  char Info_sysname[50];
  char Info_username[50];
  char Info_password[50];
  char Info_netypeid[50];
  
  char Info_Item[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];//存储的字段名称
  int  NField_Index[STAT_ITEM_COUNT];
  char SzStat_FieldName[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
  char SzStat_database[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
  char SzStat_table[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
  char SzStat_user[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
  char Info_Value[Value_Max_Num][STAT_VALUE_CONTAINER_LEN];
 // vector<INFO_RECORD> vec_info;
  
  int  NInfo_Item_Count;  //文件字段总数
  int  NInfo_value_Count;  //文件字段值总数
  int  NItem_Type[STAT_ITEM_COUNT];
  int  NStat_Item_Begin[STAT_ITEM_COUNT];
  int  NStat_Item_End[STAT_ITEM_COUNT];
  
 // int  NStat_Item_database[STAT_ITEM_COUNT];
  //char chStat_Item_SprInField[STAT_ITEM_COUNT];
  
   // int table_name_key_count;
    //char NField_dataType[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
    //char IsIndex[STAT_ITEM_COUNT]; 
};

class CInfoDeal
{
public:	
	int GetInfoEnv();
	int GetBatch(char* szAuditId, char* szTypeId, char* szRate);
	int GetSerV();
	//char *GetSubStr(char* szSrcStr, int nIndex, char cSeparator, char* szDest);
	int GetFileName(char *szFileName, char *szdealtype,char* service);
	int GetFileValue(char* szDealtype,char *starttime,char *endtime);
	int GetVar(char *szVarStr,char* szVarName, char* szVarValue, int isInt);

	int SetDate(char* startdate, char* enddate);
	int Init(char* infoDetect,char* service);
	void SeqSuc(char *szEnd);
	int GetServBCode(char *szStr,int szValue);
	int WriteFile(char* szDealtype,char *service, char *starttime,char *endtime);
    
	int StatTable(char* szDealtype,char *starttime,char *endtime);
private:
	
	map<string, int> m_mapProModule;
	vector<string> m_vecServPipe;
	vector<int> m_vecpServBCode;
	vector<string> m_vecInfoMsg;
	
  char m_sid[30];
  char m_username[30];
  char m_password[30];
  char m_szSysBcode[30];
  char m_szSysScode[30];
  int m_iIsServCat;
  int m_iIsMain;
  
	char m_szChkStart[30];
	char m_szChkEnd[30];
	char m_szInfoDetect[300];
	char m_kpid[100];
	char m_szNetId[30];
	char m_szBusiClass[30];

	int m_iBatchID;
	int m_iSeqID;
	int m_iOrder;
	int m_iStart;
	int m_iRun;
	int iCountRun;
	char m_szToday[40];
	char m_szService[30];
	char m_szDataType[30];

	char m_szHostName[40];
	char m_szHostIp[40];

	char m_szOutPath[300];
	char m_szIdepPath[300];
	char m_szDrId[30];
	char m_szDataSource[30];
	char m_szRateTime[8];
	char sz_errtime[16];
	char szSql[1024];
	char m_szSep[5];
	char szJosn[1000];
	char szVarJosn[30];
	char szKbpId[10]; //KBP编号
	char szKbpName[20]; //KBP名称
	
	float iTimeUnit;
	float iFeeUnit;
	
	char m_szServiceId[10]; //区分AB机
	INFO_FILE m_info_file;
    D_STAT_TABLE  mStatTable;

	CDatabase dbconn_1;
	/*CDatabase dbconn_2;
	CDatabase dbconn_3;
	CDatabase dbconn_4;
	CDatabase dbconn_5;
	CDatabase dbconn_6;
	CDatabase dbconn_7;*/
};

#endif

