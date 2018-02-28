/****************************************************************
filename: CF_CStat.h
module: CF - Common Function
created by: zhoulh
create date: 2005-04-01
update list: 
version: 1.0.0
description:
    the header file of the classes for CF_CStat
*****************************************************************/ 
// 更改历史
//20070509，setFileName根据文件名查询shedule，在数据量大时速度会非常慢，修改该接口，
//           sourceid从外部传入，或sourceid、dealstarttime都从外部传入
//20070516，STAT_END_TIME变量如果不配可去默认值，继续处理而不是抛出异常
//20070530，数据业务新增表要求对小数类型数据做统计，增加此功能
//20070619，增加类成员m_SzFileName数组长途，由100改为251
//20080516，程序在处理大文件时会很慢，为此作如下改进
//			 (1)改进Connect()函数算法，只扫描一次Stat_Record_toUpdate[]数组
//                   (2)为减少内存使用量，不用新的数组存放要insert或 update的记录，
//				而是直接在Stat_Record_toUpdate上修改记录，用vector存放到记录的指针
//			  (3)为提高大文件的处理速度，增加两个参数，
//				STAT_MAXNUM_UPATE:多少条记录更新一次表
//                        STAT_MAXNUM_CACULATE:多少条话单做一次累计
//			(4) 减少Stat_Record_toUpdate[]内存申请、释放次数，只在不够时候重新申请
//20080604，(1)算法改进，使用MAP取代原来的数组排序相同条件累加的统计方法
//			 (2)列表统计一直没有用到，去掉此功能
//20080714，增加功能，支持以process_id作为统计维度统计

#ifndef _CF_CSTAT_H_
#define _CF_CSTAT_H_ 1

#include <time.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#include "CF_Common.h"
#include "CF_CFmtChange.h"
#include "CF_CException.h"
//#include "CF_COracleDB.h"
#include "CF_CMemFileIO.h"
#include "CF_CInterpreter.h"
#include "CF_CLogger.h"
#include "psutil.h"

#define ConfigIDSPILT             ';'
#define ALLNULLPASS              'A'
#define PARTNULLPASS             'P'
#define NONULLPASS              'N'

//STAT_MAXNUM_CACULATE的默认值，统计话单结构数组内数据的最大个数
#define	STAT_RECORD_COUNT_MAX          3000  
//20080516，用到的地方用STAT_RECORD_COUNT_MAX代替
//达到该话单数进行update_stat 数据库操作
//#define	STAT_RECORD_COUNT              3000  
#define  STAT_INSERT_CONNT             500 //更新库表时一次插入的记录书
#define	STAT_ITEM_COUNT                100    //表项个数
#define	STAT_ITEM_NAME_LEN             50    //表项名长度
#define	STAT_ITEM_CONTAINER_LEN        251    //表项内容长度
#define  STAT_GROUP_COUNT               10    //统计标识组个数
#define  MAX_STATID_NUM                 50    //一条话单允许的最大统标识个数
#define  SQL_LENGTH               2000


#define	STAT_SZ_TYPE                   2     //表项内容为字符串型的类型标志
#define	STAT_N_TYPE                    1     //表项内容为整型的类型标志
#define	STAT_N_COUNT_TYPE              0     //表项内容为整型且为累计项的类型标志
//20070530，对小数类型数据做统计
#define	STAT_D_COUNT_TYPE              3     //表项内容为符点数类型 且为累计项的类型标志

