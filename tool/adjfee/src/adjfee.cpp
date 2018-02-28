/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2014-02-07
File:			 adjfee.cpp
Description:	 调整费用
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "adjfee.h"
CLog theJSLog;
SGW_RTInfo rtinfo;
short petri_status = -1; 
using namespace tpss;

Cadjfee::Cadjfee()
{
}

Cadjfee::~Cadjfee()
{
}

bool Cadjfee::testSQL(char *sourceid,char *adjdate)
{
    char tmp_date[9];
    char source_id[9];
    int adjtotalno;  //adj总数
    int adj_no; // adj序号
    int tmp_no=0; //计数器
    //char adj_no[5];

    char base_repid[30];
    int base_sheetid,base_col,base_row;
    char adj_repid[30];
    int adj_sheetid,adj_col,adj_row;

    double base_value;
    double adj_value;
    
    strcpy(tmp_date,adjdate);
    strcpy(source_id,sourceid);
    
    theJSLog << "当前调整账期为: " <<tmp_date<<endd;
    try{	
   	if ( dbConnect(conn) )
   		{
   		 Statement stmt = conn.createStatement();
   		 Statement stmt_adj = conn.createStatement();
	     char szsql[1000];
	     sprintf(szsql,"select count(*) from C_ADJREPORT_DEFINE where adj_group='%s'",source_id);
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>adjtotalno;
		 //cout<<adjtotalno<<endl;

		 sprintf(szsql,"select adj_no from C_ADJREPORT_DEFINE where adj_group='%s'",source_id);
         stmt.setSQLString(szsql);  
         //theJSLog << szsql <<endd;
		 stmt.execute();
		 while(stmt >> adj_no)
		 {
		   theJSLog << "adj_no = " << adj_no<<endd;
		   dealAdj(source_id,adj_no,tmp_date);	
		 }	
		 addLog(source_id,tmp_date);
	     conn.close();
	   }
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "调整结算金额出错", __FILE__, __LINE__);
    }
	return true;
}

bool Cadjfee::testUpdate(char *sourceid,int adj_no,char *adjdate)
{
    char org_fee[14];
    char tmp_date[9];
    char source_id[9];
    double fee_tmp;
    double realfee;
    char adj_table[30];
    char adj_param[30];
    char condition[400];
    
    strcpy(tmp_date,adjdate);
    strcpy(source_id,sourceid);
    theJSLog << "需要调整的费用值为" <<adj_fee<<endd;
    try{	
   //	if ( dbConnect(conn) )
   		//{
   		 Statement stmt = conn.createStatement();
	     char szsql[4000];

         sprintf(szsql,"select adj_table,adj_param,condition from C_ADJREPORT_DEFINE where ADJ_GROUP = '%s' and adj_no=%d",source_id,adj_no);
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 //theJSLog << szsql <<endd;
		 stmt>>adj_table>>adj_param>>condition;

		 //先获取原始数据中满足条件的初始数据
	     sprintf(szsql,"select %s from %s%s where rowid = "
	        "(select max(rowid) from %s%s where %s)",adj_param,adj_table,adjdate,adj_table,adjdate,condition);
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 //theJSLog << szsql <<endd;
		 stmt>>fee_tmp;  		 
		 theJSLog <<"fee_tmp = " << fee_tmp <<endd;	
		 realfee = fee_tmp + adj_fee;
		 theJSLog << "realfee = " << realfee <<endd;	

         sprintf(szsql,"update %s%s set %s = %f where rowid = "
	        "(select max(rowid) from %s%s where %s)",
	        adj_table,tmp_date,adj_param,realfee,adj_table,tmp_date,condition);         
		 stmt.setSQLString(szsql);   
		 theJSLog << szsql <<endd;
		 stmt.execute();
		 stmt.commit();
		 //conn.close();
	   //}
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "调整结算金额出错", __FILE__, __LINE__);
    }
	return true;
}

