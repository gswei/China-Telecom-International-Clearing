/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-11-13
File:			 Cimpdata.cpp
Description:	 导数据模块
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "Cimpdata.h"
CLog theJSLog;
SGW_RTInfo rtinfo;
short petri_status = -1; 
using namespace tpss;

Cimpdata::Cimpdata()
{
}

Cimpdata::~Cimpdata()
{
}

bool Cimpdata::setData()
{	
   Statement stmt = conn.createStatement();
   try{				     
	     char szsql[4000];
	     // 将历史记录标志置为N 
	     sprintf(szsql,"update C_GJGW_BILLING_GROUPLEVEL set valid_flag = 'N'");
	 	 stmt.setSQLString(szsql);
	 	 //theJSLog << szsql <<endd;
		 stmt.execute();

		 sprintf(szsql,"update C_GJGW_BILLING_RATEGROUP set valid_flag = 'N'");
	 	 stmt.setSQLString(szsql);
	 	 //theJSLog << szsql <<endd;
		 stmt.execute();

		 sprintf(szsql,"update C_GJGW_BILLING_RULE set valid_flag = 'N'");
	 	 stmt.setSQLString(szsql);
	 	 //theJSLog << szsql <<endd;
		 stmt.execute();	 
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "删除固网结算公式数据出错", __FILE__, __LINE__);
    }
	return true;
}

