/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-28
File:			 CFileRoll.cpp
Description:	 �ļ�����ģ��
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "CFileRoll.h"
CLog theJSLog;
SGW_RTInfo	rtinfo;

CFileRoll::CFileRoll()
{
	//memset(szrollfiles,0,sizeof(szrollfiles));
	memset(m_filename,0,sizeof(m_filename));
	memset(input_path,0,sizeof(input_path));
	memset(output_path,0,sizeof(output_path));
	memset(erro_path,0,sizeof(erro_path));
	memset(erro_msg,0,sizeof(erro_msg));
}

CFileRoll::~CFileRoll()
{
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog<<tmp<<endi;
		}
	}
}


	//ģ���ʼ������
bool CFileRoll::init(int argc,char** argv)
{

     if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	//PS_Process::setSignalPrc();

	if(!drInit())	
	{
		return false;
	}

	if(!(rtinfo.connect()))
	{
		return false;
	}	
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;
	
	theJSLog.setLog(szLogPath, szLogLevel,"FILEROLLBACK", "GJJS", 001);
	theJSLog<<"��־·����"<<szLogPath<<" ��־����"<<szLogLevel<<endi;

	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	}

	try
	{
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  false;
		}
		Statement stmt = conn.createStatement();
		string sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'ROLLFILE_PATH'";
		stmt.setSQLString(sql);
		stmt.execute();//ִ��sql���
		stmt >> input_path;//��ȡ���
		completeDir(input_path);
		theJSLog <<"��ȡ�ļ�·��Ϊ"<<input_path<<endi;

		sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'ROLLFILE_BAK_PATH' ";
		stmt.setSQLString(sql);
		stmt.execute();//ִ��sql���
		stmt >> output_path;//��ȡ���
		completeDir(output_path);
		theJSLog <<"����·��Ϊ"<<output_path<<endi;

		sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'ROLLFILE_ERRO_PATH' ";
		stmt.setSQLString(sql);
		stmt.execute();//ִ��sql���
		stmt >> erro_path;//��ȡ���
		completeDir(erro_path);
		theJSLog <<"�����ļ�·��Ϊ"<<erro_path<<endi;

		if(LoadSourceCfg() == -1)	    //��������Դ������Ϣ 2013-08-22
		{
			return false;
		}

		stmt.close();
		conn.close();
	}
	catch(SQLException  e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ��ʼ��ʱ���ݿ��ѯ�쳣:%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return false ;
	}
   
     theJSLog<<"��ʼ�����"<<endi;
     return true ;
}

