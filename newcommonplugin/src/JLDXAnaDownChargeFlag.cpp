#include "JLDXAnaDownChargeFlag.h"

/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
C_JLDXAnaDownChargeFlag::C_JLDXAnaDownChargeFlag()
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
C_JLDXAnaDownChargeFlag::~C_JLDXAnaDownChargeFlag()
{
}


/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
void C_JLDXAnaDownChargeFlag::init(char *szSourceGroupID, char *szServiceID, int index)
{
	//theLog<<"==========AnaDownChargeFlag class begin=========="<<endd;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_CPID_LOCALNET2DOWNFLAG");
	m_iTableOffset = table->getTableOffset(m_szTableName);
	m_iIndex = 1;
}


/*************************************************
* 描述    ：分析话单的下行计费标志
* 入口参数：本地网缩写、内容提供商代码
* 出口参数：下行计费标志
* 返回    ：0 : 成功
					其它：无资料类型
**************************************************/
void C_JLDXAnaDownChargeFlag::execute(PacketParser& pps,ResParser& retValue)
{
	if (pps.getItem_num() != 6)
	{
		sprintf( ErrorMsg,"插件 AnaDownChargeFlag 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
  	
	char Localnet[RECORD_LENGTH+1];
  	char CpId[RECORD_LENGTH+1];
  	
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
	char abn_code[RECORD_LENGTH+1];
  	memset( abn_code, 0, sizeof(abn_code) );
  	
  	memset( Localnet, 0, sizeof(Localnet) );   
  	pps.getFieldValue( 1, Localnet );
  	DeleteSpace( Localnet );  
		
	memset( CpId, 0, sizeof(CpId) );   
  	pps.getFieldValue( 2, CpId );
  	DeleteSpace( CpId );
  	
	in.clear();
	out.clear();
  	in.set( Localnet );
  	in.set( CpId );
	in.itemNum = 2;

	if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  	{
		in.clear();
		out.clear();
  		in.set( "*" );
  		in.set( CpId );

  		if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  		{
  			in.clear();
  			out.clear();
  			in.set( Localnet );
  			in.set( "*" );
  			
			if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  			{
				in.clear();
  				out.clear();
  				in.set( "*" );
  				in.set( "*" );
  				
				if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  				{
					sprintf( abn_value, "%s, %s", Localnet, CpId );
					sprintf( abn_code, "%d", LACK_DOWN_RATE );
					pluginAnaResult result=eLackInfo;
					retValue.setAnaResult(result, abn_code, abn_value);
  					return;
  				}
  			}
  		}
  	}
	
	retValue.setFieldValue( 1, out.values[2], strlen( out.values[2] ) );	
	return;	
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaDownChargeFlag::message(MessageParser&  pMessage)
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
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaDownChargeFlag::printMe()
{
	printf("\t插件名称:JLDXAnaDownChargeFlag,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_JLDXAnaDownChargeFlag::getPluginName()
{
	return "JLDXAnaDownChargeFlag";
}

std::string C_JLDXAnaDownChargeFlag::getPluginVersion(){
	return "3.0.0";
}
