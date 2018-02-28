/****************************************************************
 filename: AnaCallDirect.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	获取业务账期
 update:

 *****************************************************************/


#include"RateCycle.h"

C_RateCycle::C_RateCycle()
{
	inData.clear();
	outData.clear();
	table = NULL;
}

void C_RateCycle::execute(PacketParser& pps,ResParser& retValue)
{
	if ( pps.getItem_num() != 6 )
	{
		char errorMsg[ERROR_MSG_LEN+1];
		sprintf( errorMsg,"插件 RateCycle 的输入参数错误！");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)errorMsg,(char *)__FILE__,__LINE__);
	}

	string strLog;

	//第一个参数是表名
	char szTableName[RECORD_LENGTH+1];
	memset(szTableName, 0, sizeof(szTableName));
	pps.getFieldValue(1,szTableName);
	DeleteSpace( szTableName );
	int iTableOffset = table->getTableOffset(szTableName);

	//第二个参数是开始时间
	char szTemp[RECORD_LENGTH+1];
	char szTime[RECORD_LENGTH+1];
	memset(szTemp, 0, sizeof(szTemp));
	pps.getFieldValue(2,szTemp);
	DeleteSpace( szTemp );
	strcpy(szTime, szTemp);
	strcpy(inData.startTime, szTime);
	
	//第三个参数是处理模式
	memset(szTemp, 0, sizeof(szTemp));
	pps.getFieldValue(3,szTemp);
	DeleteSpace( szTemp );
	int iDealMethod = atoi(szTemp);

	if(iDealMethod != 1 && iDealMethod != 2)
	{
		char errorMsg[ERROR_MSG_LEN+1];
		sprintf(errorMsg, "插件 RateCycle ：处理方式[%d]未定义！", iDealMethod);
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)errorMsg,(char *)__FILE__,__LINE__);
	}

	//第四个参数是BILLING_CONFIG_ID
	char szBillingConfigID[RECORD_LENGTH+1];
	memset(szBillingConfigID, 0, sizeof(szBillingConfigID));
	pps.getFieldValue(4,szBillingConfigID);
	DeleteSpace( szBillingConfigID );

	//第五个参数是业务代码
	char szSourceID[RECORD_LENGTH+1];
	memset(szSourceID, 0, sizeof(szSourceID));
	pps.getFieldValue(5,szSourceID);
	DeleteSpace( szSourceID );

	//第六个参数是业务代码
	char szServCatID[RECORD_LENGTH+1];
	memset(szServCatID, 0, sizeof(szServCatID));
	pps.getFieldValue(6,szServCatID);
	DeleteSpace( szServCatID );

	inData.clear();
	inData.set(szBillingConfigID);
	inData.set(szSourceID);
	inData.set(szServCatID);
	inData.itemNum = 3;
	strcpy(inData.startTime, szTime);
	//DEBUG_LOG<<"查询账期"<<szBillingConfigID<<"\t"<<szSourceID<<"\t"<<szServCatID<<"\t"<<szTime<<endd;
	strLog = "查询账期";
	strLog += szBillingConfigID;
	strLog += ";";
	strLog += szSourceID;
	strLog += ";";
	strLog += szServCatID;
	strLog += ";";
	strLog += inData.startTime;

	if(table->getData(iTableOffset, &inData, &outData, 1) == 0)
	{
		//成功获取账期资料
		//账期未封账
		if(strcmp(outData.values[2], "N") == 0)
		{
			//DEBUG_LOG<<"1输出账期："<<outData.values[4]<<endd;
			retValue.setFieldValue(1, outData.values[4], strlen(outData.values[4]));

			strLog += "\t成功获取账期资料";
			strLog += outData.values[4];

			return;
		}
		//账期己封账，处理模式为归入下一个账期
		else if(iDealMethod == 2)
		{
			strcpy(szTemp, outData.values[4]);

			strLog += "\t找到账期";
			strLog += outData.values[4];
			strLog += "已封帐";

			while(1)
			{
				strcpy(szTemp, getNextMonth(szTemp));
				inData.clear();
				inData.set(szBillingConfigID);
				//2012-06-20 修改查询条件
				inData.set(szSourceID);
				inData.set(szServCatID);
				//end of 2012-06-20
				inData.set(szTemp);
				inData.itemNum = 4;
				//strcpy(inData.startTime, szTime);
				//DEBUG_LOG<<"查询账期"<<szBillingConfigID<<"\t"<<szTemp<<"\t"<<szTime<<endd;

				strLog += "\t查询账期";
				strLog += szBillingConfigID;
				strLog += ";";
				strLog += szSourceID;
				strLog += ";";
				strLog += szServCatID;
				strLog += ";";
				strLog += szTemp;
				strLog += ";";
				strLog += inData.startTime;

				if(table->getData(iTableOffset, &inData, &outData, 2) == 0)
				{
					if(strcmp(outData.values[2], "N") == 0)
					{
						//DEBUG_LOG<<"2输出账期："<<szTemp<<endd;
						retValue.setFieldValue(1, szTemp, strlen(szTemp));
						return;
					}
				}
				else
				{
					//DEBUG_LOG<<"2 lackinfo"<<endd;
					char szLackType[RECORD_LENGTH+1];
					memset(szLackType, 0, sizeof(szLackType));
					sprintf(szLackType, "%d", LACK_RATECYCLE );
					pluginAnaResult result=eLackInfo;
					retValue.setAnaResult(result, szLackType, szTime);

					strLog += "\t无资料";
					theJSLog<<strLog.c_str()<<endw;
					return;
				}
			}
		}
		//账期己封账，处理模式为返回无资料
		else if(iDealMethod == 1)
		{
			//DEBUG_LOG<<"1 lackinfo"<<endd;
			char szLackType[RECORD_LENGTH+1];
			memset(szLackType, 0, sizeof(szLackType));
			sprintf(szLackType, "%d", LACK_RATECYCLE);
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, szLackType, szTime);

			strLog += "\t无资料";
			theJSLog<<strLog.c_str()<<endw;
			return;
		}
		//处理模式为未定义
	}
	//无法找到账期资料
	else
	{
		//DEBUG_LOG<<"lack ratecycle"<<endd;
		char szLackType[RECORD_LENGTH+1];
		sprintf(szLackType, "%d", LACK_RATECYCLE);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, szTime);

		strLog += "\t无资料";
		theJSLog<<strLog.c_str()<<endw;
		return;
	}
	
}

char* C_RateCycle::getNextMonth(char* thisMonth)
{
	char szYear[5];
	char szMonth[3];
	char szTemp[9];
	int iYear, iMonth;

	strncpy(szYear, thisMonth, 4);
	iYear = atoi(szYear);
	strncpy(szMonth, thisMonth+4, 2);
	iMonth = atoi(szMonth);
	iMonth++;

	if(iMonth > 12)
	{
		iMonth = iMonth -12;
		iYear++;
	}

	sprintf(szTemp, "%4d%02d", iYear, iMonth);
	return szTemp;
}


C_RateCycle::~C_RateCycle()
{
	delete table;
}

/*************************************************
* 描述    ：获取调试标志和设置初始化标志
* 入口参数：无
* 出口参数：无
* 返回    ：0
**************************************************/
void C_RateCycle::init(char *szSourceGroupID, char *szServiceID, int index)
{
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	m_iIndex = 1;
}

/*************************************************
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_RateCycle::message(MessageParser&  pMessage)
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
void C_RateCycle::printMe()
{
	printf("\t插件名称:RateCycle,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_RateCycle::getPluginName()
{
	return "RateCycle";
}

std::string C_RateCycle::getPluginVersion(){
	return "3.0.0";
}
