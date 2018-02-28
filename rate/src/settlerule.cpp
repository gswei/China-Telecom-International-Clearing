#include "settlerule.h"
using namespace std;
using namespace tpss;
SParameter Param;
//DBConnection conn;//数据库连接

int InitFormulaStruct(char *debug_flag, FormulaStruct **formulastruct, int &formulacount, 
int &paramcount,char *tablename,char *paramtablename)
{
  int i, icount,ecount,pcount;
  char sqltmp[512];

  //CBindSQL ds(DBConn);
  DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(sqltmp, "select count(distinct(formula_id)) from %s", tablename);  //获取公式ID 总数	
			stmt.setSQLString(sqltmp);	
			stmt.execute();
			stmt >> icount;
	if (icount <= 0)
 	   {
       formulacount = 0;
       return 0;
      }
	  formulacount = icount;

	*formulastruct=new FormulaStruct[formulacount];
    	if (!formulastruct) {
    	  theJSLog<<"new ratestruct fail."<<endd;
        return(-1);
      }

     sprintf(sqltmp, "select distinct FORMULA_ID  from %s order by formula_id", tablename);
     stmt.setSQLString(sqltmp);	
 	 stmt.execute();
 	 string tmp_formula;
	 vector<string> formulas; 
	 while(stmt>>tmp_formula)
	 {
	    formulas.push_back(tmp_formula);
	 }
  //***********************************************************
  // Repeat reading all record
  //***********************************************************
    vector<string>::iterator it;
    int i=0;
    for(it=formulas.begin();it!=formulas.end(),i<icount;it++,i++){
    	//cout << *it<<endl;
    //for (i=0;i<icount;i++) {    
         //stmt >> (*formulastruct)[i].szFormulaID;   //获取公式ID
         strcpy((*formulastruct)[i].szFormulaID, (*it).c_str() );
         //(*formulastruct)[i].szFormulaID=*it;
         //cout << "(*formulastruct)[i].szFormulaID = " << (*formulastruct)[i].szFormulaID<<endl;

         // 1、获取公式表达式
         sprintf(sqltmp, "select count(*) from %s where formula_id = '%s' ", tablename,(*formulastruct)[i].szFormulaID);
         stmt.setSQLString(sqltmp);
         //cout << "sqltmp = " <<sqltmp<<endl;
	     stmt.execute();
	     stmt>>ecount; //某公式对应的多个表达式数量
	     ((*formulastruct)[i].szFormulaExp)=new FormulaExp[ecount];
	     (*formulastruct)[i].expno = ecount;  //将公式ID 对应的公式数量加载进来
         sprintf(sqltmp, "select INDEX_NUM ,SEGMENT_NAME ,FORMULA ,START_TIME,END_TIME,output_type,decimal_num from %s where formula_id = '%s' order by index_num", tablename,(*formulastruct)[i].szFormulaID);
         stmt.setSQLString(sqltmp);
         //cout << "ecount = " <<ecount << "  sqltmp = " <<sqltmp<<endl;
	     stmt.execute();
         for (int j=0;j<ecount;j++) {
    	     stmt >> ((*formulastruct)[i].szFormulaExp)[j].index_id
    	         >> ((*formulastruct)[i].szFormulaExp)[j].szSegmentName >> ((*formulastruct)[i].szFormulaExp)[j].szFormula
    	         >> ((*formulastruct)[i].szFormulaExp)[j].szStartTime >> ((*formulastruct)[i].szFormulaExp)[j].szEndTime
    	         >> ((*formulastruct)[i].szFormulaExp)[j].output_type >> ((*formulastruct)[i].szFormulaExp)[j].decimal_num;
      	}

        // 2、获取公式参数内容
         sprintf(sqltmp, "select count(*) from %s where formula_id = '%s' ", paramtablename,(*formulastruct)[i].szFormulaID);
         stmt.setSQLString(sqltmp);	
	     stmt.execute();
	     stmt>>pcount; //某公式对应的多个表达式数量
	     ((*formulastruct)[i].szFormulaParam)=new FormulaParam[pcount];
         (*formulastruct)[i].paramno = pcount;  //将公式ID 对应的参数数量加载进来
	    if (pcount <= 0)
 	   {
         paramcount = 0;
         return 0;
       }
       
	   sprintf(sqltmp, "select PARAM_NAME,PARAM_TYPE,PARAM_VALUE from %s where FORMULA_ID = '%s' " ,paramtablename,(*formulastruct)[i].szFormulaID);
       stmt.setSQLString(sqltmp);	
	   stmt.execute();
       //cout << "sqltmp = " <<sqltmp<<endl;
	   for(int k=0;k<pcount;k++)
	   	{
	   	  stmt >>((*formulastruct)[i].szFormulaParam)[k].szParamName
	   	  	  >>((*formulastruct)[i].szFormulaParam)[k].szParamType
	   	  	  >>((*formulastruct)[i].szFormulaParam)[k].szParamValue;

	   	}
      } //end for  
		
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  return -1;
	 }
	    conn.close();
	 } 
	 catch( SQLException e ) 
	 {
		cout<<e.what()<<endl;
		theJSLog << "InitFormulaStruct出错" << endi;
		throw jsexcp::CException(0, "InitFormulaStruct出错", __FILE__, __LINE__);
		conn.close();
		return -1;
   }
  //formulacount=icount;
  return 0;
}

int InitRateStruct(char *debug_flag, RATESTRUCT **ratestruct, int &ratecount, char *tablename)
{
  int i, icount;
  char sqltmp[512];

  //CBindSQL ds(DBConn);
  DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(sqltmp, "select count(*) from %s", tablename);	
			stmt.setSQLString(sqltmp);	
			stmt.execute();
			stmt >> icount;
			
			if (icount <= 0)
 	   {
       ratecount = 0;
       return 0;
      }

      *ratestruct=new RATESTRUCT[icount];
      if (!ratestruct) {
    	  theJSLog<<"new ratestruct fail."<<endd;
        return(-1);
      }
      sprintf(sqltmp, "select TARIFF_ID, START_TIME, END_TIME, CDRSTART_DATE, CDREND_DATE, CDRSTART_TIME, CDREND_TIME,\
    START_WEEKLY, END_WEEKLY, PRI_NO, RATE_A, RATE_B,  \
    RATE_ADD_A, RATE_ADD_B, METER_COUNT, CHARGE_UNIT \
    from %s ORDER BY TARIFF_ID, PRI_NO, START_TIME", tablename);
     stmt.setSQLString(sqltmp);	
		 stmt.execute();
  //***********************************************************
  // Repeat reading all record
  //***********************************************************
    for (i=0;i<icount;i++) {
    stmt>>(*ratestruct)[i].tariff_id>>(*ratestruct)[i].start_time>>
      (*ratestruct)[i].end_time>>(*ratestruct)[i].cdrstart_date>>
      (*ratestruct)[i].cdrend_date>>(*ratestruct)[i].cdrstart_time>>
      (*ratestruct)[i].cdrend_time>>(*ratestruct)[i].start_weekly>>
      (*ratestruct)[i].end_weekly>>(*ratestruct)[i].pri_no>>
      (*ratestruct)[i].rate_a>>
      (*ratestruct)[i].rate_b>>(*ratestruct)[i].rate_add_a>>
      (*ratestruct)[i].rate_add_b>>(*ratestruct)[i].meter_count>>
      (*ratestruct)[i].charge_unit;
      } //end for  
		
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  return -1;
	 }
	    conn.close();
	 } 
	 catch( SQLException e ) 
	 {
		cout<<e.what()<<endl;
		theJSLog << "InitRateStruct出错" << endi;
		throw jsexcp::CException(0, "InitRateStruct出错", __FILE__, __LINE__);
		conn.close();
		return -1;
   }
 
  ratecount=icount;
  return 0;
}

int FreeRateStruct(char *debug_flag, RATESTRUCT **ratestruct)
{
  if (*ratestruct) {
    delete[] (*ratestruct);
    (*ratestruct) = NULL;
  }
  return 0;
}

