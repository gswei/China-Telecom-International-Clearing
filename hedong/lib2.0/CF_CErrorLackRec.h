#ifndef _CERRORLACKREC_
#define _CERRORLACKREC_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "CF_Lack.h"
#include "CFmt_Change.h"
#include "CF_CError.h"
#include "CF_CFscan.h"
#include "COracleDB.h"
#include "CF_Error_Table.h"
#include "Common.h"


#define PATHLEN          256 //�ļ�·������
#define MSGLEN	 	 	 300 //������¼�ĳ���
#define FILELEN	         256 //�ļ����ĳ���
#define LACKLEN		 	 500 //������Ϣ����
#define COLLEN		 	 500 //ÿ������Դ��¼����
#define CURLEN			 2048//�洢�α��ַ����ĳ���
#define CURCOUNT		 20  //�����α���������¼��
#define TABLELEN		 50	 //�����ĳ���
//#define MAX_COLUMN	     30	 //���ֶ���	
#define MAX_VARLEN 		 100


//#define ERR_FILE_OPEN	 -9  //�ļ��������ʱΪ��
#define ERR_DIR_OPEN	 -10 //Ŀ¼��ʱΪ��
//#define ERR_FILE_WRITE   -2  //д�ļ�����
//#define ERR_FILE_CLOSE   -3  //�ر��ļ�����
//#define ERR_RENAME_FILE	 -4  //
//#define ERR_REMOVE_FILE  -8  //
#define ERR_NOT_COMMIT	 -5  //û���ύ
//#define ERR_GET_RECORD   -6  //��������Դ��¼�����CFmt_Change������
#define ERR_UNKOWN	     -7  //δ֪����
#define DO_CLOSE_FIRST	 1   //�ɹ�(�Զ�������ύ)
#define SUCCESS		     0   //�ɹ�
#define FAIL		     -1  //ʧ��
#define SEARCH_EOF	     100 //���Ҳ�����������������
#define SUC_DO_COMMIT    20  //�ɹ�,�������ύ������

enum abnormity_type{
		abenum_lackinfo,    /*�����ϻ���*/
		abenum_error,       /*��*/
		abenum_nonStand,    /*cp������ͨ�������ļ�*///rewrite by wulei 2005-02-22	
		abenum_FixDup      /*cp��������ص��ļ�*///rewrite by wulei  2005-02-22
	};
	
struct ErrorLackRec_Link
{
	char filename[FILELEN];
	int  error_type;
	char source_id[6];
	CFmt_Change change;
	ErrorLackRec_Link* next;
};
struct SErrLackParam
{
  char pipe_id[6];
  char Stat_Table_Config[100];
  int Is_Save_Table;
  int Save_Table_Config;
  int Is_Stat_Table;
};


class CF_CErrorLackRec
{
private:
	FILE* fp;                       //���������ϻ������ļ�ָ��
	int Count;			            //���浱ǰ�ļ����ݼ�¼��
	char path_name[PATHLEN];        //�����ļ��������ϻ����ĸ�Ŀ¼+�쳣����Ŀ¼+������̴���Ŀ¼
	char file_name[FILELEN];	    //�����ϴε��ļ���
	char temp_file[FILELEN];	    //������ʱ�ļ���
	int proc_id;			        //������̴���
	abnormity_type type;		    //�����������쳣��������
	ErrorLackRec_Link* header;		//�������ݼ�¼�׼�¼
	ErrorLackRec_Link* current;		//�������ݼ�¼β��¼
	int LinkCount;					//�������ݼ�¼������
	char table[TABLELEN];			//��¼�������ֵ��ı�
	char szInCtlTabname[TABLELEN];
	char szSourceId[200][6];
	int iSourceCount;
    SErrLackParam sELParam;
    CF_CError_Table OSave_Table;
    CF_DUP_CStat OStat_Table;
    int Is_Commit;
private:
	int process_header();
	int process_column(int id,char* filename,CFmt_Change* change);
	int process_end();
	int Open();
	
public:
	CF_CErrorLackRec();
	~CF_CErrorLackRec();
	int init_Table(char *Pipe_id,int process_id);
	int init(char* rootpath,abnormity_type type_id,int process_id);
	int saveErrLackRec(int id,char* filename,CFmt_Change* change,char* source_id);
	int Close();
	int Commit();
	int RollBack();
	int Proc_TMPFile(char* filename);
	
	int initToDB(abnormity_type type_id, int process_id, char* tablename);
	int saveErrLackRecToDB(char* filename, int error_type, char* source_id, CFmt_Change* change);
	int CommitToDB();
	int RollBackToDB();
};

#endif
