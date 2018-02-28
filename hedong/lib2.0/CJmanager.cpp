/* CJmanager.cpp */
#include "CJmanager.h"

CCJmanager::CCJmanager(){
	
}

CCJmanager::~CCJmanager(){
	
}

/***************************************************
description:	设置FTP优先级设置的ENV文件全路径+文件名
input:	        
		cFtpEnvPathName:			FTP优先级设置的ENV文件全路径+文件名
output:         
return:         		
programmer:	安	坤
date:		2005-08-19
*****************************************************/ 
void CCJmanager::SetEnvPathName( char *cDownloadEnvPathName, char *cUploadEnvPathName ){
	strcpy( m_cDownloadEnvPathName, cDownloadEnvPathName );
	strcpy( m_cUploadEnvPathName, cUploadEnvPathName );
}

/***************************************************
description:	得到某一优先级的信息
input:	        
		iGrade:			优先级
		iMode:			0:		下载用
					1：		上传用
output:		
		struGradeInfo:		取得的优先级的信息
		cErrMsg:		错误信息
return:         	
		1:			成功
		-1：			LOCAL的设置为空
		-2：			REMOTE的设置为空
		-3：			CHILD的设置为空
		-4：			TREE的设置为空
		-5：			STYLE的设置的的数据数量不够
		-6：			MODE的设置的的数据数量不
		-7：			COVER的设置的的数据数量不
		-8：			SOURCE的设置的的数据数量不	
programmer:	
		安	坤
date:		
		2005-08-19
*****************************************************/ 
int CCJmanager::GetGradeInfo( int iGrade, GRADE_INFO *struGradeInfo, char *cErrMsg, int iMode ){
	FILE *tmpfp;
	char cParam0[100], cParam1[1024], buf[2000], cGrade[5];
	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 1024 );
	memset( buf, 0, 2000 );
	memset( cGrade, 0, 5 );
	
	sprintf( cGrade, "%d", iGrade );
	struGradeInfo->iGrade = iGrade;
	//1 打开ENV文件
//	printf( "m_cFtpEnvPathName: %s\n", m_cFtpEnvPathName );
	char		m_cFtpEnvPathName[ 1024 ];
	memset( m_cFtpEnvPathName, 0, 1024 );
	
	strcpy( m_cFtpEnvPathName, ( iMode == 0 ? m_cDownloadEnvPathName : m_cUploadEnvPathName ) );

	if( ( tmpfp = fopen( m_cFtpEnvPathName, "r" ) ) == NULL ) {
		strcpy( cErrMsg, "ENV文件打开失败！\n" );
		return 0;
	}
	//2 循环读文件，读取一行
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
		//3 判断是否为某优先级的首行
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		//如果是GRADE首行
		if( strcmp( cParam0, "GRADE" ) == 0 ) {
			printf( "1\n" );
			//4 继续往下读，判断是否为PATH行
			//如果级别值符合
			if ( strcmp( cParam1, cGrade ) == 0 ){
				printf( "0: %s, 1: %s\n", cParam0, cParam1 );
				//继续往下读数据内容
				memset( buf, 0, 2000 );
				while( fgets( buf, 2000, tmpfp ) != NULL ) {
					
					Trim( buf );
					
					//#为注释行
					if( buf[0] == '#' ) {
						//printf( "in me : %c\n", buf[0] );
						continue;
					}
						
					//遇到空行则退出
					//printf( "buf: <%s>\nlen: %d\n", buf, strlen( buf ) );					
					if ( strlen( buf ) == 1 ){
						//fclose( tmpfp );
						printf( "empty line out\n" );
						//return 1;
						//printf( "111111111111111111111out\n" );
						goto OUT;
						//continue;
					}	
					
					memset( cParam0, 0, 100 );
					memset( cParam1, 0, 1024 );
					sscanf( buf, "%s %s", cParam0, cParam1);
					//printf( "1 = %s, 2 = %s\n", cParam0, cParam1 );
					//如果是GRADE首行
					if( strcmp( cParam0, "LOCAL" ) == 0 ) {
						strcpy( struGradeInfo->cLocal, cParam1 );
					} else if( strcmp( cParam0, "REMOTE" ) == 0 ) {
						strcpy( struGradeInfo->cRemote, cParam1 );
					} else if( strcmp( cParam0, "CHILD" ) == 0 ) {
						strcpy( struGradeInfo->cChild, cParam1 );
					} else if( strcmp( cParam0, "TREE" ) == 0 ) {
						strcpy( struGradeInfo->cTree, cParam1 );
					} else if( strcmp( cParam0, "STYLE" ) == 0 ) {
						
						GetDetail( cParam1, struGradeInfo, 0 );
					} else if( strcmp( cParam0, "MODE" ) == 0 ) {
						GetDetail( cParam1, struGradeInfo, 1 );
						//printf( "cover\n" );
					} else if( strcmp( cParam0, "COVER" ) == 0 ) {
						GetDetail( cParam1, struGradeInfo, 2 );
					} else if( strcmp( cParam0, "SOURCE" ) == 0 ) {
						GetDetail( cParam1, struGradeInfo, 3 );
					} else if( strcmp( cParam0, "COMPRESS" ) == 0 ) {
						//GetDetail( cParam1, struGradeInfo, 3 );
						strcpy( struGradeInfo->cCompress, cParam1 );
					} else if( strcmp( cParam0, "PASS" ) == 0 ) {
						//GetDetail( cParam1, struGradeInfo, 3 );
						strcpy( struGradeInfo->cPass, cParam1 );
					}
				}
			} 
		}
	}
OUT:	
	//sprintf( cErrMsg, "ENV文件中没有优先级=%d的设置!\n", iGrade );
	fclose( tmpfp );
	
	return ( CheckGradeInfo( *struGradeInfo, cErrMsg ) );
}

