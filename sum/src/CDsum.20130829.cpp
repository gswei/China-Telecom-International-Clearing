//2013-08-19 C_SUMTABLE_DEFINE新增字段ORGSUMT_SOUR,用来做汇总数据源的查询条件,比月汇总和日汇总多了个条件ORGSUMT_SOUR=当前数据源ID

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

		//PS_Process::setSignalPrc();				//中断信号	
	}
	else		//核心参数需要自己初始化
	{
		if( !param_cfg.bOnInit() )
		{
			string sErr;
			int nCodeId;
			param_cfg.getError(sErr,nCodeId);
			cerr<<"参数配置接口初始化失败！错误码="<<nCodeId<<", 错误信息="<<sErr<<endl;
			return false;
		}
	}

	 // 从核心参数里面读取日志的路径，级别，
	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10],sql_path[1024],bak_path[1024];
	 CString sKeyVal;
	 sprintf(sParamName, "log.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(szLogPath,0,sizeof(szLogPath));
		strcpy(szLogPath,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"请在核心参数里配置日志的路径"<<endl;
		return false ;
	 }	 
	 sprintf(sParamName, "log.level");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(szLogLevel,0,sizeof(szLogLevel));
		strcpy(szLogLevel,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"请在核心参数里配置日志的级别"<<endl;
		return false ;
	 }
	 sprintf(sParamName, "sql.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(sql_path,0,sizeof(sql_path));
		strcpy(sql_path,(const char*)sKeyVal);
		completeDir(sql_path);					
		sprintf(sqlFile,"%sSummary_day.%d.sql",sql_path,getFlowID()); //维护态时临时写SQL文件 

	 }
	 else
	 {	
		cout<<"请核心参数里配置sql文件所在的路径[sql.path]"<<endl;
		return false ;
	 }

	 //判断目录是否存在
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"日志目录["<<szLogPath<<"]打开失败"<<endl;	
		return false ;
	 }else closedir(dirptr);
	  if((dirptr=opendir(sql_path)) == NULL)
	 {		
		cout<<"SQL目录:"<<sql_path<<"打开失败"<<endl;
		return false ;
	 }else  closedir(dirptr);

	 //初始化内存日志接口
	 bool bb = initializeLog(argc,argv,false);  //是否调试模式
	 if(!bb)
	 {
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	 }

	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"DAY_SUM","SUMMARY", 001);	//文件日志接口，调用了内存日志接口
	 
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
				//cout<<"数据源组"<<source_group_id<<"没有配置"<<endl;
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
				//cout<<"数据源组"<<source_group_id<<"没有配置数据源ID,个数为0"<<endl;
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
		sprintf(erro_msg,"init 数据库出错：%s",e.what());
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
	//cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		//theJSLog<<"["<<sumday<<"]已经做过日汇总"<<endi;
		return 0;
	}

