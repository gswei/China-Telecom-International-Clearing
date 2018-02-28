/*******************************************************************************************
	Copyright 2006 Poson Co.,Ltd. Inc. All Rights Reversed   
********************************************************************************************

**Class			: ProcessRec_Mgr
**Description		: �ۺϽ���--����������
**Author		: Wulf
**StartTime		: 2005/12/02
**Last Change Time 	: 2006/02/08

*******************************************************************************************/

#ifndef _PROCESSREC_MGR_H_
#define _PROCESSREC_MGR_H_

//#include "COracleDB.h"

#include "C_PluginMethod.h"
#include "psutil.h"

#include <vector>
#include <map>

const int PATHLENTH = 250;

class Input2Output
{
protected:
	int In_Idx;
	int Out_Idx;
public:
	Input2Output(int in,int out);
	~Input2Output();
	
	//��ȡIn_Idx
	int get_In_Idx() { return In_Idx; }
	//��ȡOut_Idx
	int get_Out_Idx() { return Out_Idx; }
};
typedef vector<Input2Output*> In2Out;

class ClassifyRule
{
protected:
	/*���ȼ���*/
	int Priority;
	/*�ּ�����*/
	char ExpMethod[METHODLENTH];	
	/*�ּ�·��*/
	char PickPath[PATHLENTH];
	/*�ּ��ʶ*/
	char PickFlag[2];
public:
	ClassifyRule(int priority, char *expmth, char *pickpath, char *pickflag);
	~ClassifyRule();
	
	int get_Priority() { return Priority; }
	char * get_ExpMethod() { return ExpMethod ;}
	char * get_PickPath() { return PickPath; }
	char * get_PickFlag() { return PickFlag; }
	
	int set_ExpMethod(char *expMethod );
};

typedef vector<ClassifyRule*> ClRule;

class ProcessRec_Mgr
{
protected:
	/*����ģ��ID*/
	int WorkFlow_ID;
	/*���̱�ʶID*/
	int Process_ID;
	/*��������ṹ*/
	CFmt_Change OutRec;
	/*����������*/
	C_PluginMethod *m_Method;
	/*����������*/
	int  m_Method_cnt;
	/*�ּ���*/
	ClRule m_Clrule;
	//2005-06-16�ּ��࣬���Ϸ���ǰ��CLASSIFY_LOCATION=F
	ClRule m_ClFrule;

	/*����ļ�����ID*/
	char OutFileTypeID[6];
	/*�����ļ�����ID*/
	char InFileTypeID[6];
	/*���������ʽת����Ӧ��ϵ*/
	In2Out	FileType_Chg;
	/*������ļ��ľ���·��*/
	char SL_fname[500];
	/*PluginProxy*/
	PluginProxy pluginp;


private:
	/*��̬�������Ӳ���*/
	int AddVariable( int WorkFlowID, int ProcessID );
	/*��ʼ�����������ʽ��Ӧ��ϵ*/
	int Init_In2Out( int WorkFlowID, int ProcessID );
	/*��ʼ���ּ���*/
	int InitClRule( int WorkFlowID, int ProcessID );
	/*��ʼ������������*/
	int InitPrcM( int WorkFlowID, int ProcessID, char *slfile );
	
public:
	DBConnection conn;//���ݿ�����
	//���캯��
	ProcessRec_Mgr();
	//��������
	~ProcessRec_Mgr();
	/*��̬���������*///edit by liuw
	C_Compile m_Compile;
	/*map<string,char *>*/
	VAR_CHAR var_char;
	//��ʼ������
	int Init(int WorkFlowID, int ProcessID);
	int Init( int WorkFlowID, int ProcessID, char *slfile );
	//ͬ�����ݿ�����
	//int SycDB();
	//д�����뻰��
	int Set_InRec( CFmt_Change &inrec );
	//��ȡ�������
	int Get_OutRec( CFmt_Change &outr );
	//��ȡ�������
	int Get_OutRec( char *outr, int len );
	//���������ķּ�ҵ��
	int Process_Classify( char* retid, int& retcnt );
	//�������ǰ�ķּ�ҵ��
	int Process_FClassify( char* retid, int& retcnt );
	//����������
	int Process(int&);
	/*��̬�������Ӳ���*/
	int AddVar( const char *varname, char *varvalue );
	/* ��ӡ����汾�� */
	int PrintSLNo(char *SLName);
	/*�󶨹����ڴ�ͼ���Ƿ���Ҫ���¹����ڴ�*/
	int LoadOrUpdateMem(char *SLName, char* ShmPath);
  //���ⲿ�������������д���������
	int Set_OutRec( CFmt_Change &outrec );
	
	friend class C_PluginMethod;
};



#endif
/****************************The End!*********************************/