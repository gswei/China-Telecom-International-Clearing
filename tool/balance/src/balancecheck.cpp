/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-12-13
File:			 balancecheck.cpp
Description:	 平衡检查
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "balancecheck.h"
CLog theJSLog;
SGW_RTInfo rtinfo;
short petri_status = -1; 
using namespace tpss;

Cbalance::Cbalance()
{
}

Cbalance::~Cbalance()
{
}

//输入参数有数据源和时间的处理
bool Cbalance::checkDataforsource(char *sourceid,char *checkdate)
{
    char tmp_date[9];
    char source_id[9];
    int infile_num;      //核对文件中进入系统的总文件数
    int indbfile_num=0;   //入库总文件数
    int indbrcd_num=0;   //入库总记录数
    int errformat=0;
    int errindb=0;
    int errwrtf=0;
    int errfile_num=0;    //错误文件数
    int nofile_num=0;    //无记录的文件数
    
    strcpy(tmp_date,checkdate);
    strcpy(source_id,sourceid);
    
    theJSLog << "当前检查平衡日期为: " <<tmp_date<<endd;
    try{	
   	if ( dbConnect(conn) )
   		{
   		 Statement stmt = conn.createStatement();
	     char szsql[4000];

         //先检查表中是否有当天数据,如果有则删除当天数据
	     sprintf(szsql,"delete from D_BALANCE_CHECK where check_date = '%s' and sourceid ='%s'",tmp_date,source_id);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt.commit();       
		 	
		 //进入系统的文件总数
		 sprintf(szsql,"select count(*) from D_CHECK_FILE_DETAIL where file_time = '%s' and source_id ='%s' and check_flag ='Y' and check_type='AUD'",tmp_date,source_id);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>infile_num;

		 //入库总文件数
		 sprintf(szsql,"select count(*) from D_SCH_INDB where file_time = '%s' and source_id ='%s' and deal_flag ='Y'",tmp_date,source_id);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>indbfile_num;
		 
		 //入库总记录数
		 sprintf(szsql,"select sum(mainflow_count) from D_SCH_INDB where file_time = '%s' and source_id ='%s' and deal_flag ='Y'",tmp_date,source_id);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>indbrcd_num;
		 
		 //错误文件数，包括格式化、写文件和入库三部分
		 sprintf(szsql,"select count(*) from d_sch_format where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E'",source_id,tmp_date);
		 theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>errformat;

		 sprintf(szsql,"select count(*) from d_sch_wrtf where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E'",source_id,tmp_date);
		 theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>errwrtf;

		 sprintf(szsql,"select count(*) from d_sch_indb where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E'",source_id,tmp_date);
		 theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>errindb;

		 //汇总总数
		 errfile_num = errformat + errwrtf + errindb;
		 theJSLog << "错误文件总数为" <<errfile_num<<endd;
	
		 //无记录的文件数
		 sprintf(szsql,"select count(*) from d_sch_format where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'Y' and record_count = 0",source_id,tmp_date);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>nofile_num;

		 sprintf(szsql,"insert into D_BALANCE_CHECK(sourceid,check_date,in_file_num,indb_file_num,indb_rcd_num,err_file_num,no_file_num) "
		 	"values('%s','%s',%d,%d,%d,%d,%d)",source_id,tmp_date,infile_num,indbfile_num,indbrcd_num,errfile_num,nofile_num);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
	    
	   }
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "删除固网结算公式数据出错", __FILE__, __LINE__);
    }
	return true;
}