int FreeFormula(char *debug_flag, FormulaStruct **forstruct,int &formulano,int &paramno)
{
  if (*forstruct) {
    delete[] (*forstruct);
    (*forstruct) = NULL;
  }

   for (int i=0;i<formulano;i++)
   {
      if ( (*forstruct)[i].szFormulaExp ) {
        delete[] (*forstruct)[i].szFormulaExp;
        (*forstruct)[i].szFormulaExp = NULL;
      }
   }

   for (int j=0;j<paramno;j++)
   {
      if ((*forstruct)[j].szFormulaParam) {
        delete[] (*forstruct)[j].szFormulaParam;
        (*forstruct)[j].szFormulaParam = NULL;
      }
   }
  return 0;
}

int SearchRate(char *debug_flag, RATESTRUCT* ratestruct, int ratecount,
  RATESTRUCT &srate)
{
  int pstart=0;
  int pend = ratecount-1;
  int pmiddle = (pend+pstart)/2;
  int plast = 0;
  int ifind = 0;

  int islarger = 0;
  int inrange=-2;
  int isequal = 0;
  char oper_time[15];

  RATESTRUCT* cur_rate;

  //  theJSLog<<"Begin SearchRate(), ratecount = "<<ratecount<<" ;"endd;
  cur_rate = ratestruct;

  sprintf(oper_time, "%s", srate.start_time);
  while (pstart <= pend) {
    if ((isequal==0)&&(pstart == pend))
      isequal = 1;
      else if (isequal==1)
        break;
    pmiddle = (pend+pstart)/2;
    if (plast<pmiddle) {
      cur_rate=cur_rate+(pmiddle-plast);
    }
      else if (plast>pmiddle) {
        cur_rate=cur_rate-(plast-pmiddle);
      }

    islarger = CompareRate(debug_flag, cur_rate, srate);
    if (islarger>0) {
      //forward
      pstart = pmiddle+1;
      plast  = pmiddle;
    }
      else if (islarger<0) {
        //backward
        pend = pmiddle-1;
        plast  = pmiddle;
      }
        else if (islarger==0) 
        {
        	//移动到最后一个                          
        	while (ratecount-(pmiddle+1)>0)                          
        	{                          
        		cur_rate++;                          
        		pmiddle++;                          
        		if (CompareRate(debug_flag, cur_rate, srate) != 0)                          
        		{                          
        			cur_rate--;                          
        			pmiddle--;                          
        			break;                          
        		}                          
        	}                          
        	//倒着一个一个查找                          
        	do                          
        	{                          
        		if (CompareRateTime(debug_flag, cur_rate, oper_time) == 0)                      
        		{                          
        			ifind = 1;                          
        			break;                          
        		}                          
        		if (pmiddle-1<0)                          
        		  break;                          
       			cur_rate--;                          
      			pmiddle--;                          
        	} while (CompareRate(debug_flag, cur_rate, srate) == 0);                          

        } //end else
        if (islarger==0)
           break;
    }  //end while

  if (ifind==1) {
    //write result
    sprintf(srate.start_time, "%s", cur_rate->start_time);
    sprintf(srate.end_time, "%s", cur_rate->end_time);
    sprintf(srate.cdrstart_date, "%s", cur_rate->cdrstart_date);
    sprintf(srate.cdrend_date, "%s", cur_rate->cdrend_date);
    sprintf(srate.cdrstart_time, "%s", cur_rate->cdrstart_time);
    sprintf(srate.cdrend_time, "%s", cur_rate->cdrend_time);
    srate.start_weekly=cur_rate->start_weekly;
    srate.end_weekly=cur_rate->end_weekly;
    srate.pri_no=cur_rate->pri_no;
    srate.rate_a=cur_rate->rate_a;
    srate.rate_b=cur_rate->rate_b;
    srate.rate_add_a=cur_rate->rate_add_a;
    srate.rate_add_b=cur_rate->rate_add_b;
    srate.meter_count=cur_rate->meter_count;
    srate.charge_unit=cur_rate->charge_unit;
    /*
    theJSLog<<"tariff_id  = "<<srate.tariff_id<<endd;
    theJSLog<<"start_time = "<<srate.start_time<<endd;
    theJSLog<<"end_time		= "<<srate.end_time<<endd;
    theJSLog<<"callingfee	= "<<srate.calling_fee<<endd;
    theJSLog<<"calledfee	= "<<srate.called_fee<<endd;
    */
    //theJSLog<<"End SearchRate(), return=0;"<<endd;
    return 0;
  }
    else {
      //theJSLog<<"End SearchRate(), return=-1;"<<endd;
      return -1;
    }
}

int CompareRate(char *debug_flag, RATESTRUCT* cur_rate, RATESTRUCT &srate)
{
  int ret;
  //expTrace(debug_flag, __FILE__, __LINE__,
  //  "CompareRate(), ratetext=%s;truetext=%s;",
  //  srate.tariff_id, cur_rate->tariff_id);
  ret = strcmp(srate.tariff_id, cur_rate->tariff_id);

  return ret;
}

int CompareRateTime(char *debug_flag, RATESTRUCT* cur_rate, char *truetime)
{
  int ret;
  char szDateTime[15], szDate[9], szTime[7];
  bool bInWeekly;

  strcpy(szDateTime, truetime);
  strncpy(szDate, szDateTime, 8);
  szDate[8] = 0;
  if(strlen(szDateTime)==8)
  	{
  	  strcpy(szTime, "000000");
  	}
  else{
     strncpy(szTime, &szDateTime[8], 6);
     szTime[6] = 0;
  	}

  if ((cur_rate->start_weekly == 1) && (cur_rate->end_weekly == 7))
  {
    bInWeekly = true;
  }
  else
  {
  	int iWeekly;
    iWeekly = timeGetWeek(szDateTime);
    if ((iWeekly >= cur_rate->start_weekly) && (iWeekly <= cur_rate->end_weekly))
      bInWeekly = true;
    else
      bInWeekly = false;
  }

  if ((CompareTime(debug_flag,  cur_rate->start_time,  cur_rate->end_time,  szDateTime ) == 0) &&
     (CompareTime(debug_flag,  cur_rate->cdrstart_date,  cur_rate->cdrend_date,  szDate ) == 0) &&
     (CompareTime(debug_flag,  cur_rate->cdrstart_time,  cur_rate->cdrend_time,  szTime ) == 0) &&
     (bInWeekly == true))
  {
    ret = 0; //时间完全匹配
  }
  else
  {
  	  ret = -1; //只要有任何一个时间不匹配，即返回-1
  }

  return ret;
}

int CompareRateTime(char *debug_flag, RATESTRUCT* cur_rate, char *truetime, char *donetime)
{
  int ret;
  char szDateTime[15], szDate[9], szTime[7];
  bool bInWeekly;

  if (strcmp(truetime,donetime) == 0)
    return CompareRateTime(debug_flag, cur_rate, truetime);

  strcpy(szDateTime, truetime);
  strncpy(szDate, szDateTime, 8);
  szDate[8] = 0;
  strncpy(szTime, &szDateTime[8], 6);
  szTime[6] = 0;

  if (strncmp(truetime,donetime,8) == 0)  //不跨天
  {
    bInWeekly = true;
  }
  else
  {
  	int iWeekly;
    iWeekly = timeGetWeek(szDateTime);
    if ((iWeekly >= cur_rate->start_weekly) && (iWeekly <= cur_rate->end_weekly))
      bInWeekly = true;
    else
      bInWeekly = false;
  }

  if ((CompareTime(debug_flag,  cur_rate->start_time,  cur_rate->end_time,  szDateTime ) == 0) &&
     (CompareTime(debug_flag,  cur_rate->cdrstart_date,  cur_rate->cdrend_date,  szDate ) == 0) &&
     (CompareTime(debug_flag,  cur_rate->cdrstart_time,  cur_rate->cdrend_time,  szTime ) == 0) &&
     (bInWeekly == true))
  {
    ret = 0; //时间完全匹配
  }
  else
  {
  	  ret = -1; //只要有任何一个时间不匹配，即返回-1
  }

  return ret;
}

