/****************************************************************
 filename: TfAnaCode.cpp
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
	声讯号码分析
 update:

 *****************************************************************/

#include "TfAnaCode.h"

//extern char ErrorMsg[MSG_LEN];
static int g_iCurTimeLen = 14;

C_TfAnaCode::C_TfAnaCode()
{
	m_iIndex = 1;

}/*end of TfAnaCode*/

C_TfAnaCode::~C_TfAnaCode()
{
	table == NULL;
}/*end of ~TfAnaCode*/

std::string C_TfAnaCode::getPluginName()
{
	return "SpAnanCode";
}

std::string C_TfAnaCode::getPluginVersion()
{
	return "3.0.0";
}

void C_TfAnaCode::printMe()
{
	printf("\t插件名称:SpAnanCode,版本号：3.0.0 \n");
}

void C_TfAnaCode::init(char *szSourceGroupID, char *szServiceID, int index)
{
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	if( Gsm2Toll.Init("I_GSM2TOLLCODE") != 0 )
	{
		throw jsexcp::CException(0,"初始化手机号码头失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iTollcodeOffset = table->getTableOffset("I_TOLLCODE")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TOLLCODE失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iGsm2TollcodeOffset = table->getTableOffset("I_GSM2TOLLCODE")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_GSM2TOLLCODE失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iTollcodeRegionFlagOffset = table->getTableOffset("I_TOLLCODE_REGIONFLAG")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TOLLCODE_REGIONFLAG失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iLocalnetOffset = table->getTableOffset("I_LOCALNET")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_LOCALNET失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iHeaderDeleteOffset = table->getTableOffset("I_TELHEADER_DELETE")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TELHEADER_DELETE失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iTollDiscountPptOffset = table->getTableOffset("I_TOLL_DISCOUNT_PPT")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TOLL_DISCOUNT_PPT失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iTelenoServiceOffset = table->getTableOffset("I_TELENO_SERVICE")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TELENO_SERVICE失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iTelenoPropertyOffset = table->getTableOffset("I_TELENO_PROPERTY")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TELENO_PROPERTY失败！",(char *)__FILE__,__LINE__);
	}
	if( (m_iTelenoDefPropertyOffset = table->getTableOffset("I_TELENO_DEF_PROPERTY")) < 0)
	{
		throw jsexcp::CException(0,"初始化表I_TELENO_DEF_PROPERTY失败！",(char *)__FILE__,__LINE__);
	}
}

void C_TfAnaCode::message(MessageParser&  pMessage)
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

/*
 *description:
 *	get all the information of the calling and called teleno
 *input:
 *       1、teleno
 *       2、teleno_flag
 *       3、cdr_begin
 *       4、tollcode of the localnet which source file belongs to
 *       5、localnet_abbr
 *       6、servcat_id
 *       7、source_id
 *       8、source_group
 *       9、rate_type
 *output:
 *	calling_no:
 *       1、localnet_abbr
 *       2、regionflag
 *       3、teleno_after_tollcode
 *       4、business
 *       5、localnet_ana
 *       6、mobiletype
 *       7、citycode
 *	called_no:
 *       1、server_teleno
 *       2、server_name
 *       3、teleno_header
 */
