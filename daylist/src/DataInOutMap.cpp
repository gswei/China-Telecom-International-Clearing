#include "DataInOutMap.h"
using namespace std;
using namespace tpss;
CDataInOutMap::CDataInOutMap() 
{
}

CDataInOutMap::~CDataInOutMap() {
	//printf( "~CDataInOutMap\n" );
	if ( NULL != m_pList ) {
		delete [] m_pList;
	}
	//printf( "end ~CDataInOutMap\n" );
}

void CDataInOutMap::Init(char *szInputFiletypeId) 
{
	strcpy(inputFiletypeId,szInputFiletypeId);
	//m_db = &DBConn;
	int iCount = getOutputFieldCount();
	try
		{
	m_pList = new IN_AND_OUT[ iCount ];
		}
	catch(...)
		{
		char szLogStr[250];
		strcpy(szLogStr, "new IN_AND_OUT fail.");
      	throw CException(ERR_NEW_MEMORY,szLogStr,__FILE__,__LINE__);
		}
	
	m_iListLen = iCount;
	m_iDataCount = 0;	
	getAllData();
}

/***************************************************
description:	得到数据的总条数
input:	        
output:         
return:         数据的总条数		
programmer:	安	坤
date:		2006-01-10
*****************************************************/ 
int CDataInOutMap::getDataCount() {
	return m_iDataCount;	
}

/***************************************************
description:	得到数据的总条数
input:	        
output:         
return:         
		1:	成	功
		0：	列表已满	
programmer:	安	坤
date:		2006-01-10
*****************************************************/ 
int CDataInOutMap::addData( char* data, char fieldType) {
	IN_AND_OUT	*pData = NULL;
	pData = &m_pList[ m_iDataCount ];
	char tmp[5];
	char *p1=data;
	char *p2;
	int seg=0;
	pData->field_type=fieldType;
	//CBindSQL bs( *m_db );
	DBConnection conn;//数据库连接
	int fieldIndex=0;
	char sql[200];
	try{			
      	if (dbConnect(conn))
      	 {
      	 	Statement stmt = conn.createStatement();
      	 	string sql;
      	 	while(1)
      		{
      		p2=strchr(p1,'+');
      		if(p2==NULL)
      			{
      			if(strcmp(p1,"-1")==0)
      				{
      				pData->field_index[seg]=-1;
      				}
      			else
      				{
      				//p1为字段名，要转化成字段序号	     				
        			sql = "select COL_INDEX from C_TXTFILE_FMT where FILETYPE_ID = :FILETYPE_ID and COLNAME=:name";		
        			stmt.setSQLString(sql);
        			stmt << inputFiletypeId<<p1;			
        			stmt.execute();
        			//stmt >> ext_param;
           		
      				//bs.Open("select COL_INDEX from C_TXTFILE_FMT where FILETYPE_ID = :FILETYPE_ID and COLNAME=:name", SELECT_QUERY );			
      				//bs<<inputFiletypeId<<p1;		
      				if (!(stmt>>fieldIndex))
      				    	{
      				    	char szLogStr[100];
      			      		sprintf(szLogStr, "获取字段序号失败，文件类型:%s，字段名:%s!",
      							inputFiletypeId,p1);
      			      		errLog(LEVEL_ERROR, "",ERR_SELECT, szLogStr, __FILE__, __LINE__);
      			      		throw CException(ERR_SELECT,szLogStr,__FILE__,__LINE__);
      				    	}
      				pData->field_index[seg]=fieldIndex;
      				}
      			seg++;
      			break;
      			}
      		else
      			{
      			*p2=0;
      			if(strcmp(p1,"-1")==0)
      				{
      				pData->field_index[seg]=-1;
      				}
      			else
      				{
      					sql = "select COL_INDEX from C_TXTFILE_FMT where FILETYPE_ID = :FILETYPE_ID and COLNAME=:name";		
        			stmt.setSQLString(sql);
        			stmt << inputFiletypeId<<p1;			
        			stmt.execute();
      						
      				if (!(stmt>>fieldIndex))
      				    	{
      				    	char szLogStr[100];
      			      		sprintf(szLogStr, "获取字段序号失败，文件类型:%s，字段名:%s!",
      							inputFiletypeId,p1);
      			      		errLog(LEVEL_ERROR, "",ERR_SELECT, szLogStr, __FILE__, __LINE__);
      			      		throw CException(ERR_SELECT,szLogStr,__FILE__,__LINE__);
      				    	}
      				pData->field_index[seg]=fieldIndex;
      				}
      			seg++;
      			p1=p2+1;
      			}
      
      		}
      	m_iDataCount ++;    			
	
      	 }else{
        	 	cout<<"connect error."<<endl;
        	 	return -1;
      	 }
      	    conn.close();
      	 } catch( SQLException e ) {
        		cout<<e.what()<<endl;
        		theJSLog << "转化成字段序号 出错" << endi;
        		throw jsexcp::CException(0, "转化成字段序号 出错", __FILE__, __LINE__);
        		conn.close();
        		return -1;
   }
                
	//printf("m_iDataCount=%d\n",m_iDataCount);
	return 1;	
}

