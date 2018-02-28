
/****************************************************************
filename: CF_Error_Table.h
module: CF - Common Function
created by: zhoulh
create date: 2005-10-18
update list: 
version: 1.0.0
description:
    the header file of the classes for CF_Error_Table
*****************************************************************/ 
#ifndef _CF_Error_Table_H_
#define _CF_Error_Table_H_ 1

#include "CF_CStat.h"
#include <string.h>

#define	ERROR_RECORD_COUNT              50  //达到该话单数进行insert数据库操作
#define	KEYWORK_ERRORTYPE               16  //关键字errortype定义为16
#define	KEYWORK_PROCESSID               17  //关键字PROCESSID定义为17
#define	KEYWORK_INVALIDFLAG             18  //关键字INVALIDFLAG定义为18
#define	KEYWORK_REDOFLAG                19  //关键字REDOFLAG定义为19
#define	KEYWORK_ERRORTYPE2               31  //关键字errortype2定义为31
#define	KEYWORK_ORARCD               32  //关键字原始话单定义为32



class CF_CError_Table
{
  public:
    CF_CError_Table();
    ~CF_CError_Table();
    int Init(char *,int ,int,char *SortKey);
    int Init(char *,int ,int);
	int setFlag(char *InvalidFlag,char *RedoFlag);
    int setFileName(char *FileName,char *Source_Id = NULL);
    int dealInsertRec(CFmt_Change &InRecord,int ErrorType = 0);
    int dealInsertRec(CFmt_Change &InRecord,char *ErrorType2,int ErrorType = 0);
    int dealInsertRec(CFmt_Change &InRecord,char *ErrorType2,char *OraRcd,int ErrorType = 0);


    int Get_OutRcd(CFmt_Change &OutRcd,int Index);
    int dealSelectRec(CFmt_Change &InRecord,int ErrorType);
	int dealUpdateFlag(CFmt_Change &InRecord,char Invalid_Flag,char Redo_Flag);
    int dealBackupRec();
    void commit();
    void rollback();


  private:
    int DeleteSpace( char *ss );
    int Make_Select_Count_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp);
    int Make_Select_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp);
    int Make_Insert_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp);
    int Set_KeyWord(STAT_RECORD &Cur,int i,char *Key_Container);
    int Set_KeyStatItem(STAT_RECORD &,int );
    int Insert_Error();
    int Select_All();
    int Select_Error();
    int Update_Flag(char Invalid_Flag,char Redo_Flag);
    int Stat_Record(CFmt_Change &);
    int SplitBuf( char *ss,char c);

    int         m_NRecord_Count;
    STAT_TABLE  m_SStat_Table;
    STAT_RECORD *m_PsPre_Stat_Record;
    STAT_RECORD *m_PsRes_Stat_Record;
    STAT_RECORD *m_PsCur_Stat_Record;
    char        m_SzSchedule[STAT_ITEM_NAME_LEN];//输入控制表名
    char        m_SzSource_ID[STAT_ITEM_NAME_LEN];//控制表中source_id
    char        m_SzStat_Time[STAT_ITEM_NAME_LEN];//若统计类型为“-”，取控制表中dealtime;否则，取当前时间
    char        m_SzFileName[100];
    char        m_SzErrorType[STAT_ITEM_NAME_LEN];
    char        m_SzProcessId[STAT_ITEM_NAME_LEN];
    char        m_SzInvalidFlag[STAT_ITEM_NAME_LEN];
    char        m_SzRedoFlag[STAT_ITEM_NAME_LEN];
    char        m_szErrorType2[100];

    char        m_szOraRcd[ERROR_RECORD_COUNT+1][1000];


    int iSortKey_Num;
    char szSortKeyStr[10][50];
};

#endif