void C_TfAnaCode::execute(PacketParser& pps,ResParser& retValue)
{
	if (pps.getItem_num() != 9)
	{
		sprintf( ErrorMsg,"the params sent to SpAnaCode is lack");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *)ErrorMsg, (char *)__FILE__, __LINE__);
	}
	//取输入参数
	memset( m_InputParam.m_szTeleno, 0, sizeof( m_InputParam.m_szTeleno ) );
	pps.getFieldValue( 1,m_InputParam.m_szTeleno );
	DeleteSpace( m_InputParam.m_szTeleno );

	memset( m_InputParam.m_szTelenoFlag, 0, sizeof( m_InputParam.m_szTelenoFlag ) );
	pps.getFieldValue( 2,m_InputParam.m_szTelenoFlag );
	DeleteSpace( m_InputParam.m_szTelenoFlag );
	if( strlen(m_InputParam.m_szTelenoFlag) != 2 )
	{
		sprintf( ErrorMsg,"the params sent to SpAnaCode is lack");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *)ErrorMsg, (char *)__FILE__, __LINE__);
	}

	memset( m_InputParam.m_szCdrBegin, 0, sizeof( m_InputParam.m_szCdrBegin ) );
	pps.getFieldValue( 3,m_InputParam.m_szCdrBegin );
	DeleteSpace( m_InputParam.m_szCdrBegin );

	memset( m_InputParam.m_szLocalToll, 0, sizeof( m_InputParam.m_szLocalToll ) );
	pps.getFieldValue( 4,m_InputParam.m_szLocalToll );
	DeleteSpace( m_InputParam.m_szLocalToll );

	memset( m_InputParam.m_szLocalnet_Abbr, 0, sizeof( m_InputParam.m_szLocalnet_Abbr ) );
	pps.getFieldValue( 5,m_InputParam.m_szLocalnet_Abbr );
	DeleteSpace( m_InputParam.m_szLocalnet_Abbr );

	memset( m_InputParam.m_szServCatId, 0, sizeof(m_InputParam.m_szServCatId ) );
	pps.getFieldValue( 6,m_InputParam.m_szServCatId );
	DeleteSpace( m_InputParam.m_szServCatId );

	memset( m_InputParam.m_szSourceId, 0, sizeof( m_InputParam.m_szSourceId ) );
	pps.getFieldValue( 7,m_InputParam.m_szSourceId );
	DeleteSpace( m_InputParam.m_szSourceId );

	memset( m_InputParam.m_szGroupId, 0, sizeof( m_InputParam.m_szGroupId ) );
	pps.getFieldValue( 8,m_InputParam.m_szGroupId );
	DeleteSpace( m_InputParam.m_szGroupId );

	memset( m_InputParam.m_szRateType, 0, sizeof( m_InputParam.m_szRateType ) );
	pps.getFieldValue( 9,m_InputParam.m_szRateType );
	DeleteSpace( m_InputParam.m_szRateType );

	strcpy( m_CurTelenoAttr.m_szTeleno, m_InputParam.m_szTeleno );
	memcpy( m_CurTelenoAttr.m_szTelenoFlag, m_InputParam.m_szTelenoFlag, 1 );
	m_CurTelenoAttr.m_szTelenoFlag[1] = '\0';

	char abn_value[RECORD_LENGTH+1];
	char abn_content[RECORD_LENGTH+1];
	memset(abn_value, 0, sizeof(abn_value));
	memset(abn_content, 0, sizeof(abn_content));

	deleteTelHeader();
	int ret;
	//先分析区号
	ret = anaTollcode();
	if( ret != 0 )
	{
		//无资料
		pluginAnaResult result=eLackInfo;
		sprintf(abn_value, "%d", ret);
		sprintf(abn_content, "%s", m_CurTelenoAttr.m_szTeleno);
		retValue.setAnaResult(result, abn_value, abn_content);
		return;
	}

	if( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG)
	{
		if(1==Gsm2Toll.isGsm(m_CurTelenoAttr.m_szTeleno))
		{
			//手机号
			anaCdmaTeleno();
			//cout<<"anaCdmaTeleno="<<ret<<endl;
			if( ret != 0 )
			{
				//无资料
				pluginAnaResult result=eLackInfo;
				sprintf(abn_value, "%d", ret);
				sprintf(abn_content, "%s", m_CurTelenoAttr.m_szTeleno);
				retValue.setAnaResult(result, abn_value, abn_content);
				return;
			}
		}
		else
		{
			ret = anaTollcodeTeleno();
			//cout<<"anaTollcodeTeleno="<<ret<<endl;
			if( ret != 0 )
			{
				//无资料
				pluginAnaResult result=eLackInfo;
				sprintf(abn_value, "%d", ret);
				sprintf(abn_content, "%s", m_CurTelenoAttr.m_szTeleno);
				retValue.setAnaResult(result, abn_value, abn_content);
				return;
			}
		}
		ret = anaRegionFlag();
		//cout<<"anaRegionFlag="<<ret<<endl;
		if( ret != 0 )
		{
			//无资料
				pluginAnaResult result=eLackInfo;
				sprintf(abn_value, "%d", ret);
				sprintf(abn_content, "%s", m_CurTelenoAttr.m_szTeleno);
				retValue.setAnaResult(result, abn_value, abn_content);
				return;
		}
		else
			outputTelenoAttr( m_CurTelenoAttr, retValue );
	}
	else if( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG)
	{
		ret = anaTelenoProperty();
		//cout<<"anaTelenoProperty="<<ret<<endl;
		if( ret != 0 )
		{
			//无资料
			pluginAnaResult result=eLackInfo;
			sprintf(abn_value, "%d", ret);
			sprintf(abn_content, "%s", m_CurTelenoAttr.m_szTeleno);
			retValue.setAnaResult(result, abn_value, abn_content);
			return;
		}
		else
			outputTelenoAttr( m_CurTelenoAttr, retValue );
	}
}/* end of execute */

