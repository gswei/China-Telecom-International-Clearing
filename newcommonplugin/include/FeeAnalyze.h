/****************************************************************
 filename: C_FeeAnalyze.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
	1.	以#号为分隔符，区分开所有的账本
	2.	对账本类型不为1001、1002、1003的账本不作处理
	3.	对账本余额大于等于零的账本，变动量即为实收扣费
	4.	对账本余额小于0的账本，若realDelta=Balance-Delta>0，则实收扣费为realDelta；
                               若realDelta<0则实收扣费为0
	5.	对于所有账本类型为1001的账本的实收扣费的和，输出为本金实际扣费
	6.	对于所有账本类型为1002、1003的账本实收扣费的和，输出为赠金实际扣费

 update:
20080318      增加一个判断字段，如果是0的话，不能出现>0的delta
                                如果是1的话，不能出现<0的delta
20080326      修改由于输入的话单以#结尾导致的问题
              增加判断每一段信息的长度是否正确
 *****************************************************************/
#ifndef C_FEE_ANALYZE_H
#define C_FEE_ANALYZE_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_FeeAnalyze : public BasePlugin
{
public:
	C_FeeAnalyze(){}
	~C_FeeAnalyze(){}
	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

protected:
	int char2vector( string in , vector<string> &out, string sep);

private:
	char buff[RECORD_LENGTH+1];
	
	int fee_org;
	int fee_add;
	int realDelta;
	
	vector<string> input_group;
	
	int mode;
	char lackinfo[RECORD_LENGTH+1];
};



#endif