bool Cadjfee::dealAdj(char *sourceid,int adjno,char *adj_date)
{
    char tmp_date[9];
    char source_id[9];
    int adjtotalno;  //adj总数
    int adj_no; // adj序号
    int tmp_no=0; //计数器

    char base_repid[30];
    int base_sheetid,base_col,base_row;
    char adj_repid[30];
    int adj_sheetid,adj_col,adj_row;

    double base_value=0.0;
    double adj_value=0.0;
    
    adj_no = adjno;
    strcpy(tmp_date,adj_date);
    strcpy(source_id,sourceid);
    char szsql[1000];

    try{
    	 //dbConnect(conn);
         Statement stmt = conn.createStatement();   	 		               
		   
		   sprintf(szsql,"select base_repid,base_sheetid,base_col,base_row,adj_repid,adj_sheetid,adj_col,adj_row from C_ADJREPORT_DEFINE where adj_group='%s' and adj_no =%d",source_id,adj_no);
           stmt.setSQLString(szsql); 
           //theJSLog << szsql <<endd;
		   stmt.execute();		 
		   stmt>>base_repid>>base_sheetid>>base_col>>base_row>>adj_repid>>adj_sheetid>>adj_col>>adj_row;
		   theJSLog << "输出内容"<<base_repid<<base_sheetid<<base_col<<base_row<<adj_repid<<adj_sheetid<<adj_col<<adj_row<<endd;          

          //执行基础报表内容
           char stat_sql[30];    
           sprintf(stat_sql,"begin exec_sheet('%s',%d,'%s'); end;",base_repid,base_sheetid,tmp_date);
           theJSLog<< "base_sql = " << stat_sql<<endd;
           Statement stmt1 = conn.createStatement(stat_sql); 
           stmt1.execute(); 

           //执行调整报表内容  
           sprintf(stat_sql,"begin exec_sheet('%s',%d,'%s'); end;",adj_repid,adj_sheetid,tmp_date);
           theJSLog<< "adj_sql = " << stat_sql<<endd;
           stmt1 = conn.createStatement(stat_sql); 
           stmt1.execute(); 

         //获取基础报表内容数据
		 if(base_row != -1)  //有配置最大行
         {		      
	        sprintf(szsql,"select COLNO%d from D_SHEET_DATA where report_id ='%s' and sheet_no =%d and rate_date='%s' and row_no=%d",base_col,base_repid,base_sheetid,tmp_date,base_row);
            stmt.setSQLString(szsql);         
		    stmt.execute();
		    theJSLog << szsql <<endd;
            while(stmt>>base_value)
            {
              theJSLog << "base_value = " << base_value <<endd;  
              //printf("base_value = %lf ",base_value);
            }
         } else   //无配置最大行
         {
           sprintf(szsql,"select sum(COLNO%d) from D_SHEET_DATA where report_id ='%s' and sheet_no =%d and rate_date='%s'",base_col,base_repid,base_sheetid,tmp_date);
            stmt.setSQLString(szsql);         
		    stmt.execute();
		    theJSLog << szsql <<endd;
            while(stmt>>base_value)
            {
              theJSLog << "base_value = " << base_value <<endd;
              //printf("base_value = %lf ",base_value);
            }
         }
            //获取基础报表内容数据
           if(adj_row != -1)  //有配置最大行
          {		      
	        sprintf(szsql,"select COLNO%d from D_SHEET_DATA where report_id ='%s' and sheet_no =%d and rate_date='%s' and row_no=%d",adj_col,adj_repid,adj_sheetid,tmp_date,adj_row);
            stmt.setSQLString(szsql); 
		    stmt.execute();
		    theJSLog << szsql <<endd;
            while(stmt>>adj_value)
            {
              theJSLog << "adj_value = " << adj_value <<endd;  
              //printf("adj_value = %lf ",adj_value);
            }
         } else   //无配置最大行
         {
           sprintf(szsql,"select sum(COLNO%d) from D_SHEET_DATA where report_id ='%s' and sheet_no =%d and rate_date='%s'",adj_col,adj_repid,adj_sheetid,tmp_date);
            stmt.setSQLString(szsql);         
		    stmt.execute();
		    theJSLog << szsql <<endd;
            while(stmt>>adj_value)
            {
              theJSLog << "adj_value = " << adj_value <<endd; 
              //printf("adj_value = %lf ",adj_value);
            }           
         }
         
            //处理差异                        
            adj_fee = base_value - adj_value;
            //adj_fee = adj_value - base_value;
            theJSLog << "需要调整的费用adj_fee = " << adj_fee <<endd;
            testUpdate(source_id,adj_no,tmp_date);
            ///处理完成                  
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "调整结算金额出错", __FILE__, __LINE__);
    }
     return true;
}

bool Cadjfee::addLog(char *sourceid,char *adjdate)
{
    char tmp_date[9];
    char source_id[9];
    
    strcpy(tmp_date,adjdate);
    strcpy(source_id,sourceid);
    try{	
   		 Statement stmt = conn.createStatement();
	     char szsql[4000];

         sprintf(szsql,"insert into D_ADJFEE_LOG(adj_group,deal_date,adj_cycle,deal_flag,adj_fee) VALUES('%s',to_char(sysdate,'yyyymmddhh24miss'),'%s','Y','%s'"
         	         ,source_id,tmp_date,adj_fee);
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt.commit();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "调整结算金额出错", __FILE__, __LINE__);
    }
	return true;
}

int main(int argc,char**argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    jsadjfee               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2014-02-07 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;	

    Cadjfee adj; 
    std::string log_path,log_level;
    bool result;

   	if ( (argc != 2) && ( argc != 3 ) )
	{
	    printf("Usage : %s YYYYMMDD\n",argv[0]);
	    printf("P.S : jsadjfee GJGW 201312 \n");
		return false;
	}
   		
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "获取日志路径失败" );
	}

	if(argc == 3)  //输入参数包括数据源和时间
	{
	   strcpy(adj.source_id,argv[1]);
	   strcpy(adj.adj_time,argv[2]);
   	   theJSLog << "adj.source_id = " << adj.source_id <<endd;
   	   theJSLog << "adj.check_time = " << adj.adj_time <<endd;
	   theJSLog.setLog(log_path.c_str(), 1,adj.adj_time ,"ADJ", 1);
	   result = adj.testSQL(adj.source_id,adj.adj_time);
	}
	
    if(result == true)
    	theJSLog << adj.adj_time <<"  Adj fee success !" <<endi;
    else
    	theJSLog << adj.adj_time <<"  Adj fee Fail !" <<endi;
   return 0;
}

