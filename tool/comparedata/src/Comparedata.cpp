/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-12-10
File:			 Comparedata.cpp
Description:	 比较数据模块
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "Comparedata.h"
CLog theJSLog;
SGW_RTInfo rtinfo;
short petri_status = -1; 
using namespace tpss;

Comparedata::Comparedata()
{
}

Comparedata::~Comparedata()
{
}

bool Comparedata::DoforGWYY()
{	
    char new_destination_id[9];
    char tmp_destination_id[9];
	int value_num=0;
	GWYY tmp_des;
	char curdate[9];
    getCurDate(curdate);
    
   try{	
   	if ( dbConnect(conn) )
   		{
   		 Statement stmt = conn.createStatement();
   		 Statement m_stmt = conn.createStatement();
	     char szsql[4000];

         //先将最原始的数据备份一次
	     sprintf(szsql,"create table GLOBAL_DEST_ORG_%s as select * from GLOBAL_DEST_ORG ",curdate);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute();
		 
	     memset(tmp_des.destination_id,0,sizeof(tmp_des.destination_id));
	     memset(tmp_des.country_id,0,sizeof(tmp_des.country_id));
	     memset(tmp_des.area_codes,0,sizeof(tmp_des.area_codes));
	     memset(tmp_des.rate,0,sizeof(tmp_des.rate));
	     memset(tmp_des.change,0,sizeof(tmp_des.change));
	     memset(tmp_des.effect_date,0,sizeof(tmp_des.effect_date));
	     memset(tmp_des.expire_date,0,sizeof(tmp_des.expire_date));
	     memset(tmp_des.dir,0,sizeof(tmp_des.dir));
	     memset(tmp_des.comments,0,sizeof(tmp_des.comments));
	     memset(tmp_des.callee_type,0,sizeof(tmp_des.callee_type));
	       
	     // 从org 前台导入表中获取所有数据		 
	     sprintf(szsql,"select count(*) from GLOBAL_DEST_ORG");
	 	 stmt.setSQLString(szsql);
	 	 theJSLog << szsql <<endd;
		 stmt.execute();
		 stmt>>value_num;
		 if(value_num ==0)
		 {
		    theJSLog << "原始表格中无数据" <<endl;
		    return false;
		 }		 	
		 
	     sprintf(szsql,"select destination_id,country_id,country_name,destination,country_code,area_codes,rate,change,effect_date,expire_date,dir,comments,callee_type from GLOBAL_DEST_ORG order by destination_id");
	 	 stmt.setSQLString(szsql);
	 	 theJSLog << szsql <<endd;
		 stmt.execute();
		   
		 while(stmt>>tmp_des.destination_id>>tmp_des.country_id>>tmp_des.country_name>>tmp_des.destination>>tmp_des.country_code>>tmp_des.area_codes
		 	>>tmp_des.rate>>tmp_des.change>>tmp_des.effect_date>>tmp_des.expire_date>>tmp_des.dir>>tmp_des.comments>>tmp_des.callee_type)		
		 {	    
		    char tmp_areacode[4000];  //area_codes数据可能为空，需要单独考虑
		    if(strlen(tmp_des.area_codes)==0)
		    	strcpy(tmp_areacode," is null");
		    else
		    	sprintf(tmp_areacode," = '%s'",tmp_des.area_codes);
		    
		    //判断原始过来的数据是否有重复数据
		    /*sprintf(szsql,"select destination_id from GLOBAL_DEST_ORG where "
                       "COUNTRY_ID = '%s' and "
                       "COUNTRY_NAME = '%s' and   "
                       "DESTINATION = '%s'  and "
                       "COUNTRY_CODE = '%s' and "                       
                       "AREA_CODES %s and "                       
                       "RATE  = '%s' and "
                       "CHANGE ='%s' and "
                       "EFFECT_DATE ='%s' and "
                       "EXPIRE_DATE ='%s' and "
                       "DIR ='%s' and "
                       "COMMENTS = '%s' and "
                       "callee_type = '%s' and "
                       "destination_id <> '%s'",tmp_des.country_id,tmp_des.country_name,tmp_des.destination,tmp_des.country_code,
                       tmp_areacode,tmp_des.rate,tmp_des.change,tmp_des.effect_date,tmp_des.expire_date,tmp_des.dir
                       ,tmp_des.comments,tmp_des.callee_type,tmp_des.destination_id);
		    m_stmt.setSQLString(szsql);
	 	    theJSLog << szsql <<endd;
		    m_stmt.execute();
		    m_stmt >> tmp_destination_id; 
		    //查出有这条记录
		    if( strlen(tmp_destination_id) != 0 )
		    {
		       sprintf(szsql,"delete from GLOBAL_DEST_ORG where destination_id ='%s'",tmp_destination_id);
		       m_stmt.setSQLString(szsql);
	 	       theJSLog << szsql <<endd;
		       m_stmt.execute();
		       insertLog("N",value_num,tmp_destination_id);
		    }*/
/*		    memset(new_destination_id,0,sizeof(new_destination_id));
		    sprintf(szsql,"select destination_id from GLOBAL_DEST where "
                       "COUNTRY_ID = '%s' and "
                       //"COUNTRY_NAME = '%s' and   "
                       //"DESTINATION = '%s'  and "
                       "COUNTRY_CODE = '%s' and "                       
                       "AREA_CODES %s and "                       
                       "RATE  = '%s' and "
                       "CHANGE ='%s' and "
                       "EFFECT_DATE ='%s' and "
                       "EXPIRE_DATE ='%s' and "
                       "DIR ='%s' and "
                       //"COMMENTS = '%s' and "
                       "callee_type = '%s' ",tmp_des.country_id,
                       //tmp_des.country_name,tmp_des.destination,
                       tmp_des.country_code,
                       tmp_areacode,tmp_des.rate,tmp_des.change,tmp_des.effect_date,tmp_des.expire_date,tmp_des.dir,tmp_des.callee_type);
		    m_stmt.setSQLString(szsql);
	 	    theJSLog << szsql <<endd;
		    m_stmt.execute();
		    m_stmt >> new_destination_id;
		    theJSLog << "new_destination_id = " <<new_destination_id<<endd; 
		    
		    if( strlen(new_destination_id) ==0 )
		    {
*/		       //无相同数据，将数据插入到新表中
		       insertDataYY(tmp_des.destination_id);
		       memset(tmp_des.destination_id,0,sizeof(tmp_des.destination_id));
	           memset(tmp_des.country_id,0,sizeof(tmp_des.country_id));
	           memset(tmp_des.area_codes,0,sizeof(tmp_des.area_codes));
	           memset(tmp_des.rate,0,sizeof(tmp_des.rate));
	           memset(tmp_des.change,0,sizeof(tmp_des.change));
	           memset(tmp_des.effect_date,0,sizeof(tmp_des.effect_date));
	           memset(tmp_des.expire_date,0,sizeof(tmp_des.expire_date));
	           memset(tmp_des.dir,0,sizeof(tmp_des.dir));
	           memset(tmp_des.comments,0,sizeof(tmp_des.comments));
/*		    }
		    else
		    {
		       //有相同数据，此条数据不插入到目标表中
		       theJSLog << "表中已有相同数据，不能继续插入" <<endd;
		       insertLog("N",value_num,new_destination_id);
		       continue;
		       //return false;
		    }
*/
		 }
		 // 清空数据表GLOBAL_DEST_ORG
		 if(BakGWYY())
		    insertLog("Y",value_num,"");
   		}else{
	 	    theJSLog<<"connect error."<<endi;
	 	    return false;
	   }
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		//stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "删除固网结算公式数据出错", __FILE__, __LINE__);
    }
	return true;
}

