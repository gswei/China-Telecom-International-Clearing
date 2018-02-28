/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-11-13
File:			 CSettle.cpp
Description:	 数据短信结算模块
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "CSettle.h"
CLog theJSLog;
SGW_RTInfo rtinfo;
short petri_status = -1; 
using namespace tpss;

CSettle::CSettle()
{
}

CSettle::~CSettle()
{
}

bool CSettle::DoforIIXC(char* rate_cycle)
{
    char tmp_cycle[8] = {0};
    strcpy(tmp_cycle,rate_cycle);
    //cout << tmp_cycle <<endl;
    try{			
	if ( dbConnect(conn) )
	 {
	       //先将表格中的该账期数据删除
			Statement stmt = conn.createStatement();
	        char szsql[4000];	
	        sprintf(szsql,"delete from D_C1X_ACCOUNT_TMP_%s where DIRECTION =1 and settle_month = '%s' ",tmp_cycle,tmp_cycle);
	        stmt.setSQLString(szsql);
			stmt.execute();

	        string sql = "INSERT INTO D_C1X_ACCOUNT_TMP_%s("
                   "SETTLE_CARRIER_ID,"                  
                   "SETTLE_MONTH,"
                   "DIRECTION,"
                   "CURRENCY ,"
                   "SETTLE_UNIT,"
                   "SETTLE_RATE,"
                   "EXCHANGE_RATE,"
                   "TOTAL_DATA_FLOW,"
                   "SETTLE_FEE_RMB,"
                   "SETTLE_FEE_SDR ,"
                   "CREATE_TIME) "
                   "SELECT C.CUST_ID, '%s' ,1, 'USD',1048576,1,D.rate,"
                   "sum(A.MDR_CHARGED_VOLUME),sum(A.TOTAL_CHARGES)*1000*D.RATE,"
                   "sum(A.TOTAL_CHARGES)*1000,to_char(SYSDATE,'yyyymmddhh24miss') "
            "FROM D_C1X_DETAIL_ACCOUNT A,BSID B,STT_OBJECT C,EXCHANGE_RATE D "
            "WHERE A.DiRECTION = 1 AND A.SETTLE_MONTH= '%s' AND "
                   "A.SID =  B.BSID AND "
                   "B.CARRiER_CD = C.CUST_CODE AND "
                   "C.EXP_DATE > SYSDATE "
                   "AND A.CURRENCY = 'USD' AND " 
                   "d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' and D.src_currency='USD' and D.dest_currency='RMB' "
                   "group by c.cust_id,d.rate";	        
	        sprintf(szsql,sql.c_str(),tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle); 
	        
			stmt.setSQLString(szsql);
			//cout << szsql <<endl;
			stmt.execute();
			//stmt.commit();


			sprintf(szsql, "INSERT INTO D_C1X_ACCOUNT_TMP_%s("
                   "SETTLE_CARRIER_ID,"
                   "SETTLE_MONTH,"
                   "DIRECTION ,"
                   "CURRENCY,"
                   "SETTLE_UNIT,"
                   "SETTLE_RATE ,"
                   "EXCHANGE_RATE,"
                   "TOTAL_DATA_FLOW,"
                   "SETTLE_FEE_RMB ,"
                   "SETTLE_FEE_SDR ,"
                   "CREATE_TIME) "
        " SELECT C.CUST_ID, '%s' ,1, 'RMB',1048576,1,D.rate,"
                  " sum(A.MDR_CHARGED_VOLUME),sum(A.TOTAL_CHARGES)*1000,"
                  " sum(A.TOTAL_CHARGES)*1000/D.RATE,to_char(SYSDATE,'yyyymmddhh24miss') "
       "  FROM D_C1X_DETAIL_ACCOUNT A,BSID B,STT_OBJECT C,EXCHANGE_RATE D "
                "WHERE A.DiRECTION = 1 AND A.SETTLE_MONTH='%s' AND "
                       "A.SID =  B.BSID AND "
                       "B.CARRiER_CD = C.CUST_CODE AND "
                       "C.EXP_DATE > SYSDATE "
                       "AND A.CURRENCY = 'RMB' AND "
                       "d.rate_date =to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' and D.src_currency='USD' and D.dest_currency='RMB'"
                     "group by c.cust_id,d.rate",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle);
			stmt.setSQLString(szsql);
			//cout << szsql <<endl;
			stmt.execute();
			conn.commit();
			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }	 
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "处理IIXC 数据出错", __FILE__, __LINE__);
    }
	return true;
}

