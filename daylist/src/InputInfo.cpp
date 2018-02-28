#include "InputInfo.h"
using namespace std;
using namespace tpss;

CInputInfo::CInputInfo() 
{
	m_pList=NULL;
	m_lacalnet_numlen.clear();
	callingFieldIndex=-1;
	locaneFieldIndex=-1;
	before_endtime_flag=0;
	daycycleFieldIndex=-1;
	daycycleKeySeq=-1;
	sourceID[0]=0;
}

CInputInfo::~CInputInfo() 
{
	m_lacalnet_numlen.clear();
	map_file_keySeq_fieldIndex.clear(); 
	map_list_keySeq_fieldIndex.clear();  
	map_file_keySeq_fieldValue.clear();  
	map_list_keySeq_fieldValue.clear();  
	if ( NULL != m_pList ) 
	{
		delete [] m_pList;
	}
	//printf( "~CInputInfo\n" );

	//printf( "end ~CInputInfo\n" );
}

void CInputInfo::Init(char * file_type_id, char *endTime, char *source)
{
	//20070815
	//cout<<"source="<<source<<endl;
	if(source!=NULL)
		strcpy(sourceID,source);
	else
		strcpy(sourceID,"1");
	//cout<<"sourceID="<<sourceID<<endl;
	//200700313
	int j=0;
	//cout<<"set endtime:"<<endTime<<endl;
	for(int i=0;i<strlen(endTime);i++)
		{
		if(endTime[i]!=':')
			{
			dataType_endTime[j]=endTime[i];
			j++;
			}
		}
	dataType_endTime[j]=0;
	//cout<<"dataType_endTime="<<dataType_endTime<<endl;
	
	//初始化本地网号码长度
	 //CBindSQL ds(DBConn);	
	//CBindSQL ds2(DBConn);	
	DBConnection conn;//数据库连接
	 char localnet[10];
	 int numberLength;
	 char tollcode[10];
	 
	 try{			
  	if (dbConnect(conn))
  	 {
  			Statement stmt = conn.createStatement();
  			string tmpsql = "SELECT LOCALNET_ABBR,CODE_LENGTH, TOLLCODE FROM i_localnet ";
  			stmt.setSQLString(tmpsql);		
  			stmt.execute(); 			
  			//ds.Open("SELECT LOCALNET_ABBR,CODE_LENGTH, TOLLCODE FROM i_localnet ");	 
    	 //ds.Execute();
    	 while(stmt>>localnet>>numberLength>>tollcode)
    	 	{
    	 	string s_localnet=localnet;
    	 	strct_localnet tmp_localnet;
    	 	tmp_localnet.number_len=numberLength;
    	 	sprintf(tmp_localnet.toll_code,tollcode);	 	
    	 	m_lacalnet_numlen[s_localnet] = tmp_localnet;
    	 	}  	
    	 	
    	 	//读LIST-MAIN-KEY表信息
      	int key_seq,field_index,col_length,begin,end;
      	char sql[500];
      	char fieldName[50];
      	char name[50];
      	if(strcmp(sourceID,"1")==0)
      		sprintf(sql,"select a.KEY_SEQ,a.FIELD_INDEX_NAME,a.NAME,a.FIELD_BEGIN, a.FIELD_END from c_list_main_key a where a.INFILE_TYPE_ID='%s' and a.KEY_TYPE='F'",file_type_id);
      	else
      		sprintf(sql,"select a.KEY_SEQ,a.FIELD_INDEX_NAME,a.NAME,a.FIELD_BEGIN, a.FIELD_END from c_list_source_main_key a where a.INFILE_TYPE_ID='%s' and a.KEY_TYPE='F' and a.SOURCE_ID = '%s'",file_type_id,source);
      	stmt.setSQLString(sql);		
  			stmt.execute();
      	
      	while(stmt>>key_seq>>fieldName>>name>>begin>>end)
      		{	
      		if(strcmp(fieldName,"-1")==0)
      			{
      			field_index=-1;
      			col_length=0;
      			}
      		else
      			{
      				tmpsql = "select a.COL_LEN,a.COL_INDEX from c_txtfile_fmt a where a.FILETYPE_ID=:v1 and a.COLNAME=:v2";
      				stmt.setSQLString(tmpsql);		
      				stmt<<file_type_id<<fieldName;
  			      stmt.execute();
  			      stmt>>col_length>>field_index;
      			}
      		strctMainKey curMainKey;
      		curMainKey.field_index=field_index;
      		curMainKey.length=col_length;
      		curMainKey.field_begin=begin;
      		curMainKey.field_end=end;
      		map_file_keySeq_fieldIndex[key_seq]=curMainKey;
      
      		if(strcmp(name,"callingno")==0||strcmp(name,"CALLINGNO")==0)
      			{
      			callingFieldIndex=field_index;			      
      			}
      		if(strcmp(name,"localnet")==0||strcmp(name,"LOCALNET")==0)
      			{
      			locaneFieldIndex=field_index;			
      			}
      		if(strcmp(name,"daycycle")==0||strcmp(name,"DAYCYCLE")==0)
      			{
      			daycycleFieldIndex=field_index;	
      			daycycleKeySeq=key_seq;
      			}
      		}
      	
      	if(strcmp(sourceID,"1")==0)
      		sprintf(sql,"select a.KEY_SEQ,a.FIELD_INDEX_NAME,a.NAME,a.FIELD_BEGIN, a.FIELD_END from c_list_main_key a where a.INFILE_TYPE_ID='%s' and a.KEY_TYPE='R'",file_type_id);
      	else
      		sprintf(sql,"select a.KEY_SEQ,a.FIELD_INDEX_NAME,a.NAME,a.FIELD_BEGIN, a.FIELD_END from c_list_source_main_key a where a.INFILE_TYPE_ID='%s' and a.KEY_TYPE='R' and a.SOURCE_ID = '%s'",file_type_id,source);
      	stmt.setSQLString(sql);		
  			stmt.execute();

      	while(stmt>>key_seq>>fieldName>>name>>begin>>end)
      		{	
      		strctMainKey curMainKey;
      		if(strcmp(fieldName,"-1")==0)
      			{
      			col_length=0;//该字段为空
      			field_index=-1;
      			}
      		else
      			{
      				tmpsql = "select a.COL_LEN,a.COL_INDEX from c_txtfile_fmt a where a.FILETYPE_ID=:v1 and a.COLNAME=:v2";
      				stmt.setSQLString(tmpsql);		
      				stmt<<file_type_id<<fieldName;
  			      stmt.execute();
  			      stmt>>col_length>>field_index;
      			}
      		curMainKey.field_index=field_index;
      		curMainKey.length=col_length;
      		curMainKey.field_begin=begin;
      		curMainKey.field_end=end;
      		map_list_keySeq_fieldIndex[key_seq]=curMainKey;
      		if(strcmp(name,"callingno")==0||strcmp(name,"CALLINGNO")==0)
      			callingFieldIndex=field_index;
      		if(strcmp(name,"localnet")==0||strcmp(name,"LOCALNET")==0)
      			locaneFieldIndex=field_index;
      		}

	
  	 }else{
  	 	  cout<<"connect error."<<endl;
  	 	  //return false;
  	 }
  	    conn.close();
  	 } catch( SQLException e ) {
    		cout<<e.what()<<endl;
    		theJSLog << "CInputInfo初始化 出错" << endi;
    		throw jsexcp::CException(0, "CInputInfo初始化 出错", __FILE__, __LINE__);
    		conn.close();
    		//return false;
     } 
	PrintMe();
}

