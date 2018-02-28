#include "NetServer.h"

struct INITSTRU{
	int		iTemSock;
	CNetServer	*netServer;
	int		iFlag;
};


CNetServer::CNetServer(){
	for ( int i=0 ; i<100 ; i++ ){
		iClientSock[i] = 0;	
	}
	//m_iLoad = 0;
	m_serverSocketDescription = 0;
}

CNetServer::~CNetServer(){
	for ( int i=0 ; i<100 ; i++ ){
		if ( iClientSock[i] == 1 ) close( i ); 
	}
}

char* DEALClient(INITSTRU* s_des){	
	INITSTRU	*initStru = ( INITSTRU* )s_des;
	int		temp_sock_descriptor = initStru->iTemSock;
	initStru->iFlag = 1;

	CNetServer	*netServer = initStru->netServer;	
	char		cPackageLength[10000];//包长度 
	
	//while(1){
		/* 初始化 */
		memset(cPackageLength,0,sizeof(cPackageLength));

		/* 将包中的内容读 到变量cPackageLength 中*/
		if ( 1 != netServer->readFromNet( temp_sock_descriptor, cPackageLength, sizeof(cPackageLength) ) ) {	
			theJSLog << "读取包的内容失败!" << endi;	
			close(temp_sock_descriptor);
			netServer->iClientSock[ temp_sock_descriptor ] = 0;
			//break ;
		}	
		//netServer->readFromNet( temp_sock_descriptor, cPackageLength, sizeof(cPackageLength) )
		sleep( 1 );
		//cout << "cPackageLength = " << cPackageLength <<endl;
		
	//}
	
	close(temp_sock_descriptor);
	netServer->iClientSock[ temp_sock_descriptor ] = 0;
	//cout << "cPackageLength = " << cPackageLength <<endl;
	return cPackageLength;
}

int CNetServer::InitAndListen(int iPort){
	//建套接口//AF_INET--ARPA网际协议//SOCK_STREAM可靠的顺序的双向连接
	m_serverSocketDescription = socket(AF_INET, SOCK_STREAM, 0);
	if(m_serverSocketDescription == -1){		
		theJSLog <<"Socket创建失败!" <<endi;		
		return -1;
	}
	
	//将一个进程和一个套接口联系起来
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(iPort);
	//m_iPort = iPort;
	
	/* 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间 */ 
    //监听端口不能用的，只能减少，不能杜绝
	int n = 1;
	setsockopt(m_serverSocketDescription,SOL_SOCKET,SO_REUSEADDR,&n,sizeof(int)); 

	//绑定IP
	if(bind(m_serverSocketDescription, (struct sockaddr*)&sin, sizeof(sin))){
		//printf("服务器监听端口绑定错误!\n");		
		theJSLog <<"服务器监听端口绑定错误!" <<endi;	
		return -1;
	}
	//监听接入套接口连接//LISTENLEN是客户的最大值	
	if(listen(m_serverSocketDescription, 20) == -1){
		//printf("监听失败！\n");	
		theJSLog <<"监听失败！" <<endi;	
		return -1;
	}
	//循环等待接收
	struct		sockaddr_in pin;
	int		address_size = 0;
	//char		*cClientIp;
	address_size = sizeof(struct sockaddr_in);
	 
	//接收客户端。启动客户端对话线程
	int temp_sock_descriptor = 0;
	INITSTRU		initStru;
	initStru.netServer = this;
	initStru.iFlag = 0;
	while(1){
		//printf( "waiting...\n" );
		//theLog <<"等待中..." <<endw;
   // cout << "accept " <<endl;
#ifdef _LINUX_SERVER_                                
	temp_sock_descriptor = accept(m_serverSocketDescription, (struct sockaddr*)&pin, ( socklen_t * )&address_size);
//#ifdef _HP_SERVER_
//#else
//	temp_sock_descriptor = accept(m_serverSocketDescription, (struct sockaddr*)&pin, ( int * )&address_size);
#else
	temp_sock_descriptor = accept(m_serverSocketDescription, (struct sockaddr*)&pin, ( int * )&address_size);
//	temp_sock_descriptor = accept(m_serverSocketDescription, (struct sockaddr*)&pin, ( unsigned long * )&address_size);
#endif
		if(temp_sock_descriptor == -1){	
			theJSLog <<"客户端连接服务器失败！" <<endi;	
			close(temp_sock_descriptor);
			iClientSock[ temp_sock_descriptor ] = 0;
			continue;
		}
		
		iClientSock[ temp_sock_descriptor ] = 1;		
		
		initStru.iTemSock = temp_sock_descriptor;
		theJSLog <<"客户端连接服务器成功！" <<endi	;
		
		//char  *result;
		//memcpy(result,0,sizeof(result));
		//strcpy(result,DEALClient(&initStru));
		//cout << "result = " << DEALClient(&initStru)<<endl;
		RechargeXmlToTree( DEALClient(&initStru) );
		sleep( 1 );
		initStru.iFlag = 0;					
	}
	return 1;
}

