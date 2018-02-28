#include "RuleNo.h"
using namespace std;
using namespace tpss;

CRuleno_Condiction::CRuleno_Condiction()
{
	map_listID_rulenoCondiction.clear();	
	m_refered_field.clear();
}

CRuleno_Condiction::~CRuleno_Condiction()
{
	map_listID_rulenoCondiction.clear();
	m_refered_field.clear();
}

//从list_ruleno_define、list_ruleno_condiction表的取规则号信息到map中
void CRuleno_Condiction::Init(char* listConfigID,char *szInputFiletypeId)
{
    char list_config_id[100];
    strcpy(list_config_id,listConfigID);
    vector<int> v_list_config_id;
    char tmp[5];
	char *p1=list_config_id;
	char *p2;	
	while(1)
		{
		p2=strchr(p1,';');
		if(p2==NULL)
			{		
			v_list_config_id.push_back(atoi(p1));			
			break;
			}
		else
			{
			*p2=0;
			v_list_config_id.push_back(atoi(p1));			
			p1=p2+1;
			}
		}
    
	//CBindSQL bs1( DBConn );
	//CBindSQL bs2( DBConn );
	//CBindSQL bs3( DBConn );
	//CBindSQL bs0( DBConn );
	DBConnection conn;//数据库连接
	
	//char sqlGetBillType[SQL_LEN+1];
	char sqlGetListID[SQL_LEN+1];
	char sqlGetRuleNo[SQL_LEN+1];
	char sqlGetRefer[SQL_LEN+1];
	int tmp_ruleno=0;
	char tmp_referfiledname[100];
	int tmp_referfileindex=0;
	char tmp_refervluelist[100];
	char list_id[20];

	m_refered_field.clear();
	int list_type_count=v_list_config_id.size();
	
	try{			
	if (dbConnect(conn))
	 {
  	 	sprintf(sqlGetListID,"SELECT list_id FROM c_list_define where list_config_id in(");
  	  for(int i=0;i<list_type_count;i++)
  		{
  		sprintf(tmp,"%d,",v_list_config_id[i]);
  		strcat(sqlGetListID,tmp);
  		}
  	sqlGetListID[strlen(sqlGetListID)-1]=')'; 	
  	sprintf(sqlGetRuleNo,"select rule_no from c_list_ruleno_define a where a.LIST_ID=:v");
  	
  	//改为查询字段名
  	sprintf(sqlGetRefer,"select refer_field_name, refer_value from c_list_ruleno_condiction where rule_no=:v1");
  	
  	char sqlGetFiledIndex[200];
  	sprintf(sqlGetFiledIndex,"select COL_INDEX from C_TXTFILE_FMT where FILETYPE_ID = :FILETYPE_ID and COLNAME=:name");
  	
  	Statement stmt = conn.createStatement();	
		stmt.setSQLString(sqlGetListID);			
		stmt.execute(); 	
  	//bs0.Open(sqlGetListID);
  	//bs0.Execute();
  	while( stmt>>list_id) 
  		{
  		map_Ruleno_Condiction map_ruleno_condiction;
  		map_ruleno_condiction.clear();
  		
  		stmt.setSQLString(sqlGetRuleNo);	
  		stmt<<list_id;		
		  stmt.execute(); 
  		//bs1.Open(sqlGetRuleNo);
  		//bs1<<list_id;
  		while( stmt>>tmp_ruleno ) 
  			{	
  			if(tmp_ruleno==0 ||tmp_ruleno==-1)
  				continue;		
    		stmt.setSQLString(sqlGetRefer);	
    		stmt<<tmp_ruleno;		
  		  stmt.execute();
		  
  			//bs2.Open(sqlGetRefer, SELECT_QUERY);
  	    //bs2<<tmp_ruleno;
  	    		
  	    		v_Refer_Field_Value curr_ruleno_refer;
  	    		curr_ruleno_refer.clear();
  				
  	    		//把字段名转换成字段序号
  	    		while(stmt>>tmp_referfiledname>>tmp_refervluelist)
  	    			{  
  	    				stmt.setSQLString(sqlGetFiledIndex);	
    	        	stmt<<szInputFiletypeId<<tmp_referfiledname;	
  		          stmt.execute();  	
  	    			//bs3.Open(sqlGetFiledIndex, SELECT_QUERY);
  				//bs3<<szInputFiletypeId<<tmp_referfiledname;
  				      if (!(stmt>>tmp_referfileindex))
  				    	{
  				    	char szLogStr[100];
  			      	sprintf(szLogStr, "获取字段序号失败，文件类型:%s，字段名:%s!",
  							szInputFiletypeId,tmp_referfiledname);
  			      		errLog(LEVEL_ERROR, "",ERR_SELECT, szLogStr, __FILE__, __LINE__);
  			      		throw CException(ERR_SELECT,szLogStr,__FILE__,__LINE__);
  				    	}
  				
  	    			m_refered_field[tmp_referfileindex]=0;
  	    			//printf("tmp_referfileindex:%d  tmp_refervluelist:%s\n",tmp_referfileindex ,tmp_refervluelist );    			
  	    			Refer_Field_Value curr_refer;
  	    			curr_refer.refer_field_index=tmp_referfileindex;
  	    			
  	    			delSpace(tmp_refervluelist,strlen(tmp_refervluelist));
  	    			char *TmpPoint1=tmp_refervluelist;
    				  char *TmpPoint2=NULL;
   				    int ii=0;
    				while((TmpPoint2=strchr(TmpPoint1,';'))!=NULL)
    					{
  					*TmpPoint2=0;
  					Field_Value tmp_field_value;
  					sprintf(tmp_field_value.value,"%s",TmpPoint1);
  					//printf("tmp_field_value.value=%s",tmp_field_value.value);
  					curr_refer.v_refer_field_value.push_back(tmp_field_value);
  					TmpPoint1=TmpPoint2+1;
    					}
    				Field_Value tmp_field_value;
    				sprintf(tmp_field_value.value,"%s",TmpPoint1);
    				curr_refer.v_refer_field_value.push_back(tmp_field_value);
    				//printf("tmp_field_value.value=%s",tmp_field_value.value);
    				curr_ruleno_refer.push_back(curr_refer);
  	    			}
  	    		//bs2.Close();
  	    		map_ruleno_condiction[tmp_ruleno]=curr_ruleno_refer;
  	    		//printf("ruleno:%d\n",tmp_ruleno);
  			}
  		//bs1.Close();
  		map_listID_rulenoCondiction[list_id]=map_ruleno_condiction;
  		}  	
  	//bs0.Close();	
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  //return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "取规则号信息到map中 出错" << endi;
  		throw jsexcp::CException(0, "取规则号信息到map中 出错", __FILE__, __LINE__);
  		conn.close();
  		//return false;
   } 
   
//printf("map_listID_rulenoCondiction.size()=%d\n",map_listID_rulenoCondiction.size());
	//PrintMe();
}

//由话单字段内容获取所属规则号，找不到，返回-1
int CRuleno_Condiction::GetRuleNo(CFmt_Change &inrcd, string list_id)
{
	map_Record_Refer_Value map_refer_value;
	for (map<int ,int>::iterator Map_refer = m_refered_field.begin();
	                           Map_refer != m_refered_field.end();  Map_refer++)
		{
		char tmp_filedvalue[100];
		sprintf(tmp_filedvalue,inrcd.Get_Field(Map_refer->first));
		delSpace(tmp_filedvalue,strlen(tmp_filedvalue));			
		sprintf(map_refer_value[Map_refer->first].value,tmp_filedvalue);
		//cout<<Map_refer->first<<":"<<map_refer_value[Map_refer->first].value<<endl;
	    }
	/*
	printf("looking for mathed ruleno..........................................\n");
	printf("list_id=%s\n",list_id.c_str());
	printf("map_listID_rulenoCondiction[list_id].size()=%d\n",map_listID_rulenoCondiction[list_id].size());
	*/
	//对每一个ruleno
	//for (map<int,v_Refer_Field_Value>::iterator outMap = map_listID_rulenoCondiction[Map->first].begin();outMap != map_listID_rulenoCondiction[Map->first].end();outMap++)
		
	for (map<int ,v_Refer_Field_Value>::iterator outMap = map_listID_rulenoCondiction[list_id].begin();
	       outMap != map_listID_rulenoCondiction[list_id].end();outMap++)
	{
		//printf("current ruleno:%d\n",outMap->first);
		v_Refer_Field_Value v_curr_refer_field_value = outMap->second;
		//printf("%d condiction to match...\n",v_curr_refer_field_value.size());
		int all_refer_match=1;

		//****************************************************************************
		//对当前ruleno的每一个参考字段
		//****************************************************************************
		for(int i=0;i<v_curr_refer_field_value.size();i++)
			{
			int refer_field_match=0;
			//printf("in the %d nd condiction,  %d refer values .....\n",i+1,v_curr_refer_field_value.size());
			//-------------------------------------------
			//对参考字段的每一个参考值
			//---------------------------------------------
			for(int j=0;j<v_curr_refer_field_value[i].v_refer_field_value.size();j++)
				{
				//cout<<"refer value:"<<v_curr_refer_field_value[i].v_refer_field_value[j].value<<endl;
				int len=strlen(v_curr_refer_field_value[i].v_refer_field_value[j].value)-1;
				char sz_record_value[50];
				char sz_comp_value[50];
                 //首先判断是否为~开头
				if(v_curr_refer_field_value[i].v_refer_field_value[j].value[0]=='~')
					{
					//cout<<"~"<<endl;
					if(strlen(map_refer_value[v_curr_refer_field_value[i].refer_field_index].value)>=len)
						{
						//cout<<"to compare..."<<endl;
						strncpy(sz_record_value,map_refer_value[v_curr_refer_field_value[i].refer_field_index].value,len);
						sz_record_value[len]=0;
						for(int pos=0;pos<len;pos++)
							{
							sz_comp_value[pos]=v_curr_refer_field_value[i].v_refer_field_value[j].value[pos+1];
							}
						sz_comp_value[len]=0;
						//cout<<"sz_record_value="<<sz_record_value<<",sz_comp_value="<<sz_comp_value<<endl;
						if(strcmp(sz_record_value,sz_comp_value)!=0)
							{
							refer_field_match=1;
							//停止参考值的搜索
							break;		
							}
						}
					else 
						{
						refer_field_match=1;
						break;
						}
					}
                 //判断是否为*结尾
				if(v_curr_refer_field_value[i].v_refer_field_value[j].value[len]=='*')
					{
					//cout<<"*"<<endl;
					if(strlen(map_refer_value[v_curr_refer_field_value[i].refer_field_index].value)>=len)
						{
						//cout<<"to compare..."<<endl;
						strncpy(sz_record_value,map_refer_value[v_curr_refer_field_value[i].refer_field_index].value,len);
						sz_record_value[len]=0;
						for(int pos=0;pos<len;pos++)
							{
							sz_comp_value[pos]=v_curr_refer_field_value[i].v_refer_field_value[j].value[pos];
							}
						sz_comp_value[len]=0;
						//cout<<"sz_record_value="<<sz_record_value<<",sz_comp_value="<<sz_comp_value<<endl;
						if(strcmp(sz_record_value,sz_comp_value)==0)
							{
							refer_field_match=1;
							//停止参考值的搜索
							break;		
							}
						}
					else if(len==0)//20080703
						{
						refer_field_match=1;
						break;
						}
					}
                 //判断是否相等
				if(strcmp(v_curr_refer_field_value[i].v_refer_field_value[j].value,map_refer_value[v_curr_refer_field_value[i].refer_field_index].value)==0)
					{
					refer_field_match=1;
					//停止参考值的搜索
					break;					
					}				
				}
			//printf("refer_field_match=%d\n",refer_field_match);
			//-------------------------------------------
			//对参考字段的每一个参考值结束
			//-------------------------------------------
			if(refer_field_match==0)
				{
				all_refer_match=0;
				//没有找到匹配的参考值，放弃此ruleno
				break;
				}
			   //else,继续检查其余参考字段
			   else
			   	{
			   	//printf("go on next refer field...\n");
			   	}
			}
		//******************************************************************************
		//对当前ruleno的每一个参考字段检查结束
		//******************************************************************************

		//完全匹配，返回当前ruleno
		if(all_refer_match==1)
			{
			//printf("all match!");
			map_refer_value.clear();
			return outMap->first;
			}
		}
	
	//没找到，返回-1
	map_refer_value.clear();
	return -1;

}

void CRuleno_Condiction::PrintMe()
{
	for(map<string, map_Ruleno_Condiction> ::iterator Map = map_listID_rulenoCondiction.begin();Map!=map_listID_rulenoCondiction.end();Map++)
		{
		 printf("**********************************\n");
		 printf("list_id:%s\n",Map->first.c_str());
		for (map<int,v_Refer_Field_Value>::iterator outMap = map_listID_rulenoCondiction[Map->first].begin();outMap != map_listID_rulenoCondiction[Map->first].end();outMap++)
		{
		   
			printf("ruleno:%d\n",outMap->first);
			v_Refer_Field_Value v_curr_refer_field_value = outMap->second;
			
			for(int i=0;i<v_curr_refer_field_value.size();i++)
				{
				printf("refer field:%d\n",v_curr_refer_field_value[i].refer_field_index);
				//-------------------------------------------
				//输出参考字段的每一个参考值
				//---------------------------------------------
				for(int j=0;j<v_curr_refer_field_value[i].v_refer_field_value.size();j++)
					{
					printf("refer value:%s\n",v_curr_refer_field_value[i].v_refer_field_value[j].value);				
					}
				}
	     }
		}
	printf("**********************************\n");
}


