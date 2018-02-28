
#ifndef __SUM_DAY_CPP__
#define __SUM_DAY_CPP__

#include "CDsum.h"

CDsum::CDsum()
{
	memset(szSumDate,0,sizeof(szSumDate));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	//memset(source_id,0,sizeof(source_id));
	iSourceCount = 0;
	m_enable = false;
	flag1 = true;		
	//flag2 = true;
	pDayList = NULL;
}

CDsum::~CDsum()
{
	if(pDayList != NULL)
	{
		delete[] pDayList;   //释放结构体指针数组
	}
	
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "释放容灾平台失败,返回值=%d", ret);
			theJSLog<<tmp<<endi;
		}
	}
}

bool CDsum::init(int argc,char** argv)
{
	bool ret = true;

	if(flag1)
	{
		//继承process,注册调度表，待实现 
		cout<<"常驻,继承process.cpp"<<endl;
		if(!PS_Process::init(argc,argv))
		{
			return false;
		}
		
		memset(module_id,0,sizeof(module_id));
		strcpy(module_id,module_name);
		
		module_process_id = getPrcID();

		//PS_Process::setSignalPrc();				//中断信号	
	}
	else		//核心参数需要自己初始化
	{
		/*
		if( !param_cfg.bOnInit() )
		{
			string sErr;
			int nCodeId;
			param_cfg.getError(sErr,nCodeId);
			cerr<<"参数配置接口初始化失败！错误码="<<nCodeId<<", 错误信息="<<sErr<<endl;
			return false;
		}
		*/
		try
		{
			char *pMldName;
			pMldName=strrchr(argv[0], '/');
			if(pMldName==NULL) pMldName=argv[0];
			else pMldName++;
			
			memset(module_id,0,sizeof(module_id));
			strcpy(module_id,pMldName);

			//通过模块名查询出实例ID +1
			if(!(dbConnect(conn)))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init()  连接数据库失败 connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
				return false ;
			}
		
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select process_id from tp_module a,tp_process b where a.module_code = '%s' and a.module_id = b.module_id",module_id);
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			if(!(stmt>>module_process_id))
			{
				cout<<"模块:"<<module_id<<"找不到对应的实例ID 请在tp_process表配置"<<endl;
				return false;
			}
			module_process_id += 1;

			stmt.close();
			conn.close();

		}
		catch(util_1_0::db::SQLException e)
		{ 
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"初始化时数据库查询异常：%s",e.what());
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
			return false;
		}

	}

	if(!drInit())	return false;

	if(!(rtinfo.connect()))
	{
		return false;
	}
	short status;
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;

	 //初始化内存日志接口
	 bool bb = initializeLog(argc,argv,false);  //是否调试模式
	 if(!bb)
	 {
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	 }

	 theJSLog.setLog(szLogPath, szLogLevel,"DAY_SUM","SUMMARY", 001);	//文件日志接口，调用了内存日志接口
	 
	 return ret ;
}


bool CDsum::init(char *source_id, char *source_group_id)
{		
	bool  ret = true;
	
	//初始化日志

	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"init()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return false ;
	}
	
    //int iSourceCount = 0;
	try
	{	
		Statement stmt = conn.createStatement();
		if(source_group_id[0] != '\0')		//查找指定数据源组的数据源
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1)  from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP='%s'",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init()  数据源组[%s]没有配置",source_group_id);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);		

				return false ;
			}
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP='%s'",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init()  数据源组[%s]没有配置数据源ID,个数为0",source_group_id);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);		
				return false;
			}
			
			//查询有效的数据源个数
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(a.source_id) from C_SOURCE_GROUP_CONFIG a,C_SUMTABLE_DEFINE b where a.SOURCE_GROUP='%s' and a.source_id = b.sourceid and VALIDFLAG = 'Y'",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				theJSLog<<"数据源组"<<source_group_id<<"ID都无效"<<endi;
				return false ;
			}
			
			theJSLog<<"有效数据源个数："<<iSourceCount<<endi;

			pDayList = new SDayList[iSourceCount];   //有效的数据源个数
			char strSourceId[8];
			memset(strSourceId,0,sizeof(strSourceId));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select a.source_id from C_SOURCE_GROUP_CONFIG a ,C_SUMTABLE_DEFINE b where a.SOURCE_GROUP='%s' and a.source_id = b.sourceid and VALIDFLAG = 'Y' ",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			int i =0;
			while(stmt>>strSourceId)
			{
				//加载汇总定义表通过数据源ID
				ret = loadSumConfig(strSourceId,source_group_id,i);
				if(ret)  return false;
				memset(strSourceId,0,sizeof(strSourceId));
				i++;		
			}
		
			stmt.close();
		}
		else if(source_id[0] != '\0')
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SOURCE_GROUP_CONFIG where source_id = '%s'",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init() 数据源[%s]没有配置",source_id);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SUMTABLE_DEFINE where sourceid = '%s' and VALIDFLAG = 'Y' ",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				theJSLog<<"数据源ID"<<source_id<<"无效"<<endi;
				return false ;
			}

			theJSLog<<"有效数据源个数："<<iSourceCount<<endi;
			pDayList = new SDayList[1];
			
			//加载汇总定义表通过数据源ID,  可以通过数据源ID找到所属数据源组
			char group[10];
			memset(group,0,sizeof(group));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_group from c_source_group_config where source_id = '%s'",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>group;
			
			stmt.close();

			ret = loadSumConfig(source_id,group,0);
			if(ret)  return false;
		}
		else
		{
			ret = init();
			if(ret == false)  return false;

		}
	
	
	}catch(SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 数据库出错：%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	 }

	conn.close();

	return ret;
}

