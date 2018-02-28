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
	char		cPackageLength[10000];//������ 
	
	//while(1){
		/* ��ʼ�� */
		memset(cPackageLength,0,sizeof(cPackageLength));

		/* �����е����ݶ� ������cPackageLength ��*/
		if ( 1 != netServer->readFromNet( temp_sock_descriptor, cPackageLength, sizeof(cPackageLength) ) ) {	
			theJSLog << "��ȡ��������ʧ��!" << endi;	
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
	//���׽ӿ�//AF_INET--ARPA����Э��//SOCK_STREAM�ɿ���˳���˫������
	m_serverSocketDescription = socket(AF_INET, SOCK_STREAM, 0);
	if(m_serverSocketDescription == -1){		
		theJSLog <<"Socket����ʧ��!" <<endi;		
		return -1;
	}
	
	//��һ�����̺�һ���׽ӿ���ϵ����
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(iPort);
	//m_iPort = iPort;
	
	/* �����������ֹ��,���������Եڶ��ο������������õȴ�һ��ʱ�� */ 
    //�����˿ڲ����õģ�ֻ�ܼ��٣����ܶž�
	int n = 1;
	setsockopt(m_serverSocketDescription,SOL_SOCKET,SO_REUSEADDR,&n,sizeof(int)); 

	//��IP
	if(bind(m_serverSocketDescription, (struct sockaddr*)&sin, sizeof(sin))){
		//printf("�����������˿ڰ󶨴���!\n");		
		theJSLog <<"�����������˿ڰ󶨴���!" <<endi;	
		return -1;
	}
	//���������׽ӿ�����//LISTENLEN�ǿͻ������ֵ	
	if(listen(m_serverSocketDescription, 20) == -1){
		//printf("����ʧ�ܣ�\n");	
		theJSLog <<"����ʧ�ܣ�" <<endi;	
		return -1;
	}
	//ѭ���ȴ�����
	struct		sockaddr_in pin;
	int		address_size = 0;
	//char		*cClientIp;
	address_size = sizeof(struct sockaddr_in);
	 
	//���տͻ��ˡ������ͻ��˶Ի��߳�
	int temp_sock_descriptor = 0;
	INITSTRU		initStru;
	initStru.netServer = this;
	initStru.iFlag = 0;
	while(1){
		//printf( "waiting...\n" );
		//theLog <<"�ȴ���..." <<endw;
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
			theJSLog <<"�ͻ������ӷ�����ʧ�ܣ�" <<endi;	
			close(temp_sock_descriptor);
			iClientSock[ temp_sock_descriptor ] = 0;
			continue;
		}
		
		iClientSock[ temp_sock_descriptor ] = 1;		
		
		initStru.iTemSock = temp_sock_descriptor;
		theJSLog <<"�ͻ������ӷ������ɹ���" <<endi	;
		
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
			if ( iRealLen < iCurLen ) {//������
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
    //�� ���Ĳ����еõ�path         
    std::string filepath;
	if( !tpss::getKenelParam( "sidfile.filepath", filepath ) ) {
		tpss::writelog( 0, "��ȡfilepath ʧ��" );
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
       //����TABLE �µ�����
       // TABLENAME
         if((p_attr=p_table->FirstChildElement("TABLENAME"))!=NULL)//��ȡTABLENAME
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
         if((p_attr=p_table->FirstChildElement("OPERTYPE"))!=NULL)//��ȡTABLENAME
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
        // ITEM ���ֵ
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
