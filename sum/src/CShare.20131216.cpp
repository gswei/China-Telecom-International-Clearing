
#ifndef __SUM_SHARE_CPP__
#define __SUM_SHARE_CPP__

#include "CShare.h"

CShare::CShare()
{
	memset(szSumMonth,0,sizeof(szSumMonth));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	iSourceCount = 0;
	//m_enable = false;
	//drStatus = 2;
	pShareList = NULL;
}

CShare::~CShare()
{
	if(pShareList != NULL)
	{
		delete[] pShareList;   //释放结构体指针数组
	}

	/*
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "释放容灾平台失败,返回值=%d", ret);
			theJSLog<<tmp<<endw;
		}
	}
	*/
}

bool CShare::init(int argc,char** argv)
{
	bool ret = true;

	if( !param_cfg.bOnInit() )		//核心参数需要自己初始化
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"参数配置接口初始化失败！错误码="<<nCodeId<<", 错误信息="<<sErr<<endl;
		return false;
	}

	char *pMldName;
	pMldName=strrchr(argv[0], '/');
	if(pMldName==NULL) pMldName=argv[0];
	else pMldName++;
			
	memset(module_id,0,sizeof(module_id));
	strcpy(module_id,pMldName);
	
	bool redo =false;			//区别摊分汇总和重做摊分汇总的标志,实例ID不一样
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-rf") == 0) 
		{
			redo = true;
		}
	}

	try
	{
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

			module_process_id += 4;
			if(redo) module_process_id++;
	
			stmt.close();
			conn.close();
	}
	catch(util_1_0::db::SQLException e)
	{ 
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"初始化时数据库查询异常：%s(%s)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
			return false;
	}
/*
	bool flag = true;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-k") == 0)
		{
			theJSLog<<"模块["<<module_id<<"]不进行容灾..."<<endi;
			flag = false;
			m_enable = false;
			break;
		}	
	}
	
	if(flag && !redo)
	{
		if(!drInit())  return false;
	}
*/

	if(!(rtinfo.connect()))
	{
		return false;
	}
	short status;
	rtinfo.getDBSysMode(petri_status);
	//cout<<"petri status:"<<petri_status<<endl;

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

	 //初始化内存日志接口
	 bool bb = initializeLog(argc,argv,false);  //是否调试模式
	 if(!bb)
	 {
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	 }

	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"SETT_SUM","SUMMARY", 001);	//文件日志接口，调用了内存日志接口
	 
	 return ret ;
}

bool CShare::init(char* source_id, char* source_group_id,char* month)
{		
	bool  ret = true;
	
	strcpy(szSumMonth,month);
	
	szSumMonth[6] = '\0';		//表示终止符号

	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"init()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return false ;
	}
	
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

			pShareList = new SShareList [iSourceCount];   //有效的数据源个数
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
			pShareList = new SShareList [1];
			
			//加载汇总定义表通过数据源ID,  可以通过数据源ID找到所属数据源组
			char group[10];
			memset(group,0,sizeof(group));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_group from c_source_group_config where source_id = '%s'",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>group;
			
			stmt.close();

			ret =loadSumConfig(source_id,group,0);
			if(ret)  return false;
			else
			{
				ret = true;
			}
		}

		else
		{
			stmt.close();
			ret = init(month);
			if(ret ==  false)  return false;
		}
		
	}catch(SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 数据库出错：%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	 }

	conn.close();

	return ret;
}

//加载所有数据源信息
bool CShare::init(char* month)
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
		pShareList = new SShareList [iSourceCount];

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
		sprintf(erro_msg,"init 数据库出错：%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	 }
	 catch(jsexcp::CException &e) 
	 {	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init() : %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//字段转换出错
		return false;
	 }

	//conn.close();

	return true;
}

