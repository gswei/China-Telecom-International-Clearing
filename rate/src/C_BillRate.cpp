/****************************************************************
  Project	
  Copyright (c)	2010-2011. All Rights Reserved.		广东亿迅科技有限公司 
  FUNCTION:	一次批价
  FILE:		C_BillRate.h
  AUTHOR:	liuw
  Create Time: 2010-05-10
  last update by licm 20101223
==================================================================
  Description:  
		结合综合计费结算3.0系统的新框架设计，批价模块用插件形式实现
  UpdateRecord: 
==================================================================

 *****************************************************************/
 
#include "C_BillRate.h"
using namespace std;
using namespace tpss;
extern SParameter Param;
 
BillRate::BillRate()
	{
		//
		getCurTime(szStartupTime);
		memset(szTxtType,0,sizeof(szTxtType));
	}
	
BillRate::~BillRate()
	{

	  if (Param.pRuleFieldIndex)
	    delete[] Param.pRuleFieldIndex;
	  if (Param.pRuleFieldIsNull)
	    delete[] Param.pRuleFieldIsNull;    
	  if (Param.pRuleFieldMatchMode)
	    delete[] Param.pRuleFieldMatchMode;        
//	  if (Param.rule_exact)
//	    delete Param.rule_exact;
	  if (Param.rule_blur)
	    delete Param.rule_blur;
		 
	  if (Param.p_formula)
	    delete Param.p_formula;	  
	
	  FreeRuleStruct(Param.szDebugFlag,&Param.rsblur, Param.iBlurCount);
	  FreeRateStruct(Param.szDebugFlag,&Param.ratestruct);
	  FreeAdjRateStruct(Param.szDebugFlag,&Param.adjratestruct);
	  //FreeFormula(Param.szDebugFlag,&Param.szformulastruct,Param.iFormulacount,Param.iParamcount);
	}