/***************************************************
description:	下载优先级为grade的文件
input:	        
		iGrade:			优先级
output:		
		cErrMsg:		错误信息
return:         	
		0:			失败
		1:			成功	
programmer:	
		安	坤
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::DownloadByGrade( int grade, char *errMsg ){
	GRADE_INFO		struGradeInfo;
	SERVER_INFO		struServerInfo;
	int			iRes = 0;
	
	//1 取得级别信息
	memset( &struGradeInfo, 0, sizeof( GRADE_INFO ) );
	memset( &struServerInfo, 0, sizeof( SERVER_INFO ) );
	struGradeInfo.iModeCount = 0;

	iRes = GetServerInfo( &struServerInfo, errMsg, 0 );
	printf( "serverMsg:\n%s\n", errMsg );
	if ( iRes != 1 ) return 0;
	
	iRes = GetGradeInfo( grade, &struGradeInfo, errMsg, 0 );
	printf( "gradeMsg:\n%s\n", errMsg );
	if ( iRes != 1 ) return 0;
	
	//下载
	iRes = DownloadFrom( struGradeInfo.cRemote, struServerInfo,
					struGradeInfo, errMsg );
	if ( iRes != 1 ){
		theLog << "第"<<grade<<"级的文件下载失败，失败原因:"<<errMsg<<endw;
	} else {
		theLog << "第"<<grade<<"级的文件下载完成！"<<endi;
	}
	
	return 1;
}

/***************************************************
description:	上传优先级为grade的文件
input:	        
		iGrade:			优先级
output:		
		cErrMsg:		错误信息
return:         	
		0:			失败
		1:			成功	
programmer:	
		安	坤
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::UploadByGrade( int grade, char *errMsg ){
	GRADE_INFO		struGradeInfo;
	SERVER_INFO		struServerInfo;
	int			iRes = 0;
	
	//1 取得级别信息
	memset( &struGradeInfo, 0, sizeof( GRADE_INFO ) );
	memset( &struServerInfo, 0, sizeof( SERVER_INFO ) );
	struGradeInfo.iModeCount = 0;

	iRes = GetServerInfo( &struServerInfo, errMsg, 1 );
	if ( iRes != 1 ) return 0;
	
	iRes = GetGradeInfo( grade, &struGradeInfo, errMsg, 1 );
	if ( iRes != 1 ) return 0;
	
	//下载
	iRes = UploadFrom( struGradeInfo.cLocal, struServerInfo,
					struGradeInfo, errMsg );
	if ( iRes != 1 ){
		theLog << "第"<<grade<<"级的文件上传失败，失败原因:"<<errMsg<<endw;
	} else {
		theLog << "第"<<grade<<"级的文件上传完成！"<<endi;
	}
	
	return 1;
//	GRADE_INFO		struGradeInfo;
//	int			iRes = 0;
////	printf( "!@@@@@@@@@@@@@@grade = %d\n ", grade );
//	
//	struGradeInfo.iDataCount = 0;
//	struGradeInfo.iGrade = 0;
//	for ( int i = 0 ; i < 50 ; i ++ ){
//		memset( struGradeInfo.struDataInfo[ i ].cIp, 0, 20 );
//		memset( struGradeInfo.struDataInfo[ i ].cPath, 0, 1024 );
//		memset( struGradeInfo.struDataInfo[ i ].cStyle, 0, 100 );
//		memset( struGradeInfo.struDataInfo[ i ].cIncludeChildren, 0, 5 );
//		memset( struGradeInfo.struDataInfo[ i ].cLocal, 0, 300 );
//		memset( struGradeInfo.struDataInfo[ i ].cBuildTree, 0, 5 );
//		memset( struGradeInfo.struDataInfo[ i ].cLoginName, 0, 100 );
//		memset( struGradeInfo.struDataInfo[ i ].cLoginPassword, 0, 100 );
//		
//		struGradeInfo.struDataInfo[ i ].iMode = 0;
//		struGradeInfo.struDataInfo[ i ].iCoverFlag = 0;
//	}
//	
//	//1 取得级别信息
//	iRes = GetGradeInfo( grade, &struGradeInfo, errMsg, 1 );
//	if ( iRes != 1 ) return 0;
//	
////	printf( "!!!!!!!!!!!!!!!!!!!\n " );
//	//2 根据每个设置取文件列表
//	for ( int i = 0 ; i < struGradeInfo.iDataCount ; i ++ ){
//		//上传
//		iRes = UploadFrom( struGradeInfo.struDataInfo[i].cLocal, 
//						struGradeInfo.struDataInfo[i], errMsg );
//		if ( iRes != 1 ){
//			theLog << "第"<<grade<<"级的第"<<i<<"项设置上传失败，失败原因:"<<errMsg<<endw;
//			//printf( "第%d级的第%d个设置上传失败，失败原因：\n%s", grade, i, errMsg );
//		} else {
//			theLog << "第"<<grade<<"级的第"<<i<<"项设置上传完成!"<<endi;
//			//printf( "第%d级的第%d个设置上传成功！\n", grade, i );
//		}
//	}
	
	return 1;
}

/***************************************************
description:	下载某文件夹下的所有文件
input:	        
		downloadPath:			FTP服务器上的下载路径
		struDataInfo:			下载设置信息
output:		
		cErrMsg:			错误信息
return:         	
		0:			失败
		1:			成功	
programmer:	
		安	坤
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::DownloadFrom( char *downloadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg ){
	printf( "from1111111111111\n" );
	int			iRes = 0;
	CFTPmanager		ftp;
	char			cListName[10];
	
	memset( cListName, 0, 10 );
	
	//调用解密接口
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;

	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	//连接
	iRes = ftp.Connect( serverInfo.cIp, cRealUser, cRealPw, errMsg );
	if ( iRes != 1 ) return 0;
	
	//得到列表文件名
	for ( int i = 0 ; ; i ++ ){
		memset( cListName, 0, 10 );
		sprintf( cListName, "%d.lst", i );
		if ( IsFileExist( cListName ) != 1 ) break;
	}

	//得到文件列表
	iRes = ftp.List( struDataInfo.cRemote, cListName, errMsg );
	if ( iRes != 1 ) return 0;

	//等待硬盘创建文件
	while( 1 ){
		if ( IsFileExist( cListName ) == 1 ) break;
		//printf( "waiting...\n" );
	}
	
	//循环读取一行，并解析文件名
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( cListName, "r" ) ) == NULL ){
		sprintf( errMsg, "打开%s路径的文件列表失败！\n", downloadPath );
		return 0;	
	}
	char		cBuf[2000];
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
		
	GRADE_INFO		struNewData;

	char		cTarget[2000];
	memset( cTarget, 0, 2000 );
	
	int		iIndex = 0;		//modeInfo数组中对应的INDEX号
	int		iAllFile = 0;		//”是否下载全部文件“的标志
	int		iMatch = 0;		//文件名和要求是否匹配
	int		iSendMode = 2;		//传输模式（2/10进制）
	char		cSource[200];		//下载完成后源文件的处理模式
	
	char		cLocalFile[1024], cRemoteFile[1024];
	int		iCoverFlag = 0;
	
//	CEncryptFile		encc;
//	CF_CLzwCompressNoCatch	com;
	
	memset( cBuf, 0, 2000 );
	while( fgets( cBuf, 2000, fFile ) != NULL ) {		
		//解析文件属性和文件名
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 100 );
		memset( cParam2, 0, 100 );
		memset( cParam3, 0, 100 );
		memset( cParam4, 0, 100 );
		memset( cParam5, 0, 100 );
		memset( cParam6, 0, 100 );
		memset( cParam7, 0, 100 );
		memset( cParam8, 0, 100 );
		
		sscanf( cBuf, "%s %s %s %s %s %s %s %s %s", 
		cParam0, cParam1, cParam2, cParam3, cParam4, cParam5, cParam6, cParam7, cParam8);
		
		if ( strcmp( cParam0, "total" ) == 0 ) continue;
		
		if ( ( strcmp( cParam8, "." ) == 0 ) || ( strcmp( cParam8, ".." ) == 0 ) ){
			continue;
		}
		
		//半拉子文件的处理
		if ( strcmp( cParam0, "----------" ) == 0 ) continue;
		
		//临时文件的处理
		if ( ( cParam8[0] == '~' ) || ( IsFileMatch( cParam8, "*.tmp" ) == 1 ))
			continue;
			
		int iFiSize = 0;

		//判断文件属性
		if ( cParam0[0] == 'd' ){//文件夹
		//	printf( "jia\n" );
			//判断INCLUDECHILDREN
			if ( strcmp( struDataInfo.cChild, "Y" ) == 0 ){//包含
				//判断BUILDTREE
				
				memset( &struNewData, 0, sizeof( GRADE_INFO ) );
		
				sprintf( struNewData.cRemote, "%s/%s", struDataInfo.cRemote, cParam8 );
				strcpy( struNewData.cChild, struDataInfo.cChild );
				strcpy( struNewData.cTree, struDataInfo.cTree );
				struNewData.iModeCount = struDataInfo.iModeCount;

				for ( int iCount = 0 ; iCount < struDataInfo.iModeCount ; iCount ++ ){
					strcpy( struNewData.modeInfo[ iCount ].cStyle, struDataInfo.modeInfo[ iCount ].cStyle );
					strcpy( struNewData.modeInfo[ iCount ].cMode, struDataInfo.modeInfo[ iCount ].cMode );
					strcpy( struNewData.modeInfo[ iCount ].cCover, struDataInfo.modeInfo[ iCount ].cCover );
					strcpy( struNewData.modeInfo[ iCount ].cSource, struDataInfo.modeInfo[ iCount ].cSource );
				}
					
				if ( strcmp( struDataInfo.cTree, "Y" ) == 0 ){//建立TREE
					//printf( "first\n" );
					sprintf( struNewData.cLocal, "%s/%s", struDataInfo.cLocal, cParam8 );
					//在本地创建文件夹
					//printf( "local: %s\n", struNewData.cLocal );
					mkdir( struNewData.cLocal, S_IRWXU|S_IRGRP|S_IXGRP);			
				} else {//不用建立TREE
					strcpy( struNewData.cRemote, struDataInfo.cRemote );
				}	

//				printf( "path: %s\n", struNewData.cPath );
				iRes = DownloadFrom( struNewData.cRemote, serverInfo, struNewData, errMsg );
				if ( iRes != 1 ){
					theLog <<errMsg<< ende;
					//printf( "%s", errMsg );	
				}
			}
			
		} else { //文件
			//判断文件名是否条件
			
			//判断是否下载全部文件
			iAllFile = 0;
			iIndex = 0;
			for ( int i = 0 ; i < struDataInfo.iModeCount ; i ++ ){
				if ( strcmp( struDataInfo.modeInfo[ i ].cStyle, "*" ) == 0 ){
					iAllFile = 1;
					break;	
				}
				iIndex ++;
			}
			 
			if ( iAllFile == 1 ){
				memset( cTarget, 0, 2000 );
				sprintf( cTarget, "%s/%s", struDataInfo.cLocal, cParam8 );
			//	printf( "cTarget: <%s>\n", cTarget );
				//取传输模式
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
					iSendMode = 2;
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
					iSendMode = 10;
				} else {//error
					iSendMode = 0;
					sprintf( errMsg, "ENV文件中找到不能识别的文件传输模式 - %s", struDataInfo.modeInfo[ iIndex ].cMode );
					return 0;
				}
				
				//下载完成后源文件的处理模式
				memset( cSource, 0, 200 );
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//不处理
					strcpy( cSource, SOURCE_IGNORE );
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//删除
					strcpy( cSource, SOURCE_DELE );
				} else {//备份
					strcpy( cSource, struDataInfo.modeInfo[ iIndex ].cSource );
				}
				
				
				memset( cLocalFile, 0, 1024 );
				memset( cRemoteFile, 0, 1024 );
				
				strcpy( cLocalFile, cTarget );
				sprintf( cRemoteFile, "%s/%s", downloadPath, cParam8 );
				
				if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'Y' )
					iCoverFlag = TARGET_COVER;
				else if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'N' )
					iCoverFlag = TARGET_STORE;
				printf( "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n%s\n%s", cLocalFile, cRemoteFile );
			//	//先将文件存在压缩文件夹
			//	char		cPreFile[300], cPassFile[300];
			//	memset( cPreFile, 0, 300 );
			//	memset( cPassFile, 0, 300 );
			//	sprintf( cPreFile, "%s.prs", cLocalFile );
				
				iRes = ftp.Download( cLocalFile, cRemoteFile, iCoverFlag, errMsg, iSendMode, cSource, struDataInfo.cCompress, struDataInfo.cPass );//cSource
				if ( iRes != 1 ) {
					theLog<<"下载文件"<<downloadPath<<"/"<<cParam8<<"失败，失败原因:"<<errMsg<< endw;
					memset( cBuf, 0, 2000 );
					continue;
				} else {
			//		//解压缩
			//		if ( strcmp( struDataInfo.cCompress, "Y" ) == 0 ) {
			//			com.uncompress( cLocalFile, cPreFile );
			//		} else {
			//			rename( cLocalFile, cPreFile );
			//		}
			//		
			//		//cPreFile是解压缩后的文件
			//		
			//		//解密
			//		if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//加密了
			//			//提取真正的文件名
			//			int iPos;
			//			iPos=strrncspn( cLocalFile, '.', 3 );
			//			//if(iPos<0) return 0;
			//			strncpy( cPassFile , cLocalFile, iPos );
			//			//cPassFile[iPos]=0;
			//			
			//			encc.unEncrypt( cPassFile, cPreFile, struDataInfo.cPass );
			//		} else {//没加密
			//			//提取真正的文件名
			//			int iPos2;
			//			iPos2=strrncspn( cLocalFile, '.', 2 );
			//			//if(iPos<0) return 0;
			//			strcpy( cPassFile , cLocalFile );
			//			cPassFile[iPos2]=0;
			//			
			//			rename( cPassFile, cPreFile );
			//		}
			//		
			//		//cPassFile是解压缩和解密后的文件名,同时也已经去掉了文件大小和其他信息,是真正的文件名
			//		
			//		//核对文件大小
			//		//提取文件大小
			//		if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//加密了
			//			//提取文件大小
			//			char		cFiSize[10];
			//			memset( cFiSize, 0, 10 );
			//			int iPosa;
			//			iPosa=strrncspn( cLocalFile, '.', 3 );
			//			//if(iPos<0) return 0;
			//			char *aa = cLocalFile;
			//			aa += iPosa;
			//			int iPosb;
			//			iPosb=strrncspn( cLocalFile, '.', 2 );
			//			strncpy( cFiSize, aa, iPosb );
			//			iFiSize = atoi( cFiSize );
			//		} else {
			//			//提取文件大小
			//			char		cFiSize[10];
			//			memset( cFiSize, 0, 10 );
			//			int iPosa;
			//			iPosa=strrncspn( cLocalFile, '.', 2 );
			//			//if(iPos<0) return 0;
			//			char *aa = cLocalFile;
			//			aa += iPosa;
			//			int iPosb;
			//			iPosb=strrncspn( cLocalFile, '.', 1 );
			//			strncpy( cFiSize, aa, iPosb );
			//			iFiSize = atoi( cFiSize );
			//		}
			//		
			//		//比较文件大小
			//		if ( atoi(cParam4) != iFiSize ) {
			//			theLog<<"下载文件"<<downloadPath<<"/"<<cParam8<<"失败,失败原因:下载前后文件大小不符！"<<endi;
			//		}
			//		
			//		//处理源文件
			//		if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//删除方式
			//			ftp.RmFile( cRemoteFile, errMsg );
			//		} else if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//不处理方式
			//			
			//		} else {//备份方式
			//			ftp.MoveFile( cRemoteFile, cSource, errMsg );
			//		}
					
					memset( cBuf, 0, 2000 );
					theLog<<"下载文件"<<downloadPath<<"/"<<cParam8<<"成功！"<<endi;
					continue;	
				}
			} else {
				//循环匹配
				iMatch = 0;
				iIndex = 0;
				for ( int i = 0 ; i < struDataInfo.iModeCount ; i ++ ){
					if ( IsFileMatch( cParam8,struDataInfo.modeInfo[ i ].cStyle ) == 1 ){
						iMatch = 1;
						break;
					}
					iIndex ++;
				}
				
				if ( iMatch == 0 ) continue;
				
				if ( iMatch == 1 ){//匹配
					//下载文件
					//取传输模式
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
						iSendMode = 2;
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
						iSendMode = 10;
					} else {//error
						iSendMode = 0;
						sprintf( errMsg, "ENV文件中找到不能识别的文件传输模式 - %s", struDataInfo.modeInfo[ iIndex ].cMode );
						continue ;
					}
					
					//下载完成后源文件的处理模式
					memset( cSource, 0, 200 );
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//不处理
						
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//删除
						strcpy( cSource, "*" );
					} else {//备份
						strcpy( cSource, struDataInfo.modeInfo[ iIndex ].cSource );
					}
					
					memset( cTarget, 0, 2000 );
					sprintf( cTarget, "%s/%s", struDataInfo.cLocal, cParam8 );
				//	printf( "target: %s\n", cTarget );
				
					
					memset( cLocalFile, 0, 1024 );
					memset( cRemoteFile, 0, 1024 );
					
					strcpy( cLocalFile, cTarget );
					sprintf( cRemoteFile, "%s/%s", downloadPath, cParam8 );
				
					if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'Y' )
						iCoverFlag = TARGET_COVER;
					else if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'N' )
						iCoverFlag = TARGET_STORE;
					
					if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'Y' )
						iCoverFlag = TARGET_COVER;
					else if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'N' )
						iCoverFlag = TARGET_STORE;
					
					printf( "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n%s\n%s", cLocalFile, cRemoteFile );
					//先将文件存在压缩文件夹
			//		char		cPreFile[300], cPassFile[300];
			//		memset( cPreFile, 0, 300 );
			//		memset( cPassFile, 0, 300 );
			//		sprintf( cPreFile, "%s.tmp", cLocalFile );
					
					iRes = ftp.Download( cLocalFile, cRemoteFile, iCoverFlag, errMsg, iSendMode, cSource, struDataInfo.cCompress, struDataInfo.cPass );
					if ( iRes != 1 ) {
						memset( cBuf, 0, 2000 );
						theLog<<"下载文件"<<downloadPath<<"/"<<cParam8<<"失败，失败原因:"<<errMsg<< endw;
						continue;
					} else {
			//			//解压缩
			//			if ( strcmp( struDataInfo.cCompress, "Y" ) == 0 ) {
			//				int t = com.uncompress( cLocalFile, cPreFile );
			//				printf( "ttt= %d\n", t );
			//			} else {
			//				rename( cLocalFile, cPreFile );
			//			}
			//			
			//			remove( cLocalFile );
			//			
			//			//cPreFile是解压缩后的文件.prs
			//			
			//			//解密
			//			if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//加密了
			//				//提取真正的文件名
			//				int iPos;
			//				iPos=strrncspn( cLocalFile, '.', 3 );
			//				//if(iPos<0) return 0;
			//				strncpy( cPassFile , cLocalFile, iPos );
			//				//cPassFile[iPos]=0;
			//				
			//				int iii = encc.unEncrypt( cPreFile, cPassFile, struDataInfo.cPass );
			//				printf( "iii = %d\n", iii );
			//			} else {//没加密
			//				//提取真正的文件名
			//				int iPos2;
			//				iPos2=strrncspn( cLocalFile, '.', 2 );
			//				//if(iPos<0) return 0;
			//				strcpy( cPassFile , cLocalFile );
			//				cPassFile[iPos2]=0;
			//				
			//				rename( cPassFile, cPreFile );
			//			}
			//			
			//			remove( cPreFile );
			//			
			//			//cPassFile是解压缩和解密后的文件名,同时也已经去掉了文件大小和其他信息,是真正的文件名
			//			
			//			//核对文件大小
			//			//提取文件大小
			//			if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//加密了
			//				//提取文件大小
			//				char		cFiSize[10];
			//				memset( cFiSize, 0, 10 );
			//				int iPosa;
			//				iPosa=strrncspn( cLocalFile, '.', 3 );
			//				//if(iPos<0) return 0;
			//				char *aa = cLocalFile;
			//				aa += iPosa + 1;
			//				int iPosb;
			//				iPosb=strrncspn( cLocalFile, '.', 2 );
			//				strncpy( cFiSize, aa, iPosb - iPosa - 1 );
			//				iFiSize = atoi( cFiSize );
			//			} else {
			//				//提取文件大小
			//				char		cFiSize[10];
			//				memset( cFiSize, 0, 10 );
			//				int iPosa;
			//				iPosa=strrncspn( cLocalFile, '.', 2 );
			//				//if(iPos<0) return 0;
			//				char *aa = cLocalFile;
			//				aa += iPosa + 1;
			//				int iPosb;
			//				iPosb=strrncspn( cLocalFile, '.', 1 );
			//				strncpy( cFiSize, aa, iPosb - iPosa - 1 );
			//				iFiSize = atoi( cFiSize );
			//			}
			//			
			//			//比较文件大小
			//			int iReSize = GetFileSize( cPassFile );
			//			if ( iReSize != iFiSize ) {
			//				theLog<<"下载文件"<<downloadPath<<"/"<<cParam8<<"失败,失败原因:下载前后文件大小不符！"<<endi;
			//			}
			//			
			//			//处理源文件
			//			if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//删除方式
			//				ftp.RmFile( cRemoteFile, errMsg );
			//			} else if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//不处理方式
			//				
			//			} else {//备份方式
			//				//取得文件名
			//				char	cName[50],cPath[300];
			//				memset( cName, 0, 50 );
			//				memset( cPath, 0, 300 );
			//				ftp.DivFileName( cRemoteFile, cPath, cName );
			//				sprintf( cSource, "%s/%s", cSource, cName );
			//				ftp.MoveFile( cRemoteFile, cSource, errMsg );
			//			}
						memset( cBuf, 0, 2000 );
						theLog<<"下载文件"<<downloadPath<<"/"<<cParam8<<"成功！"<<endi;
						continue;
					}
				}
			}
			
		}
		memset( cBuf, 0, 2000 );
	}
	
	fclose( fFile );
	iRes = remove( cListName );
	
	//断开连接
	ftp.Disconnect();
	//printf( "over!\n" );    
	return 1;
}

//得到文件大小
int CCJmanager::GetFileSize( const char *filePathName ) {
	struct stat buf;
	if ( stat( filePathName, &buf ) == -1 ) {
		printf("error!\n");
		return -1;
	}
	
	return buf.st_size;

}

/***************************************************
description:	判断文件是否存在
input:	        
output:		
return:         	
		0:			不存在
		1:			存在	
programmer:	
		安	坤
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::IsFileExist(char *p_cPathName){
	return ( ( access( p_cPathName, 0 ) == 0 ) ? 1 : 0 ); 
}

/***************************************************
description:	判断文件名是否符合正则表达式条件
input:	        
		cOldFileName:		文件名
		cCondition：		正则表达式条件
output:		
return:         	
		0:			不符合
		1:			符合	
programmer:	
		安	坤
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::IsFileMatch( char *cOldFileName, char *cCondition ){
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
	
	pCur = strstr( cFileName, "*" );
	if ( ( pCur - cFileName ) >= 0 ){//含有
		//在有*的地方加个.以符合解析方式
		pCur = NULL;
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
		//printf( "%d,%d\n", pmatch[0].rm_so, pmatch[0].rm_eo );
		//int 		iRtn = 0;
		///if ( ( pmatch[0].rm_so >= 0 ) && ( pmatch[0].rm_eo > 0 ) ) iRtn = 1;
		//else iRtn = 0;
		int iRtn = ( ( ( pmatch[0].rm_so >= 0 ) && ( pmatch[0].rm_eo > 0 ) ) ? 1 : 0 );
		//printf( "%d,%d\n", pmatch[1].rm_so, pmatch[1].rm_eo );
		regfree(&struPreg);
		return iRtn;
		
	} else {//不含
		//比较是否完全匹配
		return ( strcmp( cFileName, cCondition ) == 0 ? 1 : 0 );
	}
}

/***************************************************
description:	上传某文件夹下的所有文件
input:	        
		uploadPath:		要上传的本地路径文件名
		struDataInfo：		ENV中的设置信息
output:		
return:         	
		0:			不符合
		1:			符合	
programmer:	
		安	坤
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::UploadFrom( char *uploadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg ){
	//printf( "in\n" );
	int			iRes = 0;
	CFTPmanager		ftp;
	
	//调用解密接口
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;

	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	//连接
	iRes = ftp.Connect( serverInfo.cIp, cRealUser, cRealPw, errMsg );
	if ( iRes != 1 ) return 0;

	DIR		*dDir;
	struct dirent	*struDir;
	char		cFileName[100], cPathName[2000];
	memset( cFileName, 0, 100 );
	memset( cPathName, 0, 2000 );

	
	//打开文件列表
	if ((dDir = opendir(struDataInfo.cLocal)) == NULL){
		return 0;	
	}
	
	int		iIndex = 0;		//modeInfo数组中对应的INDEX号
	int		iAllFile = 0;		//”是否下载全部文件“的标志
	int		iMatch = 0;		//文件名和要求是否匹配
	int		iSendMode = 2;		//传输模式（2/10进制）
	char		cSource[200];		//下载完成后源文件的处理模式
	
	int		iCoverFlag = 0;
	GRADE_INFO		struNewData;
	char			cLocalFile[1024], cRemoteFile[1024];
	while((struDir = readdir(dDir)) != NULL){
		if(!struDir->d_ino){
			continue;
		}
		//得到当前文件全路径
		strcpy(cFileName, struDir->d_name);
		sprintf(cPathName, "%s/%s", uploadPath, cFileName);
		//printf( "cPathName = %s\n", cPathName );
		if (strcmp(cFileName, ".") == 0){
			continue;	
		}
		if (strcmp(cFileName, "..") == 0){
			continue;	
		}
				
		//判断文件属性
		if ( IsItDir( cPathName ) == 1 ){//文件夹
			//判断INCLUDECHILDREN
			if ( strcmp( struDataInfo.cChild, "Y" ) == 0 ){//包含
				memset( &struNewData, 0, sizeof( GRADE_INFO ) );
		
				sprintf( struNewData.cLocal, "%s/%s", struDataInfo.cLocal, cFileName );
				strcpy( struNewData.cChild, struDataInfo.cChild );
				strcpy( struNewData.cTree, struDataInfo.cTree );
				struNewData.iModeCount = struDataInfo.iModeCount;

				for ( int iCount = 0 ; iCount < struDataInfo.iModeCount ; iCount ++ ){
					strcpy( struNewData.modeInfo[ iCount ].cStyle, struDataInfo.modeInfo[ iCount ].cStyle );
					strcpy( struNewData.modeInfo[ iCount ].cMode, struDataInfo.modeInfo[ iCount ].cMode );
					strcpy( struNewData.modeInfo[ iCount ].cCover, struDataInfo.modeInfo[ iCount ].cCover );
					strcpy( struNewData.modeInfo[ iCount ].cSource, struDataInfo.modeInfo[ iCount ].cSource );
				}
					
				if ( strcmp( struDataInfo.cTree, "Y" ) == 0 ){//建立TREE
					//printf( "first\n" );
					sprintf( struNewData.cRemote, "%s/%s", struDataInfo.cRemote, cFileName );
					//在本地创建文件夹
					//printf( "local: %s\n", struNewData.cLocal );
					char		cFile[1024];
					memset( cFile, 0, 1024 );
					
					sprintf( cFile, "%s/%s", struDataInfo.cRemote, cFileName );
					iRes = ftp.MkDir( cFile, errMsg );
					if ( iRes != 1 ){
						theLog << errMsg << endw;
						//printf( "%s", errMsg );	
						continue;
					}
					
					//等待硬盘上的文件夹创建好
					while(1){
						iRes = ftp.ChDir( struNewData.cRemote, errMsg );
						if ( iRes == 1 ) break;
						//printf( "wait!\n" );
					}		
				} else {//不用建立TREE
					strcpy( struNewData.cRemote, struDataInfo.cRemote );
				}
				
				//printf( "path: %s\n", struNewData.cPath );
				iRes = UploadFrom( struNewData.cRemote, serverInfo, struNewData, errMsg );
				if ( iRes != 1 ){
					theLog << errMsg << endw;
					//printf( "%s", errMsg );	
				}
			}
			
		} else { //文件
			
			//判断是否下载全部文件
			iAllFile = 0;
			iIndex = 0;
			for ( int i = 0 ; i < struDataInfo.iModeCount ; i ++ ){
				if ( strcmp( struDataInfo.modeInfo[ i ].cStyle, "*" ) == 0 ){
					iAllFile = 1;
					break;	
				}
				iIndex ++;
			}
			
			if ( iAllFile == 1 ){//下载全部文件
				
				//取传输模式
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
					iSendMode = 2;
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
					iSendMode = 10;
				} else {//error
					iSendMode = 0;
					sprintf( errMsg, "ENV文件中找到不能识别的文件传输模式 - %s", struDataInfo.modeInfo[ iIndex ].cMode );
					return 0;
				}
				
				//下载完成后源文件的处理模式
				memset( cSource, 0, 200 );
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//不处理
					
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//删除
					strcpy( cSource, "*" );
				} else {//备份
					strcpy( cSource, struDataInfo.modeInfo[ iIndex ].cSource );
				}
				
				memset( cLocalFile, 0, 1024 );
				memset( cRemoteFile, 0, 1024 );
				
				sprintf( cLocalFile, "%s/%s", struDataInfo.cLocal, cFileName );
				sprintf( cRemoteFile, "%s/%s", struDataInfo.cRemote, cFileName );
				
				if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'Y' )
					iCoverFlag = TARGET_COVER;
				else if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'N' )
					iCoverFlag = TARGET_STORE;
				
				printf( "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n%s\n%s", cLocalFile, cRemoteFile );
				iRes = ftp.Upload( cLocalFile, cRemoteFile, iCoverFlag, errMsg, iSendMode, cSource );
				if ( iRes != 1 ) {
					theLog<<"上传文件"<<uploadPath<<"/"<<cFileName<<"失败，失败原因:"<<errMsg<< endw;
					continue;
				} else {
					theLog<<"上传文件"<<uploadPath<<"/"<<cFileName<<"成功！"<<endi;
					continue;	
				}
			} else { //下载匹配文件
				//循环匹配
				iMatch = 0;
				iIndex = 0;
				for ( int i = 0 ; i < struDataInfo.iModeCount ; i ++ ){
					if ( IsFileMatch( cFileName,struDataInfo.modeInfo[ i ].cStyle ) == 1 ){
						iMatch = 1;
						break;
					}
					iIndex ++;
				}
				
				if ( iMatch == 0 ) continue;
				
				if ( iMatch == 1 ){//匹配
					//下载文件
					//取传输模式
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
						iSendMode = 2;
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
						iSendMode = 10;
					} else {//error
						iSendMode = 0;
						sprintf( errMsg, "ENV文件中找到不能识别的文件传输模式 - %s", struDataInfo.modeInfo[ iIndex ].cMode );
						continue ;
					}
					
					//下载完成后源文件的处理模式
					memset( cSource, 0, 200 );
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//不处理
						
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//删除
						strcpy( cSource, "*" );
					} else {//备份
						strcpy( cSource, struDataInfo.modeInfo[ iIndex ].cSource );
					}
						
					memset( cLocalFile, 0, 1024 );
					memset( cRemoteFile, 0, 1024 );
					
					sprintf( cLocalFile, "%s/%s", struDataInfo.cLocal, cFileName );
					sprintf( cRemoteFile, "%s/%s", struDataInfo.cRemote, cFileName );
					
					if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'Y' )
						iCoverFlag = TARGET_COVER;
					else if ( struDataInfo.modeInfo[ iIndex ].cCover[0] == 'N' )
						iCoverFlag = TARGET_STORE;
					
					printf( "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n%s\n%s", cLocalFile, cRemoteFile );				
					iRes = ftp.Upload( cLocalFile, cRemoteFile, iCoverFlag, errMsg, iSendMode, cSource );
					if ( iRes != 1 ) {
						theLog<<"上传文件"<<uploadPath<<"/"<<cFileName<<"失败，失败原因:"<<errMsg<< endw;
						continue;
					} else {
						theLog<<"上传文件"<<uploadPath<<"/"<<cFileName<<"成功！"<<endi;
						continue;
					}
				}
			}
		}
	}
	ftp.Disconnect();
	return 1;
}

/*
* 判断是否是文件夹
*/
int CCJmanager::IsItDir(char *cPathName){
	struct stat 	struStat;
	
	if (stat(cPathName, &struStat) == -1){
		return -1;	
	}
	if(struStat.st_mode & 0040000){
		return 1;
	}else{
		return 0;
	}
}