//输入参数只有时间的处理
bool Cbalance::checkData(char *checkdate)
{
    char tmp_date[9];
    char source_id[9];
    int infile_num;      //核对文件中进入系统的总文件数
    int indbfile_num=0;   //入库总文件数
    int indbrcd_num=0;   //入库总记录数
    int errformat=0;
    int errindb=0;
    int errwrtf=0;
    int errfile_num=0;    //错误文件数
    int nofile_num=0;    //无记录的文件数
    int source_num;
    
    strcpy(tmp_date,checkdate);
    
    theJSLog << "当前检查平衡日期为: " <<tmp_date<<endd;
    try{	
   	if ( dbConnect(conn) )
   		{
   		 Statement stmt = conn.createStatement();
   		 Statement m_stmt = conn.createStatement();
	     char szsql[4000];

         //先检查表中是否有当天数据
	     sprintf(szsql,"delete from D_BALANCE_CHECK where check_date = '%s'",tmp_date);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt.commit();       

	     sprintf(szsql,"select source_id from I_SOURCE_DEFINE");
         theJSLog << szsql <<endd;
         m_stmt.setSQLString(szsql);         
		 m_stmt.execute();
		 while(m_stmt>>source_id)
		 {
		    //进入系统的文件总数
		 sprintf(szsql,"select count(*) from D_CHECK_FILE_DETAIL where file_time = '%s' and source_id ='%s' and check_flag ='Y' and check_type='AUD'",tmp_date,source_id);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>infile_num;

		 //入库总文件数
		 sprintf(szsql,"select count(*) from D_SCH_INDB where file_time = '%s' and source_id ='%s' and deal_flag ='Y'",tmp_date,source_id);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>indbfile_num;
		 
		 //入库总记录数
		 sprintf(szsql,"select sum(mainflow_count) from D_SCH_INDB where file_time = '%s' and source_id ='%s' and deal_flag ='Y'",tmp_date,source_id);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>indbrcd_num;
		 
		 //错误文件数，包括格式化、写文件和入库三部分
		 sprintf(szsql,"select count(*) from d_sch_format where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E'",source_id,tmp_date);
		 theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>errformat;

		 sprintf(szsql,"select count(*) from d_sch_wrtf where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E'",source_id,tmp_date);
		 theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>errwrtf;

		 sprintf(szsql,"select count(*) from d_sch_indb where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E'",source_id,tmp_date);
		 theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>errindb;

		 //汇总总数
		 errfile_num = errformat + errwrtf + errindb;
		 theJSLog << "错误文件总数为" <<errfile_num<<endd;
	
		 //无记录的文件数
		 sprintf(szsql,"select count(*) from d_sch_format where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'Y' and record_count = 0",source_id,tmp_date);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 stmt>>nofile_num;

		 sprintf(szsql,"insert into D_BALANCE_CHECK(sourceid,check_date,in_file_num,indb_file_num,indb_rcd_num,err_file_num,no_file_num) "
		 	"values('%s','%s',%d,%d,%d,%d,%d)",source_id,tmp_date,infile_num,indbfile_num,indbrcd_num,errfile_num,nofile_num);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();

		 theJSLog << "数据源" << source_id <<"处理完成" <<endd;
		 }
		 
		 
	   }
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "删除固网结算公式数据出错", __FILE__, __LINE__);
    }
	return true;
}
int main(int argc,char**argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    jsbalance               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2013-12-13 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;	

    Cbalance balance; 
    std::string log_path,log_level;
    bool result;

   	if ( (argc != 2) && ( argc != 3 ) )
	{
	    printf("Usage : %s [source_id] YYYYMMDD\n",argv[0]);
	    printf("P.S : jsbalance IICC 20131230 \n");
		return false;
	}
   		
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "获取日志路径失败" );
	}

	if(argc == 3)  //输入参数包括数据源和时间
	{
	   strcpy(balance.source_id,argv[1]);
	   strcpy(balance.check_time,argv[2]);
   	   theJSLog << "balance.source_id = " << balance.source_id <<endd;
   	   theJSLog << "balance.check_time = " << balance.check_time <<endd;
	   theJSLog.setLog(log_path.c_str(), 1,balance.check_time ,"BALANCE", 1);
	   result = balance.checkDataforsource(balance.source_id,balance.check_time);
	}
	else  //输入参数只有时间
	{
	   strcpy(balance.check_time,argv[1]);
   	   theJSLog << "balance.check_time = " << balance.check_time <<endd;
	   theJSLog.setLog(log_path.c_str(), 1,balance.check_time ,"BALANCE", 1);
	   result = balance.checkData(balance.check_time);
	}

    if(result == true)
    	theJSLog << balance.check_time <<"  Balance Check success !" <<endi;
    else
    	theJSLog << balance.check_time <<"  Balance Check   Fail !" <<endi;
   return 0;
}