void CInputInfo::setDate()
{
	//%04u%02u%02u%02u%02u%02u,如:20030316184510,小时从第9位开始
	char crrTime[15];
	getCurTime(crrTime);
	strncpy(crrdate,crrTime,8);
	crrdate[8]=0;
	char time[5];
	int i=0;
	for(i=0;i<4;i++)
		{
		time[i]=crrTime[8+i];
		}
	time[4]=0;
	if(atoi(time)<atoi(dataType_endTime))
		before_endtime_flag=1;
	char lastdatetime[15];
	get_preday_time(lastdatetime);
	strncpy(lastdate,lastdatetime,8);
	lastdate[8]=0;
	//cout<<"crrTime:"<<crrTime<<endl;
	//cout<<"dataType_endTime:"<<dataType_endTime<<" sub crrTime:"<<time<<endl;;
	//cout<<"before_endtime_flag="<<before_endtime_flag<<" lastdate:"<<lastdate;
	//cout<<"crrdate="<<crrdate<<endl;
}

void CInputInfo:: PrintMeAll()
{
    printf("print me in CInputInfo:\n");
    cout<<"callingno:"<<callingFieldIndex;
    cout<<"   localnet:"<<locaneFieldIndex;
    cout<<"   s_crr_abnormalListID"<<s_crr_abnormalListID;
    cout<<"   m_cFeeItem:"<<m_cFeeItem<<endl;
    cout<<"file key:"<<endl;
    MAP_keySeq_fieldIndex::iterator it;
    for(it=map_file_keySeq_fieldIndex.begin();it!=map_file_keySeq_fieldIndex.end();it++)
    	{
    	cout<<"key seq:"<<it->first<<",field_index:"<<it->second.field_index<<",value:"<<map_file_keySeq_fieldValue[it->first]<<endl;
    	}
    cout<<"list key:"<<endl;
    for(it=map_list_keySeq_fieldIndex.begin();it!=map_list_keySeq_fieldIndex.end();it++)
    	{
    	cout<<"key seq:"<<it->first<<",field_index:"<<it->second.field_index<<",value:"<<map_list_keySeq_fieldValue[it->first]<<endl;
    	}

   MAP_keySeq_fieldValue::iterator it_value;
   cout<<"file key value:"<<endl;
   for(it_value=map_file_keySeq_fieldValue.begin();it_value!=map_file_keySeq_fieldValue.end();it_value++)
    	{
    	cout<<it_value->first<<":"<<it_value->second<<endl;
    	}
   cout<<"list key value:"<<endl;
   for(it_value=map_list_keySeq_fieldValue.begin();it_value!=map_list_keySeq_fieldValue.end();it_value++)
    	{
    	cout<<it_value->first<<":"<<it_value->second<<endl;
    	}
   
      cout<<"one recore value:-----------------"<<endl;
	for ( int i = 0 ; i < m_iListLen ; i ++ ) 
	{
		cout<<i<<":"<<m_pList[i].i_field_value<<endl;
	}
}
void CInputInfo:: PrintMe()
{
    printf("print me in CInputInfo:\n");
    cout<<"callingno:"<<callingFieldIndex;
    cout<<"   localnet:"<<locaneFieldIndex;
    cout<<"daycycle:"<<daycycleFieldIndex;
    cout<<endl<<"file key:"<<endl;
    MAP_keySeq_fieldIndex::iterator it;
    
    for(it=map_file_keySeq_fieldIndex.begin();it!=map_file_keySeq_fieldIndex.end();it++)
    	{
    	cout<<"key seq:"<<it->first<<",field_index:"<<it->second.field_index<<",field_begin:"<<it->second.field_begin<<",field_end:"<<it->second.field_end<<endl;
    	}
    cout<<"list key:"<<endl;
    for(it=map_list_keySeq_fieldIndex.begin();it!=map_list_keySeq_fieldIndex.end();it++)
    	{
    	cout<<"key seq:"<<it->first<<",field_index:"<<it->second.field_index<<",field_begin:"<<it->second.field_begin<<",field_end:"<<it->second.field_end<<endl;
    	}

}