bool CFileRoll::getFilenames()
{	
   while(1)
   	{
	int ret=0,i;
	bool flag;
	char szBuff[1024];
	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		prcExit();
		return false;
	}
	
	//�ж����ݿ�״̬
	short db_status = 0;
	rtinfo.getDBSysMode(db_status);
	if(db_status != petri_status)
	{
		theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
		int cmd_sn = 0;
		if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
		{
			theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
			return ;
		}
		petri_status = db_status;
	}
	if(petri_status == DB_STATUS_OFFLINE)	return ;

	if(drStatus==1)  //��ϵͳ
	{
		//���trigger�����ļ��Ƿ����
		if(!CheckTriggerFile())
		{
			sleep(1);
			return 0;
		}

		//��ȡͬ������
		memset(m_SerialString,0,sizeof(m_SerialString));
		ret=drVarGetSet(m_SerialString);
		char str[256];
		memset(str,0,sizeof(str));
		strcpy(str,m_SerialString);
		//cout << "m_SerialString = " <<m_SerialString <<"str = "<<str<<endl;
		//vector<string> data;	
		if(ret)
		{
			theJSLog<<"ͬ��ʧ��..."<<endw;
			return 0;
		}
				
		vector<string> data;		
		//splitString(m_SerialString,";",data,false,false);
		splitString(str,";",data,false,false);
				
		memset(m_filename,0,sizeof(m_filename));
		strcpy(m_filename,input_path);
		strcat(m_filename,data[0].c_str());
		
		char filename_tmp[256];
		char Filename[256];
        memset(Filename,0,sizeof(Filename));
		char* p = strrchr(m_filename,'/');
		if(p)
			strcpy(Filename,p+1);
		else
			strcpy(Filename,m_filename);
			
		int orglength=strlen(Filename); 
		strncpy(filename_tmp,Filename,orglength-5); //ȥ������5λ�ַ�
        //cout << "filename_tmp = " <<filename_tmp<<endl;
		//int dr_GetAuditMode()1��ʾͬ����2��ʾ����, ����Ϊʧ�ܣ�-1�����ô���-2�����ļ���ȡ��������
		int iStatus = dr_GetAuditMode(module_name);
		if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ�� 
		{
			bool flag=false;
			int times=1;
			while(times<31)
			{
				if(access(m_filename,F_OK|R_OK))
				{
					theJSLog<<"������"<<times<<"���ļ�"<<endd;
					times++;
					sleep(10);
				}
				else
				{
					flag=true;
					break;
				}
			}
			if(!flag)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"��ϵͳ���������ļ�[%s]������",m_filename);
				theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);

				dr_AbortIDX();
				return 0;
			}
		}
		else if(iStatus==2) //����ģʽ����ϵͳ
		{
			while(1)
			{
			//�����ж�
				if(gbExitSig)
				{
					dr_AbortIDX();
							
					theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
					prcExit();
					return ;
				}

				if(access(m_filename,F_OK|R_OK))
				{
					sleep(10);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			char tmp[50]={0};
			snprintf(tmp,sizeof(tmp),"����ƽ̨dr_GetAuditMode�������ô��󣬷���ֵ[%d]",iStatus);
			theJSLog<<tmp<<endi;
			return 0;
		}
		
		int size1,size2;
		struct stat info;
		stat(m_filename,&info);
		size1=info.st_size;

		size2=atoi(data[3].c_str());
		/*if(size1!=size2)
		{
			theJSLog<<"ͬ��ʧ��....,�����ļ��е��ļ���С��ͬ"<<endi;
			dr_AbortIDX();
			return 0;
		}*/

		ifstream in(m_filename,ios::in);
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"getFilenames �ļ�%s�򿪳���",m_filename);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			return false;
		}
		memset(szBuff,0,sizeof(szBuff));
		szrollfiles.clear();
		SRollFile szrollfile;
		strcpy(szrollfile.filename,filename_tmp);
		szrollfiles.push_back(szrollfile);
		//memset(szBuff,0,sizeof(szBuff));
			
		/*while(in.getline(szBuff,sizeof(szBuff)))   
		{
			//cout<<"��ȡ���ļ���Ϊ"<<szBuff<<endl;
			SRollFile szrollfile;
			strcpy(szrollfile.filename,szBuff);
			szrollfiles.push_back(szrollfile);
			memset(szBuff,0,sizeof(szBuff));
		}*/

		char m_Filename[256];
		memset(m_Filename,0,sizeof(m_Filename));
		strcpy(m_Filename,data[1].c_str());//��ϵͳ���͹������ļ��б���

		//�����ļ�����ѯ��Ӧ����ԴID��fileid�Ͷ�Ӧ�������,�ӽ�����н�fileid��Ӧ�Ľ��ɾ����       
				
		//int j=atoi(data[2].c_str())-0; //��ϵͳ���͹������ļ��б������

		/*if(strcmp(szrollfiles[j].filename,m_Filename))
		{
			theJSLog<<"ͬ��ʧ��....,�����ļ��е��ļ��б�����ͬ"<<endi;
			dr_AbortIDX();
			return 0;
		}*/

		for(i=0;i<szrollfiles.size();i++)
		{
			/*memset(m_Filename,0,sizeof(m_Filename));
			char* p = strrchr(m_filename,'/');
			if(p)
				strcpy(m_Filename,p+1);
			else
				strcpy(m_Filename,m_filename);*/

			//theJSLog<<"�����ļ� "<<m_Filename<<"�еĵ�["<<i+1<<"]���ļ�"<<endi;
			theJSLog<<"�����ļ� "<<filename_tmp<<endi;
			
			//�����ļ�����ѯ��Ӧ����ԴID��fileid�Ͷ�Ӧ�������,�ӽ�����н�fileid��Ӧ�Ľ��ɾ����
			flag=rollFile(i);
			if(!flag)
				break;
			theJSLog<<"�������ļ� "<<filename_tmp<<endi;
		}	

		//theJSLog <<"׼�������ļ�"<<endi;
		ret=moveFiles(flag);	
		if(ret)
		{
			theJSLog<<"�����ļ��ɹ�"<<endi;
			//continue;
		 	return true;
		}
		else
		{
			theJSLog<<"�����ļ�ʧ��"<<endi;
			//continue;
			return false;
		}

	}
	else      //��ϵͳ,������ϵͳ    
	{
		try
		{	
			if(scan.openDir(input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"���ļ�Ŀ¼[%s]ʧ��",input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
				return false;	
			}	
	//		theJSLog <<"ɨ�赽�ļ�·��"<<input_path<<endi;
			int rett = 0;
			while(1)		
			{
				memset(m_filename,0,sizeof(m_filename));
				rett = scan.getFile("*.UNDO",m_filename); 
				if(rett == 100)
				{		
						break ;
				}
				if(rett == -1)
				{	
						break ;			//��ʾ��ȡ�ļ���Ϣʧ��
				}
				theJSLog<<"ɨ�赽�ļ���"<<m_filename<<endi;    //�ļ��б����������һЩ�ļ��� 
				//ret=getFilenames();				

				ifstream in(m_filename,ios::in);
				if(!in)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"getFilenames �ļ�%s�򿪳���",m_filename);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
					return false;
				}

				//��ȡ�ļ���С
				struct stat info;
				stat(m_filename,&info);
				int size=info.st_size;

				memset(szBuff,0,sizeof(szBuff));
				szrollfiles.clear();
				char m_Filename[256];
				char realfilename[256];
				char* p = strrchr(m_filename,'/');
				memset(m_Filename,0,sizeof(m_Filename));  //m_Filename ���������㲻��·�����ļ���
					if(p)
						strcpy(m_Filename,p+1);
					else
						strcpy(m_Filename,m_filename);
				int orglength=strlen(m_Filename);
				strncpy(realfilename,m_Filename,orglength-5);
				//cout << "m_Filename = " << m_Filename << "realfilename = " << realfilename <<endl;
				/*while(in.getline(szBuff,sizeof(szBuff)))   
				{
					//cout<<"��ȡ���ļ���Ϊ"<<szBuff<<endl;
					SRollFile szrollfile;
					strcpy(szrollfile.filename,szBuff);
					szrollfiles.push_back(szrollfile);
					memset(szBuff,0,sizeof(szBuff));
				} */
			   SRollFile szrollfile;
			   strcpy(szrollfile.filename,realfilename);
				szrollfiles.push_back(szrollfile);
							
				//����ÿ���ļ��е������ļ��б�
				for(i=0;i<szrollfiles.size();i++)
				 {			
					//memset(m_Filename,0,sizeof(m_Filename));  //m_Filename ���������㲻��·�����ļ���
					/*char* p = strrchr(m_filename,'/');
					if(p)
						strcpy(m_Filename,p+1);
					else
						strcpy(m_Filename,m_filename);*/

					//theJSLog<<"�����ļ� "<<m_Filename<<"�еĵ�["<<i+1<<"]���ļ�"<<endi;

					memset(m_SerialString,0,sizeof(m_SerialString));
					//sprintf(m_SerialString,"%s;%s;%d;%d",m_Filename,szrollfiles[i].filename,i,size);
					sprintf(m_SerialString,"%s;%d",m_Filename,size);  //����ԭʼ�ļ�������UNDO
                    //cout<<"m_SerialString"<<m_SerialString<<endl;
					ret = drVarGetSet(m_SerialString);
					if(ret)
					{
						theJSLog<<"ͬ��ʧ��...."<<endi;
						break;
					} else {
				    //�����ļ�����ѯ��Ӧ����ԴID��fileid�Ͷ�Ӧ�������,�ӽ�����н�fileid��Ӧ�Ľ��ɾ����
					  flag=rollFile(i);
					  if(!flag)
					  {
						theJSLog<<"�ļ�"<<szrollfiles[i].filename<<"����ʧ��"<<endi;
					   	break;
					  }
					}
					//theJSLog<<"�������ļ� "<<m_Filename<<"�еĵ�["<<i+1<<"]���ļ�"<<endi;					
				}

				//2013-10-18 add
				/*memset(m_Filename,0,sizeof(m_Filename));  //m_Filename ���������㲻��·�����ļ���
				char* p = strrchr(m_filename,'/');
				if(p)
				strcpy(m_Filename,p+1);
					else
				strcpy(m_Filename,m_filename);
					
				char realfilename[256];
				int orglength=strlen(m_Filename);
				strncpy(realfilename,m_Filename,orglength-5);
				//cout<<"m_Filename = "<< m_Filename<< "   realfilename = " <<realfilename<<endl;
				sprintf(m_SerialString,"%s;%d",m_Filename,size);
				ret = drVarGetSet(m_SerialString);				
				if(ret)
				{
					theJSLog<<"ͬ��ʧ��...."<<endi;
					break;
				}*/	
				
				in.close();

				//theJSLog <<"׼�������ļ�"<<endi;
				 ret=moveFiles(flag);	
				 if(ret)
				 {
					 theJSLog<<"�����ļ�"<<m_Filename<<"�ɹ�"<<endi;
				 }
				 else
				 {
					 theJSLog<<"�����ļ�"<<m_Filename<<"ʧ��"<<endi;
				 }
				 break;
			}
			scan.closeDir();
		}catch (jsexcp::CException &e) 
		{	
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"openDir %s",e.GetErrMessage());
			theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);	
 		}
	  }
	
   	}
}
	
