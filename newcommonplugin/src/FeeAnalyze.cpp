#include "FeeAnalyze.h"

/*************************************************
*	函数名称:	Execute
*	功能描述:	分析账本变动信息字段
*	参数列表:	账本变动信息字段
*	返回结果:	本金实收扣费,赠金实收扣费
*************************************************/
void C_FeeAnalyze::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];

	if (pps.getItem_num() != 3)
  	{
  		sprintf( ErrorMsg,"插件 IsBalanceValid 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
	
	memset( buff, 0, sizeof(buff) );
	
	pps.getFieldValue( 1, buff );
	
	//初始化
	input_group.clear();
	fee_org = 0;
	fee_add = 0;
	
	char2vector( buff, input_group, "#" );
	
	pps.getFieldValue( 2, buff );
	mode = atoi( buff );
	
	pps.getFieldValue( 3, lackinfo );
	
	for ( int i = 0 ; i < input_group.size(); i++ )
	{
		vector<string> t;
		char2vector( input_group[i].substr(input_group[i].find_first_of(":")+1), t, "," );
		if ( t.size() != 3 )
		{
			if ( i == input_group.size() -1 )
			{
				continue;
			}
			else
			{
				pluginAnaResult result=eLackInfo;
				retValue.setAnaResult(result, lackinfo, "");
				return;
			}
		}
		
		int BalanceType01 = atoi( t[0].c_str() );
		int Delta01       = atoi( t[1].c_str() );
		int Balance01     = atoi( t[2].c_str() );
		if ( BalanceType01 != 1001 
		  && BalanceType01 != 1002
		  && BalanceType01 != 1003 )
		{
			continue;
		}
		
		if ( mode == 0 && Delta01 > 0 )
		{
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, lackinfo, "");
			return;
		}
		else if ( mode == 1 && Delta01 < 0 )
		{
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, lackinfo, "");
			return;
		}
		//正式开始处理
		if ( Balance01 >= 0 )
		{
			realDelta = -1 * Delta01;
		}
		else
		{
			if ( Balance01 - Delta01 > 0 )
			{
				realDelta =  Balance01 - Delta01;
			}
			else if ( Balance01 - Delta01 < 0 )
			{
				realDelta = 0;
			}
			else
			{
				realDelta = 0;
			}
		}
		
		if ( BalanceType01 == 1001 )
		{
			fee_org += realDelta;
		}
		else 
		{
			fee_add += realDelta;
		}
	}
	
	
	
	char result_org[50];
	char result_add[50];
	memset( result_org, 0, 50 );
	memset( result_add, 0, 50 );
	sprintf( result_org, "%d", fee_org );
	sprintf( result_add, "%d", fee_add );
	
	retValue.setFieldValue( 1, result_org, strlen(result_org) );
	retValue.setFieldValue( 2, result_add, strlen(result_add) );
	
	return;	
}

/*************************************************
*	函数名称:	char2vector
*	功能描述:	将用某种标点分隔字符串分隔开
*	参数列表:	char* vector<string>
*	返回结果:	1 
*************************************************/
int C_FeeAnalyze::char2vector( string in , vector<string> &out, string sep)
{	
	string in_string = in;
	string t;
	int loc;
	loc = in_string.find_first_of( sep );
	while ( loc != -1 )
	{
		t = in_string.substr(0,loc);
		out.push_back(t);
		in_string = in_string.substr(loc+1);
		loc = in_string.find_first_of( sep );
	}
	out.push_back(in_string);
	return 1;
}

/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：无
* 出口参数：无
* 返回    ：0
**************************************************/
void C_FeeAnalyze::init(char *szSourceGroupID, char *szServiceID, int index)
{
}


/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_FeeAnalyze::message(MessageParser&  pMessage)
{

}


/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_FeeAnalyze::printMe()
{
	printf("\t插件名称:PreFeeAnalyze,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_FeeAnalyze::getPluginName()
{
	return "PreFeeAnalyze";
}

std::string C_FeeAnalyze::getPluginVersion(){
	return "3.0.0";
}