void CInputInfo::getPureNum() 
{
	string s_localnet=localNet;
	char		cHostNum[100], cAreaNum[10];
	memset( cHostNum, 0, 100 );
	memset( cAreaNum, 0, 10 );
	strcpy( cHostNum, CallingNo );
	char		*pHostNum = cHostNum;
	strcpy( cAreaNum, m_lacalnet_numlen[s_localnet].toll_code );
	
	char		*pAreaNum = cAreaNum;
	if ( '0' == pHostNum[0] ) {
		pHostNum ++;
	}
	
	if ( '0' == pAreaNum[0] ) {
		pAreaNum ++;
	}
	
	int		iCurAreaLen = strlen( pAreaNum );
	
	//判断有无区号
	if ( 0 == strncmp( pAreaNum, pHostNum, iCurAreaLen ) ) 
	{
		//有区号
		pHostNum += iCurAreaLen;
	
		if ( '0' == pHostNum[0] ) 
		{
			pHostNum ++;
		}
		
		//判断剩下的长度
		if ( m_lacalnet_numlen[s_localnet].number_len != strlen( pHostNum ) ) 
		{
			//判断是否双区号
			if ( ( m_lacalnet_numlen[s_localnet].number_len + iCurAreaLen ) == strlen( pHostNum ) ) 
			{
				//是双区号
				pHostNum += iCurAreaLen;	
			}
		}
	}
	//防止超长的情况，截取前面几位	
	//cout<<"****old num:"<<CallingNo<<endl;
	//cout<<"cAreaNum:"<<cAreaNum<<endl;
	memset( CallingNo, 0, strlen( CallingNo ) );	
	strncpy( CallingNo, pHostNum, m_lacalnet_numlen[s_localnet].number_len );
	
	//cout<<"**********getpureNum:"<<CallingNo<<endl;
	
}