//����szrollfiles���ļ��������ļ��������ñ��в�ѯ�ļ���Ӧ������ԴID���Ӹ�ʽ���ļ��ǼǱ�D_SCH_FORMAT��
//��ѯfileid����������ԴID��ѯ�������ñ��ȡ��Ӧ�ջ��ܽ�����������ӽ������ɾ��fileid��Ӧ�ļ�¼
bool CFileRoll::rollFile(int i) 
{
	char state='F';   //�ļ���sqlִ�гɹ���־
//	bool ret=true;  //Ĭ���ٲóɹ�
	Statement stmt;
	if(!(rtinfo.connect()))		//�����ڴ���
	{
		return false;
	}
	short petri_status ;	
	cout<<"petri status:"<< petri_status <<endl;
	while(1)
	{
	    rtinfo.getDBSysMode(petri_status);		//��ȡ״̬
		if(petri_status==DB_STATUS_ONLINE)
		{
		//	theJSLog<<"���ݿ�Ϊ����̬"<<endi;
			break;
		}
		else  if(petri_status==DB_STATUS_OFFLINE)
		{
			theJSLog<<"���ݿ�Ϊֻ��,�ȴ�..."<<endi;
			sleep(5);
		}
		else
		{
			theJSLog<<"���ݿ�״̬����"<<endi;
			return false;
		}
	}
	try{			
		if (dbConnect(conn))
		{
			//��ѯ�ļ���������ԴID
			int flag=0;
			for(map<string,SOURCECFG>::const_iterator it = m_SourceCfg.begin();it != m_SourceCfg.end();++it)
			{
				if(checkFormat(szrollfiles[i].filename,it->second.filterRule))		//HDC.2013---    HD*
				{		
					theJSLog<<"�ļ�["<<szrollfiles[i].filename<<"]��������Դ:"<<it->first<<endi;

					flag = 1;
					strcpy(szrollfiles[i].sourceID ,(it->first).c_str());
				}				
			}
	
			if(flag == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�Ҳ����ļ�%s��������Դ",szrollfiles[i].filename);
				theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
				return 0;
			}

			setSQLFileName(szrollfiles[i].filename) ;

			theJSLog<<"�ҵ��ļ���������ԴID Ϊ"<<szrollfiles[i].sourceID<<endi;
			//���ļ�����ѯ��Ӧ����ԴID
		/*	string sql = "select SOURCE_ID from C_FILE_RECEIVE_ENV where FILE_FILTER = :v1";
			stmt1.setSQLString(sql);
			stmt1 <<  szrollfiles->filename;	//�������		
			stmt1.execute();//ִ��sql��
			stmt1>> szrollfiles->sourceID;	//�������		
			stmt1.close();*/

			//ͨ������ԴIDȥ��Ӧ�ջ������ñ��в�����Ҫɾ���Ľ������
			stmt = conn.createStatement();
			string sql="select a.TABLE_NAME from C_STAT_TABLE_DEFINE a,C_SUMTABLE_DEFINE b where a.CONFIG_ID=b.ORGSUMT and b.SOURCEID =:v2";
			stmt.setSQLString(sql);
			stmt <<szrollfiles[i].sourceID;	//�������		
			stmt.execute();//ִ��sql���
			stmt>> szrollfiles[i].tablename;	//�������		
			theJSLog<<"���ҽ����: "<<szrollfiles[i].tablename<<endi;
			
			//�Ӹ�ʽ���ļ��ǼǱ�D_SCH_FORMAT�в�ѯfileid
			sql="select FILE_ID from D_SCH_FORMAT where FILENAME=:v1 and DEAL_TIME =(select max(DEAL_TIME) from D_SCH_FORMAT where FILENAME=:v2)";
			stmt.setSQLString(sql);
			stmt << szrollfiles[i].filename<< szrollfiles[i].filename;	//�������		
			stmt.execute();//ִ��sql��
			int num = stmt.getCompleteRows();
            if(num ==0)
            	{
            	   theJSLog << "���ļ��ڸ�ʽ�����޼�¼" << endw;
            	   return false;
                }
            else
            	{
            	  stmt>> szrollfiles[i].fileID;	//�������
            	  char sql2[1024];
			sprintf(sql2,"delete from %s where FILEID=%d",szrollfiles[i].tablename,szrollfiles[i].fileID);
			theJSLog<<sql2<<endi;
			stmt.setSQLString(sql2);		
			stmt.execute();//ִ��sql��
			theJSLog<<"ɾ��fileid ��Ӧ�Ľ�����е����ݳɹ�"<<endi;	

		    //ִ���ٲ�
			memset(m_AuditMsg,0,sizeof(m_AuditMsg));
			sprintf(m_AuditMsg,"%s|%c",szrollfiles[i].filename,state);
			theJSLog<<"��ʼ�ٲ�"<<endi;
			if(!IsAuditSuccess(m_AuditMsg))   //�ٲ�ʧ��
			{
				theJSLog<<"�ٲ�ʧ��"<<endi;
				stmt.rollback();
				stmt.close();
				conn.close();
				return false;
			}
			stmt.close();	
			conn.close();
			theJSLog<<"�ٲóɹ�,���浽D_SQL_FILEREG����"<<endi;
			//ÿ����һ���ļ������浽D_SQL_FILEREG����
			saveLog(state,i); 
			return true;
            	  
            	}				
		
			
		}else{
	 		memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  false;
		}
	 } catch( SQLException e ) {
		state='E';
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�ӽ�����н�fileid��Ӧ�Ľ��ɾ��ʧ��:%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		//ִ���ٲ�
			memset(m_AuditMsg,0,sizeof(m_AuditMsg));
			sprintf(m_AuditMsg,"%s|%c",szrollfiles[i].filename,state);
			theJSLog<<"��ʼ�ٲ�"<<endi;
			if(!IsAuditSuccess(m_AuditMsg))   //�ٲ�ʧ��
			{
				theJSLog<<"�ٲ�ʧ��"<<endi;
			//	ret=false;
				stmt.rollback();
				stmt.close();
				conn.close();
				return false;
			}
			stmt.close();	
			conn.close();
			theJSLog<<"�ٲóɹ�,���浽D_SQL_FILEREG����"<<endi;
			//ÿ����һ���ļ������浽D_SQL_FILEREG����
			saveLog(state,i); 
			
		//	rollBackSQL();
			return false;
     } 	
}

