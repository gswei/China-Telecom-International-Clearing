/***********************************
��	����		CFTPmanager
��	;��		��FTP�������ϵ��ļ����У��ϴ������أ��õ��ļ��б�����Զ���ļ����ļ��С�
��	�ߣ�		��	��
version:    1.0.3.1
update history��
by yangzx 2006.05.16 �޸�List�ӿ�֧�ֻ�ȡԶ��dir�µ������ļ��б������Ŀ¼�µ��ļ�������private method ScanFile()
by yangzx 2006.05.20 ���ӽӿ�SizeFile()ȡ�ļ���С��	Ϊ��֤��ɳ���ļ����ԣ�����ListAllFile�ӿڣ���֧�ֻ�ȡԶ��dir�µ������ļ��б������Ŀ¼�µ��ļ���int ListAllFile( const char *path, const char *listSavePathName, char *errMsg );ԭ�е�List�ӿڹ��ܲ��䣬ֻȡ��ǰĿ¼�µ��ļ�����ȡ��Ŀ¼�µ��ļ���
by yangzx 2006.05.21 ʹ��select�������block����																	
by yangzx 2006.06.01 ��Ϊ��hp�Ļ�����ʹ��select���������⣬��ʱע�͵�ʹ��select�����Ĳ��֣��ָ���ǰ��upload��download����
by yangzx 2006.06.09 12:00 �޸�ListBy��bug��ע�͵�����ʹ�õ�select�����Ĳ���
by yangzx 2006.06.12 ʹ�� poll() �������� select()������select()������hp�Ļ����ϱ��������ʱ���ȶ�
by yangzx 2006.07.06 �޸�ListAllFile()��ScanFile()�������޸����������Ϣ,�޸�DivFileName( listSavePathName,filePath, fileName )�����ڿ���·����filePath��bug��filePath��ʱ��û���ַ�����ֹ�� 0
by yangzx 2006.07.31 �޸�ftpcmd()��read()��write()ǰ��ʹ��IsReadyToRead()��IsReadyToWrite()��װ
by yangzx 2006.08.05 �ں���CFTPmanager()������signal(SIGPIPE,SIG_IGN)
by yangzx 2006.08.10 ʹ�ý��з�������װ��Accept()����ֱ�ӵ���accept()
by yangzx 2006.08.30 ���Connect()��������ftp�û�������������ʱ��û�����m_ftpioָ���bug,��shutdown()����֮��Ҫ����close()��shutdown()����ֻ�ǹر����ӣ������汾��Ϊ1.0.2
by yangzx 2006.09.20 �ڵ���select() �� poll()ʱ���ϵͳ����real time signal ��ʹ���������أ��������errno=EINTR;�޸Ĵ����ڵ���select()��poll()ʱ������EINTR����
by yangzx 2006.10.11 debug DivFileName()�����ļ��ڸ�·��ʱ����/test.dat���ú����޷���ȷ�ֽ���ļ���·�����ļ������汾����Ϊ1.0.3.1���³��������л���ݸ132.119.200.60��ʹ��,1.0.5ֻ�ڼƷѲ������·��ӿڻ���ʹ�ù����д�ȫ����ԡ�
by zhanggq 2007.4.19 �޸ĳ�ʱʱ�䣬��60S��Ϊ300S
by zhanggq 2008.1.10 ����SizeFile()����WINOWSĿ¼��֧�֡�
by zhanggq 2008.4.09 ����SizeFile()���Ӷ�226����ֵ�÷��С�.
by zhanggq 2008.5.13 ���Ӽ���ļ��Ƿ���ڵĺ���
by zhanggq 2008.5.20 SIZEFILE���ӶԱ���ģʽ��֧�֡�
*************************************/


#include "FTPmanager.h"
#include "Common.h"

CFTPmanager::CFTPmanager()
{
	m_ftpio = NULL; 
	isockftp = -1;   
	temp_num=0;
	filelist=NULL;
	signal(SIGPIPE,SIG_IGN);
}       
        
CFTPmanager::~CFTPmanager()
{
	
}

int CFTPmanager::ShowVer()
{
  printf("*************************************************** \n");
	printf("*   FTP  Manager       \n");
	printf("*	Version Standard  1.0.1       		   \n");
	printf("* last updated: 2008-05-20 by zhangguoqiang  \n");
	printf("*		 \n");
	printf("*						   \n");
	printf("***************************************************\n");
	return 0;
}

/***************************************************
description:	����FTP������
input:	                 
		sz_serverIp;			FTP������IP              
		sz_loginName;			FTP��������¼��          
		sz_loginPassword;	FTP��������¼����		
output:         
		errMsg��        		������Ϣ
return:         
		1��				�ɹ�
		������				�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	��	��
date:		2005-08-16
*****************************************************/
int CFTPmanager::Connect(const char *pch_serverIp,const char *pch_loginName, const char *pch_loginPassword, char *errMsg,char c_PsvFlag,int iPort)
{
	//�����������Ϣ����Ա����
  strcpy(sz_serverIp,pch_serverIp);
  strcpy(sz_loginName,pch_loginName);
	strcpy(sz_loginPassword,pch_loginPassword);
	iServerPort=iPort;
  c_PassiveFlag=c_PsvFlag;
	//��ʼ������ǰҪ��ָ��FILE * m_ftpio=NULL,

	// updated by wh 2008-09-22,
	// ���FILE * m_ftpio��Ϊ�գ�˵��m_ftpio��ǰ�ɴ򿪹����������ͷž��
	if (m_ftpio !=NULL)
	{
		fclose(m_ftpio);
		m_ftpio=NULL;
   }

	//��ʼ����
	struct sockaddr_in s_sockaddr; 
	unsigned long iHostIp; 
	int	len, tmp, res; 
	int	retval = 0; 
	int	iSaveFd; 
	struct hostent *p_hosttent; 
	char sz_returnBuf[1024];
	
	iHostIp=inet_addr(pch_serverIp); 
	
	//��IPת����long��
	if(iHostIp==-1)
	{
		p_hosttent=gethostbyname(pch_serverIp); 
		if(p_hosttent==NULL)
		{
	    sprintf(errMsg,"Fail to get server IP from the name of '%s'!",pch_serverIp);  
			return GET_SERVER_NAME_ERR; 
		}
		iHostIp=*(unsigned long *)p_hosttent->h_addr; 
	}
	//cout<<"start socket"<<endl;
	//��������
	isockftp = socket(AF_INET,SOCK_STREAM,0); 
	//cout<<"end socket"<<endl;
	if (isockftp ==-1) 
	{
		sprintf(errMsg, "Fail to build socket,errno=%d",errno); 
		return CREATE_SOCK_ERR; 
	}
		
	s_sockaddr.sin_family = PF_INET; 
	s_sockaddr.sin_port = htons(iPort); 
	s_sockaddr.sin_addr.s_addr = iHostIp; 
  
  //cout<<"start connect"<<endl;

	if(connect(isockftp,(struct sockaddr *)&s_sockaddr,sizeof(s_sockaddr))==-1)
	{
		close(isockftp);
	  sprintf(errMsg,"Fail to build connection by socket,errno=%d",errno); 
    return SOCK_CONNECT_ERR;
	}
	//cout<<"end connect"<<endl;
	int iRet=0;
	
  // added by wh 2008-09-22
	m_ftpio=fdopen(isockftp,"r"); 
  if(m_ftpio==NULL)
  {
		close(isockftp);
	  sprintf( errMsg, "fdopen FILE* m_ftpio fail when connecting by socket,errno=%d \n",errno); 
	  return SOCK_CONNECT_ERR;
	}
	// end 2008-09-22
	
	
	//���Ϳ��ִ�
	memset(sz_returnBuf,0,1024);
	
	res=ftpcmd(sz_returnBuf,NULL); 
	
	if (res!=1)
	{
		shutdown( isockftp,SHUT_RDWR); 
		close(isockftp);
		
		// added by wh 2008-09-22
		fclose(m_ftpio);
		m_ftpio=NULL;
		// end 2008-09-22

		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
		}
		else
		{
			strcpy(errMsg,"Fail to get get message from server!\n"); 
		}
		return RECIEVE_SERVER_MSG_ERR;
	}
		
	//���͵�¼��
	memset(sz_returnBuf,0,1024);
	
	res = ftpcmd(sz_returnBuf,(char *)"USER %s",pch_loginName); 

	if (res != 1) 
	{
		m_ftpio=NULL;
		
		shutdown( isockftp, SHUT_RDWR ); 
		iRet=close(isockftp); 
		if(iRet==-1)
		{
			sleep(10);
			close(isockftp);
		}
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
		}
		else
		{
	    strcpy( errMsg, "Fail to get message from server!\n" ); 
	  }

  		return RECIEVE_SERVER_MSG_ERR;
	} 
		 
		
	//���͵�¼����
	memset( sz_returnBuf, 0, 1024 );
	res = ftpcmd(sz_returnBuf,(char *)"PASS %s",pch_loginPassword); 
	if (res != 1) 
	{
		m_ftpio=NULL;
		shutdown( isockftp, SHUT_RDWR ); 
		iRet=close(isockftp);
		if(iRet==-1)
		{
			sleep(10);
			close( isockftp );
		}
		
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
		}
		else
		{
			strcpy( errMsg, "Fail to get message from server!\n" ); 
		}

		//strcpy( errMsg, "���ܷ�����������Ϣʧ��!\n" );
		return RECIEVE_SERVER_MSG_ERR;
	}
	
	res = IsCmdOk( sz_returnBuf, "230" );
	
	//����¼�Ƿ�ɹ�
	if ( res != 1 )
	{
		shutdown( isockftp, SHUT_RDWR ); 
		close( isockftp );
		m_ftpio=NULL;

		sprintf( errMsg, "Fail to login: wrong username '%s' or password '%s'!\n", pch_loginName, pch_loginPassword ); 
		return LOGIN_ERR;
	}
/*
	if(c_PassiveFlag=='Y')
	{
	  memset(sz_returnBuf,0,1024);
	  res = ftpcmd(sz_returnBuf,"PASV"); 
	  if(res!=1) 
	  { 
	  	shutdown(isockftp,SHUT_RDWR); 
	  	m_ftpio=NULL;
	  	close(isockftp);
	  	if(res==-1)
	  	{
	  		strcpy(errMsg,sz_errorMessage);
	  	}
	  	else
	  	{
	  		strcpy( errMsg, "Fail to get message from server!\n" ); 
	  	}
	  	return RECIEVE_SERVER_MSG_ERR;
	  }
	}


	//�趨ģʽ
	memset( sz_returnBuf, 0, 1024 );
	res = ftpcmd(sz_returnBuf,"TYPE A"); 
	if(res!=1) 
	{ 
		shutdown(isockftp,SHUT_RDWR); 
		m_ftpio=NULL;
		close(isockftp);
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
		}
		else
		{
			strcpy( errMsg, "Fail to get message from server!\n" ); 
		}
		return RECIEVE_SERVER_MSG_ERR;
	}
  */
	return 1;
}

/***************************************************
description:	����FTP������
input:	        
		serverInfo:		��������Ϣ�ṹ��
output:		           
		errMsg��		������Ϣ                 
return:         	
		��Connect����һ���غ�����ͬ
programmer:	
		��	��
date:		
		2005-08-30
*****************************************************/ 
int CFTPmanager::Connect( SERVER_INFO serverInfo, char *errMsg )
{
	return ( Connect( serverInfo.cIp, serverInfo.cUser, serverInfo.cPassword, errMsg,'N',iServerPort) );
}

int CFTPmanager::Connect(const char *pch_serverIp,const char *pch_loginName, const char *pch_loginPassword, char *errMsg,int iPort)
{
	return (Connect(pch_serverIp,pch_loginName,pch_loginPassword,errMsg,'N',iPort));
}

	
/***************************************************
description:	�Ͽ���FTP������������
input:	        
output:         
return:         
		0��		ʧ��
		1��		�ɹ�
programmer:	��	��
date:		2005-08-16
*****************************************************/
int CFTPmanager::Disconnect()
{
	shutdown(isockftp,SHUT_RDWR);	
	int iRet=0;
	iRet=close(isockftp);
	if(iRet==-1)
	{
		sleep(10);
		close(isockftp);
	}
	
	if(m_ftpio)
	{ 
		fclose(m_ftpio); 
		m_ftpio=NULL; 
	} 
	return 1;
}

/***************************************************
description:	�Զ������ӷ�����
input:	        
output:         
return:         
		1��			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	��	��
date:		2005-08-16
*****************************************************/
int CFTPmanager::Reconnect(char *errMsg)
{
	Disconnect();
	for (int i=0;i<3;i++)
	{
		sleep(10);
		int		iRet=0;
		iRet=Connect(sz_serverIp,sz_loginName,sz_loginPassword,errMsg,c_PassiveFlag,iServerPort);
		if (1==iRet)
		{
			return 1;
		}
	}

	return 0;
}

