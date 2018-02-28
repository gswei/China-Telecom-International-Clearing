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
// ������ʷ
//20070509��setFileName�����ļ�����ѯshedule������������ʱ�ٶȻ�ǳ������޸ĸýӿڣ�
//           sourceid���ⲿ���룬��sourceid��dealstarttime�����ⲿ����
//20070516��STAT_END_TIME������������ȥĬ��ֵ����������������׳��쳣
//20070530������ҵ��������Ҫ���С������������ͳ�ƣ����Ӵ˹���
//20070619���������Աm_SzFileName���鳤;����100��Ϊ251
//20080516�������ڴ�����ļ�ʱ�������Ϊ�������¸Ľ�
//			 (1)�Ľ�Connect()�����㷨��ֻɨ��һ��Stat_Record_toUpdate[]����
//                   (2)Ϊ�����ڴ�ʹ�����������µ�������Ҫinsert�� update�ļ�¼��
//				����ֱ����Stat_Record_toUpdate���޸ļ�¼����vector��ŵ���¼��ָ��
//			  (3)Ϊ��ߴ��ļ��Ĵ����ٶȣ���������������
//				STAT_MAXNUM_UPATE:��������¼����һ�α�
//                        STAT_MAXNUM_CACULATE:������������һ���ۼ�
//			(4) ����Stat_Record_toUpdate[]�ڴ����롢�ͷŴ�����ֻ�ڲ���ʱ����������
//20080604��(1)�㷨�Ľ���ʹ��MAPȡ��ԭ��������������ͬ�����ۼӵ�ͳ�Ʒ���
//			 (2)�б�ͳ��һֱû���õ���ȥ���˹���
//20080714�����ӹ��ܣ�֧����process_id��Ϊͳ��ά��ͳ��

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

#include "Common.h"
#include "CFmt_Change.h"
#include "CF_CError.h"
#include "COracleDB.h"
#include "CF_MemFileIO.h"
#include "interpreter.h"
#include "wrlog.h"

#define ConfigIDSPILT             ';'
#define ALLNULLPASS              'A'
#define PARTNULLPASS             'P'
#define NONULLPASS              'N'

//STAT_MAXNUM_CACULATE��Ĭ��ֵ��ͳ�ƻ����ṹ���������ݵ�������
#define	STAT_RECORD_COUNT_MAX          3000  
//20080516���õ��ĵط���STAT_RECORD_COUNT_MAX����
//�ﵽ�û���������update_stat ���ݿ����
//#define	STAT_RECORD_COUNT              3000  
#define  STAT_INSERT_CONNT             500 //���¿��ʱһ�β���ļ�¼��
#define	STAT_ITEM_COUNT                100    //�������
#define	STAT_ITEM_NAME_LEN             50    //����������
#define	STAT_ITEM_CONTAINER_LEN        251    //�������ݳ���
#define  STAT_GROUP_COUNT               10    //ͳ�Ʊ�ʶ�����
#define  MAX_STATID_NUM                 50    //һ��������������ͳ��ʶ����
#define  SQL_LENGTH               2000


#define	STAT_SZ_TYPE                   2     //��������Ϊ�ַ����͵����ͱ�־
#define	STAT_N_TYPE                    1     //��������Ϊ���͵����ͱ�־
#define	STAT_N_COUNT_TYPE              0     //��������Ϊ������Ϊ�ۼ�������ͱ�־
//20070530����С������������ͳ��
#define	STAT_D_COUNT_TYPE              3     //��������Ϊ���������� ��Ϊ�ۼ�������ͱ�־

