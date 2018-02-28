
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
	//m_enable = false;
	flag1 = true;		
	//drStatus = 2;
	pDayList = NULL;
}

CDsum::~CDsum()
{
	if(pDayList != NULL)
	{
		delete[] pDayList;   //释放结构体指针数组
	}
	
	mdrDeal.dr_ReleaseDR();
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

		//2013-10-22 新增获取重做日汇总的配置参数信息
		 // 从核心参数里面读取日志的路径，级别，
		char sParamName[256];
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
		if(param_cfg.bGetMem(sParamName, sKeyVal) && sKeyVal.isNumber())
		{
			szLogLevel = sKeyVal.toInteger();
		}
		else
		{	
			cerr<<"请在配置文件中配置日志的级别 log.level "<<endl;
			return false ;
		}

		 //判断目录是否存在
		 DIR *dirptr = NULL; 
		if((dirptr=opendir(szLogPath)) == NULL)
		{
			cout<<"日志目录["<<szLogPath<<"]打开失败"<<endl;	
			return false ;
		}else closedir(dirptr);
	 
		char trigger_path[512];
		sprintf(sParamName, "dr.trigger.path");
		if(param_cfg.bGetMem(sParamName, sKeyVal))
		{
			memset(trigger_path,0,sizeof(trigger_path));
			strcpy(trigger_path,(const char*)sKeyVal);

			if((dirptr=opendir(trigger_path)) == NULL)
			{		
				cerr<<"trigger目录:"<<trigger_path<<"打开失败"<<endl;
				return false ;
			}else  closedir(dirptr);
		
			if(trigger_path[strlen(trigger_path)-1] != '/')
			{
				strcat(trigger_path,"/");
			}
		
			char tmp[32];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%ld",module_process_id);

			m_triggerFile = trigger_path;
			m_triggerFile += tmp;
			m_triggerFile += ".trigger";
		
			cout<<"trigger_path:"<<m_triggerFile<<endl;
		}
		else
		{	
			cerr<<"请在配置文件中配置容灾的触发文件路径 dr.trigger.path"<<endl;
			return false ;
		}

	}

	bool flag = true;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-k") == 0)
		{
			theJSLog<<"模块["<<module_id<<"]不进行容灾..."<<endi;
			flag = false;
			mdrDeal.mdrParam.m_enable = false;
			break;
		}	
	}
	
	if(flag && flag1)
	{
		char tmp[12];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		if(!mdrDeal.drInit(module_name,tmp))  return false;
	}

	if(!(rtinfo.connect()))
	{
		return false;
	}
	short status;
	rtinfo.getDBSysMode(petri_status);
	//cout<<"petri status:"<<petri_status<<endl;

	 //初始化内存日志接口
	 bool bb = initializeLog(argc,argv,false);  //是否调试模式
	 if(!bb)
	 {
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
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return false ;
	}
	
    //int iSourceCount = 0;
	try
	{	
		Statement stmt = conn.createStatement();

		// --------- 2014-11-26 -----------------------------------
		char fee[256];
		memset(fee,0,sizeof(fee));
		Check_Sum_Conf fmt;

		theJSLog<<"加载汇总文件配置信息(c_check_file_config)..."<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select check_type,sum_table,cdr_count,cdr_duration,cdr_fee,cdr_dataflow,rate_cycle,source_id,busi_type from c_check_file_config ");
		stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>fmt.check_type>>fmt.sum_table>>fmt.cdr_count>>fmt.cdr_duration>>fee>>fmt.cdr_dataflow>>fmt.rate_cycle>>fmt.source_id>>fmt.busi_type)
		{
			//theJSLog<<"check_type="<<fmt.check_type<<"  sum_table="<<fmt.sum_table<<"   cdr_duration="<<fmt.cdr_duration<<" fee="<<fee
			//		<<"  rate_cycle="<<fmt.rate_cycle<<" source_id="<<fmt.source_id<<"	busi_type="<<fmt.busi_type<<endi;
			
			splitString(fee,",",fmt.cdr_fee,"true");
			monthSumMap.insert( map< string,Check_Sum_Conf >::value_type(fmt.source_id,fmt));

			memset(fee,0,sizeof(fee));
			//fmt.cdr_fee.clear();
			fmt.clear();
		}

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

			ret = true;
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
			else
			{
				ret = true;
			}
		}
		else
		{
			ret = init();
			if(ret == false)  return false;

		}
		
	}catch(SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 数据库出错：%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	 }

	conn.close();
	
	theJSLog<<"初始化完毕...\n"<<endi;

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
			memset(erro_msg,0,sizeof(erro_msg));
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
		sprintf(erro_msg,"init 数据库出错：%s (%s)",e.what(),sql);
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
	sprintf(sql, "select ORGSUMT,ORGSUMT_TCOL,DAYSUMT,DAYSUMT_TCOL from C_SUMTABLE_DEFINE where sourceid = '%s'",source_id );
	stmt.setSQLString(sql);
	stmt.execute();
	
	//SDayList pday;
	SCom scom;
	stmt>>scom.iOrgSumt>>scom.szOrgSumtCol>>scom.iDestSumt>>scom.szDestSumtCol;
		
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
		<<" 目标表ID="<<scom.iDestSumt<<" 目标表表名="<<scom.iDestTableName<<" 目标表时间字段"<<scom.szDestSumtCol<<endi; //" 账期字段:"<<pDayList[pos].rate_cycle<<endi;
	
	ret = getItemInfo(scom,pDayList[pos].vItemInfo);		//获取输入输出的统计字段的值
	if(ret)  return -1;
	
 }
 catch (SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"loadSumConfig 数据库出错：%s(%s)",e.what(),sql);
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
	//char tmp[15];
	//memset(tmp,0,sizeof(tmp));
	//strcpy(tmp,sumday);

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
		//theJSLog<<"数据源:"<<Sday.szSourceId<<" 在日期:"<<"["<<sumday<<"]已经做过日汇总"<<endi;
		return 1;
	}

	int in_sum = 0,out_sum = 0,out_sum0 = 0,out_sum1 = 0,out_sum2 = 0,out_sum3 = 0,out_sum4 = 0;
	
	//从核对结果表d_check_file_detail里取文件，且正确的
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_check_file_detail where SOURCE_ID like '%s%s' and check_type = 'AUD' and file_time = '%s' and check_flag = 'Y'",Sday.szSourceId,"%",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>in_sum;
	

	memset(sql,0,sizeof(sql));		//查询格式化失败的w文件个数 和  空文件,仲裁失败 Z
	sprintf(sql,"select count(*) from d_sch_format where SOURCE_ID like '%s%s' and file_time = '%s' and (deal_flag  = 'E' or deal_flag  = 'Z') union " 
				"select count(*) from d_sch_format where SOURCE_ID like '%s%s' and file_time = '%s' and deal_flag  = 'Y' and record_count = 0 ",Sday.szSourceId,"%",sumday,Sday.szSourceId,"%",sumday);
	
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum1>>out_sum0;
			
	memset(sql,0,sizeof(sql));	   //查询预处理失败的记录条数E,仲裁失败 Z
	sprintf(sql,"select count(*) from d_sch_wrtf where SOURCE_ID like '%s%s' and file_time = '%s' and (deal_flag  = 'E' or deal_flag  = 'Z') ",Sday.szSourceId,"%",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum2;

	
	memset(sql,0,sizeof(sql));	   //查询入库失败的文件数E 和正确的文件数Y
	sprintf(sql,"select count(*) from d_sch_indb where SOURCE_ID like '%s%s' and file_time = '%s' and deal_flag = 'E' union "
				"select count(*) from d_sch_indb where SOURCE_ID like '%s%s' and file_time = '%s' and deal_flag = 'Y' ",Sday.szSourceId,"%",sumday,Sday.szSourceId,"%",sumday);				
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum3>>out_sum4;
	
	if(out_sum3)
	{	
		//memset(sql,0,sizeof(sql));
		//sprintf(sql,"select count(1) from D_BALANCE_DAYCHECK where source_id = '%s' and deal_date = '%s'",Sday.szSourceId,tmp);
		//stmt.setSQLString(sql);
		//stmt.execute();
		//stmt>>ret;
		//if(ret == 0)					   //表示当天没做过平衡校验 
		//{
		//	memset(sql,0,sizeof(sql));	   //查询入库正常的记录是
		//	sprintf(sql,"insert into D_BALANCE_DAYCHECK(SOURCE_ID,INPUT_COUNT,F_ERR_COUNT,R_ERR_COUNT,INDB_COUNT,INDB_ERR_COUNT,BLANCE_FLAG,DEAL_DATE) values('%s',%d,%d,%d,%d,%d,'N','%s')",Sday.szSourceId,in_sum,out_sum1,out_sum2,out_sum4,out_sum3,tmp);
		//	stmt.setSQLString(sql);
		//	stmt.execute();
		//}
	
		stmt.close();	 
		//cout<<"数据源："<<Sday.szSourceId<<" 日期："<<sumday<<endl;
		
	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"  数据源[%s]在日期[%s]汇总条件失败:输出表[d_sch_indb]中存在入库失败的文件,个数[%d]",Sday.szSourceId,sumday,out_sum3);
		theJSLog.writeLog(0,erro_msg);
		return -2;
	}
	
	out_sum = out_sum0+out_sum1+out_sum2+out_sum4;
	if(in_sum != out_sum) 
	{
		theJSLog<<in_sum<<" != "<<out_sum<<" 数据源["<<Sday.szSourceId<<"]在日期["<<sumday<<"]部分文件并未处理完成，需等待"<<endi;
		stmt.close();
		return 2;
	}
	
	//memset(sql,0,sizeof(sql));	  
	//sprintf(sql,"insert into D_BALANCE_DAYCHECK(SOURCE_ID,INPUT_COUNT,F_ERR_COUNT,R_ERR_COUNT,INDB_COUNT,INDB_ERR_COUNT,BLANCE_FLAG,DEAL_DATE) values('%s',%d,%d,%d,%d,%d,'Y','%s')",Sday.szSourceId,in_sum,out_sum1,out_sum2,out_sum4,out_sum3,tmp);
	//stmt.setSQLString(sql);
	//stmt.execute();

	stmt.close();

 }catch(SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"checkDayCondition 数据库出错：%s (%s)",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

	return -1;
 }

	return 0;
}

