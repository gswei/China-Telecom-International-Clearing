/****************************************************************
 filename: AnaCallDirect.h
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	��ȡҵ������
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
		sprintf( errorMsg,"��� RateCycle �������������");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)errorMsg,(char *)__FILE__,__LINE__);
	}

	string strLog;

	//��һ�������Ǳ���
	char szTableName[RECORD_LENGTH+1];
	memset(szTableName, 0, sizeof(szTableName));
	pps.getFieldValue(1,szTableName);
	DeleteSpace( szTableName );
	int iTableOffset = table->getTableOffset(szTableName);

	//�ڶ��������ǿ�ʼʱ��
	char szTemp[RECORD_LENGTH+1];
	char szTime[RECORD_LENGTH+1];
	memset(szTemp, 0, sizeof(szTemp));
	pps.getFieldValue(2,szTemp);
	DeleteSpace( szTemp );
	strcpy(szTime, szTemp);
	strcpy(inData.startTime, szTime);
	
	//�����������Ǵ���ģʽ
	memset(szTemp, 0, sizeof(szTemp));
	pps.getFieldValue(3,szTemp);
	DeleteSpace( szTemp );
	int iDealMethod = atoi(szTemp);

	if(iDealMethod != 1 && iDealMethod != 2)
	{
		char errorMsg[ERROR_MSG_LEN+1];
		sprintf(errorMsg, "��� RateCycle ������ʽ[%d]δ���壡", iDealMethod);
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)errorMsg,(char *)__FILE__,__LINE__);
	}

	//���ĸ�������BILLING_CONFIG_ID
	char szBillingConfigID[RECORD_LENGTH+1];
	memset(szBillingConfigID, 0, sizeof(szBillingConfigID));
	pps.getFieldValue(4,szBillingConfigID);
	DeleteSpace( szBillingConfigID );

	//�����������ҵ�����
	char szSourceID[RECORD_LENGTH+1];
	memset(szSourceID, 0, sizeof(szSourceID));
	pps.getFieldValue(5,szSourceID);
	DeleteSpace( szSourceID );

	//������������ҵ�����
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
	//DEBUG_LOG<<"��ѯ����"<<szBillingConfigID<<"\t"<<szSourceID<<"\t"<<szServCatID<<"\t"<<szTime<<endd;
	strLog = "��ѯ����";
	strLog += szBillingConfigID;
	strLog += ";";
	strLog += szSourceID;
	strLog += ";";
	strLog += szServCatID;
	strLog += ";";
	strLog += inData.startTime;

	if(table->getData(iTableOffset, &inData, &outData, 1) == 0)
	{
		//�ɹ���ȡ��������
		//����δ����
		if(strcmp(outData.values[2], "N") == 0)
		{
			//DEBUG_LOG<<"1������ڣ�"<<outData.values[4]<<endd;
			retValue.setFieldValue(1, outData.values[4], strlen(outData.values[4]));

			strLog += "\t�ɹ���ȡ��������";
			strLog += outData.values[4];

			return;
		}
		//���ڼ����ˣ�����ģʽΪ������һ������
		else if(iDealMethod == 2)
		{
			strcpy(szTemp, outData.values[4]);

			strLog += "\t�ҵ�����";
			strLog += outData.values[4];
			strLog += "�ѷ���";

			while(1)
			{
				strcpy(szTemp, getNextMonth(szTemp));
				inData.clear();
				inData.set(szBillingConfigID);
				//2012-06-20 �޸Ĳ�ѯ����
				inData.set(szSourceID);
				inData.set(szServCatID);
				//end of 2012-06-20
				inData.set(szTemp);
				inData.itemNum = 4;
				//strcpy(inData.startTime, szTime);
				//DEBUG_LOG<<"��ѯ����"<<szBillingConfigID<<"\t"<<szTemp<<"\t"<<szTime<<endd;

				strLog += "\t��ѯ����";
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
						//DEBUG_LOG<<"2������ڣ�"<<szTemp<<endd;
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

					strLog += "\t������";
					theJSLog<<strLog.c_str()<<endw;
					return;
				}
			}
		}
		//���ڼ����ˣ�����ģʽΪ����������
		else if(iDealMethod == 1)
		{
			//DEBUG_LOG<<"1 lackinfo"<<endd;
			char szLackType[RECORD_LENGTH+1];
			memset(szLackType, 0, sizeof(szLackType));
			sprintf(szLackType, "%d", LACK_RATECYCLE);
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, szLackType, szTime);

			strLog += "\t������";
			theJSLog<<strLog.c_str()<<endw;
			return;
		}
		//����ģʽΪδ����
	}
	//�޷��ҵ���������
	else
	{
		//DEBUG_LOG<<"lack ratecycle"<<endd;
		char szLackType[RECORD_LENGTH+1];
		sprintf(szLackType, "%d", LACK_RATECYCLE);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackType, szTime);

		strLog += "\t������";
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
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_RateCycle::init(char *szSourceGroupID, char *szServiceID, int index)
{
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	m_iIndex = 1;
}

/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
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
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_RateCycle::printMe()
{
	printf("\t�������:RateCycle,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_RateCycle::getPluginName()
{
	return "RateCycle";
}

std::string C_RateCycle::getPluginVersion(){
	return "3.0.0";
}