//加载所有数据源信息
bool CDsum::init()
{			
  /*	
	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"init()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return false ;
	}
*/
	//Statement stmt = conn.createStatement();
	char m_szSrcGrpID[8],strSourceId[8];
	int ret = 0;
	string sql ;
	try
	{	
		Statement stmt = conn.createStatement();
		sql = "select count(1) from c_source_group_define a ,C_SOURCE_GROUP_CONFIG b where a.SOURCE_GROUP = b.SOURCE_GROUP ";
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>iSourceCount;
		if(iSourceCount == 0)
		{
			//cout<<"各个数据源组都没有配置数据源"<<endl;
			sprintf(erro_msg,"init()  各个数据源组都没有配置数据源");
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return false ;
		}
		
		//查询有效的数据源个数
		sql = "select count(1) from c_source_group_define a,C_SOURCE_GROUP_CONFIG b,C_SUMTABLE_DEFINE c where a.SOURCE_GROUP = b.SOURCE_GROUP and b.source_id = c.sourceid and VALIDFLAG = 'Y' " ;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>iSourceCount;
		if(iSourceCount == 0)
		{
			theJSLog<<"各个数据源组"<<"ID汇总都无效"<<endi;

			return false ;
		}

		theJSLog<<"有效数据源个数："<<iSourceCount<<endi;
		pDayList = new SDayList[iSourceCount];

		sql = "select a.source_group,b.SOURCE_ID from c_source_group_define a ,C_SOURCE_GROUP_CONFIG b ,C_SUMTABLE_DEFINE c where a.SOURCE_GROUP = b.SOURCE_GROUP and b.source_id = c.sourceid and VALIDFLAG = 'Y' ";
		stmt.setSQLString(sql);
		stmt.execute();
		memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
		memset(strSourceId,0,sizeof(strSourceId));
		int i = 0;
		while(stmt>>m_szSrcGrpID>>strSourceId)
		{						
			//加载汇总定义表通过数据源ID
			ret = loadSumConfig(strSourceId,m_szSrcGrpID,i);
			if(ret)  return false;
			
			memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
			memset(strSourceId,0,sizeof(strSourceId));
			i++;
		}

		stmt.close();

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	 }
	
	//conn.close();

	return true;

}