void CFileRoll::saveLog(char state,int i)  //ÿ����һ���ļ������浽D_ROLL_FILEREG����
{
	char sql[1024];
	try{			
		if (dbConnect(conn))
		 {
			Statement stmt = conn.createStatement();
			char flag;
			if(state=='F')
				flag='Y';
			if(state=='E')
				flag='N';
	//		theJSLog<<"szrollfiles.filename="<<szrollfiles[i].filename<<"  szrollfiles.sourceID= "<<szrollfiles[i].sourceID <<"    flag="<<flag<<endi;
			
			/*���ļ�����·��ȥ��*/
			char m_Filename[256];
			memset(m_Filename,0,sizeof(m_Filename));
			char* p = strrchr(m_filename,'/');
			if(p)
				strcpy(m_Filename,p+1);
			else
				strcpy(m_Filename,m_filename);
			sprintf(sql,"insert into D_ROLL_FILEREG(ROLLFILE_NAME,FILE_NAME,DEAL_DATE,SOURCE_ID,DEAL_FLAG) values('%s','%s',sysdate,'%s','%c')",m_Filename,szrollfiles[i].filename,szrollfiles[i].sourceID,flag);
		//	writeSQL(sql);
			theJSLog<<sql<<endd;
			stmt.setSQLString(sql);
			stmt.execute();//ִ��sql���
			stmt.close();
			theJSLog<<"D_ROLL_FILEREG��������ݳɹ�"<<endi;
		 }else{
	 		cout<<"connect error."<<endl;
	 		return ;
		 }
	 	conn.close();
		conn.commit();
	}catch( SQLException e ) {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�����ļ���sql��� ʧ��,ȷ�� D_ROLL_FILEREG �����ɹ�:%s,%s",sql,e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		conn.close();
    } 			
}