void BillRate::init(char *jobID, char *ratecycle)
{
   DBConnection conn;
   strcpy(Param.ratecycle,ratecycle );
   cout << "Param.ratecycle = " << Param.ratecycle <<endl;
   //BillRate billRate;
    dbConnect(conn);
    {
    	string sql;
    	char source_group_id[30];
    	char serviceid[2] = {0};
		Statement stmt = conn.createStatement();
		sql = "SELECT distinct(source_group_id) from C_SETTLE_JOB where  job_id = :v1";
		stmt.setSQLString(sql);
		stmt << jobID;
		stmt.execute();
		stmt>> source_group_id;

		sql = "SELECT distinct(rate_config_id) from C_SETTLE_JOB where  job_id = :v1";
		stmt.setSQLString(sql);
		stmt << jobID;
		stmt.execute();
		stmt>> Param.szRateConfigId;

		theJSLog << "Param.szRateConfigId = "<<Param.szRateConfigId <<endd;

		init(source_group_id,"SHARE",1);
		theJSLog << "Param.szRateConfigId 2 = "<<Param.szRateConfigId <<endd;
		//sprintf(Param.szRateConfigId,"%s",source_group_id);
	}
    conn.close();
}
void BillRate::init(char *szSourceGroupID,char * ServiceId,int index)
	{
		//获取数据源组配置变量情况
		theJSLog<<"Start init BillRate"<<endd;
		strcpy(Param.szSourceGroup , szSourceGroupID);
		strcpy(Param.ServiceID , ServiceId);
		//CBindSQL ds(DBConn);
		char szTemp[512];

	    if( strlen( Param.szRateConfigId) ==0)
		{
		    getEnvFromDB(  Param.ServiceID,Param.szSourceGroup,"", "RATE_CONFIG_ID", Param.szRateConfigId);
		    theJSLog<<"RATE_CONFIG_ID = "<<Param.szRateConfigId<<endd;
	    }
	
		getEnvFromDB( Param.ServiceID,Param.szSourceGroup,"", "RATE_USE_RULE_MATCH", szTemp);
		if (strcmp(szTemp, "Y") == 0)
		{
			Param.bUseRuleMatch = true;
			theJSLog<<"bUseRuleMatch=True;"<<endd;
		}
		else
		{
			Param.bUseRuleMatch = false;
			theJSLog<<"bUseRuleMatch=False;"<<endd;
		}
		
		//用于两次查找，对比分析结果。
		getEnvFromDB(  Param.ServiceID,Param.szSourceGroup,"", "RATE_USE_MULTI_FIND", szTemp);
		if (strcmp(szTemp, "Y") == 0)
		{
			Param.bUseMultiFind = true;
			theJSLog<<"bUseMultiFind=True;"<<endd;
		}
		else
		{
			Param.bUseMultiFind = false;
			theJSLog<<"bUseMultiFind=False;"<<endd;
		}	
  
      
		//读取批价基本配置
	DBConnection conn;
	 try{			 	   
	 	   
	 if (dbConnect(conn))
	 {	 string sql;
	 	  Statement stmt = conn.createStatement();	
			//string sql = "select RATING_CONFIG_ID, TABNAME_RULE, TABNAME_GROUPLEVEL, TABNAME_RATEGROUP, TABNAME_CHARGETYPE, TABNAME_RATELEVEL, \
			//TABNAME_RATE, TABNAME_ADJRATE,TABLE_FORMULA,TABLE_FORMULA_param,RULE_PIPE_NAME from c_rating_config_def where rating_config_id=:szRateConfigId";	
			char sqltmp[512];
			//20130709 添加公式表格
			sprintf(sqltmp,"select RATING_CONFIG_ID, TABNAME_RULE, TABNAME_DEFRULE,TABNAME_GROUPLEVEL, TABNAME_RATEGROUP, TABNAME_CHARGETYPE, TABNAME_RATELEVEL, \
			TABNAME_RATE, TABNAME_ADJRATE,TABLE_FORMULA, TABLE_FORMULA_param,RULE_PIPE_NAME from c_rating_config_def where rating_config_id='%s'",Param.szRateConfigId);
			stmt.setSQLString(sqltmp);
			//stmt << Param.szRateConfigId;			
			stmt.execute();
			//cout << sqltmp <<endl;
			stmt>>Param.rate_config.szConfigId>>Param.rate_config.szTablenameRule>>Param.rate_config.szTablenameDefRule>>Param.rate_config.szTablenameGroupLevel
    		>>Param.rate_config.szTablenameRateGroup>>Param.rate_config.szTablenameChargeType
    		>>Param.rate_config.szTablenameRateLevel
    		>>Param.rate_config.szTablenameRate>>Param.rate_config.szTablenameAdjrate
    		>> Param.rate_config.szTablenameFormula >> Param.rate_config.szTablenameFormulaParam
    		>>Param.rate_config.szRulePipeName;
	    /*{
	    	strcpy(szLogStr, "read rule field information from table rating_config_def error!");
				jsexcp::CException e(CALCFEE_ERR_IN_SELECT_DB, szLogStr, __FILE__, __LINE__);
				throw e;	    	
	    }*/
	    
	    //sql = "select count(*) from c_rating_field_def where rating_config_id=:szRateConfigId";
	    //char sqltmp[512];
	    long rulecout;
	    sprintf(sqltmp,"select count(*) from c_rating_field_def where rating_config_id='%s'",Param.szRateConfigId);
	    stmt.setSQLString(sqltmp);
			//stmt << Param.szRateConfigId;			
			stmt.execute();
			stmt >> Param.iRuleFieldCount;
			//cout << "rulecout = " << Param.iRuleFieldCount <<endl;
			cout << "Param.szRateConfigId = " << Param.szRateConfigId << endl;
			//cout << "sql = " << sqltmp << endl;
			if ((Param.iRuleFieldCount < 1) || (Param.iRuleFieldCount > RATE_MAX_RULEITEM_NUM))
	    {
	    	sprintf(szLogStr, "批价规则字段个数在 1..%d!之间", RATE_MAX_RULEITEM_NUM);
				jsexcp::CException e(CALCFEE_ERR_IN_SELECT_DB, szLogStr, __FILE__, __LINE__);
				throw e;	    	
	    }
	    theJSLog<<"iRuleFieldCount = "<<Param.iRuleFieldCount<<endd;

		Param.pRuleFieldIndex = new int[Param.iRuleFieldCount];
		if (!Param.pRuleFieldIndex)
    	{
      	strcpy(szLogStr, "New pRuleFieldIndex fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;	      	
    	}

		Param.pRuleFieldIsNull = new int[Param.iRuleFieldCount];
		if (!Param.pRuleFieldIsNull)
			{
				strcpy(szLogStr, "New pRuleFieldIsNull fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;	  
			}
		
		Param.pRuleFieldMatchMode = new int[Param.iRuleFieldCount];
		if (!Param.pRuleFieldMatchMode)
			{
				strcpy(szLogStr, "New pRuleFieldMatchMode fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;	  
			}

			
		//获取入口话单格式
		//cout << "szTxtType = " <<szTxtType<<endl;
		if(strlen(szTxtType) <=0) 
		{
		  sql = "select filetype_id from C_SOURCE_GROUP_DEFINE where Source_Group= :Source_Group";
          stmt.setSQLString(sql);
		  stmt << Param.szSourceGroup;	
		  stmt.execute();
          if (!(stmt>>Param.szInputFiletypeId))
          {
    		strcpy(szLogStr, "数据源组统一格式在C_SOURCE_GROUP_DEFINE未定义");
				jsexcp::CException e(CALCFEE_ERR_IN_SELECT_DB, szLogStr, __FILE__, __LINE__);
				throw e;
          }
		} else 
		{
		   //cout << "szTxtType = " <<szTxtType<<endl;
		   strcpy(Param.szInputFiletypeId, szTxtType);
		}
        //cout << "Param.szInputFiletypeId " <<Param.szInputFiletypeId<< endl;
        inrcd.Init(Param.szInputFiletypeId);
        outrcd.Init(Param.szInputFiletypeId);
        
        //cout << "end init filetype " << endl;
        
        
			
		//改成按字段名称取序号，而不是直接配置	
		sql = 	"select r.field_name,t.col_index,r.field_isnull,r.field_matchmode \
							from c_rating_field_def r,c_txtfile_fmt t\
							where r.rating_config_id=:szRateConfigId \
							and t.filetype_id=:szInputFiletypeId and t.colname=r.field_name order by r.field_order_no";
		stmt.setSQLString(sql);
		stmt << Param.szRateConfigId<<Param.szInputFiletypeId;
		cout << "Param.szRateConfigId = "<<Param.szRateConfigId<<endl;
		//cout << "Param.szInputFiletypeId = "<<Param.szInputFiletypeId<<endl;
		//cout << sql<<endl;
		cout << "Param.iRuleFieldCount = " <<Param.iRuleFieldCount<<endl;
		stmt.execute();	
		for (int i=0; i<Param.iRuleFieldCount; i++)
			{
				stmt>>Param.szRuleFieldItem[i]>>Param.pRuleFieldIndex[i]>>Param.pRuleFieldIsNull[i]>>Param.pRuleFieldMatchMode[i];
				/*
					{
						strcpy(szLogStr, "read rule field information from table rating_field_def error!");
						jsexcp::CException e(CALCFEE_ERR_IN_SELECT_DB, szLogStr, __FILE__, __LINE__);
						throw e;
					}
					*/
					//cout << "Param.szRuleFieldItem[i] = "<<Param.szRuleFieldItem[i]<<endl;
			}
		
		Param.szRuleFieldName[0] = 0x00;
		//cout << "Param.iRuleFieldCount = "<<Param.iRuleFieldCount<<endl;
		for (int i=0; i<Param.iRuleFieldCount; i++)
			{
				int iLen;
				strcat(Param.szRuleFieldName, Param.szRuleFieldItem[i]);
				if (i<Param.iRuleFieldCount -1)
					{
						iLen = strlen(Param.szRuleFieldName);
						Param.szRuleFieldName[iLen] = ',';
						Param.szRuleFieldName[iLen + 1] = 0x00;
					}
			}
		//cout << "Param.szRuleFieldName = "<<Param.szRuleFieldName<<endl;
		Param.szRuleFieldMatchMode[0] = 0x00;
		for (int i=0; i<Param.iRuleFieldCount; i++)
			{
				int iLen;
				sprintf(Param.szRuleFieldMatchMode, "%s%d", Param.szRuleFieldMatchMode, Param.pRuleFieldMatchMode[i]);
				if (i<Param.iRuleFieldCount -1)
					{
						iLen = strlen(Param.szRuleFieldMatchMode);
						Param.szRuleFieldMatchMode[iLen] = ',';
						Param.szRuleFieldMatchMode[iLen + 1] = 0x00;
					}
			}
		//*********************************************************************
    // 初始化精确规则及模糊规则类
    //*********************************************************************
    /*
    Param.rule_exact= new CRuleExact(Param.szRuleExactFile,Param.szRuleExactidx);
    if (!Param.rule_exact)
    	{
    		strcpy(szLogStr, "New rule_exact fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;
    	}
    */
    Param.rule_blur = new CRuleBlur(Param.iRuleFieldCount, Param.szRuleFieldMatchMode);
    if (!Param.rule_blur)
    	{
    		strcpy(szLogStr, "New rule_blur fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;
    	}
    
    Param.rate_group = new CRateGroup();
    if (!Param.rate_group)
    	{
    		strcpy(szLogStr, "New rate_group fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;
    	}
   
    Param.charge_type = new CChargeType();
    if (!Param.charge_type)
   	 	{
    		strcpy(szLogStr, "New charge_type fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;
    	}	

   //添加初始化公式类
    Param.p_formula = new Formula();
    if (!Param.p_formula)
   	 	{
    		strcpy(szLogStr, "New formulastruct fail.");
				jsexcp::CException e(CALCFEE_ERR_NOT_ENOUGH_MEMORY, szLogStr, __FILE__, __LINE__);
				throw e;
    	}
	//*********************************************************************
    // 获取“用于条件判断的日期时间字段”的序号
    //*********************************************************************  
    sql = "select col_index from c_txtfile_fmt where filetype_id= :filetype_id and col_fmt='C'";
    stmt.setSQLString(sql);
		stmt << Param.szInputFiletypeId;			
		stmt.execute();	
		cout << "Param.szInputFiletypeId = " << Param.szInputFiletypeId <<endl;
    if (!(stmt>>Param.iTimeIndex))
    {
    		strcpy(szLogStr, "select col_index from c_txtfile_fmt where filetype_id= :filetype_id and col_fmt='C' is NULL");
				jsexcp::CException e(CALCFEE_ERR_IN_SELECT_DB, szLogStr, __FILE__, __LINE__);
				throw e;
    }
    theJSLog<<"CDR Time Index = "<<Param.iTimeIndex<<endd;  
   
    //***********************************************************
    // 将pipe_id加入到rule_pipe表中
    //***********************************************************
    /*
    int iCount;
    char szRuleName[31];

    strcpy(szRuleName, Param.rate_config.szRulePipeName);
    sql= "select count(*) from c_rule_update where rule_name=:rule_name \
       and Source_Group=:Source_Group";
    stmt.setSQLString(sql);
		stmt << szRuleName<<Param.szSourceGroup;			
		stmt.execute();	
		stmt >>iCount;

    if (iCount == 0) {
    	sql = "insert into c_rule_update(Source_Group,rule_name,need_update) \
         values(:Source_Group,:rule_name,'N')";
       stmt.setSQLString(sql);
		   stmt << Param.szSourceGroup<<szRuleName;	
		   stmt.execute();	
		   stmt.commit(); 
    }
    */
			    //将模糊匹配RULE数据由数据表载入内存
		theJSLog<<"Loading table "<<Param.rate_config.szTablenameRule<<" into memory..."<<endd;
		//cout << "Param.szRuleFieldName 1= "<< Param.szRuleFieldName<<endl;
		if (InitRuleStruct(Param.szDebugFlag, &Param.rsblur, Param.rate_config.szTablenameRule,
			Param.szRuleFieldName, Param.iBlurCount, Param.iBlurMax, "") == 0)
			{
				theJSLog<<"Load table "<<Param.rate_config.szTablenameRule<<" into memory successfully!"<<endd;
			}
		else
			{
				sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameRule);
				jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
				throw e;			        			
			}

			    //将模糊匹配RULE数据由数据表载入内存,默认费率
		
		//cout << "Param.szRuleFieldName 1= "<< Param.szRuleFieldName<<endl;
		if(strlen(Param.rate_config.szTablenameDefRule)==0)
			Param.bUseDefRateRule = false;
		else{
			theJSLog<<"Use Default RateRule!"<<endd;
			Param.bUseDefRateRule = true;
		}
		
		if(Param.bUseDefRateRule){
			theJSLog<<"Loading table "<<Param.rate_config.szTablenameDefRule<<" into memory..."<<endd;
			if (InitDefRuleStruct(Param.szDebugFlag, &Param.defblur, Param.rate_config.szTablenameDefRule,
				Param.szRuleFieldName, Param.iDefBlurCount, Param.iDefBlurMax, "") == 0)
				{
					theJSLog<<"Load table "<<Param.rate_config.szTablenameDefRule<<" into memory successfully!"<<endd;
				}
			else
				{
					sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameDefRule);
					jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
					throw e;			        			
				}
		}

		
		//将精确匹配RULE数据由文件载入内存
		//加锁
		/*
		Param.rule_exact->memManager.filelock.Lock();
		
		//载入
		
		if ( Param.bUseRuleMatch == true && Param.rule_exact->memManager.pShm->MemVersion == 0)
			{
			  theJSLog<<"Loading rule_exact into memory..."<<endd;
			  try
			  	{
			  		 if((Param.rule_exact->memManager.ShmVarPos - Param.rule_exact->memManager.RealVarShm + Param.rule_exact->getIncreasedNo()+10000)*sizeof(SRuleVar)  > Param.rule_exact->memManager.pShm->iCurSize )
							{
								  Param.rule_exact->memManager.Rebuild(Param.rule_exact->getIncreasedNo()*sizeof(SRuleVar)/(50*1024*1024)+1);//重建共享内存
									Param.rule_exact->clearRule();
							}		     		  										
			  	  char szRuleFieldAll[CALCFEE_RULE_LEN_MAX];
			  	  char szExpirationDate[15];
			  	  
			  	  getCurTime(szExpirationDate);
			  	  time2TimeStr(timeStr2Time(szExpirationDate) - (Param.iRuleExpirationDate * 86400), szExpirationDate);
			  	  
			  	  strcpy(Param.rstmp.szRateRule , szRuleFieldAll);
			  	  Param.rule_exact->rewindFile();
			  	  while (Param.rule_exact->readFromFile(Param.rstmp) == true)
			  	  {
			  	    if ((CheckRuleExact(Param.szDebugFlag, Param.rsblur, Param.iBlurCount, Param.rstmp) == 0)
			  	      && (strcmp(Param.rstmp.szUpdateTime, szExpirationDate) >= 0))
			  	      if (Param.rule_exact->insertRule(Param.rstmp)==false)
			  	      {
			  	      	  theJSLog<<"insert rule fail!"<<endd;
			  	      }
			  	  }
			  	  Param.rule_exact->closeRuleFile();
			  	  Param.rule_exact->saveRule(Param.szRuleExactFileBak);
			  	  Param.rule_exact->clearIncreasedNo();
			  	  theJSLog<<"Load rule_exact into memory successfully!"<<endd;
			  	}
			  catch (jsexcp::CException e)
			  	{
						e.PushStack(CALCFEE_ERR_IN_LOAD_TABLE, "Load rule_exact into memory failed!", __FILE__,__LINE__);
								Param.rule_exact->memManager.filelock.UnLock();
						throw e;
			  	}
			  Param.rule_exact->memManager.pShm->MemVersion ++;
			  Param.MemVersion = Param.rule_exact->memManager.pShm->MemVersion;
				theJSLog<<"Param.MemVersion = "<<Param.MemVersion<<"\tpShm->MemVersion = "<<Param.rule_exact->memManager.pShm->MemVersion<<endd;			  
			}
		else if (Param.bUseRuleMatch == true && Param.rule_exact->memManager.pShm->MemVersion > 0)
			{
				theJSLog<<"Loading Share Memory rule_exact ..."<<endd;
				Param.rule_exact->loadRuleMap();
			  Param.rule_exact->clearIncreasedNo();
			  Param.rule_exact->iTotalNo = Param.rule_exact->memManager.pShm->iRuleCount; 				
				Param.MemVersion = Param.rule_exact->memManager.pShm->MemVersion;
				theJSLog<<"Load rule_exact into memory successfully!"<<endd;
				theJSLog<<"Param.MemVersion = "<<Param.MemVersion<<"\tpShm->MemVersion = "<<Param.rule_exact->memManager.pShm->MemVersion<<endd;
			}
		//解锁
		Param.rule_exact->memManager.filelock.UnLock();
		*/
		//将RATEGROUP数据由数据表载入内存
		theJSLog<<"Loading table "<<Param.rate_config.szTablenameRateGroup<<" into memory..."<<endd;
		if (Param.rate_group->init(Param.rate_config.szTablenameRateGroup, Param.rate_config.szTablenameRateLevel, Param.rate_config.szTablenameGroupLevel, Param.szDebugFlag) == true)
			{
			  theJSLog<<"Load table "<<Param.rate_config.szTablenameRateGroup<<" into memory successfully!"<<endd;
			}
		else
			{
			  sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameRateGroup);
				jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
				throw e;
			}
		
		//将CHARGETYPE数据由数据表载入内存
		theJSLog<<"Loading table "<<Param.rate_config.szTablenameChargeType<<" into memory..."<<endd;
		if (Param.charge_type->init(Param.rate_config.szTablenameChargeType, Param.szDebugFlag) == true)
			{
			  theJSLog<<"Load table "<<Param.rate_config.szTablenameChargeType<<" into memory successfully!"<<endd;
			}
		else
			{
			  sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameChargeType);
				jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
				throw e;
			}
		
		//将RATE数据由数据表载入内存
		theJSLog<<"Loading table "<<Param.rate_config.szTablenameRate<<" into memory..."<<endd;
		if (InitRateStruct(Param.szDebugFlag, &Param.ratestruct, Param.iRateCount, Param.rate_config.szTablenameRate) == 0)
			{
			  theJSLog<<"Load table "<<Param.rate_config.szTablenameRate<<" into memory successfully!"<<endd;
			}
		else
			{
		  	sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameRate);
				jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
				throw e;
			}
		
		//将FORMULA 数据由数据表载入内存
		theJSLog<<"Loading table "<<Param.rate_config.szTablenameFormula<<" into memory..."<<endd;
		if (InitFormulaStruct(Param.szDebugFlag, &Param.szformulastruct, Param.iFormulacount,Param.iParamcount, Param.rate_config.szTablenameFormula,Param.rate_config.szTablenameFormulaParam) == 0)
			{
			  theJSLog<<"Load table "<<Param.rate_config.szTablenameFormula<<" into memory successfully!"<<endd;
			}
		else
			{
		  	    sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameRate);
				jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
				throw e;
			}
		
		//将ADJRATE数据由数据表载入内存
		theJSLog<<"Loading table "<<Param.rate_config.szTablenameAdjrate<<" into memory..."<<endd;
		if (InitAdjRateStruct(Param.szDebugFlag, &Param.adjratestruct, Param.iAdjRateCount, Param.rate_config.szTablenameAdjrate) == 0)
			{
			  theJSLog<<"Load table "<<Param.rate_config.szTablenameAdjrate<<" into memory successfully!"<<endd;
			}
		else
			{
		  	sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameAdjrate);
				jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
				throw e;
			}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		////20130709 添加公式表格内容的获取
		
//		theJSLog<<"iExactCount 			= "<<Param.rule_exact->getTotalNo()	<<endd;
		theJSLog<<"iBlurCount  			= "<<Param.rule_blur->getCount()		<<endd;
		theJSLog<<"iRateGroupCount 	= "<<Param.rate_group->getCount()		<<endd;
		theJSLog<<"iChargeTypeCount =	"<<Param.charge_type->getCount()	<<endd;
		theJSLog<<"iRateCount				=	"<<Param.iRateCount								<<endd;
		theJSLog<<"iAdjRateCount		=	"<<Param.iAdjRateCount						<<endd;
	
			//Set update flag
		getCurTime(szCurDatetime);
		/*
		sql = "update c_rule_update set need_update='N', update_time=:szCurDatetime \
		   where Source_Group=:SourceGroup and rule_name=:rule_name";
		stmt.setSQLString(sql);
		stmt <<szCurDatetime<<Param.szSourceGroup<<Param.rate_config.szRulePipeName;	
		stmt.execute();	
		stmt.commit(); 
	*/
	 }else{
	 	cout<<"connect error."<<endl;
	 	//return false;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		theJSLog << "将精确匹配RULE数据由文件载入内存 出错" << endi;
		throw jsexcp::CException(0, "将精确匹配RULE数据由文件载入内存 出错", __FILE__, __LINE__);
		conn.close();
		//return false;
     } 
    ////////////////////////////////////////////////////////////// 
					
    /*ds.Open("update c_rule_update set need_update='Y' where Source_Group=:Source_Group and \
       rule_name=:rule_name",NONSELECT_DML);
    ds<<Param.szSourceGroup<<szRuleName;
    ds.Execute();
    ds.Close();
    DBConn.Commit();*/    
} 

	//针对结算摊分程序初始化获取部分数据
