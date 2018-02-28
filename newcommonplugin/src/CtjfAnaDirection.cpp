#include "CtjfAnaDirection.h"


/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
C_CtjfAnaDirection::C_CtjfAnaDirection()
{
	m_InData.clear();
	m_OutData.clear();
	table = NULL;
}

/*************************************************
* 描述    ：析构函数，删除两个结构体
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
C_CtjfAnaDirection::~C_CtjfAnaDirection()
{
}

/*************************************************
* 描述    ：初始化函数，初始化读取表的两个变量
* 入口参数：无
* 出口参数：无
* 返回    ：0
**************************************************/
void C_CtjfAnaDirection::init(char *szSourceGroupID, char *szServiceID, int index)
{
	theJSLog<<"==========CtjfAnaDirection class begin=========="<<endd;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(table_RouteType, "I_ROUTE_TYPE");
	strcpy(table_RouteUseWay, "I_ROUTETYPE_USEWAY");
	m_iTableRouteUseWayOffset = table->getTableOffset(table_RouteType);
	m_iTableRouteTypeOffset = table->getTableOffset(table_RouteUseWay);
	m_iIndex = 1;
}


/*************************************************
* 描述    ：执行函数，通过查询两个类型的使用方式，判断出来去转标志
* 入口参数：$OutRouteType,$InRouteType,$Type
* 出口参数：$DirectFlag
* 返回    ：0 或者 无资料代码
**************************************************/
void C_CtjfAnaDirection::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	if (pps.getItem_num() != 3)
	{
		sprintf( ErrorMsg,"插件 IsFreeNbr 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
  	
  	memset( m_szOutRouteType,0,sizeof( m_szOutRouteType ) );   
  	pps.getFieldValue( 1,m_szOutRouteType );
  	DeleteSpace( m_szOutRouteType );
  	
  	memset( m_szInRouteType, 0, sizeof( m_szInRouteType ) );   
  	pps.getFieldValue( 2, m_szInRouteType );
  	DeleteSpace( m_szInRouteType );
  	
  	memset( m_szType, 0, sizeof( m_szType ) );   
  	pps.getFieldValue( 3, m_szType );
  	DeleteSpace( m_szType );
  	//取source的3，4，位
  	m_szType[0] = m_szType[2];
  	m_szType[1] = m_szType[3];
  	m_szType[2] = 0;
  	
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
  	
  	//========================================================================== 
  	//获取出路有类型
	m_InData.clear();
	m_OutData.clear();
  	m_InData.set(m_szOutRouteType);
	m_InData.itemNum = 1;
	if( table->getData(m_iTableRouteTypeOffset, &m_InData, &m_OutData, m_iIndex) != 0 )
  	{
		memset( abn_value, 0, sizeof(abn_value) );
		sprintf( abn_value, "%d", LACK_OUTROUTE_ROUTE_TYPE );
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, abn_value, m_szOutRouteType);
		return;
  	}
  	else 
  	{
		strcpy(m_szOutUseWay,m_OutData.values[1]);
  	}
    
    //获取入路有类型
  	m_InData.clear();
	m_OutData.clear();
  	m_InData.set(m_szInRouteType);
	m_InData.itemNum = 1;
	if( table->getData(m_iTableRouteTypeOffset, &m_InData, &m_OutData, m_iIndex) != 0 )
  	{
		memset( abn_value, 0, sizeof(abn_value) );
		sprintf( abn_value, "%d", LACK_INROUTE_ROUTE_TYPE );
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, abn_value, m_szInRouteType);
		return;
  	}
  	else 
  	{
		strcpy(m_szInUseWay,m_OutData.values[1]);
  	}
  
  	//根据出入路由类型的使用方式来判断 来/去/转标志
  	m_InData.clear();
	m_OutData.clear();
  	m_InData.set(m_szOutUseWay);
  	m_InData.set(m_szInUseWay);
  	m_InData.set(m_szType );
	m_InData.itemNum = 3;
	if( table->getData(m_iTableRouteUseWayOffset, &m_InData, &m_OutData, m_iIndex) == 0 )
  	{
		strcpy(m_szDirectFlag,m_OutData.values[3]);
		retValue.setFieldValue( 1, m_szDirectFlag, strlen(m_szDirectFlag) );	
		return;
  	}
  	else
  	{
  		m_InData.clear();
		m_OutData.clear();
  		m_InData.set(m_szOutUseWay);
  		m_InData.set(m_szInUseWay);
  		m_InData.set( "==" );
		m_InData.itemNum = 3;
		if( table->getData(m_iTableRouteUseWayOffset, &m_InData, &m_OutData, m_iIndex) == 0 )
  		{
			strcpy(m_szDirectFlag,m_OutData.values[3]);
			retValue.setFieldValue( 1, m_szDirectFlag, strlen(m_szDirectFlag) );
			return;
  		}
  		else
  		{
  			//在这里输出无资料的具体原因
  			char szLackinfoReaSon[RECORD_LENGTH+1];
  			sprintf( szLackinfoReaSon, "%s+%s", m_szOutUseWay, m_szInUseWay);
  			memset( abn_value, 0, sizeof(abn_value) );
  			sprintf( abn_value, "%d", LACK_ROUTE_ROUTE_USEWAY );
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, abn_value, szLackinfoReaSon);
  			return;
		}
	}
	return;
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_CtjfAnaDirection::message(MessageParser&  pMessage)
{
}

/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_CtjfAnaDirection::printMe()
{
	printf("\t插件名称:CTJFAnaDirection,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_CtjfAnaDirection::getPluginName()
{
	return "CTJFAnaDirection";
}

std::string C_CtjfAnaDirection::getPluginVersion(){
	return "3.0.0";
}