/***************************************************
description:	�õ�FTP��������ָ��Ŀ¼(path����ָ��·����)���ļ��б�,�����listSavePathNameָ�����ļ���
input:	        
		path:			FTP��������Ҫȡ���ļ��б��·��
		listSavePathName:	ȡ�õ��ļ��б����ڱ����ļ��У��˲�����Ϊ���ļ���·��+�ļ���
output:        
		errMsg��		������Ϣ
return:         
		1��			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	��	��
date:		2005-08-16
update by yangzx solve block with poll() 2006.06.13
*****************************************************/ 
int CFTPmanager::List(const char *pch_dataPath,const char *pch_listFile,char *errMsg)
{
	int iRet=-1;
	int iLen=-1;
	char sz_buff[1024];
	struct sockaddr_in s_sockaddr; 
	unsigned char	*c, *p; 

CHECK_AGAIN:	
	//ɾ��ԭ���ļ��б�
	remove(pch_listFile);

	memset(sz_buff,0,1024); 

	//�������ݽ���socket
	int iSockData=socket(AF_INET,SOCK_STREAM,0);
	if(iSockData==-1)
	{
		close(iSockData);
	  strcpy(errMsg,"Fail to build socket for receiving data!\n");
		return CREATE_LISTENING_SOCK_ERR;
	}

	int iTmp=sizeof(s_sockaddr);
	_getsockname(isockftp,(struct sockaddr *)&s_sockaddr,&iTmp);
	s_sockaddr.sin_port=0;

	if(bind(iSockData,(struct sockaddr *)&s_sockaddr,sizeof(s_sockaddr))==-1)
	{
		close(iSockData); 
	  strcpy(errMsg, "Fail to bind port for socket!\n" );
		return LISTENING_SOCK_BIND_ERR;
	}

	if(listen(iSockData,1)== -1)
	{                                          
		close(iSockData);
	  strcpy(errMsg,"Fail to listen!\n");
		return SOCK_LISTENING_ERR;
	}

	iTmp=sizeof(s_sockaddr);
	_getsockname(iSockData,(struct sockaddr *)&s_sockaddr,&iTmp);
	c = (unsigned char *)&s_sockaddr.sin_addr;
	p = (unsigned char *)&s_sockaddr.sin_port;

	if(c_PassiveFlag=='Y')
	{
	  memset(sz_buff,0,1024);
	  iRet = ftpcmd(sz_buff,"PASV"); 
	  if(iRet!=1)
	  {
	  	close(iSockData);
	  	if(iRet==-1)
	  	{
	  		strcpy(errMsg,sz_errorMessage);
	  	}
	  	else
	  	{
	  		strcpy( errMsg, "Fail to get message from server!\n" ); 
	  	}
	  	return RECIEVE_SERVER_MSG_ERR;
	  }
	}

	memset(sz_buff,0,1024);
	iRet=ftpcmd(sz_buff,(char *)"PORT %d,%d,%d,%d,%d,%d",c[0],c[1],c[2],c[3],p[0],p[1]);
	if(iRet!=1)
	{//�����ѶϿ����ȴ�1������� 
		close(iSockData); 
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if(Reconnect(errMsg)==1)
		{
			goto CHECK_AGAIN;
		} 
		else 
		{
			strcpy( errMsg, "Connection was elimilated!\n" );			
			return SOCK_CONNECT_ERR;
		}
	}

	//����Ϣ��λ
	while(1)
	{
		if(IsCmdOk(sz_buff,"200 PORT" )==1)
		{
			break;
		}
		else
		{
			do
			{ 
				memset(sz_buff,0,1024);
				if (fgets(sz_buff,1024,m_ftpio)== NULL) 
				{
		      close(iSockData);
					return 0;
				}          
			}while(sz_buff[3] == '-'); 
		}
	}
			
	memset(sz_buff,0,1024);
	iRet=ftpcmd(sz_buff,(char *)"CWD %s",pch_dataPath);
	if(iRet==-1)
	{
		close(iSockData);
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	iRet=IsCmdOk(sz_buff,"250");
	if(iRet!=1)
	{
		close( iSockData );
		sprintf(errMsg,"Inexistant directory: %s!\n",pch_dataPath);
		return FOLDER_NOT_EXIST;
	}

	memset(sz_buff,0,1024);
	iRet=ftpcmd(sz_buff,(char *)"LIST"); // 150 is ok
	if(iRet==-1)
	{
		close(iSockData);
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	iRet=IsCmdOk(sz_buff,"150");
	if (iRet!=1)
	{
		close( iSockData );
		sprintf( errMsg, "Inexistant directory: %s!\n", pch_dataPath );		
		return FOLDER_NOT_EXIST;
	}
	
	iTmp = sizeof(s_sockaddr); 
	int m_sockxfer=Accept(iSockData,(struct sockaddr *)&s_sockaddr,&iTmp); 
	if (m_sockxfer==-1)
	{
		close(iSockData); 
		strcpy(errMsg,sz_errorMessage);
		return RECIEVE_SERVER_MSG_ERR;
	} 
	
	//���´����ļ��б�
	int iSaveFd=open(pch_listFile,O_WRONLY|O_CREAT,0644);
	if (iSaveFd == -1)
	{
		close(iSockData);
		close(m_sockxfer);
	  sprintf(errMsg, "Fail to create and open file: %s!\n",pch_listFile);
		return TARGET_FILE_CREATE_OPEN_ERR;
	}
	memset(sz_buff,0,1024);
	
	//begin 2006.06.13 ��ӣ���ֹblock
	int timeout=BLOCK_TIME_OUT_MSEC;
	pollfd spoll[3];
	spoll[0].fd=m_sockxfer;
	spoll[0].events=POLLRDNORM;
	while(1)
	{
    iRet=poll(spoll,1,timeout);
    if(iRet>0)
    {
      iLen=read(m_sockxfer,sz_buff,1024);
      if(iLen>0)
      {
        write(iSaveFd,sz_buff,iLen); 
		    memset(sz_buff,0,1024);
      }
      else
      {
      	break;
      }
    }
    else
    {
    	if(iRet==0)
    	{
    		strcpy(errMsg,"read block comes out!");
        close(iSaveFd);
				close(m_sockxfer);
				close(iSockData);
        return SOCKET_BLOCK_READ;
      }
      else
      {
      	//2006-9-20 19:17 added ����EINTR����
      	if(errno==EINTR)
        {
          continue;
        }
      	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
      	close(iSaveFd);
				close(m_sockxfer);
				close(iSockData);
      	return SOCKET_POLL_ERROR;;
      }
    }
	} 
	
	close(iSockData);
	close(m_sockxfer);
	close(iSaveFd);

	return 1;
}


/***************************************************
description:	��FTP�������������ļ�
input:	        
		localFile:		�ļ����غ��ڱ��صĴ��·��+�ļ���
		remoteFile:		FTP��������Ҫ�����ص�Դ�ļ�·��+�ļ���
		coverFlag:		���Զ�̻�����Ҫ�ϴ����ļ��Ѿ����ڣ��������ִ�������
						TARGET_COVER��		�����Ѿ��е��ļ�
						TARGET_STORE��		���Ѿ��е��ļ�������Ϊ��ԭ�ļ���.tmp��,Ȼ���������ļ�
		sendMode:		����ģʽ
						MODE_BIN:		����2��������
						MODE_ASC:		����10��������
		cStorePath:		������ɺ�,Դ�ļ���3�ִ���ʽ:1.������;2.ɾ��;3.���ݵ���һĿ¼
						SOURCE_IGNORE:		������
						SOURCE_DELE:		  ɾ��
						����:			        ����·��(�������ļ���)
output:         
		errMsg:			������Ϣ
return:         
		1��			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	��	��
date:		2005-08-16
*****************************************************/

int CFTPmanager::Download(const char *pch_localFile,const char *pch_remoteFile,int iCoverFlag, char *errMsg,
	int iSendMode, const char *cStorePath)
{
	int 	iRet=-1;
	int 	iLen=-1;
	char	sz_buff[1024], cRmFlag[1000];
	char	sz_sourcePath[255], sz_sourceName[255];
	char	sz_targetPath[255], sz_targetName[255];
	//char  sz_TmpPathFile[255];
  struct sockaddr_in s_sockaddr; 
	unsigned char *c, *p;
	
	memset(sz_sourcePath,0,255);
	memset(sz_sourceName,0,255);
	memset(sz_targetPath,0,255);
	memset(sz_targetName,0,255);
	memset(cRmFlag,0,255);
  
	strcpy(cRmFlag,cStorePath);	
	
	DivFileName(pch_remoteFile,sz_sourcePath,sz_sourceName);
	DivFileName(pch_localFile,sz_targetPath,sz_targetName);
	
	//memset(sz_TmpPathFile,0,255);
	//sprintf(sz_TmpPathFile,"%s/~%s",sz_targetPath,sz_targetName);
  memset(sz_buff,0,1024); 

CHECK_AGAIN:	

	//��鱾��Ŀ���ļ��Ƿ��Ѿ�����
	if(IsFileExist(pch_localFile)!=0)
	{
		if (iCoverFlag==TARGET_COVER)
		{//����
			unlink(pch_localFile);
		}
		else if(iCoverFlag==TARGET_STORE)
		{
			char sz_tmpFile[255];
			sprintf(sz_tmpFile,"%s.tmp",pch_localFile);
			rename( pch_localFile,sz_tmpFile);
		}
		else 
		{
	    sprintf( errMsg, "Unrecognized file cover mode: %d", iCoverFlag ); 
			return UNRECOGNIZED_COVER_MODE;
		}
	}

	//�������������Ŀ���ļ�·�������������½���
	chkAllDir(sz_targetPath);

	//�������ݽ���socket 
	int iSockData=socket(AF_INET,SOCK_STREAM,0);
	if (iSockData==-1)
	{
	  strcpy( errMsg, "Fail to build socket for receiving data!" );
		return CREATE_LISTENING_SOCK_ERR;
	}

	int iSizeSockAddr = sizeof(s_sockaddr);
	_getsockname(isockftp,(struct sockaddr *)&s_sockaddr,&iSizeSockAddr);
	s_sockaddr.sin_port = 0;

	if (bind(iSockData,(struct sockaddr *)&s_sockaddr,sizeof(s_sockaddr)) == -1)
	{
		close(iSockData);
	  strcpy(errMsg, "Fail to bind port for socket!" );  
		return LISTENING_SOCK_BIND_ERR;
	}

	if(listen(iSockData,1)==-1)
	{
		close(iSockData);
	  strcpy(errMsg, "Fail to listen!\n" );
		return SOCK_LISTENING_ERR; 
	}

	iSizeSockAddr=sizeof(s_sockaddr);
	_getsockname(iSockData,(struct sockaddr *)&s_sockaddr,&iSizeSockAddr);                      
	c = (unsigned char *)&s_sockaddr.sin_addr;
	p = (unsigned char *)&s_sockaddr.sin_port;
	memset(sz_buff,0,1024);
	iRet = ftpcmd(sz_buff,(char *)"PORT %d,%d,%d,%d,%d,%d",c[0],c[1],c[2],c[3],p[0],p[1]);         
	if (iRet != 1)
	{//�����ѶϿ����ȴ�1������� 
		close(iSockData); 
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if(Reconnect(errMsg)==1)
		{
			goto CHECK_AGAIN;
		}
		else
		{
	    strcpy( errMsg, "Connection was elimilated" ); 			
			return SOCK_CONNECT_ERR;
		}
	}

	while( 1 )//����Ϣ��λ
	{
		if(IsCmdOk(sz_buff,"200 PORT")==1)
		{
			break;
		}
		else
		{
			do
			{
				memset(sz_buff,0,1024);
				if (fgets(sz_buff,1024,m_ftpio)==NULL)
				{
					close(iSockData);
					return 0;
				}          
			}while(sz_buff[3] == '-'); //?
		}
	}

	//��FTP���������뵽Ŀ��·��
	if (sz_sourcePath!=NULL)
	{
		//�趨��ǰ·������Ŀ¼
		memset(sz_buff,0,1024);
		iRet=ftpcmd(sz_buff,(char *)"CWD %s",sz_sourcePath);  //250 is ok
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			close(iSockData);
			return -1;
		}
		iRet=IsCmdOk(sz_buff, "250" );
		if (1!=iRet)
		{
  	  sprintf(errMsg,"Inexistent Path: %s",sz_sourcePath);
			return SOURCE_FILE_NOT_EXIST;
		}
	}

	if (iSendMode==10)
	{//10���� A
		memset(sz_buff,0,1024);
		iRet = ftpcmd(sz_buff,(char *)"TYPE A");  
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			close(iSockData);
			return -1;
		}
	}
	else if (iSendMode==2)
	{//2���� I
		memset(sz_buff,0,1024);
		iRet=ftpcmd(sz_buff,(char *)"TYPE I"); 
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			close(iSockData);
			return -1;
		}
	}
	else
  {
	  close(iSockData);
	  sprintf(errMsg, "Unrecognized transform mode - %d\n",iSendMode);
		return UNRECOGNIZED_SENDING_MODE;	
	}

	memset(sz_buff,0,1024);
	iRet=ftpcmd(sz_buff,(char *)"RETR %s",sz_sourceName); //150 is ok     

	if(iRet==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		close(iSockData);
		return -1;
	}
	iRet=IsCmdOk(sz_buff,"150");
	if (1!=iRet)
	{
	  close(iSockData);
	  sprintf(errMsg, "Inexistent file: %s", sz_sourceName);
		return SOURCE_FILE_NOT_EXIST;
	}
	//�������ʱ�ļ����ڣ���ɾ��ԭ����ʱ�ļ�
	//unlink(sz_TmpPathFile);
	//���´����ļ��б�
	
	int iSaveFd;
	//iSaveFd=open(sz_TmpPathFile,O_WRONLY|O_CREAT,0644);    
	iSaveFd=open(pch_localFile,O_WRONLY|O_CREAT,0644);
	if (iSaveFd == -1)
	{
	  close(iSockData);
	  sprintf( errMsg, "Fail to create and open file: %s!", pch_localFile);
		return TARGET_FILE_CREATE_OPEN_ERR;
	}
   
  //if(my_lock(iSaveFd)==-1)
  //{
	//  sprintf( errMsg, "Fail to lock file: %s!", pch_localFile);
	//	close(iSockData);
  //  close(iSaveFd);
  //	return LOCK_FILE_ERR;
  //}
	//��������
	iSizeSockAddr=sizeof(s_sockaddr);
	
	int iSockxfer;
	iSockxfer=Accept(iSockData,(struct sockaddr *)&s_sockaddr,&iSizeSockAddr); 
	if (iSockxfer==-1)
	{
		//my_unlock(iSaveFd);
		close(iSockData);
    close(iSaveFd);
		strcpy(errMsg,sz_errorMessage);
		return RECIEVE_SERVER_MSG_ERR;
	} 

	memset(sz_buff,0,1024);
	int timeout=BLOCK_TIME_OUT_MSEC;
	pollfd spoll[3];
	spoll[0].fd=iSockxfer;
	spoll[0].events=POLLRDNORM;
	if(iSendMode== 10 )
	{//10���� A
		char sz_readBuff[1024+1];
		memset(sz_readBuff,0,1024+1);
		
		while(1)
		{
	    iRet=poll(spoll,1,timeout);
      if(iRet>0)
      {      			
      	iLen=read(iSockxfer,sz_readBuff,1024);
        if(iLen>0)
        {
        	write(iSaveFd,sz_readBuff,iLen); 
			  	memset(sz_readBuff,0,1024);
        }
        else
        {
        	break;
        }
      }
      else
      {
      	if(iRet==0)
      	{
      		strcpy(errMsg,"read block comes out!");
      	  //my_unlock(iSaveFd);
          close( iSaveFd);
					close( iSockxfer);
					close( iSockData );
          return SOCKET_BLOCK_READ;
        }
        else
        {
        	//2006-9-20 19:17 added ����EINTR����
      		if(errno==EINTR)
        	{
        		continue;
        	}
        	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
        	//my_unlock(iSaveFd);
        	close(iSaveFd);
					close(iSockxfer);
					close(iSockData);
        	return SOCKET_POLL_ERROR;;
        }
      }
		}
	}
	else if(iSendMode==2) 
	{//2���� I
		void *cBuf=malloc(1024);
		memset(cBuf,0,1024);

		while(1)
		{
	    iRet=poll(spoll,1,timeout);
      if(iRet>0)
      {
      	iLen=read(iSockxfer,cBuf,1024);
        if(iLen>0)
        {
        	write(iSaveFd,cBuf,iLen); 
			  	memset( cBuf, 0, 1024 );
        }
        else
        {
        	break;
        }
      }
      else
      {
      	if(iRet==0)
      	{
      		strcpy(errMsg,"read block comes out!");
      		//my_unlock(iSaveFd);
          close(iSaveFd);
					close(iSockxfer);
					close(iSockData);
					free(cBuf);
          return SOCKET_BLOCK_READ;
        }
        else
        {
        	//2006-9-20 19:17 added ����EINTR����
      		if(errno==EINTR)
        	{
        		continue;
        	}
        	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
        	//my_unlock(iSaveFd);
          close(iSaveFd);
					close(iSockxfer);
					close(iSockData);
					free( cBuf );
        	return SOCKET_POLL_ERROR;
        }
      }
		}
		free(cBuf);
	}

  close(iSaveFd);
  close(iSockxfer);
  close(iSockData);

	//������ɺ���ʱ�ļ�����Ϊ��ʽ�ļ���
	//if (rename(sz_TmpPathFile,pch_localFile)!=0)
	//{
	//	return RENAME_FILE_ERR;	
	//}
	
	//����Դ�ļ�
	if (strcmp(cRmFlag,SOURCE_IGNORE)==0)
	{//������
	}
	else if(strcmp(cRmFlag,SOURCE_DELE)==0)
	{
		iRet=RmFile(pch_remoteFile,errMsg);
		if(iRet!=1)
		{
			return DELETE_SOURCE_ERR;
		}
	}
	else
	{
		char sz_bakFile[1024];
		memset(sz_bakFile,0,1024);
		sprintf(sz_bakFile,"%s/%s",cRmFlag,sz_sourceName);
		iRet=MoveFile(pch_remoteFile,sz_bakFile,errMsg);
		if(iRet!=1)
		{
			if(iRet==-1)
			{
				return -1;
			}
			return STORE_SOURCE_ERR;
		}
	}
	return 1;
} 