int InitRuleStruct(char *debug_flag, SRuleStruct **rulestruct, char *tablename,
  char *fieldname, int &rulecount, int &maxcount, char *source_group)
{
  int i, j, icount, imax;
  char sqltmp[512];

  //CBindSQL ds(DBConn);
  DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(sqltmp, "select count(*) from %s", tablename);
			//cout << "source_group = "<<source_group <<endl;
      if (strcmp(source_group, "")!=0) 
      {
        strcat(sqltmp, " where source_group=:source_group");
        stmt.setSQLString(sqltmp);
        stmt<<source_group;	        
			  stmt.execute();
      }
      else 
      {
        stmt.setSQLString(sqltmp);
       // cout << sqltmp <<endl;
			  stmt.execute();
      }   
      stmt>>icount;
      //cout << "icount = "<<icount <<endl;
      if (icount <= 0)
     	{
        rulecount = 0;
        maxcount = 0;
        return 0;
      }
    
      if (strcmp(source_group, "")!=0)
        imax=(icount/RULE_BENCHMARK+1)*RULE_BENCHMARK;
      else
        imax=icount;
      *rulestruct=new SRuleStruct[imax];
      if (!rulestruct) 
      {
        //theJSLog<<"new rulestruct fail."<<ende;
        theJSLog.writeLog(LOG_CODE_NEWRULE_ERR,"new rulestruct fail.");
        return(-1);
      }
    
      sprintf(sqltmp, "select ruleno, %s, \
        start_time, rategroup_id, end_time from %s order by ruleno, start_time",
        fieldname, tablename);
       // cout << "fieldname "<< fieldname<<endl;
        //cout << "tablename "<< tablename<<endl;
        //cout << "imax "<< imax<<endl;
      if (strcmp(source_group, "")!=0)
      {
        strcat(sqltmp, " where source_group=:source_group");
        stmt.setSQLString(sqltmp);
        stmt<<source_group;	
       
		stmt.execute();
      }
      else
      {
        stmt.setSQLString(sqltmp);	
       // cout << sqltmp <<endl;
		stmt.execute();
      }
      //***********************************************************
  // Repeat reading all record
  //***********************************************************
  for (i=0; i<icount; i++)
  {
    char szRuleField[RATE_MAX_RULEITEM_LEN];
    char szRuleFieldAll[CALCFEE_RULE_LEN_MAX];

    stmt>>(*rulestruct)[i].iRuleNo;
    
    szRuleFieldAll[0] = 0x00;
    for (j=0; j<Param.iRuleFieldCount; j++)
    {
      int iLen;
      stmt>>szRuleField;
      delSpace(szRuleField, 0);
      strcat(szRuleFieldAll, szRuleField);
      iLen = strlen(szRuleFieldAll);
      szRuleFieldAll[iLen] = RATE_RULE_SEPERATOR;
      szRuleFieldAll[iLen + 1] = 0x00;
    }

	
    stmt>>(*rulestruct)[i].szStartTime>>(*rulestruct)[i].szRateGroupId>>
      (*rulestruct)[i].szEndTime;

    int iFieldLen;
    iFieldLen = strlen(szRuleFieldAll);
    szRuleFieldAll[iFieldLen - 1] = 0x00;		//去掉最后面的分隔符
    //(*rulestruct)[i].szRateRule = new char[iFieldLen];
    /*if (!(*rulestruct)[i].szRateRule)
    {
      theJSLog<<"new rulestruct field fail."<<endd;
      return(-1);
    }*/
    strcpy((*rulestruct)[i].szRateRule, szRuleFieldAll);
    //cout<<(*rulestruct)[i].szRateRule<<endl;
	
    strcpy((*rulestruct)[i].szRuleFieldMatchMode, "1");

    //cout<<(*rulestruct)[i].szRuleFieldMatchMode<<endl;
    if (Param.rule_blur->addRule((*rulestruct)[i]) == false)
    {
      theJSLog<<"insert into ruleblur fail."<<endd;
      return(-1);    	
    }
  } //end for
  conn.commit();  
  		
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return false;
	 }
	 conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "InitRuleStruct 出错" << endi;
  		throw jsexcp::CException(0, "InitRuleStruct 出错", __FILE__, __LINE__);
  		conn.close();
  		return false;
     }        

  rulecount=icount;
  maxcount=imax;
  return 0;
}

int InitDefRuleStruct(char *debug_flag, SRuleStruct **rulestruct, char *tablename,
  char *fieldname, int &rulecount, int &maxcount, char *source_group)
{
  int i, j, icount, imax;
  char sqltmp[512];

  //CBindSQL ds(DBConn);
  DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(sqltmp, "select count(*) from %s", tablename);
			//cout << "source_group = "<<source_group <<endl;
      if (strcmp(source_group, "")!=0) 
      {
        strcat(sqltmp, " where source_group=:source_group");
        stmt.setSQLString(sqltmp);
        stmt<<source_group;	        
			  stmt.execute();
      }
      else 
      {
        stmt.setSQLString(sqltmp);
       // cout << sqltmp <<endl;
			  stmt.execute();
      }   
      stmt>>icount;
      //cout << "icount = "<<icount <<endl;
      if (icount <= 0)
     	{
        rulecount = 0;
        maxcount = 0;
        return 0;
      }
    
      if (strcmp(source_group, "")!=0)
        imax=(icount/RULE_BENCHMARK+1)*RULE_BENCHMARK;
      else
        imax=icount;
      *rulestruct=new SRuleStruct[imax];
      if (!rulestruct) 
      {
        //theJSLog<<"new rulestruct fail."<<ende;
        theJSLog.writeLog(LOG_CODE_NEWRULE_ERR,"new rulestruct fail.");
        return(-1);
      }
    
      sprintf(sqltmp, "select ruleno, %s, \
        start_time, rategroup_id, end_time from %s order by ruleno, start_time",
        fieldname, tablename);
       // cout << "fieldname "<< fieldname<<endl;
        //cout << "tablename "<< tablename<<endl;
        //cout << "imax "<< imax<<endl;
      if (strcmp(source_group, "")!=0)
      {
        strcat(sqltmp, " where source_group=:source_group");
        stmt.setSQLString(sqltmp);
        stmt<<source_group;	
       
		stmt.execute();
      }
      else
      {
        stmt.setSQLString(sqltmp);	
       // cout << sqltmp <<endl;
		stmt.execute();
      }
      //***********************************************************
  // Repeat reading all record
  //***********************************************************
  for (i=0; i<icount; i++)
  {
    char szRuleField[RATE_MAX_RULEITEM_LEN];
    char szRuleFieldAll[CALCFEE_RULE_LEN_MAX];

    stmt>>(*rulestruct)[i].iRuleNo;
    
    szRuleFieldAll[0] = 0x00;
    for (j=0; j<Param.iRuleFieldCount; j++)
    {
      int iLen;
      stmt>>szRuleField;
      delSpace(szRuleField, 0);
      strcat(szRuleFieldAll, szRuleField);
      iLen = strlen(szRuleFieldAll);
      szRuleFieldAll[iLen] = RATE_RULE_SEPERATOR;
      szRuleFieldAll[iLen + 1] = 0x00;
    }

	
    stmt>>(*rulestruct)[i].szStartTime>>(*rulestruct)[i].szRateGroupId>>
      (*rulestruct)[i].szEndTime;

    int iFieldLen;
    iFieldLen = strlen(szRuleFieldAll);
    szRuleFieldAll[iFieldLen - 1] = 0x00;		//去掉最后面的分隔符
    //(*rulestruct)[i].szRateRule = new char[iFieldLen];
    /*if (!(*rulestruct)[i].szRateRule)
    {
      theJSLog<<"new rulestruct field fail."<<endd;
      return(-1);
    }*/
    strcpy((*rulestruct)[i].szRateRule, szRuleFieldAll);
    //cout<<(*rulestruct)[i].szRateRule<<endl;
	
    strcpy((*rulestruct)[i].szRuleFieldMatchMode, "1");

    //cout<<(*rulestruct)[i].szRuleFieldMatchMode<<endl;
    //if (Param.rule_blur->addRule((*rulestruct)[i]) == false)
   // {
    //  theJSLog<<"insert into ruleblur fail."<<endd;
    //  return(-1);    	
    //}
  } //end for
  conn.commit();  
  		
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return false;
	 }
	 conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "InitRuleStruct 出错" << endi;
  		throw jsexcp::CException(0, "InitRuleStruct 出错", __FILE__, __LINE__);
  		conn.close();
  		return false;
     }        

  rulecount=icount;
  maxcount=imax;
  return 0;
}


int FreeRuleStruct(char *debug_flag, SRuleStruct **rulestruct, int rulecount)
{
  if (*rulestruct)
  {
    for (int i=0; i<rulecount; i++)
    {
      if ((*rulestruct)[i].szRateRule) 
      {
        //delete[] (*rulestruct)[i].szRateRule;
        //(*rulestruct)[i].szRateRule = NULL;
        memset((*rulestruct)[i].szRateRule,0,sizeof((*rulestruct)[i].szRateRule));
      }
    }
    delete[] (*rulestruct);
    (*rulestruct) = NULL;
  }
  return 0;
}