bool Cimpdata::dealGWYY()
{	
   //序列号
   char formula_seq[11];
   char group_seq[11];
   char rule_seq[11];
   //批价变量和值
   char column_name[15];
   char all_column_name[1024];
   char column_value[15];
   char all_column_value[1024];

   int column_no=0;
   int value_num =0;
   int record_num =0;
   char fee[14];
   char starttime[14];
   char endtime[14];
   char index_id[14];
   double num_fee;

   try{			
	if ( dbConnect(conn) )
	 {
	     Statement stmt = conn.createStatement();
	     Statement m_stmt = conn.createStatement();
	     char szsql[4000];	    
		 //setData();   //暂时不将原始数据标志改为N
		 
		 //获取SQL 语句中的变量值
		 sprintf(szsql,"select count(*) from C_RATING_FIELD_DEF where rating_config_id = 'GWYY' ");
	 	 stmt.setSQLString(szsql);
		 stmt.execute();
		 stmt>>column_no;
		 //theJSLog << "column_no = "<<column_no <<endd;
		 
		 sprintf(szsql,"select field_name from C_RATING_FIELD_DEF where rating_config_id = 'GWYY' order by field_order_no");
	 	 stmt.setSQLString(szsql);
		 stmt.execute();
		 memset(column_name,0,sizeof(column_name));
		 memset(all_column_name,0,sizeof(all_column_name));
		 while(stmt>>column_name)
			{
			   strcat(all_column_name,column_name);
	           strcat(all_column_name,",");
			}
		 all_column_name[strlen(all_column_name)-1] = 0;  
		 theJSLog << "all_column_name :  "<<all_column_name <<endd;
		 
         //获取需要导入的数量
		sprintf(szsql,"select count(*) from C_GWYY_RATE where valid_flag = 'Y' and deal_flag ='W' ");		
        //theJSLog <<"szsql="<<szsql<<endd;
		stmt.setSQLString(szsql);
		stmt.execute();
		stmt >> value_num;	
		theJSLog << "value_num = "<<value_num <<endd;

		 //循环读取导入表格的数据
		sprintf(szsql,"select destination_id,fee,eff_time,exp_time,%s from C_GWYY_RATE where valid_flag = 'Y' and deal_flag='W'",all_column_name);		
		theJSLog <<"szsql="<<szsql<<endd;
		m_stmt.setSQLString(szsql);
		m_stmt.execute();
		int i=0;
		memset(column_value,0,sizeof(column_value));
		memset(all_column_value,0,sizeof(all_column_value));
		while(m_stmt >> column_value)
			{		
			   i++;
			   if(i==1)
			   	{
			   	   strcpy(index_id,column_value);
			   	   theJSLog << "index_id = " << index_id <<endd;
			   	}
			   else if(i==2)
			   	{
			   	   //strcpy(fee,column_value);
			   	   num_fee = atof(column_value);  //先转换为double类型
			   	   sprintf(fee,"%.4f",num_fee);
			   	   //cout << "column_value = "<<column_value << endl;
			   	   //theJSLog << "num_fee = " << num_fee <<endd;
			   	   theJSLog << "fee = " << fee <<endd;
			   	}
			   else if(i==3)
			   	{
			   	   strcpy(starttime,column_value);
			   	   if( strlen(starttime)==0 )
			   	   	 strcpy(starttime,"20000101");
			   	   //theJSLog << "starttime = " << starttime <<endd;
			   	} 
			   else if(i==4)
			   	{
			   	   strcpy(endtime,column_value);
			   	   if(strlen(endtime)==0)
			   	   	strcpy(endtime,"29991231");
			   	    //theJSLog << "endtime = " << endtime <<endd;
			   	}		   
			   else {  //非特殊字段
			     strcat(all_column_value,column_value);
	             strcat(all_column_value,",");

	           if(i==column_no+4)  //完成的一条记录
	           	{
	           	    // 将序列号从数据库中获取
	           	    i=0; //重新置为0
                   sprintf(szsql,"select GWYY_FORMULA_ID.NEXTVAL from dual");
	 	           stmt.setSQLString(szsql);
	 	           //theJSLog << szsql <<endd;
		           stmt.execute();		          
		           stmt >> formula_seq;
		           theJSLog<< "formula_seq = " <<formula_seq<<endd;

		           sprintf(szsql,"select GWYY_RATEGROUP_ID.NEXTVAL from dual");
	 	           stmt.setSQLString(szsql);
	 	           //theJSLog << szsql <<endd;
		           stmt.execute();
		           stmt >> group_seq;
		           theJSLog<< "group_seq = " <<group_seq<<endd;

		           sprintf(szsql,"select GWYY_RULE_ID.nextval from dual");
	 	           stmt.setSQLString(szsql);
		           stmt.execute();
		           stmt >> rule_seq;	
		           theJSLog<< "rule_seq = " <<rule_seq<<endd;
		 
	           	   all_column_value[strlen(all_column_value)-1]=0;	           	  
                   //theJSLog << "待插入数据: " << all_column_value <<endd;
                   
                   getYYSql(group_seq,formula_seq,rule_seq,starttime,endtime,fee,all_column_name,all_column_value);	              
	               record_num++;
	               memset(all_column_value,0,sizeof(all_column_value));
	              // end 一条完整记录
	              sprintf(szsql,"update C_GWYY_RATE set rategroup_id ='%s',deal_flag='Y',stat_time = to_char(sysdate,'yyyymmdd') where destination_id ='%s'",group_seq,index_id);
	 	          stmt.setSQLString(szsql);
		          stmt.execute();
	           	}
			  }
			}

			//数量正确 全部处理完成
	    if(record_num == value_num)
	    {		  
	      conn.commit();
	      conn.close();
	      theJSLog << "Success ,dealGWYY Rate over ,record_num="<<record_num<<endd;
	   }
	  else 
	  {
	     conn.close();
	     stmt.rollback();
	     m_stmt.rollback();
	     return false;
	  }

			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 	return false;
	 }		 
	 
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "删除固网结算公式数据出错", __FILE__, __LINE__);
    }
	return true;
}