//从话单取数据
void CInputInfo:: getRecordValue(CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, 
									string crr_abnormalListID,int ruleno,int fieldCount, int withTollcode_flag)
{

	char		cOut[100];
	m_iListLen=fieldCount;
	char szLogStr[250];
	//m_iDataCount = 0;
	if ( NULL != m_pList ) 
	{
		delete [] m_pList;
	}
	try
		{
		m_pList = new INPUT_FIELD_STRU[ m_iListLen ];
		}
	catch(...)
		{		
		strcpy(szLogStr, "new INPUT_FIELD_STRU fail.");
      	throw CException(ERR_NEW_MEMORY,szLogStr,__FILE__,__LINE__);
		}
	
	s_crr_abnormalListID=crr_abnormalListID;
	sprintf(m_cFeeItem,"%d",ruleno);

	//PrintMe();

	
	//要去区号
	if(withTollcode_flag==0)
		{
		inrcd.Get_Field( locaneFieldIndex, localNet );
		inrcd.Get_Field( callingFieldIndex, CallingNo );
		getPureNum();	
		}
	
	MAP_keySeq_fieldIndex::iterator it;
	char keyValue[10];
	//取文件级关键字值
	//cout<<"file key"<<endl;
    for(it=map_file_keySeq_fieldIndex.begin();it!=map_file_keySeq_fieldIndex.end();it++)
    	{
    	//cout<<it->second.field_index<<endl;
    	inrcd.Get_Field( it->second.field_index,cOut);    	
    	//cout<<cOut<<endl;
    	//如果需要,截取子字段,从0开始计数
    	if(it->second.field_begin!=-1 && it->second.field_end!=-1)
    		{
    		if(it->second.field_end+1>strlen(cOut))
    			{
    			cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
    			sprintf(szLogStr,"key error:field %d  is not long enough!",it->second.field_index);
    			throw CException(ERR_MAINKEY_FIELD,szLogStr,__FILE__,__LINE__);
    			}
    		int seq;
    		for(seq=0;seq<= it->second.field_end-it->second.field_begin;seq++)
    			{
    			keyValue[seq]=cOut[it->second.field_begin+seq];
    			}
    		keyValue[seq]=0;    		
    	 	map_file_keySeq_fieldValue[it->first] = keyValue;
    	 	//cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
    		}
    	else
    		{
    		map_file_keySeq_fieldValue[it->first] = cOut;
    		strcpy(keyValue,cOut);
    		}
    	 if(it->first==daycycleKeySeq)
    	 	{
    	 	//是昨天且当前时间超过截止时间，或是昨天以前的
    	 	//则需要纠正日帐期为今天
    	 	//cout<<"keyValue:"<<keyValue<<endl;
    	 	if((strcmp(lastdate,keyValue)==0 && before_endtime_flag==0)
    	 		|| (strcmp(lastdate,keyValue)!=0 && strcmp(crrdate,keyValue)!=0))
    	 		{
    	 		map_file_keySeq_fieldValue[it->first]=crrdate;
    	 		}
    	 	}
    	}
    
    //取帐单级关键字值
    //cout<<"list key"<<endl;
    for(it=map_list_keySeq_fieldIndex.begin();it!=map_list_keySeq_fieldIndex.end();it++)
    	{
    	//如果field_index=-1，说明此处应为空
    	if(it->second.field_index==-1)
    		{
    		map_list_keySeq_fieldValue[it->first] ="";
		continue;
    		}
	//如果要去区号且此字段为主叫，此时CallingNo为不带区号的主叫号码
	//cout<<it->second.field_index<<endl;
    	if(it->second.field_index==callingFieldIndex && withTollcode_flag==0)
    		{
    		map_list_keySeq_fieldValue[it->first] = CallingNo;
    		//cout<<"---"<<CallingNo<<endl;
    		}
    	else
    		//到话单里取值
    		{
    		inrcd.Get_Field( it->second.field_index, cOut );

    		if(it->second.field_begin!=-1 && it->second.field_end!=-1)
    			{
    			if(it->second.field_end+1>strlen(cOut))
	    			{
	    			cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
	    			sprintf(szLogStr,"key error:field %d  is not long enough!",it->second.field_index);
	    			throw CException(ERR_MAINKEY_FIELD,szLogStr,__FILE__,__LINE__);
	    			}
	    		int seq;
	    		for(seq=0;seq<= it->second.field_end-it->second.field_begin;seq++)
	    			{
	    			keyValue[seq]=cOut[it->second.field_begin+seq];
	    			}
	    		keyValue[seq]=0;	    		
	    	 	map_list_keySeq_fieldValue[it->first] = keyValue;
	    	 	//cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
    			}
    		else
    			map_list_keySeq_fieldValue[it->first] = cOut;
    	 	//cout<<"---"<<cOut<<endl;
    		}
    	
    	}

	//cout<<"one recore value:-----------------"<<endl;
	for ( int i = 0 ; i < m_iListLen ; i ++ ) 
	{
		//cout<<i<<":";
		m_pList[i].i_field_value=0;
		int field=0;
		while(1)
			{			
			if(pInOutInfo->m_pList[i].field_index[field]==0)
				break;
			if(pInOutInfo->m_pList[i].field_index[field]==-1)
				{
				m_pList[i].i_field_value = 1;
				break;
				}
			else if(pInOutInfo->m_pList[i].field_type=='D')
				{
				memset( cOut, 0, 100 );
				inrcd.Get_Field( pInOutInfo->m_pList[i].field_index[field], cOut );
				delSpace(cOut,strlen(cOut));
				m_pList[i].i_field_value+=atoi(cOut);
				field++;
				}
			else if(pInOutInfo->m_pList[i].field_type=='A')
				{
				inrcd.Get_Field( pInOutInfo->m_pList[i].field_index[field], m_pList[i].sz_field_value );
				//cout<<"m_pList[i].sz_field_value="<<m_pList[i].sz_field_value<<endl;
				break;
				}
			}		
		//cout<<m_pList[i].i_field_value<<endl;
	}    
	//cout<<m_iListLen<<" altogether!!!"<<endl;
	//PrintMeAll();
}


