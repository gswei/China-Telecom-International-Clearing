/****************************************************************
  Project	
  Copyright (c)	2005-2007. All Rights Reserved.
		������Ѹ�Ƽ����޹�˾ 
  SUBSYSTEM:	�û��Զ�����ͷ�ļ�  
  FILE:		C_ISMPSPCheck.h
  AUTHOR:	ouyanhong   	
  Create Time: 2009-08-12
==================================================================
  Description:  
            20090812 Ч��ҵ��������š�SP��ż���Ʒ����Ƿ���SP_Info���д��ڣ���Ʒ���֧��ͨ�����*��
  UpdateRecord: 
            20091015 ����Service CapabilityID��SPID��ProductOfferID����SP_Info�����ҵ�SP_AttachID��
                     ����Service CapabilityID��SP_AttachID��SP_AttachInfo�����ҵ�SPLocalType��SPProperty��
                     ����������SP_AttachID��SPLocalType��SPSettleModel��SPProperty
==================================================================

*****************************************************************/
#ifndef _C_ISMPSPCHECK_H_
#define _C_ISMPSPCHECK_H_

#include "CF_CPlugin.h"
#include "ComFunction.h"

class	C_ISMPSPCheck: public BasePlugin
{
private:
	BaseAccessMem *table;
	char table_SPInfo[TABLENAME_LEN+1];
	char table_SPAttachInfo[TABLENAME_LEN+1];
	int m_iTableSPInfoOffset;
	int m_iTableSPAttachInfoOffset;

	DataStruct m_InData;
	DataStruct m_OutData;
	int m_iIndex;
	
	char m_szServiceName[FIELD_LEN+1];
	char m_szServiceId[FIELD_LEN+1];
	char m_szSourceGroupId[FIELD_LEN+1];
	char m_szSourceId[FIELD_LEN];
public:
	C_ISMPSPCheck();
	~C_ISMPSPCheck();	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
};

#endif
