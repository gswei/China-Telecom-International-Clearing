#include "CDaylistPlugin.h"

//CDatabase DBConn;
CLog theJSLog;

int main(int argc, char **argv)
{

	/* �����������Ƿ���ȷ */
	if (!( argc == 5))
		{
		/* ���������ƽ̨��� ����Դ���� ����index ���������ļ� */
		printf("Usage : %s <service_id> <source_group> <process_index> <env_path>\n",argv[0]);
		exit(-1);
		}

	char sourceID[10];
	strcpy(sourceID,"GZb01");
	char InFilePath[1000];
	strcpy(InFilePath,"/home/zhjs/source_data/service/200X/GZb01/stat_work_path");
	char filename[200];
	strcpy(filename,"GZb01.00000.20100606.1228.dat.hw201006061183.txt.100");
	char InFile[1000];
	sprintf(InFile,"%s/%s",InFilePath,filename);
	
	
	  /* ����������ж�ȡ���� */
	char szServiceId[20];
	char szSourceGroupId[20];
	int iProcessId;
	char szEnvPath[250];
	char szEnvFile[250];
	char szOutputFiletypeId[20];
	char szOutrcdType[2];        
	
	strcpy(szServiceId, argv[1]);
	strcpy(szSourceGroupId, argv[2]);
	iProcessId = atoi(argv[3]);
	strcpy(szEnvPath, argv[4]);
	if(szEnvPath[strlen(szEnvPath)-1] != '/')
	 	strcat(szEnvPath, "/");
	sprintf(szEnvFile, "%szhjs.ini", szEnvPath);
	

	int ret=connectDB(szEnvFile, DBConn);
	if(ret==0)
		theLog<<"���ӳɹ�"<<endi;
	else
		theLog<<"����ʧ��"<<endi;

	CBindSQL ds(DBConn);	
		
  	CReadIni IniFile;
	char szLogPath[101];
  	char szLogLevel[10];
	char szLogStr[500];
	  /* �ӻ��������ļ��ж�ȡ���� */
	  if(!IniFile.init(szEnvFile))
	  {
		  cout<<"open ini err: "<<szEnvFile<<endl;
		  return 0;
	  }	
	   /* ��ʼ����־ */
	  IniFile.GetValue("COMMON", "log_path", szLogPath, 'N');
	  IniFile.GetValue("COMMON", "log_level", szLogLevel, 'N');
	  theLog.setLog(szLogPath, atoi(szLogLevel), szServiceId, szSourceGroupId, iProcessId);
	  
	  CDaylistPlugin daylist_t;
	  daylist_t.init(szSourceGroupId, szServiceId,iProcessId);

	 //�ļ�����
	 char szSqlStr[500];
	  sprintf(szSqlStr, "select  filetype_id from c_source_group_define where source_group='%s'", szSourceGroupId);
	  ds.Open(szSqlStr, SELECT_QUERY );
	  if (!(ds>>szOutputFiletypeId))
	  	{
	  	  strcpy(szLogStr, "select  filetype_id from c_source_group_define where source_group= :szSourceGroupId is NULL");
		  theLog<<szLogStr<<endi;
		  exit;
	  	}

	  //��¼����
	  ds.Open("select record_type from c_filetype_define where filetype_id=:v1", SELECT_QUERY );
	  ds<<szOutputFiletypeId;
	  if (!(ds>>szOutrcdType))
	  {
		  strcpy(szLogStr, "select record_type from c_filetype_define where filetype_id= :filetype_id is NULL");
		  theLog<<szLogStr<<endi;
		  exit;
	  }
	  
	  MessageParser msgParser;
	  //int setMessage(int MessageType, char* SourceId, char* Filename);
	  PacketParser packParser;
	  //int setRcd(CFmt_Change &inRcd);
	  ResParser reParser;
	  
	  theLog<<"szOutputFiletypeId="<<szOutputFiletypeId<<",szOutrcdType="<<szOutrcdType<<endi;
	  CF_MemFileI _infile;
	  CFmt_Change _inrcd;
	  _infile.Init(szOutputFiletypeId);
	  _inrcd.Init(szOutputFiletypeId, szOutrcdType[0]);
	  _infile.Open(InFile);
	  
	  msgParser.setMessage(MESSAGE_NEW_BATCH,sourceID,filename);
	  daylist_t.message(msgParser);
	  msgParser.setMessage(MESSAGE_NEW_FILE,sourceID,filename);
	  daylist_t.message(msgParser);
         while(1)
		{
		if (_infile.readRec(_inrcd) == READ_AT_END)
			break;
		packParser.setRcd(_inrcd);
		daylist_t.execute(packParser, reParser);
		}
	_infile.Close();
	
	msgParser.setMessage(MESSAGE_END_FILE,sourceID,filename);
	daylist_t.message(msgParser);
	msgParser.setMessage(MESSAGE_END_BATCH_END_FILES,sourceID,filename);
	daylist_t.message(msgParser);
	msgParser.setMessage(MESSAGE_END_BATCH_END_DATA,sourceID,filename);
	daylist_t.message(msgParser);
	DBConn.Commit();
	
	theLog<<"sleep 20...,check ok!!"<<endi;
	msgParser.setMessage(MESSAGE_NEW_DAY,sourceID,filename);
	daylist_t.message(msgParser);
	DBConn.Commit();
	sleep(20);
	
	theLog<<"sleep 10...,check new end!!"<<endi;
	msgParser.setMessage(MESSAGE_PROGRAM_QUIT,sourceID,filename);
	daylist_t.message(msgParser);
	DBConn.Commit();
	sleep(10);
	
	theLog<<"�����������"<<endi;
	
}

