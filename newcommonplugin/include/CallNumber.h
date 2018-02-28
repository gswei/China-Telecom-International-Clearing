 /****************************************************************
 filename: CallNumber.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
	将号码相关属性封装
 update:

 *****************************************************************/

#ifndef _CALLNUMBER_H_
#define _CALLNUMBER_H_	1

#include <map>
#include "CF_CPlugin.h"
#include "ComFunction.h"

//将表索引封装
/*
struct  Table_Index
{
	int m_indexDelTelHead;
	int m_indexBefTolSer;
	int m_indexGsm2Tollcode;
	int m_indexTelenoPro;
	int m_indexTelenoSer;
	int m_indexTollCode;
	int m_indexLocalnet;
	int m_indexFreeTel;
	//add by weixy 20080121 
	int m_indexTelenoLen;
	//add by weixy 20080403
	int m_indexTelDefPro;
	//add by ouyh 20090626
	int m_indexFreeLack;
};
*/

//将默认号码属性封装
struct  Def_Attribute
{
	char m_DefServBefore[RECORD_LENGTH+1];
	char m_DefServAfter[RECORD_LENGTH+1];
	char m_DefBusiness[RECORD_LENGTH+1];
	char m_DefMobile[RECORD_LENGTH+1];
};

class C_CallNumber
{
private:
	STelenoAttr  m_CurTelAttr;
	SInputParam m_InputParam;
	
	DataStruct  m_DataIn;
	DataStruct  m_DataOut;

	//map 
	map<string,Def_Attribute> map_DefAttribute;
	//map<string,Table_Index>  map_ServerIndex;
//	Table_Index   ms_Table;
	Def_Attribute  ms_Attribute;
	map<string,Def_Attribute>::iterator  mapIt_DefAttribute;
//	map<string,Table_Index>::iterator  mapIt_ServerIndex;

	char m_szIniPath[PATH_NAME_LEN+1];
	//char ServerName[RECORD_LENGTH+1];
	char m_szServCatId[RECORD_LENGTH+1];
	/*
	int m_indexDelTelHead;
	int m_indexBefTolSer;
	int m_indexGsm2Tollcode;
	int m_indexTelenoPro;
	int m_indexTelenoSer;
	int m_indexTollCode;
	int m_indexLocalnet;
	int m_indexFreeTel;
	//add by weixy 20080121
	int m_indexTelenoLen;
	//add by weixy 20080403
	int m_indexTelDefPro;
	//add by ouyh 20090626
	int m_indexFreeLack;
	*/
	char table_DelTelHead[TABLENAME_LEN+1];
	char table_BefTolSer[TABLENAME_LEN+1];
	char table_Gsm2Tollcode[TABLENAME_LEN+1];
	char table_TelenoPro[TABLENAME_LEN+1];
	char table_TelenoSer[TABLENAME_LEN+1];
	char table_TollCode[TABLENAME_LEN+1];
	char table_Localnet[TABLENAME_LEN+1];
	char table_FreeTel[TABLENAME_LEN+1];
	char table_TelenoLen[TABLENAME_LEN+1];
	char table_TelDefPro[TABLENAME_LEN+1];
	char table_FreeLack[TABLENAME_LEN+1];
	
	int m_iTableDelTelHeadOffset;
	int m_iTableBefTolSerOffset;
	int m_iTableGsm2TollcodeOffset;
	int m_iTableTelenoProOffset;
	int m_iTableTelenoSerOffset;
	int m_iTableTollCodeOffset;
	int m_iTableLocalnetOffset;
	int m_iTableFreeTelOffset;
	int m_iTableTelenoLenOffset;
	int m_iTableTelDefProOffset;
	int m_iTableFreeLackOffset;

	BaseAccessMem *table;

	C_JudgeGsm Gsm2Toll;
		
	char m_szLastSourceId[RECORD_LENGTH+1];
	//int m_iOmitTollcode;
	int m_iGsmLength;
	int m_iIndex;

	char m_DefServBefore[RECORD_LENGTH+1];
	char m_DefServAfter[RECORD_LENGTH+1];
	char m_DefBusiness[RECORD_LENGTH+1];
	char m_DefMobile[RECORD_LENGTH+1];

	int loadDefTelenoAttr();
	
public:

	C_CallNumber();
	~C_CallNumber();
	//int Init(char *m_szPipeId,int m_iProcessId,char * m_szSourceId,char * m_szServCatId);
	//int Init(char *szService, char *szSourceID, char *szServCatID, char *szIniPath);
	//int Init(char *szService, char *szSourceID, BaseAccessMem *pAccessMem);
//	int UpdateTable();
	int Init(BaseAccessMem *pAccessMem);
	int Reset(char *szService, char *szSourceID);
	//get value interface
	int Set_Value();
	int Set_ServCatId(char * m_szServCatId);
	int Set_SourceId(char * m_szSourceId);
	int Set_CallNbr(char * m_szCallNbr);
	int Set_NbrType(char *m_szNbrType);
	int Set_CdrBegin(char * m_szCdrBegin);
	int Set_DefTollcode(char * m_szDefTollcode);	
	int Set_BefTollCode(char * m_szBefTollcode);
	int Set_ChargeFlag(char * m_szChargeFlag);
	//add by weixy 20080219
	int Set_Tollcode(char * m_szTollcode);
	//end add by weixy 20080219
	
	//return value interace
	int Get_BefTollTel(char * m_szBefTollTel);
	int Get_Tollcode(char * m_szTollcode);
	int Get_AfterTollTel(char * m_szAfterTollTel);
	int Get_Teleno(char * m_szTeleno);
	int Get_District(char *m_szDistrict);
	int Get_BusiAfter(char *m_szBusiAfter);
	int Get_BusiBefore(char *m_szBusiBefore);
	int Get_BusiPriority(char *m_szBusiPriority);
	int Get_Mobile(char *m_szMobile);
	int Get_SvrBefore(char *m_szSvrBefore);
	int Get_SvrAfter(char *m_szSvrAfter);
	int Get_ChargeFlag(char *m_szChargeFlag);


	//query value interface
	int queryTollcode(char * Lack_Code);
	int queryGsm(char * Lack_Code);
	int queryTeleProperty(char * Lack_Code);
	int queryLocalnet(char * Lack_Code);
	int queryTeleServ(char * Lack_Code);
	int queryFreeTele();
	int queryTelenoLen(int &);
	int queryTelDefProperty(char * Lack_Code);
	
	int queryRubTelHead(char * m_szServCatId,char * m_szSourceId,char *m_szTelenoFlag,char*m_szTeleno,char*m_szCdrBegin,int Len,int &OLen,int &OPos);

	//modify value interface
	int DealRubTelHead();
	int DealBefTelServ();
	int DealTollcode();
	int DealGsm();
	int AddZero();
	int AddDefTollcode();
	int setDefTollcode();
	int ConnectTollGsm();
	
	//judge th value interface
	int IsGsm();
	int IsZeroHead();
	int IsEmpty();//add by weixy 20080429
	//int IsTollcodeOmit();
	int IsDefTollProvince(char * Lack_Code);
	int IsTeleLengthRight();
	int IsDefTollHead();
	bool IsForeign();
	
	int CheckValid(int CheckLen,char * Lack_Code);
	int CheckGsm(char * Lack_Code);
	int CheckGsmLen(char * Lack_Code);
	int CheckLen(char * Lack_Code);
	int CheckGDLen(char * Lack_Code);
	int Check2Tollcode(char * Lack_Code);
	int CheckBefTel(char * Lack_Code);
	int Check10Len(char * Lack_Code);

};

#endif