int C_TfAnaCode::deleteTelHeader()
{
	char szTmp1[RECORD_LENGTH+1];
	memset(szTmp1, 0, sizeof(szTmp1));
	char szTmp2[RECORD_LENGTH+1];
	memset(szTmp2, 0, sizeof(szTmp2));

	m_InData.clear();
	m_OutData.clear();
	//查询条件：ServCat、Source、CallFlag、Teleno
	m_InData.set(m_InputParam.m_szServCatId, 0);
	m_InData.set(m_InputParam.m_szSourceId, 0);
	if( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG )
		m_InData.set("01", 0);
	else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
		m_InData.set("10", 0);
	m_InData.set(m_CurTelenoAttr.m_szTeleno, 0);
	m_InData.itemNum = 1;
	strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
	if( table->getData(m_iHeaderDeleteOffset, &m_InData, &m_OutData, 1) == 0 )
	{
		strcpy(szTmp1, m_OutData.values[9]);
		strcpy(szTmp2, m_CurTelenoAttr.m_szTeleno+atoi(szTmp1));
		strcpy(m_CurTelenoAttr.m_szTeleno, szTmp2);
		return 0;
	}
	else
		return -1;
}

int C_TfAnaCode::anaTollcode()
{
	char szTmpTeleno[RECORD_LENGTH+1];
	memset( szTmpTeleno, 0, sizeof( szTmpTeleno ) );
	strcpy( szTmpTeleno, m_CurTelenoAttr.m_szTeleno );

	//如果该业务是默认的省略区号的，添加默认的区号

	if(szTmpTeleno[0] != '0')
	{
		if( m_InputParam.m_szTelenoFlag[1] == '0' )//if(m_cOmitTollcode == '0')
		{
			strcpy( m_CurTelenoAttr.m_szTollcode, m_InputParam.m_szLocalToll );
			strcpy( m_CurTelenoAttr.m_szLocalNet, m_InputParam.m_szLocalnet_Abbr );
			strcpy( m_CurTelenoAttr.m_szGroupB, m_InputParam.m_szGroupId );
			strcpy( m_CurTelenoAttr.m_szAfterTollTel, m_CurTelenoAttr.m_szTeleno );
			sprintf( m_CurTelenoAttr.m_szTollTel, "%s%s",m_InputParam.m_szLocalToll, m_CurTelenoAttr.m_szAfterTollTel );

			return 0;
		}
	}
	else
	{
		m_InData.clear();
		m_OutData.clear();
		m_InData.set(m_CurTelenoAttr.m_szTeleno);
		m_InData.itemNum = 1;
		strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
		if( table->getData(m_iTollcodeOffset, &m_InData, &m_OutData, 1) == 0 )
		{
			strcpy(m_CurTelenoAttr.m_szTollcode, m_OutData.values[0]);
			strcpy( m_CurTelenoAttr.m_szAfterTollTel, m_CurTelenoAttr.m_szTeleno + strlen( m_CurTelenoAttr.m_szTollcode ) );
			sprintf( m_CurTelenoAttr.m_szTollTel, "%s%s",m_CurTelenoAttr.m_szTollcode, m_CurTelenoAttr.m_szAfterTollTel );
			//从LOCALNET表查本地网缩写
			m_InData.clear();
			m_OutData.clear();
			m_InData.set(m_CurTelenoAttr.m_szTollcode);
			m_InData.itemNum = 1;
			strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
			if( table->getData(m_iLocalnetOffset, &m_InData, &m_OutData, 1) == 0 )
			{
				strcpy(m_CurTelenoAttr.m_szLocalNet, m_OutData.values[1]);
			}
			//查组B标识
			m_InData.clear();
			m_OutData.clear();
			m_InData.set(m_CurTelenoAttr.m_szTollcode, 0);
			m_InData.set(m_InputParam.m_szRateType, 0);
			m_InData.itemNum = 1;
			strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
			if( table->getData(m_iTollDiscountPptOffset, &m_InData, &m_OutData, 1) == 0 )
			{
				strcpy( m_CurTelenoAttr.m_szGroupB, m_OutData.values[2] );
				return 0;
			}
		}
	}
	
	if( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG )
		return LACK_CALLING_TOLLCODE;
	else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
		return LACK_CALLED_TOLLCODE;

}/* end of anaTollcode */