/***************************************************
description:	根据ENV开始自动下载
input:	        
output:		
		errMsg:			错误信息
return:         	
		0:			不符合
		1:			符合	
programmer:	
		安	坤
date:		
		2005-08-23
*****************************************************/
int CCJmanager::BeginDownload( char *errMsg ){
	FILE *tmpfp;
	char cParam0[100], cParam1[1024], buf[2000];
	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 1024 );
	memset( buf, 0, 2000 );
	int		iRes = 0;
	int		iGradeGroup[30];
	int		iGradeCount = 0;
	
	//1 打开ENV文件
	printf( "m_cDownloadEnvPathName: %s\n", m_cDownloadEnvPathName );
	if( ( tmpfp = fopen( m_cDownloadEnvPathName, "r" ) ) == NULL ) {
		sprintf( errMsg, "ENV文件%s打开失败！\n", m_cDownloadEnvPathName );
		return 0;
	}
	
	//2 循环读文件，读取一行
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
			
		//3 判断是否为某优先级的首行
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		
		printf( "line: %s\n", buf );
		//如果是GRADE首行,则将所有grade值都添加入数组iGradeGroup
		if( strcmp( cParam0, "GRADE" ) == 0 ) {
			iGradeGroup[ iGradeCount ] = atoi( cParam1 );
			iGradeCount ++;
		}
	}	
	
	fclose( tmpfp );
	
	for ( int i = 0; i < iGradeCount ; i ++ ){
		printf( "begin :%d\n", i );
		DownloadByGrade( iGradeGroup[ i ], errMsg );	
	}
	
	return 1;
	
	
}