//加载所有的统计配置信息
int CDsum::loadSumConfig(char *source_id, char *source_group_id,int pos)
{
	
	strcpy(pDayList[pos].szSourceId,source_id);
	strcpy(pDayList[pos].szSourceGrpId,source_group_id);
	
 try
 {
	int ret = 0;

	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql, "select ORGSUMT,ORGSUMT_TCOL,DAYSUMT,DAYSUMT_TCOL,ORGSUMT_SOUR from C_SUMTABLE_DEFINE where sourceid = '%s'",source_id );
	stmt.setSQLString(sql);
	stmt.execute();
	
	//SDayList pday;
	SCom scom;
	stmt>>scom.iOrgSumt>>scom.szOrgSumtCol>>scom.iDestSumt>>scom.szDestSumtCol>>pDayList[pos].org_source;
		
	//原始结果表和目标表通过configID查表 C_STAT_TABLE_DEFINE	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select table_name from C_STAT_TABLE_DEFINE where CONFIG_ID = %d",scom.iOrgSumt);
	stmt.setSQLString(sql);
	ret = stmt.execute();
	if(ret == 0)  
	{
		sprintf(erro_msg,"没有在C_STAT_TABLE_DEFINE表中配置config_id = '%d' 对应的表",scom.iOrgSumt);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		return -1;
	}
	stmt>>scom.iOrgTableName;

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select table_name from C_STAT_TABLE_DEFINE where CONFIG_ID = %d",scom.iDestSumt);
	stmt.setSQLString(sql);
	ret = stmt.execute();
	if(ret == 0)  
	{
		sprintf(erro_msg,"没有在C_STAT_TABLE_DEFINE表中配置config_id = '%d' 对应的表",scom.iDestSumt);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		return -1;
	}
	stmt>>scom.iDestTableName;
	
	stmt.close();

	pDayList[pos].tableItem = scom;			

	theJSLog<<"数据源ID="<<source_id<<" 原表ID="<<scom.iOrgSumt<<" 原表表名="<<scom.iOrgTableName<<" 原始表时间字段"<<scom.szOrgSumtCol
		<<" 目标表ID="<<scom.iDestSumt<<" 目标表表名="<<scom.iDestTableName<<" 目标表时间字段"<<scom.szDestSumtCol<<endi;
	
	ret = getItemInfo(scom,pDayList[pos].vItemInfo);		//获取输入输出的统计字段的值
	if(ret)  return -1;
	
 }
 catch (SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"loadSumConfig 数据库出错：%s",e.what());
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

	return -1;
 }
	
	return  0;
}

//设置日期
void CDsum::setDate(char* date)
{
	strcpy(szSumDate,date);
}

//设置是否常驻
void CDsum::setDaemon(bool flag)
{
	flag1 =flag;
}

//判断日汇总条件，解析结构SDayList中所有的文件都达到汇总条件，根据不同的情况，返回不同值，
//注意正常汇总和重汇总的返回值
int CDsum::checkDayCondition(SDayList &Sday,char *sumday)
{
	int ret = 0;
try
{
	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype = 'D' and sumdate = '%s'",Sday.szSourceId,sumday);
	cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		theJSLog<<"数据源:"<<Sday.szSourceId<<" 在日期:"<<"["<<sumday<<"]已经做过日汇总"<<endi;
		return 0;
	}

	int in_sum = 0,out_sum = 0;
	
	//从核对结果表d_check_file_detail里取文件，且正确的
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_check_file_detail where check_type = 'AUD' and file_time = '%s' and check_flag = 'Y'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>in_sum;

	//查询d_sch_end表是否存在入库失败的文件
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_sch_end where file_time = '%s' and deal_flag = 'N'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		//memset(sql,0,sizeof(sql));
		//sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-1);
		//stmt.setSQLString(sql);
		//stmt.execute();
		stmt.close();	

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"汇总条件失败:输出表[d_sch_end]中存在入库失败的文件");
		theJSLog.writeLog(0,erro_msg);
		return -1;
	}
	

	//正常情况 核对的入口文件 = d_sch_end 的入口成功的文件Y+话单块出错的文件E
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(*) from d_sch_end where file_time = '%s' and deal_flag in('Y','E')",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum;
	
	if(in_sum != out_sum) 
	{
		theJSLog<<in_sum<<" != "<<out_sum<<",部分文件并未处理完成，需等待"<<endi;
		stmt.close();
		return 2;
	}

	stmt.close();

 }catch(SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"checkDayCondition 数据库出错：%s [%s]",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

	return -11;
 }

	return 1;
}

