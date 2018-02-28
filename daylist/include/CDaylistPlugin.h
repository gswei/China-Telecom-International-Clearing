/****************************************************************
filename: CDaylistPlugin.h
module: Daylist Plugin
created by: xiehp
create date: 2010-05-12
update list: 
version: 3.0.0
description:
    the header file of the classes for CDaylistPlugin
*****************************************************************/ 
// ������ʷ

#ifndef _CDAYLISTPLUGIN_H_
#define _CDAYLISTPLUGIN_H_ 1


#include "CF_Common.h"
#include "CF_CPlugin.h"
#include "CF_Cerrcode.h"

#include "DataInOutMap.h"
#include "InputInfo.h"
#include "TreeTool.h"
#include "ListSpecialCtl.h"
#include "ExpressCtl.h"
#include "RuleNo.h"
#include "psutil.h"

const int  SOURCEID_LEN       = 10;

#define   ERR_INIT                      8100    //�ļ���ʼ�������
#define  ERR_GET_ENV               8101  /*��ȡ������������*/



class CDaylistPlugin:public BasePlugin
{
public:
		CDaylistPlugin();
		~CDaylistPlugin();
		void init(char *szSourceGroupID, char *szServiceID, int index);

		void execute(PacketParser& ps,ResParser& retValue) ;

		void message(MessageParser&  pMessage);
		
		void printMe(){
			//��ӡ�汾��
			printf( "\t�������:stat,�汾�ţ�3.0.0 \n"   );
			return;
			};
		std::string getPluginName(){
			return "stat";
			};
		std::string getPluginVersion(){
			return "3.0.0";
		}
		
private:
	
	char szLogStr[500];
	char szSourceGroup[10];
	char szFileName[FILE_NAME_LEN];
	char szSourceID[SOURCEID_LEN];
	int IntTreeMaxSize;		//TREE���������
	char szInputFiletypeId[10];      /*�����ļ�����*/
	char szLackind[50];
	int merge_count;/*�ڴ��л�����*/
	int iTotalNum;/*��ǰ�����ļ���������*/
	int iRightNum;/*��ǰ�����ļ�������������*/
	int iLackNum;/*��ǰ�����ļ������ϻ�����*/
	int iOtherNum;/*��ǰ�����ļ��μӺ��ʻ�����*/
	
	char cWithTollcodeCondiction[250];
	char cMergeCondiction[250];
	
	CExpress_CTL    mergeCondictionCtl;//�����жϻ����Ƿ������������	
       CExpress_CTL    withTollcodeCondictionCtl;//�����жϻ������к����費��Ҫ����
	CList_Special_CTL  list_special_ctl;  
	CRuleno_Condiction Ruleno_Condiction;
	CSource_Merge_Condiction sourceMergeCond;
	CSourceInpuInfo sourceInputInfo;
	CTreeTool		treeTool;
	CFmt_Change inrcd;
	int newDealFlag;
};




#endif;

