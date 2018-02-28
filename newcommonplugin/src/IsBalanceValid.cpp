#include "IsBalanceValid.h"

/*************************************************
*	��������:	Execute
*	��������:	���������Ч����
*	�����б�:	$input_time,$valid_time
*	���ؽ��:	0,1,2
*************************************************/
void C_IsBalanceValid::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];

	if (pps.getItem_num() != 2)
  	{
  		sprintf( ErrorMsg,"��� IsBalanceValid �������������ȷ��");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}
	
	memset( input_time, 0, sizeof(input_time) );
	memset( valid_time, 0, sizeof(valid_time) );
	
	pps.getFieldValue( 1, input_time );
	pps.getFieldValue( 2, valid_time );	
	
	buff = input_time;
	year_month_i = atoi( buff.substr(0,6).c_str() );
	day_i        = atoi( buff.substr(6,2).c_str() );
	
	buff = valid_time;
	year_month_v = atoi( buff.substr(0,6).c_str() );
	day_v        = atoi( buff.substr(6,2).c_str() );
	
	//======================================================
	//�ж��Ƿ����
	if ( year_month_v > year_month_i )
	{
		retValue.setFieldValue( 1, "0", strlen("0") );
	}
	
	else if ( year_month_v == year_month_i )
	{
		if ( day_v >= day_i )
		{
			retValue.setFieldValue( 1, "0", strlen("0") );
		}
		else
		{
			retValue.setFieldValue( 1, "1", strlen("1") );
		}
	}
	
	else if ( year_month_v < year_month_i )
	{
		if ( (year_month_v+3) > year_month_i )
		{
			retValue.setFieldValue( 1, "1", strlen("1") );
		}
		
		else if ( (year_month_v+3) == year_month_i )
		{
			if ( day_v >= day_i )
			{
				retValue.setFieldValue( 1, "1", strlen("1") );
			}
			else
			{
				retValue.setFieldValue( 1, "2", strlen("2") );
			}
		}
		else if ( (year_month_v+3) < year_month_i )             
		{
			retValue.setFieldValue( 1, "2", strlen("2") );
		}
	}
	
	//======================================================
	//�ж��Ƿ��¹���
	if ( year_month_i == year_month_v )
	{
		retValue.setFieldValue( 2, "1", strlen("1") );
	}
	else
	{
		retValue.setFieldValue( 2, "0", strlen("0") );
	}
	return;	
}


/*************************************************
* ����    ����ȡ���Ա�־�����ó�ʼ����־
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/
void C_IsBalanceValid::init(char *szSourceGroupID, char *szServiceID, int index)
{
}


/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_IsBalanceValid::message(MessageParser&  pMessage)
{
}


/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_IsBalanceValid::printMe()
{
	printf("\t�������:IsBalanceValid,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_IsBalanceValid::getPluginName()
{
	return "IsBalanceValid";
}

std::string C_IsBalanceValid::getPluginVersion(){
	return "3.0.0";
}


