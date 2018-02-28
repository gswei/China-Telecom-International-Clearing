#include "JLDXAnaUpRate.h"

/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
C_JLDXAnaUpRate::C_JLDXAnaUpRate()
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
C_JLDXAnaUpRate::~C_JLDXAnaUpRate()
{
}


/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
void C_JLDXAnaUpRate::init(char *szSourceGroupID, char *szServiceID, int index)
{
	//theLog<<"==========AnaUpRate class begin=========="<<endi;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(m_szTableName, "I_CPID_LOCALNET2UPRATE");
	m_iTableOffset = table->getTableOffset(m_szTableName);
	m_iIndex = 1;
}


/*************************************************
* ����    ��������������ͨ�ŷ���
* ��ڲ�������������д�������ṩ�̴��롢ͨ����ʼʱ�䡢������ԭ�������ϴ���
* ���ڲ���������ͨ�ŷ���
* ����    ��0 : �ɹ�
						����������������
**************************************************/
void C_JLDXAnaUpRate::execute(PacketParser& pps,ResParser& retValue)
{
	if ( pps.getItem_num() != 3 )
	{
		sprintf( ErrorMsg,"��� AnaUpRate �������������ȷ��");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}

  	char Localnet[RECORD_LENGTH+1];
  	char CpId[RECORD_LENGTH+1];
  	char CdrBegin[RECORD_LENGTH+1];
  	
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
	char abn_code[RECORD_LENGTH+1];
  	memset( abn_code, 0, sizeof(abn_code) );
  	
  	memset( Localnet, 0, sizeof(Localnet) );   
  	pps.getFieldValue( 1, Localnet );
  	DeleteSpace( Localnet );  
			
	memset( CpId, 0, sizeof(CpId) );   
  	pps.getFieldValue( 2, CpId );
  	DeleteSpace( CpId );
  	
  	memset( CdrBegin, 0, sizeof(CdrBegin) );   
  	pps.getFieldValue( 3, CdrBegin );
  	DeleteSpace( CdrBegin );
    
	in.clear();
	out.clear();
    in.set( Localnet );
  	in.set( CpId );
	in.itemNum = 2;
  	strcpy( in.startTime, CdrBegin );
  	
	if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  	{
		in.clear();
		out.clear();
  		in.set( "*" );
  		in.set( CpId );
		in.itemNum = 2;
		strcpy( in.startTime, CdrBegin );
  		if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  		{
			in.clear();
			out.clear();
  			in.set( Localnet );
  			in.set( "*" );
			in.itemNum = 2;
  			strcpy( in.startTime, CdrBegin );
  			if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  			{
  				in.clear();
  				out.clear();
  				in.set( "*" );
  				in.set( "*" );
				in.itemNum = 2;
				strcpy( in.startTime, CdrBegin );
  				if( table->getData(m_iTableOffset, &in, &out, m_iIndex) != 0 )
  				{
					sprintf( abn_value, "%s, %s", Localnet, CpId );
  					sprintf( abn_code, "%d", LACK_UP_RATE );
					pluginAnaResult result=eLackInfo;
					retValue.setAnaResult(result, abn_code, abn_value);
  					return;
  				}
  			}
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
void C_JLDXAnaUpRate::message(MessageParser&  pMessage)
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
void C_JLDXAnaUpRate::printMe()
{
	printf("\t�������:JLDXAnaUpRate,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_JLDXAnaUpRate::getPluginName()
{
	return "JLDXAnaUpRate";
}

std::string C_JLDXAnaUpRate::getPluginVersion(){
	return "3.0.0";
}