//加载所有的统计配置信息 -1出错  0表示正常
int CShare::loadSumConfig(char *source_id, char *source_group_id,int pos)
{
	
	strcpy(pShareList[pos].szSourceId,source_id);
	strcpy(pShareList[pos].szSourceGrpId,source_group_id);
	
 try
 {
	int ret = 0;

	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql, "select MONSUMT,MONSUMT_TCOL,SETTSUMT,SETTSUMT_TCOL from C_SUMTABLE_DEFINE where sourceid = '%s'",source_id );
	stmt.setSQLString(sql);
	stmt.execute();

	SCom scom;
	stmt>>scom.iOrgSumt>>scom.szOrgSumtCol>>scom.iDestSumt>>scom.szDestSumtCol;
		
	//原始结果表和目标表通过configID查表 C_STAT_TABLE_DEFINE
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select table_name from C_STAT_TABLE_DEFINE where CONFIG_ID = %d",scom.iOrgSumt);
	stmt.setSQLString(sql);
	ret = stmt.execute();
	if(ret == 0)  
	{
		memset(erro_msg,0,sizeof(erro_msg));
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
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"没有在C_STAT_TABLE_DEFINE表中配置config_id = '%d' 对应的表",scom.iDestSumt);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		return -1;
	}
	stmt>>scom.iDestTableName;
	
	stmt.close();

	pShareList[pos].tableItem = scom;			

	theJSLog<<"数据源ID="<<source_id<<" 原表ID="<<scom.iOrgSumt<<" 原表表名="<<scom.iOrgTableName<<" 原始表时间字段"<<scom.szOrgSumtCol
		<<" 目标表ID="<<scom.iDestSumt<<" 目标表表名="<<scom.iDestTableName<<"  目标表时间字段"<<scom.szDestSumtCol<<endi;
	
	ret = getItemInfo(scom,pShareList[pos].vItemInfo);		//获取输入输出的统计字段的值
	if(ret)  return -1;
	
	ret = loadShareConfig(source_id,pShareList[pos]);		//加载摊分配表，计算费用或者占用比
	if(ret)  return -1;
	
 }catch(util_1_0::db::SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"loadSumConfig 数据库出错：%s(sql)",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

	return -1;
 }
 catch(jsexcp::CException &e) 
 {	
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"loadSumConfig() : %s",e.GetErrMessage());
	theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//字段转换出错
	return -1;
 }
	return  0;
}