/*
int CDsum::getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char* org_source,char* source_id,char *sql)
{
  //通过分析vItemInfo，并区分统计项、统计维度、特殊统计值
  //where条件是szOrgSumtCol=fromDateValue
  //group by 统计维度
  //返回sql
  //0为成功；-1为失败。
 
 try
 {

  memset(sql,0,sizeof(sql));
  sprintf(sql,"insert into %s (",szSCom.iDestTableName);
  string sqlA1,sqlA2,sqlB1,sqlB2;				//分别连接 原表统统计维度，原表统计项，目标表统计项
  char currTime[15],time_flag;
 
  char time_col[20];
  memset(time_col,0,sizeof(time_col));

  //sqlA1.append("select  "); 
  for(int i = 0;i<vItemInfo.size();i++)		//查询表的统计项统计维度等
  {
	SItem fromItem = vItemInfo[i].fromItem;
	SItem toItem = vItemInfo[i].toItem;
	
	if(strcmp("",toItem.szItemName))							//目标表不考虑统计维度
	{
		switch(toItem.iItemType)		//区分统计项和统计维度，是为了保证原表和目标表字段对齐
		{
			case 0:							
				//break;
				sqlB2.append(toItem.szItemName).append(",");
				break;
			case 1:						//整形统计维度
				//break ;
			case 2:						
				sqlB1.append(toItem.szItemName).append(",");
				break ;
			case 12:					//标志取当前时间
				time_flag = 'Y';
				strcpy(time_col,toItem.szItemName);  //保存时间字段
				break;
			default :
				break ;
		}
	}
	if(strcmp("",fromItem.szItemName))					//原表字段类型
	{
		switch(fromItem.iItemType)
		{
			case 0:					//标志统计项
			    sqlA2.append(" sum(").append(fromItem.szItemName).append("),");
				break;
			case 1:					//整形统计维度
				//break ;
			case 2:					//字符型统计维度
				sqlA1.append(fromItem.szItemName).append(",");
				break ;
			case 12:
				//??????????????????
				break;
			default :
				break ;
		}
	}
  }
  if(time_flag == 'Y')
  {
		sqlB2.append(time_col) ; 							  //添加到目标表的时间日汇总表的帐务月字段名，值填写汇总的日期
		sqlA2.append("to_char(sysdate,'yyyymmddhh24miss')");  //取数据库系统时间
  }
  else
  {
	sqlB2 = sqlB2.substr(0, sqlB2.length()-1);  //存放目标表字段
	sqlA2 = sqlA2.substr(0, sqlA2.length()-1);  //存放原始表统计项sum()
  }
  
  //sqlB1 = sqlB1.substr(0, sqlB1.length()-1);
  sqlA1 = sqlA1.substr(0, sqlA1.length()-1);  //存放原始表统计维度
 
  sprintf(sql,"%s%s%s) select %s,%s from %s where %s like '%s%s' and %s = '%s' group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,szSCom.szOrgSumtCol,fromDateValue,"%",org_source,source_id,sqlA1);
  
  

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

*/

int CDsum::sum(SDayList &Sday,char *sumday)
{
	int ret = 1;
	char sql[1024];
    try
	{
		ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql);	//拼接sql
		
		string str(sql);
		int pos = str.find("group by ");
		if(pos  == string::npos)
		{
			theJSLog<<"没有找到group by语句"<<endi;
			return -1;
		}	
		
		memset(sql,0,sizeof(sql));
		strcpy(sql,str.substr(0,pos-1).c_str());
		sprintf(sql,"%s and %s = '%s' ",sql,Sday.org_source,Sday.szSourceId);
		strcat(sql,str.substr(pos).c_str());

		if(ret == -1)
		{		
				return -1 ;
		}	
		cout<<"拼接的sql = "<<sql<<endl;	
		
		//theJSLog<<"插入日汇总表["<<Sday.tableItem.iDestTableName<<"]成功"<<endi;

		//查询日汇总记录新增条数，判断条件，时间字段原始表中ORGSUMT_TCOL字段对应的输出字段，SItemPair通过源头找目标
		long cnt = 0;
		char time_col[30];
		memset(time_col,0,sizeof(time_col));

		for(int i = 0;i<Sday.vItemInfo.size();i++)		//查询表的统计项统计维度等
		{
				SItem fromItem = Sday.vItemInfo[i].fromItem;
				//SItem toItem = vItemInfo[i].toItem;
				if(strcmp(fromItem.szItemName,Sday.tableItem.szOrgSumtCol) == 0)
				{
						strcpy(time_col,Sday.vItemInfo[i].toItem.szItemName);
						cnt = 1;
						break;
				}
		}
			
		if(cnt == 0)
		{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"字段[%s]没有在统计格式表[%s]配置",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1 ;
		}
			
		Statement stmt = conn.createStatement();		
		stmt.setSQLString(sql);
		stmt.execute();


		//截取sql查询 仲裁内容为统计项 ,根据结果表来获取仲裁数据
		string strsql;
		strsql = "select ";
		for(int i = 0;i<Sday.vItemInfo.size();i++)		
		{
			SItem toItem = Sday.vItemInfo[i].toItem;
			if(toItem.iItemType == 0)
			{
				strsql.append("sum(").append(toItem.szItemName).append("),");
			}
			
		}
		
		memset(sql,0,sizeof(sql));
		sprintf(sql," from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);

		strsql = strsql.substr(0, strsql.length()-1);
		strsql.append(sql);

		theJSLog<<"查询结果表统计项和数据sql:"<<strsql<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"%s",strsql);

		stmt.setSQLString(sql);
		stmt.execute();
		str  = "";
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		strcpy(m_AuditMsg,Sday.szSourceId);
		strcat(m_AuditMsg,":");
		while(stmt>>str)
		{
			sprintf(m_AuditMsg,"%s%s|",m_AuditMsg,str);
		}
		
		if(!IsAuditSuccess(m_AuditMsg))				//仲裁失败,回滚数据库,删除临时文件
		{
			stmt.rollback();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-2);
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		else		//需不需要统计以前的结果表残留的数据?????????????????????
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);

			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			
			theJSLog<<"新增记录条数"<<cnt<<"	写汇总日志表[D_SUMMARY_RESULT]"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,cnt);
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
			
	}
	catch(SQLException e)
	{
		//dr_AbortIDX();

		//stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"sum 数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	}	
		
	return ret ;

}