bool CFileRoll::moveFiles(bool flag)//���Ѿ��������ļ��ƶ���ָ������Ŀ¼	
{
			
			theJSLog <<"�����ļ�ȫ·����Ϊ:"<<m_filename<<endi;
			
			/*���ļ�����·��ȥ��*/
			char m_Filename[256],path[256];;
			memset(m_Filename,0,sizeof(m_Filename));
			char* p = strrchr(m_filename,'/');
			if(p)
				strcpy(m_Filename,p+1);
			else
				strcpy(m_Filename,m_filename);
			if (flag)
			{
				theJSLog<<"�ƶ��ļ� "<<m_filename<<" ������Ŀ¼ "<<output_path<<endi;
				memset(path,0,sizeof(path));
				strcpy(path,output_path);
				strcat(path,m_Filename);
				if(rename(m_filename,path))	
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",m_filename,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					return false;
				}	
			}
			else
			{
				theJSLog<<"�ƶ��ļ� "<<m_filename<<" ������Ŀ¼ "<<erro_path<<endi;
				memset(path,0,sizeof(path));
				strcpy(path,erro_path);
				strcat(path,m_Filename);
				if(rename(m_filename,path))	
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",m_filename,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					return false;
				}
			}					
			return true;
}

//��������Դ������Ϣ��ȡȫ������Դ�������Դ��Ϣ
int CFileRoll::LoadSourceCfg()
{
	char m_szSrcGrpID[8];
	int iSourceCount=0;
	string sql ;
	try
	{		
		Statement stmt = conn.createStatement();
		Statement stmt2 = conn.createStatement();
		sql = "select source_group from c_source_group_define ";
		stmt2.setSQLString(sql);
		stmt2.execute();
		memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));

		while(stmt2>>m_szSrcGrpID)
		{			
			sql = "select count(*) from C_SOURCE_GROUP_CONFIG  where SOURCE_GROUP =:1";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID;
			stmt.execute();
			if(!(stmt>>iSourceCount))
			{
				continue ;
			}

			theJSLog<<"����Դ�飺"<<m_szSrcGrpID<<"  iSourceCount="<<iSourceCount<<endi;
		
			sql = "select a.source_id,b.source_path from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID;
			if(stmt.execute())
			{
				for (int i=0; i<iSourceCount; i++)
				{
					SOURCECFG SourceCfg;
					string strSourceId;

					stmt>>SourceCfg.szSourceId>>SourceCfg.szSourcePath;      
					strSourceId=SourceCfg.szSourceId;			    
					completeDir(SourceCfg.szSourcePath);

					if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
					{
							return -1;
					}
		
					m_SourceCfg[strSourceId]=SourceCfg;
				}
			}
			
			memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
		}
		
		stmt2.close();
		stmt.close();

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	 }

	return 0;
}

