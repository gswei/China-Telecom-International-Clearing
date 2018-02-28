#include "200SPAnaCalledNo.h"

/*
extern char ErrorMsg[MSG_LEN];
static int g_iCurTimeLen = 14;
*/

/*************************************************
* 描述    ：构造函数
* 入口参数：无
* 出口参数：无SERV_CAT_ID||VALID_FLAG||TELENO_HEADER
* 返回    ：无
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
* 描述    ：析构函数
* 入口参数：无
* 出口参数：无
* 返回    ：无
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
		throw jsexcp::CException(0,"初始化表I_TOLLCODE失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iLocalnetOffset = table->getTableOffset("I_LOCALNET")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_LOCALNET失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iTelenoServiceOffset = table->getTableOffset("I_TELENO_SERVICE")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TELENO_SERVICE失败！",(char *)__FILE__,__LINE__);
	}
}

void C_200SPAnaCalledNo::message(MessageParser&  pMessage)
{
}

void C_200SPAnaCalledNo::printMe()
{
	printf("\t插件名称:AnaCalledNo,版本号：3.0.0 \n");
}

std::string C_200SPAnaCalledNo::getPluginName()
{
	return "AnaCalledNo";
}

std::string C_200SPAnaCalledNo::getPluginVersion(){
	return "3.0.0";
}

/*************************************************
* 描述    ：执行函数,规整被叫号码，输出批价需要信息，
* 入口参数：$CalledNo, $Cdrbegin, $Call_location, $Card_location, $ServCatId
* 出口参数：$CalledNo, $Fee_Type, $CalledTollcode, $Localnet, $service_id
* 返回    ：0 或者 无资料代码
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
		//无资料
		memset(abn_value, 0, sizeof(abn_value));
		memset(abn_content, 0, sizeof(abn_content));
		pluginAnaResult result=eLackInfo;
		sprintf(abn_value, "%d", lackinfo);
		sprintf(abn_content, "%s", lackreason);
		retValue.setAnaResult(result, abn_value, abn_content);
	}

}


/*************************************************
* 描述    ：分析区号,省外的区号打标志分拣，省内区号截去
* 入口参数：$CalledNo
* 出口参数：$CalledNo
* 返回    ：0, 无资料代码
**************************************************/
bool C_200SPAnaCalledNo::anaTollcode( PacketParser& pps, ResParser& retValue )
{
	//通过被叫号码查出来区号，
	//1.本省区号，截去 return true
	//2.外省区号，打标志 lackinfo = 0 return false
	//3.无区号，不作处理 return true
	//如果该业务是默认的省略区号的，添加默认的区号
	char szTmpTeleno[RECORD_LENGTH+1];
	memset( szTmpTeleno, 0, sizeof( szTmpTeleno ) );
	strcpy( szTmpTeleno, CalledNo);

	if(szTmpTeleno[0] != '0')
	{
		//不以0开头的，作为没有区号，直接返回
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
			//是否本省
			m_InData.clear();
			m_OutData.clear();
			m_InData.set(Tollcode);
			m_InData.itemNum = 1;
			if( table->getData(m_iLocalnetOffset, &m_InData, &m_OutData, 1) != 0 )
			{
				//外省
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
* 描述    ：分析使用类型，通过分析号码分析出计费类型
* 入口参数：$CalledNo
* 出口参数：$CalledNo_Ana, $Fee_Type
* 返回    ：true, false
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
* 描述    ：分析本地网标示，通过发话地，发卡地分析出
* 入口参数：$Tollcode
* 出口参数：$Localnet
* 返回    ：true, false
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
		//外省
		m_InData.clear();
		m_OutData.clear();
		m_InData.set(Tollcode);
		m_InData.itemNum = 1;
		if( table->getData(m_iTollcodeOffset, &m_InData, &m_OutData, 1) != 0 )
		{
			lackinfo = LACK_CALLED_TOLLCODE;	//不是区号
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
* 描述    ：分析SP_ABBR，通过被叫号码头分析
* 入口参数：$CalledNo
* 出口参数：$service_id
* 返回    ：true, false
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

//一个用来比较字符串是否符合一定格式
/**************************************************
*	Function Name:	checkFormat
*	Description:	比较两个字符串是否匹配（相等）
*	Input Param:
*		cmpString -------> 被比较的字符串
*		format	   -------> 匹配的字符串，支持*,?,[]等通配符
*	Returns:
*		如果两个字符串匹配，返回SUC
*		如果两个字符串不匹配，返回FAIL
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