/***************************************************
description:	��FTP�������������ļ�(��������/ѹ)
input:	        
		localFile:		�ļ����غ��ڱ��صĴ��·��+�ļ���
		remoteFile:		FTP��������Ҫ�����ص�Դ�ļ�·��+�ļ���!
		coverFlag:		���Զ�̻�����Ҫ�ϴ����ļ��Ѿ����ڣ��������ִ�������
						TARGET_COVER��		�����Ѿ��е��ļ�
						TARGET_STORE��		���Ѿ��е��ļ�������Ϊ��ԭ�ļ���.tmp��,Ȼ���������ļ�
		sendMode:		����ģʽ
						MODE_BIN:		����2��������
						MODE_ASC:		����10��������
		cStorePath:		������ɺ�,Դ�ļ���3�ִ���ʽ:1.������;2.ɾ��;3.���ݵ���һĿ¼
						SOURCE_IGNORE:������
						SOURCE_DELE:	ɾ��
						����:			����·��(�������ļ���)
		compress:		��������Ƿ��ѹ
						COMPRESS_Y:		Ҫ��ѹ
						COMPRESS_N:		����ѹ
		pass:		��������Ƿ����:
						PASS_N:			  ������
						������			  ����ʱ���ܳ�
output:
		errMsg:			������Ϣ
return:         
		1��			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
programmer:	��	��
date:		2005-08-16
*****************************************************/
int CFTPmanager::Download( const char *localFile, const char *remoteFile, int coverFlag, char *errMsg, 
			int sendMode, const char *cStorePath, const char *compress, const char *pass )
{
	int 		res = -1;
	int 		len = -1;
	char		buf[1024], cRmFlag[1000];
	char		sourcePath[1000], sourceFile[500];
	char		targetPath[1000], targetFile[500];
	
	memset(sourcePath,0,1000);
	memset(sourceFile,0,500);
	memset(targetPath,0,1000);
	memset(targetFile,0,500);
	memset(cRmFlag,0,1000);

	strcpy( cRmFlag, cStorePath );	

	DivFileName(remoteFile,sourcePath,sourceFile);
	DivFileName(localFile,targetPath,targetFile);

	//���ع����б���Ϊ��ʱ�ļ�
	char cTmpPathName[1024];
	memset(cTmpPathName,0,1024);
	sprintf(cTmpPathName,"%s/~%s",targetPath,targetFile);

	struct sockaddr_in addr; 
	unsigned char *c,*p; 
	
	memset(buf,0,1024); 

CHECK_AGAIN:	

	//��鱾��Ŀ���ļ��Ƿ��Ѿ�����
	if ( IsFileExist( localFile ) != 0 )
	{
		if ( coverFlag == TARGET_COVER )
		{//����
			remove( localFile );
		}
		else if ( coverFlag == TARGET_STORE )
		{
			char		cTem[1024];
			memset( cTem, 0, 1024 );
			sprintf( cTem, "%s.tmp", localFile );
			//printf( "cTem: %s\n", cTem );
			int q = rename( localFile, cTem );
		}
		else
		{
	    sprintf( errMsg, "Unrecognized file cover mode: %d", coverFlag ); 
			return UNRECOGNIZED_COVER_MODE;
		}	
	}


	//�������������Ŀ���ļ�·�������������½���
	chkAllDir( targetPath );

	//�������ݽ���socket 
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );


		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData ); 

	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                              
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
	if (listen(iSockData,1) == -1)  {                                          
		close( iSockData );   


	strcpy( errMsg, "Fail to listen!\n" );  

		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}

	tmp = sizeof(addr);
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port; 

	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",c[0],c[1],c[2],c[3],p[0],p[1]);         
	if (res != 1)  {//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated\n" ); 

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}
	}    
	
	//����Ϣ��λ
	while( 1 ) {
		if ( IsCmdOk( buf, "200 PORT" ) == 1 ) {
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
				{
					close(iSockData);
					return 0;
				}          
			}while(buf[3] == '-'); 
		}
	}
	
	//��FTP���������뵽Ŀ��·��
	if ( sourcePath != NULL ){
		//�趨��ǰ·������Ŀ¼
		memset( buf, 0, 1024 );
		res = ftpcmd(buf,(char *)"CWD %s", sourcePath);  //250 is ok
		if(res==-1)
		{
			close(iSockData);
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		res = IsCmdOk( buf, "250" );
		if ( 1 != res ) {
	
	sprintf( errMsg, "Inexistent file: %s!\n", remoteFile );

			return SOURCE_FILE_NOT_EXIST;
		}
	}

	if ( sendMode == 10 ) {//10���� A
		memset( buf, 0, 1024 );                                                                                     
		res = ftpcmd(buf,(char *)"TYPE A"); 
		if(res==-1)
		{
			close(iSockData);
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		//char *cBuf = ( char * )malloc( 1024 );
		//memset( cBuf, 0, 1024 );
	} else if ( sendMode == 2 ) {//2���� I
		memset( buf, 0, 1024 );
		res = ftpcmd(buf,(char *)"TYPE I");   
		if(res==-1)
		{
			close(iSockData);
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}

	}
	else
	{
		close( iSockData );
	  sprintf( errMsg, "Unrecognized transform mode - %d\n", sendMode );
		return UNRECOGNIZED_SENDING_MODE;	
	}

	memset( buf, 0, 1024 );                                                                                     
	res = ftpcmd(buf,(char *)"RETR %s",sourceFile); //150 is ok    
	if(res==-1)
	{
	  	close(iSockData);
	  	strcpy(errMsg,sz_errorMessage);
	  	return -1;
	} 
	res = IsCmdOk( buf, "150" );
	if ( 1 != res )
	{

	sprintf( errMsg, "Inexistent file: %s!\n", remoteFile );

		//sprintf( errMsg, "Զ�̻�����Ҫ���ص��ļ�%s������!\n", remoteFile );
		return SOURCE_FILE_NOT_EXIST;
	}

	//�������ʱ�ļ����ڣ���ɾ��ԭ����ʱ�ļ�
	remove( cTmpPathName );
	//���´����ļ��б�
	int iSaveFd = open(cTmpPathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		close( iSockData );
		close( iSaveFd );

	sprintf( errMsg, "Fail to create and open file: %s!\n", localFile );

		//sprintf( errMsg, "Ҫ�����ڱ��ص�Ŀ���ļ�%s�����ʹ�ʧ��!\n", localFile );
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}

	//��������
	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1)
	{
		close( iSockData );
		strcpy(errMsg,sz_errorMessage);
		return RECIEVE_SERVER_MSG_ERR;
	} 
	
	memset( buf, 0, 1024 );

	int timeout=BLOCK_TIME_OUT_MSEC;
	pollfd spoll[3];
	spoll[0].fd=m_sockxfer;
	spoll[0].events=POLLRDNORM;
	
	if(sendMode== 10 )
	{//10���� A
		char		cBuf[1024+1];
		memset( cBuf, 0, 1024+1 );

		while(1)
		{
       res=poll(spoll,1,timeout);
      if(res>0)
      {
      		len=read(m_sockxfer,cBuf,1024);
          if(len>0)
          {
          	write(iSaveFd,cBuf,len); 
			    	memset( cBuf, 0, 1024 );
          }
          else
          {
          	//exit_flag=0;
          	break;
          }
      }
      else
      {
      	if(res==0)
      	{
      		strcpy(errMsg,"read block comes out!");
          close( iSaveFd );
					close( m_sockxfer );
					close( iSockData );
          return SOCKET_BLOCK_READ;
        }
        else
        {
        	//2006-9-20 19:17 added ����EINTR����
      		if(errno==EINTR)
        	{
        		continue;
        	}
        	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
        	close( iSaveFd );
					close( m_sockxfer );
					close( iSockData );
        	return SOCKET_POLL_ERROR;
        }
      }
		}
	}
	else if( sendMode == 2 ) 
	{//2���� I
		void *cBuf = malloc( 1024 );
		memset( cBuf, 0, 1024 );

		while(1)
		{
	   res=poll(spoll,1,timeout);
      if(res>0)
      {
      		len=read(m_sockxfer,cBuf,1024);
          if(len>0)
          {
          	write(iSaveFd,cBuf,len); 
			    	memset( cBuf, 0, 1024 );
          }
          else
          {
          	break;
          }
      }
      else
      {
      	if(res==0)
      	{
      		strcpy(errMsg,"read block comes out!");
          close( iSaveFd );
					close( m_sockxfer );
					close( iSockData );
					free( cBuf );
          return SOCKET_BLOCK_READ;
        }
        else
        {
        	//2006-9-20 19:17 added ����EINTR����
      		if(errno==EINTR)
        	{
        		continue;
        	}
        	
        	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
        	close( iSaveFd );
					close( m_sockxfer );
					close( iSockData );
					free( cBuf );
        	return SOCKET_POLL_ERROR;
        }
      }
		}
		free( cBuf );
	}
	//solve block end
	
	close( iSaveFd );
	close( m_sockxfer );
	close( iSockData );
	
	char		cPreFile[300];
	memset( cPreFile, 0, 300 );
	sprintf( cPreFile, "%s_pre.tmp", cTmpPathName );
	
	CEncryptFile		encc;
	CF_CLzwCompressNoCatch	com;

	//��ѹ��
	if ( strcmp( compress, COMPRESS_Y ) == 0 ) {
		com.uncompress( cTmpPathName, cPreFile );
		remove( cTmpPathName );
	} else {
		rename( cTmpPathName, cPreFile );
	}
	
	//cPreFile�ǽ�ѹ������ļ�
	
	char		cPassFile[300];
	memset( cPassFile, 0, 300 );
	
	//��ȡ�������ļ���
	int iPos = 0;
	iPos=strrncspn( cTmpPathName, '.', 3 );
	//if(iPos<0) return 0;
	strncpy( cPassFile , cTmpPathName, iPos );
	
	//����
	if ( strcmp( pass, PASS_N ) != 0 ) {//������
		char		cPass[100];
		memset( cPass, 0, 100 );
		strcpy( cPass, pass );
		encc.unEncrypt( cPreFile, cPassFile, cPass );
		remove( cPreFile );
	} else {//û����	
		rename( cPassFile, cPreFile );
	}
	
	//cPassFile�ǽ�ѹ���ͽ��ܺ���ļ���,ͬʱҲ�Ѿ�ȥ�����ļ���С��������Ϣ,���������ļ���
	
	//�˶��ļ���С
	//��ȡ�ļ���С
	int		iFiSize = 0;
	if ( strcmp( pass, PASS_N ) != 0 ) {//������
		//��ȡ�ļ���С
		char		cFiSize[10];
		memset( cFiSize, 0, 10 );
		int iPosa;
		iPosa=strrncspn( cTmpPathName, '.', 3 );
		//if(iPos<0) return 0;
		char *aa = cTmpPathName;
		aa += iPosa + 1;
		int iPosb;
		iPosb=strrncspn( cTmpPathName, '.', 2 );
		strncpy( cFiSize, aa, iPosb - iPosa - 1 );
		iFiSize = atoi( cFiSize );
	} else {
		//��ȡ�ļ���С
		char		cFiSize[10];
		memset( cFiSize, 0, 10 );
		int iPosa;
		iPosa=strrncspn( cTmpPathName, '.', 2 );
		//if(iPos<0) return 0;
		char *aa = cTmpPathName;
		aa += iPosa;
		int iPosb;
		iPosb=strrncspn( cTmpPathName, '.', 1 );
		strncpy( cFiSize, aa, iPosb );
		iFiSize = atoi( cFiSize );
	}
	
	int iReSize = GetFileSize( cPassFile );
	
	//�Ƚ��ļ���С
	if ( iReSize != iFiSize ) {
		//theLog<<"�����ļ�"<<downloadPath<<"/"<<cParam8<<"ʧ��,ʧ��ԭ��:����ǰ���ļ���С����!"<<endi;
		remove( cPassFile );
		return FILE_SIZE_ERR;
	}	
	
	//������ɺ���ʱ�ļ�����Ϊ��ʽ�ļ���
	char		cName[100], cPath[300];
	memset( cName, 0, 100 );
	memset( cPath, 0, 300 );
	DivFileName( cPassFile, cPath, cName );
	char		*cReName = cName;
	cReName ++;
	char		cFFile[300];
	memset( cFFile, 0, 300 );
	sprintf( cFFile, "%s/%s", cPath, cReName );
	
	if ( rename( cPassFile, cFFile ) != 0 ){
		return RENAME_FILE_ERR;	
	}
	
	//����Դ�ļ�
	if ( strcmp( cRmFlag, SOURCE_IGNORE ) == 0 ){//������
	} else if ( strcmp( cRmFlag, SOURCE_DELE ) == 0 ){
		res = RmFile( remoteFile, errMsg );
		if ( res != 1 ) {
			return DELETE_SOURCE_ERR;
		}
	} else {
		char		cNewPathName[1024];
		memset( cNewPathName, 0, 1024 );
		sprintf( cNewPathName, "%s/%s", cRmFlag, sourceFile );
		res = MoveFile( remoteFile, cNewPathName, errMsg );
		if ( res != 1 ){
			if(res==-1)
			{
				return -1;
			}
			return STORE_SOURCE_ERR;
		}
	}
	
	
	return 1;
} 