//从话单取数据,得到的值是负的，供冲销使用
void CInputInfo:: getRecordValue_undo(CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, 
								string crr_abnormalListID,int ruleno,int fieldCount,int withTollcode_flag)
{
	char		cOut[100];
	char szLogStr[250];
	m_iListLen=fieldCount;
	if ( NULL != m_pList ) 
	{
		delete [] m_pList;
	}
	try
		{
		m_pList = new INPUT_FIELD_STRU[ m_iListLen ];
		}
	catch(...)
		{
		
		strcpy(szLogStr, "new INPUT_FIELD_STRU fail.");
      	throw CException(ERR_NEW_MEMORY,szLogStr,__FILE__,__LINE__);
		}
	
	s_crr_abnormalListID=crr_abnormalListID;
	sprintf(m_cFeeItem,"%d",ruleno);
	/*
	char		cAreaNum[20];
	char		cOut[100], *pCurFieldName;  		    		
	
	//主叫号码
	memset( cOut, 0, 100 );
	inrcd.Get_Field( mainkey_field, cOut );	    		
	strcpy( m_cHostNum, cOut);	
	
	//本地网
	memset( cOut, 0, 100 );
	inrcd.Get_Field( localnet_field, cOut );	    		
	strcpy( m_cLocalNet, cOut );
	*/

	if(withTollcode_flag==0)
		{
		inrcd.Get_Field( locaneFieldIndex, localNet );
		inrcd.Get_Field( callingFieldIndex, CallingNo );
		getPureNum();	
		}


	MAP_keySeq_fieldIndex::iterator it;
	char keyValue[10];
	//取文件级关键字值
	//cout<<"file key"<<endl;
    for(it=map_file_keySeq_fieldIndex.begin();it!=map_file_keySeq_fieldIndex.end();it++)
    	{
    	//cout<<it->second.field_index<<endl;
    	inrcd.Get_Field( it->second.field_index,cOut);    	
    	//如果需要,截取子字段,从0开始计数
    	if(it->second.field_begin!=-1 && it->second.field_end!=-1)
    		{
    		if(it->second.field_end+1>strlen(cOut))
    			{
    			cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
    			sprintf(szLogStr,"key error:field %d  is not long enough!",it->second.field_index);
    			throw CException(ERR_MAINKEY_FIELD,szLogStr,__FILE__,__LINE__);
    			}
    		int seq;
    		for(seq=0;seq<= it->second.field_end-it->second.field_begin;seq++)
    			{
    			keyValue[seq]=cOut[it->second.field_begin+seq];
    			}
    		keyValue[seq]=0;    		
    	 	map_file_keySeq_fieldValue[it->first] = keyValue;
    	 	//cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
    		}
    	else
    		{
    		map_file_keySeq_fieldValue[it->first] = cOut;
    		strcpy(keyValue,cOut);
    		}
    	
    	if(it->first==daycycleKeySeq)
    	 	{
    	 	//是昨天且当前时间超过截止时间，或是昨天以前的
    	 	//则需要纠正日帐期为今天
    	 	if((strcmp(lastdate,keyValue)==0 && before_endtime_flag==0)
    	 		|| (strcmp(lastdate,keyValue)!=0 && strcmp(crrdate,keyValue)!=0))
    	 		{
    	 		map_file_keySeq_fieldValue[it->first]=crrdate;
    	 		}
    	 	}
    	 
    	}
    
    //取帐单级关键字值
    //cout<<"list key"<<endl;
    for(it=map_list_keySeq_fieldIndex.begin();it!=map_list_keySeq_fieldIndex.end();it++)
    	{
		//如果要去区号且此字段为主叫，此时CallingNo为不带区号的主叫号码
		//cout<<it->second.field_index<<endl;
	    	//如果field_index=-1，说明此处应为空
    	if(it->second.field_index==-1)
    		{
    		map_list_keySeq_fieldValue[it->first] ="";
		continue;
    		}
    	if(it->second.field_index==callingFieldIndex && withTollcode_flag==0)
    		{
    		map_list_keySeq_fieldValue[it->first] = CallingNo;
    		//cout<<"---"<<CallingNo<<endl;
    		}
    	else
    		//到话单里取值
    		{
    		inrcd.Get_Field( it->second.field_index, cOut );

    		if(it->second.field_begin!=-1 && it->second.field_end!=-1)
    			{
    			if(it->second.field_end+1>strlen(cOut))
	    			{
	    			cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
	    			sprintf(szLogStr,"key error:field %d  is not long enough!",it->second.field_index);
	    			throw CException(ERR_MAINKEY_FIELD,szLogStr,__FILE__,__LINE__);
	    			}
	    		int seq;
	    		for(seq=0;seq<= it->second.field_end-it->second.field_begin;seq++)
	    			{
	    			keyValue[seq]=cOut[it->second.field_begin+seq];
	    			}
	    		keyValue[seq]=0;	    		
	    	 	map_list_keySeq_fieldValue[it->first] = keyValue;
	    	 	//cout<<"filed_value:"<<cOut<<"|key_value:"<<keyValue<<endl;
    			}
    		else
    			map_list_keySeq_fieldValue[it->first] = cOut;
    	 	//cout<<"---"<<cOut<<endl;
    		}
    	
    	}

	//PrintMe();
	
	

	for ( int i = 0 ; i < m_iListLen ; i ++ ) 
	{
		m_pList[i].i_field_value=0;
		int field=0;
		while(1)
			{			
			if(pInOutInfo->m_pList[i].field_index[field]==0)
				break;
			if(pInOutInfo->m_pList[i].field_index[field]==-1)
				{
				m_pList[i].i_field_value = -1;
				break;
				}
			else if(pInOutInfo->m_pList[i].field_type=='D')
				{
				memset( cOut, 0, 100 );
				inrcd.Get_Field( pInOutInfo->m_pList[i].field_index[field], cOut );
				delSpace(cOut,strlen(cOut));
				m_pList[i].i_field_value+=atoi(cOut);
				field++;
				}
			else if(pInOutInfo->m_pList[i].field_type=='A')
				{
				inrcd.Get_Field( pInOutInfo->m_pList[i].field_index[field], m_pList[i].sz_field_value );
				field++;
				}
			}
	} 
}