int CDsum::sum(SDayList &Sday,char *sumday)
{
	int ret = 1;
	char sql[2048];
	Statement stmt,stmt2;
	
	long cdr_cnt = 0,cdr_duration = 0,cdr_dataflow=0,totalFee = 0,feeTmp = 0,FileID=0,deal_flag=0;
	map< string,Check_Sum_Conf >::const_iterator iter;
	getCurTime(currTime);

    try
	{	
		long cnt = 0,total = 0;
		char time_col[30];
		memset(time_col,0,sizeof(time_col));
		//查询日汇总记录新增条数，判断条件，时间字段原始表中ORGSUMT_TCOL字段对应的输出字段，SItemPair通过源头找目标
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
		
		memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
		strcpy(mdrDeal.m_AuditMsg,Sday.szSourceId);
		strcat(mdrDeal.m_AuditMsg,":");
		
		stmt = conn.createStatement();
		stmt2 = conn.createStatement();
		
		//求出当天的账期,1个或2个
		vector<string> vrc;
		vector<string> vrc_tmp;

		string rc;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select distinct(rate_cycle) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and file_time = '%s' order by rate_cycle desc ",Sday.szSourceId,"%",sumday);
		cout<<"sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>rc)
		{
			vrc.push_back(rc);
			vrc_tmp.push_back(rc);
		}

		if(vrc.size() == 0)
		{
			theJSLog<<"当天没有清单文件过来"<<endi;
		}
		
		//2014-07-17 长途业务特殊处理,可能夸三个账期,20140702出账 可能有201405 201406 201407三个账期文件,但201405 201406能归为210406里面
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*)from C_CYCLE_ADJ_DEFINE where source_id = '%s' and vail_flag='Y' ",Sday.szSourceId);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>ret;
		if(ret)
		{
			deal_flag=0;
			if(vrc.size() == 3)
			{
				theJSLog<<"数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"跨三个账期,过滤后补账期"<<vrc[2]<<endi;
				vrc.pop_back();
				deal_flag=1;
			}
			else if(vrc.size() == 2) //  
			{
				memset(sql,0,sizeof(sql));
				sprintf(sql,"select count(*) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and file_time = '%s' and cycle_flag=1 ",Sday.szSourceId,"%",sumday);
				stmt.setSQLString(sql);
				stmt.execute();
				stmt>>ret;
				if(ret)
				{
					theJSLog<<"数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"封帐!"<<endi;

					memset(sql,0,sizeof(sql));
					sprintf(sql,"select count(*) from c_rate_cycle where source_id='%s' and cycle_flag='Y' and rate_cycle='%s'",Sday.szSourceId,vrc[0]);
					stmt.setSQLString(sql);
					stmt.execute();
					stmt>>ret;
					if(ret)
					{
						theJSLog<<"后补+正常,数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的账期"<<vrc[1]<<"文件为后补,"<<"adj to "<<vrc[0]<<endi;	
						vrc.pop_back();
						deal_flag=2;
					}
					else
					{
						theJSLog<<"正常+下月,数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的正常汇总到两个账期|"<<vrc[1]<<"|"<<vrc[0]<<endi;
						deal_flag=4;
					}
				}
				else  
				{
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select count(*) from c_rate_cycle where source_id='%s' and cycle_flag='Y' and rate_cycle='%s'",Sday.szSourceId,vrc[1]);
					stmt.setSQLString(sql);
					stmt.execute();
					stmt>>ret;
					if(ret)
					{
						theJSLog<<"数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的账期"<<vrc[1]<<"文件为后补,"<<"adj to "<<vrc[0]<<endi;
						vrc.pop_back();
						//vrc.pop_back();  暂时去掉 20140901 可能出现201408 201409两个账期的文件
						deal_flag=2;
					}
					else
					{
						theJSLog<<"正常+下月,数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的正常汇总到两个账期|"<<vrc[1]<<"|"<<vrc[0]<<endi;
						deal_flag=3;
					}
				}

			}
		}

		for(int i =0;i<vrc.size();i++)
		{	
			theJSLog<<"话单文件账期:"<<vrc[i]<<endi;
			
			//2013-12-11
			theJSLog<<"删除日期"<<sumday<<"的数据"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);
			cout<<"删除汇总表结果sql="<<sql<<endl;	
			stmt.setSQLString(sql);
			stmt.execute();

			ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql,1,vrc[i].c_str());	//拼接sql

			if(ret == -1)
			{		
				return -1 ;
			}	
			theJSLog<<"拼接的sql = "<<sql<<endi;		
				
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
			sprintf(sql," from  %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);

			strsql = strsql.substr(0, strsql.length()-1);
			strsql.append(sql);

			theJSLog<<"查询结果表统计项和数据sql:"<<strsql<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"%s",strsql);
			sprintf(mdrDeal.m_AuditMsg,"%s%s_%s>",mdrDeal.m_AuditMsg,Sday.tableItem.iDestTableName,vrc[i]);
			stmt.setSQLString(sql);
			stmt.execute();
			string str  = "";
			while(stmt>>str)
			{
				sprintf(mdrDeal.m_AuditMsg,"%s%s|",mdrDeal.m_AuditMsg,str);
			}
			
			//需不需要统计以前的结果表残留的数据?????????????????????
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);

			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			
			theJSLog<<"预增记录条数:"<<cnt<<endi;
			total += cnt;
			
			//----- 2014-11-26 ----------------------------------------------------------------
			if(deal_flag == 0)	//非长途业务或者长途业务单账期时
			{
				iter = monthSumMap.find(Sday.szSourceId);

				if(iter != monthSumMap.end())
				{
					theJSLog<<"统计 通话次数,话单时长,费用....."<<endi;

					memset(sql,0,sizeof(sql));
					sprintf(sql,"delete from d_daysum_log where source_id='%s' and rate_cycle=%s and accum_date=%s",
								 Sday.szSourceId,vrc[i],sumday);
					stmt.setSQLString(sql);
					stmt.execute();
					
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration,iter->second.cdr_dataflow);
					vector<string> vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
							sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s group by FileID ",sql,iter->second.sum_table,vrc[i],time_col,sumday);

					theJSLog<<"(张 分 钱) sql = "<<sql<<endi;

					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration>>cdr_dataflow)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}

						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,curr_dataflow,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,%ld,2,%s,%s)",vrc[i],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,cdr_dataflow,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
				}
				else
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"checkMonthFile 配置表中没有找到文件文件类型为[%s]的信息",Sday.szSourceId);
					theJSLog<<erro_msg<<endw;
				}
			}

		}//for rate_cycle	
		
		//----- 2014-11-27 针对长途业务跨账期时特殊处理----------------------------------------------------------------
		if(deal_flag)	
		{
			iter = monthSumMap.find(Sday.szSourceId);
			if(iter != monthSumMap.end())
			{
				//long cdr_cnt_tmp = 0,cdr_duration_tmp = 0,totalFee_tmp;
				
				theJSLog<<"统计 通话次数,话单时长,费用....."<<endi;

				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from d_daysum_log where source_id='%s' and accum_date=%s",
							 Sday.szSourceId,sumday);
				stmt.setSQLString(sql);
				stmt.execute();
				
				vector<string> vfee;
				if(deal_flag == 2)    //两个账期取1个表
				{ 
					//当前账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[0]);

					theJSLog<<"当前账期数据 (张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[0],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					
					//后补账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"后补账期数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
				}
				else if(deal_flag == 3)    //两个账期取两个表(分开)
				{ 
					//下个月账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[0]);

					theJSLog<<"下个月账期数据 (张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[0],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					//当前账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[1],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"当前账期数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
				}
				else if((deal_flag == 4) || (deal_flag == 1))    //两个账期取两个表(本月账期数据部分作为后补数据)
				{ 
					//下个月账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[0]);

					theJSLog<<"下个月账期数据 (张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[0],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					
					//当前账期数据+下月后补数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[1],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"当前账期数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
						
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"下月后补数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
						
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}			
					//cdr_cnt +=  cdr_cnt_tmp;
					//cdr_duration += cdr_duration_tmp;
					//totalFee += totalFee_tmp;
					
					if(deal_flag == 1)		//上月后补数据
					{
						memset(sql,0,sizeof(sql));
						sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
						vfee = iter->second.cdr_fee;
						for(int i = 0;i<vfee.size();i++)
						{
							sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
						}
						sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[1],time_col,sumday,iter->second.rate_cycle,vrc_tmp[2]);

						theJSLog<<"上月调账后补数据(张 分 钱) sql = "<<sql<<endi;
						stmt.setSQLString(sql);
						stmt.execute();
						
						while(stmt>>FileID>>cdr_cnt>>cdr_duration)
						{
							totalFee = 0;
							for(int i = 0;i<vfee.size();i++)
							{	
								stmt>>feeTmp;
								totalFee += feeTmp;			
							}
							memset(sql,0,sizeof(sql));
							sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
									 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[2],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
							stmt2.setSQLString(sql);
							stmt2.execute();
						}
					}
				}
				//stmt.close();

			}
			else
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"checkMonthFile 配置表中没有找到文件文件类型为[%s]的信息",Sday.szSourceId);
				theJSLog<<erro_msg<<endw;
			}
		}

		//theJSLog<<"wait dr_audit() ..."<<endi;
		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)									//仲裁失败,回滚数据库,删除临时文件
		{
			stmt.rollback();
			stmt2.rollback();
			//memset(sql,0,sizeof(sql));
			//sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-2);
			//stmt.setSQLString(sql);
			//stmt.execute();
			
			stmt2.close();
			stmt.close();
		}
		else									
		{	
			theJSLog<<"总共新增记录条数"<<total<<"	写汇总日志表[D_SUMMARY_RESULT]"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,total);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
			stmt2.close();
		}
		
		ret = 0;
	}
	catch(SQLException e)
	{
		//dr_AbortIDX();
		stmt2.rollback();
		stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"sum 数据库出错：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	}	
		
	return ret ;

}