bool Comparedata::insertDataYY(char *destination_id)
{	   
   char sql[5000];
   Statement m_stmt = conn.createStatement();
    try{				     
         sprintf(sql,"insert into GLOBAL_DEST select * from GLOBAL_DEST_ORG "
   	                 " where destination_id ='%s'",destination_id);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);         
		 m_stmt.execute();    

		  sprintf(sql,"insert into C_GWYY_RATE(DESTINATION_ID ,SETTLE_DIR ,FEE ,EFF_TIME , EXP_TIME ,STAT_TIME,VALID_FLAG,DEAL_FLAG) "
		          "select destination_id,dir,rate,effect_date,expire_date,to_char(sysdate,'yyyymmddhh24miss'),'Y','W' from GLOBAL_DEST_ORG where destination_id='%s'",destination_id);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);         
		 m_stmt.execute();
		 m_stmt.commit();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		m_stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "插入固网结算公式数据出错", __FILE__, __LINE__);
    }
   
	return true;
}

bool Comparedata::BakGWYY()
{	   
   char sql[5000];
   Statement stmt = conn.createStatement();
   char curdate[9];
   getCurDate(curdate);
    try{				                 		 		 
		 //将destination 表的内容拷贝到C_ISSF_RATE 表中生成结算规则
		 sprintf(sql,"truncate table GLOBAL_DEST_ORG");
         theJSLog << sql <<endd;
         stmt.setSQLString(sql);         
		 stmt.execute(); 
		 
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "插入固网结算公式数据出错", __FILE__, __LINE__);
    }   
	return true;
}

