 /****************************************************************
 filename: 200SPAnaCalledNo.h
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-09-20
 version: 3.0.0
 description: 
	��ѶSP����
 update:
  2007-12-18	�޸ķ������ŵ�ʱ��������ڷּ�ı�־NGD���������ֶ�
  				ͬʱ���ӻ�дԭ���ı��к���
  2007-12-19	�޸Ļ�д���к������õĺ���
  				�޸�NGD�ķּ𷽷���һ���г�����NGD��ֱ�ӷ���
 *****************************************************************/

#ifndef C_200SPANACALLEDNO_H
#define C_200SPANACALLEDNO_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

/*
const int IVR_LACK_CARD_LOCATION = 501;
const int IVR_LACK_CALLEDNO = 502;
const int IVR_LACK_TOLLCODE = 506;
const int IVR_LACK_TOLLCODE_LOCALNET = 503;
const int IVR_LACK_CALLEDNO_TELENO_PROPERTY = 504;
const int IVR_LACK_CALLEDNO_TOLLCODE = 505;
*/


class C_200SPAnaCalledNo: public BasePlugin
{
public:
	/*
	C_200SPAnaCalledNo();
	~C_200SPAnaCalledNo();

	int execute(BufferParser& bps,ResParser& retValue){	return 0;}
	int execute(PacketParser& pps,ResParser& retValue);
	char* execute( PacketParser& pps ){ return "true"; };

	int init();
	bool commem_init();
	*/

	C_200SPAnaCalledNo();
	~C_200SPAnaCalledNo();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
public:
	char CalledNo[RECORD_LENGTH+1];
	char CalledNo_Ana[RECORD_LENGTH+1];
	char Call_Location[RECORD_LENGTH+1];
	char Card_Location[RECORD_LENGTH+1];
	char CdrBegin[RECORD_LENGTH+1];
	char ServCatId[RECORD_LENGTH+1];

	char Tollcode[RECORD_LENGTH+1];

	int m_iTollcodeOffset;
	int m_iLocalnetOffset;
	int m_iTelenoServiceOffset;

private:	
	char abn_value[RECORD_LENGTH+1];
	char abn_content[RECORD_LENGTH+1];

	DataStruct m_InData;
	DataStruct m_OutData;
	BaseAccessMem *table;

	int lackinfo;
	char lackreason[RECORD_LENGTH+1];

	/*
	char m_szLastSource[RECORD_LENGTH];
	int m_iMaxCallingDelLen;
	int m_iMinCallingDelLen;
	int m_iMaxCalledDelLen;
	int m_iMinCalledDelLen;
	bool setAllTabItemNum();
	bool setAllTabShmIndex();
	bool setAllTabSemIndex();
	bool setAllTabShmAddr();
	void newAllStruct();
	void deleteAllStruct();
	*/


protected:
	bool anaTollcode(PacketParser& pps, ResParser& retValue);
	bool anaFeeType(PacketParser& pps, ResParser& retValue);
	bool anaLocalnet(PacketParser& pps, ResParser& retValue);
	bool andService_id(PacketParser& pps, ResParser& retValue);

	bool checkFormat(const char *cmpString,const char *format);
	//bool shm_table_init();
};

#endif