void CDsum::prcExit()
{
	mdrDeal.dr_ReleaseDR();

	PS_Process::prcExit();
}

//按照数据源进行汇总处理
void CDsum::run()
{	
	//int ret = -1;
	char currDate[8+1],tmpTime[2+1];
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;
	short db_status = 0;

while(1)
{	
	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "日汇总程序收到退出信号");
		prcExit();
		return;
	}
	
	ret=getCmd(event_sn, event_type, param1, param2, src_id);
	if(ret == 1)
	{
		if(event_type == EVT_CMD_STOP)
		{
			theJSLog<<"***********接收到退出命令**********************\n"<<endw;
			prcExit();
		}
	}
	
	theJSLog.reSetLog();

	rtinfo.getDBSysMode(db_status);
	if(db_status != petri_status)
	{
		theJSLog<<"数据库状态切换... "<<petri_status<<"->"<<db_status<<endw;
		int cmd_sn = 0;
		if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
		{
			theJSLog<<"报告数据库更换状态失败！\n"<<endw;
			continue ;
		}
		petri_status = db_status;
	}
	if(petri_status == DB_STATUS_OFFLINE)	continue ;
	
	
	if(mdrDeal.mdrParam.drStatus == 1)		//主备系统
	{
		//检查trigger触发文件是否存在
		if(!mdrDeal.CheckTriggerFile(m_triggerFile))
		{
			sleep(1);
			return ;
		}

		memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
		ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
		if(ret)
		{
			theJSLog<<"备系统同步失败...."<<endw;
			continue ;
		}
		
		//获取同步变量,数据源ID,核对日期
		vector<string> data;		
		splitString(mdrDeal.m_SerialString,"|",data,false,false);
		
		memset(szSumDate,0,sizeof(szSumDate));
		strcpy(szSumDate,data[1].c_str());
		szSumDate[8]='\0';
		theJSLog<<"汇总日期:"<<szSumDate<<"数据源ID:"<<data[0]<<endi;
		
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run()  连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			continue  ;
		}

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
			//dr_AbortIDX();
			mdrDeal.dr_abort();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"没有找到该数据源信息[%s]",data[0]);		//环境变量未设置
			theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);  
			continue ;
		}
		
		//int iStatus = dr_GetAuditMode(module_id);
		int iStatus = mdrDeal.mdrParam.aduitMode;

		if(iStatus == 1)		//同步模式,	主系统等待指定时间
		{	
				bool flag = false;
				int times = 0;
				while(times < 11)
				{
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if( ret == 2)
					{
						times++;
						theJSLog<<"日汇总条件不满足:查找了"<<times<<"次"<<endi;
						sleep(30);
					}
					else
					{
						flag = true;
						break;
					}
				}
				if(!flag)
				{
					//dr_AbortIDX();
					mdrDeal.dr_abort();
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"日汇总条件不满足");
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg); 
					continue ;
				}		
		}
		else if(iStatus == 2)		//跟随模式,备系统
		{
				while(1)
				{
					//设置中断
					if(gbExitSig)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();
						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
						prcExit();
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
	
		//主系统只有做了汇总才会向备系统发送消息,而此时备系统应该满足汇总条件了
		if(ret != 0)
		{
			//dr_AbortIDX();
			mdrDeal.dr_abort();
			continue ;
		}
		
		theJSLog<<"核对条件满足,准备汇总"<<endi;
		ret = sum(pDayList[pos],szSumDate);
		if(ret == -1) 
		{
			//dr_AbortIDX();
			mdrDeal.dr_abort();
		}
		
		conn.close();
	}
	else
	{
		getCurTime(currTime);			//获取当前日汇总时间
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);
		currDate[8] = '\0';

		memset(tmpTime,0,sizeof(tmpTime));
		strncpy(tmpTime,currTime+8,2);
		tmpTime[2] = '\0';
		if(strcmp(tmpTime,"06") < 0)			continue ;  //6点钟以后执行

		memset(szSumDate,0,sizeof(szSumDate));
		addDays(-1,currDate,szSumDate);			//获取昨天的日期
		
		//strcpy(szSumDate,"20131008");			//测试 2013-12-29

		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run()  连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			continue  ;
		}

		for(int i = 0;i<iSourceCount;i++)
		{				
			//每个数据源判断是否达到汇总条件
			ret = checkDayCondition(pDayList[i],szSumDate);		//备汇总条件
			
			switch(ret)
			{
				case 0:
						theJSLog<<"汇总条件满足,处理数据源:"<<pDayList[i].szSourceId<<"汇总日期:"<<szSumDate<<endi;
						//发送同步信息
						memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
						sprintf(mdrDeal.m_SerialString,"%s|%s",pDayList[i].szSourceId,szSumDate);
						ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
						if(ret)
						{
							theJSLog<<"主系统同步失败...."<<endw;
							break;
						}

						ret = sum(pDayList[i],szSumDate);
						if(ret == -1) 
						{
							//dr_AbortIDX();
							mdrDeal.dr_abort();
						}
						break;

				case 1:
						break;
				case -2:				//有核对或者入库失败的文件,写日志表
						break;
				case 2:					//条件尚未完成，下次查询
						break;		
				case -1:
						break  ;
				default:
						break;
			}
		}
		
		conn.close();

		sleep(300);
	}
	
	sleep(30);
  } //while(1)

	//return ;
}

