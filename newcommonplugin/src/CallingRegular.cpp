/****************************************************************
 filename: CallingRegular.cpp
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
            ��������ҵ��;�Ʒ�ҵ��Ʒѽ׶����к�����������ں�������
            ��Ҫ����: 
				��Ϻ������ϣ������С����С��ƷѵȺ���ͳһ����Ϊ"����+����"�ĸ�ʽ�����н����������ָ���ֶΡ�
 update:
�����У� 
   1�����Ž���ҵ��a����13800138000��013800138000��������ɡ�020+13800138000���� 
                   b����0+����+1380013800���룬ȡԭ������Ϊ138����Ĺ������ţ������к�������� 
	   ���Ž���ҵ����������֮ǰ��֤���������ʽΪ����+ 13800138000
   2������ҵ��13800138000��013800138000��0+����+13800138000����������
 *****************************************************************/
#include "CallingRegular.h"

C_CallingRegular::C_CallingRegular()
{
	memset(m_szSourceGroupId,0,sizeof(m_szSourceGroupId));
	memset(m_szServCatId,0,sizeof(m_szServCatId));
	memset(m_szSourceId,0,sizeof(m_szSourceId));
	memset(m_szCallNbr,0,sizeof(m_szCallNbr));
	memset(m_szNbrType,0,sizeof(m_szNbrType));
	memset(m_szCdrBegin,0,sizeof(m_szCdrBegin));
	memset(m_szDealType,0,sizeof(m_szDealType));
	memset(m_szDefTollcode,0,sizeof(m_szDefTollcode));
	memset(m_szBefTollTel,0,sizeof(m_szBefTollTel));//������
	memset(m_szTollTel,0,sizeof(m_szTollTel));//��0����+���루�̻����ֻ����룩
	memset(m_szAfterTollTel,0,sizeof(m_szAfterTollTel));	//���루�̻����ֻ����룩
	memset(m_szTollcode,0,sizeof(m_szTollcode));//��0����
	memset(m_szCallNumber,0,sizeof(m_szCallNumber));//������+��0����+���루�̻����ֻ����룩
	memset(m_sAbnReason,0,sizeof(m_sAbnReason));//������ԭ��
	memset(m_szAbnField,0,sizeof(m_szAbnField));//����������
	//m_iProcessId=0;
	m_iAbnReason=0;
	initFlag=0;
	table = NULL;
}

C_CallingRegular::~C_CallingRegular()
{
}