/******��������Դ��ȡ���˹��� 0û�в鵽����1�鵽������ ���ӻ�ȡ�ļ�����ʱ�����ʼλ��,�ͳ���*********************/
int CFileRoll::getSourceFilter(char* source,char* filter,int &index,int &length)
{	
	try
	{	
		string file_time;
		char tmp[5];
		memset(tmp,0,sizeof(tmp));

		Statement stmt = conn.createStatement();
		string sql = "select file_filter,file_time_index_len from C_FILE_RECEIVE_ENV where source_id = :1 ";		
		stmt.setSQLString(sql);
		stmt << source;
		stmt.execute();
		if(!(stmt>>filter>>file_time))
		{
				stmt.close();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"����Դ[%s]û�����ù��˹�������ļ���ʱ���ȡ����",source);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1;
		}
		
		//cout<<"file_time = "<<file_time<<endl;
		strcpy(tmp,file_time.c_str());

		vector<string> fileTime;		
		splitString(tmp,",",fileTime,false);
		if(fileTime.size() != 2)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]�ļ���ʱ���ȡ�������ù������:%s  [��3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}
		
		index = atoi(fileTime[0].c_str());
		length = atoi(fileTime[1].c_str());
		
		if(index < 1 || length == 0)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]�ļ���ʱ���ȡ�������ù������:%s  [��3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}

		index--;

		stmt.close();

	}
	catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter ���ݿ��ѯ�쳣: %s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter �ֶ�ת������%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}