//按照数据源进行汇总处理
void CDsum::run()
{	
	int ret = -1;
	char currDate[9],tmpTime[2+1];
	

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "日汇总程序收到退出信号");
		PS_Process::prcExit();
		return;
	}
	
	rtinfo.getDBSysMode(petri_status);
	//cout<<"状态值:"<<petri_status<<endl;
	if(petri_status == DB_STATUS_OFFLINE)	return ;
		
	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return  ;
	}

	if(drStatus == 1)		//主备系统
	{
		//检查trigger触发文件是否存在
		if(!CheckTriggerFile())
		{
			sleep(1);
			return ;
		}

		memset(m_SerialString,0,sizeof(m_SerialString));
		ret = drVarGetSet(m_SerialString);
		if(ret)
		{
			theJSLog<<"备系统同步失败...."<<endi;
			return ;
		}
		
		//获取同步变量,数据源ID,核对日期
		vector<string> data;		
		splitString(m_SerialString,"|",data,true,true);
		
		memset(szSumDate,0,sizeof(szSumDate));
		strcpy(szSumDate,data[1].c_str());
		theJSLog<<"汇总日期:"<<szSumDate<<"数据源ID:"<<data[0]<<endi;

		int pos = -1;
		for(int i = 0;i< iSourceCount;i++)
		{
			if(strcmp(pDayList[i].szSourceId,data[0].c_str()) == 0)
			{
				pos = i;
				break;
			}
		}

		if(pos == -1)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"没有找到该数据源信息[%s]",data[0]);		//环境变量未设置
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				
			dr_AbortIDX();  
			return ;
		}
		
		int iStatus = dr_GetAuditMode(module_id);
		if(iStatus == 1)		//同步模式,	主系统等待指定时间
		{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if( ret == 2)
					{
						times++;
						theJSLog<<"日汇总条件不满足:查找了"<<times<<"次"<<endi;
						sleep(10);
					}
					else
					{
						flag = true;
						break;
					}
				}
				if(!flag)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"日汇总条件不满足");
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					
					dr_AbortIDX();  
					return ;
				}		
		}
		else if(iStatus == 2)		//跟随模式,备系统
		{
				while(1)
				{
					//设置中断
					if(gbExitSig)
					{
						dr_AbortIDX();

						if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
						PS_Process::prcExit();
						return;
					}
					
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if(ret == 2)
					{
						sleep(10);
					}
					else
					{
						break;
					}
				}	
			}
		else
		{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return ;
		}

		//主系统只有做了汇总才会向备系统发送消息,而此时备系统应该满足汇总条件了
		if(ret != 1)
		{
			dr_AbortIDX();

			return ;
		}
		
		theJSLog<<"核对条件满足,准备汇总"<<endi;
		ret = sum(pDayList[pos],szSumDate);
		if(ret == -1) dr_AbortIDX();

	}

	else
	{
		getCurTime(currTime);		//获取当前日汇总时间
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);
		
		memset(tmpTime,0,sizeof(tmpTime));
		strncpy(tmpTime,currTime+8,2);
		if(strcmp(tmpTime,"04") < 0)	return ;  //4点钟以后执行

		memset(szSumDate,0,sizeof(szSumDate));
		addDays(-1,currDate,szSumDate);			//获取昨天的日期
		
		theJSLog<<"汇总日期:"<<szSumDate<<endi;
	}

	for(int i = 0;i<iSourceCount;i++)
	{			
			theJSLog<<"处理数据源:"<<pDayList[i].szSourceId<<endi;
	
			//每个数据源判断是否达到汇总条件
			ret = checkDayCondition(pDayList[i],szSumDate);		//备汇总条件
			
			switch(ret)
			{
				case 0:
						//theJSLog<<"已经做过日汇总"<<endi;
						break;
				case 1:
					
						//发送同步信息
						memset(m_SerialString,0,sizeof(m_SerialString));
						sprintf(m_SerialString,"%s|%s",pDayList[i].szSourceId,szSumDate);
						ret = drVarGetSet(m_SerialString);
						if(ret)
						{
							theJSLog<<"主系统同步失败...."<<endi;
							return ;
						}

						theJSLog<<"核对条件满足,准备汇总"<<endi;
						ret = sum(pDayList[i],szSumDate);
						if(ret == -1) dr_AbortIDX();

						//仲裁
						break;
				case -1:				//有核对或者入库失败的文件,写日志表
						break;
				case 2:					//条件尚未完成，下次查询
						break;		
				case -11:
						conn.close();
						break  ;
				default:

						break;
			}
		
	}
	
	conn.close();

	return ;
}

