#include "IsBalanceValid.h"

/*************************************************
*	函数名称:	Execute
*	功能描述:	分析余额有效日期
*	参数列表:	$input_time,$valid_time
*	返回结果:	0,1,2
*************************************************/
void C_IsBalanceValid::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];

	if (pps.getItem_num() != 2)
  	{
  		sprintf( ErrorMsg,"插件 IsBalanceValid 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
	
	memset( input_time, 0, sizeof(input_time) );
	memset( valid_time, 0, sizeof(valid_time) );
	
	pps.getFieldValue( 1, input_time );
	pps.getFieldValue( 2, valid_time );	
	
	buff = input_time;
	year_month_i = atoi( buff.substr(0,6).c_str() );
	day_i        = atoi( buff.substr(6,2).c_str() );
	
	buff = valid_time;
	year_month_v = atoi( buff.substr(0,6).c_str() );
	day_v        = atoi( buff.substr(6,2).c_str() );
	
	//======================================================
	//判断是否过期
	if ( year_month_v > year_month_i )
	{
		retValue.setFieldValue( 1, "0", strlen("0") );
	}
	
	else if ( year_month_v == year_month_i )
	{
		if ( day_v >= day_i )
		{
			retValue.setFieldValue( 1, "0", strlen("0") );
		}
		else
		{
			retValue.setFieldValue( 1, "1", strlen("1") );
		}
	}
	
	else if ( year_month_v < year_month_i )
	{
		if ( (year_month_v+3) > year_month_i )
		{
			retValue.setFieldValue( 1, "1", strlen("1") );
		}
		
		else if ( (year_month_v+3) == year_month_i )
		{
			if ( day_v >= day_i )
			{
				retValue.setFieldValue( 1, "1", strlen("1") );
			}
			else
			{
				retValue.setFieldValue( 1, "2", strlen("2") );
			}
		}
		else if ( (year_month_v+3) < year_month_i )             
		{
			retValue.setFieldValue( 1, "2", strlen("2") );
		}
	}
	
	//======================================================
	//判断是否本月过期
	if ( year_month_i == year_month_v )
	{
		retValue.setFieldValue( 2, "1", strlen("1") );
	}
	else
	{
		retValue.setFieldValue( 2, "0", strlen("0") );
	}
	return;	
}


/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：无
* 出口参数：无
* 返回    ：0
**************************************************/
void C_IsBalanceValid::init(char *szSourceGroupID, char *szServiceID, int index)
{
}


/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_IsBalanceValid::message(MessageParser&  pMessage)
{
}


/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_IsBalanceValid::printMe()
{
	printf("\t插件名称:IsBalanceValid,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_IsBalanceValid::getPluginName()
{
	return "IsBalanceValid";
}

std::string C_IsBalanceValid::getPluginVersion(){
	return "3.0.0";
}


