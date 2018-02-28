#include "CtjfAnaRoute.h"

/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
C_CtjfAnaRoute::C_CtjfAnaRoute()
{
	memset(&m_InData, 0, sizeof(m_InData));
	memset(&m_OutData, 0, sizeof(m_OutData));
	table = NULL;
}


/*************************************************
* 描述    ：析构函数
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
C_CtjfAnaRoute::~C_CtjfAnaRoute()
{
}


/*************************************************
* 描述    ：初始化函数，初始化需要读取表用的参数
* 入口参数：无
* 出口参数：无
* 返回    ：0
**************************************************/         
void C_CtjfAnaRoute::init(char *szSourceGroupID, char *szServiceID, int index) 
{
	theJSLog<<"==========CtljAnaRoute class begin=========="<<endd;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_ROUTE");
	m_iTableOffset = table->getTableOffset(m_szTableName);
	m_iIndex = 1;
}

/*************************************************
* 描述    ：执行函数，查询路由的相关信息
* 入口参数：$SOURCE_ID,$Route_Name,$Direct_Flag,$CdrBegin,$Lackinfo_value,$Lackinfo_code
* 出口参数：$Route_Type1,$Route_Type2,$Buiness_id,$Audit_id
* 返回    ：0 或者 无资料代码
**************************************************/  
void C_CtjfAnaRoute::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	if( pps.getItem_num() != 4 )
	{
		sprintf( ErrorMsg,"插件 CtjfAnaRoute 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
  	
	char szSourceId[RECORD_LENGTH+1];
	memset( szSourceId, 0, sizeof( szSourceId ) );   
  	pps.getFieldValue( 1,szSourceId );
  	DeleteSpace( szSourceId );  
  	  
  	char szRouteName[RECORD_LENGTH+1];
  	memset( szRouteName, 0, sizeof( szRouteName ) );   
  	pps.getFieldValue( 2, szRouteName );
  	DeleteSpace( szRouteName ); 
  	 
  	char szDirectFlag[RECORD_LENGTH+1]; 	
	memset( szDirectFlag, 0, sizeof( szDirectFlag ) );   
  	pps.getFieldValue( 3, szDirectFlag );
  	DeleteSpace( szDirectFlag );
  	  
  	char szStartTime[RECORD_LENGTH+1]; 	
  	memset( szStartTime, 0, sizeof( szStartTime ) );   
  	pps.getFieldValue( 4, szStartTime );
  	DeleteSpace( szStartTime );
  	
	char szBusiness[RECORD_LENGTH+1];
	char szAuditId[RECORD_LENGTH+1];
	char szRouteType1[RECORD_LENGTH+1];
	char szRouteType2[RECORD_LENGTH+1];
	
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
	
 	m_InData.clear();
 	m_InData.set(szSourceId);
 	m_InData.set(szRouteName);
 	m_InData.set(szDirectFlag);
	m_InData.itemNum = 3;
 	strcpy(m_InData.startTime, szStartTime);
 	if( table->getData(m_iTableOffset, &m_InData, &m_OutData, m_iIndex) == 0 )
 	{
		strcpy(szRouteType1,m_OutData.values[3]); 
		strcpy(szRouteType2, m_OutData.values[4]);
  	strcpy(szBusiness,m_OutData.values[5]);
		strcpy(szAuditId,m_OutData.values[6]);
	
		retValue.setFieldValue( 1, szRouteType1, strlen(szRouteType1) );   
		retValue.setFieldValue( 2, szRouteType2, strlen(szRouteType2) );
		retValue.setFieldValue( 3, szBusiness, strlen(szBusiness) );  	
		retValue.setFieldValue( 4, szAuditId, strlen(szAuditId) );  
  	}
  	else
	{
		if( szDirectFlag[0] == 'I' )
		{
  			memset( abn_value, 0, sizeof(abn_value) );
  			sprintf( abn_value, "%d", LACK_INROUTE_ROUTE );
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, abn_value, szRouteName);
		}
		else 
		{
  			memset( abn_value, 0, sizeof(abn_value) );
  			sprintf( abn_value, "%d", LACK_OUTROUTE_ROUTE );
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, abn_value, szRouteName);
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
void C_CtjfAnaRoute::message(MessageParser&  pMessage)
{
}

/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_CtjfAnaRoute::printMe()
{
	printf("\t插件名称:CtjfAnaRoute,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_CtjfAnaRoute::getPluginName()
{
	return "CtjfAnaRoute";
}

std::string C_CtjfAnaRoute::getPluginVersion(){
	return "3.0.0";
}
