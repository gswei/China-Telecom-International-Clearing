/****************************************************************
filename: CStatPlugin.h
module: Stat Plugin
created by: xiehp
create date: 2010-05-12
update list: 
version: 3.0.0
description:
    the header file of the classes for CStatPlugin
*****************************************************************/ 
// 更改历史


#ifndef _CSTATPLUGIN_H_
#define _CSTATPLUGIN_H_ 1

#include "CF_CStat.h"
#include "CF_Common.h"
#include "CF_CPlugin.h"
#include "CF_Cerrcode.h"
#include "psutil.h"
//#include "ctest.h"
const int  SOURCEID_LEN       = 10;


#define   ERR_DEAL_RECORD      8101    //文件开始处理错误
#define   ERR_DEAL_1      8102    //文件结束处理错误
#define   ERR_DEAL_2      8103    //批次开始处理错误
#define   ERR_DEAL_3      8104    //批次结束，提交数据库处理错误
#define   ERR_DEAL_4      8105    //批次结束，提交文件处理错误
#define   ERR_DEAL_5      8106    //异常中断处理错误

class CStatPlugin:public BasePlugin
{
public:
		CStatPlugin();
		~CStatPlugin();
		void init( char *szSourceGroupID, char *szServiceID, int index);

		void execute(PacketParser& ps,ResParser& retValue) ;

		void message(MessageParser&  pMessage);
		
		void printMe(){
			//打印版本号
			printf( "\t插件名称:stat,版本号：3.0.0 \n"   );
			return;
			};
		std::string getPluginName(){
			return "Stat";
			};
		std::string getPluginVersion(){
			return "3.0.0";
			}
		
private:
	
	CF_DUP_CStat cStat;
//	Ctest cTest;
	char szFileName[FILE_NAME_LEN];
	char szSourceID[SOURCEID_LEN];
	char szLogTableName[20];
	int IntStatRcdCout;
	char Today[9];
  	int Group_ID;
	int statMaxUpdate;
	int firstRcdFlag;
};



#endif;