void BillRate::m_init(char * sourceid,STparam &szTparam,char* job_id)
{
	//strcpy(Param.szSourceGroup , szSourceGroupID);
	//strcpy(Param.ServiceID , ServiceId);
	DBConnection conn;//数据库连接
    try{			
	if (dbConnect(conn))
	 {
	       //获取数据源和路径
		Statement stmt = conn.createStatement();
		char sqltmp[512];
		sprintf(sqltmp,"select SETTLE_MONTH,SETTLE_INTYPE,SETTLE_OUTTYPE,SETTLE_INVALUE,"
			"SETTLE_OUTVALUE,SETTLE_INTXTTYPE,SETTLE_OUTTXTTYPE from C_SETTLE_CONFIG where job_id='%s'",job_id);
		stmt.setSQLString(sqltmp);		
		stmt.execute();		

		stmt>>szTparam.settle_month>>szTparam.intype
			>>szTparam.outtype>>szTparam.invalue>>szTparam.outvalue
			>>szTparam.intxt_type>>szTparam.outtxt_type;
		strcpy(szTxtType,szTparam.intxt_type); //将格式赋值

		theJSLog<<"SETTLE_CFGID = "<<szTparam.cfgid<<endd;
		theJSLog<<"SETTLE_MONTH = "<<szTparam.settle_month<<endd;
		theJSLog<<"SETTLE_INTYPE = "<<szTparam.intype<<endd;
		theJSLog<<"SETTLE_OUTTYPE = "<<szTparam.outtype<<endd;
		theJSLog<<"SETTLE_INVALUE = "<<szTparam.invalue<<endd;
		theJSLog<<"SETTLE_OUTVALUE = "<<szTparam.outvalue<<endd;
		theJSLog<<"SETTLE_INTXTTYPE = "<<szTparam.intxt_type<<endd;
		theJSLog<<"SETTLE_OUTTXTTYPE = "<<szTparam.outtxt_type<<endd;
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }
	    conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "获取数据源信息失败", __FILE__, __LINE__);
    }	 
}
	