int CNetServer::readFromNet( int iSock, char *cBuf, int iLength ) {
	char		cBuffer[10000];
	int iCurLen = iLength;
	int iRealLen = 0;
	while( 1 ) {
		memset( cBuffer, 0, 10000 );
		iRealLen = recv( iSock, cBuffer, iCurLen, 0);
		if ( 0 >= iRealLen ) {
			return iRealLen;
		} else {
			strcat( cBuf, cBuffer );
			if ( iRealLen < iCurLen ) {//不够长
				iCurLen -= iRealLen;
			} else {
				break;
			}
		}
	}
	return 1;
}

bool CNetServer::WriteFile(string & result,int &num)
{
    //从 核心参数中得到path         
    std::string filepath;
	if( !tpss::getKenelParam( "sidfile.filepath", filepath ) ) {
		tpss::writelog( 0, "获取filepath 失败" );
	}

   char szOutFile[300];
   char szMesFile[300];
   char filename[20];
   strcpy(szOutFile,filepath.c_str());
   
   strcpy(szMesFile,filepath.c_str());
   strcat(szMesFile,"mesfile/");
   
   ofstream outInfo;
   //cout << "num = " <<num<<endl;
   // HQ.SID.FROM.XXXX.AAA.BATCH.YYYYMMDDHHMISS.dat
   strcat(filename,"HQ.SID.FROM.3055.000.BATCH.");
   std::string filetime=Poco::DateTimeFormatter::format(Poco::LocalDateTime(),"%Y%m%d%H%M%S");   
   
   strcat(filename,filetime.c_str());
   strcat(filename,".dat");
   //cout << "filename = " << filename <<endl;  
   strcat(szOutFile,filename);
   strcat(szMesFile,filename);
   
   //cout << "szOutFile = " << szOutFile << "  szMesFile = " << szMesFile<<endl;
   outInfo.open(szOutFile); 
   std::string startDate=Poco::DateTimeFormatter::format(Poco::LocalDateTime(),"%Y%m%d%H%M%S");
   //cout << start <<endl;
   outInfo << "00/"<<num<<"/"<< startDate <<endl;
   outInfo<<result.c_str();
    std::string endDate=Poco::DateTimeFormatter::format(Poco::LocalDateTime(),"%Y%m%d%H%M%S");
   outInfo<<"99/"<<num<< "/"<< endDate <<endl;
   outInfo.close();

   // mesfile for trigger
   outInfo.open(szMesFile);
   outInfo.close();
   return true;
}

