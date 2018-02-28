



/*
* Copyright (c) 2004, 广东普信科技有限公司
* All rights reserved.
*
* bill_statistic.cpp
* 摘要：话单流量统计类，精确到分钟
*
* 当前版本：
* 作    者：陈祥运
* 完成日期：2004-06-14
* 更新记录：2004-07-15 当操错出错时候，要写详细信息入日志,同时入库之前检查记录有效性。
           
*/
#include <stdio.h>
#include "bill_statistic_filenametime.h"
//EXEC SQL INCLUDE sqlca;
//EXEC SQL INCLUDE oraca;
/*
1.20060802 修改查找算法。提高效率

*/
C_BillStat_fnTime::~C_BillStat_fnTime()
{	
	if(p_stat!=NULL)
		{
    	delete []p_stat;
    	p_stat = NULL;
		}
}

C_BillStat_fnTime::C_BillStat_fnTime( char *_id,int index)
{

	Init(_id,index);
}

int C_BillStat_fnTime::Init( char *_id,int index )
{

    strcpy( source_id,_id );
    iIndex=index;
    hour_num = 0;
    if(max_count == 0)
    {
    	max_count = MAX_BILL_STAT;
   		p_stat = new S_BillStat_fnTime[ MAX_BILL_STAT ];
    }

    int i,j;
    for(i=0;i<2;i++)
    {
      for(j =0;j<MAX_H_BILL_STAT;j++)
      {
        p_H_stat[i][j].stat_time[0]=0;
        p_H_stat[i][j].number =0;
        p_H_stat[i][j].iExist=0;
      }
    }
}

int C_BillStat_fnTime::GetTime()
{
 time_t  timer;
 struct tm nowtimer;
 time( &timer ); 
 nowtimer = *localtime ( &timer ); nowtimer.tm_mon++;
 sprintf( Month[0],"%04u%02u", nowtimer.tm_year+1900,nowtimer.tm_mon);
 sprintf(Month[1],"%s",Month[0]);
 if(!strncmp(Month[1]+4,"01",2))
 {
   Month[1][3] -= 1;
   Month[1][4] = '1';
   Month[1][5] = '2';
 }
 else
 {
   Month[1][5] -= 1;
 }
 
 return(0);
}


int C_BillStat_fnTime::check_time(char *buf)
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


int C_BillStat_fnTime::AddItem( char *_time )
{
    if( p_stat == NULL )
        return -1;
    int i,found = 0;
    if(strlen(_time)>15) return 0;
    char time[14+2];
    strcpy( time,_time );
    DelSpace( time );
    if( check_time(time) )
        return 0;
    //end
    if( strlen( time )>=8 ) time[8] = 0;


    if(!strncmp(Month[0],time,6)) found = 1;
    if(!strncmp(Month[1],time,6)) found = 2;
    
    if(found)
    {
      char Day[3];
      char Hour[3];
      char Minu[3];
      Day[0] = time[6];
      Day[1] = time[7];
      Day[2] = 0;
      Hour[0] = time[8];
      Hour[1] = time[9];
      Hour[2] = 0;
      Minu[0] = time[10];
      Minu[1] = time[11];
      Minu[2] = 0;
      int Index = (atoi(Day)-1)*24*60 + atoi(Hour)*60 + atoi(Minu);
      if(p_H_stat[found-1][Index].iExist==0)
      {
        p_H_stat[found-1][Index].number = 1;
        strcpy( p_H_stat[found-1][Index].stat_time, time );
        p_H_stat[found-1][Index].iExist=1;
      }
      else
      {
        p_H_stat[found-1][Index].number += 1;
      }
      return 0;
    }

    
    for (i = 0;i < hour_num;i++)
    {
        if ( strcmp(time, p_stat[i].stat_time) == 0 )
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
            S_BillStat_fnTime *tmp = new S_BillStat_fnTime[max_count + MAX_BILL_STAT];
            max_count = max_count + MAX_BILL_STAT;
            for (i = 0;i < (hour_num - 1);i++)
            {
                strcpy(tmp[i].stat_time, p_stat[i].stat_time);
                tmp[i].number = p_stat[i].number;
            }
            delete []p_stat;
            p_stat = tmp;   
            tmp = NULL; 
        }
        strcpy( p_stat[hour_num - 1].stat_time, time );
        p_stat[hour_num - 1].number = 1;
    }
    return 0;
}