int SearchRate(char *debug_flag, RATESTRUCT &srate, char *tablename)
{
  //expTrace(debug_flag, __FILE__, __LINE__, "Begin SearchRate()");

  char szDateTime[15], szDate[9], szTime[7];
  char szStartTime[15], szEndTime[15], szCdrStartDate[9], szCdrEndDate[9], szCdrStartTime[7], szCdrEndTime[7];
  int iRateA, iRateB, iRateAddA, iRateAddB, iMeterCount, iChargeUnit;
  char szSqlTmp[SQL_LEN+1];
  sprintf(szDateTime, "%s", srate.start_time);
  strncpy(szDate, szDateTime, 8);
  szDate[8]=0;
  if(strlen(szDateTime)==8)
  	{
  	  strcpy(szTime, "000000");
  	}
  else{
     strncpy(szTime, &szDateTime[8], 6);
     szTime[6] = 0;
  	}
  //strncpy(szTime, &szDateTime[8], 6);
  //szDate[6]=0;

  //CBindSQL ds(DBConn);
  DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(szSqlTmp,"select start_time, end_time, cdrstart_date, cdrend_date, cdrstart_time, cdrend_time, rate_a, rate_b, rate_add_a, rate_add_b, meter_count, charge_unit from %s where tariff_id=:tariff_id and start_time<=:oper_time and cdrstart_date<=:cdr_date and cdrstart_time<=:cdr_time and cdrend_time>=:cdr_time",tablename);
            stmt.setSQLString(szSqlTmp);
			stmt << srate.tariff_id<<szDateTime<<szDate<<szTime<<szTime;
			//cout << "szSqlTmp " <<szSqlTmp<<endl;
			stmt.execute();
            //cout<<srate.tariff_id<<szDateTime<<szDate<<szTime<<szTime;
      while (stmt>>szStartTime>>szEndTime>>szCdrStartDate>>szCdrEndDate>>szCdrStartTime>>szCdrEndTime>>iRateA>>iRateB>>iRateAddA>>iRateAddB>>iMeterCount>>iChargeUnit)
      {
        if (((strcmp(szEndTime, "")==0) || ((strcmp(szEndTime, "")!=0) && (strcmp(szDateTime, szEndTime)<=0))) &&
           ((strcmp(szCdrEndDate, "")==0) || ((strcmp(szCdrEndDate, "")!=0) && (strcmp(szDate, szCdrEndDate)<=0))))
        {
          sprintf(srate.start_time, "%s", szStartTime);
          sprintf(srate.end_time, "%s", szEndTime);
          sprintf(srate.cdrstart_date, "%s", szCdrStartDate);
          sprintf(srate.cdrend_date, "%s", szCdrEndDate);
          sprintf(srate.cdrstart_time, "%s", szCdrStartTime);
          sprintf(srate.cdrend_time, "%s", szCdrEndTime);
          srate.rate_a=iRateA;
          srate.rate_b=iRateB;
          srate.rate_add_a=iRateAddA;
          srate.rate_add_b=iRateAddB;
          srate.meter_count=iMeterCount;
          srate.charge_unit=iChargeUnit;
          //expTrace(debug_flag, __FILE__, __LINE__,  "End SearchRate(), return=%d;", 0);
          DEBUG_LOG<<"End SearchRate(), return 0 "<<endd;
          return 0;
        } //end if
      } //end while	
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return 0;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "SearchRate查找批价规则出错" << endi;
  		throw jsexcp::CException(0, "SearchRate查找批价规则出错", __FILE__, __LINE__);
  		conn.close();
  		return 0;
     }
  //expTrace(debug_flag, __FILE__, __LINE__, "End SearchRate(), return=%d;", -1);
  return -1;
}

int SearchRuleBlur(char *debug_flag, SRuleStruct *rulestruct,
  int rulecount, SRuleStruct &srule)
{
  int i;
  //expTrace(debug_flag, __FILE__, __LINE__,
  //  "Begin SearchRuleBlur(), rulecount=%d", rulecount);
  //cout<<"rulecount:"<<rulecount<<endl;
  for (i=0; i<rulecount; i++)
  {
    if ( (CompareRuleBlur(debug_flag, &(rulestruct[i]), &srule) == 0)
      && (CompareTime(debug_flag, rulestruct[i].szStartTime, rulestruct[i].szEndTime, srule.szStartTime) == 0) )
    {
      /*
        expTrace(debug_flag, __FILE__, __LINE__,
          "iRuleNo=%d;", rulestruct[i].iRuleNo);
        expTrace(debug_flag, __FILE__, __LINE__,
          "callingbusiness=%s;", rulestruct[i].callingbusiness);
        expTrace(debug_flag, __FILE__, __LINE__,
          "callingregion=%s;", rulestruct[i].callingregion);
        expTrace(debug_flag, __FILE__, __LINE__,
          "callingmobile=%s;", rulestruct[i].callingmobile);
        expTrace(debug_flag, __FILE__, __LINE__,
          "callingservice=%s;", rulestruct[i].callingservice);
        expTrace(debug_flag, __FILE__, __LINE__,
          "calledbusiness=%s;", rulestruct[i].calledbusiness);
        expTrace(debug_flag, __FILE__, __LINE__,
          "calledregion=%s;", rulestruct[i].calledregion);
        expTrace(debug_flag, __FILE__, __LINE__,
          "calledmobile=%s;", rulestruct[i].calledmobile);
        expTrace(debug_flag, __FILE__, __LINE__,
          "calledservice=%s;", rulestruct[i].calledservice);
        expTrace(debug_flag, __FILE__, __LINE__,
          "fwdsmgbusiness=%s;", rulestruct[i].fwdsmgbusiness);
        expTrace(debug_flag, __FILE__, __LINE__,
          "start_time=%s;", rulestruct[i].start_time);
        expTrace(debug_flag, __FILE__, __LINE__,
          "tariff_id=%s;", rulestruct[i].tariff_id);
        expTrace(debug_flag, __FILE__, __LINE__,
          "rulerepnoa=%d;", rulestruct[i].rulerepnoa);
        expTrace(debug_flag, __FILE__, __LINE__,
          "rulerepnob=%d;", rulestruct[i].rulerepnob);
        expTrace(debug_flag, __FILE__, __LINE__,
          "end_time=%s;", rulestruct[i].end_time);
      */
      srule.iRuleNo=rulestruct[i].iRuleNo;
      sprintf(srule.szRateGroupId, "%s", rulestruct[i].szRateGroupId);
      sprintf(srule.szStartTime, "%s", rulestruct[i].szStartTime);
      sprintf(srule.szEndTime, "%s", rulestruct[i].szEndTime);
      //expTrace(debug_flag, __FILE__, __LINE__,
      //  "End SearchRuleBlur(), return=%d", 0);
     // cout<<"find rule"<<endl;
      return 0;
    }
  }
  //expTrace(debug_flag, __FILE__, __LINE__, "End SearchRuleBlur(), return=%d", -1);
  return -1;
}

int CheckRuleExact(char *debug_flag, SRuleStruct *rulestruct,
  int rulecount, SRuleStruct &srule)
{
  int pstart=0;
  int pend = rulecount-1;
  int pmiddle = (pend+pstart)/2;
  int plast = 0;
  int ifind = 0;

  int islarger = 0;
  int isequal = 0;
  
  SRuleStruct* cur_rule;
  cur_rule = rulestruct;

  while (pstart <= pend) 
  {
    if ((isequal==0) && (pstart == pend))
      isequal = 1;
    else if (isequal==1)
      break;
    pmiddle = (pend+pstart)/2;
    if (plast<pmiddle) 
    {
      cur_rule=cur_rule+(pmiddle-plast);
    }
    else if (plast>pmiddle) 
    {
      cur_rule=cur_rule-(plast-pmiddle);
    }

    if (srule.iRuleNo > cur_rule->iRuleNo)
      islarger = 1;
    else if (srule.iRuleNo < cur_rule->iRuleNo)
      islarger = -1;
    else if (srule.iRuleNo == cur_rule->iRuleNo)
    {
    	islarger = strcmp(srule.szStartTime,cur_rule->szStartTime);
    }
    if (islarger>0)
    {
      //forward
      pstart = pmiddle+1;
      plast  = pmiddle;
    }
    else if (islarger<0) 
    {
      //backward
      pend = pmiddle-1;
      plast  = pmiddle;
    }
    else if (islarger==0) 
    {
      ifind = 1;
    } //end else

    if (ifind == 1)
      break;
  } //end while

  if ( (ifind == 1)
    && (CompareRuleBlur(debug_flag, cur_rule, &srule)==0)
    //&& (cur_rule->iRuleNo == srule.iRuleNo)
    //&& (strcmp(cur_rule->szStartTime,srule.szStartTime) == 0)
    && (strcmp(cur_rule->szEndTime,srule.szEndTime) == 0)
    && (strcmp(cur_rule->szRateGroupId,srule.szRateGroupId) == 0) )
  {
    return 0;
  }
  return -1;
}

