#include "TreeTool.h"

extern char GvarDebugFlag[10];
//int fileList_count = 0;

int count=0;
int group_count=0;
//extern v_pfeeGoup;


CFeeGroup::CFeeGroup()
{
	for(int i=0;i<50;i++)
		{
		v_fee[i]=0;
		}
	group_count++;
	//cout<<"group_count(++)="<<group_count<<endl;
}
CFeeGroup::~CFeeGroup() 
{
	//cout<<"CFeeGroup::~CFeeGroup() "<<endl;
	
	group_count--;
	//cout<<"group_count(--)="<<group_count<<endl;
}



/****************************************************************************************************/
/*CDataTable::CDataTable()
{
}*/
CDataTable::~CDataTable() 
{	

	//cout<<"CDataTable::~CDataTable() "<<count<<endl;	

	for(int i=0;i<m_iGroupCount;i++)
		{			
		//cout<<"free	map_groupSeq_feeGroup[i]-------"<<map_groupSeq_feeGroup[i]<<endl;		
		delete map_groupSeq_feeGroup[i];
		}
 	
	//cout<<"count="<<count<<",group_count="<<group_count<<endl;
}

CDataTable::CDataTable( int groupCount, int fieldCount ) 
{
	m_iGroupCount = groupCount;
	m_iFieldCount = fieldCount;
	int i=0;
       m_attrData_group_index=-1;
       map_fieldID_attr.clear();

	for(int i=0;i<groupCount;i++)
		{		
		CFeeGroup *p_CFeeGroup;
		try
			{
			p_CFeeGroup = new CFeeGroup();
			}
		catch(...)
			{
			throw CException(ERR_NEW_MEMORY,(char *)"new error!",__FILE__,__LINE__);
			}		
		map_groupSeq_feeGroup[i]=p_CFeeGroup;
		}
		
}

CDataTable::CDataTable( int groupCount, int fieldCount, MAP_keySeq_fieldValue & map_listKey) 
{
	m_iGroupCount = groupCount;
	m_iFieldCount = fieldCount;
	m_key_count=map_listKey.size();
	cout<<"m_iFieldCount="<<m_iFieldCount<<endl;
	cout<<"groupCount="<<groupCount<<endl;
	cout<<"m_key_count="<<m_key_count<<endl;
	m_attrData_group_index=-1;
       map_fieldID_attr.clear();
	   
	int i=0;
		
	//cout<<"groupCount="<<groupCount<<endl;
	for(int i=0;i<groupCount;i++)
		{		
		CFeeGroup *p_CFeeGroup;
		try
			{
			p_CFeeGroup = new CFeeGroup();
			}
		catch(...)
			{
			throw CException(ERR_NEW_MEMORY,(char *)"new error!",__FILE__,__LINE__);
			}		
		map_groupSeq_feeGroup[i]=p_CFeeGroup;
		}
	
	map<int, string>::iterator it=map_listKey.begin();	
	int key_seq=0;
	for(;it!=map_listKey.end();it++)
    	{
    	//maplist_key_value[it->first]=it->second;
    	strcpy(maplist_key_value[key_seq],it->second.c_str());
    	key_seq++;
    	}
    	//PrintMe() ;
}

void CDataTable::PrintMe() 
{
	printf("current datatable--------------------\n");  
   cout<<"list_caption:"<<list_caption<<endl;
    int i=0;
	for(i=0;i<m_key_count;i++)
		{
		cout<<"key seq:"<<i<<",key value:"<<maplist_key_value[i]<<endl;
		}
	
	cout<<"m_iGroupCount="<<m_iGroupCount<<endl;
	for ( int i = 0 ; i < m_iGroupCount ; i ++ ) 
	{
		cout<<"fieldCount["<<i<<"]="<<m_iFieldCount<<endl;
		for ( int j = 0 ; j <m_iFieldCount  ; j ++ ) 
		{
			cout<<"["<<i<<"]["<<j<<"]="<<map_groupSeq_feeGroup[i]->v_fee[j]<<"   ";
		}				
		cout<<endl;
	}
	printf( "datatable end--------------------\n" );
}

//����һ��ListInfo����������
void CDataTable::InputAttrData(  CInputInfo &inputInfo, int groupIndex ) 
{
	m_attrData_group_index=groupIndex;
	string crr_attr;
	map_fieldID_attr.clear();
	for ( int i = 0 ; i < m_iFieldCount ; i ++ ) 
	{
		if(strcmp(inputInfo.m_pList[i].sz_field_value,"@@@")!=0)
			{
			crr_attr=inputInfo.m_pList[i].sz_field_value;
			map_fieldID_attr[i]=crr_attr;
			}
	}
	
}

//��һ��ListInfo�����ݺϲ�����
void CDataTable::InputData(  CInputInfo &inputInfo, int groupIndex ) 
{
	CFeeGroup		*pCurGoup= map_groupSeq_feeGroup[groupIndex] ;
	DEBUG_LOG<<"groupIndex="<<groupIndex<<endd;
	DEBUG_LOG<<"m_iFieldCount="<<m_iFieldCount<<endd;
	//PrintMe();
	for ( int i = 0 ; i < m_iFieldCount ; i ++ ) 
	{
		pCurGoup->v_fee[i] += inputInfo.m_pList[i].i_field_value;
	}
}

//��һ��DataTable�����ݺϲ�����
void CDataTable::InputData( CDataTable *dataTable ) {
	
	CFeeGroup	 *pInTableGroup;
	
	int i = 0, j = 0;
	for ( j = 0 ; j < dataTable->m_iGroupCount ; j ++ ) 
	{
		CFeeGroup		*pCurGoup = map_groupSeq_feeGroup[j];
		pInTableGroup=dataTable->map_groupSeq_feeGroup[j];
		for ( i = 0 ; i < m_iFieldCount ; i ++ ) 
		{
			//cout<<pCurGoup.v_fee[i]<<"     ";
			pCurGoup->v_fee[i]  += pInTableGroup->v_fee[i];
			//cout<<pCurGoup.v_fee[i]<<"   "<<pInTableGroup.v_fee[i]<<endl;
		}
	}
	
	//printf("\n------------------------");
	
}


CListIO::CListIO() 
{
	map_fileType_cfmtChange.clear();
}

CListIO::~CListIO() {
	map<string,CFmt_Change*>::iterator it=map_fileType_cfmtChange.begin();
	for(;it!=map_fileType_cfmtChange.end();it++)
		{
		if(it->second!=NULL)
			delete it->second;
		}
	map_fileType_cfmtChange.clear();
}

void CListIO::Init(char *listfile_type_id,char list_record_type_id)
{
	map<string,CFmt_Change*>::iterator it=map_fileType_cfmtChange.find(listfile_type_id);
	CFmt_Change *p_list_record;
	if(it==map_fileType_cfmtChange.end())
		{
		try
			{
			p_list_record=new CFmt_Change();
			}
		catch(...)
			{
			char szLogStr[100];
			strcpy(szLogStr, "new CFmt_Change fail.");
      			throw CException(ERR_NEW_MEMORY,szLogStr,__FILE__,__LINE__);
			}
		p_list_record->Init(listfile_type_id, list_record_type_id);
		map_fileType_cfmtChange[listfile_type_id]=p_list_record;
		}
}

