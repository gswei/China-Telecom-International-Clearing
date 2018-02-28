#include "ExpressCtl.h"
using namespace std;
using namespace tpss;

int sourceflag=0;
CExpress_CTL::CExpress_CTL()
{
	
}

CExpress_CTL::~CExpress_CTL()
{
}

//获取变量
void CExpress_CTL::Init(char *express,char* InputFiletypeId,CFmt_Change &inrecord)
{	
	char errmsg[512];
	sprintf(contex,"  %s",express);	
	//CBindSQL SqlStr( DBConn );
	DBConnection conn;//数据库连接
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();   			   
    	char ColName[50];
    	//取输出格式的字段名、索引号、字段类型
    	string sql = "select COLNAME from C_TXTFILE_FMT where FILETYPE_ID = :FILETYPE_ID ";
		  stmt.setSQLString(sql);
			stmt << InputFiletypeId;			
			stmt.execute();
		
    	while ( stmt>>ColName )
    	{
    		//对动态编译器增加变量
    		theCompiler.DefineVariable( ColName,inrecord.Get_Field(ColName) );
    	}
	
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  //return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "CExpress_CTL::Init 出错" << endi;
  		throw jsexcp::CException(0, "CExpress_CTL::Init 出错", __FILE__, __LINE__);
  		conn.close();
  		//return false;
   }
	
}


char *CExpress_CTL::Operation(char *ResultOut)
{
	//2008-4-10 11:55
	//不可以直接用参数，sizeof对数组参数无效
	  char Result[255] = "";
  	int ErrorNo=0;
  	char TmpLogMsg[250];  		
  
	theCompiler.Operation(Result, sizeof(Result)-1, &ErrorNo,contex);
    if(ErrorNo!=0)
  			{
  			sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  			throw CException(ERROR_EXPRESS,TmpLogMsg,__FILE__,__LINE__);
  			}  		
	//if(strcmp(Result,"true")==0)
		//cout<<"sourceflag="<<sourceflag<<"|contex:"<<contex<<"|Result:"<<Result<<endl;
		strcpy(ResultOut,Result);
	return Result;
}


CSource_Merge_Condiction::CSource_Merge_Condiction()
{
	map_source_mergeCondicion.clear();
}

CSource_Merge_Condiction::~CSource_Merge_Condiction()
{
	map<string,CExpress_CTL*>::iterator it=map_source_mergeCondicion.begin();
	for(;it!=map_source_mergeCondicion.end();it++)
			delete it->second;
	map_source_mergeCondicion.clear();
}

//提取各个source的合帐条件
void CSource_Merge_Condiction::init(char* szInputFiletypeId,CFmt_Change &inrcd, char *pipe)
{
	char sourceid[10],express[500];
	//CBindSQL SqlStr( DBConn );
	//CBindSQL SqlStr2( DBConn );
	DBConnection conn;//数据库连接
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();   			   
    	char getSourceSql[200];	
    	sprintf(getSourceSql,"select source_id from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP ='%s'",pipe);
    	stmt.setSQLString(getSourceSql);		
			stmt.execute();
			
    	//SqlStr2.Open(getSourceSql);
    	while(stmt>>sourceid)
    		{
    			sprintf(getSourceSql,"select a.VAR_VALUE from C_SOURCE_ENV a where a.SOURCE_ID=:v1 and a.VARNAME=:v2 ",sourceid,"DAYLY_LIST_CONDICTION");
    			stmt.setSQLString(getSourceSql);		
			    stmt.execute();
    		//SqlStr.Open("select a.VAR_VALUE from C_SOURCE_ENV a where a.SOURCE_ID=:v1 and a.VARNAME=:v2 ", SELECT_QUERY );
    		//SqlStr<<sourceid<<"DAYLY_LIST_CONDICTION";
    		while ( stmt>> express)
    			{
    			CExpress_CTL *crr_condiction=new CExpress_CTL();
    			crr_condiction->Init(express,szInputFiletypeId,inrcd);
    			map_source_mergeCondicion[sourceid]=crr_condiction;
    			cout<<"condiction:"<<sourceid<<"|"<<express<<endl;
    			}
    	  	//SqlStr.Close();
    		}
    	//SqlStr2.Close();
	
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  //return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "CExpress_CTL::Init 出错" << endi;
  		throw jsexcp::CException(0, "CExpress_CTL::Init 出错", __FILE__, __LINE__);
  		conn.close();
   }   
	printMe();
}
void CSource_Merge_Condiction::printMe()
{
	cout<<"map_source_mergeCondicion.size()="<<map_source_mergeCondicion.size()<<endl;
	map<string,CExpress_CTL*>::iterator it=map_source_mergeCondicion.begin();
	for(;it!=map_source_mergeCondicion.end();it++)
			cout<<it->first<<"|"<<it->second->contex<<endl;
}
//若匹配到条件，则返回条件判断结果，否则返回true
char *CSource_Merge_Condiction::Operation(char *sourceId, char *result)
{
	map<string,CExpress_CTL*>::iterator it;
	//cout<<"sourceId:"<<sourceId<<endl;
	//printMe();

		it=map_source_mergeCondicion.find(string(sourceId));
	
	if(it==map_source_mergeCondicion.end())
		{
		/*
		cout<<"no condiction to match!return true! source:"<<sourceId<<endl;
		for(;it!=map_source_mergeCondicion.end();it++)
			cout<<it->first<<endl;
		*/
		strcpy(result,"true");
		return result;
		}
	else
		{
		//cout<<"to match source condiction !!"<<endl;
		sourceflag=1;
		return it->second->Operation(result);
		}
}