bool CNetServer::RechargeXmlToTree(char *pch_buff)
{
    int tablenum=0;
    int itemnum=0;
    
    string tablename;
    string opertype;
    string itemname;
    string key;
    string itemtype;
    string itemvalue;
    
    ostrstream oss;   
    
    TiXmlDocument ChargeDoc;
    TiXmlElement* p_root=NULL;
    TiXmlElement* p_attr=NULL;
    TiXmlElement* p_table=NULL;
    TiXmlElement* p_item=NULL;
  
    theJSLog<<"--------------------Start Change Xml To osstream---------------------"<<endi;
	theJSLog<<"The Buff is:\n"<<pch_buff<<endi;
	
	ChargeDoc.Parse(pch_buff);
	
	p_root = ChargeDoc.RootElement();
	
	TiXmlElement* p_messsage;
  if(skip_to(&p_messsage,"MSG",p_root))
  {
    theJSLog<<"----------------------Can't find MSG---------------------"<<endi;
  }
 
  for(p_table=p_messsage->FirstChildElement("TABLE");p_table;p_table=p_table->NextSiblingElement()) //table
  {
     tablenum++;
     if(strcmp(p_table->Value(),"TABLE")==0)
     { 
       //处理TABLE 下的内容
       // TABLENAME
         if((p_attr=p_table->FirstChildElement("TABLENAME"))!=NULL)//获取TABLENAME
         {
  	        if(p_attr->GetText()==NULL)
  	         {
               theJSLog<<"----------------------Can't find TABLENAME---------------------"<<endi;
  	         }
  	        else
  	        {
  	          tablename=p_attr->GetText();
  	        }
         }
        theJSLog<<"TABLENAME = "<<tablename<<endi;
        // OPERTYPE 
         if((p_attr=p_table->FirstChildElement("OPERTYPE"))!=NULL)//获取TABLENAME
         {
  	        if(p_attr->GetText()==NULL)
  	         {
               theJSLog<<"----------------------Can't find OPERTYPE---------------------"<<endi;
  	         }
  	        else
  	        {
  	          opertype= p_attr->GetText();
  	        }
         }
        theJSLog<<"OPERTYPE = "<<opertype<<endi;
        //01#SERV#!#serv_id :11:1236#!#state :02 :2HA#!#state_time :03 :2006-01-01 12 00 00   
        oss << opertype << "#" << tablename  ;
        // ITEM 多个值
        itemnum=0;
        for(p_item=p_table->FirstChildElement("ITEM");p_item;p_item=p_item->NextSiblingElement())
        {
          itemname = ( p_item->FirstChildElement("ITEMNAME") )->GetText();
          key = ( p_item->FirstChildElement("KEY") )->GetText();
          itemtype = ( p_item->FirstChildElement("ITEMTYPE") )->GetText();
          itemvalue = ( p_item->FirstChildElement("ITEMVALUE") )->GetText();
          itemnum++;
          cout << "itemname = " << itemname << endl;
          cout << "key = " << key << endl;
          cout << "itemtype = " << itemtype << endl;
          cout << "itemvalue = " << itemvalue << endl;
          oss << "#!#" <<itemname << ":" << key << itemtype << ":" << itemvalue ;
        }
        oss << "\r\n";
     }
     else {
     	theJSLog<<"----------------------Can't find TABLE---------------------"<<endi;
     }
  }
   oss <<ends;
   //cout << "all oss = " << oss.str() << endl;
  if(itemnum==0||tablenum==0)
  {
    theJSLog<<"----------------------There is no table or item in Xml---------------------"<< endi;
  }

   theJSLog<<"----------------------End Change Xml To osstream---------------------"<< endi;
  WriteFile( oss.str() ,tablenum);
  return  true;
}

int CNetServer::skip_to(TiXmlElement** dst, const char* name, const TiXmlElement* src)
{
	const TiXmlElement* ele = src;
	while(NULL != ele)
	{
		if(strcmp(ele->Value(), name) == 0)
		{
			*dst = (TiXmlElement*)ele;
			return 0;
		}
		ele = ele->NextSiblingElement();
	}
	return -1;
}
