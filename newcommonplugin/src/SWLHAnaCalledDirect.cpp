#include "SWLHAnaCalledDirect.h"

/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
C_SWLHAnaCalledDirect::C_SWLHAnaCalledDirect()
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
C_SWLHAnaCalledDirect::~C_SWLHAnaCalledDirect()
{
}


/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�����
* ���ڲ�����
* ����    ��
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
*	��������:	Execute
*	��������:	ͨ���������
*	�����б�:	����
*	���ؽ��:	����ȡ��ͨ������
*************************************************/
void C_SWLHAnaCalledDirect::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];

	if ( pps.getItem_num() != 2 )
	{
		sprintf( ErrorMsg,"��� SWLHAnaCalledDirect �������������ȷ��");
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
		//û�ҵ�
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
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_SWLHAnaCalledDirect::message(MessageParser&  pMessage)
{
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_SWLHAnaCalledDirect::printMe()
{
	printf("\t�������:SWLHAnaCalledDirect,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_SWLHAnaCalledDirect::getPluginName()
{
	return "SWLHAnaCalledDirect";
}

std::string C_SWLHAnaCalledDirect::getPluginVersion(){
	return "3.0.0";
}