bool CFileRoll::checkFormat(const char *cmpString,const char *format)
{
	while(1)
	{
		switch(*format)
	  	{
	  		case '\0':
					if (*cmpString == '\0')
					{
						return true;
					}
					else
					{
						return false;
					}
			case '!':
					if (checkFormat(cmpString,format + 1) == true)
					{
						return false;
					}
					else
					{
						return true;
					}
			case '?' :
					if(*cmpString == '\0')
					{
						return false;
					}
					return checkFormat(cmpString + 1,format + 1);
			case '*' :
					if(*(format+1) == '\0')
					{
						return true;
					}
					do
					{
						if(checkFormat(cmpString,format+1)==true)
						{
							return true;
						}
					}while(*(cmpString++));
					return false;
			case '[' :
					format++;
					do
					{
						
						if(*format == *cmpString)
						{
							while(*format != '\0' && *format != ']')
							{
								format++;
							}
							if(*format == ']')
							{
								format++;
							}
							return checkFormat(cmpString+1,format);			
						}
						format++;
						if((*format == ':') && (*(format+1) != ']'))
						{
							if((*cmpString >= *(format - 1)) && (*cmpString <= *(format + 1)))
							{
								while(*format != '\0' && *format != ']')
								{
									format++;
								}
								if(*format == ']')
								{
									format++;
								}
								return checkFormat(cmpString+1,format);
							}
							format++;
							format++;

						}
					}while(*format != '\0' && *format != ']');

					return false;
			default  :
					if(*cmpString == *format)
					{
						return checkFormat(cmpString+1,format+1);
					}
					else
					{
						return false;
					}
		}//switch
	}//while(1)
}

//2013-11-02 �����˳�����
void CFileRoll::prcExit()
{
	int ret = 0;
	if(m_enable) 
	{
		ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog<<tmp<<endw;
		}
		
		m_enable = false;
	}
	
	PS_Process::prcExit();
}