/*************************************************
* ����    �����к������
* ��ڲ�����StdChkCallingNbr(SOURCEGROUP_ID,PROCESS_ID,SERV_CAT_ID,SOURCE_ID,CALL_NBR,NBR_TYPE,CDR_BEGIN,DEAL_TYPE,DEF_TOLLCODE)
* ���ڲ�����
* ����    ��
**************************************************/
void C_CallingRegular::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[MSG_LEN];
	
	//���ִ�����
	if ( pps.getItem_num() != 6 )
	{
		sprintf( ErrorMsg,"��� StdChkCallingNbr �������������ȷ��");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);		
	}
			
	if( initFlag != 1 )
		Init( pps );
	
	//�����������
	//���й���

	//�õ��������
	getInputParams(pps);

	//���ݵ�CallNumber ��
	setParam();

	//�жϷϺ���ͷ,���������ɾ��
	m_callnumber.DealRubTelHead();

	//�ж��Ƿ����ֻ���
	if(m_callnumber.IsGsm())
	{	
		//��ѯ֮ǰȥ���ֻ���֮ǰ��0
		m_callnumber.DealGsm();
		//��H����ȡ����
		m_iAbnReason=m_callnumber.queryGsm(m_szAbnField);
		if (m_iAbnReason)
		{
			//������
			pluginAnaResult result=eLackInfo;
			sprintf(m_sAbnReason,"%d",m_iAbnReason);
			retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
			//reOutputNum(pps,retValue);
			return;
		}

		//��������+�ֻ���
		m_callnumber.ConnectTollGsm();
	}
	else
	{
		if (m_callnumber.IsZeroHead())
		{
			//��0��ͷ
			m_iAbnReason=m_callnumber.queryTollcode(m_szAbnField);
			if (m_iAbnReason)
			{
				//������
				pluginAnaResult result=eLackInfo;
				sprintf(m_sAbnReason,"%d",m_iAbnReason);
				retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
				//reOutputNum(pps,retValue);
				return;
					
			}
			//����+�����ݲ����д���
		}
		else
		{
			//add by weixy 0080128 �޸ķ�������
			m_iAbnReason=m_callnumber.Check10Len(m_szAbnField);
			if (m_iAbnReason==0)
			{
				//���볤�ȴ��ڵ���10λ
				//���벹0�����·�������
				m_callnumber.AddZero();
				m_iAbnReason=m_callnumber.queryTollcode(m_szAbnField);
				if (m_iAbnReason)
				{
					//������
					//����ѡ������볤�ȴ���10����0�Ժ����Ҳ������ŵģ�����������
					if(m_szDealType[0]!='3')
					{
						pluginAnaResult result=eLackInfo;
						sprintf(m_sAbnReason,"%d",m_iAbnReason);
						retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
						//reOutputNum(pps,retValue);
						return;
					}
					m_callnumber.AddDefTollcode();	
				}
			}
			else
			{
				//�жϱ�����Դ�Ƿ���ȱ����
				if(m_szDealType[0]=='1')
				//������Դ����ȱ����
				{
					//�����ϳ���С��10λ�����Ҳ���ȱ���ţ�ֱ�ӹ���
					pluginAnaResult result=eLackInfo;
					sprintf(m_sAbnReason,"%d",m_iAbnReason);
					retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
					//reOutputNum(pps,retValue);
					return;
				}
				else
				{
					//������Դ����ȱ����
					
					//Ĭ�������Ƿ���ʡ��
					m_iAbnReason=m_callnumber.IsDefTollProvince(m_szAbnField);
					if(m_iAbnReason==1)
					{
						//Ĭ��������ʡ��
						
						//���볤���Ƿ���Ĭ�����Ź���������һ��
						if (m_callnumber.IsTeleLengthRight())
						{
							//���볤��һ��
							//�Ƿ���ȱ0Ĭ�����ſ�ͷ
							if (m_callnumber.IsDefTollHead()==0)
							{
								//�Ƿ��оֺ�����
								if (!m_callnumber.queryTeleProperty(m_szAbnField))
								{
									//�оֺ����ϣ���Ĭ������
									m_callnumber.AddDefTollcode();
								}
								else
								{
									//û�оֺ�����
									//���벹0��
									m_callnumber.AddZero();
								}
							}
							else
							{
								//��Ĭ������
								m_callnumber.AddDefTollcode();
							}
						}
						else
						{
							//���볤�Ȳ�һ��
							//������ȱ0Ĭ�����ſ�ͷ
							if (m_callnumber.IsDefTollHead()==0)
							{
								//���벹0�����·�������
								m_callnumber.AddZero();
							}
							else
							{
								//û�����ţ���Ĭ������
								m_callnumber.AddDefTollcode();
							}
						}
					}//end of Ĭ��������ʡ��
					else if(m_iAbnReason ==0 )
					{
						//Ĭ�����Ų���ʡ��
						//���벹0�����·�������
						m_callnumber.AddZero();
						//û�����ţ���Ĭ������
						if (m_callnumber.queryTollcode(m_szAbnField))
							m_callnumber.AddDefTollcode();
					}
					else
					{
						//������
						pluginAnaResult result=eLackInfo;
						sprintf(m_sAbnReason,"%d",m_iAbnReason);
						retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
						//reOutputNum(pps,retValue);
						return;
					}
				}//end of ������Դ��ȱ����
			}
		}
	}//end of �̻�
	
	//��CallNumber �������ĺ��봫�ݵ�����
	m_callnumber.Get_Teleno(m_szCallNumber);
	
	//����У��	,������ʽΪ ����+����
	m_callnumber.Set_CallNbr(m_szCallNumber);
	m_callnumber.queryTollcode(m_szAbnField);
	m_callnumber.DealTollcode();
	
	//��AfterTollTel����Teleno��
	m_callnumber.Get_AfterTollTel(m_szCallNumber);
	m_callnumber.Set_CallNbr(m_szCallNumber);
	
	//�Ӵ�֮��Ĺ����������AfterTollTel ����
	if (m_szDealType[1]=='1')
	{
		//У��˫����	
		if (m_iAbnReason=m_callnumber.Check2Tollcode(m_szAbnField))
		{
			//������
			//˫���Ź���ԭ����д��ڵ��еĺ���
			pluginAnaResult result=eLackInfo;
			sprintf(m_sAbnReason,"%d",m_iAbnReason);
			retValue.setAnaResult(result, m_sAbnReason, m_szCallNbr);
			//reOutputNum(pps,retValue);
			return;
		}
		
	}
	
	if (m_szDealType[2]!='0')
	{
		int checkLen;
		if (m_szDealType[2]=='Y') 
			checkLen=0;
		else 
			checkLen=m_szDealType[2]-48;
		//У��Ƿ��ַ�
		if (m_iAbnReason=m_callnumber.CheckValid(checkLen,m_szAbnField))
		{
			pluginAnaResult result=eLackInfo;
			sprintf(m_sAbnReason,"%d",m_iAbnReason);
			retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
			//reOutputNum(pps,retValue);
			return;
		}
	}
	
	if (m_callnumber.IsForeign())
	{
		if (m_szDealType[3]=='1')
		//У�����ź���볤�ȴ��ڵ���3λ
		{
			if(m_iAbnReason=m_callnumber.CheckLen(m_szAbnField))
			{
				pluginAnaResult result=eLackInfo;
				sprintf(m_sAbnReason,"%d",m_iAbnReason);
				retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
				//reOutputNum(pps,retValue);
				return;
			}
		}

		if (m_szDealType[4]=='1')
		//У��ʡ��2��8����κ��볤�ȵĺϷ���
		{
			if(m_iAbnReason=m_callnumber.CheckGDLen(m_szAbnField))
			{
				pluginAnaResult result=eLackInfo;
				sprintf(m_sAbnReason,"%d",m_iAbnReason);
				retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
				//reOutputNum(pps,retValue);
				return;
			}
		}
	}

	else
	{
		//�ж��Ƿ����ֻ���
		if(m_callnumber.IsGsm())
		{
			//�ֻ���
			//����У�鷽ʽ2��3 �ֱ��ʾ���Ž��㣬��;�Ʒ�
			if (m_szDealType[5]!='0')
			{
				//��У��13800138000��Ȼ���ڴ�����������
				m_callnumber.Get_AfterTollTel(m_szCallNumber);
				if (strcmp(m_szCallNumber,"13800138000")==0)
				{
					if (m_szDealType[5]=='2')
					{
						//���Ž���ҵ�񲻹���
						//ֱ������������֤����������+13800138000����������0GD+13800138000�ĺ���
					}
					else
					{
						//����ҵ�����
						pluginAnaResult result=eLackInfo;
						sprintf(m_sAbnReason,"%d",ABN_CALLING_138);
						retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
						//reOutputNum(pps,retValue);
						return;
					}
				}
				else
				{
					//У���ֻ���ǰ���ź�H���һ����
					if(m_iAbnReason=m_callnumber.CheckGsm(m_szAbnField))
					{
						//�澯�������
						getParam();
						sendOutputParams(retValue);
						
						//��ԭʼ������������ԭ���ֶ�
						pps.getFieldValue(2,m_szCallNbr);
						DeleteSpace( m_szCallNbr ); 
						pluginAnaResult result=eAbnormal;
						sprintf(m_sAbnReason,"%d",m_iAbnReason);
						retValue.setAnaResult(result, m_sAbnReason, m_szCallNbr);
						return;
					}
				}
			}
			if (m_szDealType[6]=='1')
			{
				//У���ֻ����볤�ȵĺϷ���
				if (m_iAbnReason=m_callnumber.CheckGsmLen(m_szAbnField))
				{
					pluginAnaResult result=eLackInfo;
					sprintf(m_sAbnReason,"%d",m_iAbnReason);
					retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
					//reOutputNum(pps,retValue);
					return;
				}
			}
		}//end of �ֻ���
		else
		{	
			//�̻�
			if (m_szDealType[3]=='1')
			//У�����ź���볤�ȴ��ڵ���3λ
			{
				if(m_iAbnReason=m_callnumber.CheckLen(m_szAbnField))
				{
					pluginAnaResult result=eLackInfo;
					sprintf(m_sAbnReason,"%d",m_iAbnReason);
					retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
					//reOutputNum(pps,retValue);
					return;
				}
			}
			if (m_szDealType[4]=='1')
			//У��ʡ��2��8����κ��볤�ȵĺϷ���
			{
				if(m_iAbnReason=m_callnumber.CheckGDLen(m_szAbnField))
				{
					pluginAnaResult result=eLackInfo;
					sprintf(m_sAbnReason,"%d",m_iAbnReason);
					retValue.setAnaResult(result, m_sAbnReason, m_szAbnField);
					//reOutputNum(pps,retValue);
					return;
				}
			}
		}//end of �̻�
	}

	//���ִ�г���
	getParam();
	sendOutputParams(retValue);
	return;
}