//Summary -rd-t YYYYMMDD 重新日汇总 重做表明已经做过,且汇总条件已经满足
int CDsum::redorun(char* date,bool del)
{
	int  ret = -1;
	//char currDate[9];	
	
	theJSLog<<"重新汇总日期["<<date<<"]的数据"<<endi;
	
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	
	{
		theJSLog<<"当前数据库状态为备份态,无法汇总..."<<endi;
		return ;
	}

	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return -1 ;
	}
	
	if(drStatus == 1)		//主备系统
	{
		//检查trigger触发文件是否存在
		if(!CheckTriggerFile())
		{
			sleep(1);
			return ;
		}

		memset(m_SerialString,0,sizeof(m_SerialString));
		ret = drVarGetSet(m_SerialString);
		if(ret)
		{
			theJSLog<<"备系统同步失败...."<<endi;
			return ;
		}
		
		//获取同步变量,数据源ID,核对日期,删除标志
		vector<string> data;		
		splitString(m_SerialString,"|",data,true,true);
		
		memset(szSumDate,0,sizeof(szSumDate));
		strcpy(szSumDate,data[1].c_str());
		theJSLog<<"重做汇总日期:"<<szSumDate<<"数据源ID:"<<data[0]<<endi;

		int pos = -1;
		for(int i = 0;i< iSourceCount;i++)
		{
			if(strcmp(pDayList[i].szSourceId,data[0].c_str()) == 0)
			{
				pos = i;
				break;
			}
		}

		if(pos == -1)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"没有找到该数据源信息[%s]",data[0]);		//环境变量未设置
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				
			dr_AbortIDX();  
			return ;
		}
		
		int iStatus = dr_GetAuditMode(module_id);
		if(iStatus == 1)		//同步模式,	主系统等待指定时间
		{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if( ret == 2)
					{
						times++;
						theJSLog<<"日汇总条件不满足:查找了"<<times<<"次"<<endi;
						sleep(10);
					}
					else
					{
						flag = true;
						break;
					}
				}
				if(!flag)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"日汇总条件不满足");
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					
					dr_AbortIDX();  
					return ;
				}		
		}
		else if(iStatus == 2)		//跟随模式,备系统
		{
				while(1)
				{
					//设置中断
					if(gbExitSig)
					{
						dr_AbortIDX();

						if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
						PS_Process::prcExit();
						return;
					}
					
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if(ret == 2)
					{
						sleep(10);
					}
					else
					{
						break;
					}
				}	
			}
		else
		{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return ;
		}

		//主系统只有做了汇总才会向备系统发送消息,而此时备系统应该满足汇总条件了
		if(ret == 0 || ret == 1)
		{
				theJSLog<<"核对条件满足,准备汇总"<<endi;
				ret = sum(pDayList[pos],szSumDate);
				if(ret == -1) dr_AbortIDX();
			
		}
		else
		{
				dr_AbortIDX();
				return ;
		}

	}
	
	else
	{
		for(int i = 0;i<iSourceCount;i++)
		{			
			theJSLog<<"处理数据源:"<<pDayList[i].szSourceId<<endi;

    /*	建议放到检查命令参数时去判断
		getCurTime(currTime);		//获取当前日汇总时间
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);

		memset(sumDate,0,sizeof(sumDate));
		addDays(-1,currDate,sumDate);			//获取昨天的日期
		if(strcmp(date,sumDate) > 0)
		{
			cout<<"重做日期不能超过"<<sumDate<<endl;
			return -1 ;
		}

	*/
			//每个数据源判断是否达到汇总条件
			ret = checkDayCondition(pDayList[i],date);

			switch(ret)
			{
				case 0:					//表示已经做过汇总，不予理会，最可能是这种情况

				case 1:					//表示还没做过汇总
					
					//发送同步信息
					memset(m_SerialString,0,sizeof(m_SerialString));
					sprintf(m_SerialString,"%s|%s|%d",pDayList[i].szSourceId,szSumDate,del);
					ret = drVarGetSet(m_SerialString);
					if(ret)
					{
							theJSLog<<"主系统同步失败...."<<endi;
							return ;
					}
					
					theJSLog<<"核对条件满足,准备汇总"<<endi;
					ret = redosum(pDayList[i],date,del);
					if(ret == -1) dr_AbortIDX();
					break;

				case 2:			//文件未处理完
					break;
		
				case -1 :		//文件核对失败，告警处理
					break;

				case -11:		//数据库异常
					conn.close();
					return -1;

				default:
					break;
			}
		}


	}


	conn.close();

	theJSLog<<"重做日期["<<date<<"]完成"<<endi;

	return ret ;
}


