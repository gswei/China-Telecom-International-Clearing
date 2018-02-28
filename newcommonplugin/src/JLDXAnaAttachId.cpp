#include "JLDXAnaAttachId.h"

/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
C_JLDXAnaAttachId::C_JLDXAnaAttachId()
{
	in.clear();
	out.clear();
	table = NULL;
	iIndex = 1;
}


/*************************************************
* 描述    ：析构函数
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
C_JLDXAnaAttachId::~C_JLDXAnaAttachId()
{
}


/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
void C_JLDXAnaAttachId::init(char *szSourceGroupID, char *szServiceID, int index)
{
	//theLog<<"==========AnaCpAttachId class begin=========="<<endd;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_JLDX_CPATTACH");
	m_iTableOffset = table->getTableOffset(m_szTableName);
	iIndex = 1;
}


/*************************************************
* 描述    ：获取attachid
* 入口参数：内容提供商代码、服务
* 出口参数：attachid
* 返回    ：
**************************************************/
void C_JLDXAnaAttachId::execute(PacketParser& pps,ResParser& retValue)
{
	
	if (pps.getItem_num() != 2)
	{
		sprintf( ErrorMsg,"插件 JLDXAnaAttachId 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}

  	char CpId[RECORD_LENGTH+1];
  	char ServiceId[RECORD_LENGTH+1];
  	
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
  	
  	memset( CpId, 0, sizeof(CpId) );   
  	pps.getFieldValue( 1, CpId );
  	DeleteSpace( CpId );  
		
  	memset( ServiceId, 0, sizeof(ServiceId) );   
  	pps.getFieldValue( 2, ServiceId );
  	DeleteSpace( ServiceId );
  	
	in.clear();
	out.clear();
  	in.set( CpId );
  	in.set( ServiceId );
	in.itemNum = 2;

  	if( table->getData(m_iTableOffset, &in, &out, iIndex) != 0 )
  	{
  		in.clear();
  		out.clear();
  		in.set( CpId );
  		in.set( "*" );
  		in.itemNum = 2;

		if( table->getData(m_iTableOffset, &in, &out, iIndex) != 0 )
		{
			sprintf( abn_value, "%s, %s", CpId, ServiceId );
			char szLackType[RECORD_LENGTH+1];
			sprintf( szLackType, "%d", LACK_CPATTACH );
			DeleteSpace(szLackType);
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, szLackType, abn_value);
			return;
		}
	}
	
	retValue.setFieldValue( 1, out.values[2], strlen(out.values[2]) );	
	return;	
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaAttachId::message(MessageParser&  pMessage)
{
}

/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaAttachId::printMe()
{
	printf("\t插件名称:JLDXAnaAttachId,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_JLDXAnaAttachId::getPluginName()
{
	return "JLDXAnaAttachId";
}

std::string C_JLDXAnaAttachId::getPluginVersion(){
	return "3.0.0";
}