/***************************************************
description:	ȥ���ַ������ߵĿո�,Tab,�س�
input:		cStr:		Ҫ������ַ���
output:		cStr��		�������ַ���
return:
data:		2005-07-07
*****************************************************/
void CListIO::Trim(char *cStr){
	int iLen = strlen(cStr);
	int iLeft = 0;
	int iRight = iLen;
	
	for (int i=0;i<iLen;i++){
		if (cStr[i] != ' ' && cStr[i] != '	' && cStr[i] != '\n') break;
		iLeft ++;
	}
	if (iLeft == iLen){
		memset(cStr, 0, iLen);
		return;
	}
	
	for (int i=iLen-1;i>=0;i--){
		if (cStr[i] != ' ' && cStr[i] != '	' && cStr[i] != '\n') break;
		iRight = i;			
	}
	if (iLeft == 0 && iRight == iLen) return ;
	char		*pC;

	
	try
		{
		pC = new char[iLen];
		}
	catch(...)
		{
		char szLogStr[250];
		strcpy(szLogStr, "new char fail.");
      		throw CException(ERR_NEW_MEMORY,szLogStr,__FILE__,__LINE__);
		}
	
	memset(pC, 0, sizeof(pC));
	char		*pp;
	pp = cStr;

	pp += iLeft;
	memset(pC, 0, iLen);
	strncpy(pC, pp, iRight-iLeft);
	memset(cStr, 0, iLen);
	strcpy(cStr, pC);
	delete[] pC;
	pC=NULL;

}

void CListIO::writeFormalList( CDataTable &dataTable,CF_MemFileO &_outTmpFile, CFeeGroupMapList &pGlobalMap,CFmt_Change &inrcd,char Write_Type)
{
	char	cData[200];
	memset( cData, 0, 200 );
	int tmp;

	int field;
	map<string,CFmt_Change*>::iterator it=map_fileType_cfmtChange.find(inrcd.Get_id());
	if(it==map_fileType_cfmtChange.end())
		{
		//expTrace(GvarDebugFlag, __FILE__, __LINE__,"error filetype in writeFormalList:%s", inrcd.Get_id());
		theJSLog<<"error filetype in writeFormalList:"<<inrcd.Get_id()<<ende;
		exit(0);
		}
	CFmt_Change &list_record = *(it->second);
       //dataTable.PrintMe();
  //��дһ���ʵ�
  if(Write_Type=='W')
  {
  	/*
	for(field=1;field<=list_record.Get_fieldcount();field++)
		{
		list_record.Set_Field(field,"0");
		}
	*/
	
	//�ʵ����ؼ�����Ϣ	
	//cout<<"dataTable.m_key_count="<<dataTable.m_key_count<<endl;
	for(field=1;field<=dataTable.m_key_count;field++)
		{
		list_record.Set_Field(field,dataTable.maplist_key_value[field-1]);
		//cout<<"list_record.Set_Field(field,dataTable.maplist_key_value[field-1])"<<endl;
		}
	
	//���÷����ֶ�ֵ
	
	int i = 0, j = 0;
	int field_seq=dataTable.m_key_count+1;
	int iLen1 = dataTable.m_iGroupCount;
	
	for ( i = 0 ; i < dataTable.m_iGroupCount ; i ++ ) 
	{		
	
		//��������
		if(dataTable.m_attrData_group_index==i)
			{		
			for(j=0;j<pGlobalMap.m_pList[i].num_list.iNumCount;j++)
				{
				//cout<<"field_seq="<<field_seq<<endl;
				tmp = pGlobalMap.m_pList[i].num_list.iNumList[j];	
				
				//û�и����ԣ�ֱ����Ϊ0 ���
				if(tmp==-1)
					{
					list_record.Set_Field(field_seq,"0");
					//cout<<"cData:"<<"0"<<endl;
					}
				else if(tmp==-2)
					{
					list_record.Set_Field(field_seq,"");
					//cout<<"cData:"<<""<<endl;
					}
				//��
				else
					{
					sprintf( cData, "%s", dataTable.map_fieldID_attr[tmp].c_str());
					//cout<<"cData:"<<cData<<endl;
					list_record.Set_Field(field_seq,cData);
					}
				field_seq++;
				}
			
			}
		//�Ƿ�����
		else 
			{			
			CFeeGroup *p_feeGroup=dataTable.map_groupSeq_feeGroup[i];			
			for(j=0;j<pGlobalMap.m_pList[i].num_list.iNumCount;j++)
				{
				
				tmp = pGlobalMap.m_pList[i].num_list.iNumList[j];	
				//cout<<"field_seq="<<field_seq<<"tmp="<<tmp<<endl;
				//û�и�����ã�ֱ����Ϊ0 ���
				if(tmp==-1)
					{
					list_record.Set_Field(field_seq,"0");
					//cout<<"cData:"<<"0"<<endl;
					}
				else if(tmp==-2)
					{
					list_record.Set_Field(field_seq,"");
					//cout<<"cData:"<<""<<endl;
					}
				//��
				else
					{
					sprintf( cData, "%ld", p_feeGroup->v_fee[tmp] );
					//cout<<"cData:"<<cData<<endl;
					list_record.Set_Field(field_seq,cData);
					
					}
				field_seq++; 
				}	
			}
	}

    _outTmpFile.writeRec(list_record);

  }
  //�ۼӵ�ԭ���ʵ�
  else
  {
    //���÷����ֶ�ֵ
	
	int i = 0, j = 0;
	int field_seq=dataTable.m_key_count+1;
	int iLen1 = dataTable.m_iGroupCount;
	
	for ( i = 0 ; i < dataTable.m_iGroupCount ; i ++ ) 
	{		
		if(dataTable.m_attrData_group_index==i)
			{
			field_seq+=pGlobalMap.m_pList[i].num_list.iNumCount;
			}
		else
			{
			CFeeGroup *p_feeGroup=dataTable.map_groupSeq_feeGroup[i];
			
			for(j=0;j<pGlobalMap.m_pList[i].num_list.iNumCount;j++)
				{
				tmp = pGlobalMap.m_pList[i].num_list.iNumList[j];			
				if (tmp==-1 || tmp==-2)  
					{
					}
				else
					{
					sprintf( cData, "%ld", (p_feeGroup->v_fee[tmp] +atol(inrcd.Get_Field(field_seq))));
					inrcd.Set_Field(field_seq,cData);				
					}
				field_seq++;
				}
			}
	}
    _outTmpFile.writeRec(inrcd);

  }
	
}



//outData�ĸ��б�ĳ����Ѿ�������outData�Լ���
void CListIO::readData( CFmt_Change &inrecord, CDataTable *dataTable ,CFeeGroupMapList &pGlobalMap,int const list_key_count) 
{
	char	cData[1000];
	memset( cData, 0, 1000 );
		
	char	cCurData[100];
	char	*pInData = cData;

	dataTable->m_key_count=list_key_count;
	for(int key_seq=0;key_seq<list_key_count;key_seq++)
		{
		strcpy( dataTable->maplist_key_value[key_seq], inrecord.Get_Field(key_seq+1) );		
		//cout<<"key_seq:"<<key_seq<<",key:"<<dataTable->maplist_key_value[key_seq]<<endl;;
		}
	
	int i = 0, j = 0,tmp;
	int filed_seq=list_key_count+1;
	for ( i = 0 ; i < dataTable->m_iGroupCount ; i ++ ) 
	{
		//printf("\ngroup:%d\n",i);
		CFeeGroup	*pCurGroup = dataTable->map_groupSeq_feeGroup[i];		
		//for ( j = 0 ; j < pGlobalMap.m_groupLenth[i] ; j ++ ) 
		for ( j = 0 ; j < pGlobalMap.m_pList[i].num_list.iNumCount ; j ++ ) 
		{
			tmp = pGlobalMap.m_pList[i].num_list.iNumList[j];	
			memset( cCurData, 0, 100 );
			strncpy( cCurData, pInData, 9 );
			//cout<<"filed_seq="<<filed_seq<<endl;
			pCurGroup->v_fee[tmp] += atoi(inrecord.Get_Field(filed_seq));
			filed_seq++;	
		}	
	}
	//dataTable->PrintMe();
	
	//printf("\n************************\n");
}