int CompareRuleBlur(char *debug_flag, SRuleStruct *blurstruct, SRuleStruct *truestruct)
{
  char szBlurFieldItem[RATE_MAX_RULEITEM_NUM][RATE_MAX_RULEITEM_LEN];
  char szTrueFieldItem[RATE_MAX_RULEITEM_NUM][RATE_MAX_RULEITEM_LEN];
  char szRateModeItem[RATE_MAX_RULEITEM_NUM][RATE_MAX_RULEITEM_LEN];
  int iBlurField,iTrueField,iRateMode;
  blurstruct->getRuleItem(szBlurFieldItem,iBlurField);
  truestruct->getRuleItem(szTrueFieldItem,iTrueField);
  truestruct->getRateModeItem(szRateModeItem,iRateMode);
  
  if (((iBlurField != iTrueField) ||(iBlurField != iRateMode)|| (iBlurField != Param.iRuleFieldCount)))
  {
    //theJSLog<<"Field count error!"<<ende;
    theJSLog.writeLog(LOG_CODE_RATECOUNT_ERR,"Field count error!");
    return -1;
  }

 // cout << "Param.iRuleFieldCount = " << Param.iRuleFieldCount << endl;
  for (int i=0; i<Param.iRuleFieldCount; i++)
  {
  	if (CompareRuleBlur(debug_flag, szBlurFieldItem[i], szTrueFieldItem[i],szRateModeItem[i]) != 0)
  	  return -1;
  }

  return 0;
}

int CompareRuleBlur(char *debug_flag, char *ruletext, char *truetext,char *ratemode)
{
  //expTrace(debug_flag, __FILE__, __LINE__,
   // "CompareRuleBlur(), ruletext=%s;truetext=%s;ratemode=%s;", ruletext, truetext,ratemode);
   //sprintf(debug_flag,"CompareRuleBlur(), ruletext=%s;truetext=%s;ratemode=%s;", ruletext, truetext,ratemode);
   //cout<<debug_flag<<endl;
  
  if (strcmp(ruletext, "")==0) {
 // cout<<"ruletext is null"<<endl;
    return 0;
  }

  if(strcmp(ratemode,"1")==0){
	  if ((strncmp(ruletext, "~", 1)!=0) && (strncmp(ruletext, truetext, strlen(ruletext))==0)) {
	    return 0;
	  }

	  if ((strncmp(ruletext, "~", 1)==0) &&
	    (strncmp(&ruletext[1], truetext, strlen(ruletext)-1)!=0)) {
	    return 0;
	  }
  }

   if(strcmp(ratemode,"2")==0){
	  if ((strcmp(ruletext, "~")!=0) && (strcmp(ruletext, truetext)==0)) {
	  
	    return 0;
	  }

	  if ((strcmp(ruletext, "~")==0) &&
	    (strcmp(&ruletext[1], truetext)!=0)) {
	    return 0;
	  }
   }

  //20131213 lij  add 添加可以匹配多个字段的内容，用,  分隔
  
  //expTrace(debug_flag, __FILE__, __LINE__, "CompareRuleBlur(), retrun=%d;", -1);
  return -1;
}

int CompareTime(char *debug_flag, char *begintime, char *endtime, char *truetime)
{
  int inrange;
/*
  expTrace(debug_flag, __FILE__, __LINE__, "Start CompareTime()");
  expTrace(debug_flag, __FILE__, __LINE__, "begintime=%s;", begintime);
  expTrace(debug_flag, __FILE__, __LINE__, "endtime=%s;", endtime);
  expTrace(debug_flag, __FILE__, __LINE__, "truetime=%s;", truetime);

  cout << "begintime = " <<begintime<<endl;
  cout << "endtime = " <<endtime<<endl;
  cout << "truetime = " <<truetime<<endl;*/

  if ((endtime == NULL) || (strcmp(endtime,"") == 0))
  {
    if (strcmp(truetime, begintime)>=0) {
      inrange = 0;
    }
      else {
        inrange = 1;
      }
  }
    else {
      if ((strcmp(truetime, begintime)>=0) &&
        (strcmp(truetime, endtime)<0)) {
        inrange = 0;
      }
        else if (strcmp(truetime, begintime)<0) {
          inrange = 1;
        }
          else {
            inrange = -1;
          }
    }

  //expTrace(debug_flag, __FILE__, __LINE__, "End CompareTime(), Ret=%d;", inrange);
  //cout << "inrange = " <<inrange<<endl;
  return inrange;
}

int CompareDuration(char *debug_flag, int beginduration, int endduration, int trueduration)
{
  int inrange;

  if ((trueduration>=beginduration) &&
    (trueduration<endduration)) {
    inrange = 0;
  }
    else if (trueduration<beginduration) {
      inrange = 1;
    }
      else {
        inrange = -1;
      }

  return inrange;
}

int InitAdjRateStruct(char *debug_flag, ADJRATESTRUCT **adjratestruct, int &adjratecount, char *tablename)
{
  int i, icount;
  char sqltmp[512];

  //CBindSQL ds(DBConn);
  DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(sqltmp, "select count(*) from %s", tablename);
			stmt.setSQLString(sqltmp);	
			stmt.execute();
			stmt >> icount;
			
			if (icount <= 0)
     	{
        adjratecount = 0;
        return 0;
      }
    
      *adjratestruct=new ADJRATESTRUCT[icount];
      if (!adjratestruct) {
        //theJSLog<<"new adjratestruct fail."<<ende;
        theJSLog.writeLog(LOG_CODE_NEWADJRULE_ERR,"new adjratestruct fail.");
        return(-1);
      }
      
      sprintf(sqltmp, "select TARIFF_ID, START_TIME, END_TIME, DURATION_BEGIN, DURATION_END, RATE_A_FACTOR, RATE_A_BASE,  \
        RATE_B_FACTOR, RATE_B_BASE, RATE_ADD_A_FACTOR, RATE_ADD_A_BASE, RATE_ADD_B_FACTOR, RATE_ADD_B_BASE, METER_COUNT, CHARGE_UNIT \
        from %s ORDER BY TARIFF_ID, START_TIME", tablename);
      stmt.setSQLString(sqltmp);	
			stmt.execute();
      //***********************************************************
      // Repeat reading all record
      //***********************************************************
      for (i=0;i<icount;i++) {
        stmt>>(*adjratestruct)[i].tariff_id>>(*adjratestruct)[i].start_time>>
          (*adjratestruct)[i].end_time>>(*adjratestruct)[i].duration_begin>>
          (*adjratestruct)[i].duration_end>>(*adjratestruct)[i].rate_a_factor>>
          (*adjratestruct)[i].rate_a_base>>(*adjratestruct)[i].rate_b_factor>>
          (*adjratestruct)[i].rate_b_base>>(*adjratestruct)[i].rate_add_a_factor>>
          (*adjratestruct)[i].rate_add_a_base>>(*adjratestruct)[i].rate_add_b_factor>>
          (*adjratestruct)[i].rate_add_b_base>>(*adjratestruct)[i].meter_count>>
          (*adjratestruct)[i].charge_unit;
      } //end for 			
	
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return false;
	 }
	 conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "InitAdjRateStruct出错" << endi;
  		throw jsexcp::CException(0, "InitAdjRateStruct出错", __FILE__, __LINE__);
  		conn.close();
  		return false;
  } 

  adjratecount=icount;
  return 0;
}

int FreeAdjRateStruct(char *debug_flag, ADJRATESTRUCT **adjratestruct)
{
  if (*adjratestruct) {
    delete[] (*adjratestruct);
    (*adjratestruct) = NULL;
  }
  return 0;
}