/***************************************************
description:	��FTP�������������ļ�
input:	        
		sendInfo:		������ƽṹ��
output:		           
		errMsg��		������Ϣ                 
return:         	
		��Download����һ���غ�����ͬ
programmer:	
		��	��
date:		
		2005-08-30
*****************************************************/ 
int CFTPmanager::Download( SEND_INFO sendInfo, char *errMsg )
{
	return (Download(sendInfo.localFile,sendInfo.remoteFile,sendInfo.coverFlag,errMsg,sendInfo.sendMode,sendInfo.cStorePath));
}
/***************************************************
description:	�ϴ��ļ���FTP��������
input:	        
		pch_localFile:	Ҫ���ϴ�����Դ�ļ���·��+�ļ���
		pch_remoteFile:	�ļ��ϴ���FTP�������Ϻ��ڷ������ϱ����·��+�ļ���
		coverFlag:		  �ļ����Ǳ�־�����FTP�������ϸ��ļ��Ѵ��ڣ���
			TARGET_COVER�������Ѿ��е��ļ�
			TARGET_STORE�����Ѿ��е��ļ�������Ϊ��ԭ�ļ���.tmp��,Ȼ���������ļ�
		sendMode��		  �ļ�����ģʽ
			MODE_BIN��		2����
			MODE_ASC��		�ı�
		cStorePath:		  Դ�ļ��ı���·��
			SOURCE_IGNORE:������
			SOURCE_DELE��	ɾ��Դ�ļ�
			������			  �����ڸ�·����(��·������Ϊ����·��)
output: 
		errMsg��		������Ϣ        
return:         
		1:			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	��	��
date:		2005-08-16
*****************************************************/
int CFTPmanager::Upload(const char *pch_localFile,const char *pch_remoteFile,int iCoverFlag,char *errMsg,int iSendMode,const char *pch_storePath)
{
	int iRet=-1;
  int iLen=-1;
	char sz_buff[1024];
	char sz_sourceFile[255],sz_targetFile[255];
  char sz_sourcePath[255],sz_sourceName[255];
  char sz_targetPath[255],sz_targetName[255];//*cRmFlag;
  struct sockaddr_in s_sockaddr;
  
  memset(sz_sourcePath,0,255);
	memset(sz_sourceName,0,255);
	memset(sz_targetPath,0,255);
	memset(sz_targetName,0,255);
	
	strcpy(sz_sourceFile,pch_localFile);
	strcpy(sz_targetFile,pch_remoteFile);
	DivFileName(sz_sourceFile,sz_sourcePath,sz_sourceName);
	DivFileName(sz_targetFile,sz_targetPath,sz_targetName);
	
CHECK_AGAIN:

	unsigned char *c, *p; 
	
	memset(sz_buff,0,1024 ); 

	//��������������ļ��Ƿ����
	if(IsFileExist(sz_sourceFile)!=1)
	{
	  sprintf(errMsg,"Inexistent file: %s\n",sz_sourceFile);
		return SOURCE_FILE_NOT_EXIST;	
	}

	//����Ŀ���ļ��Ƿ���ڣ�����ͼ�����²���
	if(iCoverFlag==TARGET_COVER)//����
	{
		RmFile(pch_remoteFile,errMsg);
	} 
	else if(iCoverFlag==TARGET_STORE)//����
	{
		char sz_tmpBuff[1024];
		memset(sz_tmpBuff,0,1024);
		sprintf(sz_tmpBuff,"%s.tmp",sz_targetFile);
		iRet=MoveFile(sz_targetFile,sz_tmpBuff,errMsg);
		if(iRet!=1)
		{
			return iRet;
		}
	}
	else
	{
	  sprintf(errMsg, "Unrecognized file cover mode: %d",iCoverFlag); 
		return UNRECOGNIZED_COVER_MODE;
	}

	//�������ݽ���socket
	int iSockData=socket(AF_INET,SOCK_STREAM,0);
	if (iSockData==-1)
	{
	  strcpy(errMsg,"Fail to build socket for receiving data!\n");
		return CREATE_LISTENING_SOCK_ERR;
	}

	int iTmp=sizeof(s_sockaddr);
	_getsockname(isockftp,(struct sockaddr *)&s_sockaddr,&iTmp);
	s_sockaddr.sin_port=0;

	if(bind(iSockData,(struct sockaddr *)&s_sockaddr,sizeof(s_sockaddr))==-1)
	{
		close(iSockData);  
	  strcpy(errMsg,"Fail to bind port for socket!\n");
		return LISTENING_SOCK_BIND_ERR;
	}

	if(listen(iSockData,1)==-1)
	{
		close(iSockData);
	  strcpy(errMsg,"Fail to listen!\n");
		return SOCK_LISTENING_ERR;
	}

	iTmp=sizeof(s_sockaddr);
	_getsockname(iSockData,(struct sockaddr *)&s_sockaddr,&iTmp);
	c=(unsigned char *)&s_sockaddr.sin_addr;
	p=(unsigned char *)&s_sockaddr.sin_port;
	
	if(c_PassiveFlag=='Y')
	{
	  memset(sz_buff,0,1024);
	  iRet = ftpcmd(sz_buff,"PASV"); 
	  if(iRet!=1)
	  {
	  	close(iSockData);
	  	if(iRet==-1)
	  	{
	  		strcpy(errMsg,sz_errorMessage);
	  	}
	  	else
	  	{
	  		strcpy( errMsg, "Fail to get message from server!\n" ); 
	  	}
	  	return RECIEVE_SERVER_MSG_ERR;
	  }
	}

	memset(sz_buff,0,1024);
	iRet=ftpcmd(sz_buff,(char *)"PORT %d,%d,%d,%d,%d,%d",c[0],c[1],c[2],c[3],p[0],p[1]);//200 is ok
	if(iRet!=1)
	{//�����ѶϿ����ȴ�1������� 
		close(iSockData); 
		if(iRet==-1)
	  {
	  	strcpy(errMsg,sz_errorMessage);
	  	return -1;
	  }
		if(Reconnect(errMsg)==1)
		{
			goto CHECK_AGAIN;
		} 
		else
		{
			close(iSockData );  
	    strcpy(errMsg, "Connection was elimilated\n" ); 
			return SOCK_CONNECT_ERR;
		}
	}

	//����Ϣ��λ
	while(1)
	{
		if(IsCmdOk(sz_buff,"200 PORT")==1)
		{
			break;
		}
		else
		{
			memset(sz_buff,0,strlen(sz_buff));	
			do
			{ 
				memset(sz_buff,0,1024);
				if (fgets(sz_buff,1024,m_ftpio)==NULL)
				{ 
					close(iSockData);
					return 0;
				}
			}while(sz_buff[3] == '-'); 
		}
	}
	
	//��FTP���������뵽Ŀ��·��
	if(sz_targetPath!=NULL)
	{
		//�趨��ǰ·������Ŀ¼
		memset(sz_buff,0,1024);
		iRet=ftpcmd(sz_buff,(char *)"CWD %s",sz_targetPath); 
		if(iRet==-1)
	  {
	  	close(iSockData);
	  	strcpy(errMsg,sz_errorMessage);
	  	return -1;
	  }
		iRet=IsCmdOk(sz_buff,"250"); 
		
		if(iRet!=1)
		{
			close(iSockData);  
    	sprintf(errMsg,"Inexistant directory: %s\n",sz_targetPath);
			return UPLOAD_TARGET_PATH_ERR;
		}
	}
	
	//�򿪱���Դ�ļ�
	int iSaveFd=open(sz_sourceFile,O_RDONLY,0644);
	if(iSaveFd==-1)
	{
		close( iSockData ); 
	  sprintf(errMsg,"Inexistant file: %s!\n",sz_sourceFile);
		return OPEN_SOURCE_FILE_ERR;
	} 

	if(iSendMode==10) 
	{//10���� A
		memset(sz_buff,0,1024);
		iRet=ftpcmd(sz_buff,(char *)"TYPE A");
		if(iRet==-1)
	  {
	  	close(iSockData);
	  	close(iSaveFd);
	  	strcpy(errMsg,sz_errorMessage);
	  	return -1;
	  }
	} 
	else if(iSendMode==2)
	{//2���� I
		memset(sz_buff,0,1024);
		iRet=ftpcmd(sz_buff,(char *)"TYPE I");
		if(iRet==-1)
	  {
	  	close(iSockData);
	  	close(iSaveFd);
	  	strcpy(errMsg,sz_errorMessage);
	  	return -1;
	  }
	}
	else
	{
		close(iSockData);
		close(iSaveFd);
	  sprintf(errMsg,"Unrecognized transform mode - %d\n",iSendMode);
		return UNRECOGNIZED_SENDING_MODE;	
	}
	
	memset(sz_buff,0,1024);
	iRet=ftpcmd(sz_buff,(char *)"STOR %s",sz_targetName);  
	if(iRet==-1)
	{
		close(iSockData);
		close(iSaveFd);
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}

	//��������
	iTmp= sizeof(s_sockaddr); 
	int m_sockxfer=Accept(iSockData,(struct sockaddr *)&s_sockaddr,&iTmp); 
	if (m_sockxfer==-1)
	{
		close(iSockData);
		close(iSaveFd);
		strcpy(errMsg,sz_errorMessage);		
		return SEND_CMD_ERR;
	}
	memset(sz_buff,0,1024);	  
	int timeout=BLOCK_TIME_OUT_MSEC;
	pollfd spoll[3];
	spoll[0].fd=m_sockxfer;
	spoll[0].events=POLLWRNORM;
		while((iLen=read(iSaveFd,sz_buff,sizeof(sz_buff)))>0)
		{
	    iRet=poll(spoll,1,timeout);
      if(iRet>0)
      {
        write(m_sockxfer,sz_buff,iLen);  
      }
      else
      {
      	if(iRet==0)
      	{
      		strcpy(errMsg,"write block comes out!");
          close(iSaveFd);
					close(iSockData);
					close(m_sockxfer);
          return SOCKET_BLOCK_WRITE;
        }
        else
        {
        	//2006-9-20 19:17 added ����EINTR����
      		if(errno==EINTR)
        	{
        		continue;
        	}
        	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
        	close(iSaveFd);
					close(iSockData);
					close(m_sockxfer);
        	return SOCKET_BLOCK_WRITE;
        }
      }
		}
	// solve block end
	close(iSaveFd);
	close(iSockData);
	close(m_sockxfer);

	//����Դ�ļ�
	if(strcmp(pch_storePath,SOURCE_IGNORE)==0)
	{//������
	}
	else if(strcmp(pch_storePath,SOURCE_DELE)==0)
	{
    iRet=remove(sz_sourceFile);
		if(iRet!=0)
		{
      sprintf(errMsg,"Delete File(%s) ERROR!",sz_sourceFile);
			return DELETE_SOURCE_ERR;
		}
	}
	else
	{
		char sz_bakFile[255];
		memset(sz_bakFile,0,255);
		sprintf(sz_bakFile, "%s/%s", pch_storePath, sz_sourceName);
		iRet=rename(sz_sourceFile,sz_bakFile);
		if(iRet!=0)
		{
      sprintf(errMsg,"Backup File(%s) ERROR!",sz_sourceFile);
			return STORE_SOURCE_ERR;
		}
	}

	return 1;
}

/***************************************************
description:	�ϴ��ļ���FTP��������
input:	        
		sendInfo:		������ƽṹ��
output:		           
		errMsg��		������Ϣ                 
return:         	
		��Upload����һ���غ�����ͬ
programmer:	
		��	��
date:		
		2005-08-30
*****************************************************/ 
int CFTPmanager::Upload( SEND_INFO sendInfo ,char *errMsg )
{
	return(Upload(sendInfo.localFile,sendInfo.remoteFile,sendInfo.coverFlag,errMsg,sendInfo.sendMode,sendInfo.cStorePath));
}


/***************************************************
description:	ɾ��Զ���ļ�
input:	        
		filePathName:		Ҫ��ɾ����Զ���ļ�·��+�ļ���
output:		
		errMsg��		������Ϣ
return:         	
		1:			�ɹ�
		����:			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
programmer:	
		��	��
date:		
		2005-08-26
*****************************************************/ 
int CFTPmanager::RmFile( const char *pch_delteFile,char *errMsg)
{
	int iRet=0;
	char sz_buff[1024];
	char sz_filePath[255], sz_fileName[255];
	
	memset(sz_filePath,0,255);
	memset(sz_fileName,0,255);
	DivFileName(pch_delteFile,sz_filePath,sz_fileName);

CHECK_AGAIN:
	
	//��Ϣ��λ
	Position();
	
	memset(sz_buff,0,1024);
	iRet = ftpcmd( sz_buff, (char *)"DELE %s", pch_delteFile ); //250 is ok
	if(iRet==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		return DELETE_FILE_ERR;
	}
	
	iRet=IsCmdOk(sz_buff,"250");

	return iRet; 
}

