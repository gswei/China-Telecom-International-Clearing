/**************************************************************************
* �ļ�����	main.cpp
* ���ܣ�	ǰ��̨ͨѶ�ӿڵ�������
//20070614 ���ӽӿ�����S ,��������kill����
//20080723 ���ӽӿ�����R,���ļ�����
//20080805 	LogicManager.cpp ����359��delSpace(cExe,0);
//20090710  R,r,C�ӿ�
**************************************************************************/
//#include <Common.h>
#include <sys/utsname.h>
#include <iostream>  
#include <strstream> 
#include <fstream>
#include <string> 
#include "time.h"
#include "psutil.h"
#include "bill_process.h"
#include "CF_CMemFileIO.h"
#include "CF_Cerrcode.h"
#include "CF_CLogger.h"
#include "CF_Common.h"
#include "es/util/StringUtil.h"
#include "main.h"

using namespace std; 
using namespace tpss;
CLog theJSLog;

CNetServer::CNetServer(){
}

CNetServer::~CNetServer(){
}
int main(int argc, char **argv){	
	// log
	CNetServer		netServer;
	netServer.printVersion();
	//���Ĳ�����ȡ
	std::string log_path,log_level;
	char* sid_file;
	char szLogPath[1024];
	int szLogLevel;	
	
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		//theJSLog.writeLog(GET_LOGPATH_ERR,"�Ӻ��Ĳ�����ȡ��־·������");
		cout << "�Ӻ��Ĳ�����ȡ��־·������" <<endl;
	}
	if( !tpss::getKenelParam( "log.level", log_level ) ) {
		//theJSLog.writeLog(GET_LOGLEVEL_ERR,"�Ӻ��Ĳ�����ȡ��־�������");
		cout << "�Ӻ��Ĳ�����ȡ��־·������" <<endl;
	}
	int level_no = es::StringUtil::toInt( log_level );
	strcpy(szLogPath,log_path.c_str());
	szLogLevel =  level_no;	
	theJSLog.setLog(szLogPath, szLogLevel, "SID", "SID", 1);

	//��ָ��Ŀ¼��ȡ����
	/*if( !tpss::getKenelParam( "sidfile.inpath", sid_file ) ) {
		//tpss::writelog( 0, "��ȡ��־·��ʧ��" );
		//theJSLog.writeLog(GET_LOGPATH_ERR,"�Ӻ��Ĳ�����ȡ��־·������");
		cout << "�Ӻ��Ĳ�����ȡ��־·������" <<endl;
	}*/
	sid_file = getenv("SID_PATH");
	sprintf(netServer.szSidinPath,"%sin/",sid_file);
	theJSLog << "sid �ļ�����·��" << netServer.szSidinPath <<endd;
	
	while(1)
	{      
       netServer.getBuffer();
       sleep(100);
    }    	

}

void CNetServer::printVersion()
{
	/* ���ģ������������ơ��汾����Ϣ */
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    jssidfile               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2013-11-03 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;
}

bool CNetServer::getBuffer()
{
	if(scan.openDir(szSidinPath))
	 {
	     theJSLog << "�򿪻����ļ�Ŀ¼ʧ��" << szSidinPath << endi;
	     return ;	
	  }
	//ѭ����ȡĿ¼��ɨ���ļ��У���ȡ�ļ�
	int rett = -1 ;
	int counter = 0;
	char fileName[1024];
    char tmp[512];   //��ʱ�ļ�
	char m_szFileName[1024];  //ʵ���ļ���
	char m_szinFilePath[1024];
	char m_szFileBakpath[1024];  //����·��
	char *p;
	char sz_buff[10000];

	sprintf(m_szFileBakpath,"%sbak/",szSidinPath);
	//theJSLog << "���ݱ���Ŀ¼Ϊ" << m_szFileBakpath <<endd;
	
	while(1) 
	{	
		memset(fileName,0,sizeof(fileName));
		rett = scan.getFile("*xml",fileName);  			
	    //theJSLog << "fileName = " << fileName << endi;  //fileName��·���ļ���
	    
		if(rett == 0)
		{
		    p = strrchr(fileName,'/');
			memset(m_szFileName,0,sizeof(m_szFileName));
		    strcpy(m_szFileName,p+1);   //�����ļ���,ȥ·��		
		    cout<<"ɨ�赽�ļ���"<<fileName<<endl;

            int iFd=open(fileName,O_RDONLY);
           if(iFd==-1)
           {
    	      cout<<"Can't Open the file "<<fileName<<endl;
    	      return false;
           }
           sz_buff[0]='\0';
           int nsize;
           if((nsize=read(iFd,sz_buff,8096))==-1)
           {    	
       	      cout<<"Read Xml Err!"<<endl;
           }
           sz_buff[nsize]='\0';
           close(iFd);
           //theJSLog <<"sz_buff = "<<sz_buff<< endl;
           RechargeXmlToTree(sz_buff);
    		
		    memset(tmp,0,sizeof(tmp));
		    strcpy(tmp,fileName);
		    strcat(m_szFileBakpath,m_szFileName);
		    if(rename(fileName,m_szFileBakpath) != 0) //�ļ���������ʱ�ļ�
		    {
			   cout<<"�ļ�����ʧ��!"<<m_szFileBakpath<<endl;
			   continue ;
		    }
		}
        else if(rett == 100)
        {
           break ;	
        }
		else if(rett == -1) 			//��ȡ�ļ���Ϣʧ��
		{
			break;
		}		
	}//while(1)
	scan.closeDir();
}

bool CNetServer::WriteFile(string & result,int &num)
{
    //�� ���Ĳ����еõ�path         
    /*std::string filepath;
	if( !tpss::getKenelParam( "sidfile.outpath", filepath ) ) {
		tpss::writelog( 0, "��ȡfilepath ʧ��" );
	}*/
	char *tmppath;
	char filepath[500];
	tmppath = getenv("SID_PATH");
	sprintf(filepath,"%sout/",tmppath);
	theJSLog << "filepath = "<< filepath << endd;

   char szOutFile[300]; 
   char szMesFile[300];  //�������ļ���
   char filename[20];  //ʵ�������ļ�
   strcpy(szOutFile,filepath);
   
   strcpy(szMesFile,filepath);
   strcat(szMesFile,"mesfile/");
   
   ofstream outInfo;
   cout << "record num = " <<num<<endl;
   //cout << "result = " <<result<<endl;
   // HQ.SID.FROM.XXXX.AAA.BATCH.YYYYMMDDHHMISS.dat
   strcat(filename,"HQ.SID.FROM.3055.000.BATCH.");
   std::string filetime=Poco::DateTimeFormatter::format(Poco::LocalDateTime(),"%Y%m%d%H%M%S");   
   
   strcat(filename,filetime.c_str());
   strcat(filename,".dat");
   theJSLog << "filename = " << filename <<endl;  
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
