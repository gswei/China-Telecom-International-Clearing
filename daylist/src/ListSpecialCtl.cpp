#include "ListSpecialCtl.h"
using namespace std;
using namespace tpss;
CList_Special_CTL::CList_Special_CTL()
{
	m_list_special.clear();
	m_varname_value.clear();
}

CList_Special_CTL::~CList_Special_CTL()
{
	m_list_special.clear();
	m_varname_value.clear();
}

//从LIST_DEFINE表的信息到m_list_special中,并初始化m_varname_value
void CList_Special_CTL::Init(char *ListConfigID,char *szInputFiletypeId)
{
	vector<int> v_list_config_id;
    char tmp[5];
    char cListConfigID[200];
    strcpy(cListConfigID,ListConfigID);
	char *p1=cListConfigID;
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
	int config_count=v_list_config_id.size();
	
	//CBindSQL ds( DBConn );
	DBConnection conn;//数据库连接
	char sqlGetAll[SQL_LEN+1];		
	string list_id;
	string express;
	int priority=0;
	sprintf(sqlGetAll,"select list_id,condiction from C_LIST_DEFINE where list_config_id in(");
	for(int i=0;i<config_count;i++)
		{
		sprintf(tmp,"%d,",v_list_config_id[i]);
		strcat(sqlGetAll,tmp);
		}
	sqlGetAll[strlen(sqlGetAll)-1]=')';
	strcat(sqlGetAll," order by priority");
	
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sqlGetAll);			
			stmt.execute();		
    	while( stmt>>list_id>>express) 
    		{			
    		Express_Ctl curr_express;
    		curr_express.v_varName.clear();
    		sprintf(curr_express.contex,"%s",express.c_str());
    		delSpace(curr_express.contex,0);
    		char tmp[1024]="  ";
    		strcat(tmp,curr_express.contex);
    		strcpy(curr_express.contex,tmp);
    		curr_express.list_id=list_id;
    		GetVariable(curr_express.contex,curr_express.v_varName);   
    		priority++;
    		m_list_special[priority]=curr_express;
    		
    		}		
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  //return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "从LIST_DEFINE表的信息到m_list_special中 出错" << endi;
  		throw jsexcp::CException(0, "从LIST_DEFINE表的信息到m_list_special中 出错", __FILE__, __LINE__);
  		conn.close();
  		//return false;
   } 
	
	DefineVariable();
	PrintMe();
}

void CList_Special_CTL::GetVariable(char *contex, vector<string> &v_var)
{
	char *p_begin,*p_end1,*p_end2,*p_end,*point=contex;
	char c_tmp[50];
	int i_tmp;			
	while((p_begin=strchr(point,'$'))!=NULL)
		{
		p_end1=strchr(p_begin,',');
		p_end2=strchr(p_begin,')');
		if(p_end1!=NULL && p_end2!=NULL)
			p_end=(p_end1 < p_end2 ? p_end1 : p_end2);					
		else if(p_end1!=NULL)
			p_end=p_end1;
		else if(p_end2!=NULL)
			p_end=p_end2;		
		strncpy(c_tmp,p_begin+1,p_end-p_begin-1);
		c_tmp[p_end-p_begin-1]=0;		
		//i_tmp=atoi(c_tmp);
		m_varname_value[c_tmp].c_value[0]=0;
		v_var.push_back(c_tmp);
		
	  	point=p_end;
		}

}

//设置变量值
int CList_Special_CTL::DefineVariable()
{
	char tmpstr[250];
	for (map<int ,Express_Ctl>::iterator outMap  = m_list_special.begin();outMap  != m_list_special.end();outMap ++)
	{
		for(int var_count=0;var_count < outMap->second.v_varName.size();var_count++)
			{
			char varname[50];
			sprintf(varname,"%s",outMap->second.v_varName[var_count]);
			if(!outMap->second.theCompiler.DefineVariable(varname,m_varname_value[outMap->second.v_varName[var_count]].c_value))
    			{
    			sprintf(tmpstr,"Define Variable %s error.",outMap->second.v_varName[var_count]);
    			throw CException(ERROR_EXPRESS,tmpstr,__FILE__,__LINE__);
			//throw CException(ERR_GET_ENV,szLogStr,__FILE__,__LINE__);
    			return -1;
    			} 
			}
	}
	return 0;
}
char* CList_Special_CTL::Operation(CFmt_Change &inrcd,char * ret)
{
	//PrintMe();
	//为表达式所有变量赋值
	for (map<string, strct_VarValue>::iterator outMap = m_varname_value.begin();
				 outMap != m_varname_value.end(); outMap++)
	{				
		inrcd.Get_Field( outMap->first.c_str(), outMap->second.c_value );
		//cout<<outMap->first<<":"<<outMap->second.c_value;
	}
				 
	char Result[255] = "";
  	int ErrorNo=0;
  	char TmpLogMsg[250];
  	//string ret="null";
	for (map<int ,Express_Ctl>::iterator outMap  = m_list_special.begin();outMap  != m_list_special.end();outMap ++)
		{
		//cout<<"Operation current list:"<<outMap->second.list_id<<endl;
  		outMap->second.theCompiler.Operation(Result, sizeof(Result)-1, &ErrorNo,outMap->second.contex);
  		if(ErrorNo==0 && strcmp(Result,"false")==0)
  			continue;
  		else if(ErrorNo!=0)
  			{
  			sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  			throw CException(ERROR_EXPRESS,TmpLogMsg,__FILE__,__LINE__);
  			}
  		else
  			{
  			strcpy(ret,outMap->second.list_id.c_str());
  			//cout<<"special list:"<<ret<<endl;
  			break;
  			}  			
		}
	
	return ret;
}

void CList_Special_CTL::PrintMe()
{
	cout<<"print CList_Special_CTL------------------------"<<endl;
	map<string, strct_VarValue>::iterator it;
	cout<<"var list-------"<<endl;
	for(it=m_varname_value.begin();it!=m_varname_value.end();it++)
		{
		cout<<"var:"<<it->first<<endl;
		}
	for (map<int ,Express_Ctl>::iterator outMap  = m_list_special.begin();outMap  != m_list_special.end();outMap ++)
		{
		cout<<"list_id:"<<outMap->second.list_id<<"----"<<endl;
		cout<<"express:"<<outMap->second.contex<<endl;
		for(int var_count=0;var_count < outMap->second.v_varName.size();var_count++)
			{
			cout<<"var "<<var_count<<":"<<outMap->second.v_varName[var_count]<<endl;			
			}
		cout<<"------------"<<endl;
		}
	cout<<"--------------------------------------------"<<endl;
}