void CListIO::toLine() 
{
	
}


CTreeTool::CFileList::CFileList()
{
	curr_size=0;
	//fileList_count++;
}

CTreeTool::CFileList::~CFileList()
{
	//cout<<"CTreeTool::CFileList::~CFileList()"<<endl;
	map_listKey_dataTable.clear();
}

void CTreeTool::CFileList::Init(const MAP_keySeq_fieldValue& map_file_keySeq_fieldValue)
{		
	map_listKey_dataTable.clear();
    map_filekey_value.clear();

    typedef MAP_keySeq_fieldValue::const_iterator CITER;
	const CITER end = map_file_keySeq_fieldValue.end();
	for(CITER it = map_file_keySeq_fieldValue.begin(); it != end ; ++it)
    	map_filekey_value[it->first]=it->second;

}

void CTreeTool::CFileList::ClearDataTables()
{
	 typedef map <string,CDataTable*>::iterator ITER;
	 const ITER end = map_listKey_dataTable.end();
	  for (ITER iter = map_listKey_dataTable.begin(); iter != end; ++iter)
	  {
	  	delete iter->second;
	 
	 	/*	
	 	map<int,CFeeGroup> ::iterator it= iter->second.map_groupSeq_feeGroup.begin();
	 	for(;it!= iter->second.map_groupSeq_feeGroup.end();it++)
	 		it->second.v_fee.resize(0);
	 	 iter->second.map_groupSeq_feeGroup.clear();
	 	*/
	}
	map_listKey_dataTable.clear();
	 
}

CTreeTool::CList::CList()
{		
	list_file_type_id[0] = 0;
    list_record_type_id = '0';
}

CTreeTool::CList::~CList()
{
	map_fileKey_fileList.clear();
}

void CTreeTool::CList::Clear()
{
    typedef map <string,CFileList>::iterator ITER;
    const ITER end = map_fileKey_fileList.end();
    for (ITER iter = map_fileKey_fileList.begin(); iter != end; ++iter)
    	{
    	cout<<"map_listKey_dataTable.size()="<<iter->second.map_listKey_dataTable.size()<<endl;
        iter->second.ClearDataTables();
 		cout<<"map_listKey_dataTable.size()="<<iter->second.map_listKey_dataTable.size()<<endl;
    	}
    map_fileKey_fileList.clear();

}

CTreeTool::CTreeTool()
{
	m_listID_list.clear();	
	GroupInfo.clear();
	filekey.clear();
	m_source_keyCount.clear();
	map_fileType_cfmtChange.clear();
	daycycleListFlag=0;
	firstOutputFlag=1;
}

CTreeTool::~CTreeTool()
{
	
	//theJSLog<<"free CTreeTool.."<<endd;
	GroupInfo.clear();

	m_listID_list.clear();

	m_source_keyCount.clear();

	for(map<string,CFmt_Change*>::iterator itfmp=map_fileType_cfmtChange.begin();
	     itfmp!=map_fileType_cfmtChange.end();itfmp++)
	     	{
		 delete itfmp->second;
	     	}
      map_fileType_cfmtChange.clear();
	//theJSLog<<"free CTreeTool ok"<<endd;
}