#define	KEYWORK_MIN                    11    //�ؼ��ִ�11 ��ʼ
#define	KEYWORK_SOURCEID               11    //�ؼ���source_id����Ϊ11
#define	KEYWORK_STATTIME               12    //�ؼ���stat_time����Ϊ12
#define	KEYWORK_FILENAME               13    //�ؼ���filename����Ϊ13
#define  KEYWORK_UPDATETIME            14    //�����м������ʱ��
#define  KEYWORK_CDRDURATION           15    //���ڰ�ͨ��ʱ��ͳ��
#define	KEYWORK_ERRORTYPE              16    //�ؼ���errortype
#define	KEYWORK_PROCESSID              17   //PROCESS_ID
//�Ӵ��Ժ�ؼ������20 ��ʼ
#define  KEYWORK_THOUSANDCALLNO        21    //���оֺţ���;�Ʒ�ҵ����ʹ��
#define  KEYWORK_DAYCYCLE                22  //�����ڣ�20070314

//���������ڴ������STAT_MAXNUM_UPATE��Ĭ��ֵ
#define  MEMORY_APPLY_UNIT              1000  
#define  ERR_GETENV					8001	//��ȡ������������
#define	ERR_REQU_MEM				8002	//�����ڴ����
#define	ERR_SELECT  				8004	//���ݿ����Ҳ�����¼
#define  ERR_STATIEMT_OVER      	       8006    //����ͳ��������������Χ
#define  ERR_FILED_VAL                        8007    //�ֶ����ݲ����϶���
#define  ERR_TABLEITEM_OVER              8008    //ͳ�Ʊ��ֶγ������Χ
#define  ERR_STATGROUP_OVER           8009    //ͳ��������������Χ
#define  ERR_FIELD_INDEX                 8010     //�ֶ���Ŵ���
#define  ERR_FIELD_NULL                   8011     //�����ֶ�����Ϊ��
#define  ERR_GET_SUBSTR                  8012    //ȡ���ַ�������
#define  ERR_KEY_ITEM                      8013     //�Ƿ��ؼ���
#define  ERR_EXPRESS                        8014     //�������ʽ����
#define  ERR_INSERT_TABLE          8015     //���롢���¼�¼����
#define  ERR_VALID_CONFIGID          8016     //���롢���¼�¼����

#define	ERR_SELECT_NONE                -1001	//���ݿ����Ҳ�����¼
#define	BILL_COUNT_IS_NEGATIVE         -1002	//bill_count Ϊ����
#define	ERR_FIND_ITEM_NONE             -1003	//�Ҳ����ùؼ���
#define	ERR_ITEM_NULL                   -1004	//������Ϊ��
#define	ERR_ALL_ITEM_NULL               -1005	//����ͳ���鶼��������Ϊ��


typedef vector<int> v_recoredSeq;

struct FIELD_VALUE//�������ʽ������Ӧ�ֶ�ֵ
{
char szField_Value[250];
//�������ֱ����������Ի����ֶ�������ͳ�Ʊ��ֶ���
// >0��ʾ�����ֶ�����-1��ʾ���ֶ���
int index;
};


struct STAT_TABLE//ͳ�Ʊ�
  {
  char SzTable_Name[STAT_ITEM_CONTAINER_LEN];//����
  char ChStat_Type;//������
  char ChIs_Count;//�Ƿ���Ҫͳ�ƻ�����
  char SzCount_Item_Name[STAT_ITEM_NAME_LEN];//������ͳ������
  char CHStat_Mode;//ͳ��ģʽ��UΪupdateģʽ��IΪinsertģʽ
  
  char SzStat_Item[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];//ͳ����Ŀ����
  int  NField_Index[STAT_ITEM_COUNT];

  char ChCondiction[250];//�������ʽ  
  FIELD_VALUE *cond_field;//�������ʽ������
  FIELD_VALUE *cond_field_value;//������������
  int cond_field_count;//�������ʽ��������
  
  int  NStat_Item_Count;
  int  NItem_Type[STAT_ITEM_COUNT];
  int  NStat_Item_Begin[STAT_ITEM_COUNT];
  int  NStat_Item_End[STAT_ITEM_COUNT];
  int  NStat_Item_IndexInField[STAT_ITEM_COUNT];
  char chStat_Item_SprInField[STAT_ITEM_COUNT];
  char Predfval_Or_Statid[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];  
  int   item_cond_index[STAT_ITEM_COUNT];//��ͳ�����Ӧ�������ʽ�еڼ�������
  //�ۼ��λת�����ձ�ʾ����Ҫת��
  //"/60"��ʾ����Ϊʱ�䣬��Ҫ����60������ȡ��
  char ChCount_Item_Unit[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];
  
    //20080220,��̬���ɱ�
    int table_name_key_count;
    vector<string> v_tableName;//�Ѿ��Ľ������
    char NField_dataType[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];
    char IsIndex[STAT_ITEM_COUNT]; 
};


