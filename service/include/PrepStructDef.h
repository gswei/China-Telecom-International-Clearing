/****************************************************************
filename: PrepStructDef.h
module: classify&analyse
created by: Wu Longfeng
create date: 20051220
version: 1.0.0
description:
	����Ԥ����ģ���������õ��Ľṹ��
update list:
	
*****************************************************************/

#ifndef _PREPSTRUCTDEF_H_
#define _PREPSTRUCTDEF_H_ 1

#include "CF_CFmtChange.h"
#include "CF_Common.h"
#include "CF_Config.h"
//#include "CF_COracleDB.h"
#include "CF_CErrorLackRec.h"
#include "CF_CProcessMonitor.h"
#include "CF_InfoPoint.h"
#include "Classify.h"
#include "CF_CPluginMessage.h"
//#include "main/pluginengine/zhjs/plugininitializer.h"
#include "plugininitializer.h"
//extern CFmt_Change g_outrcd;

const int SERVER_LEN			=10;
const int EXP_METHOD_LEN		= 512;
const int MAX_EXP_METHOD_COUNT	= 128;
const int STR_LEN			= 256;
const int FIELD_LEN				= 256;
const int DATE_LEN				= 8;
const int ERR_MSG_LEN			= 1024;

//using namespace zhjs;

/* ����LIST_SQL���еļ�¼ */
struct SListSql{
	char m_szSqlId[16+1];
	char m_szUpdateFlag[1+1];
	char m_szUpdateSqlName[32+1];
};

/* ���ڱ���ÿ���ļ���Ϣ�Ľṹ�� */
struct SFileStruct
{
	char szRealFileName[FILE_NAME_LEN+1];//��ʵ�ļ���(����proc_index)
	char szFileName[FILE_NAME_LEN+1];    //�ļ���(��proc_index)
	char szSourceId[SOURCE_ID_LEN+1];    //��������Դ
	char szSourcePath[PATH_NAME_LEN+1];  //����Դ·��
	char szDealStartTime[14+1];          //��ʼ����ʱ��
	char szDealEndTime[14+1];
	int iPartID;
//	int  szSourceLackCode;
	char szLocalNet[2+1];                //Ĭ�Ϲ�����
	char szTollcode[10+1];               //Ĭ������
	char szServCat[SERVER_LEN+1];        //����ԴĬ�ϵ�ҵ�����
	char szServCatConfig[30+1];					
	char szSourceFiletype[FIELD_LEN+1];  //������Դ��Ӧ�Ļ�����ʽ

	long lFileId;						//D_FILE_RECEIVED���е��ļ�ID
	char szReceiveTime[14+1];			//D_FILE_RECEIVED���е�RECEIVE_TIME
	int iDealFlag;						//�����־ 0:δ���� 1:�Ѵ��� 2:���ύ�ļ� 3:�쳣
	int iMaxCount;					//һ��������������

	SFileStruct()
	{
		memset(szRealFileName, 0, sizeof(szRealFileName));
		memset(szFileName, 0, sizeof(szFileName));
		memset(szSourceId, 0, sizeof(szSourceId));
		memset(szSourcePath, 0, sizeof(szSourcePath));
		memset(szDealStartTime, 0, sizeof(szDealStartTime));
		memset(szDealEndTime, 0, sizeof(szDealEndTime));
		memset(szLocalNet, 0, sizeof(szLocalNet));
		memset(szTollcode, 0, sizeof(szTollcode));
		memset(szServCat, 0, sizeof(szServCat));
		memset(szServCatConfig, 0, sizeof(szServCatConfig));
		memset(szSourceFiletype, 0, sizeof(szSourceFiletype));
		lFileId = 0;
		iDealFlag = 0;
		iMaxCount = 0;
		iPartID = 0;
	};
};

struct SSourceStruct
{
	char szSourceId[SOURCE_ID_LEN+1];    //��������Դ
	char szSourcePath[PATH_NAME_LEN+1];  //����Դ·��
//	char szLocalNet[2+1];                //Ĭ�Ϲ�����
	char szTollcode[10+1];               //Ĭ������
	char szServCat[SERVER_LEN+1];        //����ԴĬ�ϵ�ҵ�����
	char szSourceFiletype[FIELD_LEN+1];  //������Դ��Ӧ�Ļ�����ʽ
	int iMaxCount;					//һ��������������

	SSourceStruct()
	{
		memset(szSourceId, 0, sizeof(szSourceId));
		memset(szSourcePath, 0, sizeof(szSourcePath));
//		memset(szLocalNet, 0, sizeof(szLocalNet));
		memset(szTollcode, 0, sizeof(szTollcode));
		memset(szServCat, 0, sizeof(szServCat));
		memset(szSourceFiletype, 0, sizeof(szSourceFiletype));
		iMaxCount = 0;
	};
};