int SearchAdjRate(char *debug_flag, ADJRATESTRUCT* adjratestruct, int adjratecount,
  ADJRATESTRUCT &srate)
{
  int pstart=0;
  int pend = adjratecount-1;
  int pmiddle = (pend+pstart)/2;
  int plast = 0;
  int ifind = 0;

  int islarger = 0;
  int inrange=-2;
  int isequal = 0;
  char oper_time[15];
  int iDuration;

  ADJRATESTRUCT* cur_rate;
  //expTrace(debug_flag, __FILE__, __LINE__,
  //  "Begin SearchRate(), ratecount=%d;", ratecount);
  cur_rate = adjratestruct;

  sprintf(oper_time, "%s", srate.start_time);
  iDuration = srate.duration_begin;
  while (pstart <= pend) {
    if ((isequal==0)&&(pstart == pend))
      isequal = 1;
      else if (isequal==1)
        break;
    pmiddle = (pend+pstart)/2;
    if (plast<pmiddle) {
      cur_rate=cur_rate+(pmiddle-plast);
    }
      else if (plast>pmiddle) {
        cur_rate=cur_rate-(plast-pmiddle);
      }

    islarger = CompareAdjRate(debug_flag, cur_rate, srate);
    if (islarger>0) {
      //forward
      pstart = pmiddle+1;
      plast  = pmiddle;
    }
      else if (islarger<0) {
        //backward
        pend = pmiddle-1;
        plast  = pmiddle;
      }
        else if (islarger==0) {
          inrange = CompareAdjRateTime(debug_flag, cur_rate, oper_time, iDuration);
          if (inrange==0) {
            ifind = 1;
            plast = pmiddle;
            break;
          }
            else
            {
              //search around
              int move_count =0;
              if (pmiddle>0) {
                move_count++;
                pmiddle--;
                cur_rate--;
              }
                else {
                  //break;
                  move_count=0;
                }

              islarger = CompareAdjRate(debug_flag, cur_rate, srate);

              while (islarger == 0) {
                if (CompareAdjRateTime(debug_flag, cur_rate, oper_time, iDuration)==0) {
                  ifind = 1;
                  break;
                }

                if (pmiddle>0) {
                  move_count ++;
                  cur_rate--;
                  pmiddle--;
                }
                  else
                    break;

                islarger = CompareAdjRate(debug_flag, cur_rate, srate);
              }
              if (ifind == 1)
                break;

              for (int k=0;k<move_count+1;k++) {
                pmiddle++;
                cur_rate++;
              }
              move_count = 0 ;
              islarger = CompareAdjRate(debug_flag, cur_rate, srate);
              while (islarger == 0) {
                if (CompareAdjRateTime(debug_flag, cur_rate, oper_time, iDuration)==0) {
                  ifind = 1;
                  break;
                }

                if (adjratecount-pmiddle>0) {
                  pmiddle ++;
                  cur_rate++;
                  move_count ++;
                }
                  else
                    break;

                islarger = CompareAdjRate(debug_flag, cur_rate, srate);
              }
              break;
            }
        } //end else
        if (ifind == 1)
           break;
    }  //end while

  if (ifind==1) {
    //write result
    sprintf(srate.start_time, "%s", cur_rate->start_time);
    sprintf(srate.end_time, "%s", cur_rate->end_time);
    srate.duration_begin=cur_rate->duration_begin;
    srate.duration_end=cur_rate->duration_end;
    srate.rate_a_factor=cur_rate->rate_a_factor;
    srate.rate_a_base=cur_rate->rate_a_base;
    srate.rate_b_factor=cur_rate->rate_b_factor;
    srate.rate_b_base=cur_rate->rate_b_base;
    srate.rate_add_a_factor=cur_rate->rate_add_a_factor;
    srate.rate_add_a_base=cur_rate->rate_add_a_base;
    srate.rate_add_b_factor=cur_rate->rate_add_b_factor;
    srate.rate_add_b_base=cur_rate->rate_add_b_base;
    srate.meter_count=cur_rate->meter_count;
    srate.charge_unit=cur_rate->charge_unit;
    /*
    expTrace(debug_flag, __FILE__, __LINE__, "tariff_id=%s;", srate.tariff_id);
    expTrace(debug_flag, __FILE__, __LINE__, "start_time=%s;", srate.start_time);
    expTrace(debug_flag, __FILE__, __LINE__, "end_time=%s;", srate.end_time);
    expTrace(debug_flag, __FILE__, __LINE__, "callingfee=%d;", srate.calling_fee);
    expTrace(debug_flag, __FILE__, __LINE__, "calledfee=%d;", srate.called_fee);
    */
    //expTrace(debug_flag, __FILE__, __LINE__, "End SearchRate(), return=%d;", 0);
    return 0;
  }
    else {
      //expTrace(debug_flag, __FILE__, __LINE__,
      //  "End SearchRate(), return=%d;", -1);
      return -1;
    }
}

int SearchAdjRate(char *debug_flag, ADJRATESTRUCT &srate, char *tablename)
{
  //expTrace(debug_flag, __FILE__, __LINE__, "Begin SearchRate()");
  DBConnection conn;//数据库连接	
  char szDateTime[15];
  char szStartTime[15], szEndTime[15];
  char szSqlTmp[SQL_LEN+1];
  int iDurationBegin, iDurationEnd;
  int iRateAFactor, iRateABase, iRateBFactor, iRateBBase, iRateAddAFactor, iRateAddABase, iRateAddBFactor, iRateAddBBase, iMeterCount, iChargeUnit;
  sprintf(szDateTime, "%s", srate.start_time);

  try{		  	
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(szSqlTmp, "select start_time, end_time, duration_begin, duration_end, rate_a_factor, rate_a_base, rate_b_factor, rate_b_base, rate_add_a_factor, rate_add_a_base, rate_add_b_factor, rate_add_b_base, meter_count, charge_unit from %s where tariff_id=:tariff_id and start_time<=:oper_time and duration_begin<=:druation and duration_end>:duration", tablename);
      stmt.setSQLString(szSqlTmp);	
      stmt<<srate.tariff_id<<szDateTime<<srate.duration_begin<<srate.duration_begin;	
			stmt.execute();
			
      while (stmt>>szStartTime>>szEndTime>>iDurationBegin>>iDurationEnd>>iRateAFactor>>iRateABase>>iRateBFactor>>iRateBBase>>iRateAddAFactor>>iRateAddABase>>iRateAddBFactor>>iRateAddBBase>>iMeterCount>>iChargeUnit)
      {
        if ((strcmp(szEndTime, "")==0) || ((strcmp(szEndTime, "")!=0) && (strcmp(szDateTime, szEndTime)<=0)))
        {
          sprintf(srate.start_time, "%s", szStartTime);
          sprintf(srate.end_time, "%s", szEndTime);
          srate.duration_begin = iDurationBegin;
          srate.duration_end = iDurationEnd;
          srate.rate_a_factor=iRateAFactor;
          srate.rate_a_base=iRateABase;
          srate.rate_b_factor=iRateBFactor;
          srate.rate_b_base=iRateBBase;
          srate.rate_add_a_factor=iRateAddAFactor;
          srate.rate_add_a_base=iRateAddABase;
          srate.rate_add_b_factor=iRateAddBFactor;
          srate.rate_add_b_base=iRateAddBBase;
          srate.meter_count=iMeterCount;
          srate.charge_unit=iChargeUnit;
          //ds.Close();
          //expTrace(debug_flag, __FILE__, __LINE__,
          //  "End SearchRate(), return=%d;", 0);
          return 0;
        } //end if
      } //end while
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return 0;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "SearchAdjRate出错" << endi;
  		throw jsexcp::CException(0, "SearchAdjRate出错", __FILE__, __LINE__);
  		conn.close();
  		return 0;
     } 	     
  //expTrace(debug_flag, __FILE__, __LINE__, "End SearchRate(), return=%d;", -1);
  return -1;
}

int CompareAdjRate(char *debug_flag, ADJRATESTRUCT* cur_rate, ADJRATESTRUCT &srate)
{
  int ret;
  //expTrace(debug_flag, __FILE__, __LINE__,
  //  "CompareRate(), ratetext=%s;truetext=%s;",
  //  srate.tariff_id, cur_rate->tariff_id);
  ret = strcmp(srate.tariff_id, cur_rate->tariff_id);

  return ret;
}