//listPath���������Ŀ¼
//ListConfigIDΪ����1��2��3������һ������ȡpipe_env���DAYLY_LIST_HIGH_MONITOR_ID��ֵ	
//szInputFiletypeId��ҵ�񻰵�����
void CTreeTool::Init(strct_InitPara &para)
{	
	IndexID=para.indexID;
	strcpy(szServiceID,para.service);
	strcpy(sourceGroupID,para.sourceGroup);
	//����ʱ��ֽ��TimeForDaySeperate
	if(para.endTime==NULL)
		strcpy(TimeForDaySeperate,"0000");
	else
		{
		int j=0;
		for(int i=0;i<SEPERATETIMELENGTH;i++)
			{
			if(para.endTime[i]!=':')
				{
				TimeForDaySeperate[j]=para.endTime[i];
				j++;
				}
			}
		TimeForDaySeperate[j]=0;
		}
	
	//���òο��������ReferOutputDate
	char		cTime[15];	
	memset( cTime, 0, 15 );
	getCurTime(cTime);
	char HourTime[5];
	int i=0;
	for(i=0;i<4;i++)
		{
		HourTime[i]=cTime[8+i];
		}
	HourTime[4]=0;
	if(atoi(HourTime)<atoi(TimeForDaySeperate))
		{
		char preDate[9];
		memset( preDate, 0, 9 );
		get_preday(preDate);
		strcpy(ReferOutputDate,preDate);
		}
	else
		{
		strncpy(ReferOutputDate,cTime,8);
		ReferOutputDate[8]=0;
		}
	theJSLog<<"������:"<<ReferOutputDate<<",����ʱ��ָ���:"<<TimeForDaySeperate<<endi;
	//updateLog("RUN");

	//20110617
	//���ǰһ�������ڽ���״̬�Ƿ�Ϊ"RUN"������ǣ�Ӧ�øĳ�"-1"
	//char* addDays(int nDays, const char* pchOrgDate, char* pchTgtDate)
	char preOutputDate[9];//YYYYMMDD
	addDays(-1, ReferOutputDate, preOutputDate);
	checkLog(preOutputDate);
	
	firstOutputFlag=1;
	//m_TreeMaxSize=TreeMaxSize;
	//CBindSQL ds(DBConn);	
	//CBindSQL ds1( DBConn );
	//CBindSQL ds2( DBConn );
	DBConnection conn;//���ݿ�����

	char cListConfigID[200];
	strcpy(cListConfigID,para.ListConfigID);
	
    char tmp[5];
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
		
	//��ʼ�������ڵ�m_listID_list		
	int config_count=v_list_config_id.size();
	char sqlstr[200];
	char c_config[10];
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(sqlstr,"select list_id,list_filetype_id from c_list_define where list_config_id in(");
    	for(int i=0;i<config_count;i++)
    		{
    		sprintf(c_config,"%d,",v_list_config_id[i]);
    		strcat(sqlstr,c_config);
    		}
    	sqlstr[strlen(sqlstr)-1]=')';
    	char tmp_list_id[20];	
    char list_filetype_id[10];
    char list_record_type_id;
    
    stmt.setSQLString(sqlstr);				
		stmt.execute();

	while( stmt>>tmp_list_id>>list_filetype_id) 
		{
		CList crrList;
		crrList.ListFlag=tmp_list_id;		
		sprintf(crrList.list_file_type_id,"%s",list_filetype_id);
		sprintf(sqlstr,"select record_type from c_filetype_define where filetype_id=:id",list_filetype_id);
		stmt.setSQLString(sqlstr);				
		stmt.execute();
		if(!(stmt>>list_record_type_id))
			{
				cout<<list_filetype_id<<endl;
			throw CException(ERR_SELECT,
				(char *)"select from c_filetype_define:no match filetype_id!",
				__FILE__,__LINE__);
			}
		crrList.list_record_type_id=list_record_type_id;		
		m_listID_list[tmp_list_id]=crrList;	
		CFmt_Change *p_listRecord;
		try{
			p_listRecord=new CFmt_Change();
			}
		catch(...)
			{
			char szLogStr[100];
			strcpy(szLogStr, "new CFmt_Change fail.");
      			throw CException(ERR_NEW_MEMORY,szLogStr,__FILE__,__LINE__);
			}
		p_listRecord->Init(list_filetype_id, list_record_type_id);
		map_fileType_cfmtChange[list_filetype_id]=p_listRecord;
		
		}
     //��ʼ��list_type��Ϣ,list_ruleno_define
  	char list_id[10];
  	sprintf(sqlstr,"select distinct a.LIST_ID from C_LIST_DEFINE a where a.list_config_id in(");
  	for(int i=0;i<config_count;i++)
  		{
  		sprintf(c_config,"%d,",v_list_config_id[i]);
  		strcat(sqlstr,c_config);
  		}
  	sqlstr[strlen(sqlstr)-1]=')';
  	stmt.setSQLString(sqlstr);				
		stmt.execute();
  	while(stmt>>list_id)
  		{				
  		CFeeGroupMapList crr_list;
  		crr_list.Init(list_id);		
  		string s=list_id;
  		GroupInfo[s]=crr_list;
  		}	
  	 //��ʼ�������ֶ���Ϣ:list_output_field
  	data.Init(para.szInputFiletypeId);	
  	data.printMe();
  
  	//Ҫȡ�ùؼ��ֳ���,update 20070815
  	config_count=0;
  	char sourceID[10];
  	int key_seq,field_index,col_length;
  	sprintf(sqlstr,"select count(distinct(a.source_id)) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v ",para.szInputFiletypeId);
  	//ds.Open("select count(distinct(a.source_id)) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v ");
  	stmt.setSQLString(sqlstr);				
		stmt.execute();
  	stmt>>config_count;
  	if(config_count>0)
  		{
  		cout<<config_count<<" key type!!!!"<<endl;
  		sprintf(sqlstr,"select distinct(a.source_id) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v ",para.szInputFiletypeId);
  		//ds.Open("select distinct(a.source_id) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v ");
  		//ds<<para.szInputFiletypeId;
  		stmt.setSQLString(sqlstr);				
		  stmt.execute();
  		while(stmt>>sourceID)
  			{		
  			//cout<<sourceID<<"----"<<endl;
  			keySeqLength crr_keySeqLength;
  			sprintf(sqlstr,"select count(*) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v1 and a.SOURCE_ID =:v2 and a.KEY_TYPE='R'",para.szInputFiletypeId,sourceID);
  		  stmt.setSQLString(sqlstr);				
		    stmt.execute();
		    stmt>>list_key_count;
		    
  			//ds1.Open("select count(*) from c_list_source_main_key a where a.INFILE_TYPE_ID=:v1 and a.SOURCE_ID =:v2 and a.KEY_TYPE='R'",para.szInputFiletypeId,sourceID);
  			//ds1<<para.szInputFiletypeId<<sourceID;
  			//ds1.Execute();
  			//ds1>>list_key_count;
  			m_source_keyCount[sourceID]=list_key_count;
  			}
  		strcpy(sqlstr,"select count(*) from c_list_source_main_key where infile_type_id=:v and key_type='F' \
  		    and (name='DAYCYCLE' or name='daycycle')");
  		stmt.setSQLString(sqlstr);
  		stmt<<	para.szInputFiletypeId;			
		  stmt.execute();
		  stmt>>daycycleListFlag;
		    
  		//ds.Open(sqlstr);
  		//ds<<para.szInputFiletypeId;
  		//ds>>daycycleListFlag;
  		if(daycycleListFlag!=config_count && daycycleListFlag!=0)
  			{
  			//expTrace("Y", __FILE__, __LINE__,"c_list_source_main_key error!!");
  			theJSLog<<"c_list_source_main_key error!!"<<ende;
  			exit(0);
  			}
  		}
  	else
  		{
  		//cout<<"one key type!!!!"<<endl;
  		sprintf(sqlstr,"select count(*) from C_LIST_MAIN_KEY a where a.KEY_TYPE='R' and a.INFILE_TYPE_ID=:v",para.szInputFiletypeId);
  		stmt.setSQLString(sqlstr);		
		  stmt.execute();
		  stmt>>list_key_count;
		  
  		//ds.Open("select count(*) from C_LIST_MAIN_KEY a where a.KEY_TYPE='R' and a.INFILE_TYPE_ID=:v");
  		//ds<<para.szInputFiletypeId;
  		//ds>>list_key_count; 
  		m_source_keyCount["1"]=list_key_count;
  		strcpy(sqlstr,"select count(*) from c_list_main_key where infile_type_id=:v and key_type='F' \
  		    and (name='DAYCYCLE' or name='daycycle')");
  		stmt.setSQLString(sqlstr);
  		stmt<<	para.szInputFiletypeId;			
		  stmt.execute();
		  stmt>>daycycleListFlag;
		  
  		//ds.Open(sqlstr);
  		//ds<<para.szInputFiletypeId;
  		//ds>>daycycleListFlag;
  		}
		
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  //return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "��ȡ�����ߺ�ʵ��ID ����" << endi;
  		throw jsexcp::CException(0, "��ȡ�����ߺ�ʵ��ID ����", __FILE__, __LINE__);
  		conn.close();
  		//return false;
   }   

	

	//cout<<"daycycleListFlag="<<daycycleListFlag<<endl;
	 //��ʼ�����·��
	 chkAllDir( para.listPath );
	 sprintf(cDaylyListPath,"%s",para.listPath);	
	 char okpath[300];
	 sprintf(okpath,"%s/ok",para.listPath);	
	chkAllDir( okpath );	
}