//int C_BillStat_fnTime::UpdateDB( char *m_log,CDatabase& DBConn)
int C_BillStat_fnTime::UpdateDB( char *m_log,DBConnection conn)
{
    int i,j;
    if( p_stat == NULL ){
    	sprintf( m_log,"the pointer p_stat is NULL!!" );
    	return -1;
    }
    //CBindSQL ds(DBConn);
//    EXEC SQL BEGIN DECLARE SECTION;
    char _source_id[5+1];
    char _time[12+1];
    int _billnum=0;
    int _billnum2;
//    EXEC SQL END DECLARE SECTION;
    strcpy( _source_id,source_id );

	string sql = "";
	//int cout = 0;  //受影响行数

    for( i = 0;i<hour_num;i++ )
    {
        strcpy( _time,p_stat[i].stat_time );
        _billnum2 = p_stat[i].number;

//        ds.Open("UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = BILL_COUNT+:billnum2 "
//	        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time",NONSELECT_DML);
//    	ds<<_billnum2<<_source_id<<iIndex<<_time;
//        ds.Execute();    
//        if( !ds.GetUpdateCount() ){//insert
//            ds.Close();
//            ds.Open("insert into D_BILL_STATICS_FNTIME VALUES( :source_id,:proc_index,:time,:billnum2 )",NONSELECT_DML);
//			ds<<_source_id<<iIndex<<_time<<_billnum2;
//            ds.Execute();    
//            ds.Close();
//			
//            }else{//update
//            ds.Close();
//            }
		//count = 0;
		Statement stmt = conn.createStatement();
		//sql = "select count(*) from D_BILL_STATICS_FNTIME WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
		//stmt.setSQLString(sql);
		//stmt<<_source_id<<iIndex<<_time;
		//stmt.execute();
		//stmt>>count;
		sql = "UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = BILL_COUNT+:billnum2 "
					 "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";		
		stmt.setSQLString(sql);
		stmt<<_billnum2<<_source_id<<iIndex<<_time;
		if(!stmt.execute())
		{	
			sql = "insert into D_BILL_STATICS_FNTIME VALUES( :source_id,:proc_index,:time,:billnum2 )";
			stmt.setSQLString(sql);
			stmt<<_source_id<<iIndex<<_time<<_billnum2;
			stmt.execute();
		}
		stmt.close();

/*        
        ds.Open("SELECT BILL_COUNT FROM BILL_STATICS  "
        "WHERE SOURCE_ID = :source_id and STATICS_TIME = :time");
		ds<<_source_id<<_time;
		ds>>_billnum;

        if( ds.IsEnd() ){//insert
            ds.Close();
            ds.Open("insert into BILL_STATICS VALUES( :source_id,:time,:billnum2 )",NONSELECT_DML);
			ds<<_source_id<<_time<<_billnum2;
            ds.Execute();    
            ds.Close();
			
            }else{//update
            _billnum2 += _billnum;
            ds.Close();
            ds.Open("UPDATE BILL_STATICS SET BILL_COUNT = :billnum2 "
	        "WHERE SOURCE_ID = :source_id and STATICS_TIME = :time",NONSELECT_DML);
			ds<<_billnum2<<_source_id<<_time;
            ds.Execute();    
            ds.Close();
            }
*/

        }
        
  for(i=0;i<2;i++)
  {
    for( j = 0;j<MAX_H_BILL_STAT;j++ )
    {
        if(p_H_stat[i][j].iExist==0) continue;
        
        strcpy( _time,p_H_stat[i][j].stat_time );
        _billnum2 = p_H_stat[i][j].number;



		
//        ds.Open("UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = BILL_COUNT+:billnum2 "
//	        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time",NONSELECT_DML);
//    	ds<<_billnum2<<_source_id<<iIndex<<_time;
//        ds.Execute();    
//        if( !ds.GetUpdateCount() ){//insert
//            ds.Close();
//            ds.Open("insert into D_BILL_STATICS_FNTIME VALUES( :source_id,:proc_index,:time,:billnum2 )",NONSELECT_DML);
//			ds<<_source_id<<iIndex<<_time<<_billnum2;
//            ds.Execute();    
//            ds.Close();
//			
//            }else{//update
//            ds.Close();
//            }
		//count = 0;
		Statement stmt = conn.createStatement();
		sql = "UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = BILL_COUNT+:billnum2 ";
					   "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
		stmt.setSQLString(sql);
		stmt<<_billnum2<<_source_id<<iIndex<<_time;
		if(!stmt.execute())
		{
				sql = "insert into D_BILL_STATICS_FNTIME VALUES( :source_id,:proc_index,:time,:billnum2 )";
				stmt.setSQLString(sql);
				stmt<<_source_id<<iIndex<<_time<<_billnum2;
				stmt.execute();
		}
		 stmt.close();
/*        
        ds.Open("SELECT BILL_COUNT FROM BILL_STATICS  "
        "WHERE SOURCE_ID = :source_id and STATICS_TIME = :time");
		ds<<_source_id<<_time;
		ds>>_billnum;

        if( ds.IsEnd() ){//insert
            ds.Close();
            ds.Open("insert into BILL_STATICS VALUES( :source_id,:time,:billnum2 )",NONSELECT_DML);
			ds<<_source_id<<_time<<_billnum2;
            ds.Execute();    
            ds.Close();
			
            }else{//update
            _billnum2 += _billnum;
            ds.Close();
            ds.Open("UPDATE BILL_STATICS SET BILL_COUNT = :billnum2 "
	        "WHERE SOURCE_ID = :source_id and STATICS_TIME = :time",NONSELECT_DML);
			ds<<_billnum2<<_source_id<<_time;
            ds.Execute();    
            ds.Close();
            }

*/  
        }
  }
    return 0;	
}