//Summary -rd-t YYYYMMDD 重新日汇总 重做表明已经做过,且汇总条件已经满足
int CDsum::redorun(char* date,bool del)
{
	int  ret = -1;
	
	theJSLog<<"############################重新汇总日期["<<date<<"]的数据################################"<<endi;
	
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	
	{
		theJSLog<<"当前数据库状态为备份态,无法汇总..."<<endw;
		return ;
	}

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return -1 ;
	}	
	
	memset(szSumDate,0,sizeof(szSumDate));
	strcpy(szSumDate,date);
	szSumDate[8] = '\0';

	for(int i = 0;i<iSourceCount;i++)
	{			
		theJSLog<<"##########################处理数据源:"<<pDayList[i].szSourceId<<"####################"<<endi;
			
		ret = redosum(pDayList[i],date,del);	
	}

	conn.close();

	theJSLog<<"##########################重做日期["<<date<<"]完成################################"<<endi;

	return ret ;
}


int CDsum::redosum(SDayList &Sday,char *sumday,bool del)
{
	int ret = 0;
	char sql[2048];
	
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
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"字段[%s]没有在统计格式表[%s]配置",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

		return -1 ;
	}

	Statement stmt,stmt2;
	
	long cdr_cnt = 0,cdr_duration = 0,cdr_dataflow=0,totalFee = 0,feeTmp = 0,FileID=0,deal_flag=0;
	map< string,Check_Sum_Conf >::const_iterator iter;
	getCurTime(currTime);

	try
	{
		long cnt = 0,last_cnt = 0,total = 0;

		stmt = conn.createStatement();
		stmt2 = conn.createStatement();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,DEALTIME,DEAL_FLAG) values('%s','%s','%s',to_char(sysdate,'yyyymmddhh24miss'),'W')",Sday.szSourceId,"RD",sumday); 
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();

		//memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		//strcpy(m_AuditMsg,Sday.szSourceId);
		//strcat(m_AuditMsg,":");

		//求出当天的账期,1个或2个
		vector<string> vrc;
		vector<string> vrc_tmp;

		string rc;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select distinct(rate_cycle) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and file_time = '%s' order by rate_cycle desc ",Sday.szSourceId,"%",sumday);
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>rc)
		{
			vrc.push_back(rc);
			vrc_tmp.push_back(rc);
		}

		if(vrc.size() == 0)
		{
			theJSLog<<"当天没有清单文件过来"<<endi;
		}
		
		//2014-07-17 长途业务特殊处理,可能夸三个账期,20140702出账 可能有201405 201406 201407三个账期文件,但201405 201406能归为210406里面
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*)from C_CYCLE_ADJ_DEFINE where source_id = '%s' and vail_flag='Y' ",Sday.szSourceId);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>ret;
		if(ret)
		{
			deal_flag=0;	//默认一个账期

			if(vrc.size() == 3)
			{
				theJSLog<<"数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"跨三个账期,过滤后补账期"<<vrc[2]<<endi;
				vrc.pop_back();

				deal_flag=1;
			}
			else if(vrc.size() == 2) 
			{
				memset(sql,0,sizeof(sql));
				sprintf(sql,"select count(*) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and file_time = '%s' and cycle_flag=1 ",Sday.szSourceId,"%",sumday);
				stmt.setSQLString(sql);
				stmt.execute();
				stmt>>ret;
				if(ret)
				{
					theJSLog<<"数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"封帐!"<<endi;

					memset(sql,0,sizeof(sql));
					sprintf(sql,"select count(*) from c_rate_cycle where source_id='%s' and cycle_flag='Y' and rate_cycle='%s'",Sday.szSourceId,vrc[0]);
					stmt.setSQLString(sql);
					stmt.execute();
					stmt>>ret;
					if(ret)
					{
						theJSLog<<"后补+正常,数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的账期"<<vrc[1]<<"文件为后补,"<<"adj to "<<vrc[0]<<endi;	
						vrc.pop_back();
						deal_flag=2;
					}
					else
					{
						theJSLog<<"正常+下月,数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的正常汇总到两个账期|"<<vrc[1]<<"|"<<vrc[0]<<endi;
						deal_flag=4;
					}
				}
				else  
				{
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select count(*) from c_rate_cycle where source_id='%s' and cycle_flag='Y' and rate_cycle='%s'",Sday.szSourceId,vrc[1]);
					stmt.setSQLString(sql);
					stmt.execute();
					stmt>>ret;
					if(ret)
					{
						theJSLog<<"数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的账期"<<vrc[1]<<"文件为后补,"<<"adj to "<<vrc[0]<<endi;
						vrc.pop_back();
						//vrc.pop_back();  暂时去掉 20140901 可能出现201408 201409两个账期的文件
						deal_flag=2;
					}
					else
					{
						theJSLog<<"正常+下月,数据源:"<<Sday.szSourceId<<"在日期:"<<sumday<<"的正常汇总到两个账期|"<<vrc[1]<<"|"<<vrc[0]<<endi;
						deal_flag=3;
					}
				}

			}
		}

		for(int i =0;i<vrc.size();i++)
		{
			theJSLog<<"话单文件账期:"<<vrc[i]<<endi;

			if(del)				//先删除原先汇总表的数据,判断条件，时间字段原始表中ORGSUMT_TCOL字段对应的输出字段，SItemPair通过源头找目标
			{		
				theJSLog<<"删除日期"<<sumday<<"的数据"<<endi;
				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);
				cout<<"删除汇总表结果sql="<<sql<<endl;
			
				stmt.setSQLString(sql);
				stmt.execute();
			}
			else
			{		
				memset(sql,0,sizeof(sql));
				sprintf(sql,"select count(1) from %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);
				stmt.setSQLString(sql);
				stmt.execute();
				stmt>>last_cnt;
				//cout<<"保留以前数据记录条数:"<<last_cnt<<endl;
			}

			ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql,1,vrc[i].c_str());			//拼接sql

			if(ret == -1)
			{		
				return -1 ;
			}

			theJSLog<<"拼接的sql = "<<sql<<endi;	//插入汇总记录
			stmt.setSQLString(sql);
			stmt.execute();
		
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);	
			cout<<"统计重汇总记录条数sql:"<<sql<<endl;		
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			
			theJSLog<<"预增记录条数:"<<(cnt-last_cnt)<<endi;
			total += (cnt-last_cnt);

			//----- 2014-11-26 ----------------------------------------------------------------
			if(deal_flag == 0)	//非长途业务或者长途业务单账期时
			{
				iter = monthSumMap.find(Sday.szSourceId);

				if(iter != monthSumMap.end())
				{
					theJSLog<<"统计 通话次数,话单时长,费用....."<<endi;

					memset(sql,0,sizeof(sql));
					sprintf(sql,"delete from d_daysum_log where source_id='%s' and rate_cycle=%s and accum_date=%s",
								 Sday.szSourceId,vrc[i],sumday);
					stmt.setSQLString(sql);
					stmt.execute();
					
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration,iter->second.cdr_dataflow);
					vector<string> vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
							sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s group by FileID ",sql,iter->second.sum_table,vrc[i],time_col,sumday);

					theJSLog<<"(张 分 钱) sql = "<<sql<<endi;

					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration>>cdr_dataflow)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}

						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,curr_dataflow,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,%ld,2,%s,%s)",vrc[i],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,cdr_dataflow,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
				}
				else
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"checkMonthFile 配置表中没有找到文件文件类型为[%s]的信息",Sday.szSourceId);
					theJSLog<<erro_msg<<endw;
				}
			}
		}
		
		//----- 2014-11-27 针对长途业务跨账期时特殊处理----------------------------------------------------------------
		if(deal_flag)	
		{
			iter = monthSumMap.find(Sday.szSourceId);
			if(iter != monthSumMap.end())
			{
				//long cdr_cnt_tmp = 0,cdr_duration_tmp = 0,totalFee_tmp;
				
				theJSLog<<"统计 通话次数,话单时长,费用....."<<endi;

				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from d_daysum_log where source_id='%s' and accum_date=%s",
							 Sday.szSourceId,sumday);
				stmt.setSQLString(sql);
				stmt.execute();
				
				vector<string> vfee;
				if(deal_flag == 2)    //两个账期取1个表
				{ 
					//当前账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[0]);

					theJSLog<<"当前账期数据 (张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[0],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					
					//后补账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"后补账期数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
				}
				else if(deal_flag == 3)    //两个账期取两个表(分开)
				{ 
					//下个月账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[0]);

					theJSLog<<"下个月账期数据 (张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[0],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					//当前账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[1],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"当前账期数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
				}
				else if((deal_flag == 4) || (deal_flag == 1))    //两个账期取两个表(本月账期数据部分作为后补数据)
				{ 
					//下个月账期数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[0]);

					theJSLog<<"下个月账期数据 (张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
					
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[0],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					
					//当前账期数据+下月后补数据
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[1],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"当前账期数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
						
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}
					
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
					vfee = iter->second.cdr_fee;
					for(int i = 0;i<vfee.size();i++)
					{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
					}
					sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[0],time_col,sumday,iter->second.rate_cycle,vrc_tmp[1]);

					theJSLog<<"下月后补数据(张 分 钱) sql = "<<sql<<endi;
					stmt.setSQLString(sql);
					stmt.execute();
					
					while(stmt>>FileID>>cdr_cnt>>cdr_duration)
					{
						totalFee = 0;
						for(int i = 0;i<vfee.size();i++)
						{	
							stmt>>feeTmp;
							totalFee += feeTmp;			
						}
						
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
								 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[1],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
						stmt2.setSQLString(sql);
						stmt2.execute();
					}			
					//cdr_cnt +=  cdr_cnt_tmp;
					//cdr_duration += cdr_duration_tmp;
					//totalFee += totalFee_tmp;
					
					if(deal_flag == 1)		//上月后补数据
					{
						memset(sql,0,sizeof(sql));
						sprintf(sql,"select FileID,sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
						vfee = iter->second.cdr_fee;
						for(int i = 0;i<vfee.size();i++)
						{
							sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
						}
						sprintf(sql,"%s from %s_%s where %s=%s and %s = '%s' group by FileID ",sql,iter->second.sum_table,vrc_tmp[1],time_col,sumday,iter->second.rate_cycle,vrc_tmp[2]);

						theJSLog<<"上月调账后补数据(张 分 钱) sql = "<<sql<<endi;
						stmt.setSQLString(sql);
						stmt.execute();
						
						while(stmt>>FileID>>cdr_cnt>>cdr_duration)
						{
							totalFee = 0;
							for(int i = 0;i<vfee.size();i++)
							{	
								stmt>>feeTmp;
								totalFee += feeTmp;			
							}
							memset(sql,0,sizeof(sql));
							sprintf(sql,"insert into d_daysum_log(rate_cycle,source_id,fileid,curr_call_times,curr_duration,curr_fee,data_flag,accum_date,stat_time)"
									 " values (%s,'%s',%ld,%ld,%ld,%ld,2,%s,%s)",vrc_tmp[2],Sday.szSourceId,FileID,cdr_cnt,cdr_duration,totalFee,sumday,currTime);
							stmt2.setSQLString(sql);
							stmt2.execute();
						}
					}
				}
				//stmt.close();

			}
			else
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"checkMonthFile 配置表中没有找到文件文件类型为[%s]的信息",Sday.szSourceId);
				theJSLog<<erro_msg<<endw;
			}
		}

		theJSLog<<"新增记录条数:"<<total<<"	写汇总结果表D_SUMMARY_RESULT"<<endi;		
		memset(sql,0,sizeof(sql));
		//sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"RD",sumday,total);
		sprintf(sql,"update D_SUMMARY_RESULT set DEAL_FLAG='Y',SUMCOUNT=%d,DEALTIME=to_char(sysdate,'yyyymmddhh24miss') where SOURCEID='%s' and SUMDATE='%s' and DEAL_FLAG='W' ",total,Sday.szSourceId,sumday);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();
		stmt2.close();
		
		ret = 0;
		
	}catch(SQLException e)
	{
		stmt.rollback();
		stmt2.rollback();
		stmt2.close();
		
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"redosum 数据库出错： %s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_SUMMARY_RESULT set DEAL_FLAG='N',DEAL_DESC='%s',DEALTIME=to_char(sysdate,'yyyymmddhh24miss') where SOURCEID='%s' and SUMDATE='%s' and DEAL_FLAG='W' ",e.what(),Sday.szSourceId,sumday);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();	
		//cout<<"sql = "<<sql<<endl;

		return -1;
	}

	return 0 ;
}