#define	KEYWORK_MIN                    11    //关键字从11 开始
#define	KEYWORK_SOURCEID               11    //关键字source_id定义为11
#define	KEYWORK_STATTIME               12    //关键字stat_time定义为12
#define	KEYWORK_FILENAME               13    //关键字filename定义为13
#define  KEYWORK_UPDATETIME            14    //用于中间表，更新时间
#define  KEYWORK_CDRDURATION           15    //用于按通话时长统计
#define	KEYWORK_ERRORTYPE              16    //关键字errortype
#define	KEYWORK_PROCESSID              17   //PROCESS_ID
//从此以后关键字需从20 开始
#define  KEYWORK_THOUSANDCALLNO        21    //主叫局号，长途计费业务中使用
#define  KEYWORK_DAYCYCLE                22  //日帐期，20070314
/*
//这部分错单统计已用
#define	STAT_DAYE_TYPE              5    //表项内容为date类型且时间串是输入的，如:to_date(:%d,'YYYYMMDDHH24MISS')
#define	STAT_SERI_TYPE              4     //表项内容为计数器且计数器名为S_(字段名).NEXTVAL

#define	KEYWORK_INVALIDFLAG             18  //关键字INVALIDFLAG定义为18
#define	KEYWORK_REDOFLAG                19  //关键字REDOFLAG定义为19
#define	KEYWORK_ERRORTYPE2               31  //关键字errortype2定义为31
#define	KEYWORK_ORARCD               32  //关键字原始话单定义为32

*/

//递增申请内存个数
#define  MEMORY_APPLY_UNIT              500  
#define  STAT_MAXNUM_UPATE              1000  


#define  ERR_CONFIGID				8001      //stat_config_id错误
#define	ERR_REQU_MEM				8002     //申请内存出错
#define	ERR_SELECT  				8003     //数据库里找不到记录
#define  ERR_FILED_VAL                         8004     //字段内容不符合定义
#define  ERR_TABLEITEM_OVER              8005     //统计表字段超出最大范围
#define  ERR_FIELD_INDEX                 	8006     //字段序号错误
#define  ERR_FIELD_NULL                   	8007     //话单字段内容为空
#define  ERR_GET_SUBSTR                  	8008     //取子字符串错误
#define  ERR_KEY_ITEM                      	8009     //非法关键字
#define  ERR_EXPRESS                        	8010     //条件表达式错误
#define  ERR_INSERT_TABLE                  8011     //插入、更新记录错误
#define   ERR_GET_FIELD_VALUE           8012     //获取话单字段内容错误
#define	ERR_BILLCOUNT_NEGATIVE     8013    //bill_count 为负数
#define	ERR_SELECT_UNDO                  8014    //冲销统计没有匹配记录
#define	ERR_RENAME                            8015    //更名错误


typedef vector<int> v_recoredSeq;

//条件表达式变量对应字段值
struct FIELD_VALUE
{
	char szField_Value[250];
	FIELD_VALUE()
	  	{
	  	szField_Value[0]=0;
		}
};

//条件表达式变量信息
struct COND_PARA
{
	char szParaName[250];
	//用于区分变量名是来自话单字段名还是统计表字段名
	// >0表示话单字段名，-1表示表字段名
	int index;
	COND_PARA()
	  	{
	  	szParaName[0]=0;
		index=-1;
		}
};