/***************************************************
description:	�ƶ�Զ���ļ�
input:	        
		sourceFilePathName:	Ҫ���ƶ���Դ�ļ�·��+�ļ���
		targetFilePathName��	�ƶ��󱣴��Ŀ���ļ�·��+�ļ�����
output:		
		errMsg��		������Ϣ                 
return:         	
		1:			�ɹ�
		����:			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
programmer:	
		��	��
date:		
		2005-08-26
*****************************************************/ 
int CFTPmanager::MoveFile(const char *pch_sourceFile,const char *pch_targetFile,char *errMsg)
{
	int	iRet=0;

  char sz_buff[1024];
	
CHECK_AGAIN:
	
	//��Ϣ��λ
	Position();
	
	iRet=ftpcmd(sz_buff,(char *)"RNFR %s",pch_sourceFile);//RNFR--RENAME FROM    		
	if(iRet==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	
	//����Ϣ��λ
	while(1) 
	{
		if(IsCmdOk(sz_buff,"226")==1) 
		{
			memset(sz_buff,0,1024);	
			do
			{ 
				if(fgets(sz_buff,1024,m_ftpio) == NULL)
				{
					return 0;
				}
			}while(sz_buff[3] == '-'); 
		}
		else
		{
			break;	
		}
	}

	iRet=IsCmdOk(sz_buff,"350");
	if(iRet!=1)
	{
	  sprintf(errMsg,"Inexistant file: %s\n",pch_sourceFile);
		return SOURCE_FILE_NOT_EXIST;
	}
	
	memset(sz_buff,0,1024);
	iRet = ftpcmd(sz_buff,(char *)"RNTO %s",pch_targetFile);//RNTO-- RENAME TO
	if(iRet==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	iRet=IsCmdOk(sz_buff,"250");
	
	if (iRet!=1)//��һ�θ���ʧ��
	{
		iRet=ftpcmd(sz_buff,(char *)"RNFR %s",pch_targetFile);//���Ŀ���ļ��Ƿ����
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		iRet=IsCmdOk(sz_buff,"350");
		if( iRet==1 )//���Ŀ���ļ�������ɾ��
		{
			iRet=ftpcmd(sz_buff,(char *)"DELE %s",pch_targetFile); //250 is ok
			if(iRet==-1)
			{
				strcpy(errMsg,sz_errorMessage);
				return -1;
			}
			iRet=IsCmdOk(sz_buff,"250");
			if(iRet==1)
			{
				iRet=ftpcmd(sz_buff,(char *)"RNFR %s",pch_sourceFile);
				if(iRet==-1);
				{
					strcpy(errMsg,sz_errorMessage);
					return -1;
				}
				iRet=IsCmdOk( sz_buff, "350" );
				if(iRet!=1)
				{
					sprintf( errMsg, "Inexistant file: %s\n", pch_sourceFile );
					return SOURCE_FILE_NOT_EXIST;
				}
				iRet=ftpcmd(sz_buff,(char *)"RNTO %s",pch_targetFile);
				if(iRet==-1)
				{
					strcpy(errMsg,sz_errorMessage);
					return -1;
				}
				iRet=IsCmdOk( sz_buff, "250" );
				if(iRet!=1)
				{
					sprintf(errMsg,"move file %s to %s error",pch_sourceFile,pch_targetFile);
					return MOVE_FILE_ERR;
				}
			}
			else
			{
				sprintf( errMsg, "File already exists: %s!\n", pch_targetFile );
				return FOLDER_EXIST;
			}
		}
		else
		{
			sprintf(errMsg,"move file %s to %s error",pch_sourceFile,pch_targetFile);
			return MOVE_FILE_ERR;
		}
	}
	
	return 1;
}

/****************************************************************
description:	��ȡԶ��Ŀ¼�µ������ļ���Ϣ��������Ϣд���ļ��б���
							��sun������û��ftp��SIZE����
input:	      
output:		           
return:     1 success, other fail    	
programmer:	
		yangzx
date:		
		2006-04-16
****************************************************************/
int CFTPmanager::SizeFile(char *pch_dealFile,int & iFileSize,char * errMsg)
{ 
	int iRet=-1;
	int iLen=-1;
	char sz_buff[1024];
	temp_num=0;
  struct sockaddr_in s_sockaddr; 

	unsigned char *c, *p; 
	char sz_filePath[256];
	char sz_fileName[256];
	memset(sz_filePath,0,sizeof(sz_filePath));
	memset(sz_fileName,0,sizeof(sz_fileName));
	DivFileName(pch_dealFile,sz_filePath,sz_fileName);
	
	CheckPath(sz_filePath);
	
	char s[9][255];
	for(int i=0;i<=8;i++)
	{
		memset(s[i],0,255);
	}

CHECK_AGAIN:	
	
	memset(sz_buff,0,1024); 

	//�������ݽ���socket
	int iSockData=socket(AF_INET,SOCK_STREAM,0);
	if(iSockData==-1)
	{
		close( iSockData );
		strcpy( errMsg, "Fail to build socket for receiving data!" );
		return CREATE_LISTENING_SOCK_ERR;
	}

	int iTmp = sizeof(s_sockaddr);
	_getsockname(isockftp,(struct sockaddr *)&s_sockaddr,&iTmp);
	s_sockaddr.sin_port = 0;

	if (bind(iSockData,(struct sockaddr *)&s_sockaddr,sizeof(s_sockaddr)) == -1) 
	{
		close( iSockData ); 
		strcpy( errMsg, "Fail to bind port for socket!" );
		return LISTENING_SOCK_BIND_ERR;
	}

	if (listen(iSockData,1) == -1)  
	{
		close( iSockData );
		strcpy( errMsg, "Fail to listen!");
		return SOCK_LISTENING_ERR;
	}

	iTmp = sizeof(s_sockaddr);
	_getsockname(iSockData,(struct sockaddr *)&s_sockaddr,&iTmp);
	c=(unsigned char *)&s_sockaddr.sin_addr;
	p=(unsigned char *)&s_sockaddr.sin_port;

	if(c_PassiveFlag=='Y')
	{
	  memset(sz_buff,0,1024);
	  iRet = ftpcmd(sz_buff,"PASV"); 
	  if(iRet!=1)
	  {
	  	close(iSockData);
	  	if(iRet==-1)
	  	{
	  		strcpy(errMsg,sz_errorMessage);
	  	}
	  	else
	  	{
	  		strcpy( errMsg, "Fail to get message from server!\n" ); 
	  	}
	  	return RECIEVE_SERVER_MSG_ERR;
	  }
	}
	
	memset( sz_buff, 0, 1024 );
	iRet = ftpcmd(sz_buff,(char *)"PORT %d,%d,%d,%d,%d,%d",c[0],c[1],c[2],c[3],p[0],p[1]);

	if (iRet != 1)  
	{//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if ( Reconnect( errMsg ) == 1 )
		{
			goto CHECK_AGAIN;
		} 
		else 
		{
			strcpy( errMsg, "Connection was elimilated\n" ); 
			return SOCK_CONNECT_ERR;
		}
	}

	//����Ϣ��λ
	while(1) 
	{
		if(IsCmdOk(sz_buff,"200 PORT")==1)
		{
			break;
		} 
		else
		{
			memset(sz_buff,0,strlen(sz_buff));
			do
			{
				memset(sz_buff,0,1024);
				if(fgets(sz_buff,1024,m_ftpio)==NULL)
				{
					close(iSockData);
					return 0;
				}
			}while(sz_buff[3]=='-');
		}
	}

	memset(sz_buff,0,1024);

  iRet=ftpcmd(sz_buff,(char *)"CWD %s",sz_filePath);
	if(iRet==-1)
	{
	  close(iSockData);
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	iRet=IsCmdOk(sz_buff,"250");
	if(iRet!=1)
	{
    iRet=IsCmdOk(sz_buff,"226");
		if(iRet!=1)
		{
		  sprintf(errMsg,"CWD %s ERROR",sz_filePath);
			close(iSockData);
		  return FOLDER_PATH_NOT_EXIST;
	  }
  }

	iRet=ftpcmd(sz_buff,(char *)"LIST %s",sz_fileName);
	if(iRet!=1)
	{
	  close(iSockData);
		if(iRet==-1)
    {
		  strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		else
		{
		  sprintf(errMsg,"LIST %s error",sz_fileName);
			return 0;
		}
	}

  iRet=IsCmdOk(sz_buff,"150");
	iTmp=sizeof(s_sockaddr); 
	int m_sockxfer=Accept(iSockData,(struct sockaddr *)&s_sockaddr,&iTmp); 
	if(m_sockxfer==-1)
	{
	  close(iSockData); 
		strcpy(errMsg,sz_errorMessage);
		return RECIEVE_SERVER_MSG_ERR;
	} 

	char tempfile[256];
	iLen=read(m_sockxfer,sz_buff,sizeof(sz_buff));
	close(m_sockxfer);
	close(iSockData);
	s[4][0]='\0';
	sscanf(sz_buff,"%s %s %s %s %s %s %s %s %s",s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7],s[8]);

  if(s[4][0]=='0')
	{
	  iFileSize=0;
  }
	else if(s[4][0]!='\0')
	{
	  iFileSize=-1;
		iFileSize=atoi(s[4]);
		if(iFileSize==0)
		{
			sprintf(errMsg,"file %s not found",sz_fileName);
			iFileSize=-1;
			return SOURCE_FILE_NOT_EXIST;
		}
	}
	else
	{
    if(s[2][0]=='0')
    {
	    iFileSize=0;
    }
    else
    {
			iFileSize=-1;
			iFileSize=atoi(s[2]);
			if(iFileSize==0)
			{
				sprintf(errMsg,"file %s not found",sz_fileName);
				iFileSize=-1;
				return SOURCE_FILE_NOT_EXIST;
			}
	  }
	}
	return 1;
}

int CFTPmanager::ChFile(const char *pch_sourceFile,char *errMsg)
{
	int	iRet=0;

  char sz_buff[1024];
	
  CHECK_AGAIN:
	
	//��Ϣ��λ
	Position();

	iRet=IsCmdOk(sz_buff,"350");

	iRet=ftpcmd(sz_buff,(char *)"RNFR %s",pch_sourceFile);//���Ŀ���ļ��Ƿ����
	if(iRet==-1)
	{
	  strcpy(errMsg,sz_errorMessage);
			return -1;
	}
	iRet=IsCmdOk(sz_buff,"350");
	if(iRet==1) return 1;
	else
	{
    sprintf( errMsg, "Inexistant file: %s\n", pch_sourceFile );
    return -1;
  }
}
/***************************************************
description:	�������FTP������
input:	        
		returnBuf:	���������ص���Ϣ	
		fmt:		��ʽ��������        
		
output:         
return:         
		0��		ʧ��
		1��		�ɹ�
programmer:	��	��
date:		2005-08-16
*****************************************************/
int CFTPmanager::ftpcmd( char *returnBuf, char *fmt, ...)
{
	va_list vp; 
	int err,len; 
	char buf[1024];
	memset(buf,0,1024);
	int iRet;

	if(m_ftpio==NULL)
	{
		m_ftpio=fdopen(isockftp,"r"); 
		if(m_ftpio==NULL) 
		return 0; 
	}

	if(fmt) 
	{ 
		va_start(vp,fmt); 
		len = vsprintf(buf,fmt,vp); 
		buf[len++] = '\r';
		buf[len++]='\n'; 
		iRet=IsReadyToWrite(isockftp);
		
		if(iRet)
		{
			iRet=write(isockftp,buf,len); 
			if(iRet==-1)
			{
				sprintf(sz_errorMessage,"write error, errno=%d",errno);
				fclose(m_ftpio);
        m_ftpio=NULL;
				return -1;
			}
		}
		else
		{
			strcpy(sz_errorMessage,"write to socket block");
			return -1;
		}
	}

	do 
	{ 
		//iRet=IsReadyToRead(isockftp);
		iRet=1;
		if(iRet)
		{
			if (fgets(returnBuf,1024,m_ftpio) == NULL) 
				return 0;
			//cout<<"ftpcmd"<<returnBuf<<endl;
		}
		else
		{
		  fclose(m_ftpio);
      m_ftpio=NULL;
			strcpy(sz_errorMessage,"read blocks");
			return -1;
		}
	}while(returnBuf[3] == '-'); 
	//cout<<"----end---- "<<endl;
	return 1;	
}

/***************************************************
description:	��ÿ�β�����ʼǰ�Զ���λ������Ϣ
input:	      
output:		           
return:         	
programmer:	
		��	��
date:		
		2005-10-09
*****************************************************/ 
void CFTPmanager::Position()//?
{
	char sz_Buff[1024];
	
	memset(sz_Buff,0,1024);
	int ret=ftpcmd(sz_Buff,(char *)"PWD");	 
	while(1)
	{
		if(IsCmdOk(sz_Buff,"257")==1)
		{
			break;	
		}

		//�ٶ�һ����Ϣ
		do
		{ 
			memset(sz_Buff,0,1024);
			if(fgets(sz_Buff,1024,m_ftpio)==NULL) 
				return;          
		}while(sz_Buff[3]== '-'); 
	}	
}

/***************************************************
description:	����buf�ж�����ִ���Ƿ�ɹ�
input:	        
		buf:			ִ������󷵻ص��ַ���
		okNum:			����ִ�гɹ�ʱ���ص�Num    
return:         
		1:			ִ�гɹ�
		0:			ִ��ʧ��
		
programmer:	��	��
date:		2005-09-27
*****************************************************/
int CFTPmanager::IsCmdOk(const char *pch_buff,const char *pch_key)
{
	char *pch_ret=NULL;
  char sz_checkBuff[1024];
  
  strcpy(sz_checkBuff,pch_buff);

	pch_ret=strstr(sz_checkBuff,pch_key);
	
	return(pch_ret!=NULL ? 1:0);
}

/***************************************************
description:	�õ�FTP��������ָ��Ŀ¼(path����ָ��·����)���ļ��б�,������Ŀ¼�µ��ļ����ļ����Ǵ�·����ȫ���������listSavePathNameָ�����ļ���
input:	        
		path:			FTP��������Ҫȡ���ļ��б��·��
		listSavePathName:	ȡ�õ��ļ��б����ڱ����ļ��У��˲�����Ϊ���ļ���·��+�ļ���
output:        
		errMsg��		������Ϣ
return:         
		1��			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	yangzx
date:		2006-05-16
*****************************************************/ 
int CFTPmanager::ListAllFile(  char *path, const char *listSavePathName, char *errMsg )
{
	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	temp_num=0;
	struct sockaddr_in addr; 
	
	unsigned char			*c, *p; 
	//char filePathName[255];
	char filePath[255];
	memset(filePath,0,sizeof(filePath));
	char fileName[255];
	memset(fileName,0,sizeof(fileName));
	DivFileName( listSavePathName,filePath, fileName );
	
	CheckPath(path);
	
CHECK_AGAIN:	
	//ɾ��ԭ���ļ��б�
	remove(listSavePathName);
	
	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData ); 


	strcpy( errMsg, "Fail to bind port for socket!\n" );  

                                          
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//close( iSockData );
		close( iSockData );                                                
		//shutdown( isockftp, SHUT_RDWR );    


	strcpy( errMsg, "Fail to listen!\n" );  
  
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 
			
			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//����Ϣ��λ
	while( 1 ) {
		if ( IsCmdOk( buf, "200 PORT" ) == 1 ) {
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
			}while(buf[3] == '-'); 
		}
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"CWD %s", path);
	if(res==-1)
	{
		close(iSockData);
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {
		close( iSockData );


	sprintf( errMsg, "Inexistant directory: %s!\n", path );

		//sprintf( errMsg, "Զ�̻�����Ҫ�õ��ļ��б��·��%s������!\n", path );
		return FOLDER_NOT_EXIST;
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST"); // 150 is ok
	if(res==-1)
	{
		close(iSockData);
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	res = IsCmdOk( buf, "150" );
	if ( 1 != res ){
		close( iSockData );


	sprintf( errMsg, "Inexistant directory: %s!\n", path );
		
		//sprintf( errMsg, "ȡԶ�̻�����·��%s���ļ��б�ʧ��!\n", path );
		return FOLDER_NOT_EXIST;
	}
	
///////////////////////////////////////////////receive
	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1)
	{
		close( iSockData ); 
		strcpy(errMsg,sz_errorMessage);
		return RECIEVE_SERVER_MSG_ERR;
	} 
	
	
	//���´����ļ��б�
	//int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);  
	char tempfile[256];
	sprintf(tempfile,"%s_temp",listSavePathName);
	
	int tempfd = open(tempfile,O_WRONLY|O_CREAT,0644);                                              
	
	/*if (iSaveFd == -1)
	{                                                                                 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//sprintf( errMsg, "Ҫ�����ڱ��ص��ļ�%s�����ʹ�ʧ��!\n", listSavePathName );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	} */                        
	memset( buf, 0, 1024 );
	int count_flag=0;
int timeout=BLOCK_TIME_OUT_MSEC;
	pollfd spoll[3];
	spoll[0].fd=m_sockxfer;
	spoll[0].events=POLLRDNORM;
	while(1)
	{
		 res=poll(spoll,1,timeout);
    if(res>0)
    {
    		//if(FD_ISSET(m_sockxfer,&fdSet))
    			
    		len=read(m_sockxfer,buf,sizeof(buf));
        if(len>0)
        {
        	write(tempfd,buf,len); 
		    	memset( buf, 0, 1024 );
        }
        else
        {
        	//exit_flag=0;
        	break;
        }
    }
    else
    {
    	if(res==0)
    	{
    		strcpy(errMsg,"read block comes out!");
        close( tempfd );
				close( m_sockxfer );
				close( iSockData );
				unlink(tempfile);
        return SOCKET_BLOCK_READ;
      }
      else
      {
      	//2006-9-20 19:17 added ����EINTR����
      		if(errno==EINTR)
        	{
        		continue;
        	}
        	//end added
      	//sprintf(errMsg,"SOCKET_SELECT_ERROR,the errno=%d",errno);
      	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
      	//printf("%s\n",errMsg);
      	close( tempfd );
				close( m_sockxfer );
				close( iSockData );
				unlink(tempfile);
      	return SOCKET_POLL_ERROR;;
      }
    }
  }//end while

	filelist=fopen(listSavePathName,"w+t");
	if(filelist==NULL)
	{
		sprintf(errMsg,"open file %s error!",listSavePathName);
		return TARGET_FILE_CREATE_OPEN_ERR;
		
	}
	FILE * savefile;
	char buf2[800];
	char s[9][255];
	char curpath[256];
	char fullfilename[256];
	savefile=fopen(tempfile,"r");
	if(savefile==NULL)
	{
			sprintf(errMsg,"open %s error!\n",tempfile);
			return TARGET_FILE_CREATE_OPEN_ERR;
			
	}			
	char strlog[500];
	char str[1024];
	while(fgets(buf2,800,savefile)!=NULL)
	{
		sscanf(buf2,"%s %s %s %s %s %s %s %s %s",s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7],s[8]);
		if(buf2[0]=='t')
			continue;
		if(buf2[0]=='d')
		{
			
			if( strcmp(s[8],".")==0 || strcmp(s[8],"..")==0 )
			{
					continue;
			}
			else
			{
				sprintf(curpath,"%s/%s",path,s[8]);
				res=ScanFile(curpath,filePath,strlog);// scan curpath, find all files and store file info in file list
				if(res!=1)
				{
					strcpy(errMsg,strlog);
					if(savefile!=NULL)
						fclose(savefile);
					remove(tempfile);
					return res;
				}
			}
		}
		else
		{
			sprintf(fullfilename,"%s/%s",path,s[8]);
			sprintf(str,"%s %s %s %s %s %s %s %s %s\n",s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7],fullfilename);
			fwrite(str,sizeof(char),strlen(str),filelist);
		}
		
	}
	if(filelist!=NULL)
		fclose(filelist);	
	
	remove(tempfile);

	return 1;
}	
	
	
	

/***************************************************
description:	�л�FTP�������ϵĵ�ǰ·������ĳһָ��·��·��
input:	        
		path:			Ҫ�л�����·��(�����Ǿ��Ժ����·��)
output:         
		errMsg:			������Ϣ
return:         
		1:			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	��	��
date:		2005-08-23
*****************************************************/ 
int CFTPmanager::ChDir( const char *path, char *errMsg ){
	char		buf[1024];
	
CHECK_AGAIN:

	//��Ϣ��λ
	Position();

	memset( buf, 0, 1024 );
	int res = ftpcmd(buf, (char *)"CWD %s", path); 
	res = IsCmdOk( buf, "250" );	
	return res;
}

/***************************************************
description:	��ĳһ·���ڴ���һ·��
input:	        
		newDir:			Ҫ������·��(�����Ǿ��Ժ����·��)
					ע�⣺ĩβҪ�Ӳ��ӡ�/������
output:         
		errMsg:			������Ϣ
return:         
		1:			�ɹ�
		������			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
		
programmer:	��	��
date:		2005-08-23
*****************************************************/ 
int CFTPmanager::MkDir( const char *newDir, char *errMsg ){
	char		buf[1024];
	char		newFolder[200], newFolder2[200], path[1000];
	memset( newFolder, 0, 200 );
	memset( newFolder2, 0, 200 );
	memset( path, 0, 1000 );
	
	//��Ϣ��λ
	Position();
	
	//�жϽ�β���ޡ�/��
	if ( '/' != newDir[ strlen( newDir ) - 1 ] ){
		//�����ֽ�������
		DivFileName( newDir, path, newFolder );		
	} else {		
		//ȥ����β�ġ�/��
		DivFileName( newDir, path, newFolder );
		strcpy( newFolder2, path );
		memset( newFolder, 0, 200 );
		memset( path, 0, 1000 );
		//�����ֽ�������
		DivFileName( newFolder2, path, newFolder );	
	}
	

CHECK_AGAIN:

	memset( buf, 0, 1024 );
	int res = ftpcmd(buf, (char *)"CWD %s", path); 
	if ( res != 1 ) {//�����ѶϿ����ȴ�1������� 
		if( res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}
	}
	
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {


	sprintf( errMsg, "Inexistant path: %s\n", path );
		
		//sprintf( errMsg, "Զ�̻�����Ҫ������·����·��%s������!\n", path );
		return UPLOAD_TARGET_PATH_ERR;	
	}
	
	memset( buf, 0, 1024 );
	res = ftpcmd(buf, (char *)"CWD %s", newDir);
	if(res==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	res = IsCmdOk( buf, "250" ); 
	if ( 1 == res ) {//����
		return 1;	
	}
		
//	printf( "222222222222\n" );
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"MKD %s", newFolder); 
	if(res==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	
	return 1;
}
//�õ��ļ���С
int CFTPmanager::GetFileSize(const char *filePathName)
{
	struct stat buf;
	if(stat(filePathName,&buf)==-1)
	{
		return -1;
	}

	return buf.st_size;
}

/***************************************************
description:	���Զ���ļ���·���Ƿ����
input:	        
		filePathName :			Ҫ�������ļ�(·��+�ļ���)��·��
output:         
return:         
		<0:		����
		0��		������
		1��		����
programmer:	��	��
date:		2005-08-23
*****************************************************/ 
int CFTPmanager::DirFileExist( const char *filePathName ){
	char		fileName[200], filePath[1000];
	memset( fileName, 0, 200 );
	memset( filePath, 0, 1000 );
	
	DivFileName( filePathName, filePath, fileName);	

	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	struct sockaddr_in addr; 
	char			errMsg[1024];
	memset( errMsg, 0, 1024 );
	
	unsigned char			*c, *p; 
	
CHECK_AGAIN:	

	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );       

	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                    
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData ); 


	strcpy( errMsg, "Fail to listen!\n" );  
                                               
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);             
	                                                  
	if (res != 1)  {//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if(res==-1)
		{//����read��write����������󣬷�����һ��
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}     
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                        
	}    
	
	//����Ϣ��λ
	while( 1 ) {
		if ( IsCmdOk( buf, "200 PORT" ) == 1 ) {
			//printf("port:<%s>\n",buf);
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
				//printf( "cmd: %s\n", buf );
			}while(buf[3] == '-'); 
		}
	}
	
	memset( buf, 0, 1024 );
	//printf( "dirfileexistaaaaaaaaaaaaaaaaaaaaaaa\n");
	res = ftpcmd(buf,(char *)"CWD %s", filePath); 
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {


	sprintf( errMsg, "Inexistant directory: %s!\n", filePath );

		//sprintf( errMsg, "����ļ��Ƿ����ʱ��Զ�̻�����Ҫ�õ��ļ��б��·��%s������!\n", filePath );
		return FOLDER_NOT_EXIST;	
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST"); 
	res = IsCmdOk( buf, "150" );	
	if ( 1 != res ){
		close( iSockData );


	sprintf( errMsg, "Inexistant directory: %s!\n", filePath );

		//sprintf( errMsg, "����ļ��Ƿ����ʱ��ȡԶ�̻�����·��%s���ļ��б�ʧ��!\n", filePath );
		return FOLDER_NOT_EXIST;
	}
	
///////////////////////////////////////////////receive
	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1){
		//close( iSockData );
		close( iSockData );        
		return -1;
	} 
	
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
	
	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	int iRetu = 0;
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//�ֽ�buf
			memset( cParam0, 0, 100 );
			memset( cParam1, 0, 100 );
			memset( cParam2, 0, 100 );
			memset( cParam3, 0, 100 );
			memset( cParam4, 0, 100 );
			memset( cParam5, 0, 100 );
			memset( cParam6, 0, 100 );
			memset( cParam7, 0, 100 );
			memset( cParam8, 0, 100 );
			
			sscanf( buf, "%s %s %s %s %s %s %s %s %s", 
			cParam0, cParam1, cParam2, cParam3, cParam4,
				 cParam5, cParam6, cParam7, cParam8);
			
			if ( strcmp( cParam8, fileName ) == 0 ){
				iRetu = 1;	
				break;
			}
			memset( buf, 0, 1024 );
		}
		memset( cCur, 0, 5 );
	}
	
	close( iSockData );
	close( m_sockxfer );

	return iRetu;	
}

