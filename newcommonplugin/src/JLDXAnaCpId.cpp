#include "JLDXAnaCpId.h"

/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
C_JLDXAnaCpId::C_JLDXAnaCpId()
{
	in.clear();
	out.clear();
	table = NULL;
}


/*************************************************
* ����    ����������
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
C_JLDXAnaCpId::~C_JLDXAnaCpId()
{
		
}


/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�����
* ���ڲ�����
* ����    ��
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
* ����    ���������ṩ�̴���
* ��ڲ����������ṩ�̴��롢ͨ����ʼʱ��
* ���ڲ�����cpid��lackinfo_value
* ����    ��0 : �ɹ�
					����������������
**************************************************/
void C_JLDXAnaCpId::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	if (pps.getItem_num() != 2)
	{
		sprintf( ErrorMsg,"��� JLDXAnaCpId �������������ȷ��" );
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
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_JLDXAnaCpId::message(MessageParser&  pMessage)
{
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_JLDXAnaCpId::printMe()
{
	printf("\t�������:JLDXAnaCpId,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_JLDXAnaCpId::getPluginName()
{
	return "JLDXAnaCpId";
}

std::string C_JLDXAnaCpId::getPluginVersion(){
	return "3.0.0";
}