bool CSettle::DoIIXCOut(char* rate_cycle)
{
    char tmp_cycle[8] = {0};
    strcpy(tmp_cycle,rate_cycle);
    //cout << tmp_cycle <<endl;
    try{			
	if ( dbConnect(conn) )
	 {
	       //先将表格中的该账期数据删除
			Statement stmt = conn.createStatement();
	        char szsql[4000];		        

	        sprintf(szsql, "INSERT INTO D_C1X_ACCOUNT_TMP_%s("
                        "SETTLE_CARRIER_ID,"
                        "SETTLE_MONTH     ,"
                        "DIRECTION        ,"
                        "CURRENCY         ,"
                        "SETTLE_UNIT      ,"
                        "SETTLE_RATE      ,"
                        "EXCHANGE_RATE    ,"
                        "TOTAL_DATA_FLOW  ,"
                        "SETTLE_FEE_RMB   ,"
                        "SETTLE_FEE_SDR   ,"
                        "CREATE_TIME,"
                        "SOURCE_TYPE) "
           "SELECT A.HOME_CARRIER_ID,'%s',1,A.CURRENCY,1048576,1,D.rate,"
           "sum(A.total_flow),0,0,to_char(SYSDATE,'yyyymmddhh24miss'),2 "
           " FROM D_IIXC_INMONTH_RESULT_%s A,EXCHANGE_RATE D "
                "WHERE A.HOME_CARRIER_ID in(  select carrier_id from CARRIER_DIRECTION "
                " where source_id='IIXC' and valid_flag='Y'  ) and "
                " d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' and src_currency='USD' and dest_currency='RMB' "
                " group by A.HOME_CARRIER_ID,A.CURRENCY,d.rate ",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle); 
	        
			stmt.setSQLString(szsql);
			stmt.execute();		
			conn.commit();
			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }	 
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "处理IIXC 数据出错", __FILE__, __LINE__);
    }
	return true;
}

bool CSettle::DoforIOXC(char* rate_cycle)
{
    char tmp_cycle[8] = {0};
    strcpy(tmp_cycle,rate_cycle);
    try{			
	if ( dbConnect(conn) )
	 {
	       //先将表格中的该账期数据删除
			Statement stmt = conn.createStatement();
	        char szsql[4000];
	        sprintf(szsql,"delete from D_C1X_ACCOUNT_TMP_%s where DIRECTION =2 and settle_month = '%s' ",tmp_cycle,tmp_cycle);
	        stmt.setSQLString(szsql);
			stmt.execute();
	        
	        sprintf(szsql, "INSERT INTO D_C1X_ACCOUNT_TMP_%s("
                   "SETTLE_CARRIER_ID,"                  
                   "SETTLE_MONTH,"
                   "DIRECTION,"
                   "CURRENCY ,"
                   "SETTLE_UNIT,"
                   "SETTLE_RATE,"
                   "EXCHANGE_RATE,"
                   "TOTAL_DATA_FLOW,"
                   "SETTLE_FEE_RMB,"
                   "SETTLE_FEE_SDR ,"
                   "CREATE_TIME) "
                   "SELECT C.CUST_ID, %s ,2, 'USD',1048576,1,D.rate,"
                   "sum(A.MDR_CHARGED_VOLUME),sum(A.TOTAL_CHARGES)*1000*D.RATE,"
                   "sum(A.TOTAL_CHARGES)*1000,to_char(SYSDATE,'yyyymmddhh24miss') "
            "FROM D_C1X_DETAIL_ACCOUNT A,BSID B,STT_OBJECT C,EXCHANGE_RATE D "
            "WHERE A.DiRECTION = 2 AND A.SETTLE_MONTH= %s AND "
                   "A.SID =  B.BSID AND "
                   "B.CARRiER_CD = C.CUST_CODE AND "
                   "C.EXP_DATE > SYSDATE "
                   "AND A.CURRENCY = 'USD' AND " 
                   "d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' and D.src_currency='USD' and D.dest_currency='RMB' "
                   "group by c.cust_id,d.rate",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle); 
	        
			stmt.setSQLString(szsql);
			//cout << szsql <<endl;
			stmt.execute();

			sprintf(szsql, "INSERT INTO D_C1X_ACCOUNT_TMP_%s("
                   "SETTLE_CARRIER_ID,"
                   "SETTLE_MONTH,"
                   "DIRECTION ,"
                   "CURRENCY,"
                   "SETTLE_UNIT,"
                   "SETTLE_RATE ,"
                   "EXCHANGE_RATE,"
                   "TOTAL_DATA_FLOW,"
                   "SETTLE_FEE_RMB ,"
                   "SETTLE_FEE_SDR ,"
                   "CREATE_TIME) "
        " SELECT C.CUST_ID, '%s' ,2, 'RMB',1048576,1,D.rate,"
                  " sum(A.MDR_CHARGED_VOLUME),sum(A.TOTAL_CHARGES)*1000,"
                  " sum(A.TOTAL_CHARGES)*1000/D.RATE,to_char(SYSDATE,'yyyymmddhh24miss')"
       "  FROM D_C1X_DETAIL_ACCOUNT A,BSID B,STT_OBJECT C,EXCHANGE_RATE D "
                "WHERE A.DiRECTION = 2 AND A.SETTLE_MONTH='%s' AND "
                       "A.SID =  B.BSID AND "
                       "B.CARRiER_CD = C.CUST_CODE AND "
                       "C.EXP_DATE > SYSDATE "
                       "AND A.CURRENCY = 'RMB' AND "
                       "d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' and D.src_currency='USD' and D.dest_currency='RMB'"
                     "group by c.cust_id,d.rate",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle);
			stmt.setSQLString(szsql);
			//cout << szsql <<endl;
			stmt.execute();
			conn.commit();
			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }	 
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "处理IIXC 数据出错", __FILE__, __LINE__);
    }
	return true;
}


