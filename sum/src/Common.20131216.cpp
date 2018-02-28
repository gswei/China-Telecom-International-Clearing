#ifndef __SUM_COMMON_CPP__
#define __SUM_COMMON_CPP__

#include "Common.h"

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <list>
#include <set>

#include "CF_CException.h" 

//CLog theJSLog;

int getItemInfo(SCom szSCom,  vector<SItemPair> &vItemInfo)
{
   char sqltmp[SQL_LEN+1];
   int iconfig,dconfig;
   iconfig = szSCom.iOrgSumt;
   dconfig = szSCom.iDestSumt;
   int noitem;
   
   DBConnection conn;//数据库连接
   try{			
	if (dbConnect(conn))
	 {
	     Statement stmt = conn.createStatement(); 
	     //先检查是否有数据
	     sprintf(sqltmp,"select count(*) from C_STAT_TABLE_FMT where config_id = %d ",iconfig);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
	     stmt>>noitem;     
		 if(noitem == 0)
		 {
			cout<<"原表["<<szSCom.iOrgTableName<<"]没有配置字段到C_STAT_TABLE_FMT"<<endl;
			return -1 ;
		 }
		 sprintf(sqltmp,"select count(*) from C_STAT_TABLE_FMT where config_id = %d ",dconfig);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
	     stmt>>noitem;
		 if(noitem == 0)
		 {
			cout<<"目标表["<<szSCom.iDestTableName<<"]没有配置字段到C_STAT_TABLE_FMT"<<endl;
			return -1;
		 }
		 
		 //配置中主要是目标表的内容
	     sprintf(sqltmp,"select a.table_item,a.field_name,a.item_type,a.field_begin,a.field_end from C_STAT_TABLE_FMT a where a.config_id = %d order by a.seq ",dconfig);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
		 SItemPair item ;
	  
	    while(stmt>>item.toItem.szItemName>>item.fromItem.szItemName>>item.toItem.iItemType>>item.toItem.iBeginPos>>item.toItem.iEndPos)
		{
				if(item.toItem.iBeginPos > 0)
				{
					item.toItem.iBeginPos--;
					item.toItem.iEndPos = item.toItem.iEndPos - item.toItem.iBeginPos;	//表示长度
					//char tmp[50];
					//memset(tmp,0,sizeof(tmp));
					//sprintf(tmp,"SUBSTR(%s,%d,%d)",item.fromItem.szItemName,item.fromItem.iBeginPos,item.toItem.iEndPos);
					//strcpy(item.fromItem.szItemName,tmp);
					//cout<<"item.fromItem.szItemName = "<<item.fromItem.szItemName<<endl;
				}
				vItemInfo.push_back(item);
		}

		stmt.close();
		conn.close();
	     
	     
	 }else
	 {
  	 	cout<<"connect error."<<endl;
  	 	return -1;
	 }
	    conn.close();

		return 0;

	 }catch( SQLException e )
	 {
  		cout<<e.what()<<endl;
  		theJSLog << "getItemInfo 数据库出错" <<e.what()<< endi;
  		//throw jsexcp::CException(0, "删除数据库出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
     }
	 
  //根据汇总类型填充结构体和vector
  //获取需要处理的所有数据源ID
  //获取数据源ID对应的原始表configid和目标表configid，填充szSCom
  //根据数据源ID获取源头表结构和目标表结构，填充vItemInfo
}