void BillRate::execute(PacketParser& ps,ResParser& retValue)
	{
        //theJSLog<<"BillRate::execute() begin"<<endd;
		//CBindSQL ds(DBConn);
		//获取每次执行开始时间，备用
		getCurTime(szStartupTime);
		
		//申请临时变量备用，不放入私有变量，减少重置清零操作	
		int iRet=-9;
		char szTemp[10];
		char ratecycle[8];
		dealFlag = 0;
		
		//获取出入口话单		
		inrcd=ps.m_inRcd;
		outrcd=retValue.m_outRcd;
		
		try{
			if( strlen(Param.ratecycle) !=0 )
		       iRet = dealRecord(Param,ps,retValue,pAuditFile,pDispFiles,Param.ratecycle); 
		       //iRet =0;
			else
			{
			    inrcd.Get_Field("RateCycle" ,ratecycle);
			    iRet = dealRecord(Param,ps,retValue,pAuditFile,pDispFiles,ratecycle);
			   //iRet=0;
			}
			ps.clearAll();
            retValue.clearAll();
            ps.clear();
            retValue.clear();
		}
		catch(jsexcp::CException e)
		{
		    sprintf(szLogStr, "原始记录值为%s",ps.m_inRcd.Get_record());
		    theJSLog.writeLog(LOG_CODE_FORMULA_ERR,szLogStr);
			//strcpy(szLogStr, "批价处理未知错误");
			//e.PushStack(CALCFEE_ERR_UNKNOWN_CATCH, szLogStr, __FILE__, __LINE__);
			throw e;
		}

        switch (iRet)
        {
      	  case 0: //处理成功，提交事务
            dealFlag = 0;
			//theJSLog<<"BillRate::execute() successful!"<<endd;
            break;
            
          case -1: //批价规则缺资料
          	theJSLog<<"BillRate::execute() lackinfo!"<<endd;
            dealFlag = -1;
          	break;
          case -2://费率缺资料
          	theJSLog<<"BillRate::execute() lack tariff err!"<<endd;
             dealFlag = -2;
 						break;     
 		   case -3:
 		   	   dealFlag = -3;
 		   	   theJSLog<<"BillRate::execute() find rule err!"<<endd;
 		       break;
 		   case -5: //公式错误
          	theJSLog<<"BillRate::execute() formula err!"<<endd;
          	 dealFlag = -5;
          	 break;
         case -4: //计算错误
          	theJSLog<<"BillRate::execute() formula calculate err!"<<endd;
          	dealFlag = -4;
          	break;
          default: //其他一切不可预知的情况，应该不会发生
          	dealFlag = -5;
						strcpy(szLogStr, "批价处理未知错误");
						jsexcp::CException e(CALCFEE_ERR_UNKNOWN_CATCH, szLogStr, __FILE__, __LINE__);
					    throw e; 
            break;
        } //end switch
		           
   
	}
	