int C_CallingRegular::Init(PacketParser &pps)
{
	/*get some of input params which will not change after first coming*/
	/*
	memset( m_szPipeId, 0, sizeof( m_szPipeId ) );   
	pps.getFieldValue( 1,m_szPipeId );
	DeleteSpace(m_szPipeId ); 

	char szProcessId[FIELD_LEN];
	memset( szProcessId, 0, sizeof( szProcessId ) );   
	pps.getFieldValue( 2,szProcessId );
	DeleteSpace( szProcessId ); 
  	m_iProcessId = atoi( szProcessId );
	*/
	
	memset(m_LastSerCatId,0,sizeof(m_LastSerCatId));
	pps.getFieldValue(1,m_LastSerCatId);
	DeleteSpace( m_LastSerCatId ); 

	memset(m_LastSourceId,0,sizeof(m_LastSourceId));
	strcpy(m_LastSourceId, m_szSourceId);
	DeleteSpace( m_LastSourceId ); 

	strcpy(m_szIniPath, getenv("ZHJS_INI"));
	
	//��ʼ��������
	//m_callnumber.Init(m_szServiceId, m_LastSourceId, table);
	m_callnumber.Reset(m_szServiceId, m_LastSourceId);

	initFlag=1;

	return 0;
}

int C_CallingRegular::getInputParams(PacketParser & pps)
{
	//��ȡ��������ɱ����
	//��ȡ�����Ӧ��ҵ����룬��ȥ��ѯʵ�ʵ��������
	memset(m_szServCatId,0,sizeof(m_szServCatId));
	pps.getFieldValue(1,m_szServCatId);
	DeleteSpace( m_szServCatId ); 

	/*
	memset(m_szSourceId,0,sizeof(m_szSourceId));
	pps.getFieldValue(4,m_szSourceId);
	DeleteSpace( m_szSourceId ); 
	*/
	
	//��ʼ��������
	if (strcmp(m_LastSerCatId,m_szServCatId)!=0  ||  strcmp(m_LastSourceId,m_szSourceId)!=0  )
	{
		strcpy(m_LastSerCatId, m_szServCatId);
		strcpy(m_LastSourceId, m_szSourceId);
		//m_callnumber.Init(m_szServiceId, m_LastSourceId, table);
		m_callnumber.Reset(m_szServiceId, m_LastSourceId);
	}
	
	memset(m_szCallNbr,0,sizeof(m_szCallNbr));
	pps.getFieldValue(2,m_szCallNbr);
	DeleteSpace( m_szCallNbr ); 
	
	memset(m_szNbrType,0,sizeof(m_szNbrType));
	pps.getFieldValue(3,m_szNbrType);
	DeleteSpace( m_szNbrType ); 
	
	memset(m_szCdrBegin,0,sizeof(m_szCdrBegin));
	pps.getFieldValue(4,m_szCdrBegin);
	DeleteSpace( m_szCdrBegin ); 
	
	memset(m_szDealType,0,sizeof(m_szDealType));
	pps.getFieldValue(5,m_szDealType);
	DeleteSpace( m_szDealType ); 

	memset(m_szDefTollcode,0,sizeof(m_szDefTollcode));
	pps.getFieldValue(6,m_szDefTollcode);
	DeleteSpace( m_szDefTollcode ); 
	
	return 0;
}


