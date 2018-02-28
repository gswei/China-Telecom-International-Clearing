/***********************************
类	名：		CFTPmanager
用	途：		对FTP服务器上的文件进行：上传，下载，得到文件列表，操作远程文件和文件夹。
作	者：		安	坤
version:    1.0.3.1
update history：
by yangzx 2006.05.16 修改List接口支持获取远程dir下的所有文件列表包括子目录下的文件，增加private method ScanFile()
by yangzx 2006.05.20 增加接口SizeFile()取文件大小，	为保证与旧程序的兼容性，增加ListAllFile接口，以支持获取远程dir下的所有文件列表包括子目录下的文件，int ListAllFile( const char *path, const char *listSavePathName, char *errMsg );原有的List接口功能不变，只取当前目录下的文件，不取子目录下的文件。
by yangzx 2006.05.21 使用select函数解决block问题																	
by yangzx 2006.06.01 因为在hp的机器上使用select函数有问题，暂时注释掉使用select函数的部分，恢复以前的upload和download函数
by yangzx 2006.06.09 12:00 修改ListBy的bug，注释掉所有使用的select函数的部分
by yangzx 2006.06.12 使用 poll() 函数代替 select()函数，select()函数在hp的机器上被程序调用时不稳定
by yangzx 2006.07.06 修改ListAllFile()和ScanFile()函数，修改输出错误信息,修改DivFileName( listSavePathName,filePath, fileName )函数在拷贝路径到filePath的bug，filePath有时会没有字符串终止符 0
by yangzx 2006.07.31 修改ftpcmd()在read()和write()前，使用IsReadyToRead()和IsReadyToWrite()封装
by yangzx 2006.08.05 在函数CFTPmanager()里增加signal(SIGPIPE,SIG_IGN)
by yangzx 2006.08.10 使用进行非阻塞封装的Accept()代替直接调用accept()
by yangzx 2006.08.30 解决Connect()里，如果发送ftp用户名、密码阻塞时，没有清空m_ftpio指针的bug,在shutdown()函数之后还要调用close()，shutdown()函数只是关闭连接，升级版本号为1.0.2
by yangzx 2006.09.20 在调用select() 或 poll()时如果系统产生real time signal 会使函数报错返回，错误代码errno=EINTR;修改代码在调用select()和poll()时，忽略EINTR错误
by yangzx 2006.10.11 debug DivFileName()，当文件在根路径时，如/test.dat，该函数无法正确分解出文件的路径和文件名，版本号升为1.0.3.1，新程序已在市话东莞132.119.200.60上使用,1.0.5只在计费部数据下发接口机上使用过，有待全面测试。
by zhanggq 2007.4.19 修改超时时间，由60S改为300S
by zhanggq 2008.1.10 函数SizeFile()增加WINOWS目录的支持。
by zhanggq 2008.4.09 函数SizeFile()增加对226返回值得放行。.
by zhanggq 2008.5.13 增加检测文件是否存在的函数
by zhanggq 2008.5.20 SIZEFILE增加对被动模式的支持。
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
description:	连接FTP服务器
input:	                 
		sz_serverIp;			FTP服务器IP              
		sz_loginName;			FTP服务器登录名          
		sz_loginPassword;	FTP服务器登录密码		
output:         
		errMsg：        		错误信息
return:         
		1：				成功
		其他：				参考FTPmanager.h里的错误信息的定义
		
programmer:	安	坤
date:		2005-08-16
*****************************************************/
int CFTPmanager::Connect(const char *pch_serverIp,const char *pch_loginName, const char *pch_loginPassword, char *errMsg,char c_PsvFlag,int iPort)
{
	//保存服务器信息到成员函数
  strcpy(sz_serverIp,pch_serverIp);
  strcpy(sz_loginName,pch_loginName);
	strcpy(sz_loginPassword,pch_loginPassword);
	iServerPort=iPort;
  c_PassiveFlag=c_PsvFlag;
	//开始新连接前要把指针FILE * m_ftpio=NULL,

	// updated by wh 2008-09-22,
	// 如果FILE * m_ftpio不为空，说明m_ftpio以前成打开过，所以先释放句柄
	if (m_ftpio !=NULL)
	{
		fclose(m_ftpio);
		m_ftpio=NULL;
   }

	//开始连接
	struct sockaddr_in s_sockaddr; 
	unsigned long iHostIp; 
	int	len, tmp, res; 
	int	retval = 0; 
	int	iSaveFd; 
	struct hostent *p_hosttent; 
	char sz_returnBuf[1024];
	
	iHostIp=inet_addr(pch_serverIp); 
	
	//将IP转换成long型
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
	//建立连接
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
	
	
	//发送空字串
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
		
	//发送登录名
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
		 
		
	//发送登录密码
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

		//strcpy( errMsg, "接受服务器返回信息失败!\n" );
		return RECIEVE_SERVER_MSG_ERR;
	}
	
	res = IsCmdOk( sz_returnBuf, "230" );
	
	//检查登录是否成功
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


	//设定模式
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
description:	连接FTP服务器
input:	        
		serverInfo:		服务器信息结构体