/*
	//sch_format表里获取file_id,file_name,条件file_time = '汇总要求时间' and deal_flag = 'Y' source_id
	sprintf(sql,"select file_id,filename from d_sch_format where source_id = '%s' and file_time = '%s' and deal_flag = 'Y'",Sday.szSourceId,sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	string fileName;
	int file_id;
	while(stmt>>file_id>>fileName)
	{
		Sday.allFiles.insert(map<int,string>::value_type(file_id,fileName));
	}
	
	if(Sday.allFiles.size() == 0)
	{
			cout<<"当天没有接收到文件"<<endl;
			return 0;
	}
	
	//循环find file_id在d_sch_end表    //deal_flag = 'E'的情况怎么办
	char sql2[1024],sql3[1024],filename[1024];
	int cnt = 0;
	sprintf(sql2,"select filename from d_sch_end where file_id = :1 ");
	sprintf(sql3,"select count(1) from d_check_file_detail where filename = :1 and deal_flag = 'Y'");
	Statement stmt2 = conn.createStatement();
	Statement stmt3 = conn.createStatement();
	stmt.setSQLString(sql);
	stmt2.setSQLString(sql2);
	stmt3.setSQLString(sql3);
	for(map<int,string>::const_iterator iter = Sday.allFiles.begin();iter != Sday.allFiles.end(); ++iter)
	{	
		stmt<<iter->first;
		stmt.execute();
		memset(filename,0,sizeof(filename));
		if(stmt>>filename)
		{
			//比较 d_check_file_detail,根据文件名查找是否完成了核对
			stmt2<<filename;
			stmt2.execute();
			stmt2>>cnt;
			if(cnt)
			{
				cout<<"文件"<<filename<<"核对成功"<<endl;
			}
			else
			{
				cout<<"文件"<<filename<<"核对失败，告警处理"<<endl;
				stmt.close();
				stmt2.close();	
				stmt3.close();
				return -2;
			}
		}
		else		//说明此时写文件模块话单块出错，写了err_file_info,由异常输出模块处理，会写 d_out_file_reg 表
		{
			stmt3<<iter->first;
			stmt3.execute();
			stmt3>>cnt;
			if(cnt)
			{
				cout<<"找到文件"<<iter->first<<"在d_out_file_reg表，由异常输出模块处理的"<<endl;			
			}
			else
			{
				cout<<"文件"<<iter->first<<"并未处理完成，需等待"<<endl;
				stmt.close();
				stmt2.close();
				stmt3.close();
				return 2;
			}	
		}
	}
	
	stmt.close();
	stmt2.close();
	stmt3.close();
 */

	//从核对结果表d_check_file_detail里取文件，且正确的
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_check_file_detail where check_type = 'AUD' and file_time = '%s' and check_flag = 'N'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		sprintf(erro_msg,"汇总条件失败:核对表[d_check_file_detail]中存在核对失败的文件");
		theJSLog.writeLog(0,erro_msg);
		return -2;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select content from d_check_file_detail where check_type = 'AUD' and file_time = '%s' and check_flag = 'Y'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	string fileName;
	vector<string> fc;
	while(stmt>>fileName)
	{
		fc.push_back(fileName);
	}
	
	//查询d_sch_end表是否存在入库失败的核对文件
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_sch_end where file_time = '%s' and deal_flag = 'E'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		//theJSLog<<"输出表[d_sch_end]中存在入库失败的文件"<<endi;
		sprintf(erro_msg,"汇总条件失败:输出表[d_sch_end]中存在入库失败的文件");
		theJSLog.writeLog(0,erro_msg);
		return -2;
	}
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select filename from d_sch_end where file_time = '%s' and deal_flag = 'Y'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	vector<string> fe;
	bool find_flag = true;
	while(stmt>>fileName)
	{
		fe.push_back(fileName);
	}
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_out_file_reg where file_type = 'N' and filename = :1");
	stmt.setSQLString(sql);

	for(int i = 0;i<fc.size();i++)
	{	
		find_flag = false;
		for(int j = 0;j<fe.size();j++)
		{
			if(fc[i] == fe[j])					//找到对应文件名，核对成功
			{
				find_flag = true;
				break;
			}
		}
		
		if(!find_flag)	//说明此时写文件模块话单块出错，写了err_file_info,由异常输出模块处理，会写 d_out_file_reg 表
		{
			stmt<<fc[i];
			stmt.execute();
			stmt>>ret;
			if(ret)
			{
				//theJSLog<<"找到文件["<<fc[i]<<"]在[d_out_file_reg]表，由异常输出模块处理"<<endi;			
			}
			else
			{
				theJSLog<<"文件["<<fc[i]<<"]并未处理完成，需等待"<<endi;
				stmt.close();
				return 2;
			}	
		
		}
	}
	
	stmt.close();

 }catch(SQLException e)
 {
	sprintf(erro_msg,"checkDayCondition 数据库出错：%s",e.what());
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
				//sprintf(erro_msg,"拼接sql失败")
				//theJSLog<<"sql"<<"拼接失败"<<endl;
				return -1 ;
		}	
		cout<<"拼接的sql = "<<sql<<endl;

		//return -1;

		ret = insertSql(sql);					//插入汇总记录

		if(ret == 0)
		{
			return -1;
		}
		else
		{
			theJSLog<<"插入日汇总表["<<Sday.tableItem.iDestTableName<<"]成功"<<endi;

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
				//cout<<"字段"<<Sday.tableItem.szOrgSumtCol<<"没有在统计格式表"<<Sday.tableItem.iOrgTableName<<"配置"<<endl;
				sprintf(erro_msg,"字段[%s]没有在统计格式表[%s]配置",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1 ;
			}
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			
			//cout<<"统计汇总记录条数sql:"<<sql<<endl;

			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			//theJSLog<<"新增记录条数:"<<cnt<<endi;
			theJSLog<<"新增记录条数"<<cnt<<"	写汇总日志表[D_SUMMARY_RESULT]"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,cnt);
			cout<<"汇总日志sql = "<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
			
						
		}

		return ret ;

}