//���ֳ�ʼ��
bool CFileRoll::drInit()
{
		//����ģ������ʵ��ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		theJSLog << "��ʼ������ƽ̨,ģ����:"<< module_name<<" ʵ����:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_name,tmp);
		if(ret != 0)
		{
			theJSLog << "����ƽ̨��ʼ��ʧ��,����ֵ=" << ret<<endi;
			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		m_enable = true;

		drStatus = _dr_GetSystemState();	//��ȡ����ϵͳ״̬
		if(drStatus < 0)
		{
			theJSLog<<"��ȡ����ƽ̨״̬����: ����ֵ="<<drStatus<<endi;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 1)	theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 2)	theJSLog<<"��ǰϵͳ���÷�����ϵͳ"<<endi;

		return true;
}

//��ϵͳ����ͬ������,��ϵͳ��ȡͬ������
int CFileRoll::drVarGetSet(char* serialString)
{		
		int ret ;
		char tmpVar[5000] = {0};

		//�������ƽ̨���л���
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"��������л���ʧ��,����ֵ="<<ret<<endi;
			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"��ʼ��indexʧ��,����ֵ=" <<ret<<endi;
			dr_AbortIDX();
			return -1;  
		}
		
		//��ϵͳ�����ļ�����·�����ļ��� ֻ������ƽ̨���Ը�֪,��ϵͳ�޷�ʶ��
	/*	if(drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s",input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�����·��ʧ��,�ļ�·��["<<input_path<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}
			vector<string> data;		
			splitString(serialString,";",data,false,false);			
			char m_Filename[256];
			memset(m_Filename,0,sizeof(m_Filename));
			strcpy(m_Filename,data[0].c_str());
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_Filename);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�ʧ��,�ļ���["<<tmpVar<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}

			theJSLog<<"�����ļ�·��:"<<input_path<<" �ļ���:"<<tmpVar<<endl;
		}*/


		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"�����д�ʱʧ��,������:["<<serialString<<"]"<<endi;
			dr_AbortIDX();
			return -1;
		}
		//serialString = tmpVar;			//ͬ�������ַ���,��ϵͳ�Ǹ�ֵ����ϵͳ��ȡֵ
		strcpy(serialString,tmpVar);
		//m_AuditMsg = tmpVar;			//Ҫ�ٲõ��ַ���
		
		// <5> ����ʵ����  ������ϵͳע���IDX��ģ����ò�����
		//��ϵͳ��index manager���IDX��������󣬰�ʹ�øú���ע������������Ϊģ��ĵ��ò���trigger��Ӧ�Ľ���
		snprintf(tmpVar, sizeof(tmpVar), "%d", getPrcID());
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			theJSLog<<"����ʵ����ʧ�ܣ�"<<tmpVar<<endi;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"Ԥ�ύindexʧ��"<<endi;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"�ύindexʧ��,����ֵ="<<ret<<endi;
			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;
        theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test
		return ret;

}

//�ٲ��ַ���
 bool CFileRoll::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
//cout<<"��ʼ�ٲ�"<<endl;
		ret = dr_Audit(dealresult);
//cout<<"�Ѿ��ٲ�"<<endl;
		if(2 == ret )
		{
			theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endi;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endi;
			dr_AbortIDX();
			return false;
		}
		else if(4 == ret)
		{
			theJSLog<<"�Զ�idx�쳣��ֹ..."<<endi;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "ҵ��ȫ���ύʧ��(����ƽ̨)" << endi;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endi;
			dr_AbortIDX();
			return false;
		}
	
	return true;
 }

bool CFileRoll::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�����ɾ��"<< m_triggerFile <<endi;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"ɾ��trigger�ļ�[%s]ʧ��: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
		return false;
	}

	return true;
}

int main(int argc,char**argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network          "<<endl;
	cout<<"*    InterNational Account Settle System       "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*           jsrollfile						  "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	              "<<endl;
	cout<<"*     last update time : 2013-08-28 by  cwf	  "<<endl;
	cout<<"********************************************** "<<endl;


	CFileRoll fm ;
   	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
	while(1)
	{
		theJSLog.reSetLog();
		fm.getFilenames();
		sleep(30);
	}

   return 0;
}