int CompareAdjRateTime(char *debug_flag, ADJRATESTRUCT* cur_rate, char *truetime, int duration)
{
  int ret;

  if ((CompareTime(debug_flag,  cur_rate->start_time,  cur_rate->end_time,  truetime ) == 0) &&
     (CompareDuration(debug_flag,  cur_rate->duration_begin, cur_rate->duration_end, duration) == 0))
  {
    ret = 0; //完全匹配
  }
  else
  {
	  ret = -1; //只要有任何一个条件不匹配，即返回-1
  }

  return ret;
}

void PrintRuleCondition(char *debug_flag, SRuleStruct rulestruct, char *szDoneTime)
{
  theJSLog<<"szRateRule = "<<rulestruct.szRateRule<<";"<<endd;
  theJSLog<<"szDoneTime = "<<szDoneTime<<";"<<endd;  
}

void PrintRuleResult(char *debug_flag, SRuleStruct rulestruct)
{
  theJSLog<<"iRuleNo 		 	 = "<<rulestruct.iRuleNo<<endd;
  theJSLog<<"szStartTime 	 = "<<rulestruct.szStartTime<<endd;
  theJSLog<<"szEndTime		 = "<<rulestruct.szEndTime<<endd;
  theJSLog<<"szRateGroupId = "<<rulestruct.szRateGroupId<<endd;
}

int timeStrAddSecond(char *timeStr, int addSecond)
{
	if ((timeStr != NULL) && ((strlen(timeStr) == 14) || (strlen(timeStr) == 6)))
	{
		bool bTertian = false;
		int  i, j, h, m, s;
		char szDateTime[15];
		char szTime[7];
		if (strlen(timeStr) == 14)
		{
		  strcpy(szDateTime, timeStr);
		  memcpy(szTime, timeStr + 8,  2);  szTime[2] =0;	 h = atoi(szTime);
		  memcpy(szTime, timeStr + 10, 2);	szTime[2] =0;	 m = atoi(szTime);
		  memcpy(szTime, timeStr + 12, 2);	szTime[2] =0;	 s = atoi(szTime);
		}
		else if (strlen(timeStr) == 6)
		{
		  sprintf(szDateTime, "%s%s", "20010101", timeStr);
		  memcpy(szTime, timeStr + 0, 2);  szTime[2] =0;	 h = atoi(szTime);
		  memcpy(szTime, timeStr + 2, 2);	 szTime[2] =0;	 m = atoi(szTime);
		  memcpy(szTime, timeStr + 4, 2);	 szTime[2] =0;	 s = atoi(szTime);			
		}

    if (addSecond > 0) 
    {
      i = s + addSecond;
      s = (i % 60);
      j = (i / 60);
      
      if (j > 0) //秒进分
      {
      	i = m + j;
      	m = (i % 60);
      	j = (i / 60);
      	if (j > 0)  //分进时
      	{
      		h = h + j;
      		if (h > 23)
      		{
      			time2TimeStr(timeStr2Time(szDateTime) + addSecond, szDateTime);
      			bTertian = true;
      		}
      	}
      }
    }
    else if (addSecond < 0)
    {
      i = s + addSecond;
      s = (i % 60);
      j = (i / 60);
      
      if (i < 0) //秒退分
      {
      	s = s + 60;
      	i = m - j -1;
      	m = (i % 60);
      	j = (i / 60);
      	if (i < 0)  //分退时
      	{
      		m = m + 60;
      		h = h - j -1;
      		if (h < 0)
      		{
      			time2TimeStr(timeStr2Time(szDateTime) + addSecond, szDateTime);
      			bTertian = true;
      		}
      	}
      }    	
    }
    
    if (bTertian == false)
    {
      if (strlen(timeStr) == 14)
      {
        sprintf(szTime, "%02d%02d%02d", h, m, s);
        memcpy(timeStr + 8, szTime, 6);
      }
      else if (strlen(timeStr) == 6)
      {
        sprintf(szTime, "%02d%02d%02d", h, m, s);
        strcpy(timeStr , szTime);
      }
	  }
	  else
	  {
      if (strlen(timeStr) == 14)
      {
        strcpy(timeStr, szDateTime);
      }
      else if (strlen(timeStr) == 6)
      {
        strcpy(timeStr , szDateTime + 8);
      }	  	
	  }
	}
	return 0;
}

int timeGetWeek(char *timeStr)
{
  time_t Time;
  struct tm tmDay;
  static char strTmp[5];
  bzero( &tmDay , sizeof( struct tm ) );
  bzero( strTmp , sizeof( strTmp));
  strncpy( strTmp , timeStr , 4 );
  tmDay.tm_year = atoi( strTmp ) - 1900 ;
  bzero( strTmp , sizeof( strTmp));
  strncpy( strTmp , timeStr + 4 , 2 ) ;
  tmDay.tm_mon = atoi( strTmp ) - 1;
  bzero( strTmp , sizeof( strTmp));
  strncpy( strTmp , timeStr + 6 , 2 );
  tmDay.tm_mday = atoi( strTmp );
  Time = mktime(&tmDay);

  if (tmDay.tm_wday == 0)
    return 7;
  else
    return tmDay.tm_wday;
}

//根据tariff_id返回第一个匹配的费率及所有匹配费率的个数
int SearchRate(char *debug_flag, RATESTRUCT* ratestruct, int ratecount,
  RATESTRUCT srate, RATESTRUCT **rslast, int &matchcount)
{
  int pstart=0;
  int pend = ratecount-1;
  int pmiddle = (pend+pstart)/2;
  int plast = 0;
  int ifind = 0;

  int islarger = 0;
  int inrange=-2;
  int isequal = 0;
  int iMatchCount = 0;

  RATESTRUCT* cur_rate;
  cur_rate = ratestruct;

  while (pstart <= pend) {
    if ((isequal==0)&&(pstart == pend))
      isequal = 1;
      else if (isequal==1)
        break;
    pmiddle = (pend+pstart)/2;
    if (plast<pmiddle) {
      cur_rate=cur_rate+(pmiddle-plast);
    }
      else if (plast>pmiddle) {
        cur_rate=cur_rate-(plast-pmiddle);
      }

    islarger = CompareRate(debug_flag, cur_rate, srate);
    if (islarger>0) {
      //forward
      pstart = pmiddle+1;
      plast  = pmiddle;
    }
      else if (islarger<0) {
        //backward
        pend = pmiddle-1;
        plast  = pmiddle;
      }
        else if (islarger==0) 
        {
        	ifind = 1;
        	//移动到最后一个                          
        	while (ratecount-(pmiddle+1)>0)                          
        	{                          
        		cur_rate++;                          
        		pmiddle++;                          
        		if (CompareRate(debug_flag, cur_rate, srate) != 0)                          
        		{                          
        			cur_rate--;                          
        			pmiddle--;                          
        			break;                          
        		}                          
        	}                          
        	//倒着一个一个查找                          
        	do                          
        	{                          
        		iMatchCount++;
        		if (pmiddle<0)                          
        		  break;                          
       			cur_rate--;                          
      			pmiddle--;                          
        	} while (CompareRate(debug_flag, cur_rate, srate) == 0);                          
        } //end else
        if (islarger==0)
           break;
    }  //end while

  if (ifind==1) {
    matchcount = iMatchCount;
   	*rslast = cur_rate + iMatchCount;
    return 0;
  }
    else {
      return -1;
    }
}

int getCondIdx(char *Sourceid,int total,RateCondition *Rate_Condition)
{
	int pstart = 0;
	int pend = total-1;
	int pmiddle = (pend+pstart)/2;
	int plast = 0;
	int isequal = -3;
	int isend = 0;
	RateCondition *Cur_Condition = Rate_Condition;
	       
	while (pstart <= pend)
	{
		if ((isend==0)&&(pstart == pend))
			isend = 1;
		else if (isend==1)
			break;
	  
		pmiddle = (pend+pstart)/2; 
		//找到当前的匹配点
		Cur_Condition = Cur_Condition+pmiddle-plast;
		//plast=pmiddle;
	
		//进行比较
	  isequal = strcmp(Sourceid,Cur_Condition->szSourceId);
						
		if(isequal > 0)//向大的方向找
		{
			pstart = pmiddle+1;
			plast  = pmiddle;
		}
		
		else if (isequal < 0)
		{
			//向小的方向找
			pend = pmiddle-1;
			plast  = pmiddle;
		}
		else if (isequal==0)
		{
			return pmiddle;     
		}
		
	}
	return -1;	
	
}