bool CSettle::DoIOXCOut(char* rate_cycle)
{
    char tmp_cycle[8] = {0};
    strcpy(tmp_cycle,rate_cycle);
    //cout << tmp_cycle <<endl;
    try{			
	if ( dbConnect(conn) )
	 {
	       //先将表格中的该账期数据删除
			Statement stmt = conn.createStatement();
	        char szsql[4000];		        

	        sprintf(szsql, "INSERT INTO D_C1X_ACCOUNT_TMP_%s("
                        "SETTLE_CARRIER_ID,"
                        "SETTLE_MONTH     ,"
                        "DIRECTION        ,"
                        "CURRENCY         ,"
                        "SETTLE_UNIT      ,"
                        "SETTLE_RATE      ,"
                        "EXCHANGE_RATE    ,"
                        "TOTAL_DATA_FLOW  ,"
                        "SETTLE_FEE_RMB   ,"
                        "SETTLE_FEE_SDR   ,"
                        "CREATE_TIME,"
                        "SOURCE_TYPE) "
           "SELECT A.VISITCARRIER,'%s',2,A.CURRENCY,1048576,1,D.rate,"
           "sum(A.total_flow),0,0,to_char(SYSDATE,'yyyymmddhh24miss'),2 "
           " FROM D_IOXC_OUTMONTH_RESULT_%s A,EXCHANGE_RATE D "
                "WHERE A.VISITCARRIER in(  select carrier_id from CARRIER_DIRECTION "
                " where source_id='IOXC' and valid_flag='Y'  ) and "
                " d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' and src_currency='USD' and dest_currency='RMB' "
                " group by A.VISITCARRIER,A.CURRENCY,d.rate ",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle); 
	        
			stmt.setSQLString(szsql);
			stmt.execute();		
			conn.commit();
			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }	 
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "处理IIXC 数据出错", __FILE__, __LINE__);
    }
	return true;
}

