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
// 更改历史

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

#define   ERR_INIT                      8100    //文件开始处理错误
#define  ERR_GET_ENV               8101  /*读取环境变量出错*/



class CDaylistPlugin:public BasePlugin
{
public:
		CDaylistPlugin();
		~CDaylistPlugin();
		void init(char *szSourceGroupID, char *szServiceID, int index);

		void execute(PacketParser& ps,ResParser& retValue) ;

		void message(MessageParser&  pMessage);
		
		void printMe(){
			//打印版本号
			printf( "\t插件名称:stat,版本号：3.0.0 \n"   );
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
	int IntTreeMaxSize;		//TREE的最大容量
	char szInputFiletypeId[10];      /*输入文件类型*/
	char szLackind[50];
	int merge_count;/*内存中话单数*/
	int iTotalNum;/*当前输入文件处理话单数*/
	int iRightNum;/*当前输入文件正常处理话单数*/
	int iLackNum;/*当前输入文件无资料话单数*/
	int iOtherNum;/*当前输入文件参加合帐话单数*/
	
	char cWithTollcodeCondiction[250];
	char cMergeCondiction[250];
	
	CExpress_CTL    mergeCondictionCtl;//用于判断话单是否满足合帐条件	
       CExpress_CTL    withTollcodeCondictionCtl;//用于判断话单主叫号码需不需要带区
	CList_Special_CTL  list_special_ctl;  
	CRuleno_Condiction Ruleno_Condiction;
	CSource_Merge_Condiction sourceMergeCond;
	CSourceInpuInfo sourceInputInfo;
	CTreeTool		treeTool;
	CFmt_Change inrcd;
	int newDealFlag;
};




#endif;