//加载摊分配表，计算费用或者占用比
int CShare::loadShareConfig(char*source_id,SShareList &Share)
{
	int ret = 0,settle_type  = 0,cnt = 0;
	char settle_id[6],varname[20],varvalue[20];

try
{
	Statement stmt,stmt2;
	stmt = conn.createStatement();

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from c_settlesum_config where source_id = '%s'",source_id);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>cnt;
	if(cnt == 0)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"数据源[%s]没有配置分摊ID",source_id);
		//theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		theJSLog<<erro_msg<<endw;
		strcpy(Share.szFlag,"N");
		return 0;
	}

	//**************************************************************************************************

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select settle_type,settle_id from c_settlesum_config where source_id = '%s'",source_id);
	stmt.setSQLString(sql);
	stmt.execute();
	
	//char sql2[512];
	memset(sql,0,sizeof(sql));
	stmt2 = conn.createStatement();
	sprintf(sql,"select varname,varvalue from C_SETTLESUM_DETAIL where settle_id = :1 ");
	stmt2.setSQLString(sql);

	memset(settle_id,0,sizeof(settle_id));
	while(stmt>>settle_type>>settle_id)
	{
		theJSLog<<"settle_type="<<settle_type<<" settle_id="<<settle_id<<":"<<endi;
		if(settle_type == 1)
		{
			Share.shareFeeFlag = true;
			memset(varname,0,sizeof(varname));
			memset(varvalue,0,sizeof(varvalue));
			
			stmt2<<settle_id;
			stmt2.execute();
			while(stmt2>>varname>>varvalue)
			{
				if(strcmp(varname,"SETTLESUM_COL") == 0)		
				{
					strcpy(Share.stShareFee.settlesum_col,varvalue);
				}
				else if(strcmp(varname,"SETTLEOUT_FOMULAID") == 0)
				{
					strcpy(Share.stShareFee.settle_formula,varvalue);
				}
				else if(strcmp(varname,"SETTLEOUT_FOMULAPARAM") == 0)
				{
					strcpy(Share.stShareFee.settle_formula_param,varvalue);
				}
				else 
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"费用配置项[%s]不合法",varname);
					theJSLog.writeLog(LOG_CODE_PARAM_WRON,erro_msg);

					return -1;
				}
			}

		}
		else if(settle_type == 2)
		{
			Share.sharePercentFlag = true;
			memset(varname,0,sizeof(varname));
			memset(varvalue,0,sizeof(varvalue));
			
			stmt2<<settle_id;
			stmt2.execute();
			while(stmt2>>varname>>varvalue)
			{
				if(strcmp(varname,"BASE_COL") == 0)
				{
					strcpy(Share.stSharePercent.base_col,varvalue);
				}
				else if(strcmp(varname,"PROVINCE_COL") == 0)
				{
					strcpy(Share.stSharePercent.province_col,varvalue);
				}
				else if(strcmp(varname,"PERCENT_POS") == 0)
				{
					Share.stSharePercent.percent_pos = atoi(varvalue);
					if(Share.stSharePercent.percent_pos == 0) Share.stSharePercent.percent_pos = 2; //默认为2
				}
				else if(strcmp(varname,"OUTTABLE_NAME") == 0)
				{
					strcpy(Share.stSharePercent.outtable_name,varvalue);
				}
				else
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"占比配置项[%s]不合法",varname);
					theJSLog.writeLog(LOG_CODE_PARAM_WRON,erro_msg);
					return -1;
				}
			}

		}
		else
		{
			stmt2.close();
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"摊分配置类型错误(1计算费用和,2计算占比) :%d",settle_type);
			theJSLog.writeLog(LOG_CODE_PARAM_WRON,erro_msg);
			return -1;
		}

		memset(settle_id,0,sizeof(settle_id));

	}

	stmt2.close();
	stmt.close();

 }catch (SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"loadShareConfig 数据库出错：%s(sql)",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

	return -1;
 }
	
   return ret ;

}


//设置日期
void CShare::setDate(char* date)
{
	strcpy(szSumMonth,date);
	szSumMonth[6] = '\0';
}


//判断日汇总条件，解析结构 SShareList  中所有的文件都达到汇总条件，根据不同的情况，返回不同值，
//注意正常汇总和重汇总的返回值
int CShare::checkSettCondition(SShareList  &Smonth,char *month)
{
	int ret = 0;
	char tmp[6+1];
	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,month);
	tmp[6] = '\0';
try
{
	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype = 'F' and sumdate = '%s'",Smonth.szSourceId,tmp);
	//cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		stmt.close();
		theJSLog<<"该月已经做过摊分汇总"<<endi;
		return 1;
	}

	//***********************************
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype in('M','RM') and sumdate = '%s'",Smonth.szSourceId,tmp);
	//cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret == 0)
	{
		stmt.close();
		theJSLog<<"该月还没做过月汇总,请先完成月汇总..."<<endw;
		return 2;
	}

	stmt.close();
	
 }catch(util_1_0::db::SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"checkSettCondition 数据库出错：%s(%s)",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

	return -1;
 }

	return 0;
}

