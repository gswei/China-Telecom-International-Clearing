#include "SWLHAnaCalledDirect.h"

/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
C_SWLHAnaCalledDirect::C_SWLHAnaCalledDirect()
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
C_SWLHAnaCalledDirect::~C_SWLHAnaCalledDirect()
{
}


/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
void C_SWLHAnaCalledDirect::init(char *szSourceGroupID, char *szServiceID, int index)
{
	//theLog<<"==========SWLHAnaCalledDirect class begin=========="<<endd;
	m_iIndex = 1;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_SWLH_REGION_DIRECT");
	m_iTableOffset = table->getTableOffset(m_szTableName);
}



/*************************************************
*	函数名称:	Execute
*	功能描述:	通话方向分析
*	参数列表:	区号
*	返回结果:	查表获取的通话方向
*************************************************/
void C_SWLHAnaCalledDirect::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];

	if ( pps.getItem_num() != 2 )
	{
		sprintf( ErrorMsg,"插件 SWLHAnaCalledDirect 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
	
	memset( tollcode, 0, sizeof(tollcode) );
	pps.getFieldValue( 2, tollcode );
	
	int ResultOfAnalyse;
  	in.clear();
  	in.set( tollcode );
	in.itemNum = 1;
	if(table->getData(m_iTableOffset, &in, &out, m_iIndex)!=0)
  	{
		//没找到
  		in.clear();
  		in.set( "=" );
		in.itemNum = 1;
		if(table->getData(m_iTableOffset, &in, &out, m_iIndex)!=0)
  		{
  			retValue.setFieldValue( 1, "0", strlen("0") );	
  		}
  		else
  		{
  			retValue.setFieldValue( 1, out.values[1], strlen(out.values[1]) );	
  		}
  	}
  	else
  	{
  		retValue.setFieldValue( 1, out.values[1], strlen(out.values[1]) );	
  	}
  	
	return;	
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_SWLHAnaCalledDirect::message(MessageParser&  pMessage)
{
}

/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_SWLHAnaCalledDirect::printMe()
{
	printf("\t插件名称:SWLHAnaCalledDirect,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_SWLHAnaCalledDirect::getPluginName()
{
	return "SWLHAnaCalledDirect";
}

std::string C_SWLHAnaCalledDirect::getPluginVersion(){
	return "3.0.0";
}
