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
// ������ʷ


#ifndef _CSTATPLUGIN_H_
#define _CSTATPLUGIN_H_ 1

#include "CF_CStat.h"
#include "CF_Common.h"
#include "CF_CPlugin.h"
#include "CF_Cerrcode.h"
#include "psutil.h"
//#include "ctest.h"
const int  SOURCEID_LEN       = 10;


#define   ERR_DEAL_RECORD      8101    //�ļ���ʼ�������
#define   ERR_DEAL_1      8102    //�ļ������������
#define   ERR_DEAL_2      8103    //���ο�ʼ�������
#define   ERR_DEAL_3      8104    //���ν������ύ���ݿ⴦�����
#define   ERR_DEAL_4      8105    //���ν������ύ�ļ��������
#define   ERR_DEAL_5      8106    //�쳣�жϴ������

class CStatPlugin:public BasePlugin
{
public:
		CStatPlugin();
		~CStatPlugin();
		void init( char *szSourceGroupID, char *szServiceID, int index);

		void execute(PacketParser& ps,ResParser& retValue) ;

		void message(MessageParser&  pMessage);
		
		void printMe(){
			//��ӡ�汾��
			printf( "\t�������:stat,�汾�ţ�3.0.0 \n"   );
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