// 将短信明细表D_SMS_DETAIL_ACCOUNT 中的数据汇总到表D_SMS_ACCOUNT
bool CSettle::DoforIISM(char* rate_cycle)
{
    char tmp_cycle[8] = {0};
    strcpy(tmp_cycle,rate_cycle);
    try{			
	if ( dbConnect(conn) )
	 {
	       //先将表格中的该账期数据删除
			Statement stmt = conn.createStatement();
	        char szsql[4000];	
	        sprintf(szsql,"delete from D_SMS_ACCOUNT_TMP_%s where roam_type =1 and settle_month = '%s' ",tmp_cycle,tmp_cycle);
	        stmt.setSQLString(szsql);
			stmt.execute();
			//cout << szsql <<endl;

	        sprintf(szsql, "INSERT INTO D_SMS_ACCOUNT_TMP_%s("
                    "SETTLE_MONTH         ,"         
                    "ROAM_TYPE            , "        
                    "SETTLE_CARRIER_ID              , "                             
                    "MO_SUC_COUNT         , "        
                    "TOTAL_FEE_SDR        , "      
                    "SETTLE_FEE_SDR       , "        
                    "RMB_SDR_RATE         ,"         
                    "RMB_CURRENCY         ,"         
                    "SDR_CURRENCY         ,"         
                    "CREATE_TIME ) "           
                   "select '%s','1',a.cust_id,sum(a.MO_SUC_COUNT), "        
                      "sum(a.TOTAL_FEE_SDR),sum(a.SETTLE_FEE_SDR),d.rate,"
                      "'RMB','USD',to_char(SYSDATE,'yyyymmddhh24miss') "      
            "from D_SMS_DETAIL_ACCOUNT a,EXCHANGE_RATE d where a.roam_type =1 and "
                       "a.settle_month='%s' and d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' "
              "group by a.cust_id,d.rate",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle); 
	        
			stmt.setSQLString(szsql);
			//cout << szsql <<endl;
			stmt.execute();

			 sprintf(szsql, "INSERT INTO D_SMS_ACCOUNT_TMP_%s("
                    "SETTLE_MONTH         ,"         
                    "ROAM_TYPE            , "        
                    "SETTLE_CARRIER_ID              , "                                   
                    "MO_SUC_COUNT         , "        
                    "TOTAL_FEE_SDR        , "      
                    "SETTLE_FEE_SDR       , "        
                    "RMB_SDR_RATE         ,"         
                    "RMB_CURRENCY         ,"         
                    "SDR_CURRENCY         ,"         
                    "CREATE_TIME ) "           
                    "select '%s','1',a.home_carrier_id,sum(a.call_counts),0,0,d.rate,"
                      "'RMB','USD',to_char(SYSDATE,'yyyymmddhh24miss') "      
            "from D_IISM_INMONTH_RESULT_%s a,EXCHANGE_RATE D where a.roam_type =1 and "
                       "a.rate_cycle='%s' and d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' "
                       "and a.home_carrier_id in(select carrier_id from CARRIER_DIRECTION "
                  " where source_id='IISM' and valid_flag='Y') "
              "group by a.home_carrier_id,d.rate",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle); 
	        
			stmt.setSQLString(szsql);
			stmt.execute();
			//cout << szsql <<endl;
			conn.commit();
			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }	 
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "处理IIXC 数据出错", __FILE__, __LINE__);
    }
	return true;
}