bool Cimpdata::getYYSql(char *rategroup_id,char *formula_id,char *rule_id,char *start_time,char *end_time,char* fee,char *column,char* value)
{	   
   char sql[5000];
   char group_desc[1024];
   char column_name[1024];
   char column_value[1024];
   strcpy(column_name,column);
   strcpy(column_value,value);
   theJSLog <<"插入变量: "<< column_name<<endd;
   theJSLog <<"插入值: "<< column_value<<endd;
   Statement m_stmt = conn.createStatement();
    try{				     
         sprintf(group_desc,"国际固网结算费率_%s",fee);
         sprintf(sql,"insert into C_GWYY_BILLING_GROUPLEVEL(rategroup_id,REPNO_CDRCOUNT,group_desc,valid_flag) "
   	                 " values('%s',0,'%s','Y')",rategroup_id,group_desc);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);         
		 m_stmt.execute();
   
         sprintf(sql,"insert into C_GWYY_BILLING_RATEGROUP(rategroup_id,START_TIME,END_TIME,TARIFF_ID,RULEREPNOA,RULEREPNOB,"
   	             "REPNO_CDRDURATION,REPNO_RATEDURATION,CHARGE_TYPE,PRI_NO,RATEGROUP_DESC,FORMULA_ID,valid_flag) "
        	     " values('%s','%s','%s','4001',-1,-1,-1,-1,4000,1,'%s','%s','Y')",rategroup_id,start_time,end_time,group_desc,formula_id);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);
		 m_stmt.execute();
   
         sprintf(sql,"insert into C_GWYY_BILLING_RULE(ruleno,rategroup_id,rule_desc,%s,start_time,end_time,valid_flag)"
   	              " values('%s','%s','%s',%s,'%s','%s','Y')",column_name,rule_id,rategroup_id,group_desc,column_value,start_time,end_time);  
         theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();

		 sprintf(sql,"insert into C_GWYY_FORMULA_DEF(formula_id,index_num,segment_name,formula,formula_desc,start_time,end_time,output_type,decimal_num)"
   	              " select '%s',index_num,segment_name,formula,formula_desc,start_time,end_time,output_type,decimal_num from C_GWYY_FORMULA_DEF "
   	              " where formula_id ='0' ",formula_id);   
		 theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();

		 sprintf(sql,"insert into C_GWYY_FORMULA_PARAM_DEF(formula_id,param_name,param_type,param_value)"
   	              " select '%s',param_name,param_type,param_value from C_GWYY_FORMULA_PARAM_DEF "
   	              " where formula_id ='0' ",formula_id);  
		 theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();

		 sprintf(sql,"update C_GWYY_FORMULA_PARAM_DEF set param_value = '%s' "
   	              " where formula_id ='%s' and param_name ='SETTLE_RATE' ",fee,formula_id);   
		 theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();
		 //m_stmt.commit();	 
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		m_stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "插入固网结算公式数据出错", __FILE__, __LINE__);
    }
   
	return true;
}