int C_TfAnaCode::anaCdmaTeleno()
{
	/*
	char szTmpTeleno[RECORD_LENGTH];
	memset( szTmpTeleno, 0, sizeof( szTmpTeleno ) );
		strcpy( szTmpTeleno, m_CurTelenoAttr.m_szAfterTollTel );
	szTmpTeleno[strlen(szTmpTeleno)] = '\0';
	*/

	m_InData.clear();
	m_OutData.clear();
	m_InData.set(m_CurTelenoAttr.m_szAfterTollTel);
	m_InData.itemNum = 1;
	strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
	if( table->getData(m_iGsm2TollcodeOffset, &m_InData, &m_OutData, 1) == 0 )
	{
		//m_CurTelenoAttr.m_szTollcode
		strcpy(m_CurTelenoAttr.m_szTollcode, m_OutData.values[3]); 
		strcpy(m_CurTelenoAttr.m_szBusiAfter, m_OutData.values[4]); 	
		//分析mobile_type
    	if(strcmp("M01", m_OutData.values[5])==0)
    		strcpy(m_CurTelenoAttr.m_szMobile, "G" );//GSM手机号码
    	else
    		strcpy(m_CurTelenoAttr.m_szMobile, "M" );//cdma手机号码
		strcpy(m_CurTelenoAttr.m_szLocalNet, m_InputParam.m_szLocalnet_Abbr ); 
		//根据区号分析本地网缩写
		m_InData.clear();
		m_OutData.clear();
		m_InData.set(m_CurTelenoAttr.m_szTollcode);
		m_InData.itemNum = 1;
		strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
		if( table->getData(m_iLocalnetOffset, &m_InData, &m_OutData, 1) == 0 )
		{
			strcpy(m_CurTelenoAttr.m_szLocalNet_Ana, m_OutData.values[1]);
		}
		//查组B标识
		m_InData.clear();
		m_OutData.clear();
		m_InData.set(m_CurTelenoAttr.m_szTollcode, 0);
		m_InData.set(m_InputParam.m_szRateType, 0);
		m_InData.itemNum = 1;
		strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
		if( table->getData(m_iTollDiscountPptOffset, &m_InData, &m_OutData, 1) == 0 )
		{
			strcpy( m_CurTelenoAttr.m_szGroupB, m_OutData.values[2] );
			return 0;
		}
	}
	
	if ( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG)
		return LACK_CALLING_GSM2TOLLCODE;
	else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
		return LACK_CALLED_GSM2TOLLCODE;
}/* end of anaTollcode */

int C_TfAnaCode::anaTollcodeTeleno()
{
	m_InData.clear();
	m_OutData.clear();
	m_InData.set(m_CurTelenoAttr.m_szTollTel);
	m_InData.itemNum = 1;
	strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
	if( table->getData(m_iTelenoPropertyOffset, &m_InData, &m_OutData, 1) == 0 )
	{
		strcpy(m_CurTelenoAttr.m_szMobile, "H" );
		strcpy(m_CurTelenoAttr.m_szBusiAfter, m_OutData.values[6] );
		strcpy(m_CurTelenoAttr.m_szLocalNet_Ana, m_CurTelenoAttr.m_szLocalNet);
		strcpy(m_CurTelenoAttr.m_szLocalNet, m_InputParam.m_szLocalnet_Abbr);
	}
	else
	{
		m_InData.clear();
		m_OutData.clear();
		m_InData.set("S4", 0);
		m_InData.set("01", 0);
		m_InData.set(m_CurTelenoAttr.m_szTollcode, 0);
		m_InData.set(m_CurTelenoAttr.m_szAfterTollTel, 0);
		m_InData.itemNum = 1;
		strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
		if( table->getData(m_iTelenoDefPropertyOffset, &m_InData, &m_OutData, 1) == 0 )
		{
			strcpy(m_CurTelenoAttr.m_szMobile, "H" );
			strcpy(m_CurTelenoAttr.m_szBusiAfter, m_OutData.values[7] );
			strcpy(m_CurTelenoAttr.m_szLocalNet_Ana, m_CurTelenoAttr.m_szLocalNet);
			strcpy(m_CurTelenoAttr.m_szLocalNet, m_InputParam.m_szLocalnet_Abbr);
		}
		else
			return LACK_CALLING_TELENO_PROPERTY;
	}
	return 0;

	/*
	m_InData.clear();
	m_OutData.clear();
	m_InData.set(m_InputParam.m_szServCatId, 0);
	//if( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG )
		m_InData.set("01", 0);
	//else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
	//	m_InData.set("10", 0);
	m_InData.set(m_CurTelenoAttr.m_szTollcode, 0);
	m_InData.set(m_CurTelenoAttr.m_szAfterTollTel, 0);
	m_InData.itemNum = 1;
	strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
	//cout<<"search:"<<m_InputParam.m_szServCatId<<"01"<<m_CurTelenoAttr.m_szTollcode<<m_CurTelenoAttr.m_szAfterTollTel<<"("<<m_InputParam.m_szCdrBegin<<")"<<endl;
	if( table->getData(m_iTelenoServiceOffset, &m_InData, &m_OutData, 1) == 0 )
	{
		strcpy(m_CurTelenoAttr.m_szSvrAfter, m_OutData.values[6] );
		return 0;
	}
	else
		return LACK_TELENO_SERVICE;
	*/
	
	//if ( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG)
	//	return LACK_CALLING_TELENO_PROPERTY;
	//else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
	//	return LACK_CALLED_TELENO_PROPERTY;

}/* end of anaTollcodeTeleno */

