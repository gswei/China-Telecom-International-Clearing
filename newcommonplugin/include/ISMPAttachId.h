 /****************************************************************
 filename: CallNumber.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-12
 version: 3.0.0
 description: 
	ISMP业务ATTACH_ID匹配插件
 update:

 *****************************************************************/
#ifndef C_ISMPATTACHID
#define C_ISMPATTACHID

#include "CF_CPlugin.h"
#include "ComFunction.h"

const int ISMP_Lack_Info = 2009901;
const int ISMP_Out_Time  = 2009902;

class C_ISMPAttachId : public BasePlugin
{
public: 
	C_ISMPAttachId();
	~C_ISMPAttachId();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

private:
	char m_szServiceId[RECORD_LENGTH+1];
	char m_szSourceGroupId[RECORD_LENGTH+1];
	char m_szServerId[RECORD_LENGTH+1];
	int m_iIndex;
	
	BaseAccessMem *table;
	char table1[TABLENAME_LEN+1];
	char table2[TABLENAME_LEN+1]; 
	int m_iTableOffset1;
	int m_iTableOffset2;
	DataStruct in;
	DataStruct out;
	
	char sp_id[RECORD_LENGTH+1];
	char service_cap_id[RECORD_LENGTH+1];
	char attach_id[RECORD_LENGTH+1];
	char cdr_begin[RECORD_LENGTH+1];
	
	char start_time[RECORD_LENGTH+1];
	char end_time[RECORD_LENGTH+1];
	
	char abn_value[RECORD_LENGTH+1];
	char abn_content[RECORD_LENGTH+1];

};



#endif