void CTreeTool::Input(CInputInfo &inputInfo)
{	
	DEBUG_LOG<<"inputInfo.m_cFeeItem="<<inputInfo.m_cFeeItem<<endd;
	//��list_id����ƥ���list��
	
	map<string,CList>::iterator it_list=m_listID_list.find(inputInfo.s_crr_abnormalListID);			
	if(it_list==m_listID_list.end())
	{
		char errormsg[250];
		sprintf(errormsg,"error list_id:%s",inputInfo.s_crr_abnormalListID.c_str());
		cout<<errormsg<<endl;
		throw CException(ERR_LIST_ID,errormsg,__FILE__,__LINE__);
	}
	
	CList &List = it_list->second;
	DEBUG_LOG<<"�����ļ�����ؼ���"<<endd;
	// �ļ�����ؼ���
    std::string file_Caption; // ������std::string

	MAP_keySeq_fieldValue::iterator it;
    MAP_keySeq_fieldValue::iterator end = inputInfo.map_file_keySeq_fieldValue.end();

	
	for (it = inputInfo.map_file_keySeq_fieldValue.begin(); it != end; ++it)
        file_Caption += it->second;

    
	map <string,CFileList>::iterator it_fileList=List.map_fileKey_fileList.find(file_Caption);
	if(it_fileList==List.map_fileKey_fileList.end())
		{		
		CFileList fileList;
		fileList.Init(inputInfo.map_file_keySeq_fieldValue);
		fileList.sourceID=inputInfo.sourceID;
		List.map_fileKey_fileList[file_Caption]=fileList;
		DEBUG_LOG<<"new:"<<file_Caption<<",source:"<<fileList.sourceID<<endd;
		}
    
	//�ʵ����ؼ���
	DEBUG_LOG<<"�����˵�����ؼ���"<<endd;
    std::string list_caption;
    end = inputInfo.map_list_keySeq_fieldValue.end();
	for (it = inputInfo.map_list_keySeq_fieldValue.begin();it != end; ++it)
    	{
	        const int mainKeyLen = inputInfo.map_list_keySeq_fieldIndex[it->first].length;
		 if(mainKeyLen==0)
		 	continue;
	        //list_caption += std::string(mainKeyLen - it->second.length(), ' ');
	        list_caption += it->second;
	}
	DEBUG_LOG<<"list_caption="<<list_caption<<endd;
	
    // ��Ҫ�޸ĵı������ͳһ��ָ�룬���ӿ��Ķ���
	CFileList* reffileList = &List.map_fileKey_fileList[file_Caption];
//////////billtype�����ⲿ����
	//string bill_type = GetBillType(inputInfo.s_crr_abnormalListID);
	map <string,CDataTable*>::iterator it_record = reffileList->map_listKey_dataTable.find(list_caption);
	CDataTable *p_DataTable;
	if (it_record == reffileList->map_listKey_dataTable.end())
		{
		//û���ҵ��ʵ�����ؼ���
		try
			{
			cout<<"to new table.."<<endl;
			//theJSLog<<GroupInfo[inputInfo.s_crr_abnormalListID].m_iGroupCount<<endd;
			//theJSLog<<data.m_iListLen<<endd;
			//theJSLog<<inputInfo.map_list_keySeq_fieldValue.size()<<endd;
			p_DataTable=new CDataTable(GroupInfo[inputInfo.s_crr_abnormalListID].m_iGroupCount, data.m_iListLen,inputInfo.map_list_keySeq_fieldValue);
			}
		catch(...)
			{
			char szLogStr[100];
			strcpy(szLogStr, "new CDataTable fail.");
      			throw CException(ERR_NEW_MEMORY,szLogStr,__FILE__,__LINE__);
			}
		DEBUG_LOG<<"new ok"<<endd;
		//p_DataTable.Init( GroupInfo[inputInfo.s_crr_abnormalListID].m_iGroupCount, data.m_iListLen,inputInfo.map_list_keySeq_fieldValue);
		strcpy(p_DataTable->list_caption, list_caption.c_str());					
		
		reffileList->map_listKey_dataTable[list_caption] = p_DataTable;
		reffileList->curr_size++;
		DEBUG_LOG<<"reffileList->curr_size="<<reffileList->curr_size<<endd;
		//20080528
		/*
		//20080502��m_TreeMaxSizeΪ-1ʱ��������
		if(reffileList->curr_size>m_TreeMaxSize && m_TreeMaxSize!=-1)
			outputflag=1;
		else 
			outputflag=0;
		*/
		
		//delete p_DataTable;
		//reffileList.map_listKey_dataTable.insert(map <string,CDataTable>::value_type(list_caption,p_DataTable));
		}
	
	//��inputInfo��ֵ����p_DataTable
	int iGroupIndex = GroupInfo[inputInfo.s_crr_abnormalListID].getGroupIndex( inputInfo.m_cFeeItem );	
	DEBUG_LOG<<"iGroupIndex="<<iGroupIndex<<endd;
	reffileList->map_listKey_dataTable[list_caption]->InputData( inputInfo, iGroupIndex );	
	//theJSLog<<"InputData ok"<<endd;
	
	//cout<<"iGroupIndex="<<iGroupIndex<<endl;

	//�����Ҫ �����ܵķ���
	if(GroupInfo[inputInfo.s_crr_abnormalListID].total_count_flag==1)
		{
		iGroupIndex = GroupInfo[inputInfo.s_crr_abnormalListID].getGroupIndex("0");
		reffileList->map_listKey_dataTable[list_caption]->InputData( inputInfo, iGroupIndex );
		}

	//�����������
	if(GroupInfo[inputInfo.s_crr_abnormalListID].attr_group_seq!=-1)
		{
		iGroupIndex = GroupInfo[inputInfo.s_crr_abnormalListID].getGroupIndex("-1");
		reffileList->map_listKey_dataTable[list_caption]->InputAttrData( inputInfo, iGroupIndex );
		}
	//cout<<"input-------------------------"<<endl;
	//reffileList->map_listKey_dataTable[list_caption]->PrintMe();
	
	inputInfo.map_file_keySeq_fieldValue.clear();
	inputInfo.map_list_keySeq_fieldValue.clear();
}



/*
��ӡTREE
level:		0
haveBrother:	0
*/
/*
void CTreeTool::PrintTree( CMyTree *tRoot, int level, int haveBrother )
{            
	int iLevel = level;
	char		cCaption[19];
	memset( cCaption, 0, 19 );
	CMyNode		*node = NULL;
	CMyTree		*child = NULL;
	CMyTree		*brother = NULL;
	CMyLeaf		*leaf = NULL;
	CMyTree		*tree = NULL;
	
	tree = tRoot;
	//��ӡ�����ֵܽڵ�
		node = ( CMyNode * )tree;
		
		
		//��ӡcaption
		memset( cCaption, 0, 19 );
		node->GetCaption( cCaption );
		printf( "%s\n", cCaption );
		
		//�õ��ӽڵ�
		if(node->GetStyle()!=STYLE_LEAF)
		{
			child =  node->GetFirstChild() ;
			if(child!=NULL) PrintTree(child,0,0);
		}
		brother =  node->GetNextBrother() ;
		
		if(brother!=NULL) 
		{
			PrintTree(brother,0,0);
		 }
}
*/
const char* CTreeTool::GetBillType(const string& list_id)
{		
	map<string,CList>::iterator it=m_listID_list.find(list_id);
			
	if(it==m_listID_list.end() )
	{
		char errormsg[250];
		sprintf(errormsg,"error list_id:%s",list_id.c_str());
		cout<<errormsg<<endl;
		throw CException(ERR_LIST_ID,errormsg,__FILE__,__LINE__);
	}
	else
		return m_listID_list[list_id].list_file_type_id;
}


int CTreeTool::Commit() 
{
	//cout<<"in commit:-------------------------------"<<endl;
	map<string,string>::iterator it=map_listFile.begin();
	
	for(;it!=map_listFile.end();it++)
		{
		//cout<<"tmp:"<<it->second<<endl<<"formal"<<it->first<<endl;
		remove(it->first.c_str());
		
		if(rename( it->second.c_str(), it->first.c_str() ))
			{
			char errmsg[100];
			sprintf(errmsg,"%s","rename error!!errno=%d",errno);
			cout<<errmsg<<endl;
			throw CException(ERR_RENAME,errmsg,__FILE__,__LINE__);
			return 1;
			}
		}
	//cout<<"in commit:-------------------------------"<<endl;
	map_listFile.clear();
	return 0;
}

//ɾ����ʱ�ļ�
int CTreeTool::RollBack() 
{	
	map<string,string>::iterator it=map_listFile.begin();
	for(;it!=map_listFile.end();it++)
		{
		//cout<<"tmp:"<<it->second<<endl<<"formal"<<it->first<<endl;
		remove(it->second.c_str());
		}
	map_listFile.clear();
	FreeAllTree();
	
	return 0;
}
void CTreeTool::print_list( ) 
{
	cout<<"-------print list_id-------------"<<endl;
	for (map<string,CList>::iterator outMap = m_listID_list.begin();outMap != m_listID_list.end();outMap++)
		{
		cout<<"list_id:"<<outMap->first<<endl;
		printf("%s,%s,%c\n",outMap->second.list_file_type_id,outMap->second.ListFlag.c_str(),outMap->second.list_record_type_id);
		}
	cout<<"-----------------------------"<<endl;
}