struct STAT_RECORD//ͳ�Ƽ�¼
{
  char SzStat_Item[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];
  int  NItem_Type[STAT_ITEM_COUNT];
  int  NStat_Item_Count;
  int  NBill_Count;  
};

struct STAT_ITEM_GROUP_INFOR//����ͳ������Ϣ
{
int iStat_Item_Group_Num;//ͳ���������
int ID_Group_Filed_Index[STAT_GROUP_COUNT];//ͳ�Ʊ�ʶ������λ��
int Value_Group_Field_Index[STAT_GROUP_COUNT];//ͳ��ֵ�黰������λ��
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
    int Init(char *,int ,int,char *,int);
    //int setFileName(char *);
    //20070509,�޸�setFileName�ӿ�
    int setFileName(char *FileName, char *sourceid);
    int setFileName(char *FileName, char *sourceid,char *dealstarttime);
    int setFileName(char *FileName, char *sourceid,char *dealstarttime,int process_id);
    
    int dealRedoRec(CFmt_Change &, int IError_Type);
    void update_commit();
    void rollback();    
    void Set_TempOutFile(char * today, int group_id);//���ý�������ʱ�ļ���
    void Close_Temp_Outfile();
    void Unlink_TemFile();//ɾ����ʱ�ļ�
    void Unlink_File();
    int   Rename_OutFile();//����ʱ�ļ�����Ϊ��ʽ�ļ�    
  private:
    int DeleteSpace( char *ss );
    int Make_Select_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp);
    int Make_Delete_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp);
    int Make_Insert_Sql(char *sqltmp,const char *table_name);
    int Make_Update_Sql(char *sqltmp,const char *table_name);
    int Set_KeyWord(STAT_RECORD &Cur,int i,char *Key_Container);
    int Set_KeyStatItem(STAT_RECORD &,int );  
    int Insert_Stat();
    int Prepare_Rec();
    int Insert_Table();
    int Update_Table();
//20080604
//	int Stat_Record_Col(CFmt_Change &, int IError_Type=0);//����
//    int Update_Stat_File();    
//    int Update_Stat_File_Map();
//    int Connect(int Add_Count);
    int Stat_Record_Insert(CFmt_Change &, int IError_Type=0);
   int Stat_Record_Update(CFmt_Change &InRecord , int IError_Type);
    int AddOneRecord(STAT_RECORD& addRcd);
    void Rqsort(void  *base,  size_t  nel,  size_t  width);
    int Count_Item(char *string,char seperator);
    void Get_Item(char *string,char seperator,int *item_group,int item_count);
