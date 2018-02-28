#include "CallProperty.h"

C_CallProperty::C_CallProperty()
{
	initFlag = 0;
	table = NULL;
}
C_CallProperty::~C_CallProperty()
{
	if(table != NULL)
		delete table;
}

//
void C_CallProperty::execute(PacketParser& pps,ResParser& retValue)
{
	//���ִ�����
	if ( pps.getItem_num() != 6 )
	{
		sprintf( ErrorMsg,"���AnaNbrProperty�������������6��");
		throw jsexcp::CException(0,(char *)ErrorMsg,(char *)__FILE__,__LINE__);		
	}
			
	if( initFlag != 1 )
		Init( pps );

	//�����������

	//�õ��������
	getInputParams(pps);

//	theLog<<"��ȡ��������ɹ���"<<endd;

	//���ݵ�CallNumber ��
	setParam();
//	theLog<<"���ݵ�CallNumber��"<<endd;

	//�Ƿ����������
	m_callnumber.DealBefTelServ();
//	theLog<<"�Ƿ����������"<<endd;
	
	//����У��	,������ʽΪ ����+����
//	theLog<<"����У��	,������ʽΪ����+���룺"<<m_szCallNumber<<endd;
	m_callnumber.Get_Teleno(m_szCallNumber);
	m_callnumber.Set_CallNbr(m_szCallNumber);
	m_iAbnReason=m_callnumber.queryTollcode(m_szAbnField);

	if (m_iAbnReason)
	{
		//������
//		theLog<<"������"<<endd;
		char szLackType[RECORD_LENGTH+1];
		memset(szLackType, 0, sizeof(szLackType));
		sprintf(szLackType, "%d", m_iAbnReason );
		DeleteSpace(szLackType);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, m_szAbnField);
		return;
	}
//	theLog<<"������"<<endd;
	m_callnumber.DealTollcode();
//	theLog<<"ɾ������"<<endd;

	//�Ƿ��������
	if (m_szDealType[1]=='1')
	{
		//��ʱ�����κδ���
	}
	
	//��AfterTollTel����Teleno��
	m_callnumber.Get_AfterTollTel(m_szCallNumber);
//	theLog<<"ȥ����"<<m_szCallNumber<<endd;

	m_callnumber.Set_CallNbr(m_szCallNumber);

	//�Ӵ�֮��Ĺ����������AfterTollTel ����

	//�������Ϊ��
	//�ڹ���ģ��Ὣ����������Ϊ�յĺ����ɽ�����+���ŵ���ʽ�����ź����Ϊ��
	if(strlen(m_szCallNumber)==0)
	{
		//theLog<<"m_szCallNumber��"<<endd;
		getParam();
		sendOutputParams(retValue);
		return;
	}
	
	//��������ֱ����Ĭ������
	m_callnumber.Get_Tollcode(m_szCallNumber);
//	theLog<<"��������ֱ����Ĭ������"<<endd;

	if (strncmp(m_szCallNumber,"00",2)==0 )
	{
	}
	else
	{
		if (m_callnumber.IsGsm())
		{
//			theLog<<"�ֻ���"<<endd;
			//�ֻ���
			//�Ƿ������Ӫ�̺��ƶ�����
			if (m_szDealType[2]=='1')
			{
//				theLog<<"������Ӫ�̺��ƶ�����"<<endd;
				//���Է��������ֻ������ţ�ȡ����ĺ���ǰ���ţ���ѯ���������ϴ������ٸ�����������
				m_iAbnReason=m_callnumber.queryGsm(m_szAbnField);
				if (m_iAbnReason)
				{
//					theLog<<"������"<<endd;
					//������
					char szLackType[RECORD_LENGTH+1];
					memset(szLackType, 0, sizeof(szLackType));
					sprintf(szLackType, "%d", m_iAbnReason );
					DeleteSpace(szLackType);
					pluginAnaResult result=eLackInfo;
					retValue.setAnaResult(result, szLackType, m_szAbnField);
					return;
				}
//				theLog<<"������"<<endd;
				m_callnumber.Set_Tollcode(m_szCallNumber);
			}
		}
		else
		{
//			theLog<<"�̻�"<<endd;
			//�̻�
			//�Ƿ������Ӫ�̺��ƶ�����	,�Ƿ�����ֺ�
			if (m_szDealType[2]=='1' ||m_szDealType[3]=='1' )
			{
//				theLog<<"������Ӫ�̺��ƶ�����	,�����ֺ�"<<endd;
				m_iAbnReason=m_callnumber.queryTeleProperty(m_szAbnField);
				if (m_iAbnReason)
				{
//					theLog<<"������"<<endd;
					//������
					//���ŷ���TELENO_DEF_PROPERTY,�������������й���
					m_iAbnReason=m_callnumber.queryTelDefProperty(m_szAbnField);
//					theLog<<"����TELENO_DEF_PROPERTY"<<endd;
					if (m_iAbnReason)
					{
						char szLackType[RECORD_LENGTH+1];
						memset(szLackType, 0, sizeof(szLackType));
						sprintf(szLackType, "%d", m_iAbnReason );
						DeleteSpace(szLackType);
						pluginAnaResult result=eLackInfo;
						retValue.setAnaResult(result, szLackType, m_szAbnField);
						return;
					}
				}
			}
		}
	}

	//�Ƿ�������ź�ҵ������
	if (m_szDealType[4]=='1')
	{
//		theLog<<"�������ź�ҵ������"<<endd;
		m_callnumber.queryTeleServ(m_szAbnField);
//		theLog<<"���"<<m_szAbnField<<endd;
	}

	getParam();
