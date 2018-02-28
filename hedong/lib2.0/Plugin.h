/****************************************************************
  Project	
  Copyright (c)	2004-2006. All Rights Reserved.
		�������ſƼ����޹�˾ 
  SUBSYSTEM:	���ͷ�ļ�  
  FILE:		Plugin.h
  AUTHOR:	    wulei
  Create Time:  2004-08-25
==================================================================
  Description:  

  UpdateRecord: 
  2005-11-28:��BasePlugin���������˺���virtual char* execute( PacketParser& ps ) = 0��
		�����ڱ��ʽ�е��ò���Ľӿں���
	2006-11-27:���Ӷ�initflag�ĳ�ʼ��

==================================================================

 *****************************************************************/

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#define SUCC    0
#define FAIL    -1
#include "Packet.h"
#include <vector>


typedef struct SSParam{
	int idx;
	char name[10];
	char type;
	char value[120];
}SParam;

typedef vector<SParam> VParam;

class BasePlugin {
	private:
		char m_versionNo[16];
		char m_name[32];
		VParam m_params;
		int initflag;
	public:
		BasePlugin(){
			//setVersionNo("1.0");
			//setName("Base Plugin");
			//initflag  = 0;
			//cout <<"BasePlugin constrator,name is :"<<_name<<endl;
			//add by wulf at 20061127���Ӷ�initflag�ĳ�ʼ��
			initflag  = 0;
		};

		virtual ~BasePlugin(){
			//cout<<"BasePlugin destrator,name is :"<< _name<<endl;
		};

		char* getVersionNo() ;
		//void setVersionNo(const char* versionNo);

		
		VParam* getVParam() ;

		char* getName() ;
		//void setName(const char* name);
		
		int getInitFlag() {return initflag;};
		void setInitFlag() {initflag= 1;};

		int addParam(const int idx ,const char name[10] ,const char type,const char val[120]);
		SParam* getParam(const int idx);
		
		//virtual int init(char *compath,int index) = 0;
		//virtual int init() = 0;

		virtual int execute(PacketParser& ps,ResParser& retValue) = 0;
		virtual int execute(BufferParser& ps,ResParser& retValue) = 0;
		
		//add by weixy 20080604
		virtual int execute(PacketParser& ps,ResParser& retValue, void * point  )
		{
			return 0;
			} ;
		//end add by weixy 20080604
		
		/* add by wulf 2005-11-28 */
		virtual char* execute( PacketParser& ps ) = 0;
		/* end */

		virtual int saveQuota(const int nBillingCycleID){
			return 0;
		};
		
		virtual void printMe(){
			//cout << "\t�������:" << m_name << ",�汾�ţ�" << m_versionNo << endl;
			printf( "\t�������:%s,�汾�ţ�%s \n" ,m_name ,m_versionNo );
			return;
		};

};


#endif

