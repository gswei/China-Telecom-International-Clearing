/*
update list:
		1��20070816������ҵ����Ҫ��source���ú��ʹؼ��֣��Թؼ��ִ��������޸�
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

#define  ERR_NEW_MEMORY   10001  /*��̬�����ڴ�ʧ��*/
#define ERR_MAINKEY_FIELD         10014 //�ʵ��ؼ��ִ���
#define ERR_SOURCE_MAINKEY        10017 //source�ʵ��ؼ������ò�����

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

//��¼һ���ؼ�����Ϣ
struct strctMainKey
{
	int field_index;//�ؼ����ڻ��������
	int length;//�ؼ��ֳ���
	int field_begin;
	int field_end;
};

typedef map<int,strctMainKey> MAP_keySeq_fieldIndex;//�ؼ����_�ڻ�������ţ�����
typedef map<int,string> MAP_keySeq_fieldValue;//�ؼ����_�ڻ��������


//�ɼ��������ݽ������浽��
class CInputInfo {
public:
	INPUT_FIELD_STRU	*m_pList;	//���ֶ�����
	int		m_iListLen;	//�б���
	//int 		m_iDataCount;	
	//�������������ɼ���������,�򱾱���Ϊ�������ݵ����к���,����Ͳ�������
	//char		m_cHostNum[19];	
	char		m_cFeeItem[20];
	//char		m_cLocalNet[20];
	//int          mainkey_field;
	//int          localnet_field;
	char contex[1024];				
	Interpreter theCompiler;
	string s_crr_abnormalListID;
	CExpress_CTL ExpressCTL_withTollCode;
	map<string,strct_localnet> m_lacalnet_numlen;      //��¼�������������볤��
	MAP_keySeq_fieldIndex map_file_keySeq_fieldIndex;  //��¼�ļ����ؼ�����Ϣ
	MAP_keySeq_fieldIndex map_list_keySeq_fieldIndex;  //��¼�ʵ����ؼ�����Ϣ
	MAP_keySeq_fieldValue map_file_keySeq_fieldValue;  //��¼�ļ����ؼ�����Ϣ
	MAP_keySeq_fieldValue map_list_keySeq_fieldValue;  //��¼�ʵ����ؼ�����Ϣ
	int callingFieldIndex;//���к������ʵ��ؼ����е�˳��
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

//20070816������ͬsource���岻ͬ�ؼ���ʹ��
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