//添加数据
int CInputInfo::addData( const char *data, int index ) 
{
	if ( index >= m_iListLen ) 
	{
		return -1;//数据已满
	}
	
	int iData = atoi( data );
	 m_pList[ index ].i_field_value = iData;
	 
	//m_iDataCount ++;
	
	return 1;
}

void CInputInfo::get_preday_time(char* curtime)
/*取得前一天的时间 */
{
  	time_t		time1;
	struct tm	*time2;
	time(&time1);
	time_t timepre = time1-86400;
	time2 = localtime(&timepre);	
	sprintf(curtime, "%4d%02d%02d%02d%02d%02d", time2->tm_year+1900, 
		time2->tm_mon+1, time2->tm_mday, time2->tm_hour, time2->tm_min, time2->tm_sec);

}

//20070816，供不同source需要不同关键字时使用

CSourceInpuInfo::CSourceInpuInfo()
{
	map_source_inputInfo.clear();
	sourceConfigFlag=0;
}

CSourceInpuInfo::~CSourceInpuInfo()
{
	map<string,CInputInfo*> ::iterator it = map_source_inputInfo.begin();
	for(;it!=map_source_inputInfo.end();it++)
		{
		delete it->second;
		}
	map_source_inputInfo.clear();
}


void CSourceInpuInfo::Init(char * file_type_id ,char *endTime,char *sourceGroup)
{
	int config_count=0;
	char sourceID[10];
	
	//CBindSQL ds(DBConn);	
	//CBindSQL SqlStr(DBConn);
	DBConnection conn;//数据库连接	
	char getSourceSql[200];
	
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(getSourceSql,"select source_id from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP ='%s'",sourceGroup);
	    string sql ="select count(*) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v ";
			stmt.setSQLString(sql);
			stmt << file_type_id;			
			stmt.execute();
			stmt >> config_count;

    	//ds.Open("select count(*) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v ");
    	//ds<<file_type_id;
    	//ds.Execute();
    	//ds>>config_count;
    	//ds.Close();
    	if(config_count>0)
    		{
    			stmt.setSQLString(getSourceSql);		
    			stmt.execute();   			
    		//SqlStr.Open(getSourceSql);
    		while(stmt>>sourceID)
    			{		
    			sql = "select count(*) from c_list_source_main_key a where a.source_id=:v ";
    			stmt.setSQLString(sql);
    			stmt<<sourceID;
    			stmt.execute();
    			stmt >> config_count;
    			
    			//ds.Open("select count(*) from c_list_source_main_key a where a.source_id=:v ");
    			//ds<<config_count;
    			if(config_count==0)
    				continue;
    			CInputInfo *crrInputInfo=new CInputInfo();
    			crrInputInfo->Init(file_type_id,endTime,sourceID);
    			map_source_inputInfo[sourceID]=crrInputInfo;
    			}
    		sourceConfigFlag=1;
    		}
    	else
    		{
    		CInputInfo *crrInputInfo=new CInputInfo();
    		crrInputInfo->Init(file_type_id,endTime);
    		map_source_inputInfo["1"]=crrInputInfo;
    		sourceConfigFlag=0;
    		}			
		
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  //return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "CSourceInpuInfo初始化 出错" << endi;
  		throw jsexcp::CException(0, "CSourceInpuInfo初始化 出错", __FILE__, __LINE__);
  		conn.close();
  		//return false;
   }    		
}