int  C_CallingRegular::sendOutputParams(ResParser& retValue)
{
	retValue.setFieldValue( 1, m_szBefTollTel, strlen(m_szBefTollTel) );
	retValue.setFieldValue( 2, m_szTollcode, strlen(m_szTollcode) );
	retValue.setFieldValue( 3, m_szAfterTollTel, strlen(m_szAfterTollTel) );
	retValue.setFieldValue( 4, m_szTollTel, strlen(m_szTollTel) );
	retValue.setFieldValue( 5, m_szCallNumber, strlen(m_szCallNumber) );
	return 0;
}

int C_CallingRegular::setParam()
{
	m_callnumber.Set_Value();
	m_callnumber.Set_ServCatId(m_szServCatId);
	m_callnumber.Set_SourceId(m_szSourceId);
	m_callnumber.Set_CallNbr(m_szCallNbr);
	m_callnumber.Set_NbrType(m_szNbrType);
	m_callnumber.Set_CdrBegin(m_szCdrBegin);
	m_callnumber.Set_DefTollcode(m_szDefTollcode);	
	return 0;
}

int C_CallingRegular::getParam()
{
	m_callnumber.Get_BefTollTel(m_szBefTollTel);
	m_callnumber.Get_Tollcode(m_szTollcode);
	m_callnumber.Get_AfterTollTel(m_szAfterTollTel);

	sprintf(m_szTollTel,"%s%s",m_szTollcode,m_szAfterTollTel);
	sprintf(m_szCallNumber,"%s%s",m_szBefTollTel,m_szTollTel);

	return 0;
}


int C_CallingRegular::reOutputNum(PacketParser & pps,ResParser& retValue)
{
	//���������д�������У�ֻ�������ϳ���ʱʹ��
	pps.getFieldValue(2,m_szCallNbr);
	DeleteSpace( m_szCallNbr ); 
	retValue.setFieldValue( 5, m_szCallNbr, strlen(m_szCallNbr) );
	return 0;
}


/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_CallingRegular::init(char *szSourceGroupID, char *szServiceID, int index)
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
void C_CallingRegular::message(MessageParser&  pMessage)
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
void C_CallingRegular::printMe()
{
	printf("\t�������:StdChkCallingNbr,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_CallingRegular::getPluginName()
{
	return "StdChkCallingNbr";
}

std::string C_CallingRegular::getPluginVersion(){
	return "3.0.0";
}


