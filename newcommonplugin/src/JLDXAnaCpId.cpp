#include "JLDXAnaCpId.h"

/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
C_JLDXAnaCpId::C_JLDXAnaCpId()
{
	in.clear();
	out.clear();
	table = NULL;
}


/*************************************************
* 描述    ：析构函数
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
C_JLDXAnaCpId::~C_JLDXAnaCpId()
{
		
}


/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
void C_JLDXAnaCpId::init(char *szSourceGroupID, char *szServiceID, int index)
{ 
	//theLog<<"==========AnaCpId class begin=========="<<endd;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_CPID");
	m_iTableOffset = table->getTableOffset(m_szTableName);
	m_iIndex = 1;
}


/*************************************************
* 描述    分析内容提供商代码
* 入口参数：内容提供商代码、通话开始时间
* 出口参数：cpid，lackinfo_value
* 返回    ：0 : 成功
					其它：无资料类型
**************************************************/
void C_JLDXAnaCpId::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	if (pps.getItem_num() != 2)
	{
		sprintf( ErrorMsg,"插件 JLDXAnaCpId 的输入参数不正确！" );
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
	
	char CpId[RECORD_LENGTH+1];
	char CdrBegin[RECORD_LENGTH+1];
	
	char abn_value[RECORD_LENGTH+1];
	memset( abn_value, 0, sizeof(abn_value) );
  
	memset( CpId, 0, sizeof(CpId) );   
	pps.getFieldValue( 1, CpId );
	DeleteSpace( CpId );  
	
	memset( CdrBegin, 0, sizeof(CdrBegin) ); 
	pps.getFieldValue( 2, CdrBegin );
	DeleteSpace( CdrBegin );

	in.clear();
	in.set( CpId );
	in.itemNum = 1;
	strcpy( in.startTime, CdrBegin );
	
	if(table->getData(m_iTableOffset, &in, &out, m_iIndex)==0)
	{
		retValue.setFieldValue( 1, out.values[1], strlen(out.values[1]) );
	} 
	else
	{
		sprintf(abn_value, "%d", LACK_CPID);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, abn_value, CpId);
	}
	return;
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaCpId::message(MessageParser&  pMessage)
{
}

/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaCpId::printMe()
{
	printf("\t插件名称:JLDXAnaCpId,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_JLDXAnaCpId::getPluginName()
{
	return "JLDXAnaCpId";
}

std::string C_JLDXAnaCpId::getPluginVersion(){
	return "3.0.0";
}