//固网短信
bool Cimpdata::dealGWDX()
{	
   //序列号
   char formula_seq[11];
   char group_seq[11];
   char rule_seq[11];
   //批价变量和值
   char column_name[15];
   char all_column_name[1024];
   char column_value[15];
   char all_column_value[1024];

   int column_no=0;
   int value_num =0;
   int record_num =0;
   char fee[14];
   char starttime[14];
   char endtime[14];
   char index_id[14];
   double num_fee;

   try{			
	if ( dbConnect(conn) )
	 {
	     Statement stmt = conn.createStatement();
	     Statement m_stmt = conn.createStatement();
	     char szsql[4000];	    
		 //setData();   //暂时不将原始数据标志改为N
		 
		 //获取SQL 语句中的变量值
		 sprintf(szsql,"select count(*) from C_RATING_FIELD_DEF where rating_config_id = 'GWDX' ");
	 	 stmt.setSQLString(szsql);
		 stmt.execute();
		 stmt>>column_no;
		 //theJSLog << "column_no = "<<column_no <<endd;
		 
		 sprintf(szsql,"select field_name from C_RATING_FIELD_DEF where rating_config_id = 'GWDX' order by field_order_no");
	 	 stmt.setSQLString(szsql);
		 stmt.execute();
		 memset(column_name,0,sizeof(column_name));
		 memset(all_column_name,0,sizeof(all_column_name));
		 while(stmt>>column_name)
			{
			   strcat(all_column_name,column_name);
	           strcat(all_column_name,",");
			}
		 all_column_name[strlen(all_column_name)-1] = 0;  
		 theJSLog << "all_column_name :  "<<all_column_name <<endd;
		 
         //获取需要导入的数量
		sprintf(szsql,"select count(*) from C_GWDX_RATE where valid_flag = 'Y' and deal_flag='W'");		
        //theJSLog <<"szsql="<<szsql<<endd;
		stmt.setSQLString(szsql);
		stmt.execute();
		stmt >> value_num;	
		theJSLog << "value_num = "<<value_num <<endd;

		 //循环读取导入表格的数据
		sprintf(szsql,"select index_id,fee,eff_time,exp_time,%s from C_GWDX_RATE where valid_flag = 'Y' and deal_flag='W'",all_column_name);		
		theJSLog <<"szsql="<<szsql<<endd;
		m_stmt.setSQLString(szsql);
		m_stmt.execute();
		int i=0;
		memset(all_column_value,0,sizeof(all_column_value));
		while(m_stmt >> column_value)
			{		
			   i++;
			   if(i==1)
			   	{
			   	   strcpy(index_id,column_value);
			   	   theJSLog << "index_id = " << index_id <<endd;
			   	}
			   else if(i==2)
			   	{
			   	   num_fee = atof(column_value);  //先转换为double类型
			   	   sprintf(fee,"%.4f",num_fee);
			   	   //strcpy(fee,column_value);
			   	   theJSLog << "fee = " << fee <<endd;
			   	}
			   else if(i==3)
			   	{
			   	   strcpy(starttime,column_value);
			   	   if( strlen(starttime)==0 )
			   	   	 strcpy(starttime,"20000101");
			   	   //theJSLog << "starttime = " << starttime <<endd;
			   	} 
			   else if(i==4)
			   	{
			   	   strcpy(endtime,column_value);
			   	   if(strlen(endtime)==0)
			   	   	strcpy(endtime,"20351231");
			   	    //theJSLog << "endtime = " << endtime <<endd;
			   	}		   
			   else {  //非特殊字段
			     strcat(all_column_value,column_value);
	             strcat(all_column_value,",");

	           if(i==column_no+4)  //完成的一条记录
	           	{
	           	    // 将序列号从数据库中获取
	           	    i=0; //重新置为0
                   sprintf(szsql,"select GWDX_FORMULA_ID.NEXTVAL from dual");
	 	           stmt.setSQLString(szsql);
	 	           //theJSLog << szsql <<endd;
		           stmt.execute();		          
		           stmt >> formula_seq;
		           theJSLog<< "formula_seq = " <<formula_seq<<endd;

		           sprintf(szsql,"select GWDX_RATEGROUP_ID.NEXTVAL from dual");
	 	           stmt.setSQLString(szsql);
	 	           //theJSLog << szsql <<endd;
		           stmt.execute();
		           stmt >> group_seq;
		           theJSLog<< "group_seq = " <<group_seq<<endd;

		           sprintf(szsql,"select GWDX_RULE_ID.nextval from dual");
	 	           stmt.setSQLString(szsql);
		           stmt.execute();
		           stmt >> rule_seq;	
		           theJSLog<< "rule_seq = " <<rule_seq<<endd;
		 
	           	   all_column_value[strlen(all_column_value)-1]=0;	           	  
                   theJSLog << "待插入数据: " << all_column_value <<endd;
	               //getSql(char *rategroup_id,char *formula_id,char *rule_id,char *start_time,char *end_time,char *fee,char *column,char* value)
                   getDXSql(group_seq,formula_seq,rule_seq,starttime,endtime,fee,all_column_name,all_column_value);	              
	               record_num++;
	               memset(all_column_value,0,sizeof(all_column_value));
	              // end 一条完整记录
	              sprintf(szsql,"update C_GWDX_RATE set rategroup_id ='%s',deal_flag='Y',stat_time = to_char(sysdate,'yyyymmdd') where index_id ='%s'",group_seq,index_id);
	 	          stmt.setSQLString(szsql);
		          stmt.execute();
	           	}
			  }
			}

			//数量正确 全部处理完成
	    if(record_num == value_num)
	    {		  
	      conn.commit();
	      conn.close();
	      cout << "Success ,dealGWDX Rate over ,record_num="<<record_num<<endl;
	   }
	  else 
	  {
	     conn.close();
	     stmt.rollback();
	     m_stmt.rollback();
	     return false;
	  }

			
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 	return false;
	 }		 
	 
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "删除固网结算公式数据出错", __FILE__, __LINE__);
    }
	return true;
}

