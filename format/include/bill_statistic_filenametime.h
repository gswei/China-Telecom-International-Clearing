#ifndef _Bill_statistic_filenametime_h_
#define _Bill_statistic_filenametime_h_ 1
//#include "CF_COracleDB.h"
#include "psutil.h"

#include <string.h>
#include <stdio.h>
//#include "sqlca.h"
//#include "oraca.h"
using namespace tpss;

#define MAX_BILL_STAT 10000
#define MAX_H_BILL_STAT 44640
/*
#ifndef SQLCODE
    #define SQLCODE sqlca.sqlcode
#endif

#ifndef DBNOFOUND
    #define DBNOFOUND ((SQLCODE ==100)||(SQLCODE ==1403)||(SQLCODE ==4)||(SQLCODE ==1095))
#endif
*/

struct S_BillStat_fnTime
{
    char stat_time[14+2];//��ʾͳ�Ƶ�ʱ��,to minute
    long number;    //��ʱ�εĻ�����
};


struct S_H_BillStat_fnTime
{
    char stat_time[14+2];//��ʾͳ�Ƶ�ʱ��,to minute
    long number;    //��ʱ�εĻ�����
    int iExist;
};

class C_BillStat_fnTime
{
private:    
	int check_time(char *buf);
    void DelSpace( char * );
private:
    char source_id[ 5+3 ];
    int iIndex;
    long   hour_num;  /*��ǹ��ж��ٸ�ʱ�εĻ�����ͳ��, �Է���Ϊ��λ*/
    struct S_BillStat_fnTime  *p_stat; 
    long   max_count;
    char  Month[2][16];
    S_H_BillStat_fnTime p_H_stat[2][MAX_H_BILL_STAT];
    
    char NowMonth[16];
    
public:
    C_BillStat_fnTime(){max_count=0;}
    C_BillStat_fnTime( char *,int index );
    ~C_BillStat_fnTime( );
    int Init( char *,int );
    int GetTime();
    int  AddItem( char * );
    //int  UpdateDB( char * ,CDatabase&);
	int  UpdateDB( char * ,DBConnection);
    //int  DelUpdateDB( char * ,CDatabase&);
	int  DelUpdateDB( char * ,DBConnection);
    int  ResetAll();
};


#endif

