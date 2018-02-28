#include "CallModify.h"

C_CallModify::C_CallModify()
{
	memset(m_szSourceGroupId, 0, sizeof(m_szSourceGroupId));
	memset(m_szServCatId,0,sizeof(m_szServCatId));
	memset(m_szSourceId,0,sizeof(m_szSourceId));
	memset(m_szSourceId,0,sizeof(m_szSourceId));
	memset(m_szCalling,0,sizeof(m_szCalling));
	memset(m_szCalled,0,sizeof(m_szCalled));
	memset(m_szMCalling,0,sizeof(m_szMCalling));
	memset(m_szMCalled,0,sizeof(m_szMCalled));
	memset(m_sAbnReason,0,sizeof(m_sAbnReason));//������ԭ��
	memset(m_szAbnField,0,sizeof(m_szAbnField));//����������
	//m_iProcessId=0;
	m_iAbnReason=0;
	initFlag=0;
	table = NULL;
}

C_CallModify::~C_CallModify()
{

}

void C_CallModify::execute(PacketParser& pps,ResParser& retValue)
{
	//���ִ�����
	if ( pps.getItem_num() != 3 )
	{
		sprintf( ErrorMsg,"��� StdChkCallingNbr �������������");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);		
	}
			
	if( initFlag != 1 )
		Init( pps );

	//��ȡ��������ɱ����
	//��ȡ�����Ӧ��ҵ����룬��ȥ��ѯʵ�ʵ��������
	memset(m_szServCatId,0,sizeof(m_szServCatId));
	pps.getFieldValue(1,m_szServCatId);
	DeleteSpace( m_szServCatId ); 
	
	//��ʼ��������
	if (strcmp(m_LastSerCatId,m_szServCatId)!=0  ||  strcmp(m_LastSourceId,m_szSourceId)!=0  )
	{
		strcpy(m_LastSerCatId,m_szServCatId);
		strcpy(m_LastSourceId,m_szSourceId);
		m_callnumber.Reset(m_szServiceId, m_LastSourceId);
	}

	memset(m_szCalling,0,sizeof(m_szCalling));
	pps.getFieldValue(2,m_szCalling);
	DeleteSpace( m_szCalling ); 
	
	memset(m_szCalled,0,sizeof(m_szCalled));
	pps.getFieldValue(3,m_szCalled);
	DeleteSpace( m_szCalled ); 

	char tmpTollcodeA[FIELD_LEN];
	char tmpTollcodeB[FIELD_LEN];
	char tmpTeleno[FIELD_LEN];
	
	/*calling number*/
	//get the tollcode
	m_callnumber.Set_Value();
	m_callnumber.Set_ServCatId(m_szServCatId);
	m_callnumber.Set_NbrType("01");

	m_callnumber.Set_CallNbr(m_szCalling);

	m_iAbnReason=m_callnumber.queryTollcode(m_szAbnField);

	if (m_iAbnReason)
	//������
	{
		char szLackType[RECORD_LENGTH+1];
		memset(szLackType, 0, sizeof(szLackType));
		sprintf(szLackType, "%d", m_iAbnReason );
		DeleteSpace(szLackType);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, m_szAbnField);
		return;
	}
	
	m_callnumber.DealTollcode();
	m_callnumber.Get_Tollcode(tmpTollcodeA);
	m_callnumber.Get_AfterTollTel(tmpTeleno);
	m_callnumber.Set_CallNbr(tmpTeleno);
	//�������������ж�bug
	if (m_callnumber.IsGsm() && strncmp(tmpTollcodeA,"00",2)!=0)
		strcpy(m_szMCalling,tmpTeleno);
	else
		strcpy(m_szMCalling,m_szCalling);

	/*called number*/
	//get the tollcode
	m_callnumber.Set_Value();
	m_callnumber.Set_ServCatId(m_szServCatId);
	m_callnumber.Set_NbrType("10");

	m_callnumber.Set_CallNbr(m_szCalled);

	m_iAbnReason=m_callnumber.queryTollcode(m_szAbnField);

	if (m_iAbnReason)
	//������
	{
		char szLackType[RECORD_LENGTH+1];
		memset(szLackType, 0, sizeof(szLackType));
		sprintf(szLackType, "%d", m_iAbnReason );
		DeleteSpace(szLackType);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, m_szAbnField);
		return;
	}
	
	m_callnumber.DealTollcode();
	m_callnumber.Get_Tollcode(tmpTollcodeB);
	m_callnumber.Get_AfterTollTel(tmpTeleno);
	m_callnumber.Set_CallNbr(tmpTeleno);
	
	//�������������ж�bug
	if (m_callnumber.IsGsm() && strncmp(tmpTollcodeB,"00",2)!=0)
	{
		if( strcmp(tmpTollcodeA,tmpTollcodeB)==0)
			strcpy(m_szMCalled,tmpTeleno);
		else
			sprintf(m_szMCalled,"0%s",tmpTeleno);
	}
	else
		strcpy(m_szMCalled,m_szCalled);

	//output
	retValue.setFieldValue( 1, m_szMCalling, strlen(m_szMCalling) );
	retValue.setFieldValue( 2, m_szMCalled, strlen(m_szMCalled) );

	return;

}

int C_CallModify::Init(PacketParser &pps)
{
	/*get some of input params which will not change after first coming*/
	
	memset(m_LastSerCatId,0,sizeof(m_LastSerCatId));
	pps.getFieldValue(1,m_LastSerCatId);
	DeleteSpace( m_LastSerCatId ); 
	
	memset(m_LastSourceId,0,sizeof(m_LastSourceId));
	strcpy(m_LastSourceId, m_szSourceId);
	DeleteSpace( m_LastSourceId ); 

	//��ʼ��������
	//m_callnumber.Init(m_szServiceId, m_LastSourceId, table);
	m_callnumber.Reset(m_szServiceId, m_LastSourceId);
	
	/* set init_flag of this plugin */
	initFlag = 1;

	return 0;
}

/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_CallModify::init(char *szSourceGroupID, char *szServiceID, int index)
{
	memset(m_szSourceGroupId, 0, sizeof(m_szSourceGroupId));
	strcpy(m_szSourceGroupId, szSourceGroupID);
	DeleteSpace(m_szSourceGroupId ); 

	memset( m_szServiceId, 0, sizeof( m_szServiceId ) );   
	strcpy( m_szServiceId, szServiceID );
	DeleteSpace(m_szServiceId ); 

	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	//��ʼ��������
	m_callnumber.Init(table);
}

/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_CallModify::message(MessageParser&  pMessage)
{
	int message=pMessage.getMessageType();
	switch(message)
	{
		case MESSAGE_NEW_FILE:
			memset(m_szSourceId,0,sizeof(m_szSourceId));
			strcpy(m_szSourceId, pMessage.getSourceId());
			DeleteSpace( m_szSourceId ); 
			break;
		default:
			break;
	}
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_CallModify::printMe()
{
	printf("\t�������:ModifyCallNumber,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_CallModify::getPluginName()
{
	return "ModifyCallNumber";
}

std::string C_CallModify::getPluginVersion(){
	return "3.0.0";
}
