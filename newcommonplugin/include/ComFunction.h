/****************************************************************
 filename: ComFunction.h
 module:
 created by:
 create date:
 version: 3.0.0
 description:

 update:

 *****************************************************************/
#ifndef _COM_FUNCTION_H_
#define _COM_FUNCTION_H_ 1

#include "CF_Config.h"
#include "CF_Common.h"
#include "CF_CPlugin.h"
//#include "CF_CError.h"
#include "CF_PREP_Error.h"
#include "CF_Lack_Abn_Code.h"

#include "psutil.h"
using namespace std;
using namespace tpss;

const int MAX_COL_LEN=30;
const int MSG_LEN 	= 1000;
const int PATH_LEN 	= 256;
const int FIELD_LEN = 256;
extern char ErrorMsg[MSG_LEN];

const char EMPTY_STR[]					 = "";
const char DEF_BUSI_PRI[]				 = "1";
const char CALLING_VALID[]			 = "01";
const char CALLED_VALID[]				 = "10";
const int NOT_CHARGE 		= 100;


#define BEF_TOLL_LEVEL 		"1"
#define TOLL_LEVEL				"2"
#define AFTER_TOLL_LEVEL	"3"

const int MINTELEN   =3;

const char REDO_FLAG[] =  	 "96";
#define IGNOREFLAG           '1'
#define CALLING_FLAG         'C'
#define CALLED_FLAG          'D'
#define INROUTE_FLAG         'I'
#define OUTROUTE_FLAG        'O'
#define SETTLE_FLAG	     		 'Y'

/* the struct of params set to class C_TyAnaCode */
struct SInputParam
{
	char m_szTeleno[FIELD_LEN+1];
	char m_szTelenoFlag[FIELD_LEN+1];
	char m_szCdrBegin[FIELD_LEN+1];
	char m_szIgnoreFlag[FIELD_LEN+1];
	char m_szRedoFlag[FIELD_LEN+1];
	char m_szLocalToll[FIELD_LEN+1];
	char m_szLocalnet_Abbr[FIELD_LEN+1];
	char m_szGroupId[FIELD_LEN+1];
	char m_szSourceId[FIELD_LEN+1];
	char m_szServCatId[FIELD_LEN+1];
	char m_szSrcGroupId[FIELD_LEN+1];	
	char m_szChargeFlag[FIELD_LEN+1];
	char m_szOtherRegion[FIELD_LEN+1];	
	//char m_szPipeId[FIELD_LEN+1];
	//int m_iProcessId;	
	char m_szSourceGroupId[FIELD_LEN+1];
	char m_szServiceId[FIELD_LEN+1];
	//char m_szSemaPath[PATH_LEN];
	//char m_szShmPath[PATH_LEN];	
	char m_szRateType[FIELD_LEN+1];
};

/* the struct of property of teleno input */
struct STelenoAttr
{
	char m_szTeleno[FIELD_LEN+1];
	char m_szBefTollTel[FIELD_LEN+1];
	char m_szTollTel[FIELD_LEN+1];
	char m_szAfterTollTel[FIELD_LEN+1];	
	char m_szTelenoFlag[FIELD_LEN+1];
	char m_szChargeFlag[FIELD_LEN+1];
	char m_szSvrBefore[FIELD_LEN+1];
	char m_szTollcode[FIELD_LEN+1];
	char m_szLocalNet[FIELD_LEN+1];
	char m_szLocalNet_Ana[FIELD_LEN+1];
	char m_szGroupA[FIELD_LEN+1];
	char m_szGroupB[FIELD_LEN+1];
	char m_szRegionFlag[FIELD_LEN+1];
	char m_szDistrict[FIELD_LEN+1];
	char m_szBusiAfter[FIELD_LEN+1];
	char m_szBusiBefore[FIELD_LEN+1];
	char m_szBusiPriority[FIELD_LEN+1];
	char m_szMobile[FIELD_LEN+1];
	char m_szSvrAfter[FIELD_LEN+1];	
	char m_szCallingNoType[FIELD_LEN+1];	
	char m_szCodeLength[FIELD_LEN+1];
	char m_szIsGDProvince[FIELD_LEN+1];
	//add by weixy 20080429
	char m_szBTEmpty[FIELD_LEN+1];
	char m_szTelPptHeader[FIELD_LEN+1];
	char m_szSpCode[FIELD_LEN+1];
	
};

struct S_HcodeHeader
{
	char m_szHcodeHeader[FIELD_LEN+1];
};

class C_JudgeGsm
{
private:
	S_HcodeHeader* m_pGsmTab;
	int m_iGsmHeaderLen;
	int m_iItemCount;
public:
	C_JudgeGsm();
	C_JudgeGsm(const  C_JudgeGsm&  T );
	int Init(const char* pchGsmTableName);
	bool isGsm(const char* pchCallNo);
	~C_JudgeGsm();
};

int getVersion( const char* tablename, const char* source_group_id, const char* szServId, const char* servcat_id );

void DeleteSpace(char *ss);

int getFromGlobalEnv( char *Value, char *Name );

int getFromSrcEnv( char *Value, char *Name, char *SourceId, char *Service );

int getFromGroupEnv( char *Value, char *Name, char *SourceGroupId, char *Service );

#endif