int CompareFormula(char *debug_flag, FormulaStruct* cur_formula, FormulaStruct &sformula)
{
  int ret;
  ret = strcmp(sformula.szFormulaID, cur_formula->szFormulaID);
  //cout << "sformula.szFormulaID = " << sformula.szFormulaID<<endl;
  //cout << "cur_formula->szFormulaID = " << cur_formula->szFormulaID<<endl;
  //ret = sformula.szFormulaID - cur_formula->szFormulaID;
  return ret;
}

int CompareFormulaTime(char *debug_flag, FormulaStruct* cur_formula, char *truetime)
{
  //暂时不处理
  int iret = 0;
  return iret;
}

int SearchFormula(char *debug_flag,int fcount,FormulaStruct *formula,FormulaStruct &desformula)
{
    int pstart=0;
    int pend = fcount-1;
    int pmiddle = (pend+pstart)/2;
    int plast = 0;
    int ifind = 0;

    int islarger = 0;
    int inrange=-2;
    int isequal = 0;
    char oper_time[15];
    FormulaStruct* cur_formula;
    cur_formula = formula;

    //sprintf(oper_time, "%s", formula.start_time);
    //cout << "desformula.szFormulaID = " << desformula.szFormulaID <<endl;
    while (pstart <= pend) {
    if ((isequal==0)&&(pstart == pend))
      isequal = 1;
      else if (isequal==1)
        break;
    pmiddle = (pend+pstart)/2;
    if (plast<pmiddle) {
      cur_formula=cur_formula+(pmiddle-plast);
    }
      else if (plast>pmiddle) {
        cur_formula=cur_formula-(plast-pmiddle);
      }

     islarger = CompareFormula(debug_flag, cur_formula, desformula);
     //cout << "islarger = " <<islarger<<endl;
    if (islarger>0) {
      //forward
      pstart = pmiddle+1;
      plast  = pmiddle;
    }
      else if (islarger<0) {
        //backward
        pend = pmiddle-1;
        plast  = pmiddle;
      }
        else if (islarger==0) 
        {
        	//移动到最后一个                          
        	while (fcount-(pmiddle+1)>0)                          
        	{                          
        		cur_formula++;                          
        		pmiddle++;                          
        		if (CompareFormula(debug_flag, cur_formula, desformula) != 0)                          
        		{                          
        			cur_formula--;                          
        			pmiddle--;                          
        			break;                          
        		}                          
        	}                          
        	//倒着一个一个查找                          
        	do                          
        	{                          
        		if (CompareFormulaTime(debug_flag, cur_formula, oper_time) == 0)                      
        		{                          
        			ifind = 1;                          
        			break;                          
        		}                          
        		if (pmiddle-1<0)                          
        		  break;                          
       			cur_formula--;                          
      			pmiddle--;                          
        	} while (CompareFormula(debug_flag, cur_formula, desformula) == 0);                          

        } //end else
        if (islarger==0)
           break;
      }
           //cout << "ifind = " <<ifind<<endl;
     if(ifind == 1){
     	//将查找到的公式内容赋值    	   
        		int formulano= cur_formula->expno;
        		int paramno = cur_formula->paramno;
        		//cout << "formulano = " <<formulano<<endl;
        		//cout << "paramno = " <<paramno<<endl;
        		desformula.expno = formulano;
        		desformula.paramno = paramno;
        		
        		(desformula.szFormulaExp)=new FormulaExp[formulano];
        		(desformula.szFormulaParam)=new FormulaParam[paramno];
        		 for (int j=0;j<formulano;j++) {
        		 	//赋公式值
        		 	desformula.szFormulaExp[j].index_id = (cur_formula->szFormulaExp)[j].index_id;
        		 	strcpy(desformula.szFormulaExp[j].szSegmentName,(cur_formula->szFormulaExp)[j].szSegmentName);
        		 	strcpy(desformula.szFormulaExp[j].szFormula,(cur_formula->szFormulaExp)[j].szFormula);
        		 	strcpy(desformula.szFormulaExp[j].szStartTime,(cur_formula->szFormulaExp)[j].szStartTime);
        		 	strcpy(desformula.szFormulaExp[j].szEndTime,(cur_formula->szFormulaExp)[j].szEndTime);
        		 	desformula.szFormulaExp[j].output_type = (cur_formula->szFormulaExp)[j].output_type;
        		 	desformula.szFormulaExp[j].decimal_num = (cur_formula->szFormulaExp)[j].decimal_num;
        		 	//cout << "desformula.szFormulaExp[j].szSegmentName = " <<desformula.szFormulaExp[j].szSegmentName<<endl;
        		 	
        		}
                 for(int k = 0;k<paramno; k++) {
                 	//赋参数值
                 	strcpy(desformula.szFormulaParam[k].szParamName,(cur_formula->szFormulaParam)[k].szParamName);
                 	strcpy(desformula.szFormulaParam[k].szParamType,(cur_formula->szFormulaParam)[k].szParamType);
                 	strcpy(desformula.szFormulaParam[k].szParamValue,(cur_formula->szFormulaParam)[k].szParamValue);       		 	
                  }
     	}
  
}

int SearchFormula(char *debug_flag,char *formulaID,FormulaStruct *desformula,int &expno,int &paramno,char *formulatablename,char *paramtablename)
{
    char sqltmp[SQL_LEN+1];
   // int ecount,pcount;
    DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
	  // 1、获取公式表达式
	     Statement stmt = conn.createStatement();
         sprintf(sqltmp, "select count(*) from %s where formula_id = '%s' ", formulatablename,formulaID);
         stmt.setSQLString(sqltmp);
         //cout << sqltmp <<endl;
	     stmt.execute();
	     stmt>>expno; //某公式对应的多个表达式数量
	     //FormulaExp *p = new FormulaExp[expno];
         //desformula->szFormulaExp = p;
	     (desformula->szFormulaExp)=new FormulaExp[expno];
	     
         sprintf(sqltmp, "select INDEX_NUM ,SEGMENT_NAME ,FORMULA ,START_TIME,END_TIME,output_type,decimal_num from %s where formula_id = '%s' order by index_num", formulatablename,formulaID);
         stmt.setSQLString(sqltmp);	
         //cout << sqltmp <<endl;
	     stmt.execute();
         for (int j=0;j<expno;j++) {
    	     stmt >> (desformula->szFormulaExp)[j].index_id
    	         >> (desformula->szFormulaExp)[j].szSegmentName >> (desformula->szFormulaExp)[j].szFormula
    	         >> (desformula->szFormulaExp)[j].szStartTime >> (desformula->szFormulaExp)[j].szEndTime
    	         >> (desformula->szFormulaExp)[j].output_type >> (desformula->szFormulaExp)[j].decimal_num;

    	     //cout << "输出类型和小数点" << (desformula->szFormulaExp)[j].output_type << (desformula->szFormulaExp)[j].decimal_num;
      	}

        // 2、获取公式参数内容
         sprintf(sqltmp, "select count(*) from %s where formula_id = '%s' ", paramtablename,formulaID);
         stmt.setSQLString(sqltmp);
         //cout << sqltmp<<endl;
	     stmt.execute();
	     stmt>>paramno; //某公式对应的多个表达式数量
	     (desformula->szFormulaParam)=new FormulaParam[paramno];
       
	   sprintf(sqltmp, "select PARAM_NAME,PARAM_TYPE,PARAM_VALUE from %s where FORMULA_ID = '%s' " ,paramtablename,formulaID);
       stmt.setSQLString(sqltmp);
       //cout << sqltmp<<endl;
	   stmt.execute();

	   for(int k=0;k<paramno;k++)
	   	{
	   	  stmt >>(desformula->szFormulaParam)[k].szParamName
	   	  	  >>(desformula->szFormulaParam)[k].szParamType
	   	  	  >>(desformula->szFormulaParam)[k].szParamValue;
	   	}

	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return 0;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		//cout<<e.what()<<endl;
  		theJSLog << "SearchRate查找批价规则出错" << endi;
  		throw jsexcp::CException(0, "SearchRate查找批价规则出错", __FILE__, __LINE__);
  		conn.close();
  		return 0;
     }
}
