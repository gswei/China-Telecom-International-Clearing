#include "ISMPSPCheck.h"

/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_ISMPSPCheck::C_ISMPSPCheck()
{
	memset(&m_InData, 0, sizeof(m_InData));
	memset(&m_OutData, 0, sizeof(m_OutData));
	table = NULL;
}

/*************************************************
* ����    ������������ɾ������ָ��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_ISMPSPCheck::~C_ISMPSPCheck()
{
}

/*************************************************
* ����    ����ʼ����������ʼ����Ҫ��ȡ���õĲ���
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/ 
void C_ISMPSPCheck::init(char *szSourceGroupID, char *szServiceID, int index)
{
	strcpy(m_szSourceGroupId, szSourceGroupID);
	strcpy(m_szServiceId, szServiceID);
	strcpy(table_SPInfo, "I_SP_INFO");
	strcpy(table_SPAttachInfo, "I_SP_ATTACHINFO");
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	m_iTableSPInfoOffset = table->getTableOffset(table_SPInfo);
	m_iTableSPAttachInfoOffset = table->getTableOffset(table_SPAttachInfo);
	m_iIndex = 1;
}

/*************************************************
* ����    ��ִ�к�������ѯSP�������Ϣ��֧��ProductIDʹ��ͨ���*
* ��ڲ�����$ServiceName,$SPID,$ProductID,$StartTime,$LackinfoID
* ���ڲ�����$SP_AttachID,$SPLocalType,$SPSettleModel,$SPProperty
* ����    ��
**************************************************/  
void C_ISMPSPCheck::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	if( pps.getItem_num() != 5 )
	{
		sprintf( ErrorMsg,"��� SPInfoCheck �������������ȷ��");
		throw jsexcp::CException( ERR_LACK_PARAM, (char *)ErrorMsg, (char *)__FILE__, __LINE__);
	}

	memset(m_szServiceName, 0, sizeof(m_szServiceName));
	pps.getFieldValue(1, m_szServiceName);
	DeleteSpace(m_szServiceName);

	char szSPID[RECORD_LENGTH+1];
	memset(szSPID, 0, sizeof(szSPID));
	pps.getFieldValue(2, szSPID);
	DeleteSpace(szSPID);
	char szProductID[RECORD_LENGTH+1];
	memset(szProductID, 0, sizeof(szProductID));
	pps.getFieldValue(3, szProductID);
	DeleteSpace(szProductID);
	char szStartTime[RECORD_LENGTH+1];
	memset(szStartTime, 0, sizeof(szStartTime));
	pps.getFieldValue(4, szStartTime);
	DeleteSpace(szStartTime);
	char szLackinfoID[RECORD_LENGTH+1];
	memset(szLackinfoID, 0, sizeof(szLackinfoID));
	pps.getFieldValue(5, szLackinfoID);
	DeleteSpace(szLackinfoID);

	char szLackinfoReason[RECORD_LENGTH+1];
	memset(szLackinfoReason, 0, sizeof(szLackinfoReason));
	pps.getFieldValue(6, szLackinfoReason);
	DeleteSpace(szLackinfoReason);
	char szLackinfoCode[RECORD_LENGTH+1];
	memset(szLackinfoCode, 0, sizeof(szLackinfoCode));
	pps.getFieldValue(7, szLackinfoCode);
	DeleteSpace(szLackinfoCode);
	
	char szAttachID[RECORD_LENGTH+1];
	memset(szAttachID, 0, sizeof(szAttachID));
	char szSPLocalType[RECORD_LENGTH+1];
	memset(szSPLocalType, 0, sizeof(szSPLocalType));
	//add 20091015 by ouyh
	char szSPSettleModel[RECORD_LENGTH+1];
	memset(szSPSettleModel, 0, sizeof(szSPSettleModel));
	//end of add 20091015
	char szSPProperty[RECORD_LENGTH+1];
	memset(szSPProperty, 0, sizeof(szSPProperty));
	char szSettleFlag[RECORD_LENGTH+1];
	memset(szSettleFlag, 0, sizeof(szSettleFlag));
	char szAuditFlag[RECORD_LENGTH+1];
	memset(szAuditFlag, 0, sizeof(szAuditFlag));
	
	m_InData.clear();
	m_InData.set(m_szServiceName);
	m_InData.set(szSPID);
	m_InData.set(szProductID);
	strcpy(m_InData.startTime, szStartTime);
	m_InData.itemNum = 3;
	m_OutData.clear();
	//��SP_Info��
	if( table->getData(m_iTableSPInfoOffset, &m_InData, &m_OutData, m_iIndex)==0 )
	{
		strcpy(szAttachID, m_OutData.values[3]);
		retValue.setFieldValue(1, szAttachID, strlen(szAttachID));
	}
	else
	{
		//û���ҵ�����,��ProductID��*ƥ������һ��
		m_InData.clear();
		m_InData.set(m_szServiceName);
		m_InData.set(szSPID);
		m_InData.set("*");
		strcpy(m_InData.startTime, szStartTime);
		m_InData.itemNum = 3;
		m_OutData.clear();
		if( table->getData(m_iTableSPInfoOffset, &m_InData, &m_OutData, m_iIndex)==0 )
		{
			strcpy(szAttachID, m_OutData.values[3]);
			retValue.setFieldValue(1, szAttachID, strlen(szAttachID));
		}
		else
		{
			//��������������
			sprintf(szLackinfoCode, "%s+%s+%s", m_szServiceName, szSPID, szProductID);
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, szLackinfoID, szLackinfoCode);
			return;
		}
	}
	//add 20091015 by ouyh
	//��SP_AttachInfo��
	m_InData.clear();
	m_InData.set(m_szServiceName);
	m_InData.set(szAttachID);
	m_InData.itemNum = 2;
	m_OutData.clear();
	
	if( table->getData(m_iTableSPAttachInfoOffset, &m_InData, &m_OutData, m_iIndex)==0 )
	{
		strcpy(szSPLocalType, m_OutData.values[2]);
		strcpy(szSPSettleModel, m_OutData.values[3]);
		strcpy(szSPProperty, m_OutData.values[4]);
		strcpy(szSettleFlag, m_OutData.values[5]);
		strcpy(szAuditFlag, m_OutData.values[6]);
		retValue.setFieldValue(2, szSPLocalType, strlen(szSPLocalType));
		retValue.setFieldValue(3, szSPSettleModel, strlen(szSPSettleModel));
		retValue.setFieldValue(4, szSPProperty, strlen(szSPProperty));
		retValue.setFieldValue(5, szSettleFlag, strlen(szSettleFlag));
		retValue.setFieldValue(6, szAuditFlag, strlen(szAuditFlag));
		
		return;
	}
	else
	{
		//��������������
		sprintf(szLackinfoCode, "%s+%s+%s", m_szServiceName, szSPID, szProductID);
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, szLackinfoID, szLackinfoCode);
		return;
	}
}

/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_ISMPSPCheck::message(MessageParser&  pMessage)
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
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_ISMPSPCheck::printMe()
{
	printf("\t�������:ISMP_SPCheck,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_ISMPSPCheck::getPluginName()
{
	return "ISMP_SPCheck";
}

std::string C_ISMPSPCheck::getPluginVersion(){
	return "3.0.0";
}
