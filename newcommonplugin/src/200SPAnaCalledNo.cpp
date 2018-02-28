#include "200SPAnaCalledNo.h"

/*
extern char ErrorMsg[MSG_LEN];
static int g_iCurTimeLen = 14;
*/

/*************************************************
* ����    �����캯��
* ��ڲ�������
* ���ڲ�������SERV_CAT_ID||VALID_FLAG||TELENO_HEADER
* ����    ����
**************************************************/
C_200SPAnaCalledNo::C_200SPAnaCalledNo()
{
	/*
	lackinfo = 0;

	strcpy( m_TelHeaderDelete.m_szTableName, "TELHEADER_DELETE" );
	strcpy( m_BefTollcodeService.m_szTableName, "BEFTOLLCODE_SERVICE" );
	strcpy( m_Tollcode.m_szTableName, "TOLLCODE" );
	strcpy( m_TollcodeRegionFlag.m_szTableName, "TOLLCODE_REGIONFLAG" );
	strcpy( m_TollcodeTeleno.m_szTableName, "TOLLCODE_TELENO" );
	strcpy( m_TelenoProperty.m_szTableName, "TELENO_PROPERTY" );
	strcpy( m_TollStruct.m_szTableName, "TOLLSTRUCTURE" );

	memset(m_szLastSource,0,sizeof(m_szLastSource));
	*/
	table = NULL;
}

/*************************************************
* ����    ����������
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_200SPAnaCalledNo::~C_200SPAnaCalledNo()
{
	table == NULL;
}

void C_200SPAnaCalledNo::init(char *szSourceGroupID, char *szServiceID, int index)
{
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	if( (m_iTollcodeOffset = table->getTableOffset("I_TOLLCODE")) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TOLLCODEʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iLocalnetOffset = table->getTableOffset("I_LOCALNET")) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_LOCALNETʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTelenoServiceOffset = table->getTableOffset("I_TELENO_SERVICE")) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TELENO_SERVICEʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
}

void C_200SPAnaCalledNo::message(MessageParser&  pMessage)
{
}

void C_200SPAnaCalledNo::printMe()
{
	printf("\t�������:AnaCalledNo,�汾�ţ�3.0.0 \n");
}

std::string C_200SPAnaCalledNo::getPluginName()
{
	return "AnaCalledNo";
}

std::string C_200SPAnaCalledNo::getPluginVersion(){
	return "3.0.0";
}

/*************************************************
* ����    ��ִ�к���,�������к��룬���������Ҫ��Ϣ��
* ��ڲ�����$CalledNo, $Cdrbegin, $Call_location, $Card_location, $ServCatId
* ���ڲ�����$CalledNo, $Fee_Type, $CalledTollcode, $Localnet, $service_id
* ����    ��0 ���� �����ϴ���
**************************************************/
void C_200SPAnaCalledNo::execute(PacketParser& pps,ResParser& retValue)
{
	if (pps.getItem_num() != 5)
	{
		sprintf( ErrorMsg,"the params sent to 200SPAnaCalledNo is lack");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}

	memset( CalledNo, 0, sizeof( CalledNo ) );
	memset( Call_Location, 0, sizeof( Call_Location ) );
	memset( Card_Location, 0, sizeof( Card_Location ) );
	memset( CdrBegin, 0, sizeof( CdrBegin ) );
	memset( ServCatId, 0, sizeof( ServCatId ) );
	memset( Tollcode, 0, sizeof( Tollcode ) );

	pps.getFieldValue( 1, CalledNo );
	pps.getFieldValue( 2, CdrBegin );
	pps.getFieldValue( 3, Call_Location );
	pps.getFieldValue( 4, Card_Location );
	pps.getFieldValue( 5, ServCatId );

	DeleteSpace( CalledNo );
	DeleteSpace( CdrBegin );
	DeleteSpace( Call_Location );
	DeleteSpace( Card_Location );
	DeleteSpace( ServCatId );

	if ( !(anaTollcode(pps,retValue) 
		&& anaFeeType(pps,retValue) 
		&& anaLocalnet(pps,retValue) 
		&& andService_id(pps,retValue)) )
	{
		//������
		memset(abn_value, 0, sizeof(abn_value));
		memset(abn_content, 0, sizeof(abn_content));
		pluginAnaResult result=eLackInfo;
		sprintf(abn_value, "%d", lackinfo);
		sprintf(abn_content, "%s", lackreason);
		retValue.setAnaResult(result, abn_value, abn_content);
	}

}


