/****************************************************************
 filename: CommonPlugin.h
 module:
 created by:
 create date:
 version: 3.0.0
 description:
 基础插件
 *****************************************************************/

#ifndef _COMMONPLUGIN_H_
#define _COMMONPLUGIN_H_  1

#define YEAR  1
#define MONTH 2
#define DAY   3
#define HOUR  4
#define SECOND   6
const int MIN = 5;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "CF_Common.h"
#include "CF_CPlugin.h"
#include "CF_Lack_Abn_Code.h"
#include "CF_PREP_Error.h"
#include "CommonPluginFactory.h"

namespace zhjs
{
class CommonPluginFactory;
}

bool isNo(const char * array);

//用来初始化时间用的类
class my_time
{
public:
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int sec;
public:
	my_time();
	bool init(string t_time);
};

class BaseCommonPlugin: virtual public BasePlugin
{
public:
	virtual void init(char *szSourceGroupID, char *szServiceID, int index);
	virtual void message(MessageParser& pMessage);
	virtual ~BaseCommonPlugin()
	{
	}
};

class CLength: public BaseCommonPlugin
{
public:
	static BasePlugin* getInstance();
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	CLength();
	~CLength();
};

class SubString: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	SubString();
	~SubString();
};

class CCase: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	CCase();
	~CCase();
private:
	int m_iInitFlag;
	int m_iParamNum;
};

class CConnect: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	CConnect();
	~CConnect();
private:
	int m_iParamNum;
};

class Cut: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	Cut();
	~Cut();
private:
	char m_szFieldValue[RECORD_LENGTH + 1];
	char m_szValueAfterCut[RECORD_LENGTH + 1];
	char m_szLocation[RECORD_LENGTH + 1];
	int m_iCutLen;
	char m_szToBeCut[RECORD_LENGTH + 1];
};

class Output: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	Output();
	~Output();
};

class Fill: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	Fill();
	~Fill();
private:
	char m_szPipeId[RECORD_LENGTH + 1];
	int m_iProcessId;
	char m_szDebugFlag[RECORD_LENGTH + 1];
	char m_szFieldValue[RECORD_LENGTH + 1];
	int m_iLength;
	char m_szWhere[RECORD_LENGTH + 1];
	char m_szFilling[RECORD_LENGTH + 1];
};

class Change: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	Change();
	~Change();
private:
	char m_szValue[RECORD_LENGTH];
	char m_szFillValue[RECORD_LENGTH];
	int m_iPos;
};

//class initial: public BaseCommonPlugin
//{
//public:
//	void execute(PacketParser& pps, ResParser& retValue);
//	std::string getPluginName();
//	void printMe();
//	initial();
//	~initial();
//public:
//	char col_num[RECORD_LENGTH];
//};

class Sum: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	Sum();
	~Sum();
public:
	int m_iParamNum;
	long m_lResult;
};

class C_HhMmSsTt2S: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_HhMmSsTt2S();
	~C_HhMmSsTt2S();
private:
	int m_iHBeginPos;
	int m_iHLength;
	int m_iMBeginPos;
	int m_iMLength;
	int m_iSBeginPos;
	int m_iSLength;
	int m_iTBeginPos;
	int m_iTLength;
	int m_iOutLength;
};

//减法
class C_Minus: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_Minus();
	~C_Minus();
private:
	int m_iParamNum;
	long m_lResult;
	long m_minuend;
	long m_subtrahend;
};

//乘法
class C_Multiple: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_Multiple();
	~C_Multiple();
private:
	int m_iParamNum;
	long m_lResult;
};

//除法
class C_Divide: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_Divide();
	~C_Divide();

private:
	double Round(double d, int iPrecision);

private:
	int m_dealMethod;
	double m_dividend;
	double m_divisor;
};

class C_TimeOperator: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_TimeOperator();
	~C_TimeOperator();
private:
	my_time org_time;
	long org_seconds;
	long add_seconds;
	int add_time;
	int add_position;
	int t;
};

class C_IsDigit: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_IsDigit();
	~C_IsDigit();
};

class C_StringReplace: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_StringReplace();
	~C_StringReplace();
private:
	char buff[RECORD_LENGTH + 1];
	string in;
	string search_item;
	string replace_item;
};

class C_TimeCalculate: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	C_TimeCalculate();
	~C_TimeCalculate();

private:
	bool check_time(char* date, char* time, char* result, int type);
private:
	my_time org_time;
	my_time aft_time;
	char date1[RECORD_LENGTH + 1];
	char time1[RECORD_LENGTH + 1];
	char date2[RECORD_LENGTH + 1];
	char time2[RECORD_LENGTH + 1];
	char day[RECORD_LENGTH + 1];
};

class LackInfo: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	LackInfo();
	~LackInfo();
};

class Abnormal: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	Abnormal();
	~Abnormal();
};

class Classify: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	Classify();
	~Classify();
};

// add by jiangjz 20110413
class TimeFormat: public BaseCommonPlugin
{
public:
	void execute(PacketParser& pps, ResParser& retValue);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	TimeFormat();
	~TimeFormat();
};

// add by vivi 20120918
class C_IsXDigit: public BaseCommonPlugin
{
public:
        void execute(PacketParser& pps, ResParser& retValue);
        std::string getPluginName();
        std::string getPluginVersion();
        void printMe();
        C_IsXDigit();
        ~C_IsXDigit();
};

// add by viv 20120919
class C_MinusTime: public BaseCommonPlugin
{
public:
        void execute(PacketParser& pps, ResParser& retValue);
        std::string getPluginName();
        std::string getPluginVersion();
        void printMe();
        C_MinusTime();
        ~C_MinusTime();
};

#endif
