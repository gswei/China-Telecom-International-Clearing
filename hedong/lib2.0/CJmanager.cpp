/* CJmanager.cpp */
#include "CJmanager.h"

CCJmanager::CCJmanager(){
	
}

CCJmanager::~CCJmanager(){
	
}

/***************************************************
description:	����FTP���ȼ����õ�ENV�ļ�ȫ·��+�ļ���
input:	        
		cFtpEnvPathName:			FTP���ȼ����õ�ENV�ļ�ȫ·��+�ļ���
output:         
return:         		
programmer:	��	��
date:		2005-08-19
*****************************************************/ 
void CCJmanager::SetEnvPathName( char *cDownloadEnvPathName, char *cUploadEnvPathName ){
	strcpy( m_cDownloadEnvPathName, cDownloadEnvPathName );
	strcpy( m_cUploadEnvPathName, cUploadEnvPathName );
}

/***************************************************
description:	�õ�ĳһ���ȼ�����Ϣ
input:	        
		iGrade:			���ȼ�
		iMode:			0:		������
					1��		�ϴ���
output:		
		struGradeInfo:		ȡ�õ����ȼ�����Ϣ
		cErrMsg:		������Ϣ
return:         	
		1:			�ɹ�
		-1��			LOCAL������Ϊ��
		-2��			REMOTE������Ϊ��
		-3��			CHILD������Ϊ��
		-4��			TREE������Ϊ��
		-5��			STYLE�����õĵ�������������
		-6��			MODE�����õĵ�����������
		-7��			COVER�����õĵ�����������
		-8��			SOURCE�����õĵ�����������	
programmer:	
		��	��
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
	//1 ��ENV�ļ�
//	printf( "m_cFtpEnvPathName: %s\n", m_cFtpEnvPathName );
	char		m_cFtpEnvPathName[ 1024 ];
	memset( m_cFtpEnvPathName, 0, 1024 );
	
	strcpy( m_cFtpEnvPathName, ( iMode == 0 ? m_cDownloadEnvPathName : m_cUploadEnvPathName ) );

	if( ( tmpfp = fopen( m_cFtpEnvPathName, "r" ) ) == NULL ) {
		strcpy( cErrMsg, "ENV�ļ���ʧ�ܣ�\n" );
		return 0;
	}
	//2 ѭ�����ļ�����ȡһ��
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
		//3 �ж��Ƿ�Ϊĳ���ȼ�������
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		//�����GRADE����
		if( strcmp( cParam0, "GRADE" ) == 0 ) {
			printf( "1\n" );
			//4 �������¶����ж��Ƿ�ΪPATH��
			//�������ֵ����
			if ( strcmp( cParam1, cGrade ) == 0 ){
				printf( "0: %s, 1: %s\n", cParam0, cParam1 );
				//�������¶���������
				memset( buf, 0, 2000 );
				while( fgets( buf, 2000, tmpfp ) != NULL ) {
					
					Trim( buf );
					
					//#Ϊע����
					if( buf[0] == '#' ) {
						//printf( "in me : %c\n", buf[0] );
						continue;
					}
						
					//�����������˳�
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
					//�����GRADE����
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
	//sprintf( cErrMsg, "ENV�ļ���û�����ȼ�=%d������!\n", iGrade );
	fclose( tmpfp );
	
	return ( CheckGradeInfo( *struGradeInfo, cErrMsg ) );
}

/***************************************************
description:	�������ȼ�Ϊgrade���ļ�
input:	        
		iGrade:			���ȼ�
output:		
		cErrMsg:		������Ϣ
return:         	
		0:			ʧ��
		1:			�ɹ�	
programmer:	
		��	��
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::DownloadByGrade( int grade, char *errMsg ){
	GRADE_INFO		struGradeInfo;
	SERVER_INFO		struServerInfo;
	int			iRes = 0;
	
	//1 ȡ�ü�����Ϣ
	memset( &struGradeInfo, 0, sizeof( GRADE_INFO ) );
	memset( &struServerInfo, 0, sizeof( SERVER_INFO ) );
	struGradeInfo.iModeCount = 0;

	iRes = GetServerInfo( &struServerInfo, errMsg, 0 );
	printf( "serverMsg:\n%s\n", errMsg );
	if ( iRes != 1 ) return 0;
	
	iRes = GetGradeInfo( grade, &struGradeInfo, errMsg, 0 );
	printf( "gradeMsg:\n%s\n", errMsg );
	if ( iRes != 1 ) return 0;
	
	//����
	iRes = DownloadFrom( struGradeInfo.cRemote, struServerInfo,
					struGradeInfo, errMsg );
	if ( iRes != 1 ){
		theLog << "��"<<grade<<"�����ļ�����ʧ�ܣ�ʧ��ԭ��:"<<errMsg<<endw;
	} else {
		theLog << "��"<<grade<<"�����ļ�������ɣ�"<<endi;
	}
	
	return 1;
}

/***************************************************
description:	�ϴ����ȼ�Ϊgrade���ļ�
input:	        
		iGrade:			���ȼ�
output:		
		cErrMsg:		������Ϣ
return:         	
		0:			ʧ��
		1:			�ɹ�	
programmer:	
		��	��
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::UploadByGrade( int grade, char *errMsg ){
	GRADE_INFO		struGradeInfo;
	SERVER_INFO		struServerInfo;
	int			iRes = 0;
	
	//1 ȡ�ü�����Ϣ
	memset( &struGradeInfo, 0, sizeof( GRADE_INFO ) );
	memset( &struServerInfo, 0, sizeof( SERVER_INFO ) );
	struGradeInfo.iModeCount = 0;

	iRes = GetServerInfo( &struServerInfo, errMsg, 1 );
	if ( iRes != 1 ) return 0;
	
	iRes = GetGradeInfo( grade, &struGradeInfo, errMsg, 1 );
	if ( iRes != 1 ) return 0;
	
	//����
	iRes = UploadFrom( struGradeInfo.cLocal, struServerInfo,
					struGradeInfo, errMsg );
	if ( iRes != 1 ){
		theLog << "��"<<grade<<"�����ļ��ϴ�ʧ�ܣ�ʧ��ԭ��:"<<errMsg<<endw;
	} else {
		theLog << "��"<<grade<<"�����ļ��ϴ���ɣ�"<<endi;
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
//	//1 ȡ�ü�����Ϣ
//	iRes = GetGradeInfo( grade, &struGradeInfo, errMsg, 1 );
//	if ( iRes != 1 ) return 0;
//	
////	printf( "!!!!!!!!!!!!!!!!!!!\n " );
//	//2 ����ÿ������ȡ�ļ��б�
//	for ( int i = 0 ; i < struGradeInfo.iDataCount ; i ++ ){
//		//�ϴ�
//		iRes = UploadFrom( struGradeInfo.struDataInfo[i].cLocal, 
//						struGradeInfo.struDataInfo[i], errMsg );
//		if ( iRes != 1 ){
//			theLog << "��"<<grade<<"���ĵ�"<<i<<"�������ϴ�ʧ�ܣ�ʧ��ԭ��:"<<errMsg<<endw;
//			//printf( "��%d���ĵ�%d�������ϴ�ʧ�ܣ�ʧ��ԭ��\n%s", grade, i, errMsg );
//		} else {
//			theLog << "��"<<grade<<"���ĵ�"<<i<<"�������ϴ����!"<<endi;
//			//printf( "��%d���ĵ�%d�������ϴ��ɹ���\n", grade, i );
//		}
//	}
	
	return 1;
}

/***************************************************
description:	����ĳ�ļ����µ������ļ�
input:	        
		downloadPath:			FTP�������ϵ�����·��
		struDataInfo:			����������Ϣ
output:		
		cErrMsg:			������Ϣ
return:         	
		0:			ʧ��
		1:			�ɹ�	
programmer:	
		��	��
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::DownloadFrom( char *downloadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg ){
	printf( "from1111111111111\n" );
	int			iRes = 0;
	CFTPmanager		ftp;
	char			cListName[10];
	
	memset( cListName, 0, 10 );
	
	//���ý��ܽӿ�
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;

	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	//����
	iRes = ftp.Connect( serverInfo.cIp, cRealUser, cRealPw, errMsg );
	if ( iRes != 1 ) return 0;
	
	//�õ��б��ļ���
	for ( int i = 0 ; ; i ++ ){
		memset( cListName, 0, 10 );
		sprintf( cListName, "%d.lst", i );
		if ( IsFileExist( cListName ) != 1 ) break;
	}

	//�õ��ļ��б�
	iRes = ftp.List( struDataInfo.cRemote, cListName, errMsg );
	if ( iRes != 1 ) return 0;

	//�ȴ�Ӳ�̴����ļ�
	while( 1 ){
		if ( IsFileExist( cListName ) == 1 ) break;
		//printf( "waiting...\n" );
	}
	
	//ѭ����ȡһ�У��������ļ���
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( cListName, "r" ) ) == NULL ){
		sprintf( errMsg, "��%s·�����ļ��б�ʧ�ܣ�\n", downloadPath );
		return 0;	
	}
	char		cBuf[2000];
	char		cParam0[100], cParam1[100], cParam2[100], cParam3[100], cParam4[100]; 		
	char		cParam5[100], cParam6[100], cParam7[100], cParam8[100]; 
		
	GRADE_INFO		struNewData;

	char		cTarget[2000];
	memset( cTarget, 0, 2000 );
	
	int		iIndex = 0;		//modeInfo�����ж�Ӧ��INDEX��
	int		iAllFile = 0;		//���Ƿ�����ȫ���ļ����ı�־
	int		iMatch = 0;		//�ļ�����Ҫ���Ƿ�ƥ��
	int		iSendMode = 2;		//����ģʽ��2/10���ƣ�
	char		cSource[200];		//������ɺ�Դ�ļ��Ĵ���ģʽ
	
	char		cLocalFile[1024], cRemoteFile[1024];
	int		iCoverFlag = 0;
	
//	CEncryptFile		encc;
//	CF_CLzwCompressNoCatch	com;
	
	memset( cBuf, 0, 2000 );
	while( fgets( cBuf, 2000, fFile ) != NULL ) {		
		//�����ļ����Ժ��ļ���
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
		
		//�������ļ��Ĵ���
		if ( strcmp( cParam0, "----------" ) == 0 ) continue;
		
		//��ʱ�ļ��Ĵ���
		if ( ( cParam8[0] == '~' ) || ( IsFileMatch( cParam8, "*.tmp" ) == 1 ))
			continue;
			
		int iFiSize = 0;

		//�ж��ļ�����
		if ( cParam0[0] == 'd' ){//�ļ���
		//	printf( "jia\n" );
			//�ж�INCLUDECHILDREN
			if ( strcmp( struDataInfo.cChild, "Y" ) == 0 ){//����
				//�ж�BUILDTREE
				
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
					
				if ( strcmp( struDataInfo.cTree, "Y" ) == 0 ){//����TREE
					//printf( "first\n" );
					sprintf( struNewData.cLocal, "%s/%s", struDataInfo.cLocal, cParam8 );
					//�ڱ��ش����ļ���
					//printf( "local: %s\n", struNewData.cLocal );
					mkdir( struNewData.cLocal, S_IRWXU|S_IRGRP|S_IXGRP);			
				} else {//���ý���TREE
					strcpy( struNewData.cRemote, struDataInfo.cRemote );
				}	

//				printf( "path: %s\n", struNewData.cPath );
				iRes = DownloadFrom( struNewData.cRemote, serverInfo, struNewData, errMsg );
				if ( iRes != 1 ){
					theLog <<errMsg<< ende;
					//printf( "%s", errMsg );	
				}
			}
			
		} else { //�ļ�
			//�ж��ļ����Ƿ�����
			
			//�ж��Ƿ�����ȫ���ļ�
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
				//ȡ����ģʽ
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
					iSendMode = 2;
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
					iSendMode = 10;
				} else {//error
					iSendMode = 0;
					sprintf( errMsg, "ENV�ļ����ҵ�����ʶ����ļ�����ģʽ - %s", struDataInfo.modeInfo[ iIndex ].cMode );
					return 0;
				}
				
				//������ɺ�Դ�ļ��Ĵ���ģʽ
				memset( cSource, 0, 200 );
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//������
					strcpy( cSource, SOURCE_IGNORE );
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//ɾ��
					strcpy( cSource, SOURCE_DELE );
				} else {//����
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
			//	//�Ƚ��ļ�����ѹ���ļ���
			//	char		cPreFile[300], cPassFile[300];
			//	memset( cPreFile, 0, 300 );
			//	memset( cPassFile, 0, 300 );
			//	sprintf( cPreFile, "%s.prs", cLocalFile );
				
				iRes = ftp.Download( cLocalFile, cRemoteFile, iCoverFlag, errMsg, iSendMode, cSource, struDataInfo.cCompress, struDataInfo.cPass );//cSource
				if ( iRes != 1 ) {
					theLog<<"�����ļ�"<<downloadPath<<"/"<<cParam8<<"ʧ�ܣ�ʧ��ԭ��:"<<errMsg<< endw;
					memset( cBuf, 0, 2000 );
					continue;
				} else {
			//		//��ѹ��
			//		if ( strcmp( struDataInfo.cCompress, "Y" ) == 0 ) {
			//			com.uncompress( cLocalFile, cPreFile );
			//		} else {
			//			rename( cLocalFile, cPreFile );
			//		}
			//		
			//		//cPreFile�ǽ�ѹ������ļ�
			//		
			//		//����
			//		if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//������
			//			//��ȡ�������ļ���
			//			int iPos;
			//			iPos=strrncspn( cLocalFile, '.', 3 );
			//			//if(iPos<0) return 0;
			//			strncpy( cPassFile , cLocalFile, iPos );
			//			//cPassFile[iPos]=0;
			//			
			//			encc.unEncrypt( cPassFile, cPreFile, struDataInfo.cPass );
			//		} else {//û����
			//			//��ȡ�������ļ���
			//			int iPos2;
			//			iPos2=strrncspn( cLocalFile, '.', 2 );
			//			//if(iPos<0) return 0;
			//			strcpy( cPassFile , cLocalFile );
			//			cPassFile[iPos2]=0;
			//			
			//			rename( cPassFile, cPreFile );
			//		}
			//		
			//		//cPassFile�ǽ�ѹ���ͽ��ܺ���ļ���,ͬʱҲ�Ѿ�ȥ�����ļ���С��������Ϣ,���������ļ���
			//		
			//		//�˶��ļ���С
			//		//��ȡ�ļ���С
			//		if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//������
			//			//��ȡ�ļ���С
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
			//			//��ȡ�ļ���С
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
			//		//�Ƚ��ļ���С
			//		if ( atoi(cParam4) != iFiSize ) {
			//			theLog<<"�����ļ�"<<downloadPath<<"/"<<cParam8<<"ʧ��,ʧ��ԭ��:����ǰ���ļ���С������"<<endi;
			//		}
			//		
			//		//����Դ�ļ�
			//		if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//ɾ����ʽ
			//			ftp.RmFile( cRemoteFile, errMsg );
			//		} else if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//������ʽ
			//			
			//		} else {//���ݷ�ʽ
			//			ftp.MoveFile( cRemoteFile, cSource, errMsg );
			//		}
					
					memset( cBuf, 0, 2000 );
					theLog<<"�����ļ�"<<downloadPath<<"/"<<cParam8<<"�ɹ���"<<endi;
					continue;	
				}
			} else {
				//ѭ��ƥ��
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
				
				if ( iMatch == 1 ){//ƥ��
					//�����ļ�
					//ȡ����ģʽ
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
						iSendMode = 2;
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
						iSendMode = 10;
					} else {//error
						iSendMode = 0;
						sprintf( errMsg, "ENV�ļ����ҵ�����ʶ����ļ�����ģʽ - %s", struDataInfo.modeInfo[ iIndex ].cMode );
						continue ;
					}
					
					//������ɺ�Դ�ļ��Ĵ���ģʽ
					memset( cSource, 0, 200 );
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//������
						
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//ɾ��
						strcpy( cSource, "*" );
					} else {//����
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
					//�Ƚ��ļ�����ѹ���ļ���
			//		char		cPreFile[300], cPassFile[300];
			//		memset( cPreFile, 0, 300 );
			//		memset( cPassFile, 0, 300 );
			//		sprintf( cPreFile, "%s.tmp", cLocalFile );
					
					iRes = ftp.Download( cLocalFile, cRemoteFile, iCoverFlag, errMsg, iSendMode, cSource, struDataInfo.cCompress, struDataInfo.cPass );
					if ( iRes != 1 ) {
						memset( cBuf, 0, 2000 );
						theLog<<"�����ļ�"<<downloadPath<<"/"<<cParam8<<"ʧ�ܣ�ʧ��ԭ��:"<<errMsg<< endw;
						continue;
					} else {
			//			//��ѹ��
			//			if ( strcmp( struDataInfo.cCompress, "Y" ) == 0 ) {
			//				int t = com.uncompress( cLocalFile, cPreFile );
			//				printf( "ttt= %d\n", t );
			//			} else {
			//				rename( cLocalFile, cPreFile );
			//			}
			//			
			//			remove( cLocalFile );
			//			
			//			//cPreFile�ǽ�ѹ������ļ�.prs
			//			
			//			//����
			//			if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//������
			//				//��ȡ�������ļ���
			//				int iPos;
			//				iPos=strrncspn( cLocalFile, '.', 3 );
			//				//if(iPos<0) return 0;
			//				strncpy( cPassFile , cLocalFile, iPos );
			//				//cPassFile[iPos]=0;
			//				
			//				int iii = encc.unEncrypt( cPreFile, cPassFile, struDataInfo.cPass );
			//				printf( "iii = %d\n", iii );
			//			} else {//û����
			//				//��ȡ�������ļ���
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
			//			//cPassFile�ǽ�ѹ���ͽ��ܺ���ļ���,ͬʱҲ�Ѿ�ȥ�����ļ���С��������Ϣ,���������ļ���
			//			
			//			//�˶��ļ���С
			//			//��ȡ�ļ���С
			//			if ( strcmp( struDataInfo.cPass, "*" ) != 0 ) {//������
			//				//��ȡ�ļ���С
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
			//				//��ȡ�ļ���С
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
			//			//�Ƚ��ļ���С
			//			int iReSize = GetFileSize( cPassFile );
			//			if ( iReSize != iFiSize ) {
			//				theLog<<"�����ļ�"<<downloadPath<<"/"<<cParam8<<"ʧ��,ʧ��ԭ��:����ǰ���ļ���С������"<<endi;
			//			}
			//			
			//			//����Դ�ļ�
			//			if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//ɾ����ʽ
			//				ftp.RmFile( cRemoteFile, errMsg );
			//			} else if ( strcmp( SOURCE_DELE, cSource ) == 0 ) {//������ʽ
			//				
			//			} else {//���ݷ�ʽ
			//				//ȡ���ļ���
			//				char	cName[50],cPath[300];
			//				memset( cName, 0, 50 );
			//				memset( cPath, 0, 300 );
			//				ftp.DivFileName( cRemoteFile, cPath, cName );
			//				sprintf( cSource, "%s/%s", cSource, cName );
			//				ftp.MoveFile( cRemoteFile, cSource, errMsg );
			//			}
						memset( cBuf, 0, 2000 );
						theLog<<"�����ļ�"<<downloadPath<<"/"<<cParam8<<"�ɹ���"<<endi;
						continue;
					}
				}
			}
			
		}
		memset( cBuf, 0, 2000 );
	}
	
	fclose( fFile );
	iRes = remove( cListName );
	
	//�Ͽ�����
	ftp.Disconnect();
	//printf( "over!\n" );    
	return 1;
}

//�õ��ļ���С
int CCJmanager::GetFileSize( const char *filePathName ) {
	struct stat buf;
	if ( stat( filePathName, &buf ) == -1 ) {
		printf("error!\n");
		return -1;
	}
	
	return buf.st_size;

}

/***************************************************
description:	�ж��ļ��Ƿ����
input:	        
output:		
return:         	
		0:			������
		1:			����	
programmer:	
		��	��
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::IsFileExist(char *p_cPathName){
	return ( ( access( p_cPathName, 0 ) == 0 ) ? 1 : 0 ); 
}

/***************************************************
description:	�ж��ļ����Ƿ����������ʽ����
input:	        
		cOldFileName:		�ļ���
		cCondition��		������ʽ����
output:		
return:         	
		0:			������
		1:			����	
programmer:	
		��	��
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::IsFileMatch( char *cOldFileName, char *cCondition ){
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
	
	pCur = strstr( cFileName, "*" );
	if ( ( pCur - cFileName ) >= 0 ){//����
		//����*�ĵط��Ӹ�.�Է��Ͻ�����ʽ
		pCur = NULL;
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
		//printf( "%d,%d\n", pmatch[0].rm_so, pmatch[0].rm_eo );
		//int 		iRtn = 0;
		///if ( ( pmatch[0].rm_so >= 0 ) && ( pmatch[0].rm_eo > 0 ) ) iRtn = 1;
		//else iRtn = 0;
		int iRtn = ( ( ( pmatch[0].rm_so >= 0 ) && ( pmatch[0].rm_eo > 0 ) ) ? 1 : 0 );
		//printf( "%d,%d\n", pmatch[1].rm_so, pmatch[1].rm_eo );
		regfree(&struPreg);
		return iRtn;
		
	} else {//����
		//�Ƚ��Ƿ���ȫƥ��
		return ( strcmp( cFileName, cCondition ) == 0 ? 1 : 0 );
	}
}

/***************************************************
description:	�ϴ�ĳ�ļ����µ������ļ�
input:	        
		uploadPath:		Ҫ�ϴ��ı���·���ļ���
		struDataInfo��		ENV�е�������Ϣ
output:		
return:         	
		0:			������
		1:			����	
programmer:	
		��	��
date:		
		2005-08-22
*****************************************************/ 
int CCJmanager::UploadFrom( char *uploadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg ){
	//printf( "in\n" );
	int			iRes = 0;
	CFTPmanager		ftp;
	
	//���ý��ܽӿ�
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;

	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	//����
	iRes = ftp.Connect( serverInfo.cIp, cRealUser, cRealPw, errMsg );
	if ( iRes != 1 ) return 0;

	DIR		*dDir;
	struct dirent	*struDir;
	char		cFileName[100], cPathName[2000];
	memset( cFileName, 0, 100 );
	memset( cPathName, 0, 2000 );

	
	//���ļ��б�
	if ((dDir = opendir(struDataInfo.cLocal)) == NULL){
		return 0;	
	}
	
	int		iIndex = 0;		//modeInfo�����ж�Ӧ��INDEX��
	int		iAllFile = 0;		//���Ƿ�����ȫ���ļ����ı�־
	int		iMatch = 0;		//�ļ�����Ҫ���Ƿ�ƥ��
	int		iSendMode = 2;		//����ģʽ��2/10���ƣ�
	char		cSource[200];		//������ɺ�Դ�ļ��Ĵ���ģʽ
	
	int		iCoverFlag = 0;
	GRADE_INFO		struNewData;
	char			cLocalFile[1024], cRemoteFile[1024];
	while((struDir = readdir(dDir)) != NULL){
		if(!struDir->d_ino){
			continue;
		}
		//�õ���ǰ�ļ�ȫ·��
		strcpy(cFileName, struDir->d_name);
		sprintf(cPathName, "%s/%s", uploadPath, cFileName);
		//printf( "cPathName = %s\n", cPathName );
		if (strcmp(cFileName, ".") == 0){
			continue;	
		}
		if (strcmp(cFileName, "..") == 0){
			continue;	
		}
				
		//�ж��ļ�����
		if ( IsItDir( cPathName ) == 1 ){//�ļ���
			//�ж�INCLUDECHILDREN
			if ( strcmp( struDataInfo.cChild, "Y" ) == 0 ){//����
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
					
				if ( strcmp( struDataInfo.cTree, "Y" ) == 0 ){//����TREE
					//printf( "first\n" );
					sprintf( struNewData.cRemote, "%s/%s", struDataInfo.cRemote, cFileName );
					//�ڱ��ش����ļ���
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
					
					//�ȴ�Ӳ���ϵ��ļ��д�����
					while(1){
						iRes = ftp.ChDir( struNewData.cRemote, errMsg );
						if ( iRes == 1 ) break;
						//printf( "wait!\n" );
					}		
				} else {//���ý���TREE
					strcpy( struNewData.cRemote, struDataInfo.cRemote );
				}
				
				//printf( "path: %s\n", struNewData.cPath );
				iRes = UploadFrom( struNewData.cRemote, serverInfo, struNewData, errMsg );
				if ( iRes != 1 ){
					theLog << errMsg << endw;
					//printf( "%s", errMsg );	
				}
			}
			
		} else { //�ļ�
			
			//�ж��Ƿ�����ȫ���ļ�
			iAllFile = 0;
			iIndex = 0;
			for ( int i = 0 ; i < struDataInfo.iModeCount ; i ++ ){
				if ( strcmp( struDataInfo.modeInfo[ i ].cStyle, "*" ) == 0 ){
					iAllFile = 1;
					break;	
				}
				iIndex ++;
			}
			
			if ( iAllFile == 1 ){//����ȫ���ļ�
				
				//ȡ����ģʽ
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
					iSendMode = 2;
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
					iSendMode = 10;
				} else {//error
					iSendMode = 0;
					sprintf( errMsg, "ENV�ļ����ҵ�����ʶ����ļ�����ģʽ - %s", struDataInfo.modeInfo[ iIndex ].cMode );
					return 0;
				}
				
				//������ɺ�Դ�ļ��Ĵ���ģʽ
				memset( cSource, 0, 200 );
				if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//������
					
				} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//ɾ��
					strcpy( cSource, "*" );
				} else {//����
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
					theLog<<"�ϴ��ļ�"<<uploadPath<<"/"<<cFileName<<"ʧ�ܣ�ʧ��ԭ��:"<<errMsg<< endw;
					continue;
				} else {
					theLog<<"�ϴ��ļ�"<<uploadPath<<"/"<<cFileName<<"�ɹ���"<<endi;
					continue;	
				}
			} else { //����ƥ���ļ�
				//ѭ��ƥ��
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
				
				if ( iMatch == 1 ){//ƥ��
					//�����ļ�
					//ȡ����ģʽ
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "BIN") == 0 ){
						iSendMode = 2;
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cMode, "ASC") == 0 ) {
						iSendMode = 10;
					} else {//error
						iSendMode = 0;
						sprintf( errMsg, "ENV�ļ����ҵ�����ʶ����ļ�����ģʽ - %s", struDataInfo.modeInfo[ iIndex ].cMode );
						continue ;
					}
					
					//������ɺ�Դ�ļ��Ĵ���ģʽ
					memset( cSource, 0, 200 );
					if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "NULL" ) == 0 ){//������
						
					} else if ( strcmp( struDataInfo.modeInfo[ iIndex ].cSource, "DELE" ) == 0 ){//ɾ��
						strcpy( cSource, "*" );
					} else {//����
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
						theLog<<"�ϴ��ļ�"<<uploadPath<<"/"<<cFileName<<"ʧ�ܣ�ʧ��ԭ��:"<<errMsg<< endw;
						continue;
					} else {
						theLog<<"�ϴ��ļ�"<<uploadPath<<"/"<<cFileName<<"�ɹ���"<<endi;
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
* �ж��Ƿ����ļ���
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
description:	����ENV��ʼ�Զ�����
input:	        
output:		
		errMsg:			������Ϣ
return:         	
		0:			������
		1:			����	
programmer:	
		��	��
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
	
	//1 ��ENV�ļ�
	printf( "m_cDownloadEnvPathName: %s\n", m_cDownloadEnvPathName );
	if( ( tmpfp = fopen( m_cDownloadEnvPathName, "r" ) ) == NULL ) {
		sprintf( errMsg, "ENV�ļ�%s��ʧ�ܣ�\n", m_cDownloadEnvPathName );
		return 0;
	}
	
	//2 ѭ�����ļ�����ȡһ��
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
			
		//3 �ж��Ƿ�Ϊĳ���ȼ�������
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		
		printf( "line: %s\n", buf );
		//�����GRADE����,������gradeֵ�����������iGradeGroup
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
description:	����ENV��ʼ�Զ�����
input:	        
output:		
		errMsg:			������Ϣ
return:         	
		0:			������
		1:			����	
programmer:	
		��	��
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
	
	//1 ��ENV�ļ�
//	printf( "m_cFtpEnvPathName: %s\n", m_cFtpEnvPathName );
	if( ( tmpfp = fopen( m_cUploadEnvPathName, "r" ) ) == NULL ) {
		sprintf( errMsg, "ENV�ļ�%s��ʧ�ܣ�\n", m_cUploadEnvPathName );
		return 0;
	}
	//2 ѭ�����ļ�����ȡһ��
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
		//3 �ж��Ƿ�Ϊĳ���ȼ�������
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		//�����GRADE����,������gradeֵ�����������iGradeGroup
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
description:	��־�ĳ�ʼ������
input:	        
output:		
		logPathName:		��־�ļ�·��+�ļ���
return:         	
		0:			������
		1:			����	
programmer:	
		��	��
date:		
		2005-08-23
*****************************************************/
int CCJmanager::DealWithLog( char *logPathName ){
	theLog.Open( logPathName );
	return 1;
}

/***************************************************
description:	ȥ���ַ������ߵĿո�,Tab,�س�
input:	
		cStr:		Ҫ������ַ���
output:		
		cStr��		�������ַ���
return:
programmer:	
		��	��
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
description:	��ENV�ļ��зֽ�STYLE,MODE,COVER,SOURCE�ĸ���
input:	
		cLine:		Ҫ������ַ���
		iFlag:		0:		STYLE
				1��		MODE
				2��		COVERSOURCE
				3��		SOURCE
output:		
		gradeInfo:	Ҫ���Ľṹ��
return:
programmer:	
		��	��
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
	
	//��ȡ���һ��
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
description:	���GRADE_INFO�����ݵ�������
input:	
		gradeInfo:		�����Ľṹ��
output:		
		cErrMsg :		������Ϣ
return:
		-1��			LOCAL������Ϊ��
		-2��			REMOTE������Ϊ��
		-3��			CHILD������Ϊ��
		-4��			TREE������Ϊ��
		-5��			STYLE�����õĵ�������������
		-6��			MODE�����õĵ�����������
		-7��			COVER�����õĵ�����������
		-8��			SOURCE�����õĵ�����������
programmer:	
		��	��
data:		2005-08-29
*****************************************************/
int CCJmanager::CheckGradeInfo( GRADE_INFO gradeInfo, char *cErrMsg ){
	//���̶���
	if ( strlen( gradeInfo.cLocal ) == 0 ){
		strcpy( cErrMsg, "LOCAL������Ϊ��" );	
		return -1;
	}
	
	if ( strlen( gradeInfo.cRemote ) == 0 ){
		strcpy( cErrMsg, "REMOTE������Ϊ��" );	
		return -2;
	}
	
	if ( strlen( gradeInfo.cChild ) == 0 ){
		strcpy( cErrMsg, "CHILD������Ϊ��" );	
		return -3;
	}
	
	if ( strlen( gradeInfo.cTree ) == 0 ){
		strcpy( cErrMsg, "TREE������Ϊ��" );	
		return -4;
	}

	if ( strlen( gradeInfo.cCompress ) == 0 ){
		strcpy( cErrMsg, "COMPRESS������Ϊ��" );	
		return -9;
	}

	if ( strlen( gradeInfo.cPass ) == 0 ){
		strcpy( cErrMsg, "PASS������Ϊ��" );	
		return -10;
	}

	//�������
	for ( int i = 0 ; i < gradeInfo.iModeCount ; i ++ ){
		if ( strlen( gradeInfo.modeInfo[ i ].cStyle ) == 0 ){
			strcpy( cErrMsg, "STYLE�����õĵ�������������" );	
			return -5;
		}
		
		if ( strlen( gradeInfo.modeInfo[ i ].cMode ) == 0 ){
			strcpy( cErrMsg, "MODE�����õĵ�������������" );	
			return -6;
		}

	//	printf( "len: %d\n", strlen( gradeInfo.modeInfo[ i ].cCover ) );
		if ( strlen( gradeInfo.modeInfo[ i ].cCover ) == 0 ){
			strcpy( cErrMsg, "COVER�����õĵ�������������" );	
			return -7;
		}
		
		if ( strlen( gradeInfo.modeInfo[ i ].cSource ) == 0 ){
			strcpy( cErrMsg, "SOURCE�����õĵ�������������" );	
			return -8;
		}
		
			
	}

	return 1;
}

/***************************************************
description:	�õ���������Ϣ
input:	
		gradeInfo:		�����Ľṹ��
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		-1��			IP������Ϊ��
		-2��			USER������Ϊ��
		-3��			PASSWORD������Ϊ��

programmer:	
		��	��
data:		2005-08-29
*****************************************************/
int CCJmanager::GetServerInfo( SERVER_INFO *struGradeInfo, char *cErrMsg, int iMode ){
	FILE *tmpfp;
	char cParam0[100], cParam1[1024], buf[2000];
	memset( cParam0, 0, 100 );
	memset( cParam1, 0, 1024 );
	memset( buf, 0, 2000 );
	
	//1 ��ENV�ļ�
//	printf( "m_cFtpEnvPathName: %s\n", m_cFtpEnvPathName );
	char		m_cFtpEnvPathName[ 1024 ];
	memset( m_cFtpEnvPathName, 0, 1024 );
	
	strcpy( m_cFtpEnvPathName, ( iMode == 0 ? m_cDownloadEnvPathName : m_cUploadEnvPathName ) );

	if( ( tmpfp = fopen( m_cFtpEnvPathName, "r" ) ) == NULL ) {
		strcpy( cErrMsg, "ENV�ļ���ʧ�ܣ�\n" );
		return 0;
	}
	//2 ѭ�����ļ�����ȡһ��
	memset( buf, 0, 2000 );
	while( fgets( buf, 2000, tmpfp ) != NULL ) {
		if( buf[0] == '#' || buf[0] == '{' || buf[0] == '}' ) 
			continue;
		//3 �ж��Ƿ�Ϊĳ���ȼ�������
		memset( cParam0, 0, 100 );
		memset( cParam1, 0, 1024 );
		sscanf( buf, "%s %s", cParam0, cParam1);
		//�����GRADE����
		if( strcmp( cParam0, "SERVER" ) == 0 ) {
			//printf( "1\n" );
			//4 �������¶����ж��Ƿ�ΪPATH��
			//�������ֵ����
//			if ( strcmp( cParam1, cGrade ) == 0 ){
				//printf( "0: %s, 1: %s\n", cParam0, cParam1 );
				//�������¶���������
				memset( buf, 0, 2000 );
				while( fgets( buf, 2000, tmpfp ) != NULL ) {
					
					Trim( buf );
					
					//#Ϊע����
					if( buf[0] == '#' ) {
						//printf( "in me : %c\n", buf[0] );
						continue;
					}
						
					//�����������˳�
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
					//�����GRADE����
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
	//sprintf( cErrMsg, "ENV�ļ���û�����ȼ�=%d������!\n", iGrade );
	fclose( tmpfp );
	
	return ( CheckServerInfo( *struGradeInfo, cErrMsg ) );	
}

/***************************************************
description:	���SERVER_INFO�����ݵ�������
input:	
		gradeInfo:		�����Ľṹ��
output:		
		cErrMsg :		������Ϣ
return:
		-1��			IP������Ϊ��
		-2��			USER������Ϊ��
		-3��			PASSWORD������Ϊ��
programmer:	
		��	��
data:		2005-08-29
*****************************************************/
int CCJmanager::CheckServerInfo( SERVER_INFO gradeInfo, char *cErrMsg ){
	//���̶���
	if ( strlen( gradeInfo.cIp ) == 0 ){
		strcpy( cErrMsg, "IP������Ϊ��" );	
		return -1;
	}
	
	if ( strlen( gradeInfo.cUser ) == 0 ){
		strcpy( cErrMsg, "USER������Ϊ��" );	
		return -2;
	}
	
	if ( strlen( gradeInfo.cPassword ) == 0 ){
		strcpy( cErrMsg, "PASSWORD������Ϊ��" );	
		return -3;
	}
	
	return 1;
}

/***************************************************
description:	����ĳָ���ļ��б��е��ļ�
input:	
		listName:		�ļ��б���
		listPath��		�ļ��б�·��
		sendInfo:		�������ýṹ��,������FTPmanager.h��
					ע�⣺��ʱ��sendInfo��localFile���ļ��ڱ��صı���·����
					      remoteFile��Ҫ���ص��ļ���Զ��·��
		serverInfo:		��������Ϣ
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ָ�����ļ��б�����
		-1��			���ļ��б�ʧ��
		-2��			��¼������ʧ��
		-3��			PASSWORD������Ϊ��
programmer:	
		��	��
data:		2005-08-29
*****************************************************/
int CCJmanager::DownloadFromList( const char *listName, const char *listPath, SERVER_INFO serverInfo, SEND_INFO sendInfo, char *errMsg ){
	//�õ��ļ��б�ȫ·��
	char		cFullName[1024];
	memset( cFullName, 0, 1024 );
	sprintf( cFullName, "%s/%s", listPath, listName );
	
	if ( IsFileExist( cFullName ) != 1 ){
		sprintf( errMsg, "ָ�����ļ��б�����\n" );	
		return 0;
	}
	
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( cFullName, "r" ) ) == NULL ){
		sprintf( errMsg, "���ļ��б�%sʧ�ܣ�\n", cFullName );
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
	//����ֵ
	//strcpy( cTargetPath, sendInfo.localFile );
	
	//����Զ�̺ͱ���·��
	char		cLocalPath[1000], cRemotePath[1000];
	memset( cLocalPath, 0, 1000 );
	memset( cRemotePath, 0, 1000 );
	
	strcpy( cLocalPath, sendInfo.localFile );
	strcpy( cRemotePath, sendInfo.remoteFile );
	
	//���ӷ�����
	CFTPmanager		ftp;
	
	//���ý��ܽӿ�
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
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
	
	int		iIndex = 0;		//modeInfo�����ж�Ӧ��INDEX��
	int		iAllFile = 0;		//���Ƿ�����ȫ���ļ����ı�־
	int		iMatch = 0;		//�ļ�����Ҫ���Ƿ�ƥ��
	int		iSendMode = 2;		//����ģʽ��2/10���ƣ�
	char		cSource[200];		//������ɺ�Դ�ļ��Ĵ���ģʽ
	
	char		cTargetPathName[1024];
	memset( cBuf, 0, 2000 );
	while( fgets( cBuf, 2000, fFile ) != NULL ) {		
		//�����ļ����Ժ��ļ���
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

		//�ж��ļ�����
		if ( cParam0[0] == 'd' ){//�ļ���
			continue ;
		} else { //�ļ�	
			memset( sendInfo.localFile, 0, 1024 );
			memset( sendInfo.remoteFile, 0, 1024 );
			//strcpy( sendInfo.remoteFile, cParam8 );
			
//			memset( cTarget, 0, 2000 );
			sprintf( sendInfo.localFile, "%s/%s", cLocalPath, cParam8 );
			sprintf( sendInfo.remoteFile, "%s/%s", cRemotePath, cParam8 );
			
			iRes = ftp.Download( sendInfo, errMsg );
			if ( iRes != 1 ) {
				theLog<<"�����ļ�"<<sendInfo.remoteFile<<"ʧ�ܣ�ʧ��ԭ��:"<<errMsg<< endw;
				memset( cBuf, 0, 2000 );
				continue;
			} else {
				memset( cBuf, 0, 2000 );
				theLog<<"�����ļ�"<<sendInfo.remoteFile<<"�ɹ���"<<endi;
				continue;	
			}
		}
	}
	
	ftp.Disconnect();
		
	return 1;
}

/***************************************************
description:	��ĳһָ���ļ��б���ָ��ʱ����ڵ��ļ�������б�
input:	
		oldListFile:		���ļ��б�·��+�ļ���
		newListFile��		���ļ��б�·��+�ļ���
		beginTime:		��ʼ����,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		endTime:		��������,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-08-29
*****************************************************/
int CCJmanager::GetWindowsListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg ){
	char		cDate[15], cDate2[15], cTime[10], cTime2[10];
	memset( cDate, 0, 15 );
	memset( cDate2, 0, 15 );
	memset( cTime, 0, 10 );
	memset( cTime2, 0, 10 );

//���beginTime	
	//�ֽ�beginTime
	sscanf( beginTime, "%s %s", cDate, cTime);
	
	char		cMyDate[20];
	memset( cMyDate, 0, 20 );
	strcpy( cMyDate, cDate );
	
	//���ո������з�����
	if ( ( strlen( cDate ) <= 0 ) ){
		strcpy( errMsg, "����ʼ���зֽ����ڳ���\n" );
		return 0;	
	}
	
	//��������е�����"-"��
	char		*p;
	
	//��ȡ��
	p = strtok( cDate, "-" );
	if ( p ){//�����ķ�Χ
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "���ڲ���С��0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "�·ݵ�ֵԽ�磬ֻ����1��12֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "���ڵ�ֵԽ�磬ֻ����1��31֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime ) > 0 ){
		//��ȡСʱ
		p = strtok( cTime, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��23֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//��ȡ����
		p = strtok( NULL, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��59֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	

//���endTime	
	//�ֽ�beginTime
	sscanf( endTime, "%s %s", cDate2, cTime2);
	char		cMyDate2[20];
	memset( cMyDate2, 0, 20 );
	strcpy( cMyDate2, cDate2 );
	
	
	//���ո������з�����
	if ( ( strlen( cDate2 ) <= 0 ) ){
		strcpy( errMsg, "�ӽ������зֽ����ڳ���\n" );
		return 0;	
	}
	
	//��������е�����"-"��
	//char		*p;
	
	//��ȡ��
	p = strtok( cDate2, "-" );
	if ( p ){//�����ķ�Χ
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "���ڲ���С��0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "�·ݵ�ֵԽ�磬ֻ����1��12֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "���ڵ�ֵԽ�磬ֻ����1��31֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime2 ) > 0 ){
		//��ȡСʱ
		p = strtok( cTime2, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��23֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//��ȡ����
		p = strtok( NULL, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��59֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	
	
	//ͨ����ʽ���
	
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "��Ҫ����ȡ���ļ��б�%sʧ��\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "�������ļ��б�%sʧ��\n", newListFile );
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
				
		//�õ���ǰ���
		memset( cCurDate, 0, 20 );
		memset( cCurYear, 0, 10 );
		getCurDate( cCurDate );
		strncpy( cCurYear, cCurDate, 4 );
		
		//�õ�������
		memset( cCurMonth, 0, 10 );
		if ( GetMonth( cParam5, cCurMonth ) != 1 ){
			sprintf( errMsg, "����ʶ����·�%s\n", cParam5 );	
			continue;
		}
		
		memset( cNewDate, 0, 20 );
		
		//�ж��Ƿ����
		p = NULL;
		p = strstr( cParam7, ":" );

		if ( p ){//�ǽ���
			//printf( "jin: %s\n", cParam8 );
			//������ڱȽ��ַ���		
			sprintf( cNewDate, "%s-%s-%02s %s", cCurYear, cCurMonth, cParam6, cParam7 );
			//�Ƚ�
			if ( ( strcmp( cNewDate, beginTime ) ) >= 0 
			&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//����
				//дһ�е����ļ�
				write( fFile2, buf, strlen( buf ) ); 
			}
		} else {//���ǽ���
			sprintf( cNewDate, "%s-%s-%02s", cCurYear, cCurMonth, cParam6 );
			if ( ( strcmp( cNewDate, cMyDate ) ) >= 0 
			&&   ( strcmp( cNewDate, cMyDate2 ) <= 0 ) ){//����
				//дһ�е����ļ�
				write( fFile2, buf, strlen( buf ) ); 
			}
		}	
				
	} 	
	
	close( fFile2 );
	fclose( fFile );
	
	return 1;
}

/***************************************************
description:	��ĳһָ���ļ��б���ָ��ʱ����ڵ��ļ�������б�
input:	
		oldListFile:		���ļ��б�·��+�ļ���
		newListFile��		���ļ��б�·��+�ļ���
		beginTime:		��ʼ����,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		endTime:		��������,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-08-29
*****************************************************/
int CCJmanager::GetUnixListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg ){
	char		cDate[15], cDate2[15], cTime[10], cTime2[10];
	memset( cDate, 0, 15 );
	memset( cDate2, 0, 15 );
	memset( cTime, 0, 10 );
	memset( cTime2, 0, 10 );

//���beginTime	
	//�ֽ�beginTime
	sscanf( beginTime, "%s %s", cDate, cTime);
	
	char		cMyDate[20];
	memset( cMyDate, 0, 20 );
	strcpy( cMyDate, cDate );
	
	//���ո������з�����
	if ( ( strlen( cDate ) <= 0 ) ){
		strcpy( errMsg, "����ʼ���зֽ����ڳ���\n" );
		return 0;	
	}
	
	//��������е�����"-"��
	char		*p;
	
	//��ȡ��
	p = strtok( cDate, "-" );
	if ( p ){//�����ķ�Χ
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "���ڲ���С��0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "�·ݵ�ֵԽ�磬ֻ����1��12֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "���ڵ�ֵԽ�磬ֻ����1��31֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime ) > 0 ){
		//��ȡСʱ
		p = strtok( cTime, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��23֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//��ȡ����
		p = strtok( NULL, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��59֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	

//���endTime	
	//�ֽ�beginTime
	sscanf( endTime, "%s %s", cDate2, cTime2);
	char		cMyDate2[20];
	memset( cMyDate2, 0, 20 );
	strcpy( cMyDate2, cDate2 );
	
	
	//���ո������з�����
	if ( ( strlen( cDate2 ) <= 0 ) ){
		strcpy( errMsg, "�ӽ������зֽ����ڳ���\n" );
		return 0;	
	}
	
	//��������е�����"-"��
	//char		*p;
	
	//��ȡ��
	p = strtok( cDate2, "-" );
	if ( p ){//�����ķ�Χ
		if ( atoi( p ) <= 0 ){
			strcpy( errMsg, "���ڲ���С��0\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 12 ) ){
			strcpy( errMsg, "�·ݵ�ֵԽ�磬ֻ����1��12֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	} 
	
	//��ȡ��
	p = strtok( NULL, "-" );
	if ( p ){//�����ķ�Χ
		if ( ( atoi( p ) <= 0 ) || ( atoi( p ) > 31 ) ){
			strcpy( errMsg, "���ڵ�ֵԽ�磬ֻ����1��31֮��\n" );
			return 0;	
		}
	} else {
		strcpy( errMsg, "���ڵĸ�ʽ������ȷ��ʽ��XXXX-XX-XX XX:XX\n" );
		return 0;	
	}	

	
	p = NULL;
	if ( strlen( cTime2 ) > 0 ){
		//��ȡСʱ
		p = strtok( cTime2, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 23 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��23֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
		
		//��ȡ����
		p = strtok( NULL, ":" );
		if ( p ){//�����ķ�Χ
			if ( ( atoi( p ) < 0 ) || ( atoi( p ) > 59 ) ){
				strcpy( errMsg, "Сʱ��ֵԽ�磬ֻ����0��59֮��\n" );
				return 0;	
			}
		} else {
			strcpy( errMsg, "�����е�ʱ���ʽ������ȷ���ڸ�ʽ��XXXX-XX-XX XX:XX\n" );
			return 0;	
		} 
	}	
	
	//ͨ����ʽ���
	
	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "��Ҫ����ȡ���ļ��б�%sʧ��\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "�������ļ��б�%sʧ��\n", newListFile );
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
						
		//�õ���ǰ���
		memset( cCurDate, 0, 20 );
		memset( cCurYear, 0, 10 );
		getCurDate( cCurDate );
		//strncpy( cCurYear, cCurDate, 4 );
		
		//�õ�������
//		memset( cCurMonth, 0, 10 );
//		if ( GetMonth( cParam5, cCurMonth ) != 1 ){
//			sprintf( errMsg, "����ʶ����·�%s\n", cParam5 );	
//			continue;
//		}
		
		memset( cNewDate, 0, 20 );
		sprintf( cNewDate, "%s %s", cParam0, cParam1 );
		//�Ƚ�
		if ( ( strcmp( cNewDate, beginTime ) ) >= 0 
		&&   ( strcmp( cNewDate, endTime ) <= 0 ) ){//����
			//дһ�е����ļ�
			write( fFile2, buf, strlen( buf ) ); 
		}	
				
	} 	
	
	close( fFile2 );
	fclose( fFile );
	
	return 1;
}

/*
�����·ݵ�Ӣ�ĵõ��·ݵ������ַ���
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
description:	��ĳһָ���ļ��б���ָ��ʱ����ڵ��ļ�������б�
input:	
		path:			Ҫ�õ��б��·��
		listFile��		�ļ��б�·��+�ļ���
		beginTime:		��ʼ����,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		endTime:		��������,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		serverInfo:		������������Ϣ
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-09-01
*****************************************************/
int CCJmanager::GetUnixListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//���ý��ܽӿ�
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//����
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//�õ��б�
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//ת��
	iRes = GetUnixListByTime( cListPathFile, listFile, beginTime, endTime, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();
}

/***************************************************
description:	��ĳһָ���ļ��б���ָ��ʱ����ڵ��ļ�������б�
input:	
		path:			Ҫ�õ��б��·��
		listFile��		�ļ��б�·��+�ļ���
		beginTime:		��ʼ����,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		endTime:		��������,���ڸ�ʽ��"2005-08-08 18:55"�����ں�ʱ�����ÿո������
		serverInfo:		������������Ϣ
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-09-01
*****************************************************/
int CCJmanager::GetWindowsListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//���ý��ܽӿ�
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//����
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//�õ��б�
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//ת��
	iRes = GetWindowsListByTime( cListPathFile, listFile, beginTime, endTime, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();
}

/***************************************************
description:	��ĳһָ���ļ��б��з���ĳ������������ʽ�����ļ�������б�
input:	
		oldListFile:		Ҫ��ת�����ļ��б���+·��
		listFile��		���ļ��б�·��+�ļ���
		condition:		ƥ��������������ʽ��*.dat, a*k.*x�ȣ�
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-09-01
*****************************************************/
int CCJmanager::GetWindowsListByCondition( const char *oldListFile,
			 const char *newListFile, const char *condition, char *errMsg ){

	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "��Ҫ����ȡ���ļ��б�%sʧ��\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "�������ļ��б�%sʧ��\n", newListFile );
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
description:	��ĳһָ���ļ��б��з���ĳ������������ʽ�����ļ�������б�
input:	
		oldListFile:		Ҫ��ת�����ļ��б���+·��
		listFile��		���ļ��б�·��+�ļ���
		condition:		ƥ��������������ʽ��*.dat, a*k.*x�ȣ�
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-09-01
*****************************************************/
int CCJmanager::GetUnixListByCondition( const char *oldListFile,
			 const char *newListFile, const char *condition, char *errMsg ){

	FILE		*fFile = NULL;
	if ( ( fFile = fopen( oldListFile, "r" ) ) == NULL ){
		sprintf( errMsg, "��Ҫ����ȡ���ļ��б�%sʧ��\n", oldListFile );
		return -1;	
	}
	
	remove( newListFile );
	int		fFile2 = 0;
	if ( ( fFile2 = open( newListFile, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP ) ) == NULL ){
		sprintf( errMsg, "�������ļ��б�%sʧ��\n", newListFile );
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
description:	���ĳһԶ��·���µķ���ĳһָ��������������ʽ�����ļ��б�
input:	
		path:			Ҫȡ���ļ��б��Զ��·��
		listFile��		�ļ��б�·��+�ļ���
		condition:		ƥ��������������ʽ��*.dat, a*k.*x�ȣ�
		serverInfo:		������������Ϣ
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-09-01
*****************************************************/
int CCJmanager::GetUnixListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//���ý��ܽӿ�
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//����
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//�õ��б�
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//ת��
	iRes = GetUnixListByCondition( cListPathFile, listFile, condition, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();					
}

/***************************************************
description:	���ĳһԶ��·���µķ���ĳһָ��������������ʽ�����ļ��б�
input:	
		path:			Ҫȡ���ļ��б��Զ��·��
		listFile��		�ļ��б�·��+�ļ���
		condition:		ƥ��������������ʽ��*.dat, a*k.*x�ȣ�
		serverInfo:		������������Ϣ
output:		
		cErrMsg :		������Ϣ
return:
		1:			�ɹ�
		0:			ʧ��
programmer:	
		��	��
data:		2005-09-01
*****************************************************/
int CCJmanager::GetWindowsListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg ){
	CFTPmanager		ftp;
	int			iRes = 0;
	
	//���ý��ܽӿ�
	
	char		cRealUser[200];
	char		cRealPw[200];
	memset( cRealUser, 0, 200 );
	memset( cRealPw, 0, 200 );
	CEncryptAsc		enc;
	
	iRes = enc.Decrypt( serverInfo.cUser, cRealUser );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	iRes = enc.Decrypt( serverInfo.cPassword, cRealPw );
	if ( iRes != 0 ){
		strcpy( errMsg, "�����û�������\n" );
		return 0;	
	}
	
	memset( serverInfo.cUser, 0, 100 );
	memset( serverInfo.cPassword, 0, 100 );
	
	strcpy( serverInfo.cUser, cRealUser );
	strcpy( serverInfo.cPassword, cRealPw );
	
	//����
	iRes = ftp.Connect( serverInfo, errMsg );
	if ( iRes != 1 ) return 0;
	
	//�õ��б�
	char			cTmpListFile[200], cListPathFile[1024];
	memset( cTmpListFile, 0, 200 );
	memset( cListPathFile, 0, 1024 );
	
	sprintf( cTmpListFile, "~%s", listFile );
	sprintf( cListPathFile, "./%s", cTmpListFile );
	
	remove( cListPathFile );
	iRes = ftp.List( path, cListPathFile, errMsg );
	if ( iRes != 1 ) return 0;
	
	//ת��
	iRes = GetWindowsListByCondition( cListPathFile, listFile, condition, errMsg );
	if ( iRes != 1 ) return 0;
	
	remove( cListPathFile );
	
	ftp.Disconnect();					
}