int CShare::sum(int deal_type,SShareList  &Smonth,char *month,bool del)
{
		int ret = 1;
		char sql[2048];
		long cnt = 0,last_cnt = 0;

		char time_col[30];
		memset(time_col,0,sizeof(time_col));
		
		//查询日汇总记录新增条数，判断条件，时间字段原始表中ORGSUMT_TCOL字段对应的输出字段，SItemPair通过源头找目标
		for(int i = 0;i<Smonth.vItemInfo.size();i++)		//查询表的统计项统计维度等
		{
			SItem fromItem = Smonth.vItemInfo[i].fromItem;
			if(strcmp(fromItem.szItemName,Smonth.tableItem.szOrgSumtCol) == 0)
			{
						strcpy(time_col,Smonth.vItemInfo[i].toItem.szItemName);
						cnt = 1;
						break;
			}
		}
		
		if(cnt == 0)
		{
			//theJSLog<<"字段"<<Smonth.tableItem.szOrgSumtCol<<"没有在统计格式表"<<Smonth.tableItem.iOrgTableName<<"配置"<<endi;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"字段[%s]没有在统计格式表[%s]配置",Smonth.tableItem.szOrgSumtCol,Smonth.tableItem.iOrgTableName);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1 ;
		}

	Statement stmt;
	try
	{
		stmt = conn.createStatement();
		
		if(del)				//先删除原先汇总表的数据,判断条件，时间字段原始表中ORGSUMT_TCOL字段对应的输出字段，SItemPair通过源头找目标
		{		
			theJSLog<<"删除月份"<<month<<"的数据"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s_%s where %s = '%s'",Smonth.tableItem.iDestTableName,month,time_col,month);
			cout<<"删除汇总表结果sql="<<sql<<endl;
			
			stmt.setSQLString(sql);
			stmt.execute();

			if(Smonth.sharePercentFlag)
			{
				theJSLog<<"删除占比汇总表当月的数据"<<endi;
				SSharePercent percent = Smonth.stSharePercent;

				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from %s where SETTLEMON = '%s' and source_id = '%s'",percent.outtable_name,month,Smonth.szSourceId);
				cout<<"删除占比sql:"<<sql<<endl;
				stmt.setSQLString(sql);
				stmt.execute();
			}
		}
		else
		{		
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from %s_%s where %s = '%s'",Smonth.tableItem.iDestTableName,month,time_col,month);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>last_cnt;
			//cout<<"保留以前数据记录条数:"<<last_cnt<<endl;
		}
		
		ret = getSql(Smonth.tableItem,Smonth.vItemInfo,month,sql);			//拼接sql
		if(ret == -1)
		{		
				return -1 ;
		}	
		theJSLog<<"拼接的sql = "<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//memset(m_AuditMsg,0,sizeof(m_AuditMsg));

		//判断是否重做
		char sum_type[2];
		memset(sum_type,0,sizeof(sum_type));
		if(deal_type == 1)
		{
			strcpy(sum_type,"F");
		}
		else if(deal_type == 2)
		{
			strcpy(sum_type,"RF");
		}

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from  %s_%s where %s = '%s'",Smonth.tableItem.iDestTableName,month,time_col,month);		
		//cout<<"统计摊分汇总记录条数sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		
		ret = shareCal(Smonth,stmt);		//计算总费用或者占比 
		if(ret == -1)
		{
			//stmt.rollback();		//函数里已经回滚了
			stmt.close();

			return ret;
		}
		
		theJSLog<<"新增记录条数:"<<(cnt-last_cnt)<<"	写汇总结果表D_SUMMARY_RESULT"<<endi;
		//写摊分汇总日志
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Smonth.szSourceId,sum_type,month,(cnt-last_cnt));
		//cout<<"汇总日志sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();		
		
		ret = 0;

	}catch(util_1_0::db::SQLException e)
	{
		stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		//cout<<"test:"<<e.what()<<endl;
		sprintf(erro_msg,"sum() 数据库出错：%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	}

	return ret ;

}

//计算总费用或者占比  计算费用和占比查询摊分表的时间字段(C_SUMTABLE_DEFINE表定义的SETTSUMT_TCOL)和通过原始表时间字段找目标表时间字段(统计记录条数和删除结果表记录就是使用该字段)其实是一样的，配置是必须相同
int CShare::shareCal(SShareList  &Smonth,Statement &stmt)
{	
	int ret = 0;

try
{	
	if(Smonth.shareFeeFlag)
	{
		theJSLog<<"计算总费用:"<<endi;
		double total_fee;
		//memset(total_fee,0,sizeof(total_fee));
		SShareFee fee = Smonth.stShareFee;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(%s) from %s_%s where %s = '%s'",fee.settlesum_col,Smonth.tableItem.iDestTableName,szSumMonth,Smonth.tableItem.szDestSumtCol,szSumMonth);
		cout<<"total_fee sql :"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_fee;
		theJSLog<<"总费用:"<<total_fee<<endi;
		
		//设置仲裁信息: 表  费用字段 费用和
		//sprintf(m_AuditMsg,"%s|%s,%.2f",m_AuditMsg,fee.settlesum_col,total_fee);
	
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update C_FORMULA_PARAM_DEF set param_value = '%.2lf' where FORMULA_ID = '%s' and PARAM_NAME = '%s'",total_fee,fee.settle_formula,fee.settle_formula_param);
		cout<<"update sql: "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//if(ret == false) theJSLog<<"参数更新失败: "<<sql<<endi;
	}

	if(Smonth.sharePercentFlag)
	{
		double total_base = 0;
		theJSLog<<"计算占比:"<<endi;
		SSharePercent percent = Smonth.stSharePercent;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(%s) from %s_%s where  %s = '%s'",percent.base_col,Smonth.tableItem.iDestTableName,szSumMonth,Smonth.tableItem.szDestSumtCol,szSumMonth);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;			//占比基数，如通话次数
		cout<<"总基数sql："<<sql<< "总基数值:"<<total_base<<endl;

		//先求出总共有哪些省，再分别计算各省所在比例
		int prov_cnt = 0;
		double prov_base = 0,cur_base = 0,before_base = 0;
		char prov_code[20],sql2[1024],tmp_base[20];
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from %s_%s where %s = '%s' group by %s",Smonth.tableItem.iDestTableName,szSumMonth,Smonth.tableItem.szDestSumtCol,szSumMonth,percent.province_col);
		cout<<"省份个数sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //计算出省份个数
		theJSLog<<"省份个数:"<<prov_cnt<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select %s,sum(%s)from %s_%s where %s = '%s' group by %s order by %s",percent.province_col,percent.base_col,Smonth.tableItem.iDestTableName,szSumMonth,Smonth.tableItem.szDestSumtCol,szSumMonth,percent.province_col,percent.province_col);
		cout<<"各省份所占比sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//设置仲裁信息 省份字段  输出表名  省份:占比
		//sprintf(m_AuditMsg,"%s|%s,%s|",m_AuditMsg,percent.province_col,percent.outtable_name);
		
		//先将数据保存到内存中 省份代码,省份基数
		vector<string> v_prov_code;
		vector<double> v_prov_base;	
		while(stmt>>prov_code>>prov_base)
		{
			v_prov_code.push_back(prov_code);
			v_prov_base.push_back(prov_base);
			
			//cout<<"prov_code:"<<prov_code<<" prov_base:"<<prov_base<<endl;

			memset(prov_code,0,sizeof(prov_code));		
		}

		for(int i = 0;i<v_prov_code.size()-1;i++)
		{	
			cur_base = v_prov_base[i]/total_base;
			cur_base *= 100;		
			sprintf(tmp_base,"%.*lf",percent.percent_pos,cur_base);
			
			//sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,v_prov_code[i],tmp_base);

			sscanf(tmp_base,"%lf",&cur_base);
			before_base += cur_base;
			//cout<<"省份:"<< v_prov_code[i]<<" 值:"<<v_prov_base[i]<<" 总值:"<<total_base<<"  占比:"<<tmp_base<<"::"<<cur_base<<endl;
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into %s(PROVINCE,PERCENT,PERBASE,PERORG,SETTLEMON,SOURCE_ID)values('%s',%s,%lf,%lf,'%s','%s')",percent.outtable_name,v_prov_code[i],tmp_base,total_base,v_prov_base[i],szSumMonth,Smonth.szSourceId);
			stmt.setSQLString(sql);
		
			stmt.execute();
		}
		
		cur_base = 100 - before_base;
		sprintf(tmp_base,"%.*lf",percent.percent_pos,cur_base);

		//sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,v_prov_code[v_prov_code.size()-1],tmp_base);

		sscanf(tmp_base,"%lf",&cur_base);
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into %s(PROVINCE,PERCENT,PERBASE,PERORG,SETTLEMON,SOURCE_ID)values('%s',%s,%lf,%lf,'%s','%s')",percent.outtable_name,v_prov_code[v_prov_code.size()-1],tmp_base,total_base,v_prov_base[v_prov_base.size()-1],szSumMonth,Smonth.szSourceId);
		stmt.setSQLString(sql);	
		stmt.execute();
	}

  }catch(util_1_0::db::SQLException e)
   {
		stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"shareCal() 数据库出错：%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
  }

	return ret ;
}


int CShare::run(int deal_type,bool del)
{
	int  ret = -1;
	
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	
	{
		theJSLog<<"当前数据库为备份态,无法进行摊分汇总"<<endw;
		return ;
	}
	
	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return -1 ;
	}

	if(deal_type == 1)
	{
		theJSLog<<"#################处理摊分汇总月份["<<szSumMonth<<"]的数据################"<<endi;
	}
	else if(deal_type == 2)
	{
		theJSLog<<"#################重新摊分汇总月份["<<szSumMonth<<"]的数据################"<<endi;
	}
	else
	{
		theJSLog<<"#################类型错误##################"<<deal_type<<endw;
		return -1;
	}
	
	for(int i = 0;i<iSourceCount;i++)
	{			
			theJSLog<<"###############处理数据源"<<pShareList[i].szSourceId<<"################"<<endi;

			if(deal_type == 1)
			{
				//每个数据源判断是否达到汇总条件
				ret = checkSettCondition(pShareList[i],szSumMonth);
				switch(ret)
				{
					case 0:						

					case 1:														
						ret = sum(deal_type,pShareList[i],szSumMonth,del);
						break;	
					case 2:	
						break;					//汇总条件不满足...
					case -1:					//数据库异常
						break;
					default:
						break;
				}

			}
			else if(deal_type == 2)
			{
				//ret = 1;
				ret = sum(deal_type,pShareList[i],szSumMonth,del);
			}		
	}
	conn.close();
	
	if(deal_type == 1)
	{
	    theJSLog<<"###########处理摊分汇总月份完成############"<<endi;
	}
	else if(deal_type == 2)
	{
		theJSLog<<"###########重新摊分汇总月份完成#############"<<endi;
	}

	return ret ;
}

