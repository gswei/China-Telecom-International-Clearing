#include "FeeGroupMapList.h"
using namespace std;
using namespace tpss;
CNumList::CNumList() {
	clear();
}

CNumList::~CNumList() 
{
	clear();
}

void CNumList::PrintMe() 
{
	cout<<"iNumCount="<<iNumCount<<endl;
	for ( int i = 0 ; i < iNumCount ; i ++ ) 
	{
		printf( "Num[%d]: %d	", i, iNumList[i] );	
		printf("\n");
	}
}

//ȡһ������Ű����ķ��ã����������
void CNumList::getAllNum( const char *pChar ) {
	char		cLine[500];
	memset( cLine, 0, 500 );	
	strcpy( cLine, pChar );
	char cNum[150];
	int iNum;
	char  *p1,*p2=cLine;
	
	while((p1=strchr(p2,';'))!=NULL)
		{		
		*p1=0;
		memset( cNum, 0, 150 );
		strcpy( cNum,p2);
		iNum = atoi( cNum );
		addNum( iNum );
		p2=p1+1;
		}
	memset( cNum, 0, 150 );
	strcpy( cNum,p2);
	iNum = atoi( cNum );
	addNum( iNum );
	
	//PrintMe();
	/*
	for ( int i = 0 ; i < iLen ; i ++ ) 
	{
		if ( ';' == cLine[i] )
		{
			memset( cNum, 0, 100 );
			strncpy( cNum, pCur, i - iBegin );
			iNum = atoi( cNum );
			addNum( iNum );
			
			if ( i < iLen ) 
			{
				pCur += i - iBegin + 1;
			}
			iBegin = i + 1;
		}
		
	}
	*/
	
	
	//PrintMe();
}

void CNumList::clear() {
	iNumCount = 0;
	for ( int i = 0 ; i < 100 ; i ++ ) {
		iNumList[i] = -1;	
	}
}

int CNumList::isNumExisted( int iNum ) {
	for ( int i = 0 ; i < iNumCount ; i ++ ) {
		if ( iNum == iNumList[i] ) {
			return 1;	
		}
	}
	return 0;
}

void CNumList::addNum( int iNewNum ) 
{
	iNumList[ iNumCount ] = iNewNum;
	iNumCount ++;
}

CFeeGroupMapList::CFeeGroupMapList()
{
	m_pList.clear();
	m_groupLenth.clear();	
}

CFeeGroupMapList::~CFeeGroupMapList() 
{
	//theLog<<"free CFeeGroupMapList.."<<endd;
	m_pList.clear();
	m_groupLenth.clear();
	//theLog<<"free CFeeGroupMapList 0k"<<endd;
}

void CFeeGroupMapList::Init(char *list_id) 
{
	total_count_flag=0;
	attr_group_seq=-1;
	//m_db = &DBConn;
	int i=0;
	int iCount = getListLength(list_id);
	for(i=0;i<iCount;i++)
		{
		FEE_AND_GROUP tmp_fee_and_group;
		m_pList.push_back(tmp_fee_and_group);
		}
			
	m_iListLen = iCount;//����Ÿ���
	m_iDataCount = 0;
	m_iGroupCount = getGroupCount(list_id);//�ʵ���ĸ���
	for(i=0;i<m_iGroupCount;i++)
		{
		m_groupLenth.push_back(0);
		}
	
	getAllData(list_id);
	//cout<<list_type<<endl;
	//printAllList();
}

int CFeeGroupMapList::getGroupCount(char *list_id) 
{
	//CBindSQL bs( *m_db );
	int iData = 0;
  DBConnection conn;//���ݿ�����

 try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "SELECT COUNT(distinct group_seq) FROM c_list_ruleno_define where list_id=:v2";
			stmt.setSQLString(sql);
			stmt << list_id;			
			stmt.execute();
			stmt >> iData;
		
	 }else{
	 	  cout<<"connect error."<<endl;
	 	  return -1;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "getGroupCount ����" << endi;
  		throw jsexcp::CException(0, "getGroupCount ����", __FILE__, __LINE__);
  		conn.close();
  		return -1;
   } 
	return iData;
}


		


/***************************************************
description:	�õ����ݵ�������
input:	        
output:         
return:         ���ݵ�������		
programmer:	��	��
date:		2006-01-10
*****************************************************/ 
int CFeeGroupMapList::getDataCount() {
	return m_iDataCount;	
}