struct STAT_TABLE//统计表
  {
  char SzTable_Name[STAT_ITEM_CONTAINER_LEN];//表名
  char ChStat_Type;//表类型
  char ChIs_Count;//是否需要统计话单数
  char SzCount_Item_Name[STAT_ITEM_NAME_LEN];//话单数统计项名
  char CHStat_Mode;//统计模式，U为update模式，I为insert模式,T为临时表模式
  
  char SzStat_Item[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];//统计项目数组
  int  NField_Index[STAT_ITEM_COUNT];
  char SzStat_FieldName[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];

  char ChCondiction[250];//条件表达式  
  COND_PARA *cond_field;//条件表达式变量
  FIELD_VALUE *cond_field_value;//条件变量值
  int cond_field_count;//条件表达式变量个数
  
  int  NStat_Item_Count;
  int  NItem_Type[STAT_ITEM_COUNT];
  int  NStat_Item_Begin[STAT_ITEM_COUNT];
  int  NStat_Item_End[STAT_ITEM_COUNT];
  int  NStat_Item_IndexInField[STAT_ITEM_COUNT];
  char chStat_Item_SprInField[STAT_ITEM_COUNT];
  char Predfval_Or_Statid[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];  
  //表统计项对应条件表达式中第几个变量
  int   item_cond_index[STAT_ITEM_COUNT];
  //累计项单位转换，空表示不需要转换
  //"/60"表示该项为时间，需要除以60后向上取整
  char ChCount_Item_Unit[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];
  
    //20080220,动态生成表
    int table_name_key_count;
    vector<string> v_tableName;//已经的结果表名
    char NField_dataType[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
    char IsIndex[STAT_ITEM_COUNT]; 
};


struct STAT_RECORD//统计记录
{
  char SzStat_Item[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];
  int  NItem_Type[STAT_ITEM_COUNT];
  int  NStat_Item_Count;
  int  NBill_Count;  
};

struct STAT_ITEM_GROUP_INFOR//竖表统计项信息
{
int iStat_Item_Group_Num;//统计项组个数
int ID_Group_Filed_Index[STAT_GROUP_COUNT];//统计标识组所在位置
int Value_Group_Field_Index[STAT_GROUP_COUNT];//统计值组话单所在位置
};

int g_Cmp(const void *s1,const void *s2);

typedef vector<STAT_RECORD> v_record;
typedef vector<STAT_RECORD*> v_pRcd;

class CRecordKey
{
public:
	CRecordKey(){NStat_Item_Count=0;};
	~CRecordKey(){};
	 char SzStat_Item[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];
       int  NStat_Item_Count;
	bool operator == (const CRecordKey& key) const
	{
		for(int i=0;i<NStat_Item_Count;i++)
			{
			if(strcmp(key.SzStat_Item[i],SzStat_Item[i])!=0)
				return false;
			}
		return true;
	}
	bool operator < (const CRecordKey& key) const
	{
		for(int i=0;i<NStat_Item_Count;i++)
			{
			if(strcmp(SzStat_Item[i],key.SzStat_Item[i])<0)
				return true;
			else if(strcmp(SzStat_Item[i],key.SzStat_Item[i])>0)
				return false;
			}
		return false;
		
	}
} ;

class CF_CStat
{
  public:
    CF_CStat();
    ~CF_CStat();
    int Init(int para_config_id,int maxRcdNumToUpdat);
    //int setFileName(char *);
    //20070509,修改setFileName接口
    //int setFileName(char *FileName, char *sourceid);
    int setFileName(char *FileName, char *sourceid,char *dealstarttime);
    
    int dealRedoRec(CFmt_Change &inrcd, char *szError_Type);
    void update_commit();
    void rollback();    
    void Set_TempOutFile();//设置结果输出临时文件名
    void Close_Temp_Outfile();
    void Unlink_TemFile();//删除临时文件
    void Unlink_File();
    int   Rename_AllOutFile();//一批文件处理完后，将文件更名为正式文件
  private:
    int DeleteSpace( char *ss );
    int Make_Select_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp);
    int Make_Delete_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp);
    int Make_Insert_Sql(char *sqltmp,const char *table_name);
    int Make_Update_Sql(char *sqltmp,const char *table_name);
    int Make_Merge_Sql(char *sqltmp,const char *table_name);
    int Set_KeyWord(STAT_RECORD &Cur,int i,char *Key_Container);
    int Set_KeyStatItem(STAT_RECORD &,int );  
    int Insert_Stat();
    int Prepare_Rec();
    int Prepare_Rec_Update();
    int Prepare_Rec_TmpStat();		
    int Prepare_Rec_Merge();
    int Insert_Table();
    int Update_Table();
    int Merge_Table();
//20080604
//	int Stat_Record_Col(CFmt_Change &, int IError_Type=0);//竖表
//    int Update_Stat_File();    
//    int Update_Stat_File_Map();
//    int Connect(int Add_Count);
    int Stat_Record_Insert(CFmt_Change &InRecord, char *szError_Type);
    int Stat_Record_Update(CFmt_Change &InRecord ,  char *szError_Type);
    int Stat_Record_Merge(CFmt_Change &InRecord ,  char *szError_Type);
    int AddOneRecord(STAT_RECORD& addRcd);
    void Rqsort(void  *base,  size_t  nel,  size_t  width);
    int Count_Item(char *string,char seperator);
    void Get_Item(char *string,char seperator,int *item_group,int item_count);
