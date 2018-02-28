#ifndef _Bill_statistic_route_h_
#define _Bill_statistic_route_h_ 1
//#include "CF_COracleDB.h"
#include "psutil.h"

#include <string.h>
#include <stdio.h>
//#include "sqlca.h"
//#include "oraca.h"
using namespace tpss;

#define MAX_BILL_STAT 10000
/*
#ifndef SQLCODE
    #define SQLCODE sqlca.sqlcode
#endif

#ifndef DBNOFOUND
    #define DBNOFOUND ((SQLCODE ==100)||(SQLCODE ==1403)||(SQLCODE ==4)||(SQLCODE ==1095))
#endif
*/

struct S_BillStat_Route
{
    char inroute[8+8];
    char outroute[16];
    char stat_time[14+2];//表示统计的时段,to minute
    long number;    //该时段的话单数
};

class C_BillStat_Route
{
private:    
	int check_time(char *buf);
    void DelSpace( char * );
private:
    char source_id[ 5+3 ];
    int iIndex;
    long   hour_num;  /*标记共有多少个时段的话单被统计, 以分钟为单位*/
    struct S_BillStat_Route  *p_stat; 
    long   max_count;
public:
    C_BillStat_Route( char * ,int index);
    C_BillStat_Route(){max_count = 0;}
    ~C_BillStat_Route( );
    int Init(char *,int);
    int  AddItem( char *,char *,char *);
    //int  UpdateDB( char * ,CDatabase&);
	int  UpdateDB( char * ,DBConnection);
    int  ResetAll();
};


#endif

