#include "ISMPAttachId.h"

C_ISMPAttachId::C_ISMPAttachId()
{
	memset( &in, 0, sizeof(in) );
	memset( &out, 0, sizeof(out) );
	table = NULL;
}

C_ISMPAttachId::~C_ISMPAttachId()
{
		
}

void C_ISMPAttachId::init(char *szSourceGroupID, char *szServiceID, int index)
{
	strcpy(table1, "I_ISMP_ATTACH_ID_MAPPING");
	strcpy(table2, "I_ISMP_SP_INFO");
	m_iIndex = 1;
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
	m_iTableOffset1 = table->getTableOffset(table1);
	m_iTableOffset2 = table->getTableOffset(table2);
}

void C_ISMPAttachId::execute(PacketParser& pps,ResParser& retValue)
{
	char ErrorMsg[ERROR_MSG_LEN+1];
	if (pps.getItem_num() != 3)
	{
		sprintf( ErrorMsg,"the params sent to ISMP_AttachId is lack");
		throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
	}

	memset( sp_id,          0, sizeof(sp_id)          );  
  	memset( service_cap_id, 0, sizeof(service_cap_id) );  
  	memset( attach_id,      0, sizeof(attach_id)      );  
  	memset( cdr_begin,      0, sizeof(cdr_begin)      );  
  	 
  	pps.getFieldValue( 1, sp_id );  
	pps.getFieldValue( 2, service_cap_id );
	pps.getFieldValue( 3, cdr_begin );
  	
  	DeleteSpace( sp_id );
  	DeleteSpace( service_cap_id );
  	DeleteSpace( cdr_begin );
  	
  	//====================================================================================
  	//��ISMP_ATTACH_ID_MAPPING���ѯattachid  ȫ����ȷƥ��
  	in.clear();
  	out.clear();
  	in.set( sp_id );
  	in.set( service_cap_id );
	strcpy( in.startTime, cdr_begin );
	in.itemNum = 2;

  	//====================================================================================
	if(table->getData(m_iTableOffset1, &in, &out, m_iIndex)!=0)
  	{
		//��ISMP_ATTACH_ID_MAPPING���ѯattachid  �鲻��
  		//====================================================================================
  		//��ISMP_ATTACH_ID_MAPPING���ѯattachid  ģ��ƥ��
  		in.clear();
		out.clear();
  		in.set( sp_id );
  		in.set( "=" );
		strcpy( in.startTime, cdr_begin );
		in.itemNum = 2;
  		
		//====================================================================================
		if(table->getData(m_iTableOffset1, &in, &out, m_iIndex)!=0)
		{
			//��ISMP_ATTACH_ID_MAPPING���ѯattachid  �鲻��
  			//====================================================================================
  			//��ISMP_SP_INFO���ѯattachid  ��ȷƥ��
			in.clear();
			out.clear();
  			in.set( sp_id );
			strcpy( in.startTime, cdr_begin );
			in.itemNum = 1;
			if(table->getData(m_iTableOffset2, &in, &out, m_iIndex)!=0)
			{
				//��ISMP_SP_INFO���ѯattachid  �鲻��
				pluginAnaResult result=eLackInfo;
				sprintf( abn_value, "%d", ISMP_Lack_Info );
				sprintf( abn_content, "%s_=%s=", "ISMP_SP_INFO", sp_id );
				retValue.setAnaResult(result, abn_value, abn_content);
				return;
			}
			//====================================================================================
			else
			{
				//��ISMP_SP_INFO���ѯattachid  �鵽
				strcpy( attach_id,  out.values[1] );
				retValue.setFieldValue( 1, attach_id, strlen(attach_id) );
				return;
			}
		}
		//====================================================================================
		else
		{
			//��ISMP_ATTACH_ID_MAPPING���ѯattachid  �鵽
			strcpy( attach_id,  out.values[2] );
			retValue.setFieldValue( 1, attach_id, strlen(attach_id) );
			return;
		}
	}
	//====================================================================================
	else
	{
		//��ISMP_ATTACH_ID_MAPPING���ѯattachid  �鵽
		strcpy( attach_id,  out.values[2] );
		retValue.setFieldValue( 1, attach_id, strlen(attach_id) );
		return;
	}
			
	return;	
}

/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_ISMPAttachId::message(MessageParser&  pMessage)
{
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_ISMPAttachId::printMe()
{
	printf("\t�������:ISMP_AnaAttachId,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_ISMPAttachId::getPluginName()
{
	return "ISMP_AnaAttachId";
}

std::string C_ISMPAttachId::getPluginVersion(){
	return "3.0.0";
}