//20050621
    int Exch_Field(CFmt_Change &inrcd);
    int Read_Cond_field();//求条件表达式字段个数
    void get_preday_time(char* curtime);
    void Create_Table(char *table_name);
    void print(STAT_RECORD * rcd,int cout);
    void printOneRecord(STAT_RECORD * rcd);
    void printOneRecord(STAT_RECORD& rcd);
	
    int         m_NRecord_Count;
    STAT_TABLE  m_SStat_Table;
    STAT_RECORD *m_PsPre_Stat_Record;
    STAT_RECORD *m_PsRes_Stat_Record;
    STAT_RECORD *m_PsCur_Stat_Record;

    char crr_table_name[STAT_ITEM_CONTAINER_LEN] ;//当前记录对应表名
 //20080221，动态建表
    map<string,v_pRcd > map_table_recordToInsert;
    map<string,v_pRcd > map_table_recordToUpdate;
    map<string,v_pRcd > map_table_recordToMerge;
    map<string,int>  map_table_recordToInsertCount;
    map<string,int>  map_table_recordToUpdateCount;
    map<string,int>  map_table_recordToMergeCount;
    int curr_table_count;
	
    char        m_SzSchedule[STAT_ITEM_NAME_LEN];//输入控制表名
    char        m_SzSource_ID[STAT_ITEM_NAME_LEN];//控制表中source_id
    char        m_SzStat_Time[STAT_ITEM_NAME_LEN];//update:20060713,+、-统计m_SzStat_Time 统一用输入控制表dealstarttime
    char        m_SzFileName[STAT_ITEM_CONTAINER_LEN];
    
    STAT_RECORD *Stat_Record_toUpdate;
    int           icount_toUpdate;//Stat_Record_toUpdate 数组元素个数
    int           Current_Memorry_Unit;//当前申请内存单元数
    
    char          Result_Output_Des;//结果输出方式
    CFmt_Change  Out_Record; //输出记录类型
    CF_MemFileO   Temp_Out_File;//输出文件
    char          Out_File_Type_ID[10];//输出文件类型
    char          Result_File_Path[FILE_NAME_LEN];//统计结果文件输出路径
    char          Temp_Out_File_Name[FILE_NAME_LEN];
    char          Out_File_Name_nopath[FILE_NAME_LEN];
    char          Out_File_Name[FILE_NAME_LEN];        
    int           first_time_read_record;//是否第一次读话单
    int           cond_variable_in_record_count;//条件表达式中名为话单字段名的变量个数
    Interpreter     theCompile;

    vector<string> v_tmpOutFileName;
    vector<string> v_outFileName;
	int m_maxRcdToUpdate;
   //20080604
   map<CRecordKey,STAT_RECORD*> m_key_record;
   map<CRecordKey,int> m_key_seq;
   map<int,string> map_rcdSeq_tableName;//每条记录对应哪张表
   map<string,int> map_table_recordCount;//每张表涉及到的记录数
   int config_id;
   int middRcdCount;
   int statRcdCount;
};


class CF_DUP_CStat
{
  public:
    CF_DUP_CStat();
    ~CF_DUP_CStat();
    static char* getVersion();
    int Init(char* config_id);
    
    //int setFileName(char *FileName );
    //int setFileName(char *FileName, char *sourceid);
    int setFileName(char *FileName, char *sourceid,char *dealstarttime);
   // int dealRedoRec(CFmt_Change &,char *szError_Type=NULL);
    int dealRedoRec(CFmt_Change &inrcd, char *szError_Type=NULL);
    void update_commit();
    void rollback();
    void Set_TempOutFile();//设置临时统计结果输出文件
    void Close_Temp_Outfile();
    int Neg_Commit();//应冲销要求增加，做统计结果临时文件到正式文件的转变
    //void Unlink_TemFile();//删除临时文件
    //void Unlink_File();
    int   Rename_AllOutFile();//一批文件处理完后，将文件更名为正式文件,不成功返回1，否则返回0
    void print_iStatArr_Count();
    int   total_update_count;
	int getiStatArr_Count();
  private:
  	CF_CStat *pStatArr;
  	int iStatArr_Count;


};


#endif;