//20080528�����˷ֽ���д��һ����ʵ�
void CTreeTool::Output() 
{
	/*		
	//�õ�����������
	char		cDate[9];
	memset( cDate, 0, 9 );
	getCurDate( cDate );


	char		cTime[15];	
	memset( cTime, 0, 15 );
	getCurTime(cTime);
	char HourTime[5];
	int i=0;
	for(i=0;i<4;i++)
		{
		HourTime[i]=cTime[8+i];
		}
	HourTime[4]=0;

	//20080528�����˷ֽ���д��һ����ʵ�
	char crrOutputDate[9];
	char msg[200];
	//��0�㵽�ٽ���м䣬�þɵ��������
	if(atoi(HourTime)<atoi(TimeForDaySeperate) )
		{
		strcpy(crrOutputDate,ReferOutputDate);
		//expTrace(GvarDebugFlag, __FILE__, __LINE__,"1--HourTime=%s,ReferOutputDate=%s,OutputDate=%s", 
		//                                                                 HourTime,ReferOutputDate, crrOutputDate );
		sprintf(msg,"1--HourTime=%s,ReferOutputDate=%s,OutputDate=%s", HourTime,ReferOutputDate, crrOutputDate );
		}
	//��һ�ι��ٽ�㣬�þɵ�������ڣ��޸Ĳο���������Ϊ�µ�һ��
	else if(atoi(HourTime)>=atoi(TimeForDaySeperate) && strcmp(cDate,ReferOutputDate)>0)
		{
		strcpy(crrOutputDate,ReferOutputDate);
		//expTrace(GvarDebugFlag, __FILE__, __LINE__,"2--HourTime=%s,ReferOutputDate=%s,OutputDate=%s", 
		//                                                                 HourTime,ReferOutputDate, crrOutputDate );
		sprintf(msg,"2--HourTime=%s,ReferOutputDate=%s,OutputDate=%s",HourTime,ReferOutputDate, crrOutputDate ); 
		strcpy(ReferOutputDate,cDate);
		}
	//�����ٽ��
	//���õ�ǰ��������ʵ�
	else
		{
		strcpy(crrOutputDate,cDate);
		//expTrace(GvarDebugFlag, __FILE__, __LINE__,"3--HourTime=%s,ReferOutputDate=%s,OutputDate=%s", 
		//                                                                 HourTime,ReferOutputDate, crrOutputDate );
		sprintf(msg,"3--HourTime=%s,ReferOutputDate=%s,OutputDate=%s", HourTime,ReferOutputDate, crrOutputDate );
		}
	*/

	
	//��ʼ�������ҵ��ҵ�����ʵ�
	for (map<string,CList> ::iterator outMapList = m_listID_list.begin();outMapList != m_listID_list.end();outMapList++)
		{					
		//OutputPart2( outMapList->second,crrOutputDate);
		OutputPart2( outMapList->second,ReferOutputDate);
		}
	
}


//root: ���������
int CTreeTool::OutputPart3( CFileList &p_fileList, const char* list_id) 
{

 	CFmt_Change &list_record=*p_list_record;
	theJSLog<<p_list_record->Get_id()<<"|"<<list_record.Get_id()<<endl;
	char			cMaxNum[200];
	memset( cMaxNum, 0, 200 );
	strcpy( cMaxNum, "-1" );
	char			cPathName[500], cNewName[500];		
	memset( cPathName, 0, 500 );
	memset( cNewName, 0, 500 );

	map <string,CDataTable*>::iterator it_ListRrd;
	//����ÿ�����ݵĴ�С�������ȡ����������֤ÿ�ζ�10M���ݽ�tree
	//int iLineCount =  ( 10 * 1000 * 1000 ) / ( 19 + GroupInfo[list_id].m_iGroupCount * 9 * data.m_iListLen );
	//iLineCount=2;
	//merge_flag=0;

	
	char tmp_caption[50];

	map<string,int>::iterator it_keycount=m_source_keyCount.find( p_fileList.sourceID);
	if(it_keycount==m_source_keyCount.end())
		{
		//expTrace(GvarDebugFlag, __FILE__, __LINE__,"error sourceID!!!", p_fileList.sourceID);
		theJSLog<<"error sourceID!!!"<<ende;
		exit(0);
		}
	map<int,int>::iterator it;
	DEBUG_LOG<<"in OutputPart3,sourceID="<<p_fileList.sourceID<<endd;
	
AGAIN:
	if (merge_flag==1 ) 
	{
		//���������ڴ�
//		int ret = ReloadData( p_fileList, cMaxNum, iLineCount, list_type) ;

	if(_inListFile.readRec(list_record)==READ_AT_END)
	{
		memset( cMaxNum, 0, 200 );
		strcpy( cMaxNum, "-1" );
	}
        else
        {
            int i = 0;
  		    memset( cMaxNum, 0, 200 );
    	   // for(it= it_keylen->second.begin();it!= it_keylen->second.end();it++)
    	   for(;i<it_keycount->second; i++)
        	{	
               // int tmp_Sp_Len=it->second-strlen(list_record.Get_Field(i+1));

        	 //  if(tmp_Sp_Len!=0)
        	 // 	memset( tmp_caption, ' ', tmp_Sp_Len );
		
        	 // sprintf(tmp_caption+tmp_Sp_Len,"%s",list_record.Get_Field(i+1));
        	 sprintf(tmp_caption,"%s",list_record.Get_Field(i+1));
        	  strcat(cMaxNum,tmp_caption);
        	 
        	DEBUG_LOG<<"cMaxNum:"<<cMaxNum<<endd;
    		}
    		//cout<<"---------------------------------"<<endl;
        }    

	}		
	it_ListRrd=p_fileList.map_listKey_dataTable.begin();
	for(;it_ListRrd!=p_fileList.map_listKey_dataTable.end();)
		{
		if ( strcmp( cMaxNum, "-1" ) != 0 ) 
			{
				//�ȽϹؼ���
			    int tmp_cmp_result=strcmp(it_ListRrd->second->list_caption,cMaxNum);
			    if(tmp_cmp_result==0)
			    {
		            io.writeFormalList( *(it_ListRrd->second), _outTmpFile, GroupInfo[list_id],list_record,'A');
		            delete it_ListRrd->second;
		            p_fileList.map_listKey_dataTable.erase(it_ListRrd++);
			    	goto AGAIN;

			    }
			   else if(tmp_cmp_result>0)
				{
			        _outTmpFile.writeRec(list_record);
					goto AGAIN;
				}
			}
		//cout<<"write list--------------------"<<endl;
		//it_ListRrd->second->PrintMe();
		io.writeFormalList( *(it_ListRrd->second), _outTmpFile, GroupInfo[list_id],list_record,'W');
		delete it_ListRrd->second;
		p_fileList.map_listKey_dataTable.erase(it_ListRrd++);
		//fileList_count++;		
		}	
	
	p_fileList.map_listKey_dataTable.clear();
//	printf("\noutput the old record in list file......");
	if(strcmp( cMaxNum, "-1" ) != 0)
	{
			//printf("\noutput the old record in list file......");			
		  _outTmpFile.writeRec(list_record);
		  while(_inListFile.readRec(list_record)!=READ_AT_END)
			_outTmpFile.writeRec(list_record);
	}
		
	return 0;
}

//�ͷŸ���ҵ��ҵ�����ʵ���ռ�õ��ڴ�
int CTreeTool::FreeAllTree()
{
	//cout<<"--------------------------------------------------fileList_count="<<fileList_count<<endl;
	//fileList_count=0;
    typedef std::map<std::string, CList>::iterator ITER;
    ITER end = m_listID_list.end();

    for (ITER iter = m_listID_list.begin(); iter != end; ++iter)
    	{
        iter->second.Clear();
    	}

    //m_listID_list.clear();

    return 0;
}