//	theLog<<"getParam"<<endd;

	sendOutputParams(retValue);
//	theLog<<"sendOutputParams"<<endd;
		
	return;
}


int C_CallProperty::Init(PacketParser &pps)
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

int C_CallProperty::getInputParams(PacketParser & pps)
{
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
	
	memset(m_szBefTollcode,0,sizeof(m_szBefTollcode));
	pps.getFieldValue(2,m_szBefTollcode);
	DeleteSpace( m_szBefTollcode ); 
	
	memset(m_szCallNbr,0,sizeof(m_szCallNbr));
	pps.getFieldValue(3,m_szCallNbr);
	DeleteSpace( m_szCallNbr ); 
	
	memset(m_szNbrType,0,sizeof(m_szNbrType));
	pps.getFieldValue(4,m_szNbrType);
	DeleteSpace( m_szNbrType ); 
	
	memset(m_szCdrBegin,0,sizeof(m_szCdrBegin));
	pps.getFieldValue(5,m_szCdrBegin);
	DeleteSpace( m_szCdrBegin ); 
	
	memset(m_szDealType,0,sizeof(m_szDealType));
	pps.getFieldValue(6,m_szDealType);
	DeleteSpace( m_szDealType ); 

	return 0;
}

//�����롢��Ӫ�̡����������롢�ƶ����͡����ź�ҵ�����͡�
int  C_CallProperty::sendOutputParams(ResParser& retValue)
{
	retValue.setFieldValue( 1, m_szBefTollTel, strlen(m_szBefTollTel) );
	
	if (strcmp(m_szBusiPriority,"0")==0)
		retValue.setFieldValue( 2, m_szBusiBefore, strlen(m_szBusiBefore) );
	else
		retValue.setFieldValue( 2, m_szBusiAfter, strlen(m_szBusiAfter) );
	
	retValue.setFieldValue( 3, m_szDistrict, strlen(m_szDistrict) );
	
	retValue.setFieldValue( 4, m_szMobile, strlen(m_szMobile) );

	retValue.setFieldValue( 5, m_szSvrBefore, strlen(m_szSvrBefore) );

	retValue.setFieldValue( 6, m_szSvrAfter, strlen(m_szSvrAfter) );

	//add by weixy 20080107 
	retValue.setFieldValue( 7, m_szTollcode, strlen(m_szTollcode) );

	return 0;
}

int C_CallProperty::setParam()
{
	m_callnumber.Set_Value();
	m_callnumber.Set_ServCatId(m_szServCatId);
	m_callnumber.Set_BefTollCode(m_szBefTollcode);
	m_callnumber.Set_CallNbr(m_szCallNbr);
	m_callnumber.Set_NbrType(m_szNbrType);
	m_callnumber.Set_CdrBegin(m_szCdrBegin);
	return 0;
}

int C_CallProperty::getParam()
{
	m_callnumber.Get_BefTollTel(m_szBefTollTel);
	m_callnumber.Get_District(m_szDistrict);
	m_callnumber.Get_BusiAfter(m_szBusiAfter);
	m_callnumber.Get_BusiBefore(m_szBusiBefore);
	m_callnumber.Get_BusiPriority(m_szBusiPriority);
	m_callnumber.Get_Mobile(m_szMobile);
	m_callnumber.Get_SvrBefore(m_szSvrBefore);
	m_callnumber.Get_SvrAfter(m_szSvrAfter);
	m_callnumber.Get_Tollcode(m_szTollcode);

	return 0;
}

 /*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_CallProperty::init(char *szSourceGroupID, char *szServiceID, int index)
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
void C_CallProperty::message(MessageParser&  pMessage)
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
void C_CallProperty::printMe()
{
	printf("\t�������:AnaNbrProperty,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_CallProperty::getPluginName()
{
	return "AnaNbrProperty";
}

std::string C_CallProperty::getPluginVersion(){
	return "3.0.0";
}