/***************************************************
description:	�õ����ݵ�������
input:	        
output:         
return:         
		1:	��	��
		0��	�б�����	
programmer:	��	��
date:		2006-01-10
*****************************************************/ 
int CFeeGroupMapList::addData( FEE_AND_GROUP data ) {
	FEE_AND_GROUP	*pData = NULL;
	pData = &m_pList[ m_iDataCount ];
	
	memset( pData->fee_index, 0, 15 );
	memset( pData->group_index, 0, 15 );
	memset( pData->len_type, 0, 100 );
	
	strcpy( pData->fee_index, data.fee_index );
	strcpy( pData->group_index, data.group_index );
	strcpy( pData->len_type, data.len_type );
	
	CNumList	*pList = &data.num_list;
	CNumList	*pReal = &pData->num_list;
	
	pReal->clear();
	for ( int i = 0 ; i < pList->iNumCount ; i ++ ) 
	{
		pReal->addNum( pList->iNumList[i] );
	}
	
	m_iDataCount ++;
	
	return 1;	
}

//�õ�ĳ�������Ӧ���ֶθ�������
int CFeeGroupMapList::getLenType( char *pLenType, const char *feeIndex ) {
	FEE_AND_GROUP	*pCurData = NULL;
	for ( int i = 0 ; i < m_iDataCount ; i ++ ) {
		pCurData = &m_pList[i];
		if ( 0 == strcmp( pCurData->fee_index, feeIndex ) ) {
			memset( pLenType, 0, strlen( pLenType ) );
			strcpy( pLenType, pCurData->len_type );
			return 1;
		}
	}
	return -1;
}

//�õ�ĳindex��Ӧ���ֶθ�������
int CFeeGroupMapList::getLenType( char *pLenType, int groupIndex ) {
	FEE_AND_GROUP	*pCurData = NULL;
	pCurData = &m_pList[ groupIndex ];
	memset( pLenType, 0, strlen( pLenType ) );
	strcpy( pLenType, pCurData->len_type );
	return 1;
}

/***************************************************
description:	����б�
input:	        
output:         
return:         
programmer:	��	��
date:		2006-01-10
*****************************************************/ 
void CFeeGroupMapList::clearList() {
	m_iDataCount = 0;
}

/***************************************************
description:	�õ�ĳһ�����fee_index����Ӧ����ţ�group_index��
input:	        groupIndex��	������
output:         
		-1:		���ݲ�����
		������		���
return:         
programmer:	��	��
date:		2006-01-10
*****************************************************/ 
int CFeeGroupMapList::getGroupIndex( const char *feeIndex ) 
{
	FEE_AND_GROUP	*pCurData = NULL;
	//printf("getGroupIndex---m_iDataCount=%d\n",m_iDataCount);
	for ( int i = 0 ; i < m_iDataCount ; i ++ ) 
		{
		
		pCurData = &m_pList[i];
		//printf("i=%d,pCurData->fee_index=%s\n",i,pCurData->fee_index);
		if ( 0 == strcmp( pCurData->fee_index, feeIndex ) ) 
		{
			//printf("pCurData->fee_index=%s\n",pCurData->fee_index);
			return atoi( pCurData->group_index );	
		}
	}
	return -1;
}

/***************************************************
description:	�õ�ĳһ������
input:	        groupIndex��	������
output:         
		-1:		���ݲ�����
		������		���
return:         
programmer:	��	��
date:		2006-01-10
*****************************************************/ 
char *CFeeGroupMapList::getFeeItem( int groupIndex ) {
	char		cGroupIndex[100];
	memset( cGroupIndex, 0, 100 );
	sprintf( cGroupIndex, "%d", groupIndex );
	
	FEE_AND_GROUP	*pCurData = NULL;
	for ( int i = 0 ; i < m_iDataCount ; i ++ ) {
		pCurData = &m_pList[i];
		if ( 0 == strcmp( pCurData->group_index, cGroupIndex ) ) {
			return pCurData->fee_index;	
		}
	}
	return NULL;
}

int CFeeGroupMapList::isFieldEnabled( int iGroupIndex, int iFieldIndex ) {
	CNumList		*pNumList;
	pNumList = &m_pList[ iGroupIndex ].num_list;
	return pNumList->isNumExisted( iFieldIndex );
}

