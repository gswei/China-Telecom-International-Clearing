/****************************************************************
  Project	
  Copyright (c)	2005-2007. All Rights Reserved.
		广州亿迅科技有限公司 
  SUBSYSTEM:	用户自定义插件头文件  
  FILE:		C_ISMPSPCheck.h
  AUTHOR:	ouyanhong   	
  Create Time: 2009-08-12
==================================================================
  Description:  
            20090812 效验业务能力编号、SP编号及产品编号是否在SP_Info表中存在，产品编号支持通配符“*”
  UpdateRecord: 
            20091015 输入Service CapabilityID、SPID、ProductOfferID，在SP_Info表中找到SP_AttachID，
                     再用Service CapabilityID和SP_AttachID到SP_AttachInfo表中找到SPLocalType和SPProperty。
                     最后输出的是SP_AttachID、SPLocalType、SPSettleModel和SPProperty
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
