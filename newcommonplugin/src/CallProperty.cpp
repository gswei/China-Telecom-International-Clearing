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
	//插件执行入口
	if ( pps.getItem_num() != 6 )
	{
		sprintf( ErrorMsg,"插件AnaNbrProperty的输入参数不是6个");
		throw jsexcp::CException(0,(char *)ErrorMsg,(char *)__FILE__,__LINE__);		
	}
			
	if( initFlag != 1 )
		Init( pps );

	//分析处理过程

	//得到输入参数
	getInputParams(pps);

//	theLog<<"获取输入参数成功！"<<endd;

	//传递到CallNumber 类
	setParam();
//	theLog<<"传递到CallNumber类"<<endd;

	//是否分析接入码
	m_callnumber.DealBefTelServ();
//	theLog<<"是否分析接入码"<<endd;
	
	//主叫校验	,输入形式为 区号+号码
//	theLog<<"主叫校验	,输入形式为区号+号码："<<m_szCallNumber<<endd;
	m_callnumber.Get_Teleno(m_szCallNumber);
	m_callnumber.Set_CallNbr(m_szCallNumber);
	m_iAbnReason=m_callnumber.queryTollcode(m_szAbnField);

	if (m_iAbnReason)
	{
		//无资料
//		theLog<<"无资料"<<endd;
		char szLackType[RECORD_LENGTH+1];
		memset(szLackType, 0, sizeof(szLackType));
		sprintf(szLackType, "%d", m_iAbnReason );
		DeleteSpace(szLackType);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, m_szAbnField);
		return;
	}
//	theLog<<"有资料"<<endd;
	m_callnumber.DealTollcode();
//	theLog<<"删除区号"<<endd;

	//是否分析区号
	if (m_szDealType[1]=='1')
	{
		//暂时不做任何处理
	}
	
	//将AfterTollTel放在Teleno中
	m_callnumber.Get_AfterTollTel(m_szCallNumber);
//	theLog<<"去区号"<<m_szCallNumber<<endd;

	m_callnumber.Set_CallNbr(m_szCallNumber);

	//从此之后的工作就是针对AfterTollTel 进行

	//接入码后为空
	//在规整模块会将接入码后可以为空的号码变成接入码+区号的形式，区号后号码为空
	if(strlen(m_szCallNumber)==0)
	{
		//theLog<<"m_szCallNumber空"<<endd;
		getParam();
		sendOutputParams(retValue);
		return;
	}
	
	//国际区号直接填默认属性
	m_callnumber.Get_Tollcode(m_szCallNumber);
//	theLog<<"国际区号直接填默认属性"<<endd;

	if (strncmp(m_szCallNumber,"00",2)==0 )
	{
	}
	else
	{
		if (m_callnumber.IsGsm())
		{
//			theLog<<"手机号"<<endd;
			//手机号
			//是否分析运营商和移动类型
			if (m_szDealType[2]=='1')
			{
//				theLog<<"分析运营商和移动类型"<<endd;
				//属性分析分析手机号区号，取输入的号码前区号，查询制做无资料处理，不再更新区号属性
				m_iAbnReason=m_callnumber.queryGsm(m_szAbnField);
				if (m_iAbnReason)
				{
//					theLog<<"无资料"<<endd;
					//无资料
					char szLackType[RECORD_LENGTH+1];
					memset(szLackType, 0, sizeof(szLackType));
					sprintf(szLackType, "%d", m_iAbnReason );
					DeleteSpace(szLackType);
					pluginAnaResult result=eLackInfo;
					retValue.setAnaResult(result, szLackType, m_szAbnField);
					return;
				}
//				theLog<<"有资料"<<endd;
				m_callnumber.Set_Tollcode(m_szCallNumber);
			}
		}
		else
		{
//			theLog<<"固话"<<endd;
			//固话
			//是否分析运营商和移动类型	,是否分析局号
			if (m_szDealType[2]=='1' ||m_szDealType[3]=='1' )
			{
//				theLog<<"分析运营商和移动类型	,分析局号"<<endd;
				m_iAbnReason=m_callnumber.queryTeleProperty(m_szAbnField);
				if (m_iAbnReason)
				{
//					theLog<<"无资料"<<endd;
					//无资料
					//接着分析TELENO_DEF_PROPERTY,如果无资料则进行挂起
					m_iAbnReason=m_callnumber.queryTelDefProperty(m_szAbnField);
//					theLog<<"分析TELENO_DEF_PROPERTY"<<endd;
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

	//是否分析区号后业务类型
	if (m_szDealType[4]=='1')
	{
//		theLog<<"分析区号后业务类型"<<endd;
		m_callnumber.queryTeleServ(m_szAbnField);
//		theLog<<"结果"<<m_szAbnField<<endd;
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
	
	//初始化号码类
	//m_callnumber.Init(m_szServiceId, m_LastSourceId, table);
	m_callnumber.Reset(m_szServiceId, m_LastSourceId);
	
	/* set init_flag of this plugin */
	initFlag = 1;

	return 0;
}

int C_CallProperty::getInputParams(PacketParser & pps)
{
	//获取插件其他可变参数

	//获取插件对应的业务代码，用去查询实际的物理表名
	memset(m_szServCatId,0,sizeof(m_szServCatId));
	pps.getFieldValue(1,m_szServCatId);
	DeleteSpace( m_szServCatId ); 
	//初始化号码类
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

//接入码、运营商、行政区编码、移动类型、区号后业务类型。
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
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：无
* 出口参数：无
* 返回    ：0
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
	//初始化号码类
	m_callnumber.Init(table);
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
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
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_CallProperty::printMe()
{
	printf("\t插件名称:AnaNbrProperty,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_CallProperty::getPluginName()
{
	return "AnaNbrProperty";
}

std::string C_CallProperty::getPluginVersion(){
	return "3.0.0";
}
