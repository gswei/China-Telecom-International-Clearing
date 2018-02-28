/****************************************************************
 filename: AnaCallDirect.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
	1 插件名称 SWLHAnaCalledDirect:通话方向分析插件
	2 接口功能
	  根据被叫区号（取自TOLLCODE表）查询被叫归属地-被叫方向对应关系表(SWLH_REGION_DIRECT)获取被叫方向
	  输入参数：流水标识，Serv_cat_id 区号
	  输出：被叫方向
	3 匹配规则
	  SWLH_REGION_DIRECT表记录被叫归属地-被叫方向对应关系表，其中被叫归属地取值包括：
	  a、广州省所有本地网区号（精确匹配）
	  b、一个0开头的区号（国内区号若不在a范围内，需要匹配到此记录）
	  c、部分国家区号（模糊匹配）
	  d、00*（c部分未包含的国外区号才需要匹配到此记录）
	  e、*（不在以上情况中的才匹配到此）
 update:

 *****************************************************************/

#ifndef SWLH_ANA_CALLED_DIRECT_H
#define SWLH_ANA_CALLED_DIRECT_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_SWLHAnaCalledDirect : public BasePlugin
{
public:
	C_SWLHAnaCalledDirect();
	~C_SWLHAnaCalledDirect();
	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

protected:
	
private:
	BaseAccessMem *table;
	char m_szTableName[TABLENAME_LEN+1];
	int m_iTableOffset;
	int m_iIndex;
	DataStruct in;
	DataStruct out;
	
	char tollcode[RECORD_LENGTH+1];
};



#endif