/***************************************************
description:	根据ENV开始自动下载
input:	        
output:		
		errMsg:			错误信息
return:         	
		0:			不符合
		1:			符合	
programmer:	
		安	坤
date:		
		2005-08-23
*****************************************************/
int CCJmanager::BeginUpload( char *errMsg ){
	FILE *tmpfp;
	char cParam0[100], cParam1[1024], buf[2000];
	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 1024 );
	memset( buf, 0, 2000 );
	int		iRes = 0;
	int		iGradeGroup[30];
	int		iGradeCount = 0;
	
	//1 打开ENV文件
//	printf( "m_cFtpEnvPathName: %s\n", m_cFtpEnvPathName );
	if( ( tmpfp = fopen( m_cUploadEnvPathName, "r" ) ) == NULL ) {
		sprintf( errMsg, "ENV文件%s打开失败！\n", m_cUploadEnvPathName );
		return 0;
	}
	//2 循环读文件，读取一行
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
		//3 判断是否为某优先级的首行
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		//如果是GRADE首行,则将所有grade值都添加入数组iGradeGroup
		if( strcmp( cParam0, "GRADE" ) == 0 ) {
			iGradeGroup[ iGradeCount ] = atoi( cParam1 );
			iGradeCount ++;
		}
	}	
	
	fclose( tmpfp );
	