/*************************************************
* ����    ����������,ʡ������Ŵ��־�ּ�ʡ�����Ž�ȥ
* ��ڲ�����$CalledNo
* ���ڲ�����$CalledNo
* ����    ��0, �����ϴ���
**************************************************/
bool C_200SPAnaCalledNo::anaTollcode( PacketParser& pps, ResParser& retValue )
{
	//ͨ�����к����������ţ�
	//1.��ʡ���ţ���ȥ return true
	//2.��ʡ���ţ����־ lackinfo = 0 return false
	//3.�����ţ��������� return true
	//�����ҵ����Ĭ�ϵ�ʡ�����ŵģ����Ĭ�ϵ�����
	char szTmpTeleno[RECORD_LENGTH+1];
	memset( szTmpTeleno, 0, sizeof( szTmpTeleno ) );
	strcpy( szTmpTeleno, CalledNo);

	if(szTmpTeleno[0] != '0')
	{
		//����0��ͷ�ģ���Ϊû�����ţ�ֱ�ӷ���
		return true;
	}
	else
	{
		m_InData.clear();
		m_OutData.clear();
		m_InData.set(szTmpTeleno);
		m_InData.itemNum = 1;
		strcpy(m_InData.startTime, CdrBegin);
		if( table->getData(m_iTollcodeOffset, &m_InData, &m_OutData, 1) == 0 )
		{
			memset( Tollcode, 0, sizeof(Tollcode) );
			strcpy( Tollcode, m_OutData.values[0] );
			sprintf( CalledNo, "%s", &CalledNo[strlen(Tollcode)] );
			//�Ƿ�ʡ
			m_InData.clear();
			m_OutData.clear();
			m_InData.set(Tollcode);
			m_InData.itemNum = 1;
			if( table->getData(m_iLocalnetOffset, &m_InData, &m_OutData, 1) != 0 )
			{
				//��ʡ
				retValue.setFieldValue( 4, "NGD", sizeof("NGD") );
				pps.getFieldValue( 1, CalledNo );
				retValue.setFieldValue( 1, CalledNo, strlen(CalledNo) );
				lackinfo = 0;
				//return false;
				return true;
			}
			retValue.setFieldValue( 3, Tollcode, sizeof(Tollcode) );
		}
		else
		{
			lackinfo = LACK_CALLED_TOLLCODE;
			pps.getFieldValue( 1, CalledNo );
			sprintf(lackreason, "%s", CalledNo);
			//retValue.setFieldValue( 1, CalledNo, strlen(CalledNo) );
			return false;
		}
	}
	return true;
}

/*************************************************
* ����    ������ʹ�����ͣ�ͨ����������������Ʒ�����
* ��ڲ�����$CalledNo
* ���ڲ�����$CalledNo_Ana, $Fee_Type
* ����    ��true, false
**************************************************/
bool C_200SPAnaCalledNo::anaFeeType(PacketParser& pps, ResParser& retValue)
{
	memset(CalledNo_Ana, 0, sizeof(CalledNo_Ana));
	if ( checkFormat( CalledNo, "*A" ) )
	{
		strncpy(CalledNo_Ana, CalledNo, strlen(CalledNo)-1);
		retValue.setFieldValue( 1, CalledNo_Ana, strlen(CalledNo_Ana) );
		retValue.setFieldValue( 2, "10", strlen("10") );
	}
	else if ( checkFormat( CalledNo, "*B" ) )
	{
		strncpy(CalledNo_Ana, CalledNo, strlen(CalledNo)-1);
		retValue.setFieldValue( 1, CalledNo_Ana, strlen(CalledNo_Ana) );
		retValue.setFieldValue( 2, "20", strlen("20") );
	}
	else if ( checkFormat( CalledNo, "*C" ) )
	{
		strncpy(CalledNo_Ana, CalledNo, strlen(CalledNo)-1);
		retValue.setFieldValue( 1, CalledNo_Ana, strlen(CalledNo_Ana) );
		retValue.setFieldValue( 2, "11", strlen("11") );
	}
	else if ( CalledNo[strlen(CalledNo)-1] < '0' || CalledNo[strlen(CalledNo)-1] > '9' )
	{
		lackinfo = ABN_CALLED_LEN;
		//pps.getFieldValue( 1, CalledNo );
		sprintf(lackreason, "%s", CalledNo);
		//retValue.setFieldValue( 1, CalledNo, strlen(CalledNo) );
		return false;
	}
	else
	{
		strcpy(CalledNo_Ana, CalledNo);
		retValue.setFieldValue( 1, CalledNo, strlen(CalledNo) );
		retValue.setFieldValue( 2, "30", strlen("30") );
	}
	return true;
}