bool Cimpdata::getDXSql(char *rategroup_id,char *formula_id,char *rule_id,char *start_time,char *end_time,char *fee,char *column,char* value)
{	   
   char sql[5000];
   char group_desc[1024];
   char column_name[1024];
   char column_value[1024];
   strcpy(column_name,column);
   strcpy(column_value,value);
   //theJSLog <<"插入变量: "<< column_name<<endd;
   //theJSLog <<"插入值: "<< column_value<<endd;
   Statement m_stmt = conn.createStatement();
    try{				     
         sprintf(group_desc,"国际固网短信结算费率_%s",fee);
         sprintf(sql,"insert into C_GWDX_BILLING_GROUPLEVEL(rategroup_id,REPNO_CDRCOUNT,group_desc,valid_flag) "
   	                 " values('%s',0,'%s','Y')",rategroup_id,group_desc);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);         
		 m_stmt.execute();
   
         sprintf(sql,"insert into C_GWDX_BILLING_RATEGROUP(rategroup_id,START_TIME,END_TIME,TARIFF_ID,RULEREPNOA,RULEREPNOB,"
   	             "REPNO_CDRDURATION,REPNO_RATEDURATION,CHARGE_TYPE,PRI_NO,RATEGROUP_DESC,FORMULA_ID,valid_flag) "
        	     " values('%s','%s','%s','4001',-1,-1,-1,-1,4000,1,'%s','%s','Y')",rategroup_id,start_time,end_time,group_desc,formula_id);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);
		 m_stmt.execute();
   
         sprintf(sql,"insert into C_GWDX_BILLING_RULE(ruleno,rategroup_id,rule_desc,%s,start_time,end_time,valid_flag)"
   	              " values('%s','%s','%s',%s,'%s','%s','Y')",column_name,rule_id,rategroup_id,group_desc,column_value,start_time,end_time);  
         theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();

		 sprintf(sql,"insert into C_GWDX_FORMULA_DEF(formula_id,index_num,segment_name,formula,formula_desc,start_time,end_time,output_type,decimal_num)"
   	              " select '%s',index_num,segment_name,formula,formula_desc,start_time,end_time,output_type,decimal_num from C_GWDX_FORMULA_DEF "
   	              " where formula_id ='0' ",formula_id);   
		 theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();

		 sprintf(sql,"insert into C_GWDX_FORMULA_PARAM_DEF(formula_id,param_name,param_type,param_value)"
   	              " select '%s',param_name,param_type,param_value from C_GWDX_FORMULA_PARAM_DEF "
   	              " where formula_id ='0' ",formula_id);  
		 theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();

		 sprintf(sql,"update C_GWDX_FORMULA_PARAM_DEF set param_value = '%s' "
   	              " where formula_id ='%s' and param_name ='SETTLE_RATE' ",fee,formula_id);   
		 theJSLog << sql <<endd;
	 	 m_stmt.setSQLString(sql);
		 m_stmt.execute();
		 //m_stmt.commit();	 
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		m_stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "插入固网结算公式数据出错", __FILE__, __LINE__);
    }
   
	return true;
}

int main(int argc,char**argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    jsimpdata               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2013-12-03 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;	

    Cimpdata impdata;
   	std::string log_path,log_level;   

   	if (!(argc == 2 ))
	{
	    printf("Usage : %s <source_id> \n",argv[0]);
	    printf("P.S : jsimpdata GWYY / GWDX \n");
		return false;
	}
   		
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "获取日志路径失败" );
	}
	
	strcpy(impdata.source_id,argv[1]);
   	theJSLog << "impdata.source_id = " << impdata.source_id <<endd;
	theJSLog.setLog(log_path.c_str(), 1, "GJGW", "EXPDATA", 1);	
	
    bool result;
    if ( strcmp(impdata.source_id,"GWYY") == 0 )
         result = impdata.dealGWYY();
    else if(strcmp(impdata.source_id,"GWDX") ==0 )
    	 result = impdata.dealGWDX();
    if(result == true)
    	theJSLog << "  Expdata " << impdata.source_id << " success !" <<endi;
    else
    	theJSLog << "   Expdata " << impdata.source_id << " Fail !" <<endi;
   return 0;
}