int C_TfAnaCode::anaRegionFlag()
{
	//strcpy( m_pSTollcodeRegion->groupa_id, m_InputParam.m_szGroupId );
	//strcpy( m_pSTollcodeRegion->groupb_id, m_CurTelenoAttr.m_szGroupB );
	m_InData.clear();
	m_OutData.clear();
	m_InData.set(m_InputParam.m_szGroupId);
	m_InData.set(m_CurTelenoAttr.m_szGroupB);
	m_InData.itemNum = 2;
	strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
	//cout<<"search in TollcodeRegionFlag("<<m_iTollcodeRegionFlagOffset<<") groupA="<<m_InputParam.m_szGroupId<<" groupB="<<m_CurTelenoAttr.m_szGroupB<<endl;
	if( table->getData(m_iTollcodeRegionFlagOffset, &m_InData, &m_OutData, 1) == 0 )
	{
		strcpy(m_CurTelenoAttr.m_szRegionFlag, m_OutData.values[2]);
		return 0;
	}
	else
	{
		if ( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG)
			return LACK_CALLING_REGION;
		else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
			return LACK_CALLED_REGION;
	}
}/* end of anaRegionFlag */

int C_TfAnaCode::anaTelenoProperty()
{
	m_InData.clear();
	m_OutData.clear();
	//m_InData.set(m_CurTelenoAttr.m_szTollcode, 0);
	m_InData.set("0", 0);
	m_InData.set(m_CurTelenoAttr.m_szAfterTollTel, 0);
	m_InData.itemNum = 1;
	strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
	if( table->getData(m_iTelenoPropertyOffset, &m_InData, &m_OutData, 1) == 0 )
	{
		//strcpy(m_CurTelenoAttr.m_szSvrAfter, m_TelenoProperty.m_pCurAddr->service_id );
		//根据表teleno_property的mobiletype_id来截取SP接入码
		memset(m_CurTelenoAttr.m_szSpCode, 0, sizeof(m_CurTelenoAttr.m_szSpCode));
		strncpy(m_CurTelenoAttr.m_szSpCode, m_CurTelenoAttr.m_szAfterTollTel, atoi(m_OutData.values[7]));
		strcpy(m_CurTelenoAttr.m_szBusiAfter, m_OutData.values[6] );
		strcpy(m_CurTelenoAttr.m_szTelPptHeader, m_OutData.values[1] );
		strcpy(m_CurTelenoAttr.m_szLocalNet_Ana, m_CurTelenoAttr.m_szLocalNet);
		strcpy(m_CurTelenoAttr.m_szLocalNet, m_InputParam.m_szLocalnet_Abbr);
	}
	else
	{
		m_InData.clear();
		m_OutData.clear();
		m_InData.set("S4", 0);
		m_InData.set("10", 0);
		m_InData.set(m_CurTelenoAttr.m_szTollcode, 0);
		m_InData.set(m_CurTelenoAttr.m_szAfterTollTel, 0);
		m_InData.itemNum = 1;
		strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
		if( table->getData(m_iTelenoDefPropertyOffset, &m_InData, &m_OutData, 1) == 0 )
		{
			memset(m_CurTelenoAttr.m_szSpCode, 0, sizeof(m_CurTelenoAttr.m_szSpCode));
			strncpy(m_CurTelenoAttr.m_szSpCode, m_CurTelenoAttr.m_szAfterTollTel, atoi(m_OutData.values[8]));
			strcpy(m_CurTelenoAttr.m_szBusiAfter, m_OutData.values[7] );
			strcpy(m_CurTelenoAttr.m_szTelPptHeader, m_OutData.values[3] );
			strcpy(m_CurTelenoAttr.m_szLocalNet_Ana, m_CurTelenoAttr.m_szLocalNet);
			strcpy(m_CurTelenoAttr.m_szLocalNet, m_InputParam.m_szLocalnet_Abbr);
		}
		else
			return LACK_CALLED_TELENO_PROPERTY;
	}
	m_InData.clear();
	m_OutData.clear();
	m_InData.set(m_InputParam.m_szServCatId, 0);
	//if( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG )
	//	m_InData.set("01", 0);
	//else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
		m_InData.set("10", 0);
	//m_InData.set(m_CurTelenoAttr.m_szTollcode, 0);
	m_InData.set(m_CurTelenoAttr.m_szAfterTollTel, 0);
	m_InData.itemNum = 1;
	strcpy(m_InData.startTime, m_InputParam.m_szCdrBegin);
	if( table->getData(m_iTelenoServiceOffset, &m_InData, &m_OutData, 2) == 0 )
	{
		strcpy(m_CurTelenoAttr.m_szSvrAfter, m_OutData.values[6] );
		return 0;
	}
	else
		return LACK_TELENO_SERVICE;
	
	//if ( m_InputParam.m_szTelenoFlag[0] == CALLING_FLAG)
	//	return LACK_CALLING_TELENO_PROPERTY;
	//else if ( m_InputParam.m_szTelenoFlag[0] == CALLED_FLAG )
	//	return LACK_CALLED_TELENO_PROPERTY;

}