/***************************************************
description:	清空列表
input:	        
output:         
return:         
programmer:	安	坤
date:		2006-01-10
*****************************************************/ 
void CDataInOutMap::clearList() {
	m_iDataCount = 0;
}

/***************************************************
description:	得到某一费用项（fee_index）对应的组号（group_index）
input:	        groupIndex：	费用项
output:         
		-1:		数据不存在
		其他：		组号
return:         
programmer:	安	坤
date:		2006-01-10
*****************************************************/ 
int CDataInOutMap::getAllData() {
	//CBindSQL bs( *m_db );	
	DBConnection conn;//数据库连接	
	char		cData[50];
	memset( cData, 0, 50 );
	int field_id;
	char field_type;
	char msg[250];
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "SELECT field,field_id,field_type FROM c_list_output_field where INFILE_TYPE_ID=:v ORDER BY field_id";		
			stmt.setSQLString(sql);
			stmt << inputFiletypeId;			
			stmt.execute();			
				
    	int i = 0;
    	while( stmt>>cData>>field_id>>field_type ) 
    	{			
    		if(field_id!=i)
    			{
    			sprintf(msg,"field_id:%d in c_list_output_field is not defined correctly!",field_id);
    			cout<<msg<<endl;
    			throw CException(ERROR_OUTPUTFIELD_DEFINED,msg,__FILE__,__LINE__);
    			}
    		//printf( "round: %d, out: %s\n", i, cData );
    		i ++;
    		if(field_type!='A' && field_type!='D')
    			{
    			sprintf(msg,"err field_type:%s in c_list_output_field !",field_type);
    			cout<<msg<<endl;
    			throw CException(ERROR_OUTPUTFIELDTYPE_DEFINED,msg,__FILE__,__LINE__);
    			}
    		addData( cData, field_type );
    	}
	//bs.Close();
		
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "得到某一费用项（fee_index）对应的组号（group_index） 出错" << endi;
  		throw jsexcp::CException(0, "得到某一费用项（fee_index）对应的组号（group_index） 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
   }     
	return 1;
}

/*
*得到字段数目:
注：要用count，因为每一个重复的都有不同的对应值
*/
int CDataInOutMap::getOutputFieldCount() 
{
	//CBindSQL bs( *m_db );
	DBConnection conn;//数据库连接
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			int iData = 0;	
			string sql = "SELECT COUNT(*) FROM c_list_output_field a where a.INFILE_TYPE_ID=:v";		
			stmt.setSQLString(sql);
			stmt << inputFiletypeId;			
			stmt.execute();
			stmt >> iData;	
			return iData;

	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "获取生产线和实例ID 出错" << endi;
  		throw jsexcp::CException(0, "获取生产线和实例ID 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
     }     	
	
}

void CDataInOutMap::printMe() 
{
	cout<<"-------output field-----------"<<endl;
	cout<<"data_count="<<m_iDataCount<<endl;
	cout<<"m_iListLen="<<m_iListLen<<endl;
	for(int i=0;i<m_iDataCount;i++)
		{
		cout<<"field seq:"<<i<<endl;
		for(int j=0;j<20;j++)
			{
			if(m_pList[i].field_index[j]==0)
				break;
			else
				cout<<m_pList[i].field_index[j]<<"	";
			}
		cout<<endl;
		}
}
/*void CDataInOutMap::mapping( const CInputInfo *inputList, CListInfo *outputList ) {
/*	int iInputCount = 0;
	iInputCount = inputList->m_iListLen;
	//iInputCount = inputList->m_iDataCount;
	int iOutputIndex = 0;
	int fIn = 0, fOut = 0;
	char	cOut[11], cIn[11];
	memset( cOut, 0, 11 );

	for ( int i = 0 ; i < iInputCount ; i ++ ) {
		iOutputIndex = getOutIndex( i );

		if ( -1 == iOutputIndex ) {
			//处理错误日志
		} else {
			fOut = atoi( outputList->m_pList[ iOutputIndex ].field_data );
			fIn = atoi( inputList->m_pList[i].field_data );
			fOut += fIn;
			
			memset( cOut, 0, 11 );
			sprintf( cOut, "%d", fOut );
			memset( outputList->m_pList[ iOutputIndex ].field_data, 0, 11 );
			strcpy( outputList->m_pList[ iOutputIndex ].field_data, cOut );
		}
	}*/
//}

/*
得到输入话单的字段数:
SELECT COUNT( filetype_id ) FROM txtfile_fmt WHERE filetype_id in 
( SELECT filetype_id FROM model_interface WHERE interface_id in 
  ( SELECT input_id FROM workflow WHERE process_id='9' AND workflow_id in 
  	( SELECT workflow_id FROM pipe WHERE pipe_id='LDTGW' ) 
  ) 
)
*/
