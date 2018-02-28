/**************************************************************************
* 类：		CNetServer
* 功能：	前后台通讯接口的网络传输层
**************************************************************************/
#ifndef __NETSERVER_H
#define __NETSERVER_H

//#include <config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "CF_CLogger.h"
#include <string>
#include <iostream>  
#include <strstream> 
#include <fstream>
#include "CF_CMemFileIO.h"
#include "CF_Cerrcode.h"
#include "CF_CLogger.h"
#include "CF_Common.h"
#include "tinyxml.h"
#include "tinystr.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "es/util/StringUtil.h"
#include "psutil.h"
//#include <Common.h>

class CNetServer{
public:
	CNetServer();
	~CNetServer();

	//初始化并建立监听
	int InitAndListen(int iPort);	
	int readFromNet( int iSock, char *cBuf, int iLength );
	int skip_to(TiXmlElement** dst, const char* name, const TiXmlElement* src);
	bool RechargeXmlToTree(char *pch_buff);
	bool WriteFile(string & result,int &num);
	
	int		m_serverSocketDescription;
   // int		m_iLoad;//线程间传递数据，数据装载完成标志
	int		iClientSock[100];

};

#endif