int CDsum::redosum(SDayList &Sday,char *sumday,bool del)
{
		int ret = 0;
		char sql[1024];
		long cnt = 0,last_cnt = 0;
		
		char time_col[30];
		memset(time_col,0,sizeof(time_col));

		for(int i = 0;i<Sday.vItemInfo.size();i++)		//将时间字段查出来
		{
			SItem fromItem = Sday.vItemInfo[i].fromItem;
			if(strcmp(fromItem.szItemName,Sday.tableItem.szOrgSumtCol) == 0)
			{
					strcpy(time_col,Sday.vItemInfo[i].toItem.szItemName);
					ret = 1;
					break;
			}
		}
			
		if(ret == 0)
		{
			sprintf(erro_msg,"字段[%s]没有在统计格式表[%s]配置",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

			return -1 ;
		}

	try
	{
		Statement stmt = conn.createStatement();

		if(del)				//先删除原先汇总表的数据,判断条件，时间字段原始表中ORGSUMT_TCOL字段对应的输出字段，SItemPair通过源头找目标
		{		
			theJSLog<<"删除日期"<<sumday<<"的数据"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			//cout<<"删除汇总表结果sql="<<sql<<endl;
			
			//Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			//stmt.close();
		}
		else
		{		
			//Statement stmt = conn.createStatement();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>last_cnt;
			//cout<<"保留以前数据记录条数:"<<last_cnt<<endl;
		}

		ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql);			//拼接sql
		if(ret == -1)
		{		
				return -1 ;
		}
		
		string str(sql);
		int pos = str.find("group by ");
		if(pos  == string::npos)
		{
			theJSLog<<"没有找到group by语句"<<endi;
			return -1;
		}	
		
		memset(sql,0,sizeof(sql));
		strcpy(sql,str.substr(0,pos-1).c_str());
		sprintf(sql,"%s and %s = '%s' ",sql,Sday.org_source,Sday.szSourceId);
		strcat(sql,str.substr(pos).c_str());


		theJSLog<<"拼接的sql = "<<sql<<endi;	//插入汇总记录
		stmt.setSQLString(sql);
		stmt.execute();
		
		//截取sql查询 仲裁内容为统计项 ,根据结果表来获取仲裁数据
		string strsql;
		strsql = "select ";
		for(int i = 0;i<Sday.vItemInfo.size();i++)		
		{
			SItem toItem = Sday.vItemInfo[i].toItem;
			if(toItem.iItemType == 0)
			{
				strsql.append("sum(").append(toItem.szItemName).append("),");
			}		
		}
		
		memset(sql,0,sizeof(sql));
		sprintf(sql," from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);

		strsql = strsql.substr(0, strsql.length()-1);
		strsql.append(sql);

		theJSLog<<"查询结果表统计项和数据sql:"<<strsql<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"%s",strsql);

		stmt.setSQLString(sql);
		stmt.execute();
		str  = "";
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		strcpy(m_AuditMsg,Sday.szSourceId);
		strcat(m_AuditMsg,":");
		while(stmt>>str)
		{
			sprintf(m_AuditMsg,"%s%s|",m_AuditMsg,str);
		}
		
		if(!IsAuditSuccess(m_AuditMsg))				//仲裁失败,回滚数据库,删除临时文件
		{
			stmt.rollback();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-1);
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		else
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			
			cout<<"统计重汇总记录条数sql:"<<sql<<endl;
			
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			theJSLog<<"新增记录条数:"<<(cnt-last_cnt)<<"	写汇总结果表D_SUMMARY_RESULT"<<endi;
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"RD",sumday,(cnt-last_cnt));
			//cout<<"汇总日志sql = "<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		
		
	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"redosum 数据库出错： %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	}

	return 0 ;
}