/* ���ڱ�������Դ��Ϣ�Ľṹ�� */
/*
struct SSourceStruct
{
	char szSourceId[5+1];
	char szSourcePath[PATH_NAME_LEN+1];
};
*/

/* ������DealFile.cpp��MainFlow.cpp�д��ݲ��� */
struct SParameter
{
	char szServiceId[MAXLENGTH+1];  //������
	char szSourceGroupId[MAXLENGTH+1]; //����Դ��
	int iProcessId; //����������
	
	char szWorkflowId[SERVER_LEN+1];
	char szServerId[SERVER_LEN+1];
	int  iInputId;                  //����ӿ�ID
	int  iOutputId;                  //����ӿ�ID

	CProcessMonitor ProcMonitor;		//���̹������

	char szLogTabname[TABLENAME_LEN+1];       //��־��
	bool bBakFlag;					//�Ƿ�������ļ�
	bool bCommemFlag;				//�Ƿ����ӹ����ڴ�
	char szIsFmtFirst[2];            //�Ƿ����и�ʽ�����
	char szSlPath[PATH_NAME_LEN+1];  //����ļ�����·��
	char szSlName[FILE_NAME_LEN+1];
	CF_CErrorLackRec lack_info;      //��/�����ϵ��ӿ���
	CF_CErrorLackRec abnormal;      //��/�����ϵ��ӿ���
	zhjs::PluginInitializer pluginInitializer;		//����ӿ�
//	char FileNameConstrain[EXP_METHOD_LEN+1];
	CClassify classify;          //�ּ���
	TMP_VAR map_DefVar;				//Ĭ�ϱ�����
	//CVariableContainer defaultVar;
	
	char szInputFiletypeId[5+1];     //�����ļ�����
	char szOutputFiletypeId[5+1];    //����ļ�����
	char szInrcdType[2];           //�����ļ���¼����
	char szOutrcdType[2];          //����ļ���¼����
	char szInPath[PATH_NAME_LEN+1];  //����·�������·����
	char szOutPath[PATH_NAME_LEN+1]; //���·�������·����
	
	bool bOutputFile;     //�Ƿ�����ļ�

	char szMsgKey[10+1];

	CF_MemFileO fmt_err2file;	//��ʽ�������ļ�
	CF_CErrorLackRec fmt_err2table;	//��ʽ������ͳ�Ʊ�
	char szFmtErr2Table[2];     //Y��ʾ���
	char szFmtErrSaveTableId[TABLENAME_LEN+1];  //��ʽ������
	char szFmtErrStatTableId[TABLENAME_LEN+1];  //��ʽ����ͳ�Ʊ�
	char szFmtErr2File[2];      //Y��ʾ���ļ�
	char szFmtErrDir[PATH_NAME_LEN+1]; //�����ļ�·��
	char szFmtTimeOutDir[PATH_NAME_LEN+1];	//��ʱ�����ļ�·��
	char szFmtOtherDir[PATH_NAME_LEN+1];	//δ�����ʽ�����ļ�·��

//	char szLack2Table[2];
//	char szLackStatConfig[2];
	char szLackSaveTableId[TABLENAME_LEN+1];
	char szLackStatTableId[TABLENAME_LEN+1];
	char szAbnSaveTableId[TABLENAME_LEN+1];
	char szAbnStatTableId[TABLENAME_LEN+1];
	
	int iServCatConfig;     //SERV_CAT_ID�ڻ����е��ֶκ�
//	int iMaxNum;   //һ�����������ļ���

	InfoLog info;						//��Ϣ����־

	int iSleepTime;

	SParameter()
	{
		memset(szServiceId, 0, sizeof(szServiceId));
		memset(szSourceGroupId, 0, sizeof(szSourceGroupId));
		iProcessId = 0;

		memset(szWorkflowId, 0, sizeof(szWorkflowId));
		memset(szServerId, 0, sizeof(szServerId));

		memset( szInputFiletypeId, 0, sizeof(szInputFiletypeId) );
		memset( szOutputFiletypeId, 0, sizeof(szOutputFiletypeId) );
		memset( szInrcdType, 0, sizeof(szInrcdType) );
		memset( szOutrcdType, 0, sizeof(szOutrcdType) );
		memset( szInPath, 0, sizeof(szInPath) );
		memset( szOutPath, 0, sizeof(szOutPath) );
	};
};


#endif

