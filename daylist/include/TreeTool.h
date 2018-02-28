/* TreeTool.h */
/*
update list:
		1、20070605,长途业务要求去除帐单生成路径中pipe_id 这一层目录
		2、20070816，整合需要按source配置合帐关键字，对关键字处理做了修改
		3、20080729，(1)缩小删除临时文件的范围，不能删除别的模块的临时文件	
						(2)扩大关键字个数到25
*/
#ifndef _TREETOOL_H_
#define _TREETOOL_H_ 1


#include <dirent.h>
#include <string.h>

#include "InputInfo.h"
#include "FeeGroupMapList.h"

#include "CF_Common.h"
#include "CF_Cerrcode.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"
#include "psutil.h"

#define		ERR_LIST_ID                  10012

#ifndef ERR_SELECT
#define          ERR_SELECT                   10003  /*查询数据出错*/
#endif

#define		ERR_RENAME                  10013
#define		ERR_UPDATE                  10014

#define          SEPERATETIMELENGTH    5
#define		HOST_NUM_ERR		      -1	//主叫号码位数错误(不是7位或8位)
#define          LIST_KEY_COUNT             25   
#define          LIST_GROUP_COUNT        20   
#define          LIST_KEY_LENGTH            100

struct strct_listFile
{
	char tmp_fileName[500];
	char formal_fileName[500];
};

struct strct_InitPara
{
	char listPath[500];
	char ListConfigID[500];
	char szInputFiletypeId[10];
	char sourceGroup[10];
	char service[10];
	int indexID;
	char endTime[10];
};

typedef map<int,int> keySeqLength;

class CFeeGroup
{
public:
	//vector<long> v_fee;
	long v_fee[50];	
	CFeeGroup();
	~CFeeGroup();
	//CFeeGroup& operator =(const CFeeGroup& feeGroup);
};
 
//内存树中一个叶结点的数据将被保存到此
class CDataTable {
public:	
	char maplist_key_value[LIST_KEY_COUNT][LIST_KEY_LENGTH];//帐单级关键字信息
	CFeeGroup* map_groupSeq_feeGroup[LIST_GROUP_COUNT];//各组费用
	char list_caption[200];
	int		m_iGroupCount;
	int		m_iFieldCount;
	int           m_key_count;
	int		m_attrData_group_index;
	map <int,string> map_fieldID_attr;
	CDataTable(int groupCount, int fieldCount ,MAP_keySeq_fieldValue & map_listKey);
	CDataTable(int groupCount, int fieldCount);
	~CDataTable();
	//CDataTable& operator =(const CDataTable& table);

	void Init(int groupCount, int fieldCount ,MAP_keySeq_fieldValue & map_listKey);
	void Init(int groupCount, int fieldCount );
	void PrintMe();
	
	void InputData( CInputInfo &inputInfo, int groupIndex );
	
	void InputData(  CDataTable *dataTable );

	void InputAttrData(  CInputInfo &inputInfo, int groupIndex ); 
	
};


class CListIO {
public:
	CListIO();
	~CListIO();
	void Init(char *listfile_type_id,char list_record_type_id);
	//写一个号码下的数据到文件
	void writeFormalList( CDataTable &dataTable, CF_MemFileO &_outTmpFile, CFeeGroupMapList &pGlobalMap,CFmt_Change &inrcd,char Write_Type );
	//void writeFormalList( CDataTable &dataTable, int file );
	
	//将一个字符串中数据存到一个TotalInfo中
	void readData( CFmt_Change &inrecord, CDataTable *dataTable ,CFeeGroupMapList &pGlobalMap,int const list_key_count);
	
	void toLine();
	
	void Trim(char *cStr);
private:
	//CFmt_Change list_record;
	map<string,CFmt_Change*> map_fileType_cfmtChange;
	int init_flag;
};



class CTreeTool 
{
public:
    class CFileList {
    public:
        map <string,CDataTable*> map_listKey_dataTable;
        MAP_keySeq_fieldValue map_filekey_value;
		//增加一个source_id，20070812
		string sourceID;
        int curr_size;
        CFileList();
        ~CFileList();
        void Init(const MAP_keySeq_fieldValue &map_file_keySeq_fieldValue);
        void ClearDataTables();
    };
    
private:
	class CList {
    public:
        string ListFlag;	
        char list_file_type_id[10];
        char list_record_type_id;
        map <string,CFileList> map_fileKey_fileList;
        CList();
        ~CList();
        void Clear();
    };
    
public:
	CTreeTool();
	~CTreeTool();
	void Init(strct_InitPara &para);	
	void Input( CInputInfo &inputInfo);
	//void ReloadTable( CFileList &p_fileList, CDataTable *dataTable ) ;		
	//void PrintTree( CMyTree *tRoot, int level, int haveBrother );	
	void Output( );	
	int IsFileExist(const char *p_cPathName);		
	int OutputPart2( CList &pList, const char *date);	
	int OutputPart3( CFileList &p_fileList,  const char* list_id ) ;	
	void Trim( char *cStr );	
	//int ReloadData( CFileList &p_fileList, char *maxNum, int lineCount, char *list_type );      
	int FreeTree(CList& pList);    
	int FreeAllTree();    
	int FreeLocalTree(CFileList &p_fileList);
	int Commit(); 
	int RollBack() ;//删除tmp文件	
	void print_list() ;	
	const char* GetBillType(const string& list_id);
	//20080528
	void get_preday(char* predate);
	//20080528
	int writeOKFile(char* date);
	//20080528
	void setReferOutputDateWhileSleep(char* date);
	int checkLog(char* predate);
	int updateLog(char* state);
	CDataInOutMap	data;	
	map<string,CFeeGroupMapList> GroupInfo;	
	vector <string> filekey;//for test	
private:	
	//map<string,List*> m_abnormalNodePair;
	vector<int> v_list_config_id;
	map<string,CList> m_listID_list;	
	char		cDaylyListPath[300];		//日帐模块的根路径   
	map<string,string> map_listFile;
	int m_TreeMaxSize;
	CF_MemFileI _inListFile;
	CF_MemFileO _outTmpFile;
	int merge_flag;//是否需要与原来生成的帐单合并，1:需要，2:不需要
	int list_key_count;//帐单中关键字个数
	
	//在外买加多一层，以source为关键字
	//map<int,int>m_keySeq_length;
	map<string,int> m_source_keyCount;
	CFmt_Change* p_list_record;
	map<string,CFmt_Change*> map_fileType_cfmtChange;
	CListIO			io;
	int daycycleListFlag;
	int firstOutputFlag;
	//20080528
	char TimeForDaySeperate[SEPERATETIMELENGTH];
	char ReferOutputDate[9];//YYYYMMDD
	char sourceGroupID[10];
	int IndexID;
	char szServiceID[10];
};

#endif