/***************************************************
description:	�жϱ����ļ��Ƿ����
input:	        
		p_cPathName:		Ҫ���жϵı����ļ�·��+�ļ���
output:		
return:         	
		0:			������
		1:			����	
programmer:	
		��	��
date:		
		2005-08-22
*****************************************************/ 
int CFTPmanager::IsFileExist( const char *p_cPathName )
{
	return ( ( access( p_cPathName, 0 ) == 0 ) ? 1 : 0 ); 
}

/***************************************************
description:	ɾ��Զ��·��
input:	        
		dir:			Ҫ��ɾ����Զ��·��
output:		           
		errMsg��		������Ϣ                 
return:         	
		1:			�ɹ�
		����:			�ο�FTPmanager.h��Ĵ�����Ϣ�Ķ���
programmer:	
		��	��
date:		
		2005-08-26
*****************************************************/ 
int CFTPmanager::RmDir( const char *dir, char *errMsg )
{
	int iRes = 0;
	char		folderName[200], path[1000];
	memset( folderName, 0, 200 );
	memset( path, 0, 1000 );
	
	DivFileName( dir, path, folderName);

CHECK_AGAIN:
	
	char		cFullName[2000];
	char		cBuf[1000];
	
	memset( cBuf, 0, 1000 );
	memset( cFullName, 0, 2000 );
	sprintf( cFullName, "%s/%s", path, folderName );
	
	//��Ϣ��λ
	Position();
	
	iRes = ftpcmd( cBuf, (char *)"RMD %s", cFullName );//250 is ok
	if(iRes==-1)
	{
		strcpy(errMsg,sz_errorMessage);
	}
	return iRes;	
}




/***************************************************
description:	���ļ�ȫ·���ֽ���·�����ļ���
input:	        
		filePathName:		�ļ�ȫ·��+�ļ���
output:		           
		filePath��		�ļ�·��
		fileName:		�ļ���              
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-02
*****************************************************/ 
int CFTPmanager::DivFileName( const char *filePathName, char *filePath, char *fileName ){
	char		*p = NULL;
	char		cFile[1024];
	memset( cFile, 0, 1024 );
	strcpy( cFile, filePathName );
	/*int iLen=strlen(cFile);
	if(cFile[iLen-1]=='/')
	{
		cFile[iLen-1]=0;
	}*/
	char		*pFile = cFile;
	p = strrchr( pFile, '/' );
	if ( p == NULL ) 
	{//����ļ�·������ǰ·���µ��ļ�
		strcpy( filePath, "." );
		strcpy( fileName, filePathName );		
	} 
	else 
	{
		int		iLen = p - pFile;

		if( iLen!=0)
		{
			strncpy( filePath, pFile, iLen );
		}
		else
		{
			strcpy(filePath,"/");
		}
		pFile = p + 1;
		strcpy( fileName, pFile );
	}
	return 1;
}