//�����ǰҵ�����ʵ��������ʵ��ļ�
//20080729����Сɾ����ʱ�ļ��ķ�Χ������ɾ�����ģ�����ʱ�ļ�		
int CTreeTool::OutputPart2( CList &pList, const char *date) 
{
//	printf("OUTPUT:---%s,%c,%s\n",
//    	pList.list_file_type_id,pList.list_record_type_id,pList.ListFlag.c_str());

	map<string,CFmt_Change*> ::iterator itfmp=map_fileType_cfmtChange.find(pList.list_file_type_id);
	if(itfmp==map_fileType_cfmtChange.end())
		{
	        //cout<<"no match list_file_type_id:"<<pList.list_file_type_id<<endl;
		 //expTrace(GvarDebugFlag, __FILE__, __LINE__,"no match list_file_type_id: %s", pList.list_file_type_id);
		 theJSLog<<"no match list_file_type_id:"<<pList.list_file_type_id<<ende;
		 for(itfmp=map_fileType_cfmtChange.begin();itfmp!=map_fileType_cfmtChange.end();itfmp++)
		 	cout<<itfmp->first<<endl;
		 exit(1);
		}
	else
		{
		//cout<<"match list_file_type_id:"<<pList.list_file_type_id<<endl;
		p_list_record=itfmp->second;
		}
	
	_inListFile.Init(pList.list_file_type_id);
	_outTmpFile.Init(pList.list_file_type_id);
	io.Init(pList.list_file_type_id,pList.list_record_type_id);

	char		cTmpFileName[500], cPartTmpFileName[500],cFileName[500];	
	memset( cTmpFileName, 0, 500 );
	memset( cFileName, 0, 500 );
	int	iFile = -1;
	FILE	*fFile = NULL;

	char billFlag[30];
	sprintf(billFlag,".%s.%s.%d",szServiceID,sourceGroupID,IndexID);
	
	for(map <string,CFileList>::iterator outMapFileList = pList.map_fileKey_fileList.begin();
			outMapFileList!=pList.map_fileKey_fileList.end();)
	{
		if(outMapFileList->second.map_listKey_dataTable.empty())
			{
			outMapFileList++;
			continue;
			}
		MAP_keySeq_fieldValue::iterator it=outMapFileList->second.map_filekey_value.begin();
		memset( cFileName, 0, 500 );
		
		sprintf( cFileName, "%s/%s", cDaylyListPath, it->second.c_str() );
		chkAllDir( cFileName);		
		
		memset( cFileName, 0, 500 );
		memset( cTmpFileName, 0, 500 );
		sprintf( cTmpFileName, "%s/%s/~%s",cDaylyListPath, it->second.c_str(),it->second.c_str());
		sprintf( cFileName, "%s/%s/%s",cDaylyListPath, it->second.c_str(),it->second.c_str());

		it++;
		for(;it!=outMapFileList->second.map_filekey_value.end();it++)
			{
			strcat(cTmpFileName,".");
			strcat(cTmpFileName,it->second.c_str());
			strcat(cFileName,".");
			strcat(cFileName,it->second.c_str());
			}
			
		strcat(cFileName,".");
		strcat(cFileName,pList.ListFlag.c_str());
		if(daycycleListFlag==0)
			{
			strcat(cFileName,".");
			strcat(cFileName,date);
			}
		strcat(cFileName,".smallbill");	
		strcat(cFileName,billFlag);	
		
		strcat(cTmpFileName,".");
		strcat(cTmpFileName,pList.ListFlag.c_str());
		//20080729
		char deleteSign[500];
		sprintf(deleteSign,"%s*tmp*",cTmpFileName);
		if(daycycleListFlag==0)
			{
			strcat(cTmpFileName,".");
			strcat(cTmpFileName,date);
			}
		strcat(cTmpFileName,".smallbill");	
		strcat(cTmpFileName,billFlag);	
		strcat(cTmpFileName,".tmp");
		
		strcpy(cPartTmpFileName,cTmpFileName);
		strcat(cPartTmpFileName,".part");
		
		//expTrace(GvarDebugFlag, __FILE__, __LINE__,"write to list: %s", cFileName);
		theJSLog<<"write to list:"<<cFileName<<endi;
		//cout<<"list file name:"<<cFileName<<endl;
		//cout<<"list tmp filename:"<<cTmpFileName<<endl;
		//cout<<"cPartTmpFileName"<<cPartTmpFileName<<endl;
		
		//ɾ��ԭ���ܴ��ڵ���ʱ�ļ�
		//20080729����С��Χ������ɾ�����ģ�����ʱ�ļ�			
		//remove( cPartTmpFileName );	//�����rm�Ѿ�����������ļ��Ĵ�����		
		if(firstOutputFlag==1 )
			{
			char order[200];
		       sprintf( order, "\\rm %s",deleteSign);
			//expTrace(GvarDebugFlag, __FILE__, __LINE__,order);
			theJSLog<<order<<endi;
			system(order);
			}

		_outTmpFile.Open(cPartTmpFileName);
		//cout<<"open "<<cPartTmpFileName<<endl;
		//����ͬ����ʽ�ļ����ڣ�����Ҫ�����ļ��ϲ�
		merge_flag=0;
		
		//����ʱ�ļ��ϲ�
		if ( 1 == IsFileExist( cTmpFileName ) ) 
			{
			merge_flag=1;
//			cout<<"merge_flag=1;"<<endl;
//			cout<<"open file:"<<cTmpFileName<<endl;
			_inListFile.Open(cTmpFileName);
			} 
		//����ʽ�ʵ��ļ��ϲ�
		else if ( 1 == IsFileExist( cFileName ) ) 
			{
			merge_flag=1;
			DEBUG_LOG<<"merge_flag=1;"<<endd;
			DEBUG_LOG<<"open file:"<<cFileName<<endd;
			_inListFile.Open(cFileName);
			DEBUG_LOG<<"open ok!"<<endd;
			} 
		
	    //����ʵ���ͬʱ�ϲ��ļ�
	       DEBUG_LOG<<"update record count:"<< outMapFileList->second.map_listKey_dataTable.size()<<endd;
		OutputPart3( outMapFileList->second, pList.ListFlag.c_str() );
		DEBUG_LOG<<"OutputPart3 ok!"<<endd;
		if(merge_flag==1)
			_inListFile.Close();
		
		_outTmpFile.Close();
		remove(cTmpFileName);
		if(	rename( cPartTmpFileName,cTmpFileName ))
			{
				char errmsg[100];
				sprintf(errmsg,"rename %s to %s error!!errno=%d",cPartTmpFileName,cTmpFileName,errno);
				cout<<errmsg<<endl;
				throw CException(ERR_RENAME,errmsg,__FILE__,__LINE__);
			}
			
		//cout<<"rename "<<cTmpFileName<<endl;
		string str_cFileName=cFileName;
		string str_tmpFileName=cTmpFileName;
		map_listFile[str_cFileName]=str_tmpFileName;
			
		pList.map_fileKey_fileList.erase( outMapFileList++);
	}
	pList.map_fileKey_fileList.clear();
	firstOutputFlag=0;
	return 0;
	
}

int CTreeTool::IsFileExist(const char *p_cPathName)
{
	return ((access(p_cPathName, 0) == 0) ? 1 : -1); 
}

