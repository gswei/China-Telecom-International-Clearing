/****************************************************************
 filename: TfAnaCode.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-09-18
 version: 3.0.0
 description: 
	声讯号码分析
 update:

 *****************************************************************/

#ifndef _TFANACODE_H_
#define _TFANACODE_H_ 1

#include "CF_CPlugin.h"
#include "CF_CException.h"
#include "CF_Common.h"
#include "ComFunction.h"


//TfAnaCode Class
class C_TfAnaCode: public BasePlugin
{
private:
	/*
	char m_szDebugFlag[RECORD_LENGTH+1];
	char m_szLastSource[RECORD_LENGTH+1];

	char m_cOmitTollcode;
 	int  m_iLack;
 	*/

	/*
	int m_iMaxCallingDelLen;
	int m_iMinCallingDelLen;
	int m_iMaxCalledDelLen;
	int m_iMinCalledDelLen;
	*/

	STelenoAttr  m_CurTelenoAttr;
	SInputParam  m_InputParam;
	C_JudgeGsm Gsm2Toll;
	BaseAccessMem *table;
	DataStruct m_InData;
	DataStruct m_OutData;
	int m_iIndex;
	int m_iTollcodeOffset;
	int m_iGsm2TollcodeOffset;
	//int m_iTollcodeTelenoOffset;
	int m_iTollcodeRegionFlagOffset;
	int m_iTelenoProperty;
	int m_iLocalnetOffset;
	int m_iTollDiscountPptOffset;
	int m_iHeaderDeleteOffset;
	int m_iTelenoServiceOffset;
	int m_iTelenoPropertyOffset;
	int m_iTelenoDefPropertyOffset;

public:
	C_TfAnaCode();
	~C_TfAnaCode();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

	/*void newAllStruct();
	void deleteAllStruct();
	int setAllTabItemNum();
	int setAllTabShmIndex();
	int setAllTabSemIndex();
	int setAllTabShmAddr();
	*/
	int anaTeleno();
	int deleteTelHeader();
	int anaSvrBefore();
	int anaTollcode();
	int anaCdmaTeleno();
	int anaRegionFlag();
	int anaTollcodeTeleno();
	int anaTelenoProperty();
	int outputTelenoAttr(STelenoAttr &TelenoAttibute, ResParser& retValue);
};

#endif

