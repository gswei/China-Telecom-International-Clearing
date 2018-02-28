/****************************************************************
 filename: IsBalanceValid.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	判断余额有效日期是否在余额文件时间之后，
	如果是的话，判为未过期0；
	如果过期，但有效期在三个月内，则判为1；
	如果过期三个月以上则判为2
 update:
20080319   增加是否本月过期：0表示余额有效期跟余额文件时间不在同一个月内；
                             1表示余额有效期跟余额文件时间在同一个月内
 *****************************************************************/
#ifndef C_BALANCE_ANALYZE_H
#define C_BALANCE_ANALYZE_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_IsBalanceValid : public BasePlugin
{
public:
	C_IsBalanceValid(){}
	~C_IsBalanceValid(){}
	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
		
private:
	char input_time[RECORD_LENGTH+1];
	char valid_time[RECORD_LENGTH+1];
	
	int year_month_i, year_month_v;
	int day_i, day_v;
	string buff;
};



#endif