//20050621
    int Exch_Field(CFmt_Change &inrcd);
    int Read_Cond_field();//���������ʽ�ֶθ���
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
//20080604,ɾ���б�ͳ�ƹ���
    /*
    //��ǰһ������Ҫinsert��update�ļ�¼
    STAT_RECORD *Record_toInsert;
    int            icount_toInsert_table;
    STAT_RECORD *Record_toUpdate;
    int            icount_toUpdate_table;
    */
    
 //20080221����̬����
    map<string,v_pRcd > map_table_recordToInsert;
    map<string,v_pRcd > map_table_recordToUpdate;
    map<string,int>  map_table_recordToInsertCount;
    map<string,int>  map_table_recordToUpdateCount;
    char crr_table_name[STAT_ITEM_CONTAINER_LEN] ;//��ǰ��¼��Ӧ����
    int curr_table_count;
	
    char        m_SzSchedule[STAT_ITEM_NAME_LEN];//������Ʊ���
    char        m_SzSource_ID[STAT_ITEM_NAME_LEN];//���Ʊ���source_id
    char        m_SzStat_Time[STAT_ITEM_NAME_LEN];//update:20060713,+��-ͳ��m_SzStat_Time ͳһ��������Ʊ�dealstarttime
    char        m_SzFileName[STAT_ITEM_CONTAINER_LEN];
    char       m_SzStat_process_id[STAT_ITEM_CONTAINER_LEN];
    //char        Row_Or_Col_Flag;//����������ǣ�RΪ���CΪ�б�    
   // int          istat_item_numb;//��ǰ����ͳ�Ʊ�ʶ����  
   // int		icol_stat_insert_numb;//��ǰ�������ɼ�¼��
   
    //STAT_ITEM_GROUP_INFOR s_Col_Stat_Item;//����ͳ������Ϣ
    
    STAT_RECORD *Stat_Record_toUpdate;
    int           icount_toUpdate;//Stat_Record_toUpdate ����Ԫ�ظ���
    int           Current_Memorry_Unit;//��ǰ�����ڴ浥Ԫ��
    
    char          Result_Output_Des;//��������ʽ
    CFmt_Change  Out_Record; //�����¼����
    CF_MemFileO   Temp_Out_File;//����ļ�
    char          Out_File_Type_ID[10];//����ļ�����
    char          Result_File_Path[FILE_NAME_LEN];//ͳ�ƽ���ļ����·��
    char          Temp_Out_File_Name[FILE_NAME_LEN];
    char          Out_File_Name[FILE_NAME_LEN];        
    int           first_time_read_record;//�Ƿ��һ�ζ�����
    int           cond_variable_in_record_count;//�������ʽ����Ϊ�����ֶ����ı�������
    Interpreter     theCompile;
    //20070315
 	char endTime[6]; 	
	char lastdate[9];
	char crrdate[9];
	int before_endtime_flag;

	int m_maxRcdToUpdate;
   //20080604
   map<CRecordKey,STAT_RECORD*> m_key_record;
   map<CRecordKey,int> m_key_seq;
};


class CF_DUP_CStat
{
  public:
    CF_DUP_CStat();
    ~CF_DUP_CStat();
    int Init(char * ,int ,char *configid=NULL,char statfieldnullpass='0');
    
    //int setFileName(char *FileName );
    int setFileName(char *FileName, char *sourceid);
    int setFileName(char *FileName, char *sourceid,char *dealstarttime);
    int setFileName(char *FileName, char *sourceid,char *dealstarttime,int process_id);
    int dealRedoRec(CFmt_Change &,int IError_Type=0);
    void update_commit();
    void rollback();
    void Set_TempOutFile(char * today, int group_id );//������ʱͳ�ƽ������ļ�
    void Close_Temp_Outfile();
    int Neg_Commit();//Ӧ����Ҫ�����ӣ���ͳ�ƽ����ʱ�ļ�����ʽ�ļ���ת��
    //void Unlink_TemFile();//ɾ����ʱ�ļ�
    //void Unlink_File();
    int   Rename_OutFile();//����ʱ�ļ�����Ϊ��ʽ�ļ�,���ɹ�����1�����򷵻�0
    void print_iStatArr_Count();
    int   total_update_count;
	
  private:
  	CF_CStat *pStatArr;
  	int iStatArr_Count;
	//int iStatArr_Valid_Count;
    	char chFieldNULLPass;

};


#endif;