CInputInfo* CSourceInpuInfo::getRecordValue(char *source,
									CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, 
									string crr_abnormalListID,int ruleno,int fieldCount, int withTollcode_flag)
{
	string key;
	if(sourceConfigFlag==0)
		key="1";
	else
		key=source;
	map<string,CInputInfo*>::iterator it=map_source_inputInfo.find(key);
	if(it!=map_source_inputInfo.end())
		{
		map_source_inputInfo[key]->getRecordValue(inrcd,pInOutInfo,crr_abnormalListID,ruleno,fieldCount,withTollcode_flag);
		return map_source_inputInfo[key];
		}
	else
		{
		char szLogStr[200];
		sprintf(szLogStr,"source:%s not defined in c_list_source_main_key!",key.c_str());
	    			throw CException(ERR_SOURCE_MAINKEY,szLogStr,__FILE__,__LINE__);
		return NULL;
		}
}

CInputInfo* CSourceInpuInfo::getRecordValue_undo(char *source,
									CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, 
									string crr_abnormalListID,int ruleno,int fieldCount, int withTollcode_flag)
{
	string key;
	if(sourceConfigFlag==0)
		key="1";
	else
		key=source;
	map<string,CInputInfo*>::iterator it=map_source_inputInfo.find(key);
	if(it!=map_source_inputInfo.end())
		{
		map_source_inputInfo[key]->getRecordValue_undo(inrcd,pInOutInfo,crr_abnormalListID,ruleno,fieldCount,withTollcode_flag);
		return map_source_inputInfo[key];
		}
	else
		{
		char szLogStr[200];
		sprintf(szLogStr,"source:%s not defined in c_list_source_main_key!",key.c_str());
	    	throw CException(ERR_SOURCE_MAINKEY,szLogStr,__FILE__,__LINE__);
		return NULL;
		}
}

void CSourceInpuInfo::setDate(char *source ) 
{
	string key;
	if(sourceConfigFlag==0)
		key="1";
	else
		key=source;
	map<string,CInputInfo*>::iterator it=map_source_inputInfo.find(key);
	if(it!=map_source_inputInfo.end())
		map_source_inputInfo[key]->setDate();
	else
		{
		char szLogStr[200];
		sprintf(szLogStr,"source:%s not defined in c_list_source_main_key!",key.c_str());
	    			throw CException(ERR_SOURCE_MAINKEY,szLogStr,__FILE__,__LINE__);
		}
	
}



