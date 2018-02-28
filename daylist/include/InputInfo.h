/*
update list:
		1、20070816，整合业务需要按source配置合帐关键字，对关键字处理做了修改
*/

#ifndef _INPUTINFO_H_
#define _INPUTINFO_H_ 1


#include "DataInOutMap.h"
#include "ExpressCtl.h"
#include "CF_Common.h"
#include "CF_Cerrcode.h"
#include "CF_CFmtChange.h"
#include "psutil.h"

#include <string>
#include <map>

#define  ERR_NEW_MEMORY   10001  /*动态分配内存失败*/
#define ERR_MAINKEY_FIELD         10014 //帐单关键字错误
#define ERR_SOURCE_MAINKEY        10017 //source帐单关键字配置不完整

struct INPUT_FIELD_STRU {
	int		i_field_value;
	char        sz_field_value[100];
	INPUT_FIELD_STRU() {
		i_field_value = 0;
		strcpy(sz_field_value,"@@@");
	}
};

struct strct_localnet
{
	int number_len;
	char toll_code[10];
};

//记录一个关键字信息
struct strctMainKey
{
	int field_index;//关键字于话单中序号
	int length;//关键字长度
	int field_begin;
	int field_end;
};

typedef map<int,strctMainKey> MAP_keySeq_fieldIndex;//关键序号_在话单中序号，长度
typedef map<int,string> MAP_keySeq_fieldValue;//关键序号_在话单中序号


//采集来的数据将被保存到此
class CInputInfo {
public:
	INPUT_FIELD_STRU	*m_pList;	//各字段数据
	int		m_iListLen;	//列表长度
	//int 		m_iDataCount;	
	//如果是用来保存采集来的数据,则本变量为该条数据的主叫号码,否则就不用理它
	//char		m_cHostNum[19];	
	char		m_cFeeItem[20];
	//char		m_cLocalNet[20];
	//int          mainkey_field;
	//int          localnet_field;
	char contex[1024];				
	Interpreter theCompiler;
	string s_crr_abnormalListID;
	CExpress_CTL ExpressCTL_withTollCode;
	map<string,strct_localnet> m_lacalnet_numlen;      //记录各个本地网号码长度
	MAP_keySeq_fieldIndex map_file_keySeq_fieldIndex;  //记录文件级关键字信息
	MAP_keySeq_fieldIndex map_list_keySeq_fieldIndex;  //记录帐单级关键字信息
	MAP_keySeq_fieldValue map_file_keySeq_fieldValue;  //记录文件级关键字信息
	MAP_keySeq_fieldValue map_list_keySeq_fieldValue;  //记录帐单级关键字信息
	int callingFieldIndex;//主叫号码在帐单关键字中的顺序
	int locaneFieldIndex;//
	int daycycleFieldIndex;
	int daycycleKeySeq;
	char lastdate[9];
	char crrdate[15];
	int before_endtime_flag;
	
	char CallingNo[20];
	char localNet[10];

	char dataType_endTime[6];

	char sourceID[10];
	
	//CInputInfo(char* express,char* szInputFiletypeId,CFmt_Change *inrecord);
	CInputInfo();
	~CInputInfo();
	void Init(char * file_type_id ,char *endTime, char *source=NULL);
	void getRecordValue(CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, string crr_abnormalListID,int ruleno,int fieldCount,int withTollcode_flag);
	void getRecordValue_undo(CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, string crr_abnormalListID,int ruleno,int fieldCount, int withTollcode_flag);
	void getPureNum();
	int addData( const char *data, int index );
	void PrintMe();
	void PrintMeAll();
	void get_preday_time(char* curtime);
	void setDate();
};

//20070816，供不同source定义不同关键字使用
class CSourceInpuInfo
{
	public:
		map<string,CInputInfo*> map_source_inputInfo;
		int sourceConfigFlag;
		CSourceInpuInfo();
		~CSourceInpuInfo();
		void Init(char * file_type_id ,char *endTime,char *pipe_id);
		
		CInputInfo* getRecordValue(char *source,CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, 
								string crr_abnormalListID,int ruleno,int fieldCount, int withTollcode_flag);
		CInputInfo* getRecordValue_undo(char *source,CFmt_Change &inrcd, CDataInOutMap *pInOutInfo, 
									string crr_abnormalListID,int ruleno,int fieldCount, int withTollcode_flag);

		void setDate(char *source ) ;
		
};

#endif
