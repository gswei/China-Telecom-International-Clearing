/*
* Copyright (c) 2004, 广东普信科技有限公司
* All rights reserved.
*
* bill_statistic_route.cpp
* 摘要：话单流量统计类，精确到分钟
*
* 当前版本：
* 作    者：陈祥运
* 完成日期：2004-06-14
* 更新记录：2004-07-15 当操错出错时候，要写详细信息入日志,同时入库之前检查记录有效性。
           
*/
 #include <stdio.h>
#include "bill_statistic_route.h"
//EXEC SQL INCLUDE sqlca;
//EXEC SQL INCLUDE oraca;


const char INROUTE_DEFAULT[]   = "GPTBIN";
const char OUTROUTE_DEFAULT[]   = "GPTBOUT";


C_BillStat_Route::~C_BillStat_Route()
{	
	if(p_stat!=NULL)
		{
    	delete []p_stat;
    	p_stat = NULL;
		}
}

C_BillStat_Route::C_BillStat_Route( char *_id,int index )
{
	Init(_id,index);

}

int C_BillStat_Route::Init( char *_id,int index )
{
    strcpy( source_id,_id );
    iIndex=index;
    hour_num = 0;
    if(max_count == 0)
    {
    	max_count = MAX_BILL_STAT;
    	p_stat = new S_BillStat_Route[ MAX_BILL_STAT ];
    }
}

int C_BillStat_Route::check_time(char *buf)
{
    int i,len;
    len = strlen( buf );
    for (i = 0;i < len;i++)
    {
        if (((buf[i]&0x00ff) < 48) || ((buf[i]&0x00ff) > 57)) //如果包含非数字字符
        {
            return -1;
        }
    }
    return 0;
}


int C_BillStat_Route::AddItem( char *_time ,char *inroute,char *outroute)
{
    if( p_stat == NULL )
        return -1;
    int i,found = 0;
    if(strlen(_time)>15) return 0;
    if(strlen(_time)<10) return 0;
    char time[14+2];
    //2006 05 17
    if( strlen( _time )<14 ) //非有效时间，在年份前加20
    {
      sprintf(time,"20%s",_time);
    }
    else
    {
      strcpy( time,_time );
      DelSpace( time );
    }
    if( check_time(time) )
        return 0;
    //end
    if( strlen( time )>=8 ) time[8] = 0;
    for (i = 0;i < hour_num;i++)
    {
        if((!strcmp(time, p_stat[i].stat_time))&&(!strcmp(inroute, p_stat[i].inroute))&&(!strcmp(outroute, p_stat[i].outroute)))
        {   
            p_stat[i].number++;
            found = 1;
            break;
        }
    }
    if (!found)
    {
        hour_num++;
        if( ( hour_num - 1 ) == max_count ) 
        {
            S_BillStat_Route *tmp = new S_BillStat_Route[max_count + MAX_BILL_STAT];
            max_count = max_count + MAX_BILL_STAT;
            for (i = 0;i < (hour_num - 1);i++)
            {
                strcpy(tmp[i].stat_time, p_stat[i].stat_time);
                strcpy(tmp[i].inroute, p_stat[i].inroute);
                strcpy(tmp[i].outroute, p_stat[i].outroute);
                tmp[i].number = p_stat[i].number;
            }
            delete []p_stat;
            p_stat = tmp;   
            tmp = NULL; 
        }
        strcpy( p_stat[hour_num - 1].stat_time, time );
        if(strlen(inroute)) strcpy( p_stat[hour_num - 1].inroute, inroute);
        else strcpy( p_stat[hour_num - 1].inroute, INROUTE_DEFAULT);

        if(strlen(outroute)) strcpy( p_stat[hour_num - 1].outroute, outroute);
        else strcpy( p_stat[hour_num - 1].outroute, OUTROUTE_DEFAULT);

        p_stat[hour_num - 1].number = 1;
    }
    return 0;
}