/*
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
			theJSLog << "容灾平台初始化失败,返回值=" << ret<<endw;
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
			theJSLog<<"获取容灾平台状态出错: 返回值="<<drStatus<<endw;
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
		int ret  = 0;
		char tmpVar[5000] = {0};
		
		if(!m_enable)
		{
			return ret;
		}

		//检查容灾平台的切换锁
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"检查容灾切换锁失败,返回值=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_CHECK_LOCK_ERR,erro_msg);

			return -1;  
		} 
		//初始化index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"初始化index失败,返回值=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_IDX_ERR,erro_msg);

			//dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"传序列串时失败，序列名:%s",serialString);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		if(drStatus == 1)
		{
			//serialString = tmpVar;			//同步索引字符串,主系统是赋值，备系统是取值
			strcpy(serialString,tmpVar);
			//m_AuditMsg = tmpVar;			//要仲裁的字符串
		}

		theJSLog<<"本次的同步串serialString:"<<serialString<<endi;//for test

		// <5> 传输实例名  用于主系统注册此IDX的模块调用参数。
		//备系统的index manager检查IDX条件满足后，把使用该函数注册的随机变量作为模块的调用参数trigger相应的进程
		snprintf(tmpVar, sizeof(tmpVar), "%ld", module_process_id);
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"传输实例名失败:%s",tmpVar);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> 预提交index  此关键字用于将平台当前内存中的随机变量写入磁盘
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"预提交index失败");
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> 提交index  	提交Index。在index文件中设置完成标志
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"提交index失败,返回值=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_COMMIT_IDX_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;  
		}

		//备系统搜索目录
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<m_SerialString<<endi;

		return ret;

}

//仲裁字符串
 int CDsum::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;

		if(!m_enable)
		{
			return ret;
		}

		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			//theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endw;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"容灾仲裁失败,结果:%d,本端:%s",ret,dealresult);
			theJSLog.writeLog(LOG_CODE_DR_AUDIT_ERR,erro_msg);

			dr_AbortIDX();
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endw;
			dr_AbortIDX();
		}
		else if(4 == ret)
		{
			theJSLog<<"对端idx异常终止..."<<endw;
			dr_AbortIDX();
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"业务全部提交失败(容灾平台),返回值=%d",ret);
				theJSLog.writeLog(LOG_CODE_DR_COMMIT_SUCCESS_ERR,erro_msg);

				//dr_AbortIDX();
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endw;
			dr_AbortIDX();
		}
	
	return ret;
 }

bool CDsum::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "检查到trigger文件,并删除"<< m_triggerFile <<endi;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"删除trigger文件[%s]失败: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
		return false;
	}

	return true;
}

*/

#endif