void BillRate::message(MessageParser&  pMessage)
	{
		//CBindSQL ds(DBConn);	
		DBConnection conn;//数据库连接	
		switch(pMessage.getMessageType())
		{
			/*批次开始*/
			case  MESSAGE_NEW_BATCH:
				/*
			      		//判断是否需要保存精确规则文件
			      		 if (Param.rule_exact->getIncreasedNo() > iRuleSavingInterval)
			      			{
			      			  theJSLog<<"Saving exact rule to file..."<<endd;

										for(int i=0; i<MAX_PROC_NUM; i++)
										{
											Param.rule_exact->memManager.pShm->m_Process[i].isLoadFlag = 1;//需要更新
										}
			      				Param.rule_exact->memManager.filelock.Lock();	  									
  									while(Param.rule_exact->memManager.IsUseMemory() == 1)
  									{
  										theJSLog<<"等待其他进程结束当前处理，10秒..."<<endd;
  										sleep(10);
  									}
  									
  									theJSLog<<"old memory size"<<Param.rule_exact->memManager.pShm->iCurSize<<endi;
								    theJSLog<<"old begin address"<<Param.rule_exact->memManager.RealVarShm<<endi;
								    theJSLog<<"old current address"<<Param.rule_exact->memManager.ShmVarPos<<endi;
								    theJSLog<<"unit num"<<Param.rule_exact->memManager.ShmVarPos-Param.rule_exact->memManager.RealVarShm<<endi;
								    theJSLog<<"irule count"<<Param.rule_exact->memManager.pShm->iRuleCount<<endi;
								    theJSLog<<"max count"<<Param.rule_exact->memManager.pShm->iMaxRule<<endi;
								    theJSLog<<"incoressno"<<Param.rule_exact->getIncreasedNo()<<endi;
								    theJSLog<<"varsize:"<<sizeof(SRuleVar)<<endi;
								    /////////////////
                    
								    if((Param.rule_exact->memManager.ShmVarPos - Param.rule_exact->memManager.RealVarShm + Param.rule_exact->getIncreasedNo()+10000)*sizeof(SRuleVar)  > Param.rule_exact->memManager.pShm->iCurSize )
								    	{
								    		 
								     		  theJSLog<<"apply for new memory"<<endi;
								     		  theJSLog<<"old memory size"<<Param.rule_exact->memManager.pShm->iCurSize<<endi;
								     		  theJSLog<<"old begin address"<<Param.rule_exact->memManager.RealVarShm<<endi;
								     		  theJSLog<<"old current address"<<Param.rule_exact->memManager.ShmVarPos<<endi;
								     		 
												  Param.rule_exact->memManager.Rebuild(Param.rule_exact->getIncreasedNo()*sizeof(SRuleVar)/(50*1024*1024)+1);//重建共享内存
												  Param.rule_exact->clearRule();
												  
												  theJSLog<<"old memory size"<<Param.rule_exact->memManager.pShm->iCurSize<<endi;
								     		  theJSLog<<"old begin address"<<Param.rule_exact->memManager.RealVarShm<<endi;
								     		  theJSLog<<"old current address"<<Param.rule_exact->memManager.ShmVarPos<<endi;
								     		  										
			      			  		try
			      			  			{
			      			  			  char szRuleFieldAll[CALCFEE_RULE_LEN_MAX];
			      			  			  char szExpirationDate[15];
			      			  			  
			      			  			  getCurTime(szExpirationDate);
			      			  			  time2TimeStr(timeStr2Time(szExpirationDate) - (Param.iRuleExpirationDate * 86400), szExpirationDate);
			      			  			  
			      			  			  strcpy(Param.rstmp.szRateRule , szRuleFieldAll);
			      			  			  Param.rule_exact->rewindFile();
			      			  			  while (Param.rule_exact->readFromFile(Param.rstmp) == true)
			      			  			  {
			      			  			    if ((CheckRuleExact(Param.szDebugFlag, Param.rsblur, Param.iBlurCount, Param.rstmp) == 0)
			      			  			      && (strcmp(Param.rstmp.szUpdateTime, szExpirationDate) >= 0))
			      			  			      if (Param.rule_exact->insertRule(Param.rstmp)==false)
			  	                      {
			  	      	                 theJSLog<<"insert rule fail!"<<endd;
			  	                      }
			      			  			     
			      			  			  }
			      			  			  Param.rule_exact->closeRuleFile();
			      			  			  Param.rule_exact->clearIncreasedNo();
			      			  			  theJSLog<<"ReLoad rule_exact into memory successfully!"<<endd;
			      			  			}
			      			  		catch (jsexcp::CException e)
			      			  			{
														e.PushStack(CALCFEE_ERR_IN_LOAD_TABLE, "Load rule_exact into memory failed!", __FILE__,__LINE__);
														Param.rule_exact->memManager.filelock.UnLock();
														throw e;
			      			  			}
			      			  		Param.rule_exact->loadTmpMap();
			      			 			Param.rule_exact->saveRule(Param.szRuleExactFileBak);
			      			 			Param.rule_exact->iTotalNo = Param.rule_exact->memManager.pShm->iRuleCount;
			      			  		theJSLog<<"Save exact rule completed,total exact rule count is "<<Param.rule_exact->getTotalNo()<<";"<<endd;
			      			  		Param.MemVersion = Param.rule_exact->memManager.pShm->MemVersion;
			      			  	}
			      			  else
			      			  	{
												Param.rule_exact->loadTmpMap();
												Param.rule_exact->iTotalNo = Param.rule_exact->memManager.pShm->iRuleCount;
												Param.MemVersion = ++Param.rule_exact->memManager.pShm->MemVersion;
			      			  		Param.rule_exact->saveRule(Param.szRuleExactFileBak);
			      			  		theJSLog<<"Save exact rule completed,total exact rule count is "<<Param.rule_exact->getTotalNo()<<";"<<endd;

			      			  	}
			      			  		Param.rule_exact->clearIncreasedNo();
			      			  		
										for(int i=0; i<MAX_PROC_NUM; i++)
										{
											Param.rule_exact->memManager.pShm->m_Process[i].isLoadFlag = 0;//更新完毕
										}			      			  		
			      			  Param.rule_exact->memManager.filelock.UnLock();
			      			  	
			      			}			
			      			*/	
				break;				
			/*批次结束，提交文件*/
			case  MESSAGE_END_BATCH_END_FILES:  
				        //Param.rule_exact->memManager.pShm->m_Process[Param.rule_exact->MemIndex].iSleepFlag = 0;
						//Param.rule_exact->memManager.Release();				
				break;
			/*批次结束，提交数据库数据*/
			case  MESSAGE_END_BATCH_END_DATA: 
				
				break;
			/*批次异常中断*/
			case  MESSAGE_BREAK_BATCH: 
				
				break;
			/* 文件开始 */
			case  MESSAGE_NEW_FILE:
				/*
DEALFILE:	if(Param.rule_exact->memManager.pShm->m_Process[Param.rule_exact->MemIndex].isLoadFlag == 0)
					{
						Param.rule_exact->memManager.filelock.Lock();
						Param.rule_exact->memManager.pShm->m_Process[Param.rule_exact->MemIndex].iSleepFlag = 1;
    				//***********************************************************
    				// 检查是否需要更新内存中的rule & rate数据
    				//***********************************************************
            try{		
            	DBConnection conn;//数据库连接	
          	if (dbConnect(conn))
          	 {
          			Statement stmt = conn.createStatement();
          			char szUpdateFlag[2];
          			string sql = "select need_update from c_rule_update where Source_Group=:SourceGroup \
			       	             and rule_name= :rule_name";
          			stmt.setSQLString(sql);
          			stmt << Param.szSourceGroup<<Param.rate_config.szRulePipeName;			
          			stmt.execute();
          			if (!(stmt>>szUpdateFlag))
			    		 {
			      		 strcpy(szLogStr, "C_RULE_UPDATE中对应RULE_NAME的数据不存在");
								 jsexcp::CException e(CALCFEE_ERR_IN_SELECT_DB, szLogStr, __FILE__, __LINE__);
								 throw e;
			    		 }
          			if (strcmp(szUpdateFlag, "Y") == 0 || Param.rule_exact->memManager.pShm->MemVersion > Param.MemVersion)
			    		{
			      		FreeRuleStruct(Param.szDebugFlag, &Param.rsblur, Param.iBlurCount);
			      		FreeRateStruct(Param.szDebugFlag, &Param.ratestruct);
			      		FreeAdjRateStruct(Param.szDebugFlag, &Param.adjratestruct);
			      		Param.rule_exact->clearRule();
			     			Param.rule_blur->clearRule();
			      		Param.rule_blur->travel();
			      		Param.rate_group->clear();
			      		Param.charge_type->clear();
			
			      		//将模糊匹配RULE数据由数据表载入内存
					      theJSLog<<"Loading table "<<Param.rate_config.szTablenameRule<<" into memory..."<<endd;
					      //cout << "Param.szRuleFieldName = "<< Param.szRuleFieldName<<endl;
			      		if (InitRuleStruct(Param.szDebugFlag, &Param.rsblur, Param.rate_config.szTablenameRule,
			        		Param.szRuleFieldName, Param.iBlurCount, Param.iBlurMax, "") == 0)
			      			{
					      		theJSLog<<"Load table "<<Param.rate_config.szTablenameRule<<" into memory successfully!"<<endd;
			      			}
			     		 	else
			      			{
			        			sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameRule);
										jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
										throw e;			        			
			      			}
								
								//将精确匹配RULE数据由文件载入内存
								if ( Param.bUseRuleMatch == true && Param.rule_exact->memManager.pShm->MemVersion == Param.MemVersion)
									{
										theJSLog<<"new file 1!"<<endd;
									  theJSLog<<"Loading rule_exact into memory..."<<endd;
									  if((Param.rule_exact->memManager.ShmVarPos - Param.rule_exact->memManager.RealVarShm + Param.rule_exact->getIncreasedNo()+10000)*sizeof(SRuleVar)  > Param.rule_exact->memManager.pShm->iCurSize )
								    {
								    		 
								     	 theJSLog<<"apply for new memory"<<endd;
								     	 theJSLog<<"old memory size"<<Param.rule_exact->memManager.pShm->iCurSize<<endd;
								     	 theJSLog<<"old begin address"<<Param.rule_exact->memManager.RealVarShm<<endd;
								     	 theJSLog<<"old current address"<<Param.rule_exact->memManager.ShmVarPos<<endd;
								     		 
											 Param.rule_exact->memManager.Rebuild(Param.rule_exact->getIncreasedNo()*sizeof(SRuleVar)/(50*1024*1024)+1);//重建共享内存
											 Param.rule_exact->clearRule();
												  
											 theJSLog<<"old memory size"<<Param.rule_exact->memManager.pShm->iCurSize<<endd;
								     	 theJSLog<<"old begin address"<<Param.rule_exact->memManager.RealVarShm<<endd;
								     	 theJSLog<<"old current address"<<Param.rule_exact->memManager.ShmVarPos<<endd;
								    }
								    
									  try
									  	{
									  	  char szRuleFieldAll[CALCFEE_RULE_LEN_MAX];
									  	  char szExpirationDate[15];
									  	  
									  	  getCurTime(szExpirationDate);
									  	  time2TimeStr(timeStr2Time(szExpirationDate) - (Param.iRuleExpirationDate * 86400), szExpirationDate);
									  	  
									  	  strcpy(Param.rstmp.szRateRule , szRuleFieldAll);
									  	  Param.rule_exact->rewindFile();
									  	  while (Param.rule_exact->readFromFile(Param.rstmp) == true)
									  	  {
									  	    if ((CheckRuleExact(Param.szDebugFlag, Param.rsblur, Param.iBlurCount, Param.rstmp) == 0)
									  	      && (strcmp(Param.rstmp.szUpdateTime, szExpirationDate) >= 0))
									  	      if (Param.rule_exact->insertRule(Param.rstmp)==false)
			  	                  {
			  	      	             theJSLog<<"insert rule fail!"<<endd;
			  	                  }
									  	      
									  	  }
									  	  Param.rule_exact->closeRuleFile();
									  	  Param.rule_exact->saveRule(Param.szRuleExactFileBak);
									  	  Param.rule_exact->clearIncreasedNo();
									  	  theJSLog<<"Load rule_exact into memory successfully!"<<endd;
									  	}
									  catch (jsexcp::CException e)
									  	{
												e.PushStack(CALCFEE_ERR_IN_LOAD_TABLE, "Load rule_exact into memory failed!", __FILE__,__LINE__);
												Param.rule_exact->memManager.filelock.UnLock();
												throw e;
									  	}
									  Param.rule_exact->iTotalNo = Param.rule_exact->memManager.pShm->iRuleCount;
									  Param.rule_exact->RuleMapTemp.clear();//丢弃临时收集规则
									  Param.rule_exact->clearIncreasedNo();
			      			 	Param.rule_exact->saveRule(Param.szRuleExactFileBak);
									  Param.rule_exact->memManager.pShm->MemVersion ++;
									  Param.MemVersion= Param.rule_exact->memManager.pShm->MemVersion;
								}
								else if (Param.bUseRuleMatch == true && Param.rule_exact->memManager.pShm->MemVersion > Param.MemVersion)
									{
										theJSLog<<"new file 2!"<<endd;
										Param.rule_exact->loadRuleMap();
										Param.rule_exact->iTotalNo = Param.rule_exact->memManager.pShm->iRuleCount;
										Param.rule_exact->RuleMapTemp.clear();//丢弃临时收集规则
										Param.rule_exact->clearIncreasedNo();
										Param.MemVersion = Param.rule_exact->memManager.pShm->MemVersion;
									}

			      		
			      		//将RATEGROUP数据由数据表载入内存
			      		theJSLog<<"Loading table "<<Param.rate_config.szTablenameRateGroup<<" into memory..."<<endd;
			      		if (Param.rate_group->init(Param.rate_config.szTablenameRateGroup, Param.rate_config.szTablenameRateLevel, Param.rate_config.szTablenameGroupLevel, Param.szDebugFlag) == true)
			      			{
			      			  theJSLog<<"Load table "<<Param.rate_config.szTablenameRateGroup<<" into memory successfully!"<<endd;
			      			}
			      		else
			      			{
			      			  sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameRateGroup);
										jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
										throw e;
			      			}
			      		
			      		//将CHARGETYPE数据由数据表载入内存
			      		theJSLog<<"Loading table "<<Param.rate_config.szTablenameChargeType<<" into memory..."<<endd;
			      		if (Param.charge_type->init(Param.rate_config.szTablenameChargeType, Param.szDebugFlag) == true)
			      			{
			      			  theJSLog<<"Load table "<<Param.rate_config.szTablenameChargeType<<" into memory successfully!"<<endd;
			      			}
			      		else
			      			{
			      			  sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameChargeType);
										jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
										throw e;
			      			}
			      		
			      		//将RATE数据由数据表载入内存
			      		theJSLog<<"Loading table "<<Param.rate_config.szTablenameRate<<" into memory..."<<endd;
			      		if (InitRateStruct(Param.szDebugFlag, &Param.ratestruct, Param.iRateCount, Param.rate_config.szTablenameRate) == 0)
			      			{
			      			  theJSLog<<"Load table "<<Param.rate_config.szTablenameRate<<" into memory successfully!"<<endd;
			      			}
			      		else
			      			{
			      		  	sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameRate);
										jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
										throw e;
			      			}
			      		
			      		//将ADJRATE数据由数据表载入内存
			      		theJSLog<<"Loading table "<<Param.rate_config.szTablenameAdjrate<<" into memory..."<<endd;
			      		if (InitAdjRateStruct(Param.szDebugFlag, &Param.adjratestruct, Param.iAdjRateCount, Param.rate_config.szTablenameAdjrate) == 0)
			      			{
			      			  theJSLog<<"Load table "<<Param.rate_config.szTablenameAdjrate<<" into memory successfully!"<<endd;
			      			}
			      		else
			      			{
			      		  	sprintf(szLogStr, "Load table %s into memory failed!", Param.rate_config.szTablenameAdjrate);
										jsexcp::CException e(CALCFEE_ERR_IN_LOAD_TABLE, szLogStr, __FILE__, __LINE__);
										throw e;
			      			}
			      		
			      		theJSLog<<"iExactCount 			= "<<Param.rule_exact->getTotalNo()	<<endd;
			      		theJSLog<<"iBlurCount  			= "<<Param.rule_blur->getCount()		<<endd;
			      		theJSLog<<"iRateGroupCount 	= "<<Param.rate_group->getCount()		<<endd;
			      		theJSLog<<"iChargeTypeCount =	"<<Param.charge_type->getCount()	<<endd;
			      		theJSLog<<"iRateCount				=	"<<Param.iRateCount								<<endd;
			      		theJSLog<<"iAdjRateCount		=	"<<Param.iAdjRateCount						<<endd;
			      		
			      		//Set update flag
			      		getCurTime(szCurDatetime);
			      		sql = "update c_rule_update set need_update='N', update_time=:szCurDatetime \
			      		   where Source_Group=:SourceGroup and rule_name=:rule_name";
			      		stmt.setSQLString(sql);
		            stmt <<szCurDatetime<<Param.szSourceGroup<<Param.rate_config.szRulePipeName;	
		            stmt.execute();	
		            conn.commit(); 
			      	}						
						Param.rule_exact->memManager.filelock.UnLock();
						if(Param.MemVersion < Param.rule_exact->memManager.pShm->MemVersion)
							{
								theJSLog<<"ReLoad RuleMap from the Updated Share memory..."<<endd;
								Param.rule_exact->loadRuleMap();
								Param.MemVersion = Param.rule_exact->memManager.pShm->MemVersion;
							}
						
						
						strcpy(Param.szSourceId, pMessage.getSourceId());
						strcpy(Param.szFileName, pMessage.getFileName());
						
						sql = "select b.source_path from i_source_define b where b.source_id=:source_id";
			      stmt.setSQLString(sql);
		        stmt <<Param.szSourceId;	
		        stmt.execute();	
			   		if (!(stmt>>Param.szSourcePath))
			    		{
			      		strcpy(szLogStr, "i_source_define中对应source_id的数据不存在");
								jsexcp::CException e(CALCFEE_ERR_IN_SELECT_DB, szLogStr, __FILE__, __LINE__);
								throw e;
			    		}
			    	conn.commit();
 	
          	 }else{
          	 	cout<<"connect error."<<endl;
          	 	//return false;
          	 }
          	 conn.close();
          	 } catch( SQLException e ) {
          		cout<<e.what()<<endl;
          		theJSLog << "C_RULE_UPDATE中对应RULE_NAME的数据不存在" << endi;
          		throw jsexcp::CException(0, "C_RULE_UPDATE中对应RULE_NAME的数据不存在", __FILE__, __LINE__);
          		conn.close();
          		//return false;
            }              				      

			    }
			    
			else{
			    	theJSLog<<"等待其他进程更新共享内存区域...."<<endd;
			    	sleep(10);			    	
			    	goto DEALFILE;
			    	
			    }			
			    */	
						break;
			/* 文件结束 */
			case  MESSAGE_END_FILE:
					//	Param.rule_exact->memManager.pShm->m_Process[Param.rule_exact->MemIndex].iSleepFlag = 0;
						//Param.rule_exact->memManager.Release();
						break;
			default:
				break;
		}
		
	}
     