//int C_BillStat_Route::UpdateDB( char *m_log,CDatabase& DBConn)
int C_BillStat_Route::UpdateDB( char *m_log,DBConnection conn)
{
    int i;
    if( p_stat == NULL ){
    	sprintf( m_log,"the pointer p_stat is NULL!!" );
    	return -1;
    }
    //CBindSQL ds(DBConn);
//    EXEC SQL BEGIN DECLARE SECTION;
    char _source_id[5+1];
    char _time[12+1];
    char _inroute[9];
    char _outroute[9];
    int _billnum=0;
    int _billnum2;
//    EXEC SQL END DECLARE SECTION;
    strcpy( _source_id,source_id );
	
	string sql = "";
	int flag = 0;
    for( i = 0;i<hour_num;i++ )
    {
        strcpy( _time,p_stat[i].stat_time );
        strcpy( _inroute,p_stat[i].inroute );
        strcpy( _outroute,p_stat[i].outroute );
        _billnum2 = p_stat[i].number;
/*        ds.Open("SELECT BILL_COUNT FROM D_BILL_STATICS_ROUTE  "
        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time "
        "and IN_ROUTE = :inroute and OUT_ROUTE = :outroute");
		ds<<_source_id<<iIndex<<_time<<_inroute<<_outroute;
		ds>>_billnum;

        if( ds.IsEnd() ){//insert
            ds.Close();
            ds.Open("insert into D_BILL_STATICS_ROUTE VALUES( :source_id,:proc_index,:time,:inroute,:outroute,:billnum2 )",NONSELECT_DML);
			ds<<_source_id<<iIndex<<_time<<_inroute<<_outroute<<_billnum2;
            ds.Execute();    
            ds.Close();
			
            }else{//update
            _billnum2 += _billnum;
            ds.Close();
            ds.Open("UPDATE D_BILL_STATICS_ROUTE SET BILL_COUNT = :billnum2 "
	        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time "
	        "and IN_ROUTE = :inroute and OUT_ROUTE = :outroute",NONSELECT_DML);
			ds<<_billnum2<<_source_id<<iIndex<<_time<<_inroute<<_outroute;
            ds.Execute();    
            ds.Close();
            }
*/			
	      Statement stmt = conn.createStatement();
		  sql = "SELECT BILL_COUNT FROM D_BILL_STATICS_ROUTE  "
				"WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time "
				"and IN_ROUTE = :inroute and OUT_ROUTE = :outroute";
		  stmt.setSQLString(sql);
		  stmt<<_source_id<<iIndex<<_time<<_inroute<<_outroute;
		  //stmt.execute();
		  //stmt>>_billnum;
		  if(!stmt.execute())  //是否查询到值
		  {
			  sql = "insert into D_BILL_STATICS_ROUTE VALUES( :source_id,:proc_index,:time,:inroute,:outroute,:billnum2 )";
			  stmt.setSQLString(sql);
			  stmt<<_source_id<<iIndex<<_time<<_inroute<<_outroute<<_billnum2;
			  stmt.execute();
		  }else{
			  stmt>>_billnum;
			  _billnum2 += _billnum;
			  sql = "UPDATE D_BILL_STATICS_ROUTE SET BILL_COUNT = :billnum2 "
					"WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time "
					"and IN_ROUTE = :inroute and OUT_ROUTE = :outroute";
			  stmt.setSQLString(sql);
			  stmt<<_billnum2<<_source_id<<iIndex<<_time<<_inroute<<_outroute;
			  stmt.execute();
		  }
		  stmt.close();
     }

			
    return 0;	
}

int C_BillStat_Route::ResetAll()
{
    hour_num = 0;
    return 0;	
}

void C_BillStat_Route::DelSpace( char *ss )
{
    int i,len,j;
    i = strlen(ss)-1;
    while (i && ss[i] == ' ') i--;
    ss[i+1] = 0;
    i = 0;
    len = strlen(ss);
    while((i<len)&&ss[i]==' ')i++;
    if(i!=0)
    {
        for(j = i;j<len;j++)
        {
           ss[j-i] = ss[j];
        }
    }
    ss[len-i] = 0;	
}






