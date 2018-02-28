#include "AnaCallDirect.h"


//==============================================================================
/*************************************************
* ����    ����ʽУ��ĺ�����֧��*��?
* ��ڲ�������Ҫ����У����ַ������͸�ʽ�����ַ���
* ���ڲ�������
* ����    ��true ���� false
**************************************************/
bool checkFormat(const char *cmpString,const char *format)
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

//==============================================================================

/*************************************************
* ����    �����캯��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_AnaCallDirect::C_AnaCallDirect()
{
	table = NULL;
}


/*************************************************
* ����    ����������
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_AnaCallDirect::~C_AnaCallDirect()
{
//	table_tollcode->detach();
//	delete table_tollcode;
}

/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_AnaCallDirect::init(char *szSourceGroupID, char *szServiceID, int index)
{
	strcpy(m_szSourceGroupId, szSourceGroupID);
	strcpy(m_szServiceId, szServiceID);
	//strcpy(m_szIniPath, getenv("ZHJS_INI"));
	iVersion = 1;
	iIndex = 1;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_TOLLCODE");
	m_iTableOffset = table->getTableOffset(m_szTableName);
}
		
/*************************************************
* ����    ������ͨ������
* ��ڲ�����AnaCallDirect($SOURCEGROUP_ID,$SERVICE_ID,$TOLLCODEA,$TOLLCODEB)
* ���ڲ�����ͨ������
* ����    ��
**************************************************/
void C_AnaCallDirect::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	char LogMsg[ERROR_MSG_LEN+1];

	if (pps.getItem_num() != 2)
	{
		sprintf( ErrorMsg,"��� AnaCallDirect �������������ȷ��");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
  	
	memset( tollcode1, 0, sizeof(tollcode1) );
	memset( tollcode2, 0, sizeof(tollcode2) );
	memset( region1, 0, sizeof(region1) );
	memset( region2, 0, sizeof(region2) );
	
	pps.getFieldValue( 1, tollcode1 );
	pps.getFieldValue( 2, tollcode2 );
	
	DeleteSpace( tollcode1 );	
	DeleteSpace( tollcode2 );
	
	if ( strcmp( tollcode1, tollcode2 )==0 )
	{
		retValue.setFieldValue( 1, "CL", strlen("CL") );
		return;
	}
  	
 	//==��ʼ���
 	in.clear();
 	out.clear();
	in.set( tollcode1 );
	in.itemNum = 1;
	if(table->getData(m_iTableOffset, &in, &out, iIndex)==0)
	{
		strcpy( region1, out.values[1] );
	}
	else
	{
		//������
		char szLackType[RECORD_LENGTH+1];
		memset(szLackType, 0, sizeof(szLackType));
		sprintf(szLackType, "%d", LACK_TOLLCODE );
		DeleteSpace(szLackType);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, tollcode1);
		return;
	}

	in.clear();
 	out.clear();
	in.set( tollcode2 );
	in.itemNum = 1;
	if(table->getData(m_iTableOffset, &in, &out, iIndex)==0)
	{
		strcpy( region2, out.values[1] );
	}
	else
	{
		//������
		char szLackType[RECORD_LENGTH+1];
		memset(szLackType, 0, sizeof(szLackType));
		sprintf(szLackType, "%d", LACK_TOLLCODE );
		DeleteSpace(szLackType);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, tollcode2);
		return;
	}
 	
 	//�жϵ�һ��������ʾ�ǲ��ǹ���
 	if ( checkFormat(region1, "CN*") )
 	{
		if ( checkFormat(region2, "CN*") ) //�жϵڶ���������ʾ�ǲ��ǹ���
		{
			//�����ǲ�������������ʾ���ǹ㶫ʡ��
			if ( checkFormat(region1, "CNG*") && checkFormat(region2, "CNG*") )
			{
				retValue.setFieldValue( 1, "CNP", strlen("CNP") );
				return;
			}
			else if ( strcmp( region1, region2 ) == 0 )
			{
				retValue.setFieldValue( 1, "CNP", strlen("CNP") );
				return;
			}
			else //������Ǿͷ��ع���
			{
				retValue.setFieldValue( 1, "CNO", strlen("CNO") );
				return;
			}
		}
		else	//�жϵڶ������ǹ��ڵģ��Ϳ�ʼ�и۰�̨
		{
			if ( checkFormat(region2, "CJH") )
			{
				retValue.setFieldValue( 1, "CJH", strlen("CJH") );
				return;
			}
			else if ( checkFormat(region2, "CJM") )
			{
				retValue.setFieldValue( 1, "CJM", strlen("CJM") );
				return;
			}
			else if ( checkFormat(region2, "CJT") )
			{
				retValue.setFieldValue( 1, "CJT", strlen("CJT") );
				return;
			}
			else 
			{
				retValue.setFieldValue( 1, "F", strlen("F") );
				return;
			}
		}
 			
 	}
 	else	//������Ǿ����Ƕ���ۻ��߹���
 	{
		if ( checkFormat(region1, "CJH") )
		{
			retValue.setFieldValue( 1, "CJH", strlen("CJH") );
			return;
		}
		else if ( checkFormat(region1, "CJM") )
		{
			retValue.setFieldValue( 1, "CJM", strlen("CJM") );
			return;
		}
		else if ( checkFormat(region1, "CJT") )
		{
			retValue.setFieldValue( 1, "CJT", strlen("CJT") );
			return;
		}
		else 
		{
			retValue.setFieldValue( 1, "F", strlen("F") );
			return;
		}
  	
	}
	return;
}
/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_AnaCallDirect::message(MessageParser&  pMessage)
{
	int message=pMessage.getMessageType();
	switch(message)
	{
		case MESSAGE_NEW_BATCH:
			break;
		case MESSAGE_BREAK_BATCH:
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
void C_AnaCallDirect::printMe()
{
	printf("\t�������:AnaCallDirect,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_AnaCallDirect::getPluginName()
{
	return "AnaCallDirect";
}

std::string C_AnaCallDirect::getPluginVersion(){
	return "3.0.0";
}