//容灾初始化
bool CDsum::drInit()			//由于存在两个实例
{
		//传入模块名和实例ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",module_process_id);

		theJSLog << "初始化容灾平台,模块名:"<< module_id<<" 实例名:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_id,tmp);
		if(ret != 0)
		{
			theJSLog << "容灾平台初始化失败,返回值=" << ret<<endi;
			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		m_enable = true;

		drStatus = _dr_GetSystemState();	//获取主备系统状态
		if(drStatus < 0)
		{
			theJSLog<<"获取容灾平台状态出错: 返回值="<<drStatus<<endi;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"当前系统配置为主系统"<<endi;
		else if(drStatus == 1)	theJSLog<<"当前系统配置为备系统"<<endi;
		else if(drStatus == 2)	theJSLog<<"当前系统配置非容灾系统"<<endi;

		return true;
}

//主系统发送同步变量,备系统获取同步变量
int CDsum::drVarGetSet(char* serialString)
{
		int ret ;
		char tmpVar[5000] = {0};

		//检查容灾平台的切换锁
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"检查容灾切换锁失败,返回值="<<ret<<endi;
			return -1;  
		} 
		//初始化index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"初始化index失败,返回值=" <<ret<<endi;
			dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"传序列串时失败，序列名：["<<serialString<<"]"<<endi;
			dr_AbortIDX();
			return -1;
		}
		//serialString = tmpVar;			//同步索引字符串,主系统是赋值，备系统是取值
		strcpy(serialString,tmpVar);
		//m_AuditMsg = tmpVar;			//要仲裁的字符串

		// <5> 传输实例名  用于主系统注册此IDX的模块调用参数。
		//备系统的index manager检查IDX条件满足后，把使用该函数注册的随机变量作为模块的调用参数trigger相应的进程
		snprintf(tmpVar, sizeof(tmpVar), "%ld", module_process_id);
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			theJSLog<<"传输实例名失败："<<tmpVar<<endi;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> 预提交index  此关键字用于将平台当前内存中的随机变量写入磁盘
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"预提交index失败"<<endi;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> 提交index  	提交Index。在index文件中设置完成标志
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"提交index失败,返回值="<<ret<<endi;
			dr_AbortIDX();  
			return -1;  
		}

		//备系统搜索目录
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<m_SerialString<<endi;

		theJSLog<<"本次的同步串serialString:"<<serialString<<endi;//for test

		return ret;

}

//仲裁字符串
 bool CDsum::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endi;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endi;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "业务全部提交失败(容灾平台)" << endi;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endi;
			dr_AbortIDX();
			return false;
		}
	
	return true;
 }

bool CDsum::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "检查到trigger文件,并删除"<< m_triggerFile <<endl;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"删除trigger文件[%s]失败: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
	}
}

#endif