int C_BillStat_fnTime::ResetAll()
{
    hour_num = 0;
    int i,j;
    for(i=0;i<2;i++)
    {
      for(j =0;j<MAX_H_BILL_STAT;j++)
      {
        p_H_stat[i][j].stat_time[0]=0;
        p_H_stat[i][j].number =0;
        p_H_stat[i][j].iExist=0;
      }
    }
}

void C_BillStat_fnTime::DelSpace( char *ss )
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



//int C_BillStat_fnTime::DelUpdateDB( char *m_log,CDatabase& DBConn)
int C_BillStat_fnTime::DelUpdateDB( char *m_log,DBConnection conn)
{
    int i,j;
    if( p_stat == NULL ){
    	sprintf( m_log,"the pointer p_stat is NULL!!" );
    	return -1;
    }
   // CBindSQL ds(DBConn);
//    EXEC SQL BEGIN DECLARE SECTION;
    char _source_id[5+1];
    char _time[12+1];
    int _billnum=0;
    int _billnum2;
//    EXEC SQL END DECLARE SECTION;
    strcpy( _source_id,source_id );



	string sql = "";
    for( i = 0;i<hour_num;i++ )
    {
        strcpy( _time,p_stat[i].stat_time );
        _billnum2 = p_stat[i].number;
        
//        ds.Open("SELECT BILL_COUNT FROM D_BILL_STATICS_FNTIME  "
//        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time");
//		ds<<_source_id<<iIndex<<_time;
//		ds>>_billnum;
//        ds.Close();
		sql = "SELECT BILL_COUNT FROM D_BILL_STATICS_FNTIME  "
			  "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
		Statement stmt  = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<_source_id<<iIndex<<_time;
		//stmt.execute();
		//stmt>>_billnum;

        //if( ds.IsEnd() )
		if(!stmt.execute())
        {
            sprintf(m_log,"SELECT NONE FROM D_BILL_STATICS_FNTIME WHERE SOURCE_ID = '%s' and proc_index = %d and STATICS_TIME = '%s'",_source_id,iIndex,_time);
            return -1;
        }
        else
        {//update
		  stmt>>_billnum;
          if(_billnum < _billnum2)
          {
            sprintf(m_log,"Bill_count from D_BILL_STATICS_FNTIME where source_id = '%s' and proc_index = %d and statics_time = '%s' is %d , Less than %d from file",_source_id,iIndex,_time,_billnum,_billnum2);
            return -1;
          }
          if(_billnum == _billnum2)
          {
//            ds.Open("delete D_BILL_STATICS_FNTIME WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time",NONSELECT_DML);
// 			ds<<_source_id<<iIndex<<_time;
//            ds.Execute();    
//            ds.Close();
			sql = "delete D_BILL_STATICS_FNTIME WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
			stmt.setSQLString(sql);
			stmt<<_source_id<<iIndex<<_time;
			stmt.execute();
			stmt.close();
            continue;
           }
           else 
           {
            _billnum -= _billnum2;
//            ds.Open("UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = :billnum2 "
//	        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time",NONSELECT_DML);
//			ds<<_billnum<<_source_id<<iIndex<<_time;
//            ds.Execute();
//            ds.Close();
			sql = "UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = :billnum2 "
				  "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
		    stmt.setSQLString(sql);
			stmt<<_billnum<<_source_id<<iIndex<<_time;
			stmt.execute();
			stmt.close();
           }
        }
    }


  for(i=0;i<2;i++)
  {
    for( j = 0;j<MAX_H_BILL_STAT;j++ )
    {
        if(p_H_stat[i][j].iExist==0) continue;
        
        strcpy( _time,p_H_stat[i][j].stat_time );
        _billnum2 = p_H_stat[i][j].number;

        
//        ds.Open("SELECT BILL_COUNT FROM D_BILL_STATICS_FNTIME  "
//        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time");
//		ds<<_source_id<<iIndex<<_time;
//		ds>>_billnum;
//        ds.Close();
		
		Statement stmt  = conn.createStatement();
		sql = "SELECT BILL_COUNT FROM D_BILL_STATICS_FNTIME  "
			  "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
		stmt.setSQLString(sql);
		stmt<<_source_id<<iIndex<<_time;
		//stmt.execute();
		//stmt>>_billnum;

        //if( ds.IsEnd() )
		if(!stmt.execute())
        {
            sprintf(m_log,"SELECT NONE FROM D_BILL_STATICS_FNTIME WHERE SOURCE_ID = '%s' and proc_index = %d and STATICS_TIME = '%s'",_source_id,iIndex,_time);
            return -1;
        }
        else
        {//update
		  stmt>>_billnum;
          if(_billnum < _billnum2)
          {
            sprintf(m_log,"Bill_count from D_BILL_STATICS_FNTIME where source_id = '%s' and proc_index = %d and statics_time = '%s' is %d , Less than %d from file",_source_id,iIndex,_time,_billnum,_billnum2);
            return -1;
          }
          if(_billnum == _billnum2)
          {
//            ds.Open("delete D_BILL_STATICS_FNTIME WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time",NONSELECT_DML);
// 			ds<<_source_id<<iIndex<<_time;
//            ds.Execute();    
//            ds.Close();
			sql = "delete D_BILL_STATICS_FNTIME WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
			stmt.setSQLString(sql);
			stmt<<_source_id<<iIndex<<_time;
			stmt.execute();
			stmt.close();
            continue;
           }
           else 
           {
            _billnum -= _billnum2;
//            ds.Open("UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = :billnum2 "
//	        "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time",NONSELECT_DML);
//			ds<<_billnum<<_source_id<<iIndex<<_time;
//            ds.Execute();
//            ds.Close();
			sql = "UPDATE D_BILL_STATICS_FNTIME SET BILL_COUNT = :billnum2 "
				  "WHERE SOURCE_ID = :source_id and proc_index = :proc_index and STATICS_TIME = :time";
			stmt.setSQLString(sql);
			stmt<<_billnum<<_source_id<<iIndex<<_time;
			stmt.execute();
			stmt.close();
           }
        }

    }
  }


    return 0;	
}