/*
//容灾初始化
bool CShare::drInit()			//由于存在两个实例
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
int CShare::drVarGetSet(char* serialString)
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
			theJSLog<<"检查容灾切换锁失败,返回值="<<ret<<endw;
			return -1;  
		} 
		//初始化index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"初始化index失败,返回值=" <<ret<<endw;
			dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"传序列串时失败，序列名：["<<serialString<<"]"<<endw;
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
			theJSLog<<"传输实例名失败："<<tmpVar<<endw;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> 预提交index  此关键字用于将平台当前内存中的随机变量写入磁盘
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"预提交index失败"<<endw;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> 提交index  	提交Index。在index文件中设置完成标志
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"提交index失败,返回值="<<ret<<endw;
			dr_AbortIDX();  
			return -1;  
		}

		//备系统搜索目录
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<m_SerialString<<endi;

		theJSLog<<"本次的同步串serialString:"<<serialString<<endi;//for test

		return ret;

}

//仲裁字符串
 int CShare::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;

		if(!m_enable)
		{
			return ret;
		}

		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endw;
			dr_AbortIDX();
			//return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endw;
			dr_AbortIDX();
			//return false;
		}
		else if(4 == ret)
		{
			theJSLog<<"对端idx异常终止..."<<endw;
			dr_AbortIDX();
			//return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "业务全部提交失败(容灾平台)" << endw;
				dr_AbortIDX();
				//return false;
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
			//return true;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endw;
			dr_AbortIDX();
			//return false;
		}
	
	return ret;
 }

bool CShare::CheckTriggerFile()
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