//  固网短信处理函数
bool Comparedata::DoforGWDX()
{	
    char new_index_id[9];
    char tmp_index_id[9];
	int value_num=0;
	GWDX tmp_des;
	char curdate[9];
    getCurDate(curdate);
    
   try{	
   	if ( dbConnect(conn) )
   		{
   		 Statement stmt = conn.createStatement();
   		 Statement m_stmt = conn.createStatement();
   		 Statement m_stmt2 = conn.createStatement();
	     char szsql[4000];

         //先将原始表格备份，后期会删除重复数据导致记录不全
	     sprintf(szsql,"create table GWDX_CTRYRATE_ORG_%s as select * from GWDX_CTRYRATE_ORG ",curdate);
         theJSLog << szsql <<endd;
         stmt.setSQLString(szsql);         
		 stmt.execute(); 
		 
	     memset(tmp_des.index_id,0,sizeof(tmp_des.index_id));
	     memset(tmp_des.country_id,0,sizeof(tmp_des.country_id));
	     memset(tmp_des.country_name,0,sizeof(tmp_des.country_name));
	     memset(tmp_des.chinese_name,0,sizeof(tmp_des.chinese_name));
	     memset(tmp_des.rate,0,sizeof(tmp_des.rate));
	     memset(tmp_des.effect_date,0,sizeof(tmp_des.effect_date));
	     memset(tmp_des.dir,0,sizeof(tmp_des.dir));
	     memset(tmp_des.country_code,0,sizeof(tmp_des.country_code));
	       
	     // 从org 前台导入表中获取所有数据		 
	     sprintf(szsql,"select count(*) from GWDX_CTRYRATE_ORG");
	 	 stmt.setSQLString(szsql);
	 	 theJSLog << szsql <<endd;
		 stmt.execute();
		 stmt>>value_num;
		 if(value_num ==0)
		 {
		    theJSLog << "原始表格中无数据" <<endl;
		    return false;
		 }		 	
		 
	     sprintf(szsql,"select index_id,country_id,country_name,chinese_name,rate,effect_date,expire_date,dir,country_code from GWDX_CTRYRATE_ORG order by index_id");
	 	 stmt.setSQLString(szsql);
	 	 theJSLog << szsql <<endd;
		 stmt.execute();
		   
		 while(stmt>>tmp_des.index_id>>tmp_des.country_id>>tmp_des.country_name>>tmp_des.chinese_name>>tmp_des.rate
		 	>>tmp_des.effect_date>>tmp_des.expire_date>>tmp_des.dir>>tmp_des.country_code)		
		 {		
		    /*sprintf(szsql,"select index_id from GWDX_CTRYRATE_ORG where "
                       "COUNTRY_ID = '%s' and "
                      // "COUNTRY_NAME = '%s' and   "
                       "chinese_name = '%s'  and "
                       "rate = '%s' and "                       
                       "EFFECT_DATE ='%s' and "
                       "EXPIRE_DATE ='%s' and "
                       "DIR ='%s' and "
                       "country_code ='%s' and "
                       "index_id <> '%s'",tmp_des.country_id,
                       //tmp_des.country_name,
                       tmp_des.chinese_name,tmp_des.rate,
                       tmp_des.effect_date,tmp_des.expire_date,tmp_des.dir,tmp_des.country_code,tmp_des.index_id);
		    m_stmt.setSQLString(szsql);
	 	    theJSLog << szsql <<endd;
		    m_stmt.execute();
		    m_stmt >> tmp_index_id;

		    //查出有重复记录
		    if( strlen(tmp_index_id) != 0) 
		    {
		       sprintf(szsql,"delete from GWDX_CTRYRATE_ORG where index_id ='%s'",tmp_index_id);
		       m_stmt2.setSQLString(szsql);
	 	       theJSLog << szsql <<endd;
		       m_stmt2.execute();
		       m_stmt2.commit();
		       insertLog("N",value_num,tmp_index_id);
		    }*/
		    
		    sprintf(szsql,"select index_id from GWDX_CTRYRATE where "
                       "COUNTRY_ID = '%s' and "
                      // "COUNTRY_NAME = '%s' and   "
                       "chinese_name = '%s'  and "
                       "rate = '%s' and "                       
                       "EFFECT_DATE ='%s' and "
                       "EXPIRE_DATE ='%s' and "
                       "DIR ='%s' and "
                       "country_code ='%s' ",tmp_des.country_id,
                       //tmp_des.country_name,
                       tmp_des.chinese_name,tmp_des.rate,
                       tmp_des.effect_date,tmp_des.expire_date,tmp_des.dir,tmp_des.country_code);
		    m_stmt.setSQLString(szsql);
	 	    theJSLog << szsql <<endd;
		    m_stmt.execute();
		    memset(new_index_id,0,sizeof(new_index_id));
		    m_stmt >> new_index_id;
		    theJSLog << "new_index_id = " <<new_index_id<<endd; 
		    
		    if( strlen(new_index_id) ==0 )
		    {
		       //无相同数据，将数据插入到新表中
		       insertDataDX(tmp_des.index_id);
		       memset(tmp_des.index_id,0,sizeof(tmp_des.index_id));
	           memset(tmp_des.country_id,0,sizeof(tmp_des.country_id));
	           memset(tmp_des.country_name,0,sizeof(tmp_des.country_name));
	           memset(tmp_des.rate,0,sizeof(tmp_des.rate));
	           memset(tmp_des.chinese_name,0,sizeof(tmp_des.chinese_name));
	           memset(tmp_des.effect_date,0,sizeof(tmp_des.effect_date));
	           memset(tmp_des.expire_date,0,sizeof(tmp_des.expire_date));
	           memset(tmp_des.dir,0,sizeof(tmp_des.dir));
	           memset(tmp_des.country_code,0,sizeof(tmp_des.country_code));
		    }
		    else
		    {
		       //有相同数据，整个文件取消退出
		       theJSLog << "表中已有相同数据，不能继续插入" <<endd;
		       insertLog("N",value_num,new_index_id);
		       continue;
		       //return false;
		    }

		 }
		 // 备份原始数据表DESTINATION_ORG
		 if(BakGWDX())
		    insertLog("Y",value_num,"");
   		}else{
	 	    theJSLog<<"connect error."<<endi;
	 	    return false;
	   }
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		//stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "删除固网结算公式数据出错", __FILE__, __LINE__);
    }
	return true;
}