int C_TfAnaCode::outputTelenoAttr(STelenoAttr &TelenoAttibute, ResParser& retValue)
{
	if( TelenoAttibute.m_szTelenoFlag[0] == CALLING_FLAG )
	{
		//cout<<"output"<<TelenoAttibute.m_szLocalNet<<":"<<TelenoAttibute.m_szRegionFlag<<":"<<TelenoAttibute.m_szAfterTollTel<<":"
		//<<TelenoAttibute.m_szBusiAfter<<endl;
		retValue.setFieldValue( 1, TelenoAttibute.m_szLocalNet, strlen(TelenoAttibute.m_szLocalNet) );
		retValue.setFieldValue( 2, TelenoAttibute.m_szRegionFlag, strlen(TelenoAttibute.m_szRegionFlag) );
		retValue.setFieldValue( 3, TelenoAttibute.m_szAfterTollTel, strlen(TelenoAttibute.m_szAfterTollTel) );
		retValue.setFieldValue( 4, TelenoAttibute.m_szBusiAfter, strlen(TelenoAttibute.m_szBusiAfter) );
		retValue.setFieldValue( 5, TelenoAttibute.m_szLocalNet_Ana, strlen(TelenoAttibute.m_szLocalNet_Ana) );
		retValue.setFieldValue( 6, TelenoAttibute.m_szMobile, strlen(TelenoAttibute.m_szMobile) );
		retValue.setFieldValue( 7, TelenoAttibute.m_szTollcode, strlen(TelenoAttibute.m_szTollcode) );
	}
	else if( TelenoAttibute.m_szTelenoFlag[0] == CALLED_FLAG )
	{
		//cout<<"output"<<TelenoAttibute.m_szSpCode<<":"<<TelenoAttibute.m_szSvrAfter<<":"<<TelenoAttibute.m_szTelPptHeader<<endl;
		retValue.setFieldValue( 1, TelenoAttibute.m_szSpCode, strlen(TelenoAttibute.m_szSpCode) );
		retValue.setFieldValue( 2, TelenoAttibute.m_szSvrAfter, strlen(TelenoAttibute.m_szSvrAfter) );
		retValue.setFieldValue( 3, TelenoAttibute.m_szTelPptHeader, strlen(TelenoAttibute.m_szTelPptHeader) );
	}
	return 0;
}


