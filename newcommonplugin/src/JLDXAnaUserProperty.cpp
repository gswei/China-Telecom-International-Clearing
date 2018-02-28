#include "JLDXAnaUserProperty.h"

/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
C_JLDXAnaUserProperty::C_JLDXAnaUserProperty()
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
C_JLDXAnaUserProperty::~C_JLDXAnaUserProperty()
{
}


/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：
* 出口参数：
* 返回    ：
**************************************************/
void C_JLDXAnaUserProperty::init(char *szSourceGroupID, char *szServiceID, int index)
{
	//theLog<<"==========AnaUserProperty class begin=========="<<endi;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_JLDX_USER_PROPERTY");
	m_iTableOffset = table->getTableOffset(m_szTableName);
	m_iIndex = 1;
}


/*************************************************
* 描述    根据号码分析用户属性类型
* 入口参数：号码、通话开始时间、文件类型
* 出口参数：用户属性类型
* 返回    ：0 : 成功
						其它：无资料类型
**************************************************/
void C_JLDXAnaUserProperty::execute(PacketParser& pps,ResParser& retValue)
{
	if ( pps.getItem_num() != 3 )
	{
		sprintf( ErrorMsg,"插件 AnaUserProperty 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
	
  	char Teleno[RECORD_LENGTH+1];
  	char CdrBegin[RECORD_LENGTH+1];
  	char FileType[RECORD_LENGTH+1];
  
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
	char abn_code[RECORD_LENGTH+1];
  	memset( abn_code, 0, sizeof(abn_code) );
  	
  	memset( Teleno, 0, sizeof(Teleno) );   
  	pps.getFieldValue( 1, Teleno );
  	DeleteSpace( Teleno );  
		
	memset( CdrBegin, 0, sizeof(CdrBegin) );   
  	pps.getFieldValue( 2, CdrBegin );
  	DeleteSpace( CdrBegin );
  	
  	memset( FileType, 0, sizeof(FileType) );   
  	pps.getFieldValue( 3, FileType );
  	DeleteSpace( FileType ); 
    
	in.clear();
	out.clear();
  	in.set( Teleno );
	in.itemNum = 1;
  	strcpy( in.startTime, CdrBegin );
  	
  	if(table->getData(m_iTableOffset, &in, &out, m_iIndex) == 0)
	{
		retValue.setFieldValue(1, out.values[1], strlen(out.values[1]) );	
		return;	
	}
	else
	{
		char temp[10];
		/*
  		if( strcmp(FileType, "pre") == 0 )
  		{	
  			sprintf( temp, "1" );
  		}
  		else if( strcmp(FileType, "post") == 0 )
  		{
  			sprintf( temp, "0" );
  		}
  		*/
  		if(strcmp(FileType, "1") == 0)
  		{
  			return;
  		}
  		else if(strcmp(FileType, "0") == 0)
  		{
  			return;
  		}
  		else 
  		{
			sprintf( abn_value, "%s, %s", Teleno, FileType );
  			sprintf( abn_code, "%d", LACK_USER_PROPERTY );
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, abn_code, abn_value);
  			return;
  		}
  		//retValue.setFieldValue(1, temp, strlen(temp));	
		return;
	}
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaUserProperty::message(MessageParser&  pMessage)
{
}

/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_JLDXAnaUserProperty::printMe()
{
	printf("\t插件名称:JLDXAnaUserProperty,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_JLDXAnaUserProperty::getPluginName()
{
	return "JLDXAnaUserProperty";
}

std::string C_JLDXAnaUserProperty::getPluginVersion(){
	return "3.0.0";
}