bool Comparedata::insertDataDX(char *index_id)
{	   
   char sql[5000];
   Statement m_stmt = conn.createStatement();
    try{				     
         sprintf(sql,"insert into GWDX_CTRYRATE select * from GWDX_CTRYRATE_ORG "
   	                 " where index_id ='%s'",index_id);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);         
		 m_stmt.execute(); 

		  sprintf(sql,"insert into C_GWDX_RATE(index_id,SETTLE_COUNTRY ,SETTLE_DIR ,FEE ,EFF_TIME , EXP_TIME ,STAT_TIME,VALID_FLAG,DEAL_FLAG) "
		          "select index_id,country_id,dir,rate,effect_date,expire_date,to_char(sysdate,'yyyymmddhh24miss'),'Y','W' from GWDX_CTRYRATE_ORG where index_id='%s'",index_id);
         theJSLog << sql <<endd;
         m_stmt.setSQLString(sql);         
		 m_stmt.execute();
		 
		 m_stmt.commit();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		m_stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "插入固网短信结算公式数据出错", __FILE__, __LINE__);
    }
   
	return true;
}


bool Comparedata::BakGWDX()
{	   
   char sql[5000];
   Statement stmt = conn.createStatement();
   
    try{				       		 		 
		  //将destination 表的内容拷贝到C_ISSF_RATE 表中生成结算规则		
		 sprintf(sql,"truncate table GWDX_CTRYRATE_ORG");
         theJSLog << sql <<endd;
         stmt.setSQLString(sql);         
		 stmt.execute(); 
		 
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		stmt.rollback();
		conn.close();
		throw jsexcp::CException(errno, "插入固网结算公式数据出错", __FILE__, __LINE__);
    }   
	return true;
}

bool Comparedata::insertLog(char *flag,int num,char *destination_id)
{	   
   char sql[5000];
   Statement stmt = conn.createStatement();
   
    try{				     
         sprintf(sql,"insert into D_GWRATE_LOG(deal_date,deal_flag,total_count,err_destination,source_id) "
         	"values(to_char(SYSDATE,'yyyymmddhh24miss'),'%s',%d,'%s','%s')",flag,num,destination_id,source);
         theJSLog << sql <<endd;
         stmt.setSQLString(sql);         
		 stmt.execute();  
		 stmt.commit();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		stmt.rollback();
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
	cout<<"*                    jscompare               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2013-12-03 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;	

    Comparedata compare;
   	std::string log_path,log_level;
   	
   	if (!(argc == 2 ))
	{
	    printf("Usage : %s <source_id> \n",argv[0]);
	    printf("P.S : jscompare GWYY / GWDX \n");
		return false;
	}
   	
   	strcpy(compare.source,argv[1]);
   	theJSLog << "compare.source = " << compare.source <<endd;

	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "获取日志路径失败" );
	}
	
	theJSLog.setLog(log_path.c_str(), 1, "GJGW", "COMPARE", 1);	
	
    bool result;

    if( strcmp(compare.source,"GWYY") ==0 )
        result = compare.DoforGWYY();
    else if( strcmp(compare.source,"GWDX") ==0 )
    	result = compare.DoforGWDX();
    if(result == true)
    	theJSLog << "  Compare and insert  " << compare.source << " data Success !" <<endi;
    else
    	theJSLog << "  Compare and insert  " << compare.source << " Fail !" <<endi;
   return 0;
}