/***************************************************
description:	�õ�Unixϵͳ(Զ�̻�)ĳ·���·���ĳ������ʽ���ļ����б�
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::ListUnixBy( const char *path, const char *listSavePathName, 
		const char *condition, char *errMsg ){
	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	struct sockaddr_in addr; 
	
	unsigned char			*c, *p; 
	
	//CheckPath(path);
	//CheckPath(listSavePathName);
	
	
CHECK_AGAIN:	

	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData );         


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                  
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		close( iSockData );  


	strcpy( errMsg, "Fail to listen!\n" );  
                                              
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//����Ϣ��λ
	while( 1 ) {
		if ( IsCmdOk( buf, "200 PROT" ) == 1 ) {
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
			}while(buf[3] == '-'); 
		}
	}	
	
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"CWD %s", path); 
 	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {


	sprintf( errMsg, "Inexistant directory: %s!\n", path );

		//sprintf( errMsg, "Զ�̻����ϵ�·��%s������!\n", path );
		return FOLDER_NOT_EXIST;	
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST"); 
	
///////////////////////////////////////////////receive

	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1){
		close( iSockData );


	//strcpy( errMsg, "Fail to receive data!\n" );
	strcpy(errMsg,sz_errorMessage);

		//strcpy( errMsg, "���շ���������ʧ��!\n" );
		return SEND_CMD_ERR;
	} 
	
	//ɾ��ԭ���ļ��б�
	remove(listSavePathName);
	//���´����ļ��б�
	int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//printf( errMsg, "Ҫ�����ڱ��ص��ļ�%s�����ʹ�ʧ��!\n", listSavePathName );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//��ʼ�ж��Ƿ��������
			if ( IsUnixZhengZe( buf, condition ) == 1 ){
				write(iSaveFd,buf,strlen(buf)); 
			}	
			memset( buf, 0, 1024 );
		}
		memset( cCur, 0, 5 );
	}
	
	close( iSockData );
	close( m_sockxfer );
	close( iSaveFd );
		
	return 1;	
}

/***************************************************
description:	�õ�Unixϵͳ(Զ�̻�)ĳ·���·���ĳ������ʽ���ļ����б�
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::ListWindowsBy( const char *path, const char *listSavePathName, 
		const char *condition, char *errMsg ){
	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	struct sockaddr_in addr; 
	
	unsigned char			*c, *p; 
	
	//CheckPath(path);
	//CheckPath(listSavePathName);
	
CHECK_AGAIN:	

	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );     


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                      
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData );  


	strcpy( errMsg, "Fail to listen!\n" );  
                                              
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//����Ϣ��λ
	while( 1 ) {
		if ( IsCmdOk( buf, "200 PROT" ) == 1 ) {
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
				//printf( "cmd: %s\n", buf );
			}while(buf[3] == '-'); 
		}
	}	
	
	memset( buf, 0, 1024 );
	//printf( "aaaaaaaaaaaaaaaaaaaaaaa\n");
	res = ftpcmd(buf,(char *)"CWD %s", path); 
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {


	sprintf( errMsg, "Inexistant directory: %s!\n", path );

		//sprintf( errMsg, "Զ�̻����ϵ�·��%s������!\n", path );
		return FOLDER_NOT_EXIST;	
	}
			
	memset( buf, 0, 1024 );
//	printf( "targetPath: %s\n", targetPath);
	res = ftpcmd(buf,(char *)"LIST"); 
	
///////////////////////////////////////////////receive
//printf( "bbbbbbbbbbbbbbbbbbbbbbb\n");	
	tmp = sizeof(addr); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1){
		//printf("accepting err1!\n");
		//close( iSockData );


	strcpy( errMsg, "Fail to receive data!\n" );

		//strcpy( errMsg, "���շ���������ʧ��!\n" );
		return SEND_CMD_ERR;
	} 
	
	//ɾ��ԭ���ļ��б�
	remove(listSavePathName);
//printf( "cccccccccccccccccccccc\n");	
	//���´����ļ��б�
	int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		//printf("writing err1!\n"); 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//sprintf( errMsg, "Ҫ�����ڱ��ص��ļ�%s�����ʹ�ʧ��!\n", listSavePathName );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
//printf( "ddddddddddddddddddddddd\n");	
	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		//printf( "len = %d\n", len );   
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//��ʼ�ж��Ƿ��������
			if ( IsWindowsZhengZe( buf, condition ) == 1 ){
				write(iSaveFd,buf,strlen(buf)); 
				//printf( "a:%s\n", buf );
			}	
			memset( buf, 0, 1024 );
		}
		memset( cCur, 0, 5 );
	}
	close( iSockData );
	close( m_sockxfer );
	close( iSaveFd );
		
	return 1;	
}


/***************************************************
description:	�жϸü�¼���ļ����Ƿ����Ҫ��(������ʽ)
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::IsUnixZhengZe( const char *buf, const char *condition ){
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
	char		cCondition[200];
	
	memset( cCondition, 0, 200 );
	strcpy( cCondition, condition );

	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 100 );
	memset( cParam2, 0, 100 );
	memset( cParam3, 0, 100 );
	memset( cParam4, 0, 100 );
	memset( cParam5, 0, 100 );
	memset( cParam6, 0, 100 );
	memset( cParam7, 0, 100 );
	memset( cParam8, 0, 100 );
	
	sscanf( buf, "%s %s %s %s %s %s %s %s %s", 
	cParam0, cParam1, cParam2, cParam3, cParam4, cParam5, cParam6, cParam7, cParam8);
	
	//printf( "int 22222222222222222\n" );
	if ( strcmp( cParam0, "total" ) == 0 ) return 0;
	
	if ( strcmp( cParam8, "." ) == 0 ) return 0;

	if ( strcmp( cParam8, ".." ) == 0 ) return 0;
	
	if ( cParam0[0] == 'd' ) return 0;
	//printf( "int 333333333333333\n" );
	if ( IsMatchZhengZe( cParam8, cCondition ) == 1 ) return 1;

}

/***************************************************
description:	�жϸü�¼���ļ����Ƿ����Ҫ��(������ʽ)
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::IsWindowsZhengZe( const char *buf, const char *condition ){
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100]; 		
	char		cCondition[200];
	
	memset( cCondition, 0, 200 );
	strcpy( cCondition, condition );

	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 100 );
	memset( cParam2, 0, 100 );
	memset( cParam3, 0, 100 );
//	memset( cParam4, 0, 100 );
//	memset( cParam5, 0, 100 );
//	memset( cParam6, 0, 100 );
//	memset( cParam7, 0, 100 );
//	memset( cParam8, 0, 100 );
	
	sscanf( buf, "%s %s %s %s", 
	cParam0, cParam1, cParam2, cParam3 );
	
	//if ( strcmp( cParam0, "total" ) == 0 ) return 0;
	
	if ( strcmp( cParam3, "." ) == 0 ) return 0;

	if ( strcmp( cParam3, ".." ) == 0 ) return 0;
	
	if ( strcmp( cParam2, "<DIR>" ) == 0 ) return 0;
	
	if ( IsMatchZhengZe( cParam3, cCondition ) == 1 ) return 1;

}

/***************************************************
description:	�ж�ĳ�ַ����Ƿ����ĳ������ʽ
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::IsMatchZhengZe( char *cOldFileName, char *cCondition ){
	//���ж������Ƿ���������ʽ
	char		*pCur = NULL;
	char		cNewFileName[2000];
	char		cTemp[100];
	int		iLen = 0;
	
	memset( cTemp, 0, 100 );
	memset( cNewFileName, 0, 2000 );
	
	//����cOldFile
	char		cName[2000];
	char		*cFileName = cName;
	memset( cFileName, 0, 2000 );
	strcpy( cFileName, cCondition );
	
	//printf("cFileName=%s\n",cFileName);
	pCur = strchr( cFileName, '*' );
	if ( NULL != pCur ){//����
		//����*�ĵط��Ӹ�.�Է��Ͻ�����ʽ
		pCur = NULL;
		//printf("come here\n");
		while ( ( pCur = strstr( cFileName, "*" ) ) != NULL ){
			iLen = pCur - cFileName;
		//	printf("len: %d\n", iLen);
			memset( cTemp, 0, 100 );
			strncpy(cTemp, cFileName, iLen);
			sprintf( cNewFileName, "%s%s.*", cNewFileName, cTemp  );
			//�ƶ�ָ��
			cFileName = pCur + 1;
			//cFileName += 1;
		}
		
		//�����һ��������cNewName
		strcat( cNewFileName, cFileName );
		//printf( "newName: <%s>\n, fileName: <%s>\n", cNewFileName, cOldFileName );
		
		//�Ա�������ʽ
		regex_t		struPreg;
	
		regcomp(&struPreg, cNewFileName, 0);
		
		size_t		nmatch = 10;
		regmatch_t	pmatch[100];
		pmatch[0].rm_so = -1; 
		pmatch[0].rm_eo = -1;
		
		//pmatch[0].rm_eo = -1;
		regexec(&struPreg, cOldFileName, nmatch, pmatch, 0); 
		int iRtn = ( ( ( pmatch[0].rm_so >= 0 ) && ( pmatch[0].rm_eo > 0 ) ) ? 1 : 0 );
		regfree(&struPreg);
		return iRtn;
		
	} else {//����
		//�Ƚ��Ƿ���ȫƥ��
		//printf( "no *: %s, %s\n", cOldFileName, cCondition );
		return ( strcmp( cOldFileName, cCondition ) == 0 ? 1 : 0 );
	}	
}

/***************************************************
description:	�жϸü�¼���ļ�ʱ���Ƿ���ĳָ��ʱ�䷶Χ��
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::IsUnixBetween( const char *buf, const char *beginTime, 
				const char *endTime, char *errMsg ){
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
	char		cNewDate[20];
	//printf("cc: %s\n", buf);
	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 100 );
	memset( cParam2, 0, 100 );
	memset( cParam3, 0, 100 );
	memset( cParam4, 0, 100 );
	memset( cParam5, 0, 100 );
	memset( cParam6, 0, 100 );
	memset( cParam7, 0, 100 );
	memset( cParam8, 0, 100 );
	
	sscanf( buf, "%s %s %s %s %s %s %s %s %s", 
	cParam0, cParam1, cParam2, cParam3, cParam4, cParam5, cParam6, cParam7, cParam8);
	
	if ( strcmp( cParam0, "total" ) == 0 ) return 0;
	if ( strcmp( cParam8, "." ) == 0 ) return 0;
	if ( strcmp( cParam8, ".." ) == 0 ) return 0;
	if ( cParam0[0] == 'd' ) return 0;
	char		cCurYear[10], cCurDate[20], cCurMonth[10];
	
	//�õ���ǰ���
	memset( cCurDate, 0, 20 );
	memset( cCurYear, 0, 10 );
	GetDate( cCurDate );
	strncpy( cCurYear, cCurDate, 4 );
	
	//�õ�������
	memset( cCurMonth, 0, 10 );
	if ( GetMonthByChr( cParam5, cCurMonth ) != 1 ){


	sprintf( errMsg, "Unrecognized month: %s", cParam5 ); 

		//sprintf( errMsg, "����ʶ����·�%s\n", cParam5 );	
		return 0;
	}
	
	memset( cNewDate, 0, 20 );
	
	//�ж��Ƿ����
	char			*p = NULL;
	p = strstr( cParam7, ":" );

	if ( p ){//�ǽ���
		//������ڱȽ��ַ���		
		sprintf( cNewDate, "%s-%s-%02s %s", cCurYear, cCurMonth, cParam6, cParam7 );
	} else {//���ǽ���
		sprintf( cNewDate, "%s-%s-%02s", cParam7, cCurMonth, cParam6 );
	}
	
	//printf( "%s\n", cNewDate );
	
	
	if ( ( strcmp( cNewDate, beginTime ) >= 0 ) 
		&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//����
		return 1;		
	} else {
		return 0;	
	} 
}

/***************************************************
description:	�жϸü�¼���ļ�ʱ���Ƿ���ĳָ��ʱ�䷶Χ��
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::IsWindowsBetween( const char *buf, const char *beginTime, 
				const char *endTime, char *errMsg ){
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100]; 		
	char		cNewDate[20];
	
	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 100 );
	memset( cParam2, 0, 100 );
	memset( cParam3, 0, 100 );
	
	sscanf( buf, "%s %s %s %s", 
	cParam0, cParam1, cParam2, cParam3 );
	
	if ( strcmp( cParam3, "." ) == 0 ) return 0;

	if ( strcmp( cParam3, ".." ) == 0 ) return 0;
	
	if ( strcmp( cParam2, "<DIR>" ) == 0 ) return 0;
	
	memset( cNewDate, 0, 20 );
	sprintf( cNewDate, "%s %s", cParam0, cParam1 );	
	
	if ( ( strcmp( cNewDate, beginTime ) ) >= 0 
		&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//����
		return 1;		
	} else {
		return 0;	
	} 
}

/***************************************************
description:	ȡ��XXXX-XX-XX��ʽ�ĵ�ǰ����
input:	        
output:		           
return:         	
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::GetDate( char *date ){
	time_t  timer;
	struct tm nowtimer;

    time(&timer);
    nowtimer = *localtime (&timer);
	
	nowtimer.tm_mon++;
    sprintf( date, "%04u-%02u-%02u", nowtimer.tm_year+1900, nowtimer.tm_mon, nowtimer.tm_mday);
	return 1;	
}

/***************************************************
description:	����ĸ�·�ȡ�������·�
input:	        
output:		           
return:         	
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::GetMonthByChr( const char *oldMonth, char *newMonth ){
	if ( strcmp( oldMonth, "Jan" ) == 0 ){//1
		strcpy( newMonth, "01" );
	} else if ( strcmp( oldMonth, "Feb" ) == 0 ){//2
		strcpy( newMonth, "02" );
	} else if ( strcmp( oldMonth, "Mar" ) == 0 ){//3
		strcpy( newMonth, "03" );
	} else if ( strcmp( oldMonth, "Apr" ) == 0 ){//4
		strcpy( newMonth, "04" );
	} else if ( strcmp( oldMonth, "May" ) == 0 ){//5
		strcpy( newMonth, "05" );
	} else if ( strcmp( oldMonth, "Jun" ) == 0 ){//6
		strcpy( newMonth, "06" );
	} else if ( strcmp( oldMonth, "Jul" ) == 0 ){//7
		strcpy( newMonth, "07" );
	} else if ( strcmp( oldMonth, "Aug" ) == 0 ){//8
		strcpy( newMonth, "08" );
	} else if ( strcmp( oldMonth, "Sep" ) == 0 ){//9
		strcpy( newMonth, "09" );
	} else if ( strcmp( oldMonth, "Oct" ) == 0 ){//10
		strcpy( newMonth, "10" );
	} else if ( strcmp( oldMonth, "Nov" ) == 0 ){//11
		strcpy( newMonth, "11" );
	} else if ( strcmp( oldMonth, "Dec" ) == 0 ){//12
		strcpy( newMonth, "12" );
	} else {
		return 0;	
	}
	
	return 1;	
}

/***************************************************
description:	���ĳһԶ��·���µ�ĳһָ��ʱ����ڵ��ļ��б�
input:	      
		oldListFile:		���ļ��б�·��+�ļ���
		newListFile��		���ļ��б�·��+�ļ���
		beginTime:		��ʼ����,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		endTime:		��������,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������  
output:		           
return:         	
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::ListUnixBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, char *errMsg ){
	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	struct sockaddr_in addr; 
	
	unsigned char			*c, *p; 
	
	//CheckPath(path);
	//CheckPath(listFile);
	
CHECK_AGAIN:	

	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );     


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                      
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData ); 


	strcpy( errMsg, "Fail to listen!\n" );  
                                               
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}                                
	}    
	
	//����Ϣ��λ
	while( 1 ) {
		if ( IsCmdOk( buf, "200 PROT" ) == 1 ) {
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
			}while(buf[3] == '-'); 
		}
	}
		
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"CWD %s", path); 
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {


	sprintf( errMsg, "Inexistant directory: %s!\n", path );

		//sprintf( errMsg, "Զ�̻����ϵ�·��%s������!\n", path );
		return FOLDER_NOT_EXIST;	
	}

	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST"); 
		
	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1){
		//printf("accepting err1!\n");
		close( iSockData );


	//strcpy( errMsg, "Fail to receive data!\n" );
	strcpy(errMsg,sz_errorMessage);

		//strcpy( errMsg, "���շ���������ʧ��!\n" );
		return SEND_CMD_ERR;
	} 
	
	//ɾ��ԭ���ļ��б�
	remove( listFile );

	//���´����ļ��б�
	int iSaveFd = open(listFile,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		//printf("writing err1!\n"); 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listFile ); 

		//sprintf( errMsg, "Ҫ�����ڱ��ص��ļ�%s�����ʹ�ʧ��!\n", listFile );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//��ʼ�ж��Ƿ��������
			if ( IsUnixBetween( buf, beginTime, endTime, errMsg ) == 1 ) {
				write(iSaveFd,buf,strlen(buf)); 
			}	
			memset( buf, 0, 1024 );
		}
		
		memset( cCur, 0, 5 );
	}
	
	close( iSockData );
	close( m_sockxfer );
	close( iSaveFd );
		
	return 1;					
}

/***************************************************
description:	���ĳһԶ��·���µ�ĳһָ��ʱ����ڵ��ļ��б�
input:	      
		oldListFile:		���ļ��б�·��+�ļ���
		newListFile��		���ļ��б�·��+�ļ���
		beginTime:		��ʼ����,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		endTime:		��������,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������  
output:		           
return:         	
programmer:	
		��	��
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::ListWindowsBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, char *errMsg ){
	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	struct sockaddr_in addr; 
	
	unsigned char			*c, *p; 
	//CheckPath(path);
	//CheckPath(listFile);
	
CHECK_AGAIN:	

	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );     


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                      
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData );      


	strcpy( errMsg, "Fail to listen!\n" );  
                                          
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//�����ѶϿ����ȴ�1�������
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                        
	}    

	//����Ϣ��λ
	while( 1 ) {
		if ( IsCmdOk( buf, "200 PROT" ) == 1 ) {
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
			}while(buf[3] == '-'); 
		}
	}	
	
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"CWD %s", path); 
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {


	sprintf( errMsg, "Inexistant directory: %s\n", path );

		//sprintf( errMsg, "Զ�̻����ϵ�·��%s������!\n", path );
		return FOLDER_NOT_EXIST;	
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST"); 
	
///////////////////////////////////////////////receive
//printf( "bbbbbbbbbbbbbbbbbbbbbbb\n");	
	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1){
		//printf("accepting err1!\n");
		close( iSockData );


	//strcpy( errMsg, "Fail to receive data!\n" );
	strcpy(errMsg,sz_errorMessage);

		//strcpy( errMsg, "���շ���������ʧ��!\n" );
		return SEND_CMD_ERR;
	} 
	
	//ɾ��ԭ���ļ��б�
	remove( listFile );
//printf( "cccccccccccccccccccccc\n");	
	//���´����ļ��б�
	int iSaveFd = open(listFile,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		//printf("writing err1!\n"); 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listFile ); 

		//sprintf( errMsg, "Ҫ�����ڱ��ص��ļ�%s�����ʹ�ʧ��!\n", listFile );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
//printf( "ddddddddddddddddddddddd\n");	

	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//��ʼ�ж��Ƿ��������
			if ( IsWindowsBetween( buf, beginTime, endTime, errMsg ) == 1 ) {
				write(iSaveFd,buf,strlen(buf)); 
				//printf( "a:%s\n", buf );
			}	
			memset( buf, 0, 1024 );
		}
		memset( cCur, 0, 5 );
	}

	close( iSockData );
	close( m_sockxfer );
	close( iSaveFd );
		
	return 1;					
}




/*****************************************************************
description:	��ȡԶ��Ŀ¼�µ������ļ���Ϣ��������Ϣд���ļ��б���
input:	      
output:		           
return:     1 success, other fail    	
programmer:	
		yangzx
date:		
		2006-05-16
******************************************************************/ 


