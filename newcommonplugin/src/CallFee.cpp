/****************************************************************
  Project	
  Copyright (c)	2005-2006. All Rights Reserved.
		�������ſƼ����޹�˾ 
  SUBSYSTEM:	�û��Զ�����ͷ�ļ�  
  FILE:		C_CallFee.cpp
  AUTHOR:	weixy
  Create Time: 2007-11-19
==================================================================
  Description:  
            
            ��Ҫ����: �Ʒ���ѷ���
				
  UpdateRecord: 
==================================================================

 *****************************************************************/
#include "CallFee.h"

C_CallFee::C_CallFee()
{
	initFlag=0;
	table = NULL;
}

C_CallFee::~C_CallFee()
{
	if(table != NULL)
		delete table;
}

void C_CallFee::execute(PacketParser& pps,ResParser& retValue)
{
	//���ִ�����
	if ( pps.getItem_num() != 5)
	{
		sprintf( ErrorMsg,"��� IsFreeNbr �������������ȷ��");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);		
	}
			
	if( initFlag != 1 )
		Init( pps );

	//�����������

	//�õ��������
	getInputParams(pps);

	//���ݵ�CallNumber ��
	setParam();
	//�������з��������
	char tmp_str[FIELD_LEN];

	if (strcmp(m_szNbrType,"10")==0)
	{
		//���к��룬��ѯ��Ѻ��뼶�𣬲鲻����Ĭ�ϼƷ�
		
		//����ż���
		m_callnumber.Set_ChargeFlag(BEF_TOLL_LEVEL);
		if (!m_callnumber.queryFreeTele())
			getParam();
		else
		{
			//���ż���
			m_callnumber.DealBefTelServ();
			
			m_callnumber.Set_ChargeFlag(TOLL_LEVEL);
			
			if (!m_callnumber.queryFreeTele())
				getParam();
			else
			{
				//���ź󼶱�
				//���ź����ֻ�Թ���������Ч
				m_callnumber.queryTollcode(m_szAbnField);
				m_callnumber.DealTollcode();
				m_callnumber.Get_Tollcode(tmp_str);
				if (strncmp(tmp_str,"00",2)!=0)
				{
					m_callnumber.Get_AfterTollTel(tmp_str);
					m_callnumber.Set_CallNbr(tmp_str);
					m_callnumber.Set_ChargeFlag(AFTER_TOLL_LEVEL);
					if(!m_callnumber.queryFreeTele())
						getParam();
					else
						strcpy(m_szChargeFlag,"Y");	
				}
				else
				{
					strcpy(m_szChargeFlag,"Y");
				}
			}
		}
	}

	else
	{
		//���У��ƷѺ����ѯ����Ʒѣ��鲻����Ĭ�����
		/*
		1���ж��Ƿ�����Ʒ����У����ϱ�free_telheader��teleno_levelΪY��
		2���ж����ⲻ�Ʒ����У����ϱ�free_telheader��teleno_levelΪ1��2��3����3�ּ����ɸ������жϣ�1��ߣ�����2-8 ��ͷ�ĺ����ʱ�е�����Ϣ��������һ���н����ͻ���򸲸�֮ǰ�ж�����Ʒ����е���Ϣ��
		3�������е��κ���Ϣ�ĺ���������������
		*/

		//����ż���
		m_callnumber.Set_ChargeFlag(BEF_TOLL_LEVEL);
		if (!m_callnumber.queryFreeTele())
			getParam();
		else
		{
			//���ż���
			m_callnumber.DealBefTelServ();
			m_callnumber.Set_ChargeFlag(TOLL_LEVEL);
			if (!m_callnumber.queryFreeTele())
				getParam();
			else
			{
				//���뼶��
				m_callnumber.queryTollcode(m_szAbnField);
				m_callnumber.DealTollcode();
				m_callnumber.Get_AfterTollTel(tmp_str);
				m_callnumber.Set_CallNbr(tmp_str);
				m_callnumber.Set_ChargeFlag(AFTER_TOLL_LEVEL);
				if(!m_callnumber.queryFreeTele())
					getParam();
				else
				{
					//û�в��Ʒѵ����ϣ����ռƷѲ���
					m_callnumber.Set_ChargeFlag("Y");
					if(!m_callnumber.queryFreeTele())
						getParam();
					else
					{
						//Ҳû�мƷѵ����ϣ���Ĭ�Ϲ���
						pluginAnaResult result=eLackInfo;
						sprintf(m_sAbnReason,"%d",m_iAbnReason);
						retValue.setAnaResult(result, m_sAbnReason, tmp_str);
						return;
					}	
				}//���뼶��	
			}//���ż���
		}//�����뼶��
	}//����

	sendOutputParams(retValue);
	return;
}

int C_CallFee::Init(PacketParser &pps)
{
	/*get some of input params which will not change after first coming*/

	memset(m_LastSerCatId,0,sizeof(m_LastSerCatId));
	pps.getFieldValue(1,m_LastSerCatId);
	DeleteSpace( m_LastSerCatId ); 

	memset(m_LastSourceId, 0, sizeof(m_LastSourceId));
	strcpy(m_LastSourceId, m_szSourceId);
	DeleteSpace( m_LastSourceId ); 
	
	//��ʼ��������
	//m_callnumber.Init(m_szServiceId, m_LastSourceId, table);
	m_callnumber.Reset(m_szServiceId, m_LastSourceId);

	/* set init_flag of this plugin */
	initFlag=1;
	return 0;
}



int C_CallFee::getInputParams(PacketParser & pps)
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

	return 0;
}


int  C_CallFee::sendOutputParams(ResParser& retValue)
{
	retValue.setFieldValue( 1, m_szChargeFlag, strlen(m_szChargeFlag) );

	return 0;
}

int C_CallFee::setParam()
{
	m_callnumber.Set_Value();
	m_callnumber.Set_ServCatId(m_szServCatId);
	m_callnumber.Set_BefTollCode(m_szBefTollcode);
	m_callnumber.Set_CallNbr(m_szCallNbr);
	m_callnumber.Set_NbrType(m_szNbrType);
	m_callnumber.Set_CdrBegin(m_szCdrBegin);
	return 0;
}

int C_CallFee::getParam()
{
	m_callnumber.Get_ChargeFlag(m_szChargeFlag);
	return 0;
}

/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_CallFee::init(char *szSourceGroupID, char *szServiceID, int index)
{
	memset( m_szSourceGroupId, 0, sizeof( m_szSourceGroupId ) );   
	strcpy( m_szSourceGroupId, szSourceGroupID );
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
void C_CallFee::message(MessageParser&  pMessage)
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
void C_CallFee::printMe()
{
	printf("\t�������:IsFreeNbr,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_CallFee::getPluginName()
{
	return "IsFreeNbr";
}

std::string C_CallFee::getPluginVersion(){
	return "3.0.0";
}