int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char *sql,int type,char* cycle_col,char* rate_cycle,char* condition = NULL)
{
  //通过分析vItemInfo，并区分统计项、统计维度、特殊统计值
  //where条件是szOrgSumtCol=fromDateValue
  //group by 统计维度
  //返回sql
  //0为成功；-1为失败。
 
 try
 {

  memset(sql,0,sizeof(sql));
  if(type == 1)		//日汇总,日汇总的原始表没有后缀日期
  {
  		sprintf(sql,"insert into %s_%s (",szSCom.iDestTableName,rate_cycle);
  }
  else				//月汇总,摊分汇总
  {
  	  sprintf(sql,"insert into %s_%s (",szSCom.iDestTableName,fromDateValue);
  }

  string sqlA1,sqlA2,sqlB1,sqlB2;				//分别连接 原表统统计维度，原表统计项，目标表统计项
  char currTime[15],time_flag;
 
  char time_col[30];
  memset(time_col,0,sizeof(time_col));

  for(int i = 0;i<vItemInfo.size();i++)			//查询表的统计项统计维度等
  {
	SItem fromItem = vItemInfo[i].fromItem;
	SItem toItem = vItemInfo[i].toItem;
	switch(toItem.iItemType)					//区分统计项和统计维度，是为了保证原表和目标表字段对齐
	{
		case 0:							
				sqlB2.append(toItem.szItemName).append(",");
				sqlA2.append(" sum(").append(fromItem.szItemName).append("),");
				break;
		case 1:						//整形统计维度
		case 2:						
				sqlB1.append(toItem.szItemName).append(",");
				if(toItem.iBeginPos == -1)
				{
					sqlA1.append(fromItem.szItemName).append(",");
				}
				else
				{
					char tmp[50];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"SUBSTR(%s,%d,%d)",fromItem.szItemName,fromItem.iBeginPos,toItem.iEndPos);
					sqlA1.append(tmp).append(",");
				}
				break ;
		case 12:					//标志取当前时间
				time_flag = 'Y';
				strcpy(time_col,toItem.szItemName);  //保存时间字段
				break;
		default :
				break ;
   }

  }

  if(time_flag == 'Y')
  {
		sqlB2.append(time_col) ; 							  //添加到目标表的时间日汇总表的帐务月字段名，值填写汇总的日期
		sqlA2.append("to_char(sysdate,'yyyymmdd')");		  //取数据库系统时间
  }
  else
  {
	sqlB2 = sqlB2.substr(0, sqlB2.length()-1);  //存放目标表字段
	sqlA2 = sqlA2.substr(0, sqlA2.length()-1);  //存放原始表统计项sum()
  }
  
  sqlA1 = sqlA1.substr(0, sqlA1.length()-1);  //存放原始表统计维度
 
  //2013-11-18 日汇总的原始表也从分了账期,也就是文件入库也是按照账期入不同的表(主要是由于固网的数据量太大)
  //sprintf(sql,"%s%s%s) select %s,%s from %s where %s like '%s%s' group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,szSCom.szOrgSumtCol,fromDateValue,"%",sqlA1);
  if(type == 1)
  {
	  sprintf(sql,"%s%s%s) select %s,%s from %s_%s where %s ='%s' group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,rate_cycle,szSCom.szOrgSumtCol,fromDateValue,sqlA1);
  }
  else
  {
  	  sprintf(sql,"%s%s%s) select %s,%s from %s_%s group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,fromDateValue,sqlA1);
  }

 }catch(jsexcp::CException e)
 {
	char tmp[1024];
	memset(tmp,0,sizeof(tmp));
	sprintf("getSql()  sql拼接失败: %s",e.GetErrMessage());
	theJSLog.writeLog(0,tmp);
	return -1;
 } 

  return 0;
}

// 根据条件拼SQL语句删除对应数据
bool redo(char *sumdate,char *tablename,char*sumitem)
{
   char sqltmp[SQL_LEN+1];
   int inum;
   DBConnection conn;//数据库连接
   try{			
	if (dbConnect(conn))
	 {
	     Statement stmt = conn.createStatement(); 
	     //先检查是否有数据
	      sprintf(sqltmp,"select count(*) from %s where %s = '%s' ",tablename,sumitem,sumdate);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
	     stmt>>inum;
	     if(inum>0)
	     {
	        theJSLog<< "请先删除数据库表内的数据" <<endi;
	        return false;
	     }else
	     {
	       //删除数据
	       sprintf(sqltmp,"delete from %s where %s = '%s' ",tablename,sumitem,sumdate);
           stmt.setSQLString(sqltmp);	
	       stmt.execute();  
	     }
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "删除数据库出错" << endi;
  		throw jsexcp::CException(0, "删除数据库出错", __FILE__, __LINE__);
  		conn.close();
  		return false;
     }
	 return true; 
}

//直接将sql 语句插入到表中
int insertSql(char *sql)
{
   DBConnection conn;//数据库连接
   //int ret = 0;
   try
   {			
		if (dbConnect(conn))
		{
			Statement stmt = conn.createStatement();        
			stmt.setSQLString(sql);	
			stmt.execute();    
			stmt.close();
		}
		else
		{
  	 		cout<<"connect error."<<endl;
  	 		return 0;
		}

		conn.close();
	 }
	 catch( SQLException e ) 
	 {
		char tmp[1024];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"插入数据库出错: %s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,tmp);

  		//theJSLog << "插入数据库出错"<<e.what()<< endi;
  		//throw jsexcp::CException(0, "插入数据库出错", __FILE__, __LINE__);
  		conn.close();
  		return 0;
     }

	 //cout<<"插入记录条数："<<ret<<endl;

	 return 1 ; //
}



//输入一个时间,判断当月天数
int getDays(char* time)
{
	int year = -1;
	int month = -1;
	int days= -1;

	if(strlen(time) != 6) 
	{
		theJSLog<<"日期:"<<time<<"不合法, YYYYMM"<<endi;
		return -1;
	}
	char tmp[5];
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,time,4);
	year = atoi(tmp);

	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,time+4,2);
	month = atoi(tmp);
	switch(month)
	{
		case 2: /* 判断是否为闰年并且给2月份赋值*/
			days=((year%4==0 && year%100!=0) || year%400==0)?29:28;
			break;
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			days=31;
			break;
		default:
			days=30;
   }

   return days;
}


#endif
