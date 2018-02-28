#include "JLDXAnaAttachId.h"

/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
C_JLDXAnaAttachId::C_JLDXAnaAttachId()
{
	in.clear();
	out.clear();
	table = NULL;
	iIndex = 1;
}


/*************************************************
* ����    ����������
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
C_JLDXAnaAttachId::~C_JLDXAnaAttachId()
{
}


/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
void C_JLDXAnaAttachId::init(char *szSourceGroupID, char *szServiceID, int index)
{
	//theLog<<"==========AnaCpAttachId class begin=========="<<endd;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_JLDX_CPATTACH");
	m_iTableOffset = table->getTableOffset(m_szTableName);
	iIndex = 1;
}


/*************************************************
* ����    ����ȡattachid
* ��ڲ����������ṩ�̴��롢����
* ���ڲ�����attachid
* ����    ��
**************************************************/
void C_JLDXAnaAttachId::execute(PacketParser& pps,ResParser& retValue)
{
	
	if (pps.getItem_num() != 2)
	{
		sprintf( ErrorMsg,"��� JLDXAnaAttachId �������������ȷ��");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}

  	char CpId[RECORD_LENGTH+1];
  	char ServiceId[RECORD_LENGTH+1];
  	
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
  	
  	memset( CpId, 0, sizeof(CpId) );   
  	pps.getFieldValue( 1, CpId );
  	DeleteSpace( CpId );  
		
  	memset( ServiceId, 0, sizeof(ServiceId) );   
  	pps.getFieldValue( 2, ServiceId );
  	DeleteSpace( ServiceId );
  	
	in.clear();
	out.clear();
  	in.set( CpId );
  	in.set( ServiceId );
	in.itemNum = 2;

  	if( table->getData(m_iTableOffset, &in, &out, iIndex) != 0 )
  	{
  		in.clear();
  		out.clear();
  		in.set( CpId );
  		in.set( "*" );
  		in.itemNum = 2;

		if( table->getData(m_iTableOffset, &in, &out, iIndex) != 0 )
		{
			sprintf( abn_value, "%s, %s", CpId, ServiceId );
			char szLackType[RECORD_LENGTH+1];
			sprintf( szLackType, "%d", LACK_CPATTACH );
			DeleteSpace(szLackType);
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, szLackType, abn_value);
			return;
		}
	}
	
	retValue.setFieldValue( 1, out.values[2], strlen(out.values[2]) );	
	return;	
}

/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_JLDXAnaAttachId::message(MessageParser&  pMessage)
{
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_JLDXAnaAttachId::printMe()
{
	printf("\t�������:JLDXAnaAttachId,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_JLDXAnaAttachId::getPluginName()
{
	return "JLDXAnaAttachId";
}

std::string C_JLDXAnaAttachId::getPluginVersion(){
	return "3.0.0";
}
