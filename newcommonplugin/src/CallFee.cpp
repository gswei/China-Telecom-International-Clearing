/****************************************************************
  Project	
  Copyright (c)	2005-2006. All Rights Reserved.
		广州普信科技有限公司 
  SUBSYSTEM:	用户自定义插件头文件  
  FILE:		C_CallFee.cpp
  AUTHOR:	weixy
  Create Time: 2007-11-19
==================================================================
  Description:  
            
            主要功能: 计费免费分析
				
  UpdateRecord: 
==================================================================

 *****************************************************************/
#include "CallFee.h"

C_CallFee::C_CallFee()
{
	initFlag=0;
	table = NULL;
}

C_CallFee::~C_CallFee()
{
	if(table != NULL)
		delete table;
}

void C_CallFee::execute(PacketParser& pps,ResParser& retValue)
{
	//插件执行入口
	if ( pps.getItem_num() != 5)
	{
		sprintf( ErrorMsg,"插件 IsFreeNbr 的输入参数不正确！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);		
	}
			
	if( initFlag != 1 )
		Init( pps );

	//分析处理过程

	//得到输入参数
	getInputParams(pps);

	//传递到CallNumber 类
	setParam();
	//按主被叫分情况处理
	char tmp_str[FIELD_LEN];

	if (strcmp(m_szNbrType,"10")==0)
	{
		//被叫号码，查询免费号码级别，查不到则默认计费
		
		//接入号级别
		m_callnumber.Set_ChargeFlag(BEF_TOLL_LEVEL);
		if (!m_callnumber.queryFreeTele())
			getParam();
		else
		{
			//区号级别
			m_callnumber.DealBefTelServ();
			
			m_callnumber.Set_ChargeFlag(TOLL_LEVEL);
			
			if (!m_callnumber.queryFreeTele())
				getParam();
			else
			{
				//区号后级别
				//区号后分析只对国内区号有效
				m_callnumber.queryTollcode(m_szAbnField);
				m_callnumber.DealTollcode();
				m_callnumber.Get_Tollcode(tmp_str);
				if (strncmp(tmp_str,"00",2)!=0)
				{
					m_callnumber.Get_AfterTollTel(tmp_str);
					m_callnumber.Set_CallNbr(tmp_str);
					m_callnumber.Set_ChargeFlag(AFTER_TOLL_LEVEL);
					if(!m_callnumber.queryFreeTele())
						getParam();
					else
						strcpy(m_szChargeFlag,"Y");	
				}
				else
				{
					strcpy(m_szChargeFlag,"Y");
				}
			}
		}
	}

	else
	{
		//主叫，计费号码查询号码计费，查不到则默认免费
		/*
		1、判断是否特殊计费主叫（资料表free_telheader中teleno_level为Y）
		2、判断特殊不计费主叫（资料表free_telheader中teleno_level为1、2、3，分3种级别，由高至低判断（1最高）），2-8 开头的号码此时判到的信息，如果与第一点中结果冲突，则覆盖之前判断特殊计费主叫的信息。
		3、不能判到任何信息的号码做特殊代码挂起。
		*/

		//接入号级别
		m_callnumber.Set_ChargeFlag(BEF_TOLL_LEVEL);
		if (!m_callnumber.queryFreeTele())
			getParam();
		else
		{
			//区号级别
			m_callnumber.DealBefTelServ();
			m_callnumber.Set_ChargeFlag(TOLL_LEVEL);
			if (!m_callnumber.queryFreeTele())
				getParam();
			else
			{
				//号码级别
				m_callnumber.queryTollcode(m_szAbnField);
				m_callnumber.DealTollcode();
				m_callnumber.Get_AfterTollTel(tmp_str);
				m_callnumber.Set_CallNbr(tmp_str);
				m_callnumber.Set_ChargeFlag(AFTER_TOLL_LEVEL);
				if(!m_callnumber.queryFreeTele())
					getParam();
				else
				{
					//没有不计费的资料，则按照计费查找
					m_callnumber.Set_ChargeFlag("Y");
					if(!m_callnumber.queryFreeTele())
						getParam();
					else
					{
						//也没有计费的资料，则默认挂起
						pluginAnaResult result=eLackInfo;
						sprintf(m_sAbnReason,"%d",m_iAbnReason);
						retValue.setAnaResult(result, m_sAbnReason, tmp_str);
						return;
					}	
				}//号码级别	
			}//区号级别
		}//接入码级别
	}//主叫

	sendOutputParams(retValue);
	return;
}

int C_CallFee::Init(PacketParser &pps)
{
	/*get some of input params which will not change after first coming*/

	memset(m_LastSerCatId,0,sizeof(m_LastSerCatId));
	pps.getFieldValue(1,m_LastSerCatId);
	DeleteSpace( m_LastSerCatId ); 

	memset(m_LastSourceId, 0, sizeof(m_LastSourceId));
	strcpy(m_LastSourceId, m_szSourceId);
	DeleteSpace( m_LastSourceId ); 
	
	//初始化号码类
	//m_callnumber.Init(m_szServiceId, m_LastSourceId, table);
	m_callnumber.Reset(m_szServiceId, m_LastSourceId);

	/* set init_flag of this plugin */
	initFlag=1;
	return 0;
}



int C_CallFee::getInputParams(PacketParser & pps)
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

	return 0;
}


int  C_CallFee::sendOutputParams(ResParser& retValue)
{
	retValue.setFieldValue( 1, m_szChargeFlag, strlen(m_szChargeFlag) );

	return 0;
}

int C_CallFee::setParam()
{
	m_callnumber.Set_Value();
	m_callnumber.Set_ServCatId(m_szServCatId);
	m_callnumber.Set_BefTollCode(m_szBefTollcode);
	m_callnumber.Set_CallNbr(m_szCallNbr);
	m_callnumber.Set_NbrType(m_szNbrType);
	m_callnumber.Set_CdrBegin(m_szCdrBegin);
	return 0;
}

int C_CallFee::getParam()
{
	m_callnumber.Get_ChargeFlag(m_szChargeFlag);
	return 0;
}

/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：无
* 出口参数：无
* 返回    ：0
**************************************************/
void C_CallFee::init(char *szSourceGroupID, char *szServiceID, int index)
{
	memset( m_szSourceGroupId, 0, sizeof( m_szSourceGroupId ) );   
	strcpy( m_szSourceGroupId, szSourceGroupID );
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
void C_CallFee::message(MessageParser&  pMessage)
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
void C_CallFee::printMe()
{
	printf("\t插件名称:IsFreeNbr,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_CallFee::getPluginName()
{
	return "IsFreeNbr";
}

std::string C_CallFee::getPluginVersion(){
	return "3.0.0";
}