int CFTPmanager::ScanFile(char * path,char* localPath,char* errMsg)
{
	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	struct sockaddr_in addr; 
	
	unsigned char			*c, *p; 
	
	CheckPath(path);
	CheckPath(localPath);
	
CHECK_AGAIN:	
	//ɾ��ԭ���ļ��б�
	
	
	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) 
	{                                                   
		close( iSockData );

		strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) 
	{         
		close( iSockData ); 

		strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                          
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  
	{                                          
		//close( iSockData );
		close( iSockData );                                                
		//shutdown( isockftp, SHUT_RDWR );    
		strcpy( errMsg, "Fail to listen!\n" );  

		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  
	{//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if ( Reconnect( errMsg ) == 1 )
		{
			goto CHECK_AGAIN;
		} 
		else 
		{
				strcpy( errMsg, "Connection was elimilated!\n" ); 
				
				//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
				return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//����Ϣ��λ
	while( 1 ) 
	{
		if ( IsCmdOk( buf, "200 PORT" ) == 1 ) 
		{
			break;
		} 
		else 
		{
			memset( buf, 0, strlen( buf ) );	
			do 
			{ 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
			}while(buf[3] == '-'); 
		}
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"CWD %s", path);
	if(res==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		close(iSockData);
		return -1;
	}
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) 
	{
		close( iSockData );


		sprintf( errMsg, "Inexistant directory: %s \n", path );


		//sprintf( errMsg, "Զ�̻�����Ҫ�õ��ļ��б��·��%s������ \n", path );
		return FOLDER_NOT_EXIST;
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST"); // 150 is ok
	if(res!=1)
	{
		close(iSockData);
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		else
		{
			strcpy(errMsg,"LIST  error");
			return 0;
		}
		
	}
	res = IsCmdOk( buf, "150" );
	if ( 1 != res )
	{
		close( iSockData );
		sprintf( errMsg, "Inexistant directory: %s\n", path );

		//sprintf( errMsg, "ȡԶ�̻�����·��%s���ļ��б�ʧ��!\n", path );
		return FOLDER_NOT_EXIST;
	}
	
///////////////////////////////////////////////receive
	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1)
	{
		close( iSockData ); 
		//close( m_sockxfer );
		strcpy(errMsg,sz_errorMessage);
		return RECIEVE_SERVER_MSG_ERR ;
	} 
	
	
	//���´����ļ��б�
	//int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);  
	char tempfile[256];
	++temp_num;

	sprintf(tempfile,"%s/temp_%d",localPath,temp_num);
	
	int tempfd = open(tempfile,O_WRONLY|O_CREAT,0644);                                              
	
	if (tempfd == -1)
	{                                                                                 
		close( iSockData );
		close( m_sockxfer );

		sprintf( errMsg, "Fail to create and open file: %s!\n", tempfile ); 
		//sprintf( errMsg, "Ҫ�����ڱ��ص��ļ�%s�����ʹ�ʧ��!\n", listSavePathName );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
	memset( buf, 0, 1024 );
	int count_flag=0;
int timeout=BLOCK_TIME_OUT_MSEC;
	pollfd spoll[3];
	spoll[0].fd=m_sockxfer;
	spoll[0].events=POLLRDNORM;
	while(1)
	{
		 res=poll(spoll,1,timeout);
    if(res>0)
    {
    		//if(FD_ISSET(m_sockxfer,&fdSet))
    			
    		len=read(m_sockxfer,buf,sizeof(buf));
        if(len>0)
        {
        	write(tempfd,buf,len); 
		    	memset( buf, 0, 1024 );
        }
        else
        {
        	//exit_flag=0;
        	break;
        }
    }
    else
    {
    	if(res==0)
    	{
    		strcpy(errMsg,"read block comes out!");
        close( tempfd );
				close( m_sockxfer );
				close( iSockData );
				unlink(tempfile);
        return SOCKET_BLOCK_READ;
      }
      else
      {
      	//2006-9-20 19:17 added ����EINTR����
      		if(errno==EINTR)
        	{
        		continue;
        	}
        	//end added
      	sprintf(errMsg,"SOCKET_POLL_ERROR,the errno=%d",errno);
      	//printf("%s\n",errMsg);
      	close( tempfd );
				close( m_sockxfer );
				close( iSockData );
				unlink(tempfile);
      	return SOCKET_POLL_ERROR;;
      }
    }
  }
  
	FILE * savefile;
	char buf2[800];
	char s[9][255];
	char curpath[256];
	char fullfilename[256];
	savefile=fopen(tempfile,"r");
	if(savefile==NULL)
	{
		sprintf(errMsg,"open file %s error!",tempfile);
		return OPEN_SOURCE_FILE_ERR;
	}
	char strlog[500];
	char str[1024];
	while(fgets(buf2,800,savefile)!=NULL)
	{
		sscanf(buf2,"%s %s %s %s %s %s %s %s %s",s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7],s[8]);
		if(buf2[0]=='t')
			continue;
		if(buf2[0]=='d')
		{
			if(s[8][strlen(s[8])-1]==13)//remove CR
				s[8][strlen(s[8])-1]=0;
			if( strcmp(s[8],".")==0 || strcmp(s[8],"..")==0 )
			{
					continue;
			}
			else
			{
				sprintf(curpath,"%s/%s",path,s[8]);
				res=ScanFile(curpath,localPath,strlog);// scan curpath, find all files and store file info in file list
				if(res!=1)
				{
					strcpy(errMsg,strlog);
					if(savefile!=NULL)
						fclose(savefile);
					remove(tempfile);
					return res;
				}
			}
		}
		else
		{
			sprintf(fullfilename,"%s/%s",path,s[8]);
			sprintf(str,"%s %s %s %s %s %s %s %s %s\n",s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7],fullfilename);
			fwrite(str,sizeof(char),strlen(str),filelist);
		}
		
	}
	if(savefile!=NULL)
		fclose(savefile);	
	
	remove(tempfile);
	//close( iSaveFd );
	
	
//	memset( buf, 0, 1024 );
//	do { 
//		if (fgets(buf,1024,m_ftpio) == NULL){ 
//			break; 
//		}
//	}while(buf[3] == '-'); 
//
	return 1;
}


/***************************************************
description:	���path�����path��'/'������ȥ��'/'
input:	        
		path:		Ҫ����path
return:     ��    	
programmer:	
		yangzx
date:		
		2005-08-26
*****************************************************/ 

void CFTPmanager::CheckPath(char * path)
{
	int len=strlen(path);
	if(path[len-1]=='/')
		path[len-1]=0;
}

/***************************************************
description:	�õ�Unixϵͳ(Զ�̻�)ĳ·���·���ĳ������ʽ���ļ����б�
input:	        
		path:			Զ�̻�·��
		listSavePathName��	������ļ��б���
		condition��		������ʽ����
output:		           
return:         	
		0:			ʧ	��
		1��			��	��
		-17:����������Դ�ļ�������
programmer:	
		yangzx
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::ListBy( const char *path, const char *listSavePathName, 
		const char *condition, char *errMsg ){
	int 		res = -1;
	int 		len = -1;
	char		buf[1024];
	struct sockaddr_in addr; 
	
	unsigned char			*c, *p; 
	
	//CheckPath(path);
	//CheckPath(listSavePathName);
	
	
CHECK_AGAIN:	

	memset( buf, 0, 1024 ); 

	//�������ݽ���socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "���շ��������ݵ�SOCKET����ʧ��!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData );         


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                  
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		close( iSockData );  


	strcpy( errMsg, "Fail to listen!\n" );  
                                              
		//strcpy( errMsg, "���ܷ��������ݵ�SOCKET����ʧ��!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  
	{//�����ѶϿ����ȴ�1������� 
		close( iSockData ); 
		if(res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		 
		if ( Reconnect( errMsg ) == 1 )
		{
			goto CHECK_AGAIN;
		} 
		else 
		{
			
				strcpy( errMsg, "Connection was elimilated!\n" ); 
			

			//strcpy( errMsg, "��FTP�������ϵ������ѶϿ������Զ�������ʧ��!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//����Ϣ��λ
	/*while( 1 ) {
		if ( IsCmdOk( buf, "200 PROT" ) == 1 ) {
			break;
		} else {
			memset( buf, 0, strlen( buf ) );	
			do { 
				memset( buf, 0, 1024 );
				if (fgets(buf,1024,m_ftpio) == NULL) 
					return 0;          
			}while(buf[3] == '-'); 
		}
	}	*/
	
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"CWD %s", path); 
	if(res==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		close(iSockData);
		return -1;
	}
		 
 	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {

		close(iSockData);
			sprintf( errMsg, "Inexistant directory: %s!\n", path );

		//sprintf( errMsg, "Զ�̻����ϵ�·��%s������!\n", path );
		return FOLDER_NOT_EXIST;	
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST %s",condition);
	if(res==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		close(iSockData);
		return -1;
	}
		 
	
///////////////////////////////////////////////receive

	tmp = sizeof(addr); 
	//int m_sockxfer = _accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	int m_sockxfer = Accept(iSockData,(struct sockaddr *)&addr,&tmp); 
	if (m_sockxfer == -1){
		close( iSockData );

		
			//strcpy( errMsg, "Fail to receive data!\n" );
	 		strcpy(errMsg,sz_errorMessage);
		//strcpy( errMsg, "���շ���������ʧ��!\n" );
		return SEND_CMD_ERR;
	} 
	
	//ɾ��ԭ���ļ��б�
	remove(listSavePathName);
	//���´����ļ��б�
	int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//printf( errMsg, "Ҫ�����ڱ��ص��ļ�%s�����ʹ�ʧ��!\n", listSavePathName );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
	memset( buf, 0, 1024 );
	while ((len=read(m_sockxfer,buf,sizeof(buf)))>0) 
	{ 
		if (buf[0]!='d'&& buf[0]!='-' )
		{
			close( iSockData );
			close( m_sockxfer );
			close( iSaveFd );
			strcpy(errMsg,"file not found");
			return SOURCE_FILE_NOT_EXIST;
		}	
		write(iSaveFd,buf,len); 
	}
	
	close( iSockData );
	close( m_sockxfer );
	close( iSaveFd );
		
	return 1;	
}

int CFTPmanager::my_lock(int iFd)
{
	struct flock s_lock;
	
	s_lock.l_type=F_WRLCK;
	s_lock.l_whence=SEEK_SET;
	s_lock.l_start=0;
	s_lock.l_len=0;
	
  if(fcntl(iFd,F_SETLK,&s_lock)==-1)
	{
		return -1;
	}
	return 0;
}

int CFTPmanager::my_unlock(int iFd)
{
	struct flock s_lock;
	
	s_lock.l_type=F_UNLCK;
	s_lock.l_whence=SEEK_SET;
	s_lock.l_start=0;
	s_lock.l_len=0;
	
  if(fcntl(iFd,F_SETLK,&s_lock)==-1)
	{
		return -1;
	}
	return 0;
}

int _accept(int s, struct sockaddr *addr, int *addrlen)
{
#ifdef _LINUXOS
	return accept(s, addr, (socklen_t*)addrlen);
#else
	return accept(s, addr, addrlen);
#endif
}


int _getsockname(int s, struct sockaddr *name, int *namelen)
{
#ifdef _LINUXOS
	return getsockname(s, name, (socklen_t*)namelen);
#else
	return getsockname(s, name, namelen);
#endif
}




bool CFTPmanager::IsReadyToRead(int SocketHandle)
{
	fd_set ReadHandle;
	struct timeval TimeOut;
	int iRet=0;
	FD_ZERO(&ReadHandle);
	FD_SET(SocketHandle,&ReadHandle);

	TimeOut.tv_sec=BLOCK_TIME_OUT_SEC;
	TimeOut.tv_usec=0;

	while(1) 
	{
		iRet=select(SocketHandle + 1,&ReadHandle,NULL,NULL,&TimeOut);
		if ( iRet> 0)
		{
			return true;
		}
		else
		{
			if(iRet==-1 && errno==EINTR)
			{
				continue;
			}
			return false;
		}
	}
}

bool CFTPmanager::IsReadyToWrite(int SocketHandle)
{
	fd_set WriteHandle;
	struct timeval TimeOut;
	int iRet=0;
	FD_ZERO(&WriteHandle);
	FD_SET(SocketHandle,&WriteHandle);

	TimeOut.tv_sec=BLOCK_TIME_OUT_SEC;
	TimeOut.tv_usec=0;
	while(1)
	{
		iRet=select(SocketHandle + 1,NULL,&WriteHandle,NULL,&TimeOut);
		if ( iRet> 0)
		{
			return true;
		}
		else
		{
			if(iRet==-1 && errno==EINTR)
			{
				continue;
			}
			return false;
		}
	}
}



//added 2006-8-10 8:40 nonblocking accept()
int CFTPmanager::Accept(int fd, struct sockaddr *addr, int *addrlen)
{
	int flags;
	char errInfo[300];
	memset(errInfo,0,sizeof(errInfo));
	int errorNo=0;
	int iRet=0;
	char * chp=NULL;
  // Set a socket as nonblocking 
  /*flags = fcntl (fd, F_GETFL, 0)
	if( flags < 0)
	{
		strcpy(sz_errorMessage,"F_GETFL error");
		return -1;
	}
	
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
  {
  	strcpy(sz_errorMessage,"F_SETFL error");
  	return -1;
  }*/
  //use poll to realize nonblocking accept()
	int fd2=0;
	int timeout=BLOCK_TIME_OUT_SEC;
	pollfd sPoll[1];
	sPoll[0].fd=fd;
	sPoll[0].events=POLLRDNORM;
	
	while(1)
	{
		iRet=poll(sPoll,1,timeout);
		
		if(iRet>0)
		{
			#ifdef _LINUXOS
		 		fd2=accept(fd, addr, (socklen_t*)addrlen);
			#else
		 		fd2=accept(fd, addr, addrlen);
	    #endif
			//fd2=accept(fd,addr,addrlen);
			if(fd2==-1)
			{
				errorNo=errno;
				
				//iRet=strerror_r(errno,errInfo,sizeof(errInfo));
				//chp=strerror_r(errno,errInfo,sizeof(errInfo));
				chp=strerror(errno);
				if(chp!=NULL)
				{
					sprintf(sz_errorMessage,"accept error, errno=%d",errorNo);
					return -1;
				}
				else
				{
					sprintf(sz_errorMessage,"accept error,errno=%d,errInfo:%s",errno,errInfo);
					return -1;
				}
			}
			else
			{
				return fd2;
			}
		}
		else
		{
			if(iRet==0)
			{
				strcpy(sz_errorMessage,"accept time out");
				return -1;
			}
			else
			{
				//2006-9-20 19:38 added by yangzx ����EINTR����
				if(errno==EINTR)
				{
					continue;
				}
				errorNo=errno;
				//iRet=strerror_r(errno,errInfo,sizeof(errInfo));
				chp=strerror(errno);
				if(chp!=NULL)
				{
					sprintf(sz_errorMessage,"accept: poll() error,errno=%d");
					return -1;
				}
				else
				{
					sprintf(sz_errorMessage,"accept:poll() error,errno=%d,errInfo:%s",errno,errInfo);
					return -1;
				}
			}
		}
	}//end while
}
	
  
			
		
			