void CTreeTool::get_preday(char* predate)
/*ȡ��ǰһ���ʱ�� */
{
  	time_t		time1;
	struct tm	*time2;
	time(&time1);
	time_t timepre = time1-86400;
	time2 = localtime(&timepre);	
	sprintf(predate, "%4d%02d%02d", time2->tm_year+1900, time2->tm_mon+1, time2->tm_mday);

}

void CTreeTool::setReferOutputDateWhileSleep(char* date)
{
	strcpy(ReferOutputDate,date);
	//expTrace(GvarDebugFlag, __FILE__, __LINE__,"set ReferOutputDate=%s", ReferOutputDate );
	theJSLog<<"set ReferOutputDate="<<ReferOutputDate<<endi;
}

int CTreeTool::writeOKFile(char* date)
{
	char okFileName[256];
	sprintf(okFileName,"%s/ok/%s.%s.ok",cDaylyListPath,sourceGroupID,date);
	int okfileflag = open( okFileName, O_RDWR|O_CREAT|O_TRUNC, 0666 );
	if ( okfileflag < 0 )
	{
		char errmsg[100];
		sprintf(errmsg,"open file %s error",okFileName);
		cout<<errmsg<<endl;
		throw CException(ERR_RENAME,errmsg,__FILE__,__LINE__);
		return 1;
	}	
	close(okfileflag);
	return 0;
}

int CTreeTool::checkLog(char* predate)
{
	theJSLog<<"to check D_LIST_LOG , daycle :"<<predate<<endi;
	char szCurrTime[20];
	getCurTime(szCurrTime);
	//CBindSQL ds(DBConn);	
	DBConnection conn;//���ݿ�����
	char sql[500];
	int icount=0;
	char state[10];
	 try
	   	{
	   		if (dbConnect(conn))
    	 {
    			Statement stmt = conn.createStatement();
    			strcpy(sql,"select count(a.rowid) from D_LIST_LOG a where a.day_cycle=:v1 and a.service=:v2 and a.source_group=:v3 and a.index_id=:v4");
      		theJSLog<<predate<<"|"<<szServiceID<<"|"<<sourceGroupID<<"|"<<IndexID<<endi;
      		stmt.setSQLString(sql);
    			stmt << predate<<szServiceID<<sourceGroupID<<IndexID ;			
    			stmt.execute();
    			stmt >> icount;
      		//ds.Open(sql);
      		//ds<<predate<<szServiceID<<sourceGroupID<<IndexID;
      		//ds>>icount;
      		if(icount==0)
      			{
      			theJSLog<<"no record ! "<<endi;
      			}
      		else
      			{
      			strcpy(sql,"select a.STATE from D_LIST_LOG a where a.day_cycle=:v1 and a.service=:v2 and a.source_group=:v3 and a.index_id=:v4");
      			stmt.setSQLString(sql);
    			  stmt <<predate<<szServiceID<<sourceGroupID<<IndexID;			
    			  stmt.execute();
    			  stmt >> state;
 
      			if(strcmp(state,"RUN")==0)
      				{
      				theJSLog<<"update STATE:EXIT"<<endi;
      				strcpy(sql,"update D_LIST_LOG a set a.STATE=:v1,a.update_time=:time where a.day_cycle=:v2 and a.service=:v3 and a.source_group=:v4 and a.index_id=:v5");
      				stmt.setSQLString(sql);
    			    stmt <<"EXIT"<<szCurrTime<<predate<<szServiceID<<sourceGroupID<<IndexID;			
    			    stmt.execute();
    			    conn.commit();
    			  
      				//ds.Open(sql,NONSELECT_DML);
      				//ds<<"EXIT"<<szCurrTime<<predate<<szServiceID<<sourceGroupID<<IndexID;
      				//ds.Execute();	
      				//ds.Close();
      				}
      			else
      				{
      				theJSLog<<"record is ok !"<<endi;
      				}
      			}				
    	
    	 }else{
    	 	  cout<<"connect error."<<endl;
    	 	  return false;
    	 }
    	    conn.close();	    			
	 	}
	 catch(CException e)
		{
		theJSLog<<e.GetErrMessage()<<endi;
		char errmsg[500];
		sprintf(errmsg,"����D_LIST_LOG��ʧ��!!sql:%s",sql);
		errLog(LEVEL_ERROR,"",ERR_UPDATE,errmsg,__FILE__,__LINE__,e);
		throw e;
		}  
}

int CTreeTool::updateLog(char* state)
{
	theJSLog<<"SET STATE="<<state<<endi;
	char szCurrTime[20];
	getCurTime(szCurrTime);
	//CBindSQL ds(DBConn);	
	DBConnection conn;//���ݿ�����
	char sql[500];
	int icount=0;
	 try
	   	{
	   		if (dbConnect(conn))
    	 {
    			Statement stmt = conn.createStatement();
    			strcpy(sql,"select count(a.rowid) from D_LIST_LOG a where a.day_cycle=:v1 and a.service=:v2 and a.source_group=:v3 and a.index_id=:v4");
      		theJSLog<<ReferOutputDate<<"|"<<szServiceID<<"|"<<sourceGroupID<<"|"<<IndexID<<endi;
      		stmt.setSQLString(sql);
    			stmt <<ReferOutputDate<<szServiceID<<sourceGroupID<<IndexID;			
    			stmt.execute();
    			stmt>>icount;      		
      		
      		if(icount==0)
      			{
      			strcpy(sql,"insert into D_LIST_LOG(DAY_CYCLE,SERVICE,SOURCE_GROUP,INDEX_ID,STATE,update_time) values(:v1,:v2,:v3,:v4,:v5,:v6)");
      			stmt.setSQLString(sql);
    			  stmt <<ReferOutputDate<<szServiceID<<sourceGroupID<<IndexID<<state<<szCurrTime;			
    			  stmt.execute();
      			}
      		else
      			{
      			strcpy(sql,"update D_LIST_LOG a set a.STATE=:v1,a.update_time=:time where a.day_cycle=:v2 and a.service=:v3 and a.source_group=:v4 and a.index_id=:v5");
      			stmt.setSQLString(sql);
    			  stmt <<state<<szCurrTime<<ReferOutputDate<<szServiceID<<sourceGroupID<<IndexID;		
    			  stmt.execute();
      			}
      		if(strcmp(state,"OK")==0)
      			{
      			getCurDate( ReferOutputDate );
      			//test
      			//strcat(ReferOutputDate,"-1");
      			strcpy(sql,"insert into D_LIST_LOG(DAY_CYCLE,SERVICE,SOURCE_GROUP,INDEX_ID,STATE,update_time) values(:v1,:v2,:v3,:v4,:v5,:v6)");
      			stmt.setSQLString(sql);
    			  stmt <<ReferOutputDate<<szServiceID<<sourceGroupID<<IndexID<<"RUN"<<szCurrTime;	
    			  stmt.execute();
 
      			theJSLog<<"������:"<<ReferOutputDate<<endi;
      			}
	
    	 }else{
    	 	  cout<<"connect error."<<endl;
    	 	  return false;
    	 }
    	    conn.close();	   
	 	}
	 catch(CException e)
		{
		theJSLog<<e.GetErrMessage()<<endi;
		char errmsg[500];
		sprintf(errmsg,"����D_LIST_LOG��ʧ��!!sql:%s",sql);
		errLog(LEVEL_ERROR,"",ERR_UPDATE,errmsg,__FILE__,__LINE__,e);
		throw e;
		}  
	theJSLog<<"SET DONE"<<endi;
}

