#include "JLDXAnaUserProperty.h"

/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�����
* ���ڲ�����
* ����    ��
**************************************************/
C_JLDXAnaUserProperty::C_JLDXAnaUserProperty()
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
C_JLDXAnaUserProperty::~C_JLDXAnaUserProperty()
{
}


/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�����
* ���ڲ�����
* ����    ��
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
* ����    ���ݺ�������û���������
* ��ڲ��������롢ͨ����ʼʱ�䡢�ļ�����
* ���ڲ������û���������
* ����    ��0 : �ɹ�
						����������������
**************************************************/
void C_JLDXAnaUserProperty::execute(PacketParser& pps,ResParser& retValue)
{
	if ( pps.getItem_num() != 3 )
	{
		sprintf( ErrorMsg,"��� AnaUserProperty �������������ȷ��");
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
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_JLDXAnaUserProperty::message(MessageParser&  pMessage)
{
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_JLDXAnaUserProperty::printMe()
{
	printf("\t�������:JLDXAnaUserProperty,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_JLDXAnaUserProperty::getPluginName()
{
	return "JLDXAnaUserProperty";
}

std::string C_JLDXAnaUserProperty::getPluginVersion(){
	return "3.0.0";
}
