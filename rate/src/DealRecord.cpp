/****************************************************************
filename: dealfile.cpp
module:
created by: zou guodong
version: 1.0.0
description:
    话单文件处理函数，对输入参数中指定的文件进行处理，将结果话单
    写入输出文件并登记库表
    返回值：
        0  —— 文件正常处理完毕
        -1 —— 文件处理失败，临时性错误，需将文件状态置为“W”，等待
              下次再重新处理
        -2 —— 文件处理失败，可能是因为“文件不存在”等原因，需将文件
              状态置为“E”，不再处理
        -3 —— 发生严重错误，进程需要退出
        -4 —— 文件处理失败，因为“输出控制表中已存在同名文件”，需将
              文件状态置为“D”，不再处理
        -5 --公式配置错误
****************************************************************/
#include "DealRecord.h"

//extern SParameter Param;

std::map<string,double> formula_value;

int dealRecord(struct SParameter &Param, PacketParser& ps,ResParser& retValue,CF_MemFileO &pAuditFile,CF_MemFileO *pDispFiles ,char *ratecycle)
{
  	//***********************************************************
  	// 定义变量
  	//***********************************************************
  	//std::map<string,double> formula_value;
  	char szDoneTime[DATETIME_LEN+1];
  	int iCount, j;
  	//CBindSQL ds(DBConn);
  	char szSqlTmp[10*SQL_LEN+1];
  	char szCurDatetime[DATETIME_LEN+1];
  	char szLogStr[LOG_MSG_LEN+1];
  	
  	char szField[22];
  	char szDuration[11];
  	int iDuration;
  	int iResult;
  	int iMaxGroupPri;	//最大RateGroup优先级
  	int iLackType;	//无资料话单类型
  	int  iOutrcdCount = 0;
  	
  	SRateGroup rgtmp;
  	SChargeType cttmp;
  	RATESTRUCT ratmp;
  	ADJRATESTRUCT ratmp_adj;
  	FormulaStruct formula;
  	int iMeterTotal,iUnitTotal;
  	int iFeeA,iFeeB,iFeeA_Add,iFeeB_Add;
  	int iRateA,iRateB,iRateA_Add,iRateB_Add;
  	int iFormFeeA,iFormFeeB;
  	int iFormA,iFormB; 	
  	
 		///clock_t ckTotal, ckRule, ckRateAll, ckRate, ckAdjRate, ckRateOther;
  	///clock_t ckRateGroup, ckSum;
  	///clock_t ckTemp1, ckTemp2, ckTemp3;
  	///clock_t ckIoRead, ckIoWrite, ckOther;
  	
  	///ckTotal = clock();
  	
  	//***********************************************************
  	// 初始化一些对象和变量
  	//***********************************************************
  	
  	
		///ckTemp1 = clock();
    ///ckIoRead += clock() - ckTemp1;
   	///ckTemp1 = clock();
   	char szRuleFieldAll[CALCFEE_RULE_LEN_MAX];
   	char szRuleField[RATE_MAX_RULEITEM_LEN];

	char szRuleFieldMatchModeAll[CALCFEE_RULE_LEN_MAX];
   	char szRuleFieldMatchMode[RATE_MAX_RULEITEM_LEN];
	
   	bool bRuleIsNull = false;
		iResult = 0; 
			
  		/* 这个出来的结果(iResult):0表示正确结果
	    	*                   					   2代表无资料
	    	* 先把要分捡的分出来,然后再写入正常结果话单和无资料话单
	    	*/
				
				//add by liuw 2007-7-20 根据表达式判断话单是否要批价，如果不批，直接跳至批价后分检。
	      Param.iRateFlag = 1;
    		iOutrcdCount = 0;

    		szRuleFieldAll[0] = 0x00;
		
    		//cout << "Param.iRuleFieldCount " <<Param.iRuleFieldCount << endl;
    		for (int i=0; i<Param.iRuleFieldCount; i++)
    		{
      		       int iLen;
			szRuleFieldMatchMode[0] = 0x00;
    			ps.m_inRcd.Get_Field(Param.pRuleFieldIndex[i],szRuleField);
    			if ((strcmp(szRuleField, "") == 0) && (Param.pRuleFieldIsNull[i] == 0))					
					   bRuleIsNull = true;
   	  		
   	  		strcat(szRuleFieldAll, szRuleField);
                  //  cout<<"szRuleFieldAll:"<<szRuleFieldAll<<endl;
			//cout<<"pRuleFieldMatchMode:"<< Param.pRuleFieldMatchMode[i]<<endl;

			sprintf(szRuleFieldMatchMode,"%d",Param.pRuleFieldMatchMode[i]);
			strcat(szRuleFieldMatchModeAll, szRuleFieldMatchMode);
			// cout<<"szRuleFieldMatchModeAll:"<<szRuleFieldMatchModeAll<<endl;
      			if (i<Param.iRuleFieldCount -1)
      			{
        			iLen = strlen(szRuleFieldAll);
        			szRuleFieldAll[iLen] = RATE_RULE_SEPERATOR;
        			szRuleFieldAll[iLen + 1] = 0x00;
					
                            iLen = strlen(szRuleFieldMatchModeAll);
				szRuleFieldMatchModeAll[iLen] = RATE_RULE_SEPERATOR;
        			szRuleFieldMatchModeAll[iLen + 1] = 0x00;
      			}
    		}
    		strcpy(Param.rstmp.szRateRule , szRuleFieldAll);
		strcpy(Param.rstmp.szRuleFieldMatchMode, szRuleFieldMatchModeAll);
		
        //      cout << "Param.rstmp.szRateRule" <<Param.rstmp.szRateRule<<endl;
	     //  cout << "Param.rstmp.szRuleFieldMatchMode" <<Param.rstmp.szRuleFieldMatchMode<<endl;
	//     cout << "Param.iTimeIndex  " <<Param.iTimeIndex<<endl;
    		ps.m_inRcd.Get_Field(Param.iTimeIndex, szDoneTime);
    		sprintf(Param.rstmp.szStartTime,"%s",szDoneTime);
    	//	cout << "szDoneTime  " <<szDoneTime<<endl; 

    		///ckOther += clock() - ckTemp1;
    		//***********************************************************
    		// 在这里添加代码对话单进行处理并置iResult的值
    		//***********************************************************
    		bool bExactRet, bBlurRet;
    		int iRuleRet, iRateRet, iAdjRet;

    		if ((bRuleIsNull == false) &&(strcmp(Param.rstmp.szStartTime,"")!=0))
    		{
      		///ckTemp1 = clock();
      		
          //		cout<<"bRuleIsNull is false"<<endl;		
      		if (Param.bUseRuleMatch == true)
      			{
      				int iRet1, iRet2;
				//	cout<<"first rule match"<<endl;
              SRuleStruct rsdebug;
              iRet1 = SearchRuleBlur(Param.szDebugFlag, Param.rsblur, Param.iBlurCount, Param.rstmp);
              if(iRet1!=0)
              	{
              	    //cout << "iRet1!=0" <<endl;
              		//theJSLog<<"RuleBlur search arithmetic error!"<<endd;
					bExactRet = false;
          			//	return -3;
              	}
			 else
			 	bExactRet == true;
              	/*
        			bExactRet = Param.rule_exact->searchRule(Param.rstmp);
        			
        			if (Param.bUseMultiFind == true)
        				{
          				int iRet1, iRet2;
          				SRuleStruct rsdebug;
          				strcpy(rsdebug.szRateRule , szRuleFieldAll);
          				sprintf(rsdebug.szStartTime, "%s", szDoneTime);
          				iRet1 = SearchRuleBlur(Param.szDebugFlag, Param.rsblur, Param.iBlurCount, rsdebug);
          				if ((bExactRet == true) && (iRet1 == 0))
          				{
          						if ((strcmp(Param.rstmp.szStartTime, rsdebug.szStartTime)!=0) ||
             						(strcmp(Param.rstmp.szEndTime, rsdebug.szEndTime)!=0) ||
             						(Param.rstmp.iRuleNo != rsdebug.iRuleNo) ||
             						(strcmp(Param.rstmp.szRateGroupId, rsdebug.szRateGroupId)!=0))
          						{
            						iRet2 = -1;
          						}
          						else
          						{
            						iRet2 = 0;
          						}
          				}
          				else if ((bExactRet == true) && (iRet1 != 0))
          				{
          					iRet2 = -1;
          				}
          				else
          				{
          					iRet2 = 0;
          				}
              	
          				if (iRet2 != 0)
          				{
          						theJSLog<<"Condition:"<<endd;
          						PrintRuleCondition(Param.szDebugFlag, Param.rstmp, szDoneTime);
          						if (iRet1 == 0)
          						{
            						theJSLog<<"From Traverse:"<<endd;
            						PrintRuleResult(Param.szDebugFlag, rsdebug);
          						}
          						else
          						{
           							theJSLog<<"Not found in Traverse!"<<endd;
          						}
              	 		
          						if (bExactRet == true)
          						{
            						theJSLog<<"From Exact:"<<endd;
            						PrintRuleResult(Param.szDebugFlag, Param.rstmp);
          						}
          						else
          						{
            						theJSLog<<"Not found in Exact!"<<endd;
          						}
              	 		
          						theJSLog<<"RuleExact search arithmetic error!"<<endd;
          						return -3;
          				}
        				}
        				*/
      			}	//end if (Param.bUseRuleMatch == true)
      		else
      			{
       				bExactRet = false;
      			}
          
      		if (bExactRet == true)
      			{
        				//found
        				iRuleRet = 0;
      			}
      		else
						{
							/* 
							if SearchRuleBlur(Param.szDebugFlag, Param.rsblur, Param.iBlurCount, Param.rstmp) == 0)
							   lurRet = true;
							el 
							   lurRet = false;
							*/ 		
						//	cout<<"second rule match"<<endl;
							bBlurRet = Param.rule_blur->searchCdr(Param.rstmp);
						//	cout<<"bBlurRet"<<bBlurRet<<endl;
							int iRet1, iRet2;
							if (Param.bUseMultiFind == true)
							{								
								SRuleStruct rsdebug;
								strcpy(rsdebug.szRateRule , szRuleFieldAll);
								sprintf(rsdebug.szStartTime, "%s", szDoneTime);
							//	cout<<"second muti rule match"<<endl;
								iRet1 = SearchRuleBlur(Param.szDebugFlag, Param.rsblur, Param.iBlurCount, rsdebug);
								if ((bBlurRet == true) && (iRet1 == 0))
								{
										if ((strcmp(Param.rstmp.szStartTime, rsdebug.szStartTime)!=0) ||
						  					(strcmp(Param.rstmp.szEndTime, rsdebug.szEndTime)!=0) ||
						  					(Param.rstmp.iRuleNo != rsdebug.iRuleNo) ||
						  					(strcmp(Param.rstmp.szRateGroupId, rsdebug.szRateGroupId)!=0))
										{
						 					iRet2 = -1;
										}
										else
										{
						 					iRet2 = 0;
										}
								}
								else if ((bBlurRet == false) && (iRet1 != 0))
								{
										iRet2 = 0;
								}
								else
								{
									iRet2 = -1;
								}
						
								if (iRet2 != 0)
								{
										theJSLog<<"Condition:"<<endd;
										PrintRuleCondition(Param.szDebugFlag, Param.rstmp, szDoneTime);
										if (iRet1 == 0)
										{
						 					theJSLog<<"From Traverse:"<<endd;
						 					PrintRuleResult(Param.szDebugFlag, rsdebug);
										}
										else
										{
						 					theJSLog<<"Not found in Traverse!"<<endd;
										}
						
										if (bBlurRet == true)
										{
						 					theJSLog<<"From Blur:"<<endd;
						 					PrintRuleResult(Param.szDebugFlag, Param.rstmp);
										}
										else
										{
						 					theJSLog<<"Not found in Blur!"<<endd;
										}

						                //cout << "iRet2 != 0" <<endl;
										//theJSLog<<"RuleBlur search arithmetic error!"<<endd;
										//return -3;
										iRuleRet = -1;
									}
								
								if(iRet2==0)
									iRuleRet = 0;
								else
									iRuleRet = -1;
								
							}else{
								if(bBlurRet)
									iRuleRet = 0;
								else
									iRuleRet = -1;
							}
								
						   //cout << "bBlurRet " <<bBlurRet << endl;
							
								/*
							if (bBlurRet == true)
								{  
									if(Param.bUseRuleMatch == true)
									{
										bool bRet;
										strcpy(Param.rstmp.szUpdateTime, "");
										bRet = Param.rule_exact->collectRule(Param.rstmp);
										if (bRet != true)
										{
											jsexcp::CException e(RATE_ERR_COLLECT_RULE, "collectRule错误", __FILE__, __LINE__);
											throw e; 								//此处可能需要完善,需根据不同错误返回不同错误码
										}
									}
									iRuleRet = 0;
								}
							else
								{
									iRuleRet = -1;
								}
								*/
						}
						///ckRule += clock() - ckTemp1;
				     //   cout << "iRuleRet " <<iRuleRet << endl;

						if(iRuleRet !=0 && Param.bUseDefRateRule){
							int iRet1, iRet2;
							SRuleStruct rsdefault;
							iRet1 = SearchRuleBlur(Param.szDebugFlag, Param.defblur, Param.iDefBlurCount, Param.rstmp);
							if(iRet1 == 0)
								iRuleRet = 0;
							
						}
						if (iRuleRet == 0)  //找到规则
						{
							///ckTemp1 = clock();
							sprintf(rgtmp.szRateGroupId, "%s", Param.rstmp.szRateGroupId);
							sprintf(rgtmp.szStartTime, "%s", szDoneTime);
							//cout << "rgtmp.szRateGroupId = " <<rgtmp.szRateGroupId << endl;
							//cout << "rgtmp.szStartTime = " <<rgtmp.szStartTime << endl;
							if(strlen(rgtmp.szStartTime)==6)
							{ 
							   strcat(rgtmp.szStartTime,"01000000");
							}
							//cout << "rgtmp.szStartTime = " <<rgtmp.szStartTime << endl;
							
							iMaxGroupPri = 0;
							int iCurPriGroup = -1;
							int iOldPriGroup = -1;
							bool bFound = true;
							if (Param.rate_group->getFirst(rgtmp) == true)
							{
							   //cout << "getFirst" <<endl;
								do
								{
									if (rgtmp.iPriNo == iOldPriGroup)
					  				  continue;
									if (rgtmp.iPriNo != iCurPriGroup) //切换优先组
									{
					  				if (iOldPriGroup == iCurPriGroup) //首次或正常切换优先组
					  				{
					    					iCurPriGroup = rgtmp.iPriNo;
					  				}
					  				else //上个优先组中都没有匹配到费率
					  				{		  				      
					    					iResult = 2;
					    					iLackType = -2;
					    					break;
					  				}
									}
                  
                                    cttmp.iChargeType = rgtmp.iChargeType;
									//cout << "cttmp.iChargeType = " << cttmp.iChargeType<<endl;
									if (Param.charge_type->getRecord(cttmp) == false)
									{
										//cout << "Param.charge_type->getRecord(cttmp) false" <<endl;
					  				iResult = 2;
					  				iLackType = -4;
					  				break;
									}
									
			                ////////////////////////////////////////////////////////////////////////////////
    		                        // 如果 配置有公式，添加公式处理内容
    		                        char tmpformula[6] = {'\0'}; 			                   
			                        sprintf(tmpformula, "%s", rgtmp.szFormulaID );
			                        //cout << "rgtmp.szFormulaID = " <<rgtmp.szFormulaID << endl;
    		                        if( strlen(tmpformula) > 0 )  //能查找到公式ID 
    		                         {
    		                            int expno,paramno;
    		                            FormulaExp tmpforexp;
    		                            FormulaParam tmpforparam;
    		                            Formula math_expr;
	                                    double sum = 0;
	                                    char errlog[400];
	                                    string* p_expr = math_expr.GetExpr();
	                                    
	                                    //map<string, double>* p_vars = math_expr.GetVars();
	                                    
    		                           sprintf(formula.szFormulaID, "%s", rgtmp.szFormulaID); 

    		                            //int tmpresult = SearchFormula(Param.szDebugFlag,rgtmp.szFormulaID,&formula,expno,paramno,Param.rate_config.szTablenameFormula,Param.rate_config.szTablenameFormulaParam);
	                                    int tmpresult = SearchFormula(Param.szDebugFlag,Param.iFormulacount, Param.szformulastruct, formula);
	                                    expno = formula.expno;
	                                    paramno = formula.paramno;

    		                            // 从公式formula 结构中获取内容进行计算   		                           
    		                           
    		                            for(int i=0;i<expno;i++)
    		                            {
    		                               tmpforexp = formula.szFormulaExp[i];
    		                               *p_expr = formula.szFormulaExp[i].szFormula; // 具体公式表达式
    		                            //  theJSLog << "szFormula = " << formula.szFormulaExp[i].szFormula <<endd;
    		                               //cout << "math_expr begin" <<endl;
    		                               math_expr.Clearmap();
    		                               map<string, double>* p_vars = math_expr.GetVars();
    		                                //获取变量的值及其来源
	                                    for (int j=0;j<paramno;j++)
	                                    {
	                                       //cout << "paramno begin" <<endl;
	                                       //Param.formula_value.clear();
	                                       string type = formula.szFormulaParam[j].szParamType;
	                                       string name = formula.szFormulaParam[j].szParamName;
	                                       string value = formula.szFormulaParam[j].szParamValue;
	                                      
	                                      char tmpname[60];
	                                      
	                                       if ( strcmp(type.c_str(),"C") == 0 ) { //固定值
	                                            sprintf(&tmpname[0],"%s%s\0","$",name.c_str());                                    	   
	                                       	    
	                                       	    double x=0;
	                                       	    sscanf(value.c_str(), "%lf", &x);
	                                       	    //sprintf(x,"%f",value.c_str());
	                                       	 //  cout << tmpname <<":"<<x<<endl;
	                                       	//  printf("tmpname = %s ,x=%f  ",tmpname,x);
	                                           p_vars->insert(pair<string,double>(string(tmpname),x) );
	                                       	} else if (strcmp(type.c_str(),"R") == 0) { // 数据来自话单	                                       	   
	                                       	   char record_value[14]={0};
	                                       	   //ps.m_inRcd.Get_Field( value.c_str(), record_value );
	                                       	   retValue.m_outRcd.Get_Field( value.c_str(), record_value ); //根据字段名获取字段内容
	                                       	    double x=0;
	                                       	    sscanf(record_value, "%lf", &x);	                                       	    	                                       	    
	                                       	    sprintf(&tmpname[0],"%s%s\0","$",name.c_str());                                    	   
	                                       	   // cout << tmpname <<":"<<x<<endl;
	                                       	   // printf("tmpname = %s ,x=%f  ",tmpname,x);
	                                       	   p_vars->insert(pair<string,double>(string(tmpname),x));	                                       	   
	                                       	} else if (strcmp(type.c_str(),"T") == 0) { //数据来自表格
                                                //先在公式map 中查找一次
                                                DBConnection conn; //数据库连接
	                                        	char tmp_sql[1024]={0};	
	                                        	char dest_sql[1024]={0};	
	                                        	char value_table[14]={0};
	                                        	double x=0;
                                                
                                                int size = formula_value.size();
                                                char dest_value[20];
                                                char find_value[20];
                                              if( size==0 ) //map 没有数据
                                              {
                                                  //theJSLog<< "we do not find the value " << name <<endd; 
                                                  if (dbConnect(conn))
	                                              {
			                                        Statement stmt = conn.createStatement();
			                                        //cout << "value = "<< value <<endl;
			                                        ReplaceStr(value.c_str(),"$RATECYCLE",ratecycle,dest_sql);
			                                        stmt.setSQLString(dest_sql);	
			                                        //cout << "dest_sql = "<< dest_sql <<endl;
			                                        stmt.execute();			                                    
			                                        stmt >> value_table;
			                                        //cout << "value_table = "<< value_table <<endl;
			                                        conn.close();
	                                               }
	                                             
	                                       	     sscanf(value_table, "%lf", &x);	                        
	                                             sprintf(&tmpname[0],"%s%s\0","$",name.c_str());
	                                             //printf("tmpname = %s ,x=%f  ",tmpname,x);
	                                             sprintf(dest_value,"%s%s",formula.szFormulaID,tmpname);
	                                             p_vars->insert(pair<string,double>(string(tmpname),x));
	                                             //cout << "dest_value = " << dest_value << endl;
				                                 formula_value.insert( pair<string,double>(string(dest_value),x) ); 
                                              }
                                              else 
                                              {
                                                map<string ,double >::iterator l_it;  
                                                sprintf(find_value,"%s%s%s",formula.szFormulaID,"$",name);
                                                //cout << "find_value = " << find_value <<endl;
                                                l_it=formula_value.find(find_value); 
                                                
                                               
                                                if(l_it==formula_value.end())  
                                                {
                                                    //theJSLog<<"we do not find the value "<< find_value<<endd; 
                                                  if (dbConnect(conn))
	                                              {
						                                        Statement stmt = conn.createStatement();
						                                        //cout << "value = "<< value <<endl;
						                                        ReplaceStr(value.c_str(),"$RATECYCLE",ratecycle,dest_sql);
						                                        stmt.setSQLString(dest_sql);	
						                                        //cout << "dest_sql = "<< dest_sql <<endl;
						                                        stmt.execute();			                                    
						                                        stmt >> value_table;
						                                        conn.close();
	                                               }
	                                             
			                                       	     sscanf(value_table, "%lf", &x);	                        
			                                             sprintf(&tmpname[0],"%s%s\0","$",name.c_str());
			                                             //printf("tmpname = %s ,x=%f  ",tmpname,x);
			                                             sprintf(dest_value,"%s%s",formula.szFormulaID,tmpname);
			                                             p_vars->insert(pair<string,double>(string(tmpname),x));	                                             
						                                 formula_value.insert( pair<string,double>(string(dest_value),x) ); 
						                                 //Param.formula_value.insert( make_pair(string(tmpname),x) );
                                                }                                                   
                                                else 
                                                {
                                                   x = l_it->second;
                                                   sprintf(&tmpname[0],"%s%s\0","$",name.c_str());
                                                   p_vars->insert(pair<string,double>(string(tmpname),x));
                                                }
                                                //conn.close();
                                              }                                                         	                                  	                                       		                                        		
	                                          	                                             
	                                       	} 

	                                       	else {   //没有配置公式字段内容匹配类型                                  	   
	                                       	   sprintf(errlog,"公式%s 配置错误",formula.szFormulaExp[i].szFormula);
	                                       	   theJSLog.writeLog(LOG_CODE_FORMULA_ERR,errlog);
	                                       	   return -5;
	                                       	}	                                       
	                                       	
	                                     }
	                                    //计算对象值
	                                     int output_type = formula.szFormulaExp[i].output_type;
	                                     int decimal_num = formula.szFormulaExp[i].decimal_num;
	                                     //cout << "output_type = " << output_type << "  decimal_num = " <<decimal_num <<endl;

	                                     if (math_expr.Calc(sum)) {
		                                    //cout << "通过公式计算出的初始值sum = " << sum << endl;
		                                    //printf("通过公式计算出的初始值sum = %f   ",sum);
		                                    char total_value[14]={0}; //最后结果值
		                                    double tmpdecimal=0;
		                                    // 根据配置截取最终数据
		                                    if (output_type == 0)
		                                    {
		                                       sprintf(total_value,"%.6f",sum);
		                                       total_value[strlen(total_value)]='\0';
		                                    } else if(output_type == 1)
		                                    {
		                                        sum = ceil(sum); //向上取整
		                                        sprintf(total_value,"%.0f",sum);
		                                        total_value[strlen(total_value)]='\0';
		                                    } else if(output_type == 2)
		                                    {
		                                        sum = floor(sum);
		                                        sprintf(total_value,"%.0f",sum);
		                                        total_value[strlen(total_value)]='\0';
		                                    } else 
		                                    {
		                                        //cout << "sum = " <<sum<<endl;
		                                        tmpdecimal = math_expr.Round(double(sum),decimal_num);		
		                                        //printf("通过公式计算出的初始值tmpdecimal = %f   ",tmpdecimal);                                
		                                        sprintf(total_value,"%.*lf",decimal_num,tmpdecimal);
                                                //cout << "total_value = " <<total_value<<endl;
                                                //printf("通过公式计算出的初始值total_value = %s   ",total_value);  
		                                        //sprintf(total_value,"%.*lf",decimal_num,sum);
		                                        
		                                        //printf("tmpdecimal = %.*lf",decimal_num,sum);
		                                        total_value[strlen(total_value)]='\0';
		                                        //printf("通过公式计算出的初始值total_value = %s   ",total_value); 
		                                    }		                             		                                    		                                    
		                                    //计算出的最后值                                    
		                                    //cout <<"total_value = " << total_value <<endl ;
		                                    //cout <<"formula.szFormulaExp[i].szSegmentName = " << formula.szFormulaExp[i].szSegmentName <<endl ;
		                                    retValue.m_outRcd.Set_Field(formula.szFormulaExp[i].szSegmentName, total_value);
		                                    //cout << "over" <<endl;
		                                   		                                    
		                                    if(cttmp.iColindexRateFlag > 0)
		                                    {
		                                       retValue.m_outRcd.Set_Field(cttmp.iColindexRateFlag,"1");
		                                     }
	                                     } else {
	                                       sprintf(errlog, "错误记录值为%s",retValue.m_outRcd.Get_record());
		                                   theJSLog.writeLog(LOG_CODE_FORMULA_ERR,errlog);
		                                   return -4;
	                                     }	                                   	                                    
    		                            }    	                            	    		                         
   		                                //p_expr=NULL;    		                            
    		                            if (p_expr)
    		                            {
    		                               delete[] p_expr;
    		                               p_expr = NULL;
    		                            }    		                  
                                        math_expr.Clearmap();
                                        delete[] formula.szFormulaExp;
                                        delete[] formula.szFormulaParam;
    		                         }
    		                        //end if 有公式ID 


    		                     else{
					
									//cttmp.iChargeType = rgtmp.iChargeType;
									//cout << "cttmp.iChargeType = " << cttmp.iChargeType<<endl;
									//if (Param.charge_type->getRecord(cttmp) == false)
									//{
									//	cout << "Param.charge_type->getRecord(cttmp) false" <<endl;
					  			//	iResult = 2;
					  			//	iLackType = -4;
					  			//	break;
									//}
									if (rgtmp.iCalcMode == 0)
									{
					  				int iChargeValue = 0;
					  				char start_time[15];
					  				bool bFirst = true;
					 					bFound = true;
					
					  				iResult = 0;
					  				sprintf(start_time,"%s",szDoneTime);
					  				iMeterTotal=0;
					  				iUnitTotal=0;
							
					  				iFeeA = 0;
					  				iFeeB = 0;
					  				iFeeA_Add = 0;
					  				iFeeB_Add = 0;
					  				iRateA = 0;
					  				iRateB = 0;
					  				iRateA_Add = 0;
					  				iRateB_Add = 0;
					  				ps.m_inRcd.Get_Field(cttmp.iColindexCdrDuration, szDuration);
					  				iDuration = atoi(szDuration);
					                //cout << "iDuration = " << iDuration <<endl;
					  				while (iDuration > 0)
					  				{
					    					///ckTemp2 = clock();
					    					if ((bFirst == true) || ((bFirst == false) && (cttmp.iChargeStyle == 0)))
					    					{
					      						if (iChargeValue != 0)
					        							timeStrAddSecond(start_time, iChargeValue);
					
					      						if (!((bFirst == false) && (CompareRateTime(Param.szDebugFlag, &ratmp, start_time, szDoneTime) == 0)))
					      						//以改为4个参数的CompareRateTime函数（增加一个参数szDoneTime）以提高效率，但有待测试验证
					      						{
					        						int iRet;
					
					        						sprintf(ratmp.tariff_id, "%s", rgtmp.szTariffId);
					        						sprintf(ratmp.start_time, "%s", start_time);
					        						//cout << "ratmp.tariff_id = " <<ratmp.tariff_id<<endl;
					        						//cout << "ratmp.start_time = " <<ratmp.start_time<<endl;
					        						//cout << "Param.szDebugFlag = " <<Param.szDebugFlag<<endl;
					        						//cout << "Param.iRateCount = " <<Param.iRateCount<<endl;
					        						
					        						iRet=SearchRate(Param.szDebugFlag, Param.ratestruct, Param.iRateCount,ratmp);
					                                //cout << "iRet = " <<iRet<<endl;
					                                //cout << "Param.bUseMultiFind = " <<Param.bUseMultiFind<<endl;
					        						if (Param.bUseMultiFind == true)
					        						{
					        							int iRet1,iRet2;
					        							RATESTRUCT radebug;
					        							sprintf(radebug.tariff_id,"%s",rgtmp.szTariffId);
					        							sprintf(radebug.start_time,"%s",start_time);
					        							iRet1 = SearchRate(Param.szDebugFlag, radebug, Param.rate_config.szTablenameRate);
					        							//cout << "iRet = " << iRet<<" iRet1 =" <<iRet1<<endl;
					        							if ((iRet == 0) && (iRet1 == 0))
					        							{
					          								if ((strcmp(ratmp.start_time, radebug.start_time) != 0) ||
					            									(strcmp(ratmp.end_time,radebug.end_time) !=0 ) ||
					            									(strcmp(ratmp.cdrstart_date,radebug.cdrstart_date) != 0) ||
					            									(strcmp(ratmp.cdrend_date,radebug.cdrend_date) !=0 ) ||
					            									(strcmp(ratmp.cdrstart_time,radebug.cdrstart_time) != 0) ||
					            									(strcmp(ratmp.cdrend_time,radebug.cdrend_time) != 0) ||
					            									(ratmp.rate_a != radebug.rate_a) ||
					            									(ratmp.rate_b != radebug.rate_b) ||
					            									(ratmp.rate_add_a != radebug.rate_add_a) ||
					            									(ratmp.rate_add_b != radebug.rate_add_b) ||
					            									(ratmp.meter_count != radebug.meter_count) ||
					            									(ratmp.charge_unit != radebug.charge_unit))
					          								{
					            									iRet2 = -1;
					          								}
					          								else
					          								{
					            									iRet2 = 0;
					          								}
					        							}
					        							else if ((iRet != 0) && (iRet1 != 0))
					        							{
					          								iRet2 = 0;
					        							}
					        							else
					        							{
					         								iRet2 = -1;
					        							}

					        							if (iRet2 != 0)
					        							{
					          								//theJSLog<<"Rate search arithmetic error!"<<ende;
					          								theJSLog.writeLog(LOG_CODE_SEARCHRATE_ERR,"Rate search arithmetic error!");
					          								return -3;
					        							}
					        						}
					
					        						if (iRet < 0)
					        						{
					        							bFound = false;
					        							break;
					        						}
					      						}
					    					}
					    					///cate += clock() - ckTemp2;
					    					
					    					///cemp2 = clock();
					    					sprintf(ratmp_adj.tariff_id,"%s",rgtmp.szTariffId);
					    					sprintf(ratmp_adj.start_time,"%s",start_time);
					    					ratmp_adj.duration_begin = iUnitTotal;
					    					iAdjRet = SearchAdjRate(Param.szDebugFlag, Param.adjratestruct, Param.iAdjRateCount, ratmp_adj);
					
					    					if (Param.bUseMultiFind == true)
					    					{
					      						int iRet1,iRet2;
					      						ADJRATESTRUCT radjdebug;
					     	 						sprintf(radjdebug.tariff_id,"%s",rgtmp.szTariffId);
					      						sprintf(radjdebug.start_time,"%s",start_time);
					      						radjdebug.duration_begin = iUnitTotal;
					      						iRet1 = SearchAdjRate(Param.szDebugFlag, radjdebug, Param.rate_config.szTablenameAdjrate);
					      						if ((iAdjRet == 0) && (iRet1 == 0))
					      						{
					        							if ((strcmp(ratmp_adj.start_time, radjdebug.start_time) != 0) ||
					        								(strcmp(ratmp_adj.end_time,radjdebug.end_time) !=0 ) ||
					        								(ratmp_adj.duration_begin != radjdebug.duration_begin) ||
					        								(ratmp_adj.duration_end != radjdebug.duration_end) ||
					        								(ratmp_adj.rate_a_factor != radjdebug.rate_a_factor) ||
					        								(ratmp_adj.rate_a_base != radjdebug.rate_a_base) ||
					        								(ratmp_adj.rate_b_factor != radjdebug.rate_b_factor) ||
					        								(ratmp_adj.rate_b_base != radjdebug.rate_b_base) ||
					        								(ratmp_adj.rate_add_a_factor != radjdebug.rate_add_a_factor) ||
					        								(ratmp_adj.rate_add_a_base != radjdebug.rate_add_a_base) ||
					        								(ratmp_adj.rate_add_b_factor != radjdebug.rate_add_b_factor) ||
					        								(ratmp_adj.rate_add_b_base != radjdebug.rate_add_b_base) ||
					        								(ratmp_adj.meter_count != radjdebug.meter_count) ||
					        								(ratmp_adj.charge_unit != radjdebug.charge_unit))
					        							{
					        								iRet2 = -1;
					        							}
					        							else
					        							{
					        								iRet2 = 0;
					        							}
					      						}
					      						else if ((iAdjRet != 0) && (iRet1 != 0))
					      						{
					        							iRet2 = 0;
					      						}
					      						else
					      						{
					       							iRet2 = -1;
					      						}
					      						if (iRet2 != 0)
					      						{
					        							//theJSLog<<"AdjRate search arithmetic error!"<<ende;
					        							theJSLog.writeLog(LOG_CODE_SEARCHADJRATE_ERR,"AdjRate search arithmetic error");
					        							return -3;
					      						}
					    					}
					
					    					if (bFirst == true)
					    					{
					      						bFirst = false;
					      						iRateA = ratmp.rate_a;
					      						iRateB = ratmp.rate_b;
					      						iRateA_Add = ratmp.rate_add_a;
					      						iRateB_Add = ratmp.rate_add_b;
					    					}
					
					    					if (iAdjRet >= 0)
					    					{
					      						iChargeValue = ratmp_adj.charge_unit;
					      						iFeeA += ratmp_adj.rate_a_factor * ratmp.rate_a + ratmp_adj.rate_a_base;
					      						iFeeB += ratmp_adj.rate_b_factor * ratmp.rate_b + ratmp_adj.rate_b_base;
					      						iFeeA_Add += ratmp_adj.rate_add_a_factor * ratmp.rate_add_a + ratmp_adj.rate_add_a_base;
					      						iFeeB_Add += ratmp_adj.rate_add_b_factor * ratmp.rate_add_b + ratmp_adj.rate_add_b_base;
					      						iMeterTotal += ratmp_adj.meter_count;
					      						iUnitTotal += iChargeValue;
					    					}
					    					else
					    					{
					      						iChargeValue = ratmp.charge_unit;
					      						iFeeA += ratmp.rate_a;
					      						iFeeB += ratmp.rate_b;
					      						iFeeA_Add += ratmp.rate_add_a;
					      						iFeeB_Add += ratmp.rate_add_b;
					      						iMeterTotal += ratmp.meter_count;
					      						iUnitTotal += iChargeValue;
					    					}
					
					    					iDuration -= iChargeValue;
					    					///ckAdjRate += clock() - ckTemp2;
					  				}
									}
									else if (rgtmp.iCalcMode == 1)
									{
					  				int iChargeValue = 0;
					  				char start_time[15];
					  				char end_time[15];
					  				bool bFirst = true;
										bFound = true;
					
					  				iResult = 0;
					  				sprintf(start_time,"%s",szDoneTime);
					  				iMeterTotal=0;
					  				iUnitTotal=0;
					  				iFeeA = 0;
					  				iFeeB = 0;
					  				iFeeA_Add = 0;
					  				iFeeB_Add = 0;
					  				iRateA = 0;
					  				iRateB = 0;
					  				iRateA_Add = 0;
					  				iRateB_Add = 0;
					  				ps.m_inRcd.Get_Field(cttmp.iColindexCdrDuration, szDuration);
					  				iDuration = atoi(szDuration);
					  				sprintf(end_time,start_time);
					  				timeStrAddSecond(end_time, iDuration);
					
					  				while (iDuration > 0)
					  				{
					    					list<SPendingDuration> lstPending;
					    					SPendingDuration pdtmp;
					
					    					//将话单按天拆分加入list中
					    					while (true)
					    					{
					    						strncpy(pdtmp.szDate, start_time, 8);
					    						pdtmp.szDate[8] = 0;
					    						pdtmp.iWeekly = timeGetWeek(start_time);
					  							strncpy(pdtmp.szBeginTime, start_time + 8, 6);
					   							pdtmp.szBeginTime[6] = 0;
					    						if (strncmp(start_time, end_time, 8) == 0)
					    						{
					    							strncpy(pdtmp.szEndTime, end_time + 8, 6);
					    							pdtmp.szEndTime[6] = 0;
					    							lstPending.push_back(pdtmp);
					    							break;
					    						}
					    						else
					    						{
					    							strcpy(pdtmp.szEndTime, "235959");
					    							lstPending.push_back(pdtmp);
					    							timeStrAddSecond(start_time, 86400);
					    							memset(start_time + 8, '0', 6);
					    						}
					    					}
					
					    					//初始化rate列表
					    					RATESTRUCT *raLast, raCur;
					    					int iRateCur = 0;
					
					    					strcpy(raCur.tariff_id, rgtmp.szTariffId);
					   						if (SearchRate(Param.szDebugFlag, Param.ratestruct, Param.iRateCount,
					       						raCur, &raLast, iRateCur) < 0)
					    					{
					      						bFound = false;
					      						break;
					    					}
					
					   					 //循环读取list中的所有待计算时间段并计算费用
					    					list<SPendingDuration>::iterator pPending;
					    					pPending = lstPending.begin();
					    					while (pPending != lstPending.end())
					    					{
					    						bFound = false;
					    						for (int i=0; i<iRateCur; i++)
					    						{
					    							bool bCross = false;
					    							char szTimeBegin[7], szTimeEnd[7];
					    							char szDateTimeBegin[15], szDateTimeEnd[15];
					
					    							raCur = *(raLast - i);
					    							//判断cdrdate和weekly是否满足条件
					    							if ((strcmp((*pPending).szDate, raCur.cdrstart_date) >= 0) &&
					    								((strcmp((*pPending).szDate, raCur.cdrend_date) <= 0) || (strcmp(raCur.cdrend_date,"") == 0)) &&
					    								((*pPending).iWeekly >= raCur.start_weekly) &&
					    								((*pPending).iWeekly <= raCur.end_weekly))
					    							{
					    								if (strcmp((*pPending).szBeginTime, raCur.cdrstart_time) <= 0)
					        						{
					   			  						if (strcmp((*pPending).szEndTime, raCur.cdrend_time) >= 0)
					   			  						{
					     		   							strcpy(szTimeBegin, raCur.cdrstart_time);
					    		   							strcpy(szTimeEnd, raCur.cdrend_time);
					    		   							bCross = true;
					   			  						}
					   			  						else if (strcmp((*pPending).szEndTime, raCur.cdrstart_time) >= 0)
					   			  						{
					     		   							strcpy(szTimeBegin, raCur.cdrstart_time);
					    		   							strcpy(szTimeEnd, (*pPending).szEndTime);
					    		   							bCross = true;
					   			  						}
					   			  						else
					   			  						{
					   			    							bCross = false;
					   			  						}
					   									}
					
					
					    								if (strcmp((*pPending).szBeginTime, raCur.cdrstart_time) > 0)
					    								{
					          							if (strcmp((*pPending).szBeginTime, raCur.cdrend_time) > 0)
					          							{
					          								bCross = false;
					          							}
					    		  							else if (strcmp((*pPending).szEndTime, raCur.cdrend_time) >= 0)
					          							{
					   			 	  							strcpy(szTimeBegin, (*pPending).szBeginTime);
					   			 	  							strcpy(szTimeEnd, raCur.cdrend_time);
					   			 	  							bCross = true;
					   			  							}
					   			  							else //if ((*pPending).szEndTime < raCur.cdrend_time)
					          							{
					   			 	  							strcpy(szTimeBegin, (*pPending).szBeginTime);
					   			 	  							strcpy(szTimeEnd, (*pPending).szEndTime);
					   			 	  							bCross = true;
					   			  							}
					   									}
					    							}
					    							else
					    							{
					    								bCross = false;
					    							}
					
					    							//判断cdrtime是否满足条件
					    							if (bCross == false)
					    							{
					        								continue;
					    							}
					        					else
					        					{
					  									sprintf(szDateTimeBegin, "%s%s", (*pPending).szDate, szTimeBegin);
					   									sprintf(szDateTimeEnd, "%s%s", (*pPending).szDate, szTimeEnd);
					    								if ((strcmp(szDateTimeBegin, raCur.start_time) <= 0))
					        						{
					   			  						if ((strcmp(raCur.end_time, "") != 0) && (strcmp(szDateTimeEnd, raCur.end_time) >= 0))
					   			 							{
					    		   							strncpy(szDateTimeBegin + 8, raCur.start_time + 8, 6);
					    		   							strncpy(szDateTimeEnd + 8, raCur.end_time + 8, 6);
					    		   							bCross = true;
					   			  						}
					   			  						else if (strcmp(szDateTimeEnd, raCur.start_time) >= 0)
					   			  						{
					     		   							strcpy(szDateTimeBegin, raCur.start_time);
					    		   							bCross = true;
					   			  						}
					   			  						else
					   			  						{
					   			    							bCross = false;
					   			  						}
					   									}
					
					    								if (strcmp(szDateTimeBegin, raCur.start_time) > 0)  //?????????????????????????????
					    								{
					   			  						if ((strcmp(raCur.end_time, "") == 0) ||
					   			     						(strcmp(szDateTimeEnd, raCur.end_time) <= 0))
					          								{
					   			 	  						bCross = true;
					   			  						}
					    		  						else if (strcmp(szDateTimeEnd, raCur.end_time) >= 0)
					          								{
					   			 	  						strncpy(szDateTimeEnd + 8, raCur.end_time + 8, 6);
					   			 	 						bCross = true;
					   			  						}
					   			  						else
					   			  						{
					   			  							bCross = false;
					   			  						}
					   									}
					   								}
					
					   							//判断生效/失效时间是否满足条件
					    							if (bCross == false)
					    							{
					        								continue;
					    							}
					    							else
					    							{
					  									strncpy(szTimeBegin, szDateTimeBegin + 8, 6);
					   									strncpy(szTimeEnd, szDateTimeEnd + 8,6);
					
					        								//计算开始时间对应的费率
					        						if (bFirst == true)
					        						{
					          							iRateA = raCur.rate_a;
					          							iRateB = raCur.rate_b;
					          							iRateA_Add = raCur.rate_add_a;
					          							iRateB_Add = raCur.rate_add_b;
					          							bFirst = false;
					        						}
					        
					        						//折算时长
					        						if ((strcmp(raCur.cdrend_time, szTimeEnd) == 0) && (strcmp(szDateTimeEnd, end_time) != 0))
					          							iChargeValue = timeStr2Time(szDateTimeEnd) - timeStr2Time(szDateTimeBegin) + 1;
					        						else
					          							iChargeValue = timeStr2Time(szDateTimeEnd) - timeStr2Time(szDateTimeBegin);
					        
					        						iFeeA += raCur.rate_a * iChargeValue;
					        						iFeeB += raCur.rate_b * iChargeValue;
					        						iFeeA_Add += raCur.rate_add_a * iChargeValue;
					        						iFeeB_Add += raCur.rate_add_b * iChargeValue;
					        
					        						//将剩余的时间段放进队列里
					        						if (strcmp(szTimeBegin, (*pPending).szBeginTime) > 0)
					        						{
					        							strcpy(pdtmp.szDate, (*pPending).szDate);
					        							pdtmp.iWeekly = (*pPending).iWeekly;
					        							strcpy(pdtmp.szBeginTime, (*pPending).szBeginTime);
					        							strcpy(pdtmp.szEndTime, szTimeBegin);
					        							timeStrAddSecond(pdtmp.szEndTime, -1);
					        						lstPending.push_back(pdtmp);
					        						}
					
					        						if (strcmp(szTimeEnd, (*pPending).szEndTime) < 0)
					        						{
					        							strcpy(pdtmp.szDate, (*pPending).szDate);
					        							pdtmp.iWeekly = (*pPending).iWeekly;
					        							strcpy(pdtmp.szBeginTime, szTimeEnd);
					        							strcpy(pdtmp.szEndTime, (*pPending).szEndTime);
					        							timeStrAddSecond(pdtmp.szBeginTime, 1);
					        							lstPending.push_back(pdtmp);
					        						}
					   									bFound = true;
					        								break;
					    							}
					    						} //end of for (int i=0; i<iRateCur; i++)
					    						if (bFound == false)
					    	  						break; //break while (pPending != lstPending.end())
					    						pPending++;
					    					} //while (pPending != lstPending.end())
					    					if (bFound == false)
					      						break; //break while (iDuration > 0)
					    
					   					 //进位
					    					switch (rgtmp.iCarryMode)
					    					{
					      						case 0:  //取整
					        								iFeeA = (iFeeA / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								iFeeB = (iFeeB / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								iFeeA_Add = (iFeeA_Add / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								iFeeB_Add = (iFeeB_Add / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								break;
					      						case 1:  //四舍五入
					        								iFeeA = int(double(iFeeA) / double(rgtmp.iCarryUnit) + 0.5) * rgtmp.iCarryUnit;
					        								iFeeB = int(double(iFeeB) / double(rgtmp.iCarryUnit) + 0.5) * rgtmp.iCarryUnit;
					        								iFeeA_Add = int(double(iFeeA_Add) / double(rgtmp.iCarryUnit) + 0.5) * rgtmp.iCarryUnit;
					        								iFeeB_Add = int(double(iFeeB_Add) / double(rgtmp.iCarryUnit) + 0.5) * rgtmp.iCarryUnit;
					        								break;
					      						case 2:  //进位
					        								iFeeA = ((iFeeA + rgtmp.iCarryUnit -1) / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								iFeeB = ((iFeeB + rgtmp.iCarryUnit -1) / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								iFeeA_Add = ((iFeeA_Add + rgtmp.iCarryUnit -1) / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								iFeeB_Add = ((iFeeB_Add + rgtmp.iCarryUnit -1) / rgtmp.iCarryUnit) * rgtmp.iCarryUnit;
					        								break;
					      						default:
					        								iFeeA = 0;
					        								iFeeB = 0;
					        								iFeeA_Add = 0;
					        								iFeeB_Add = 0;
					        								break;                  
					    					};
					    
					    					//计算总价
					    					iFeeA = iFeeA * rgtmp.dUnitPrice;
					    					iFeeB = iFeeB * rgtmp.dUnitPrice;
					    					iFeeA_Add = iFeeA_Add * rgtmp.dUnitPrice;
					    					iFeeB_Add = iFeeB_Add * rgtmp.dUnitPrice;
					    					//退出循环
					    					iDuration = 0;
					  				} //end of while (iDuration > 0)
									} //end of if (rgtmp.iCalcMode == 1)
									else if (rgtmp.iCalcMode == 2)
									{
					  				char start_time[15];
					 					bFound = true;
					
					  				iResult = 0;
					  				sprintf(start_time,"%s",szDoneTime);
					  				iMeterTotal=0;
					  				iUnitTotal=0;
					  				iFeeA = 0;
					  				iFeeB = 0;
					  				iFeeA_Add = 0;
					  				iFeeB_Add = 0;
					  				iRateA = 0;
					  				iRateB = 0;
					  				iRateA_Add = 0;
					  				iRateB_Add = 0;
					
					  				int iRet;
					
					  				sprintf(ratmp.tariff_id, "%s", rgtmp.szTariffId);
					  				sprintf(ratmp.start_time, "%s", start_time);
					  				iRet=SearchRate(Param.szDebugFlag, Param.ratestruct, Param.iRateCount,ratmp);
					
					  				if (Param.bUseMultiFind == true)
					  				{
					    					int iRet1,iRet2;
					    					RATESTRUCT radebug;
					    					sprintf(radebug.tariff_id,"%s",rgtmp.szTariffId);
					    					sprintf(radebug.start_time,"%s",start_time);
					    					iRet1 = SearchRate(Param.szDebugFlag, radebug, Param.rate_config.szTablenameRate);
					    					if ((iRet == 0) && (iRet1 == 0))
					    					{
					      					if ((strcmp(ratmp.start_time, radebug.start_time) != 0) ||
					        						(strcmp(ratmp.end_time,radebug.end_time) !=0 ) ||
					        						(strcmp(ratmp.cdrstart_date,radebug.cdrstart_date) != 0) ||
					        						(strcmp(ratmp.cdrend_date,radebug.cdrend_date) !=0 ) ||
					        						(strcmp(ratmp.cdrstart_time,radebug.cdrstart_time) != 0) ||
					        						(strcmp(ratmp.cdrend_time,radebug.cdrend_time) != 0) ||
					        						(ratmp.rate_a != radebug.rate_a) ||
					        						(ratmp.rate_b != radebug.rate_b) ||
					        						(ratmp.rate_add_a != radebug.rate_add_a) ||
					        						(ratmp.rate_add_b != radebug.rate_add_b) ||
					        						(ratmp.meter_count != radebug.meter_count) ||
					        						(ratmp.charge_unit != radebug.charge_unit))
					      					{
					        						iRet2 = -1;
					      					}
					      					else
					      					{
					        						iRet2 = 0;
					      					}
					    					}
					    					else if ((iRet != 0) && (iRet1 != 0))
					    					{
					      						iRet2 = 0;
					    					}
					    					else
					    					{
					     						iRet2 = -1;
					    					}
					
					    					if (iRet2 != 0)
					    					{
					      						//<<"Rate search arithmetic error!"<<ende;
					      						theJSLog.writeLog(LOG_CODE_SEARCHRATE_ERR,"Rate search arithmetic error");
					      						return -3;
					    					}
					  				}
					
					  				if (iRet < 0)
					  				{
					    					bFound = false;
					  				}
					  				else
					  				{
					    					iRateA = ratmp.rate_a;
					    					iRateB = ratmp.rate_b;
					    					iRateA_Add = ratmp.rate_add_a;
					    					iRateB_Add = ratmp.rate_add_b;
					    
					    					iFeeA = ratmp.rate_a;
					    					iFeeB = ratmp.rate_b;
					    					iFeeA_Add = ratmp.rate_add_a;
					    					iFeeB_Add = ratmp.rate_add_b;
					    					iMeterTotal = ratmp.meter_count;
					    					iUnitTotal = ratmp.charge_unit;
					  				}
									} //end of if (rgtmp.iCalcMode == 2)
									//如果查找不到费率，则继续下一个
								
									if (bFound == false)
					  			{
										if(Param.rate_group->getNext(rgtmp) == false)
										{
											iResult = 2;
											iLackType = -2;
										}
										continue;
									}

									sprintf(szField,"%d",iFeeA);
									if (cttmp.iColindexFeeA > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexFeeA,szField,',');
					
									sprintf(szField,"%d",iFeeB);
									if (cttmp.iColindexFeeB > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexFeeB,szField,',');
					
									sprintf(szField,"%d",iFeeA_Add);
									if (cttmp.iColindexFeeAddA > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexFeeAddA,szField,',');
					
									sprintf(szField,"%d",iFeeB_Add);
									if (cttmp.iColindexFeeAddB > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexFeeAddB,szField,',');
					
									sprintf(szField,"%d",iMeterTotal);
									if (cttmp.iColindexMeterCount > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexMeterCount,szField,',');
					
									sprintf(szField,"%d",iUnitTotal);
									if (cttmp.iColindexRateDuration > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRateDuration,szField,',');
					
									sprintf(szField,"%d",Param.rstmp.iRuleNo);
									if (cttmp.iColindexRuleNo > 0)
					  				retValue.m_outRcd.Set_Field(cttmp.iColindexRuleNo,szField);
					
									sprintf(szField,"%s",Param.rstmp.szRateGroupId);
									if (cttmp.iColindexRategroupId > 0)
					  				retValue.m_outRcd.Set_Field(cttmp.iColindexRategroupId,szField);                                   
					
									sprintf(szField,"%s",rgtmp.szTariffId);
									if (cttmp.iColindexTariffId > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexTariffId,szField,',');
					
									sprintf(szField,"%d",rgtmp.iRuleReportA);
									if (cttmp.iColindexRepnoA > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRepnoA,szField,',');
					
									sprintf(szField,"%d",rgtmp.iRuleReportB);
									if (cttmp.iColindexRepnoB > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRepnoB,szField,',');
					
									sprintf(szField,"%d",rgtmp.iRepnoCdrduration);
									if (cttmp.iColindexRepnoCdrduration > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRepnoCdrduration,szField,',');
					
									sprintf(szField,"%d",rgtmp.iRepnoRateduration);
									if (cttmp.iColindexRepnoRateduration > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRepnoRateduration,szField,',');
					
									if (cttmp.iColindexCdrCount > 0)
									{
					  				if (iOldPriGroup == -1)
					    					retValue.m_outRcd.Set_Field(cttmp.iColindexCdrCount,"1");
					  				else if (Param.iOutMode == 1)
					    					retValue.m_outRcd.Set_Field(cttmp.iColindexCdrCount,"0");
									}
					
									if (cttmp.iColindexRepnoCdrcount > 0)
									{
					  				if ((iOldPriGroup == -1) || (Param.iOutMode == 1))
					  				{
					    					sprintf(szField,"%d",rgtmp.iRepnoCdrcount);
					    					retValue.m_outRcd.Set_Field(cttmp.iColindexRepnoCdrcount,szField);
					  				}
									}
					
									sprintf(szField,"%d",iRateA);
									if (cttmp.iColindexRateA > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRateA,szField,',');
					
									sprintf(szField,"%d",iRateB);
									if (cttmp.iColindexRateB > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRateB,szField,',');
					
									sprintf(szField,"%d",iRateA_Add);
									if (cttmp.iColindexRateAddA > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRateAddA,szField,',');
					
									sprintf(szField,"%d",iRateB_Add);
									if (cttmp.iColindexRateAddB > 0)
					  				retValue.m_outRcd.Field_Add(cttmp.iColindexRateAddB,szField,',');
					  				
					  			    retValue.m_outRcd.Set_Field(cttmp.iColindexRateFlag,"1");
					
									//找到费率，记录最后一个优先组的优先级序号
									iOldPriGroup = rgtmp.iPriNo;
									///ckRateOther += clock() - ckTemp2;
    		                     	}
								} while (Param.rate_group->getNext(rgtmp) == true);
									//}
							}													
    		           
							else  //end if (Param.rate_group.getFirst() == true)
							{
									iResult = 2;
									iLackType = -2;
									//retValue.m_outRcd.Set_Field(cttmp.iColindexRateFlag,"0");
									//retValue.setAnaResult(eLackInfo, "-2", "RateGroup");
							}
                        
							
							///ckRateAll += clock() - ckTemp1;
						}	//end if (iRuleRet == 0)
						
						
							else
								{
									
									iResult = 2;
									iLackType = -1;
									//retValue.m_outRcd.Set_Field(cttmp.iColindexRateFlag,"0");
									//retValue.setAnaResult(eLackInfo, "-1", "RateRule");
								}
						///ckRule += clock() - ckTemp1;
						
				}
    		
				else
				{
					iResult = 2;
					iLackType = -3;
					//retValue.m_outRcd.Set_Field(cttmp.iColindexRateFlag,"0");
					//retValue.setAnaResult(eLackInfo, "-3", "");
				}
				//expTrace(Param.szDebugFlag, __FILE__, __LINE__, "I am here3! iResult = %d iretValue.m_outRcdCount = %d",iResult,iretValue.m_outRcdCount);
				
  		

	
  ///ckTemp1 = clock();



  	///ckOther += clock() - ckTemp1;

  	///ckTotal = clock() - ckTotal;
  	//theJSLog<<"iTotalNum = "<<iTotalNum<<"; iRightNum = "<<iRightNum<<"; iLackNum = "<<iLackNum<<"; iPickNum = "<<iPickNum<<";"<<endd;
    ///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "ckTotal=%d_%d;", ckTotal, 10*ckTotal/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "ckIoRead=%d_%d;", ckIoRead, 10*ckIoRead/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "ckIoWrite=%d_%d;", ckIoWrite, 10*ckIoWrite/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "ckRule=%d_%d;", ckRule, 10*ckRule/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "ckRateAll=%d_%d;", ckRateAll, 10*ckRateAll/CLOCKS_PER_SEC);
  	///ckRateGroup = ckRateAll - ckRate - ckAdjRate - ckRateOther;
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "  ckRateGroup=%d_%d;", ckRateGroup, 10*ckRateGroup/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "  ckRate=%d_%d;", ckRate, 10*ckRate/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "  ckAdjRate=%d_%d;", ckAdjRate, 10*ckAdjRate/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "  ckRateOther=%d_%d;", ckRateOther, 10*ckRateOther/CLOCKS_PER_SEC);
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "ckOther=%d_%d;", ckOther, 10*ckOther/CLOCKS_PER_SEC);
  	///ckSum = ckIoRead + ckIoWrite + ckRule + ckRateAll + ckOther;
  	///expTrace(Param.szDebugFlag, __FILE__, __LINE__, "ckLost=%d_%d;", ckTotal - ckSum, 10*(ckTotal - ckSum)/CLOCKS_PER_SEC);
  	return 0;
}

//sSrc 源字符串,sMatchStr 匹配字符串，sReplaceStr 替换字符串
int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr, char *sDest)
{
        int  StringLen;
        char caNewString[1024] = {0};
        char caOldString[1024] = {0};
        char caDestString[1024] = {0};
        char *p1 = NULL;
        int leftLen=0;
        sprintf(caOldString,"%s",sSrc);
        //cout << "sSrc = " << sSrc <<endl;
        //cout << "sMatchStr = " << sMatchStr <<endl;
        //cout << "sReplaceStr = " << sReplaceStr <<endl;
     

        char *FindPos = strstr(caOldString, sMatchStr);  //sSrc 源字符串,sMatchStr 匹配字符串，sReplaceStr 替换字符串
        if( (!FindPos) || (!sMatchStr) )
        {
            strcpy(sDest,sSrc);
            return 0;
        }
             
        p1 = caOldString;
        leftLen = strlen(caOldString);
        //cout << "before while leftLen = " << leftLen <<endl;
        while( FindPos )
        {                
                StringLen = FindPos - p1;
                //cout << "StringLen = " << StringLen <<endl;
                memset(caNewString, 0, sizeof(caNewString));
                strncpy(caNewString, p1, StringLen);
                strcat(caNewString, sReplaceStr);
                caNewString[strlen(caNewString)]='\0';
                strcat(caDestString,caNewString);
                //cout << "caDestString = " << caDestString <<endl;
                //cout << "caNewString = " << caNewString <<endl;
                
                leftLen = leftLen - StringLen - strlen(sMatchStr);
                //cout << "leftLen = " << leftLen <<endl;
                p1 = FindPos + strlen(sMatchStr);
                //strcat(caNewString, FindPos + strlen(sMatchStr));
                //strcpy(caOldString, caNewString);
                FindPos = strstr(p1, sMatchStr);
        }
        memset(caNewString, 0, sizeof(caNewString));
        strncpy(caNewString, p1, leftLen);
        caNewString[strlen(caNewString)]='\0';
        strcat(caDestString,caNewString);
        caDestString[strlen(caDestString)]='\0';
        //cout << " out while caDestString = " << caDestString <<endl;
        //cout << "caNewString = " << caNewString <<endl;        
        strcpy(sDest,caDestString);
        
        return 0;
}

