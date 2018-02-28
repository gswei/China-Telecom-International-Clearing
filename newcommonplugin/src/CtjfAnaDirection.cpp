#include "CtjfAnaDirection.h"


/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_CtjfAnaDirection::C_CtjfAnaDirection()
{
	m_InData.clear();
	m_OutData.clear();
	table = NULL;
}

/*************************************************
* ����    ������������ɾ�������ṹ��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_CtjfAnaDirection::~C_CtjfAnaDirection()
{
}

/*************************************************
* ����    ����ʼ����������ʼ����ȡ�����������
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_CtjfAnaDirection::init(char *szSourceGroupID, char *szServiceID, int index)
{
	theJSLog<<"==========CtjfAnaDirection class begin=========="<<endd;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	strcpy(table_RouteType, "I_ROUTE_TYPE");
	strcpy(table_RouteUseWay, "I_ROUTETYPE_USEWAY");
	m_iTableRouteUseWayOffset = table->getTableOffset(table_RouteType);
	m_iTableRouteTypeOffset = table->getTableOffset(table_RouteUseWay);
	m_iIndex = 1;
}


/*************************************************
* ����    ��ִ�к�����ͨ����ѯ�������͵�ʹ�÷�ʽ���жϳ���ȥת��־
* ��ڲ�����$OutRouteType,$InRouteType,$Type
* ���ڲ�����$DirectFlag
* ����    ��0 ���� �����ϴ���
**************************************************/
void C_CtjfAnaDirection::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	if (pps.getItem_num() != 3)
	{
		sprintf( ErrorMsg,"��� IsFreeNbr �������������ȷ��");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
  	
  	memset( m_szOutRouteType,0,sizeof( m_szOutRouteType ) );   
  	pps.getFieldValue( 1,m_szOutRouteType );
  	DeleteSpace( m_szOutRouteType );
  	
  	memset( m_szInRouteType, 0, sizeof( m_szInRouteType ) );   
  	pps.getFieldValue( 2, m_szInRouteType );
  	DeleteSpace( m_szInRouteType );
  	
  	memset( m_szType, 0, sizeof( m_szType ) );   
  	pps.getFieldValue( 3, m_szType );
  	DeleteSpace( m_szType );
  	//ȡsource��3��4��λ
  	m_szType[0] = m_szType[2];
  	m_szType[1] = m_szType[3];
  	m_szType[2] = 0;
  	
  	char abn_value[RECORD_LENGTH+1];
  	memset( abn_value, 0, sizeof(abn_value) );
  	
  	//========================================================================== 
  	//��ȡ��·������
	m_InData.clear();
	m_OutData.clear();
  	m_InData.set(m_szOutRouteType);
	m_InData.itemNum = 1;
	if( table->getData(m_iTableRouteTypeOffset, &m_InData, &m_OutData, m_iIndex) != 0 )
  	{
		memset( abn_value, 0, sizeof(abn_value) );
		sprintf( abn_value, "%d", LACK_OUTROUTE_ROUTE_TYPE );
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, abn_value, m_szOutRouteType);
		return;
  	}
  	else 
  	{
		strcpy(m_szOutUseWay,m_OutData.values[1]);
  	}
    
    //��ȡ��·������
  	m_InData.clear();
	m_OutData.clear();
  	m_InData.set(m_szInRouteType);
	m_InData.itemNum = 1;
	if( table->getData(m_iTableRouteTypeOffset, &m_InData, &m_OutData, m_iIndex) != 0 )
  	{
		memset( abn_value, 0, sizeof(abn_value) );
		sprintf( abn_value, "%d", LACK_INROUTE_ROUTE_TYPE );
		pluginAnaResult result=eLackInfo;
		retValue.setAnaResult(result, abn_value, m_szInRouteType);
		return;
  	}
  	else 
  	{
		strcpy(m_szInUseWay,m_OutData.values[1]);
  	}
  
  	//���ݳ���·�����͵�ʹ�÷�ʽ���ж� ��/ȥ/ת��־
  	m_InData.clear();
	m_OutData.clear();
  	m_InData.set(m_szOutUseWay);
  	m_InData.set(m_szInUseWay);
  	m_InData.set(m_szType );
	m_InData.itemNum = 3;
	if( table->getData(m_iTableRouteUseWayOffset, &m_InData, &m_OutData, m_iIndex) == 0 )
  	{
		strcpy(m_szDirectFlag,m_OutData.values[3]);
		retValue.setFieldValue( 1, m_szDirectFlag, strlen(m_szDirectFlag) );	
		return;
  	}
  	else
  	{
  		m_InData.clear();
		m_OutData.clear();
  		m_InData.set(m_szOutUseWay);
  		m_InData.set(m_szInUseWay);
  		m_InData.set( "==" );
		m_InData.itemNum = 3;
		if( table->getData(m_iTableRouteUseWayOffset, &m_InData, &m_OutData, m_iIndex) == 0 )
  		{
			strcpy(m_szDirectFlag,m_OutData.values[3]);
			retValue.setFieldValue( 1, m_szDirectFlag, strlen(m_szDirectFlag) );
			return;
  		}
  		else
  		{
  			//��������������ϵľ���ԭ��
  			char szLackinfoReaSon[RECORD_LENGTH+1];
  			sprintf( szLackinfoReaSon, "%s+%s", m_szOutUseWay, m_szInUseWay);
  			memset( abn_value, 0, sizeof(abn_value) );
  			sprintf( abn_value, "%d", LACK_ROUTE_ROUTE_USEWAY );
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, abn_value, szLackinfoReaSon);
  			return;
		}
	}
	return;
}

/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_CtjfAnaDirection::message(MessageParser&  pMessage)
{
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_CtjfAnaDirection::printMe()
{
	printf("\t�������:CTJFAnaDirection,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_CtjfAnaDirection::getPluginName()
{
	return "CTJFAnaDirection";
}

std::string C_CtjfAnaDirection::getPluginVersion(){
	return "3.0.0";
}
