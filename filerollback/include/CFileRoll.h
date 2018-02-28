/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-23
File:			 CFileRoll.h
Description:	 �ļ�����ģ��
1��	��ʼ������ȡ���ñ��е��ļ�Ŀ¼�ͱ���Ŀ¼��
2��	���ļ�Ŀ¼�л�ȡ�ļ�
3��	��ȡ�ļ����ݣ��������ļ���ȫ����ȡ���ڴ���
4��	�����ļ�����ѯ��Ӧ����ԴID��fileid�Ͷ�Ӧ�������
5��	���ݲ�ѯ���ݣ��ӽ�����н�fileid��Ӧ�Ľ��ɾ����


�ӿ�˵��  �������jsrollfile 
**************************************************************************/

#ifndef CFILEROLL_H
#define CFILEROLL_H


#include "bill_process.h"
#include "psutil.h"
#include "tp_log_code.h"

#include <dirent.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include <unistd.h>     //��ȡ��ǰ��������Ŀ¼
#include <iostream>
#include <fstream>
#include "dr_api.h"
#include "RTInfo.h"   //petri��״̬

using namespace tpss;  //��psutil.h��Ӧ
#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFscan.h"


using namespace std;


//����Դ��Ϣ
struct SOURCECFG
{
  char szSourceId[6];		//����ԴID
  //char szFile_Fmt[6];		//����Դ��Ӧ�ĸ�ʽ
  //char szInFileFmt[6];		
  char szSourcePath[256];   //����Դ����·��
  //char szTollCode[16];      //����������
  char filterRule[256] ;  //���˹���  ͨ��ƥ���ļ���������Դ
  int  file_begin;		  //��ȡ�ļ��������ʱ��
  int  file_length;

  //char serverCatID[5];
};

struct SRollFile{
	char filename[50];//�ļ���
//	string filename;
	char sourceID[10];//�ļ�����Ӧ������ԴID
	int  fileID;//�ļ���Ӧ���ļ�ID
	char tablename[20];//����Դ��Ӧ���ջ��ܽ������
};
class CFileRoll :public  PS_Process
{
private:
	vector<SRollFile>  szrollfiles;
    char  m_filename[256];   //ȫ·���ļ���
	char  input_path[256];  //�ļ�����·��
	char  output_path[256]; //�ļ����·��
	char  erro_path[256];   //�ļ�ʧ��·��
	char  erro_msg[1024];   //������Ϣ
	CF_CFscan scan;	//�ļ�ɨ��ӿ�
    DBConnection conn;//���ݿ�����
	map<string,SOURCECFG> m_SourceCfg; 
	
	short petri_status ;

	bool m_enable ;	//����״̬
	int  drStatus;  //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	char m_SerialString[4096];
	char m_AuditMsg[4096];
    

public:
	CFileRoll();
	~CFileRoll();
	bool init(int argc,char** argv);
	//bool run();
//	bool getFile();//��ȡĿ¼�µ��ļ���
	bool getFilenames();//����һ��������ȡ���ļ��ж�ȡ�ļ������ڴ�SRollFile �У�
	bool rollFile(int i);//����szrollfiles���ļ��������ļ��������ñ��в�ѯ�ļ���Ӧ������ԴID���Ӹ�ʽ���ļ��ǼǱ�D_SCH_FORMAT�в�ѯfileid����������ԴID��ѯ�������ñ��ȡ��Ӧ�ջ��ܽ�����������ӽ������ɾ��fileid��Ӧ�ļ�¼
	void saveLog(char state,int i); 
	bool moveFiles(bool flag);//���Ѿ��������ļ��ƶ���ָ������Ŀ¼
	int  LoadSourceCfg();
	int  getSourceFilter(char* source,char* filter,int &index,int &length);
	bool checkFormat(const char *cmpString,const char *format);
	void prcExit();

	//����ƽ̨
    bool drInit();
	bool CheckTriggerFile();
	int  drVarGetSet(char* m_SerialString);
	bool IsAuditSuccess(const char* dealresult);
	//bool rollFile(char *filename);
};

#endif