output:		           
		errMsg：		错误信息                 
return:         	
		和Connect的另一重载函数相同
programmer:	
		安	坤
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
description:	断开与FTP服务器的连接
input:	        
output:         
return:         
		0：		失败
		1：		成功
programmer:	安	坤
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
description:	自动重连接服务器
input:	        
output:         
return:         
		1：			成功
		其他：			参考FTPmanager.h里的错误信息的定义
		
programmer:	安	坤
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
description:	得到FTP服务器上指定目录(path参数指定路径下)的文件列表,存放在listSavePathName指定的文件中
input:	        
		path:			FTP服务器上要取得文件列表的路径
		listSavePathName:	取得的文件列表保存在本地文件中，此参数即为该文件的路径+文件名
output:        
		errMsg：		错误信息
return:         
		1：			成功
		其他：			参考FTPmanager.h里的错误信息的定义
		
programmer:	安	坤
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
	//删除原有文件列表
	remove(pch_listFile);

	memset(sz_buff,0,1024); 

	//建立数据接收socket
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
	{//连接已断开，等待1秒后重连 
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

	//将信息定位
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
	
	//重新创建文件列表
	int iSaveFd=open(pch_listFile,O_WRONLY|O_CREAT,0644);
	if (iSaveFd == -1)
	{
		close(iSockData);
		close(m_sockxfer);
	  sprintf(errMsg, "Fail to create and open file: %s!\n",pch_listFile);
		return TARGET_FILE_CREATE_OPEN_ERR;
	}
	memset(sz_buff,0,1024);
	
	//begin 2006.06.13 添加，防止block
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
      	//2006-9-20 19:17 added 忽略EINTR错误
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
description:	从FTP服务器上下载文件
input:	        
		localFile:		文件下载后在本地的存放路径+文件名
		remoteFile:		FTP服务器上要被下载的源文件路径+文件名
		coverFlag:		如果远程机器上要上传的文件已经存在，则有两种处理方法：
						TARGET_COVER：		覆盖已经有的文件
						TARGET_STORE：		将已经有的文件重命名为“原文件名.tmp”,然后再下载文件
		sendMode:		传输模式
						MODE_BIN:		传输2进制数据
						MODE_ASC:		传输10进制数据
		cStorePath:		下载完成后,源文件的3种处理方式:1.不处理;2.删除;3.备份到另一目录
						SOURCE_IGNORE:		不处理
						SOURCE_DELE:		  删除
						其他:			        备份路径(不包含文件名)
output:         
		errMsg:			错误信息
return:         
		1：			成功
		其他：			参考FTPmanager.h里的错误信息的定义
		
programmer:	安	坤
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

	//检查本地目标文件是否已经存在
	if(IsFileExist(pch_localFile)!=0)
	{
		if (iCoverFlag==TARGET_COVER)
		{//覆盖
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

	//检查条件：本地目标文件路径（不存在则新建）
	chkAllDir(sz_targetPath);

	//建立数据接收socket 
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
	{//连接已断开，等待1秒后重连 
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

	while( 1 )//将信息定位
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

	//将FTP服务器进入到目标路径
	if (sz_sourcePath!=NULL)
	{
		//设定当前路径到跟目录
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
	{//10进制 A
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
	{//2进制 I
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
	//如果该临时文件存在，则删除原有临时文件
	//unlink(sz_TmpPathFile);
	//重新创建文件列表
	
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
	//接收数据
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
	{//10进制 A
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
        	//2006-9-20 19:17 added 忽略EINTR错误
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
	{//2进制 I
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
        	//2006-9-20 19:17 added 忽略EINTR错误
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

	//下载完成后将临时文件名改为正式文件名
	//if (rename(sz_TmpPathFile,pch_localFile)!=0)
	//{
	//	return RENAME_FILE_ERR;	
	//}
	
	//处理源文件
	if (strcmp(cRmFlag,SOURCE_IGNORE)==0)
	{//不处理
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
description:	从FTP服务器上下载文件(包括解密/压)
input:	        
		localFile:		文件下载后在本地的存放路径+文件名
		remoteFile:		FTP服务器上要被下载的源文件路径+文件名!
		coverFlag:		如果远程机器上要上传的文件已经存在，则有两种处理方法：
						TARGET_COVER：		覆盖已经有的文件
						TARGET_STORE：		将已经有的文件重命名为“原文件名.tmp”,然后再下载文件
		sendMode:		传输模式
						MODE_BIN:		传输2进制数据
						MODE_ASC:		传输10进制数据
		cStorePath:		下载完成后,源文件的3种处理方式:1.不处理;2.删除;3.备份到另一目录
						SOURCE_IGNORE:不处理
						SOURCE_DELE:	删除
						其他:			备份路径(不包含文件名)
		compress:		下载完后是否解压
						COMPRESS_Y:		要解压
						COMPRESS_N:		不解压
		pass:		下载完后是否解密:
						PASS_N:			  不解密
						其他：			  解密时的密匙
output:
		errMsg:			错误信息
return:         
		1：			成功
		其他：			参考FTPmanager.h里的错误信息的定义
programmer:	安	坤
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

	//下载过程中保存为临时文件
	char cTmpPathName[1024];
	memset(cTmpPathName,0,1024);
	sprintf(cTmpPathName,"%s/~%s",targetPath,targetFile);

	struct sockaddr_in addr; 
	unsigned char *c,*p; 
	
	memset(buf,0,1024); 

CHECK_AGAIN:	

	//检查本地目标文件是否已经存在
	if ( IsFileExist( localFile ) != 0 )
	{
		if ( coverFlag == TARGET_COVER )
		{//覆盖
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


	//检查条件：本地目标文件路径（不存在则新建）
	chkAllDir( targetPath );

	//建立数据接收socket 
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );


		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData ); 

	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                              
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
	if (listen(iSockData,1) == -1)  {                                          
		close( iSockData );   


	strcpy( errMsg, "Fail to listen!\n" );  

		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}

	tmp = sizeof(addr);
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port; 

	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",c[0],c[1],c[2],c[3],p[0],p[1]);         
	if (res != 1)  {//连接已断开，等待1秒后重连 
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

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}
	}    
	
	//将信息定位
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
	
	//将FTP服务器进入到目标路径
	if ( sourcePath != NULL ){
		//设定当前路径到跟目录
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

	if ( sendMode == 10 ) {//10进制 A
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
	} else if ( sendMode == 2 ) {//2进制 I
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

		//sprintf( errMsg, "远程机器上要下载的文件%s不存在!\n", remoteFile );
		return SOURCE_FILE_NOT_EXIST;
	}

	//如果该临时文件存在，则删除原有临时文件
	remove( cTmpPathName );
	//重新创建文件列表
	int iSaveFd = open(cTmpPathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		close( iSockData );
		close( iSaveFd );

	sprintf( errMsg, "Fail to create and open file: %s!\n", localFile );

		//sprintf( errMsg, "要保存在本地的目标文件%s创建和打开失败!\n", localFile );
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}

	//接收数据
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
	{//10进制 A
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
        	//2006-9-20 19:17 added 忽略EINTR错误
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
	{//2进制 I
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
        	//2006-9-20 19:17 added 忽略EINTR错误
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

	//解压缩
	if ( strcmp( compress, COMPRESS_Y ) == 0 ) {
		com.uncompress( cTmpPathName, cPreFile );
		remove( cTmpPathName );
	} else {
		rename( cTmpPathName, cPreFile );
	}
	
	//cPreFile是解压缩后的文件
	
	char		cPassFile[300];
	memset( cPassFile, 0, 300 );
	
	//提取真正的文件名
	int iPos = 0;
	iPos=strrncspn( cTmpPathName, '.', 3 );
	//if(iPos<0) return 0;
	strncpy( cPassFile , cTmpPathName, iPos );
	
	//解密
	if ( strcmp( pass, PASS_N ) != 0 ) {//加密了
		char		cPass[100];
		memset( cPass, 0, 100 );
		strcpy( cPass, pass );
		encc.unEncrypt( cPreFile, cPassFile, cPass );
		remove( cPreFile );
	} else {//没加密	
		rename( cPassFile, cPreFile );
	}
	
	//cPassFile是解压缩和解密后的文件名,同时也已经去掉了文件大小和其他信息,是真正的文件名
	
	//核对文件大小
	//提取文件大小
	int		iFiSize = 0;
	if ( strcmp( pass, PASS_N ) != 0 ) {//加密了
		//提取文件大小
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
		//提取文件大小
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
	
	//比较文件大小
	if ( iReSize != iFiSize ) {
		//theLog<<"下载文件"<<downloadPath<<"/"<<cParam8<<"失败,失败原因:下载前后文件大小不符!"<<endi;
		remove( cPassFile );
		return FILE_SIZE_ERR;
	}	
	
	//下载完成后将临时文件名改为正式文件名
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
	
	//处理源文件
	if ( strcmp( cRmFlag, SOURCE_IGNORE ) == 0 ){//不处理
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
description:	从FTP服务器上下载文件
input:	        
		sendInfo:		传输控制结构体
output:		           
		errMsg：		错误信息                 
return:         	
		和Download的另一重载函数相同
programmer:	
		安	坤
date:		
		2005-08-30
*****************************************************/ 
int CFTPmanager::Download( SEND_INFO sendInfo, char *errMsg )
{
	return (Download(sendInfo.localFile,sendInfo.remoteFile,sendInfo.coverFlag,errMsg,sendInfo.sendMode,sendInfo.cStorePath));
}
/***************************************************
description:	上传文件到FTP服务器上
input:	        
		pch_localFile:	要被上传本地源文件的路径+文件名
		pch_remoteFile:	文件上传到FTP服务器上后在服务器上保存的路径+文件名
		coverFlag:		  文件覆盖标志，如果FTP服务器上该文件已存在，则
			TARGET_COVER：覆盖已经有的文件
			TARGET_STORE：将已经有的文件重命名为“原文件名.tmp”,然后再下载文件
		sendMode：		  文件传输模式
			MODE_BIN：		2进制
			MODE_ASC：		文本
		cStorePath:		  源文件的背份路径
			SOURCE_IGNORE:不背份
			SOURCE_DELE：	删除源文件
			其他：			  备份在该路径下(此路径必须为备份路径)
output: 
		errMsg：		错误信息        
return:         
		1:			成功
		其他：			参考FTPmanager.h里的错误信息的定义
		
programmer:	安	坤
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

	//检查条件：本地文件是否存在
	if(IsFileExist(sz_sourceFile)!=1)
	{
	  sprintf(errMsg,"Inexistent file: %s\n",sz_sourceFile);
		return SOURCE_FILE_NOT_EXIST;	
	}

	//不管目标文件是否存在，都试图作以下操作
	if(iCoverFlag==TARGET_COVER)//覆盖
	{
		RmFile(pch_remoteFile,errMsg);
	} 
	else if(iCoverFlag==TARGET_STORE)//备份
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

	//建立数据接收socket
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
	{//连接已断开，等待1秒后重连 
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

	//将信息定位
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
	
	//将FTP服务器进入到目标路径
	if(sz_targetPath!=NULL)
	{
		//设定当前路径到跟目录
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
	
	//打开本地源文件
	int iSaveFd=open(sz_sourceFile,O_RDONLY,0644);
	if(iSaveFd==-1)
	{
		close( iSockData ); 
	  sprintf(errMsg,"Inexistant file: %s!\n",sz_sourceFile);
		return OPEN_SOURCE_FILE_ERR;
	} 

	if(iSendMode==10) 
	{//10进制 A
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
	{//2进制 I
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

	//发送数据
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
        	//2006-9-20 19:17 added 忽略EINTR错误
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

	//处理源文件
	if(strcmp(pch_storePath,SOURCE_IGNORE)==0)
	{//不处理
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
description:	上传文件到FTP服务器上
input:	        
		sendInfo:		传输控制结构体
output:		           
		errMsg：		错误信息                 
return:         	
		和Upload的另一重载函数相同
programmer:	
		安	坤
date:		
		2005-08-30
*****************************************************/ 
int CFTPmanager::Upload( SEND_INFO sendInfo ,char *errMsg )
{
	return(Upload(sendInfo.localFile,sendInfo.remoteFile,sendInfo.coverFlag,errMsg,sendInfo.sendMode,sendInfo.cStorePath));
}


/***************************************************
description:	删除远程文件
input:	        
		filePathName:		要被删除的远程文件路径+文件名
output:		
		errMsg：		错误信息
return:         	
		1:			成功
		其他:			参考FTPmanager.h里的错误信息的定义
programmer:	
		安	坤
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
	
	//信息定位
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
description:	移动远程文件
input:	        
		sourceFilePathName:	要被移动的源文件路径+文件名
		targetFilePathName：	移动后保存的目标文件路径+文件名名
output:		
		errMsg：		错误信息                 
return:         	
		1:			成功
		其他:			参考FTPmanager.h里的错误信息的定义
programmer:	
		安	坤
date:		
		2005-08-26
*****************************************************/ 
int CFTPmanager::MoveFile(const char *pch_sourceFile,const char *pch_targetFile,char *errMsg)
{
	int	iRet=0;

  char sz_buff[1024];
	
CHECK_AGAIN:
	
	//信息定位
	Position();
	
	iRet=ftpcmd(sz_buff,(char *)"RNFR %s",pch_sourceFile);//RNFR--RENAME FROM    		
	if(iRet==-1)
	{
		strcpy(errMsg,sz_errorMessage);
		return -1;
	}
	
	//将信息定位
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
	
	if (iRet!=1)//第一次改名失败
	{
		iRet=ftpcmd(sz_buff,(char *)"RNFR %s",pch_targetFile);//检测目标文件是否存在
		if(iRet==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		iRet=IsCmdOk(sz_buff,"350");
		if( iRet==1 )//如果目标文件存在先删除
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
description:	获取远程目录下的所有文件信息，并把信息写到文件列表中
							在sun机器上没有ftp的SIZE命令
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

	//建立数据接收socket
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
	{//连接已断开，等待1秒后重连 
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

	//将信息定位
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
	
	//信息定位
	Position();

	iRet=IsCmdOk(sz_buff,"350");

	iRet=ftpcmd(sz_buff,(char *)"RNFR %s",pch_sourceFile);//检测目标文件是否存在
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
description:	发送命令到FTP服务器
input:	        
		returnBuf:	服务器返回的信息	
		fmt:		格式化的命令        
		
output:         
return:         
		0：		失败
		1：		成功
programmer:	安	坤
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
description:	在每次操作开始前自动定位返回信息
input:	      
output:		           
return:         	
programmer:	
		安	坤
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

		//再读一次信息
		do
		{ 
			memset(sz_Buff,0,1024);
			if(fgets(sz_Buff,1024,m_ftpio)==NULL) 
				return;          
		}while(sz_Buff[3]== '-'); 
	}	
}

/***************************************************
description:	分析buf判断命令执行是否成功
input:	        
		buf:			执行命令后返回的字符串
		okNum:			命令执行成功时返回的Num    
return:         
		1:			执行成功
		0:			执行失败
		
programmer:	安	坤
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
description:	得到FTP服务器上指定目录(path参数指定路径下)的文件列表,包括子目录下的文件，文件名是带路径的全名，存放在listSavePathName指定的文件中
input:	        
		path:			FTP服务器上要取得文件列表的路径
		listSavePathName:	取得的文件列表保存在本地文件中，此参数即为该文件的路径+文件名
output:        
		errMsg：		错误信息
return:         
		1：			成功
		其他：			参考FTPmanager.h里的错误信息的定义
		
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
	//删除原有文件列表
	remove(listSavePathName);
	
	memset( buf, 0, 1024 ); 

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData ); 


	strcpy( errMsg, "Fail to bind port for socket!\n" );  

                                          
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//close( iSockData );
		close( iSockData );                                                
		//shutdown( isockftp, SHUT_RDWR );    


	strcpy( errMsg, "Fail to listen!\n" );  
  
		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//连接已断开，等待1秒后重连 
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
			
			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//将信息定位
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

		//sprintf( errMsg, "远程机器上要得到文件列表的路径%s不存在!\n", path );
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
		
		//sprintf( errMsg, "取远程机器上路径%s的文件列表失败!\n", path );
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
	
	
	//重新创建文件列表
	//int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);  
	char tempfile[256];
	sprintf(tempfile,"%s_temp",listSavePathName);
	
	int tempfd = open(tempfile,O_WRONLY|O_CREAT,0644);                                              
	
	/*if (iSaveFd == -1)
	{                                                                                 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//sprintf( errMsg, "要保存在本地的文件%s创建和打开失败!\n", listSavePathName );                                                                      
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
      	//2006-9-20 19:17 added 忽略EINTR错误
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
description:	切换FTP服务器上的当前路径到到某一指定路径路径
input:	        
		path:			要切换到的路径(可以是绝对和相对路径)
output:         
		errMsg:			错误信息
return:         
		1:			成功
		其他：			参考FTPmanager.h里的错误信息的定义
		
programmer:	安	坤
date:		2005-08-23
*****************************************************/ 
int CFTPmanager::ChDir( const char *path, char *errMsg ){
	char		buf[1024];
	
CHECK_AGAIN:

	//信息定位
	Position();

	memset( buf, 0, 1024 );
	int res = ftpcmd(buf, (char *)"CWD %s", path); 
	res = IsCmdOk( buf, "250" );	
	return res;
}

/***************************************************
description:	在某一路径内创建一路径
input:	        
		newDir:			要创建的路径(可以是绝对和相对路径)
					注意：末尾要加不加“/”都行
output:         
		errMsg:			错误信息
return:         
		1:			成功
		其他：			参考FTPmanager.h里的错误信息的定义
		
programmer:	安	坤
date:		2005-08-23
*****************************************************/ 
int CFTPmanager::MkDir( const char *newDir, char *errMsg ){
	char		buf[1024];
	char		newFolder[200], newFolder2[200], path[1000];
	memset( newFolder, 0, 200 );
	memset( newFolder2, 0, 200 );
	memset( path, 0, 1000 );
	
	//信息定位
	Position();
	
	//判断结尾有无“/”
	if ( '/' != newDir[ strlen( newDir ) - 1 ] ){
		//真正分解析出来
		DivFileName( newDir, path, newFolder );		
	} else {		
		//去掉结尾的“/”
		DivFileName( newDir, path, newFolder );
		strcpy( newFolder2, path );
		memset( newFolder, 0, 200 );
		memset( path, 0, 1000 );
		//真正分解析出来
		DivFileName( newFolder2, path, newFolder );	
	}
	

CHECK_AGAIN:

	memset( buf, 0, 1024 );
	int res = ftpcmd(buf, (char *)"CWD %s", path); 
	if ( res != 1 ) {//连接已断开，等待1秒后重连 
		if( res==-1)
		{
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}
	}
	
	res = IsCmdOk( buf, "250" );
	if ( 1 != res ) {


	sprintf( errMsg, "Inexistant path: %s\n", path );
		
		//sprintf( errMsg, "远程机器上要创建新路径的路径%s不存在!\n", path );
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
	if ( 1 == res ) {//存在
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
//得到文件大小
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
description:	检测远程文件或路径是否存在
input:	        
		filePathName :			要被检测的文件(路径+文件名)或路径
output:         
return:         
		<0:		出错
		0：		不存在
		1：		存在
programmer:	安	坤
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

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );       

	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                    
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData ); 


	strcpy( errMsg, "Fail to listen!\n" );  
                                               
		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);             
	                                                  
	if (res != 1)  {//连接已断开，等待1秒后重连 
		close( iSockData ); 
		if(res==-1)
		{//发生read或write阻塞，或错误，返回上一层
			strcpy(errMsg,sz_errorMessage);
			return -1;
		}     
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                        
	}    
	
	//将信息定位
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

		//sprintf( errMsg, "检查文件是否存在时，远程机器上要得到文件列表的路径%s不存在!\n", filePath );
		return FOLDER_NOT_EXIST;	
	}
			
	memset( buf, 0, 1024 );
	res = ftpcmd(buf,(char *)"LIST"); 
	res = IsCmdOk( buf, "150" );	
	if ( 1 != res ){
		close( iSockData );


	sprintf( errMsg, "Inexistant directory: %s!\n", filePath );

		//sprintf( errMsg, "检查文件是否存在时，取远程机器上路径%s的文件列表失败!\n", filePath );
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
			//分解buf
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
description:	判断本地文件是否存在
input:	        
		p_cPathName:		要被判断的本地文件路径+文件名
output:		
return:         	
		0:			不存在
		1:			存在	
programmer:	
		安	坤
date:		
		2005-08-22
*****************************************************/ 
int CFTPmanager::IsFileExist( const char *p_cPathName )
{
	return ( ( access( p_cPathName, 0 ) == 0 ) ? 1 : 0 ); 
}

/***************************************************
description:	删除远程路径
input:	        
		dir:			要被删除的远程路径
output:		           
		errMsg：		错误信息                 
return:         	
		1:			成功
		其他:			参考FTPmanager.h里的错误信息的定义
programmer:	
		安	坤
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
	
	//信息定位
	Position();
	
	iRes = ftpcmd( cBuf, (char *)"RMD %s", cFullName );//250 is ok
	if(iRes==-1)
	{
		strcpy(errMsg,sz_errorMessage);
	}
	return iRes;	
}




/***************************************************
description:	将文件全路径分解问路径和文件名
input:	        
		filePathName:		文件全路径+文件名
output:		           
		filePath：		文件路径
		fileName:		文件名              
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
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
	{//相对文件路径，当前路径下的文件
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
description:	得到Unix系统(远程机)某路经下符合某正则表达式的文件的列表
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
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

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData );         


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                  
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		close( iSockData );  


	strcpy( errMsg, "Fail to listen!\n" );  
                                              
		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//连接已断开，等待1秒后重连 
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//将信息定位
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

		//sprintf( errMsg, "远程机器上的路径%s不存在!\n", path );
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

		//strcpy( errMsg, "接收服务器数据失败!\n" );
		return SEND_CMD_ERR;
	} 
	
	//删除原有文件列表
	remove(listSavePathName);
	//重新创建文件列表
	int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//printf( errMsg, "要保存在本地的文件%s创建和打开失败!\n", listSavePathName );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//开始判断是否符合条件
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
description:	得到Unix系统(远程机)某路经下符合某正则表达式的文件的列表
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
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

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );     


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                      
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData );  


	strcpy( errMsg, "Fail to listen!\n" );  
                                              
		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//连接已断开，等待1秒后重连 
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//将信息定位
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

		//sprintf( errMsg, "远程机器上的路径%s不存在!\n", path );
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

		//strcpy( errMsg, "接收服务器数据失败!\n" );
		return SEND_CMD_ERR;
	} 
	
	//删除原有文件列表
	remove(listSavePathName);
//printf( "cccccccccccccccccccccc\n");	
	//重新创建文件列表
	int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		//printf("writing err1!\n"); 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//sprintf( errMsg, "要保存在本地的文件%s创建和打开失败!\n", listSavePathName );                                                                      
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
			//开始判断是否符合条件
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
description:	判断该记录中文件名是否符合要求(正则表达式)
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
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
description:	判断该记录中文件名是否符合要求(正则表达式)
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
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
description:	判断某字符串是否符合某正则表达式
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
date:		
		2005-09-15
*****************************************************/ 
int CFTPmanager::IsMatchZhengZe( char *cOldFileName, char *cCondition ){
	//先判断条件是否含有正则表达式
	char		*pCur = NULL;
	char		cNewFileName[2000];
	char		cTemp[100];
	int		iLen = 0;
	
	memset( cTemp, 0, 100 );
	memset( cNewFileName, 0, 2000 );
	
	//背份cOldFile
	char		cName[2000];
	char		*cFileName = cName;
	memset( cFileName, 0, 2000 );
	strcpy( cFileName, cCondition );
	
	//printf("cFileName=%s\n",cFileName);
	pCur = strchr( cFileName, '*' );
	if ( NULL != pCur ){//含有
		//在有*的地方加个.以符合解析方式
		pCur = NULL;
		//printf("come here\n");
		while ( ( pCur = strstr( cFileName, "*" ) ) != NULL ){
			iLen = pCur - cFileName;
		//	printf("len: %d\n", iLen);
			memset( cTemp, 0, 100 );
			strncpy(cTemp, cFileName, iLen);
			sprintf( cNewFileName, "%s%s.*", cNewFileName, cTemp  );
			//移动指针
			cFileName = pCur + 1;
			//cFileName += 1;
		}
		
		//把最后一个串考入cNewName
		strcat( cNewFileName, cFileName );
		//printf( "newName: <%s>\n, fileName: <%s>\n", cNewFileName, cOldFileName );
		
		//对比正则表达式
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
		
	} else {//不含
		//比较是否完全匹配
		//printf( "no *: %s, %s\n", cOldFileName, cCondition );
		return ( strcmp( cOldFileName, cCondition ) == 0 ? 1 : 0 );
	}	
}

/***************************************************
description:	判断该记录中文件时间是否在某指定时间范围内
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
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
	
	//得到当前年份
	memset( cCurDate, 0, 20 );
	memset( cCurYear, 0, 10 );
	GetDate( cCurDate );
	strncpy( cCurYear, cCurDate, 4 );
	
	//得到月数字
	memset( cCurMonth, 0, 10 );
	if ( GetMonthByChr( cParam5, cCurMonth ) != 1 ){


	sprintf( errMsg, "Unrecognized month: %s", cParam5 ); 

		//sprintf( errMsg, "不可识别的月份%s\n", cParam5 );	
		return 0;
	}
	
	memset( cNewDate, 0, 20 );
	
	//判断是否今年
	char			*p = NULL;
	p = strstr( cParam7, ":" );

	if ( p ){//是今年
		//组成日期比较字符串		
		sprintf( cNewDate, "%s-%s-%02s %s", cCurYear, cCurMonth, cParam6, cParam7 );
	} else {//不是今年
		sprintf( cNewDate, "%s-%s-%02s", cParam7, cCurMonth, cParam6 );
	}
	
	//printf( "%s\n", cNewDate );
	
	
	if ( ( strcmp( cNewDate, beginTime ) >= 0 ) 
		&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//符合
		return 1;		
	} else {
		return 0;	
	} 
}

/***************************************************
description:	判断该记录中文件时间是否在某指定时间范围内
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
programmer:	
		安	坤
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
		&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//符合
		return 1;		
	} else {
		return 0;	
	} 
}

/***************************************************
description:	取得XXXX-XX-XX形式的当前日期
input:	        
output:		           
return:         	
programmer:	
		安	坤
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
description:	由字母月份取得数字月份
input:	        
output:		           
return:         	
programmer:	
		安	坤
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
description:	获得某一远程路径下的某一指定时间段内的文件列表
input:	      
		oldListFile:		老文件列表路径+文件名
		newListFile：		老文件列表路径+文件名
		beginTime:		起始日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		endTime:		结束日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）  
output:		           
return:         	
programmer:	
		安	坤
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

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );     


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                      
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData ); 


	strcpy( errMsg, "Fail to listen!\n" );  
                                               
		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//连接已断开，等待1秒后重连 
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}                                
	}    
	
	//将信息定位
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

		//sprintf( errMsg, "远程机器上的路径%s不存在!\n", path );
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

		//strcpy( errMsg, "接收服务器数据失败!\n" );
		return SEND_CMD_ERR;
	} 
	
	//删除原有文件列表
	remove( listFile );

	//重新创建文件列表
	int iSaveFd = open(listFile,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		//printf("writing err1!\n"); 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listFile ); 

		//sprintf( errMsg, "要保存在本地的文件%s创建和打开失败!\n", listFile );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//开始判断是否符合条件
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
description:	获得某一远程路径下的某一指定时间段内的文件列表
input:	      
		oldListFile:		老文件列表路径+文件名
		newListFile：		老文件列表路径+文件名
		beginTime:		起始日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		endTime:		结束日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）  
output:		           
return:         	
programmer:	
		安	坤
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

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		//printf("Linking err5!\n");                                          
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		//printf("binding err6!\n");                                          
		close( iSockData );     


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                      
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		//printf("listening err7!\n");                                        
		close( iSockData );      


	strcpy( errMsg, "Fail to listen!\n" );  
                                          
		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
		return SOCK_LISTENING_ERR;                                                           
	}                                                                           
		                                                                    
	tmp = sizeof(addr);                                                         
	_getsockname(iSockData,(struct sockaddr *)&addr,&tmp);                      
	c = (unsigned char *)&addr.sin_addr;                                        
	p = (unsigned char *)&addr.sin_port;                                        
	                                                                            
	memset( buf, 0, 1024 );                                               
	res = ftpcmd(buf,(char *)"PORT %d,%d,%d,%d,%d,%d",                            
	c[0],c[1],c[2],c[3],p[0],p[1]);                                                                    
	if (res != 1)  {//连接已断开，等待1秒后重连
		close( iSockData ); 
		if ( Reconnect( errMsg ) == 1 ){
			goto CHECK_AGAIN;
		} else {


	strcpy( errMsg, "Connection was elimilated!\n" ); 

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                        
	}    

	//将信息定位
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

		//sprintf( errMsg, "远程机器上的路径%s不存在!\n", path );
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

		//strcpy( errMsg, "接收服务器数据失败!\n" );
		return SEND_CMD_ERR;
	} 
	
	//删除原有文件列表
	remove( listFile );
//printf( "cccccccccccccccccccccc\n");	
	//重新创建文件列表
	int iSaveFd = open(listFile,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		//printf("writing err1!\n"); 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listFile ); 

		//sprintf( errMsg, "要保存在本地的文件%s创建和打开失败!\n", listFile );                                                                      
		return TARGET_FILE_CREATE_OPEN_ERR;                                                                                  
	}                         
//printf( "ddddddddddddddddddddddd\n");	

	memset( buf, 0, 1024 );
	char		cCur[5];
	memset( cCur, 0, 5 );
	while ( ( len=read( m_sockxfer, cCur, 1 ) )>0 ) { 
		sprintf( buf, "%s%s", buf, cCur );
		if ( strcmp( cCur, "\n" ) == 0 ) {
			//开始判断是否符合条件
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
description:	获取远程目录下的所有文件信息，并把信息写到文件列表中
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
	//删除原有文件列表
	
	
	memset( buf, 0, 1024 ); 

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) 
	{                                                   
		close( iSockData );

		strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) 
	{         
		close( iSockData ); 

		strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                          
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  
	{                                          
		//close( iSockData );
		close( iSockData );                                                
		//shutdown( isockftp, SHUT_RDWR );    
		strcpy( errMsg, "Fail to listen!\n" );  

		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
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
	{//连接已断开，等待1秒后重连 
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
				
				//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
				return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//将信息定位
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


		//sprintf( errMsg, "远程机器上要得到文件列表的路径%s不存在 \n", path );
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

		//sprintf( errMsg, "取远程机器上路径%s的文件列表失败!\n", path );
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
	
	
	//重新创建文件列表
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
		//sprintf( errMsg, "要保存在本地的文件%s创建和打开失败!\n", listSavePathName );                                                                      
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
      	//2006-9-20 19:17 added 忽略EINTR错误
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
description:	检查path，如果path以'/'结束则去掉'/'
input:	        
		path:		要检查的path
return:     无    	
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
description:	得到Unix系统(远程机)某路经下符合某正则表达式的文件的列表
input:	        
		path:			远程机路经
		listSavePathName：	保存的文件列表名
		condition：		正则表达式条件
output:		           
return:         	
		0:			失	败
		1：			成	功
		-17:符合条件的源文件不存在
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

	//建立数据接收socket                                                        
	int iSockData = socket(AF_INET,SOCK_STREAM,0);                                 
	if ( iSockData == -1 ) {                                                   
		close( iSockData );


	strcpy( errMsg, "Fail to build socket for receiving data!\n" );

		//strcpy( errMsg, "接收服务器数据的SOCKET建立失败!\n" );                                                 
		return CREATE_LISTENING_SOCK_ERR;                                                           
	}                                                                           
		                                                                    
	int tmp = sizeof(addr);                                                         
	_getsockname(isockftp,(struct sockaddr *)&addr,&tmp);                       
	addr.sin_port = 0;                                                          
	                                                                            
	if (bind(iSockData,(struct sockaddr *)&addr,sizeof(addr)) == -1) {         
		close( iSockData );         


	strcpy( errMsg, "Fail to bind port for socket!\n" );  
                                  
		//strcpy( errMsg, "接受服务器数据的SOCKET端口绑定失败!\n" );    		                                           
		return LISTENING_SOCK_BIND_ERR;                                                           
	}                                                                           
		                                                                    
	if (listen(iSockData,1) == -1)  {                                          
		close( iSockData );  


	strcpy( errMsg, "Fail to listen!\n" );  
                                              
		//strcpy( errMsg, "接受服务器数据的SOCKET监听失败!\n" );    		                                           
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
	{//连接已断开，等待1秒后重连 
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
			

			//strcpy( errMsg, "与FTP服务器上的连接已断开，且自动重连接失败!\n" );
			return SOCK_CONNECT_ERR;
		}                                                                       
	}    

	//将信息定位
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

		//sprintf( errMsg, "远程机器上的路径%s不存在!\n", path );
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
		//strcpy( errMsg, "接收服务器数据失败!\n" );
		return SEND_CMD_ERR;
	} 
	
	//删除原有文件列表
	remove(listSavePathName);
	//重新创建文件列表
	int iSaveFd = open(listSavePathName,O_WRONLY|O_CREAT,0644);                                               
	if (iSaveFd == -1){                                                                                 
		close( iSockData );
		close( m_sockxfer );


	sprintf( errMsg, "Fail to create and open file: %s!\n", listSavePathName ); 

		//printf( errMsg, "要保存在本地的文件%s创建和打开失败!\n", listSavePathName );                                                                      
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
				//2006-9-20 19:38 added by yangzx 忽略EINTR错误
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
	
  
			
		
			