//按照数据源进行汇总处理
void CDsum::run()
{	
	int ret = -1;
	char currDate[9];
	while(true)
	{

		if(gbExitSig)
		{
			if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "日汇总程序收到退出信号");
			PS_Process::prcExit();
			return;
		}
	
		if(!(dbConnect(conn)))
		{
			sprintf(erro_msg,"run()  连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  ;
		}

		getCurTime(currTime);		//获取当前日汇总时间
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);

		memset(szSumDate,0,sizeof(szSumDate));
		addDays(-1,currDate,szSumDate);			//获取昨天的日期
		
		theJSLog<<"汇总日期:"<<szSumDate<<endi;

		for(int i = 0;i<iSourceCount;i++)
		{			
			theJSLog<<"处理数据源:"<<pDayList[i].szSourceId<<endi;
	
			//每个数据源判断是否达到汇总条件
			ret = checkDayCondition(pDayList[i],szSumDate);
			
			//pDayList[i].allFiles.clear();		//判断完成后文件给清空掉

			if(ret == 1)			//条件满足
			{	
				theJSLog<<"核对条件满足,准备汇总"<<endi;
				sum(pDayList[i],szSumDate);
			}
			else if(ret == 0)		//已经做过日汇总
			{
				//sum(pDayList[i],szSumDate);
				theJSLog<<"已经做过日汇总"<<endi;
				continue;
			}
			else if(ret  == -2)		//有核对或者入库失败的文件
			{
				continue ;
			}

			else if(ret == 2)
			{
				continue;			//条件尚未完成，下次查询
			}
			else if(ret == -11)
			{
				conn.close();
				return  ;
			}
		}
		
		conn.close();

		sleep(60);
		//break;
	}

	return ;
}

//Summary -rd-t YYYYMMDD 重新日汇总
int CDsum::redorun(char* date,bool del)
{
	int  ret = -1;
	//char currDate[9];	
	
	theJSLog<<"重新汇总日期["<<date<<"]的数据"<<endi;

	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return -1 ;
	}
	
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

		//pDayList[i].allFiles.clear();		//判断完成后文件给清空掉

		switch(ret)
		{
			case 0:								//表示已经做过汇总，不予理会，最可能是这种情况
				  redosum(pDayList[i],date,del);
				  break;

			case 1:							    //表示还没做过汇总
				 theJSLog<<"目前还没有日汇总当天的数据"<<endi;
				 break;

			case 2:			//文件未处理完
				break;
		
			case -2 :		//文件核对失败，告警处理
				break;

			case -11:		//数据库异常
				 conn.close();
				 return -1;

			default:
				break;
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
			//Statement stmt = conn.createStatement();

		if(del)				//先删除原先汇总表的数据,判断条件，时间字段原始表中ORGSUMT_TCOL字段对应的输出字段，SItemPair通过源头找目标
		{		
			theJSLog<<"删除日期"<<sumday<<"的数据"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			cout<<"删除汇总表结果sql="<<sql<<endl;
			
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
		}
		else
		{		
			Statement stmt = conn.createStatement();
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
				//cout<<"sql"<<"拼接失败"<<endl;
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


		theJSLog<<"拼接的sql = "<<sql<<endi;
		ret = insertSql(sql);											//插入汇总记录
	
		if(ret == 0)
		{
			return -1;
		}
		else
		{
			theJSLog<<"插入日汇总表"<<Sday.tableItem.iDestTableName<<"成功"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			
			cout<<"统计重汇总记录条数sql:"<<sql<<endl;
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			theJSLog<<"新增记录条数:"<<(cnt-last_cnt)<<"	写汇总结果表D_SUMMARY_RESULT"<<endi;
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"RD",sumday,(cnt-last_cnt));
			cout<<"汇总日志sql = "<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		
	}catch(SQLException e)
	{
		sprintf(erro_msg,"redosum sql err %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -11;
	}

		return -1 ;
}



#endif