/*************************************************
* ����    ��������������ʾ��ͨ�������أ������ط�����
* ��ڲ�����$Tollcode
* ���ڲ�����$Localnet
* ����    ��true, false
**************************************************/
bool C_200SPAnaCalledNo::anaLocalnet(PacketParser& pps, ResParser& retValue)
{
	if ( strlen(Call_Location) != 0 )
	{
		sprintf( Tollcode, "%s", Call_Location );
	}
	else
	{
		if ( strlen(Card_Location) == 0 )
		{
			lackinfo = LACK_CARD_LOCATION;
			pps.getFieldValue( 1, CalledNo );
			sprintf(lackreason, "%s", CalledNo);
			//retValue.setFieldValue( 1, CalledNo, strlen(CalledNo) );
			return false;
		}
		else
		{
			sprintf( Tollcode, "%s", Card_Location );
		}
	}
	char szTmp[RECORD_LENGTH+1];
	memset(szTmp, 0, sizeof(szTmp));
	m_InData.clear();
	m_OutData.clear();
	m_InData.set(Tollcode);
	m_InData.itemNum = 1;
	if( table->getData(m_iLocalnetOffset, &m_InData, &m_OutData, 1) == 0 )
	{
		strcpy(szTmp, m_OutData.values[1]);
	}
	else
	{
		//��ʡ
		m_InData.clear();
		m_OutData.clear();
		m_InData.set(Tollcode);
		m_InData.itemNum = 1;
		if( table->getData(m_iTollcodeOffset, &m_InData, &m_OutData, 1) != 0 )
		{
			lackinfo = LACK_CALLED_TOLLCODE;	//��������
			pps.getFieldValue( 1, CalledNo );
			sprintf(lackreason, "%s", CalledNo);
			//retValue.setFieldValue( 1, CalledNo, strlen(CalledNo) );
			return false;
		}
		retValue.setFieldValue( 4, "NGD", sizeof("NGD") );
		pps.getFieldValue( 1, CalledNo );
		retValue.setFieldValue( 1, CalledNo, strlen(CalledNo) );
		lackinfo = 0;
		//return false;
		return true;
	}
	retValue.setFieldValue( 4, szTmp, strlen(szTmp) );
	return true;
}

/*************************************************
* ����    ������SP_ABBR��ͨ�����к���ͷ����
* ��ڲ�����$CalledNo
* ���ڲ�����$service_id
* ����    ��true, false
**************************************************/
bool C_200SPAnaCalledNo::andService_id(PacketParser& pps, ResParser& retValue)
{
	char szTmpTeleno[RECORD_LENGTH+1];
	memset(szTmpTeleno, 0, sizeof(szTmpTeleno));
	strcpy( szTmpTeleno, CalledNo);

	m_InData.clear();
	m_OutData.clear();
	m_InData.set(ServCatId, 0);
	m_InData.set("10", 0);
	//m_InData.set(Tollcode, 0);
	m_InData.set(szTmpTeleno, 0);
	m_InData.itemNum = 1;
	strcpy(m_InData.startTime, CdrBegin);
	if( table->getData(m_iTelenoServiceOffset, &m_InData, &m_OutData, 2) == 0 )
	{
		retValue.setFieldValue( 5, m_OutData.values[6], strlen(m_OutData.values[6]) );
		return true;
	}
	lackinfo = LACK_CALLED_TELENO_PROPERTY;
	pps.getFieldValue( 1, CalledNo );
	sprintf(lackreason, "%s10%s", ServCatId, CalledNo);
	//retValue.setFieldValue( 1, szTmpTeleno, strlen(szTmpTeleno) );
  	return false;
}

//һ�������Ƚ��ַ����Ƿ����һ����ʽ
/**************************************************
*	Function Name:	checkFormat
*	Description:	�Ƚ������ַ����Ƿ�ƥ�䣨��ȣ�
*	Input Param:
*		cmpString -------> ���Ƚϵ��ַ���
*		format	   -------> ƥ����ַ�����֧��*,?,[]��ͨ���
*	Returns:
*		��������ַ���ƥ�䣬����SUC
*		��������ַ�����ƥ�䣬����FAIL
******************************************************/
bool C_200SPAnaCalledNo::checkFormat(const char *cmpString,const char *format)
{
	while(1)
	{
		switch(*format)
	  	{
	  		case '\0':
					if (*cmpString == '\0')
					{
						return true;
					}
					else
					{
						return false;
					}
			case '!':
					if (checkFormat(cmpString,format + 1) == true)
					{
						return false;
					}
					else
					{
						return true;
					}
			case '?' :
					if(*cmpString == '\0')
					{
						return false;
					}
					return checkFormat(cmpString + 1,format + 1);
			case '*' :
					if(*(format+1) == '\0')
					{
						return true;
					}
					do
					{
						if(checkFormat(cmpString,format+1)==true)
						{
							return true;
						}
					}while(*(cmpString++));
					return false;
			case '[' :
					format++;
					do
					{
						
						if(*format == *cmpString)
						{
							while(*format != '\0' && *format != ']')
							{
								format++;
							}
							if(*format == ']')
							{
								format++;
							}
							return checkFormat(cmpString+1,format);			
						}
						format++;
						if((*format == ':') && (*(format+1) != ']'))
						{
							if((*cmpString >= *(format - 1)) && (*cmpString <= *(format + 1)))
							{
								while(*format != '\0' && *format != ']')
								{
									format++;
								}
								if(*format == ']')
								{
									format++;
								}
								return checkFormat(cmpString+1,format);
							}
							format++;
							format++;

						}
					}while(*format != '\0' && *format != ']');

					return false;
			default  :
					if(*cmpString == *format)
					{
						return checkFormat(cmpString+1,format+1);
					}
					else
					{
						return false;
					}
		}
	}
}
