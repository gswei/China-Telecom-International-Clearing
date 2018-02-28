/****************************************************************
 filename: SjywAnaVPDN.h
 module: 用户自定义插件头文件
 created by:    vivi
 create date:   2012-09-18
 version: 3.0.0
 description: 
 update:

*****************************************************************/
#ifndef _C_SJYWANAVPDN_H_
#define _C_SJYWANAVPDN_H_

#include "CF_CPlugin.h"
#include "ComFunction.h"

class   C_SjywAnaVPDN: public BasePlugin
{
private:
public:
        C_SjywAnaVPDN();
        ~C_SjywAnaVPDN();      
        void init(char *szSourceGroupID, char *szServiceID, int index);
        void execute(PacketParser& pps,ResParser& retValue);
        void message(MessageParser&  pMessage);
        std::string getPluginName();
        std::string getPluginVersion();
        void printMe();

private:        
        BaseAccessMem *table;
        char m_szTableName[TABLENAME_LEN+1];
        int m_iTableOffset;
        int m_iIndex;
        DataStruct m_InData;
        DataStruct m_OutData;
        
        int CheckAreaCode(char *,char *,char *);
	      int CheckVPDN(char *,char *,char *);
};

#endif	