// 将短信明细表D_SMS_DETAIL_ACCOUNT 中的数据汇总到表D_SMS_ACCOUNT
bool CSettle::DoforIOSM(char* rate_cycle)
{
    char tmp_cycle[8] = {0};
    strcpy(tmp_cycle,rate_cycle);
    try{			
	if ( dbConnect(conn) )
	 {
	       //先将表格中的该账期数据删除
			Statement stmt = conn.createStatement();
	        char szsql[4000];	
	        sprintf(szsql,"delete from D_SMS_ACCOUNT_TMP_%s where roam_type in(4,6) and settle_month = '%s' ",tmp_cycle,tmp_cycle);
	        stmt.setSQLString(szsql);
			stmt.execute();

	         sprintf(szsql, "INSERT INTO D_SMS_ACCOUNT_TMP_%s("
                    "SETTLE_MONTH         ,"         
                    "ROAM_TYPE            , "        
                    "SETTLE_CARRIER_ID              , "                             
                    "MO_SUC_COUNT         , "        
                    "TOTAL_FEE_SDR        , "      
                    "SETTLE_FEE_SDR       , "        
                    "RMB_SDR_RATE         ,"         
                    "RMB_CURRENCY         ,"         
                    "SDR_CURRENCY         ,"         
                    "CREATE_TIME)  "           
                   "select '%s','4',a.cust_id,sum(a.MO_SUC_COUNT), "        
                      "sum(a.TOTAL_FEE_SDR),sum(a.SETTLE_FEE_SDR),d.rate,"
                      "'RMB','USD',to_char(SYSDATE,'yyyymmddhh24miss') "      
            "from D_SMS_DETAIL_ACCOUNT a,EXCHANGE_RATE D where a.roam_type =4 and "
                       "a.settle_month='%s' and d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10' "
              "group by a.cust_id,d.rate",tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle); 
	        
			stmt.setSQLString(szsql);
			stmt.execute();

			 sprintf(szsql, "INSERT INTO D_SMS_ACCOUNT_TMP_%s("
                    "SETTLE_MONTH         ,"         
                    "ROAM_TYPE            , "        
                    "SETTLE_CARRIER_ID              , "                                   
                    "MO_SUC_COUNT         , "        
                    "TOTAL_FEE_SDR        , "      
                    "SETTLE_FEE_SDR       , "        
                    "RMB_SDR_RATE         ,"         
                    "RMB_CURRENCY         ,"         
                    "SDR_CURRENCY         ,"         
                    "CREATE_TIME ) "           
                    "select '%s',a.roam_type,a.VISITCARRIER,sum(a.call_counts),0,0,d.rate,"
                      "'RMB','USD',to_char(SYSDATE,'yyyymmddhh24miss') "      
            "from D_IOSM_OUTMONTH_RESULT_%s a,EXCHANGE_RATE D where "
                       //" a.VISITCARRIER='7455' and "
                       " a.VISITCARRIER in(select carrier_id from CARRIER_DIRECTION "
                  " where source_id='IOSM' and valid_flag='Y') and "
                       "a.rate_cycle='%s' and d.rate_date = to_char(add_months(to_date('%s','yyyymm'),-1),'yyyymm')||'10'"
                      "group by a.roam_type, a.VISITCARRIER,d.rate", tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle,tmp_cycle);
			 stmt.setSQLString(szsql);
			 stmt.execute();
			 conn.commit();
			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }	 
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "处理IIXC 数据出错", __FILE__, __LINE__);
    }
	return true;
}

bool CSettle::checkArg(int argc, char** argv)
{	
	if (!(argc == 3 ))
	{
	    printf("Usage : %s <source_id> <rate_cycle>\n",argv[0]);
	    printf("P.S : jssettle IIXC 201310 \n");
		return false;
	}	   
   
	return true;
}

int main(int argc,char**argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    jssettle               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2013-12-03 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;
	
    CSettle set;
    char sourceid[6];
    char ratecycle[6];
    strcpy(sourceid,argv[1]);
    strcpy(ratecycle,argv[2]);
   	if( !set.checkArg( argc, argv ) )
	{
		 return -1;
	}
   	
   	std::string log_path,log_level;
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "获取日志路径失败" );
	}
	
	theJSLog.setLog(log_path.c_str(), 1, sourceid, "SETTLE", 1);	
	
     if(!(rtinfo.connect()))		//连接内存区
	{
		 return false;
	}
	rtinfo.getDBSysMode(petri_status);		//获取状态
	theJSLog<<"petri_status:"<< petri_status <<endd;

	if(petri_status==304)
	{
		return false;
	} 
    bool result;
    //获取信息
    //cout << "sourced_id = " << sourceid <<endl;
    //cout << "rate_cycle = " << ratecycle <<endl;
    if( strcmp(sourceid,"IIXC") == 0)
    {
       if( set.DoforIIXC(ratecycle) )
       result = set.DoIIXCOut(ratecycle);
    } 
    else if ( strcmp(sourceid,"IOXC") == 0 )
    {
       if( set.DoforIOXC(ratecycle) )
       	result = set.DoIOXCOut(ratecycle);
    } 
    else if ( strcmp(sourceid,"IISM") == 0)
    {
       	result = set.DoforIISM(ratecycle);
    }
    else if ( strcmp(sourceid,"IOSM") == 0)
    {
       	result = set.DoforIOSM(ratecycle);
    }

    if(result == true)
    	theJSLog << sourceid << "  Settle success !" <<endi;
    else
    	theJSLog << sourceid << "   Settle Fail !" <<endi;
   return 0;
}