void CFeeGroupMapList::printAllList() {
	cout<<"group count:"<<m_iGroupCount<<"  total_count_flag:"<<total_count_flag<<endl;
	CNumList		*pNumList;
	for ( int i = 0 ; i < m_iDataCount ; i ++ ) {
		cout<<"m_pList[i].fee_index="<<m_pList[i].fee_index<<"	group index="<<m_pList[i].group_index<<endl;
		pNumList = &m_pList[i].num_list;
		pNumList->PrintMe();
	}
}

/***************************************************
description:	�õ�ĳһ�����fee_index����Ӧ����ţ�group_index��
input:	        groupIndex��	������
output:         
		-1:		���ݲ�����
		������		���
return:         
programmer:	��	��
date:		2006-01-10
*****************************************************/ 
int CFeeGroupMapList::getAllData(char *list_id) 
{
	char msg[100];
	//CBindSQL bs( *m_db );
	DBConnection conn;//���ݿ�����
	FEE_AND_GROUP	curData;
	memset( &curData, 0, sizeof( FEE_AND_GROUP ) );
	char		cData[150];
	memset( cData, 0, 150 );	
	int i=0;
	
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
      string sql = "SELECT rule_no, group_seq, value_field FROM c_list_ruleno_define where list_id=:v2 ORDER BY group_seq ";
      stmt.setSQLString(sql);
			stmt << list_id;			
			stmt.execute();
			
			//bs.Open("SELECT rule_no, group_seq, value_field FROM c_list_ruleno_define where list_id=:v2 ORDER BY group_seq ");
    	//bs<<list_id;
    	while( stmt>>cData ) 
    	{
    		memset( &curData, 0, sizeof( FEE_AND_GROUP ) );
    		strcpy( curData.fee_index, cData );
    		
    		//��Ҫ�ۼ��ܷ��ñ�־
    		if(strcmp(curData.fee_index,"0")==0)
    			total_count_flag=1;
    
    		//�����
    		memset( cData, 0, 150 );
    		stmt>>cData;
    		strcpy( curData.group_index, cData );
    		if(atoi(cData)!=i)
    			{
    			sprintf(msg,"group_seq:%d in c_list_ruleno_define is not defined correctly!",atoi(cData));
    			cout<<msg<<endl;
    			throw CException(ERROR_GROUPSEQ_DEFINED,msg,__FILE__,__LINE__);
    			}
    		
    		//Ϊ������
    		if(strcmp(curData.fee_index,"-1")==0)
    			{
    			attr_group_seq=i;
    			}		
    		i ++;
    		
    		//�����
    		memset( cData, 0, 150 );
    		stmt>>cData;
    		strcpy( curData.len_type, cData );
    		
    		curData.num_list.clear();
    		
    		//printf("list_length:%s\n",cData);
    		
    		curData.num_list.getAllNum( cData );
    
    		m_groupLenth[atoi(curData.group_index)]=curData.num_list.iNumCount;
    			
    		memset( cData, 0, 150 );
    		
    		addData( curData );
    		}
    	//bs.Close();	
			
	 }else{
	 	cout<<"connect error."<<endl;
	 	return -1;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		theJSLog << "�õ�ĳһ�����fee_index����Ӧ����ţ�group_index�� ����" << endi;
		throw jsexcp::CException(0, "�õ�ĳһ�����fee_index����Ӧ����ţ�group_index�� ����", __FILE__, __LINE__);
		conn.close();
		return -1;
  }
  
	return 1;
}

/***************************************************
description:	�õ���������
input:	        
output:         
		-1:		���ݲ�����
		������		���
return:         
programmer:	��	��
date:		2006-01-10
*****************************************************/ 
int CFeeGroupMapList::getListLength(char *list_id) 
{
	//CBindSQL bs( *m_db );
	DBConnection conn;//���ݿ�����
	int iData = 0;
	try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "SELECT COUNT(*) FROM c_list_ruleno_define where list_id=:v2";
			stmt.setSQLString(sql);
			//bs.Open("SELECT COUNT(*) FROM c_list_ruleno_define where list_id=:v2");
    	stmt<<list_id;
    	stmt.execute();		
    	stmt>>iData;
	
	 }else{
	 	cout<<"connect error."<<endl;
	 	return -1;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		theJSLog << "�õ��������� ����" << endi;
		throw jsexcp::CException(0, "�õ��������� ����", __FILE__, __LINE__);
		conn.close();
		return -1;
   }          
	
	return iData;	
}