//	for ( int i = 0; i < iGradeCount ; i ++ ){
//		printf( "iGradeGroup[ %d ] = %d\n", i, iGradeGroup[ i ] );	
//	}
	
	for ( int i = 0; i < iGradeCount ; i ++ ){
		//printf( "begin :%d\n", i );
		UploadByGrade( iGradeGroup[ i ], errMsg );	
	}
	
	return 1;
	
	
}

/***************************************************
description:	日志的初始化设置
input:	        
output:		
		logPathName:		日志文件路径+文件名
return:         	
		0:			不符合
		1:			符合	
programmer:	
		安	坤
date:		
		2005-08-23
*****************************************************/
int CCJmanager::DealWithLog( char *logPathName ){
	theLog.Open( logPathName );
	return 1;
}

/***************************************************
description:	去掉字符串两边的空格,Tab,回车
input:	
		cStr:		要处理的字符串
output:		
		cStr：		处理后的字符串
return:
programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
void CCJmanager::Trim(char *cStr){
	int iLen = strlen(cStr);
	int iLeft = 0;
	int iRight = iLen;
	
	for (int i=0;i<iLen;i++){
		if (cStr[i] != ' ' && cStr[i] != '	' && cStr[i] != '\n') break;
		iLeft ++;
	}
	if (iLeft == iLen){
		memset(cStr, 0, iLen);
		return;
	}
	
	for (int i=iLen-1;i>=0;i--){
		if (cStr[i] != ' ' && cStr[i] != '	' && cStr[i] != '\n') break;
		iRight = i;			
	}
	if (iLeft == 0 && iRight == iLen) return ;
	char		*pC;
	pC = new char[iLen];
	memset(pC, 0, sizeof(pC));
	char		*pp;
	pp = cStr;

	pp += iLeft;
	memset(pC, 0, iLen);
	strncpy(pC, pp, iRight-iLeft);
	memset(cStr, 0, iLen);
	strcpy(cStr, pC);
}

/***************************************************
description:	从ENV文件中分解STYLE,MODE,COVER,SOURCE的各项
input:	
		cLine:		要处理的字符串
		iFlag:		0:		STYLE
				1：		MODE
				2：		COVERSOURCE
				3：		SOURCE
output:		
		gradeInfo:	要填充的结构体
return:
programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
int CCJmanager::GetDetail( const char *cLine, GRADE_INFO *gradeInfo, int iFlag ){
//	printf( "cline: %s\n", cLine );
	int		iLen = 0;
	char		*cCur = NULL;
	int		iIndex = 0;
	char		cTmp[200], cNewLine[1024];
	
	memset( cNewLine, 0, 1024 );
	strcpy( cNewLine, cLine );
	
	char		*pLine = cNewLine;
	
	while ( ( cCur = strstr( pLine, ";" ) ) != NULL ){
		iLen = cCur - pLine;
//		printf( "iLen = %d\n", iLen );
//		
		memset( cTmp, 0, 200 );
		strncpy( cTmp, pLine, iLen );
		Trim( cTmp );
//		printf( "cTmp : %s\n", cTmp );
		
		if ( iFlag == 0 ){//STYLE
			strcpy( gradeInfo->modeInfo[ iIndex ].cStyle, cTmp );
		} else if ( iFlag == 1 ){//MODE
			strcpy( gradeInfo->modeInfo[ iIndex ].cMode, cTmp );
		} else if ( iFlag == 2 ){//COVER
			strcpy( gradeInfo->modeInfo[ iIndex ].cCover, cTmp );
		} else if ( iFlag == 3 ){//SOURCE
			strcpy( gradeInfo->modeInfo[ iIndex ].cSource, cTmp );
		}
		
		pLine = ++ cCur;
		iIndex ++;
	}
	
	//提取最后一项
	if ( iFlag == 0 ){//STYLE
		strcpy( gradeInfo->modeInfo[ iIndex ].cStyle, pLine );
	} else if ( iFlag == 1 ){//MODE
		strcpy( gradeInfo->modeInfo[ iIndex ].cMode, pLine );
	} else if ( iFlag == 2 ){//COVER
		strcpy( gradeInfo->modeInfo[ iIndex ].cCover, pLine );
	} else if ( iFlag == 3 ){//SOURCE
		strcpy( gradeInfo->modeInfo[ iIndex ].cSource, pLine );
	}
	
	iIndex ++;
	if ( gradeInfo->iModeCount < iIndex )
		gradeInfo->iModeCount = iIndex;
	
	return 1;
	
}

/***************************************************
description:	检查GRADE_INFO的数据的完整性
input:	
		gradeInfo:		待检查的结构体
output:		
		cErrMsg :		错误信息
return:
		-1：			LOCAL的设置为空
		-2：			REMOTE的设置为空
		-3：			CHILD的设置为空
		-4：			TREE的设置为空
		-5：			STYLE的设置的的数据数量不够
		-6：			MODE的设置的的数据数量不
		-7：			COVER的设置的的数据数量不
		-8：			SOURCE的设置的的数据数量不
programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
int CCJmanager::CheckGradeInfo( GRADE_INFO gradeInfo, char *cErrMsg ){
	//检查固定项
	if ( strlen( gradeInfo.cLocal ) == 0 ){
		strcpy( cErrMsg, "LOCAL的设置为空" );	
		return -1;
	}
	
	if ( strlen( gradeInfo.cRemote ) == 0 ){
		strcpy( cErrMsg, "REMOTE的设置为空" );	
		return -2;
	}
	
	if ( strlen( gradeInfo.cChild ) == 0 ){
		strcpy( cErrMsg, "CHILD的设置为空" );	
		return -3;
	}
	
	if ( strlen( gradeInfo.cTree ) == 0 ){
		strcpy( cErrMsg, "TREE的设置为空" );	
		return -4;
	}

	if ( strlen( gradeInfo.cCompress ) == 0 ){
		strcpy( cErrMsg, "COMPRESS的设置为空" );	
		return -9;
	}

	if ( strlen( gradeInfo.cPass ) == 0 ){
		strcpy( cErrMsg, "PASS的设置为空" );	
		return -10;
	}

	//检查数组
	for ( int i = 0 ; i < gradeInfo.iModeCount ; i ++ ){
		if ( strlen( gradeInfo.modeInfo[ i ].cStyle ) == 0 ){
			strcpy( cErrMsg, "STYLE的设置的的数据数量不够" );	
			return -5;
		}
		
		if ( strlen( gradeInfo.modeInfo[ i ].cMode ) == 0 ){
			strcpy( cErrMsg, "MODE的设置的的数据数量不够" );	
			return -6;
		}

	//	printf( "len: %d\n", strlen( gradeInfo.modeInfo[ i ].cCover ) );
		if ( strlen( gradeInfo.modeInfo[ i ].cCover ) == 0 ){
			strcpy( cErrMsg, "COVER的设置的的数据数量不够" );	
			return -7;
		}
		
		if ( strlen( gradeInfo.modeInfo[ i ].cSource ) == 0 ){
			strcpy( cErrMsg, "SOURCE的设置的的数据数量不够" );	
			return -8;
		}
		
			
	}

	return 1;
}

/***************************************************
description:	得到服务器信息
input:	
		gradeInfo:		待检查的结构体
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		-1：			IP的设置为空
		-2：			USER的设置为空
		-3：			PASSWORD的设置为空

programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
int CCJmanager::GetServerInfo( SERVER_INFO *struGradeInfo, char *cErrMsg, int iMode ){
	FILE *tmpfp;
	char cParam0[100], cParam1[1024], buf[2000];
	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 1024 );
	memset( buf, 0, 2000 );
	
	//1 打开ENV文件
//	printf( "m_cFtpEnvPathName: %s\n", m_cFtpEnvPathName );
	char		m_cFtpEnvPathName[ 1024 ];
	memset( m_cFtpEnvPathName, 0, 1024 );
	
	strcpy( m_cFtpEnvPathName, ( iMode == 0 ? m_cDownloadEnvPathName : m_cUploadEnvPathName ) );

	if( ( tmpfp = fopen( m_cFtpEnvPathName, "r" ) ) == NULL ) {
		strcpy( cErrMsg, "ENV文件打开失败！\n" );
		return 0;
	}
	//2 循环读文件，读取一行
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
		//3 判断是否为某优先级的首行
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		//如果是GRADE首行
		if( strcmp( cParam0, "SERVER" ) == 0 ) {
			//printf( "1\n" );
			//4 继续往下读，判断是否为PATH行
			//如果级别值符合
//			if ( strcmp( cParam1, cGrade ) == 0 ){
				//printf( "0: %s, 1: %s\n", cParam0, cParam1 );
				//继续往下读数据内容
				memset( buf, 0, 2000 );
				while( fgets( buf, 2000, tmpfp ) != NULL ) {
					
					Trim( buf );
					
					//#为注释行
					if( buf[0] == '#' ) {
						//printf( "in me : %c\n", buf[0] );
						continue;
					}
						
					//遇到空行则退出
					//printf( "buf: <%s>\nlen: %d\n", buf, strlen( buf ) );					
					if ( strlen( buf ) == 1 ){
						//fclose( tmpfp );
						//printf( "} out\n" );
						//return 1;
						//printf( "111111111111111111111out\n" );
						goto OUT;
					}	
					
					memset( cParam0, 0, 100 );
					memset( cParam1, 0, 1024 );
					sscanf( buf, "%s %s", cParam0, cParam1);
					//printf( "1 = %s, 2 = %s\n", cParam0, cParam1 );
					//如果是GRADE首行
					if( strcmp( cParam0, "IP" ) == 0 ) {
						strcpy( struGradeInfo->cIp, cParam1 );
					} else if( strcmp( cParam0, "USER" ) == 0 ) {
						strcpy( struGradeInfo->cUser, cParam1 );
					} else if( strcmp( cParam0, "PASSWORD" ) == 0 ) {
						strcpy( struGradeInfo->cPassword, cParam1 );
					} 
				}
//			} 
		}
	}
OUT:	
	//sprintf( cErrMsg, "ENV文件中没有优先级=%d的设置!\n", iGrade );
	fclose( tmpfp );
	
	return ( CheckServerInfo( *struGradeInfo, cErrMsg ) );	
}

/***************************************************
description:	检查SERVER_INFO的数据的完整性
input:	
		gradeInfo:		待检查的结构体
output:		
		cErrMsg :		错误信息
return:
		-1：			IP的设置为空
		-2：			USER的设置为空
		-3：			PASSWORD的设置为空
programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
int CCJmanager::CheckServerInfo( SERVER_INFO gradeInfo, char *cErrMsg ){
	//检查固定项
	if ( strlen( gradeInfo.cIp ) == 0 ){
		strcpy( cErrMsg, "IP的设置为空" );	
		return -1;
	}
	
	if ( strlen( gradeInfo.cUser ) == 0 ){
		strcpy( cErrMsg, "USER的设置为空" );	
		return -2;
	}
	
	if ( strlen( gradeInfo.cPassword ) == 0 ){
		strcpy( cErrMsg, "PASSWORD的设置为空" );	
		return -3;
	}
	
	return 1;
}

/***************************************************
description:	下载某指定文件列表中的文件
input:	
		listName:		文件列表名
		listPath：		文件列表路径
		sendInfo:		传输设置结构体,定义在FTPmanager.h中
					注意：此时在sendInfo中localFile是文件在本地的保存路径，
					      remoteFile是要下载的文件的远程路径
		serverInfo:		服务器信息
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			指定的文件列表不存在
		-1：			打开文件列表失败
		-2：			登录服务器失败
		-3：			PASSWORD的设置为空
programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
int CCJmanager::DownloadFromList( const char *listName, const char *listPath, SERVER_INFO serverInfo, SEND_INFO sendInfo, char *errMsg ){
	//得到文件列表全路径
	char		cFullName[1024];
	memset( cFullName, 0, 1024 );
	sprintf( cFullName, "%s/%s", listPath, listName );
	
	if ( IsFileExist( cFullName ) != 1 ){
		sprintf( errMsg, "指定的文件列表不存在\n" );	
		return 0;
	}
	
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( cFullName, "r" ) ) == NULL ){
		sprintf( errMsg, "打开文件列表%s失败！\n", cFullName );
		return -1;	
	}
	char		cBuf[2000];
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
		
	//GRADE_INFO		struNewData;
	int		iRes = 0;

//	char		cTarget[2000];
	//char		cTargetPath[2000];
	
//	memset( cTarget, 0, 2000 );
	//memset( cTargetPath, 0, 2000 );
	//保存值
	//strcpy( cTargetPath, sendInfo.localFile );
	
	//保存远程和本地路径
	char		cLocalPath[1000], cRemotePath[1000];
	memset( cLocalPath, 0, 1000 );
	memset( cRemotePath, 0, 1000 );
	
	strcpy( cLocalPath, sendInfo.localFile );
	strcpy( cRemotePath, sendInfo.remoteFile );
	
	//连接服务器
	CFTPmanager		ftp;
	
	//调用解密接口
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );

	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ){
		return -2;	
	}
	
	int		iIndex = 0;		//modeInfo数组中对应的INDEX号
	int		iAllFile = 0;		//”是否下载全部文件“的标志
	int		iMatch = 0;		//文件名和要求是否匹配
	int		iSendMode = 2;		//传输模式（2/10进制）
	char		cSource[200];		//下载完成后源文件的处理模式
	
	char		cTargetPathName[1024];
	memset( cBuf, 0, 2000 );
	while( fgets( cBuf, 2000, fFile ) != NULL ) {		
		//解析文件属性和文件名
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 100 );
		memset( cParam2, 0, 100 );
		memset( cParam3, 0, 100 );
		memset( cParam4, 0, 100 );
		memset( cParam5, 0, 100 );
		memset( cParam6, 0, 100 );
		memset( cParam7, 0, 100 );
		memset( cParam8, 0, 100 );
		
		sscanf( cBuf, "%s %s %s %s %s %s %s %s %s", 
		cParam0, cParam1, cParam2, cParam3, cParam4, cParam5, cParam6, cParam7, cParam8);
		
		if ( strcmp( cParam0, "total" ) == 0 ) continue;
		
		if ( ( strcmp( cParam8, "." ) == 0 ) || ( strcmp( cParam8, ".." ) == 0 ) ){
			continue;
		}

		//判断文件属性
		if ( cParam0[0] == 'd' ){//文件夹
			continue ;
		} else { //文件	
			memset( sendInfo.localFile, 0, 1024 );
			memset( sendInfo.remoteFile, 0, 1024 );
			//strcpy( sendInfo.remoteFile, cParam8 );
			
//			memset( cTarget, 0, 2000 );
			sprintf( sendInfo.localFile, "%s/%s", cLocalPath, cParam8 );
			sprintf( sendInfo.remoteFile, "%s/%s", cRemotePath, cParam8 );
			
			iRes = ftp.Download( sendInfo, errMsg );
			if ( iRes != 1 ) {
				theLog<<"下载文件"<<sendInfo.remoteFile<<"失败，失败原因:"<<errMsg<< endw;
				memset( cBuf, 0, 2000 );
				continue;
			} else {
				memset( cBuf, 0, 2000 );
				theLog<<"下载文件"<<sendInfo.remoteFile<<"成功！"<<endi;
				continue;	
			}
		}
	}
	
	ftp.Disconnect();
		
	return 1;
}

/***************************************************
description:	将某一指定文件列表中指定时间段内的文件组成新列表
input:	
		oldListFile:		老文件列表路径+文件名
		newListFile：		老文件列表路径+文件名
		beginTime:		起始日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		endTime:		结束日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
int CCJmanager::GetWindowsListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg ){
	char		cDate[15], cDate2[15], cTime[10], cTime2[10];
	memset( cDate, 0, 15 );
	memset( cDate2, 0, 15 );
	memset( cTime, 0, 10 );
	memset( cTime2, 0, 10 );

//检查beginTime	
	//分解beginTime
	sscanf( beginTime, "%s %s", cDate, cTime);
	
	char		cMyDate[20];
	memset( cMyDate, 0, 20 );
	strcpy( cMyDate, cDate );
	
	//检查空格两边有否内容
	if ( ( strlen( cDate ) <= 0 ) ){
		strcpy( errMsg, "从起始日中分解日期出错\n" );
		return 0;	
	}
	
	//检查日期中的两个"-"号
	char		*p;
	
	//提取年
	p = strtok( cDate, "-" );
	if ( p ){//检查年的范围
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "日期不能小于0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取月
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "月份的值越界，只能在1到12之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取日
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "日期的值越界，只能在1到31之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime ) > 0 ){
		//提取小时
		p = strtok( cTime, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到23之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//提取分钟
		p = strtok( NULL, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到59之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	

//检查endTime	
	//分解beginTime
	sscanf( endTime, "%s %s", cDate2, cTime2);
	char		cMyDate2[20];
	memset( cMyDate2, 0, 20 );
	strcpy( cMyDate2, cDate2 );
	
	
	//检查空格两边有否内容
	if ( ( strlen( cDate2 ) <= 0 ) ){
		strcpy( errMsg, "从结束日中分解日期出错\n" );
		return 0;	
	}
	
	//检查日期中的两个"-"号
	//char		*p;
	
	//提取年
	p = strtok( cDate2, "-" );
	if ( p ){//检查年的范围
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "日期不能小于0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取月
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "月份的值越界，只能在1到12之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取日
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "日期的值越界，只能在1到31之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime2 ) > 0 ){
		//提取小时
		p = strtok( cTime2, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到23之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//提取分钟
		p = strtok( NULL, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到59之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	
	
	//通过格式检查
	
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "打开要被提取的文件列表%s失败\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "创建新文件列表%s失败\n", newListFile );
		fclose( fFile );
		return -1;	
	}	
	
	char		buf[1024], cNewDate[20];;
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
	char		cCurDate[20], cCurYear[10], cCurMonth[10];;
	
	
	memset( buf, 0, 1024 );
	while( fgets( buf, 1024, fFile ) != NULL ) {
		
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
		
		if ( strcmp( cParam0, "total" ) == 0 ) continue;
		
		if ( strcmp( cParam8, "." ) == 0 ) continue;

		if ( strcmp( cParam8, ".." ) == 0 ) continue;
		
		if ( cParam0[0] == 'd' ) continue;
				
		//得到当前年份
		memset( cCurDate, 0, 20 );
		memset( cCurYear, 0, 10 );
		getCurDate( cCurDate );
		strncpy( cCurYear, cCurDate, 4 );
		
		//得到月数字
		memset( cCurMonth, 0, 10 );
		if ( GetMonth( cParam5, cCurMonth ) != 1 ){
			sprintf( errMsg, "不可识别的月份%s\n", cParam5 );	
			continue;
		}
		
		memset( cNewDate, 0, 20 );
		
		//判断是否今年
		p = NULL;
		p = strstr( cParam7, ":" );

		if ( p ){//是今年
			//printf( "jin: %s\n", cParam8 );
			//组成日期比较字符串		
			sprintf( cNewDate, "%s-%s-%02s %s", cCurYear, cCurMonth, cParam6, cParam7 );
			//比较
			if ( ( strcmp( cNewDate, beginTime ) ) >= 0 
			&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//符合
				//写一行到新文件
				write( fFile2, buf, strlen( buf ) ); 
			}
		} else {//不是今年
			sprintf( cNewDate, "%s-%s-%02s", cCurYear, cCurMonth, cParam6 );
			if ( ( strcmp( cNewDate, cMyDate ) ) >= 0 
			&&   ( strcmp( cNewDate, cMyDate2 ) <= 0 ) ){//符合
				//写一行到新文件
				write( fFile2, buf, strlen( buf ) ); 
			}
		}	
				
	} 	
	
	close( fFile2 );
	fclose( fFile );
	
	return 1;
}

/***************************************************
description:	将某一指定文件列表中指定时间段内的文件组成新列表
input:	
		oldListFile:		老文件列表路径+文件名
		newListFile：		老文件列表路径+文件名
		beginTime:		起始日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		endTime:		结束日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-08-29
*****************************************************/
int CCJmanager::GetUnixListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg ){
	char		cDate[15], cDate2[15], cTime[10], cTime2[10];
	memset( cDate, 0, 15 );
	memset( cDate2, 0, 15 );
	memset( cTime, 0, 10 );
	memset( cTime2, 0, 10 );

//检查beginTime	
	//分解beginTime
	sscanf( beginTime, "%s %s", cDate, cTime);
	
	char		cMyDate[20];
	memset( cMyDate, 0, 20 );
	strcpy( cMyDate, cDate );
	
	//检查空格两边有否内容
	if ( ( strlen( cDate ) <= 0 ) ){
		strcpy( errMsg, "从起始日中分解日期出错\n" );
		return 0;	
	}
	
	//检查日期中的两个"-"号
	char		*p;
	
	//提取年
	p = strtok( cDate, "-" );
	if ( p ){//检查年的范围
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "日期不能小于0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取月
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "月份的值越界，只能在1到12之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取日
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "日期的值越界，只能在1到31之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime ) > 0 ){
		//提取小时
		p = strtok( cTime, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到23之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//提取分钟
		p = strtok( NULL, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到59之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	

//检查endTime	
	//分解beginTime
	sscanf( endTime, "%s %s", cDate2, cTime2);
	char		cMyDate2[20];
	memset( cMyDate2, 0, 20 );
	strcpy( cMyDate2, cDate2 );
	
	
	//检查空格两边有否内容
	if ( ( strlen( cDate2 ) <= 0 ) ){
		strcpy( errMsg, "从结束日中分解日期出错\n" );
		return 0;	
	}
	
	//检查日期中的两个"-"号
	//char		*p;
	
	//提取年
	p = strtok( cDate2, "-" );
	if ( p ){//检查年的范围
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "日期不能小于0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取月
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "月份的值越界，只能在1到12之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//提取日
	p = strtok( NULL, "-" );
	if ( p ){//检查年的范围
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "日期的值越界，只能在1到31之间\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "日期的格式错误，正确格式：XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime2 ) > 0 ){
		//提取小时
		p = strtok( cTime2, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到23之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//提取分钟
		p = strtok( NULL, ":" );
		if ( p ){//检查年的范围
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "小时的值越界，只能在0到59之间\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "日期中的时间格式错误，正确日期格式：XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	
	
	//通过格式检查
	
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "打开要被提取的文件列表%s失败\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "创建新文件列表%s失败\n", newListFile );
		fclose( fFile );
		return -1;	
	}	
	
	char		buf[1024], cNewDate[20];;
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
//	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
	char		cCurDate[20], cCurYear[10], cCurMonth[10];;
	
	
	memset( buf, 0, 1024 );
	while( fgets( buf, 1024, fFile ) != NULL ) {
		
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 100 );
		memset( cParam2, 0, 100 );
		memset( cParam3, 0, 100 );
		memset( cParam4, 0, 100 );
//		memset( cParam5, 0, 100 );
//		memset( cParam6, 0, 100 );
//		memset( cParam7, 0, 100 );
//		memset( cParam8, 0, 100 );
		
		sscanf( buf, "%s %s %s %s %s", 
		cParam0, cParam1, cParam2, cParam3, cParam4);
		
		//if ( strcmp( cParam0, "total" ) == 0 ) continue;
		
		if ( strcmp( cParam3, "." ) == 0 ) continue;

		if ( strcmp( cParam3, ".." ) == 0 ) continue;
		
		if ( strcmp( cParam2, "<DIR>" ) == 0 ) continue;
				
		if ( buf[0] == ' ' ) continue;
						
		//得到当前年份
		memset( cCurDate, 0, 20 );
		memset( cCurYear, 0, 10 );
		getCurDate( cCurDate );
		//strncpy( cCurYear, cCurDate, 4 );
		
		//得到月数字
//		memset( cCurMonth, 0, 10 );
//		if ( GetMonth( cParam5, cCurMonth ) != 1 ){
//			sprintf( errMsg, "不可识别的月份%s\n", cParam5 );	
//			continue;
//		}
		
		memset( cNewDate, 0, 20 );
		sprintf( cNewDate, "%s %s", cParam0, cParam1 );
		//比较
		if ( ( strcmp( cNewDate, beginTime ) ) >= 0 
		&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//符合
			//写一行到新文件
			write( fFile2, buf, strlen( buf ) ); 
		}	
				
	} 	
	
	close( fFile2 );
	fclose( fFile );
	
	return 1;
}

/*
根据月份的英文得到月份的数字字符串
*/
int CCJmanager::GetMonth( const char *oldMonth, char *newMonth ){
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
description:	将某一指定文件列表中指定时间段内的文件组成新列表
input:	
		path:			要得到列表的路径
		listFile：		文件列表路径+文件名
		beginTime:		起始日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		endTime:		结束日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		serverInfo:		服务器连接信息
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-09-01
*****************************************************/
int CCJmanager::GetUnixListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//调用解密接口
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//连接
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//得到列表
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//转换
	iRes = GetUnixListByTime( cListPathFile, listFile, beginTime, endTime, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();
}

/***************************************************
description:	将某一指定文件列表中指定时间段内的文件组成新列表
input:	
		path:			要得到列表的路径
		listFile：		文件列表路径+文件名
		beginTime:		起始日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		endTime:		结束日期,日期格式："2005-08-08 18:55"（日期和时间中用空格隔开）
		serverInfo:		服务器连接信息
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-09-01
*****************************************************/
int CCJmanager::GetWindowsListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//调用解密接口
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//连接
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//得到列表
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//转换
	iRes = GetWindowsListByTime( cListPathFile, listFile, beginTime, endTime, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();
}

/***************************************************
description:	将某一指定文件列表中符合某条件（正则表达式）的文件组成新列表
input:	
		oldListFile:		要被转换的文件列表名+路径
		listFile：		新文件列表路径+文件名
		condition:		匹配条件（正则表达式：*.dat, a*k.*x等）
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-09-01
*****************************************************/
int CCJmanager::GetWindowsListByCondition( const char *oldListFile,
			 const char *newListFile, const char *condition, char *errMsg ){

	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "打开要被提取的文件列表%s失败\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "创建新文件列表%s失败\n", newListFile );
		fclose( fFile );
		return -1;	
	}	
	
	char		buf[1024], cNewDate[20];;
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
	char		cCondition[200];
	
	memset( cCondition, 0, 200 );
	strcpy( cCondition, condition );
	
	
	memset( buf, 0, 1024 );
	while( fgets( buf, 1024, fFile ) != NULL ) {
		
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
		
		if ( strcmp( cParam0, "total" ) == 0 ) continue;
		
		if ( strcmp( cParam8, "." ) == 0 ) continue;

		if ( strcmp( cParam8, ".." ) == 0 ) continue;
		
		if ( cParam0[0] == 'd' ) continue;
		
		if ( IsFileMatch( cParam8, cCondition ) != 1 ) continue;
				
		write( fFile2, buf, strlen( buf ) ); 	
	} 	
	
	close( fFile2 );
	fclose( fFile );
	
	return 1;	
}

/***************************************************
description:	将某一指定文件列表中符合某条件（正则表达式）的文件组成新列表
input:	
		oldListFile:		要被转换的文件列表名+路径
		listFile：		新文件列表路径+文件名
		condition:		匹配条件（正则表达式：*.dat, a*k.*x等）
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-09-01
*****************************************************/
int CCJmanager::GetUnixListByCondition( const char *oldListFile,
			 const char *newListFile, const char *condition, char *errMsg ){

	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "打开要被提取的文件列表%s失败\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "创建新文件列表%s失败\n", newListFile );
		fclose( fFile );
		return -1;	
	}	
	
	char		buf[1024], cNewDate[20];;
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
//	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
	char		cCondition[200];
	
	memset( cCondition, 0, 200 );
	strcpy( cCondition, condition );
	
	
	memset( buf, 0, 1024 );
	while( fgets( buf, 1024, fFile ) != NULL ) {
		
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 100 );
		memset( cParam2, 0, 100 );
		memset( cParam3, 0, 100 );
		memset( cParam4, 0, 100 );
//		memset( cParam5, 0, 100 );
//		memset( cParam6, 0, 100 );
//		memset( cParam7, 0, 100 );
//		memset( cParam8, 0, 100 );
		
		sscanf( buf, "%s %s %s %s %s", 
		cParam0, cParam1, cParam2, cParam3, cParam4);
		
		//if ( strcmp( cParam0, "total" ) == 0 ) continue;
		
		if ( strcmp( cParam3, "." ) == 0 ) continue;

		if ( strcmp( cParam3, ".." ) == 0 ) continue;
		
		if ( strcmp( cParam2, "<DIR>" ) ) continue;
		
		if ( buf[0] == ' ' ) continue;
		
		if ( IsFileMatch( cParam4, cCondition ) != 1 ) continue;
				
		write( fFile2, buf, strlen( buf ) ); 	
	} 	
	
	close( fFile2 );
	fclose( fFile );
	
	return 1;	
}

/***************************************************
description:	获得某一远程路径下的符合某一指定条件（正则表达式）的文件列表
input:	
		path:			要取得文件列表的远程路径
		listFile：		文件列表路径+文件名
		condition:		匹配条件（正则表达式：*.dat, a*k.*x等）
		serverInfo:		服务器连接信息
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-09-01
*****************************************************/
int CCJmanager::GetUnixListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//调用解密接口
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//连接
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//得到列表
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//转换
	iRes = GetUnixListByCondition( cListPathFile, listFile, condition, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();					
}

/***************************************************
description:	获得某一远程路径下的符合某一指定条件（正则表达式）的文件列表
input:	
		path:			要取得文件列表的远程路径
		listFile：		文件列表路径+文件名
		condition:		匹配条件（正则表达式：*.dat, a*k.*x等）
		serverInfo:		服务器连接信息
output:		
		cErrMsg :		错误信息
return:
		1:			成功
		0:			失败
programmer:	
		安	坤
data:		2005-09-01
*****************************************************/
int CCJmanager::GetWindowsListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//调用解密接口
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "解密用户名出错\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//连接
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//得到列表
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//转换
	iRes = GetWindowsListByCondition( cListPathFile, listFile, condition, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();					
}
