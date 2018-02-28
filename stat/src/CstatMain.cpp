#include "CStatPlugin.h"

CDatabase DBConn;
CLog theLog;

struct SFile
{
	char sourceID[10];
	char InFilePath[1000];
	char filename[200];
	char InFile[1000];
};

int main(int argc, char **argv)
{
	/* 检查输入参数是否正确 */
	if (!( argc == 5))
		{
		/* 输入参数：平台编号 数据源组编号 进程index 环境变量文件 */
		//test1 STAT YFFX1 1 ../../env
		printf("Usage : %s <service_id> <source_group> <process_index> <env_path>\n",argv[0]);
		exit(-1);
		}
	

	
	vector<SFile> v_file;
	SFile tmp;
	strcpy(tmp.sourceID,"ZS812");
	strcpy(tmp.InFilePath,"/home/zhjs/src/statplugin/data/");
	strcpy(tmp.filename,"ZS811.00000.20100803.0370.dat.gy201008030359.760.txt");
	sprintf(tmp.InFile,"%s/%s",tmp.InFilePath,tmp.filename);
	v_file.push_back(tmp);
	strcpy(tmp.sourceID,"ZS812");
	strcpy(tmp.InFilePath,"/home/zhjs/src/statplugin/data/");
	strcpy(tmp.filename,"test.txt");
	sprintf(tmp.InFile,"%s/%s",tmp.InFilePath,tmp.filename);
	v_file.push_back(tmp);
	
	  /* 从输入参数中读取变量 */
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
	
 
	
	cout<<"this is test from statPlugin..."<<endl;
	int ret=connectDB(szEnvFile, DBConn);
	if(ret==0)
		theLog<<"连接成功"<<endi;
	else
		theLog<<"连接失败"<<endi;

	CBindSQL ds(DBConn);	

	
	try{
		/*测试merge into是否可用*/
		char mergeSql[200];
		strcpy(mergeSql,"MERGE INTO products p "
			 "USING newproducts np "
			 "ON (p.product_id = np.product_id) "
			 "WHEN MATCHED THEN "
			 "UPDATE "
			 "SET p.product_name = np.product_name,  "
			  "p.category = np.category "
			  "when not matched then "
			  "insert  "
			 "values(np.product_id, np.product_name, np.category)");
		 cout<<mergeSql<<endl;
		  ds.Open(mergeSql,NONSELECT_DML);
		  ds.Execute();
		}
 	catch (CException e)
		  {
		 errLog(LEVEL_ERROR,"",e.GetAppError(),"",__FILE__,__LINE__,e);
		 exit(1);
 		 }
	exit(0);
		
  	CReadIni IniFile;
	char szLogPath[101];
  	char szLogLevel[10];
	char szLogStr[500];
	  /* 从环境变量文件中读取参数 */
	  if(!IniFile.init(szEnvFile))
	  {
		  cout<<"open ini err: "<<szEnvFile<<endl;
		  return 0;
	  }	
	   /* 初始化日志 */
	  IniFile.GetValue("COMMON", "log_path", szLogPath, 'N');
	  IniFile.GetValue("COMMON", "log_level", szLogLevel, 'N');
	  theLog.setLog(szLogPath, atoi(szLogLevel), szServiceId, szSourceGroupId, iProcessId);
	  
	  CStatPlugin stat_t;
	  stat_t.init(szSourceGroupId, szServiceId,iProcessId);

	 //文件类型
	 char szSqlStr[500];
	  sprintf(szSqlStr, "select  filetype_id from c_source_group_define where source_group='%s'", szSourceGroupId);
	  ds.Open(szSqlStr, SELECT_QUERY );
	  if (!(ds>>szOutputFiletypeId))
	  	{
	  	  strcpy(szLogStr, "select  filetype_id from c_source_group_define where source_group= :szSourceGroupId is NULL");
		  theLog<<szLogStr<<endi;
		  exit;
	  	}

	  //记录类型
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
		  theLog<<"_infile.Init ok"<<endi;
		  _inrcd.Init(szOutputFiletypeId, szOutrcdType[0]);
		    theLog<<"_inrcd.Init ok"<<endi;
		 
		char sourceID[10];
		char InFilePath[1000];
		char filename[200];
		char InFile[1000];
		try{  
			msgParser.setMessage(MESSAGE_NEW_BATCH,sourceID,filename,0);
			stat_t.message(msgParser);
			for(int i=0;i<2;i++)
				{
				strcpy(sourceID,v_file[i].sourceID);
				strcpy(InFilePath,v_file[i].InFilePath);
				strcpy(filename,v_file[i].filename);
				strcpy(InFile,v_file[i].InFile);
				
				_infile.Open(InFile);
				theLog<<"_infile.Open(InFile); ok"<<endi;
				
				msgParser.setMessage(MESSAGE_NEW_FILE,sourceID,filename,0);
				stat_t.message(msgParser);
				while(1)
					{
					if (_infile.readRec(_inrcd) == READ_AT_END)
					break;
					packParser.setRcd(_inrcd);
					stat_t.execute(packParser, reParser);
					}
				_infile.Close();

				msgParser.setMessage(MESSAGE_END_FILE,sourceID,filename,0);
				stat_t.message(msgParser);
				}
			msgParser.setMessage(MESSAGE_END_BATCH_END_FILES,sourceID,filename,0);
			stat_t.message(msgParser);
			msgParser.setMessage(MESSAGE_END_BATCH_END_DATA,sourceID,filename,0);
			stat_t.message(msgParser);
			}
		catch(CException e)
			{	
			msgParser.setMessage(MESSAGE_BREAK_BATCH,sourceID,filename,0);
			stat_t.message(msgParser);
			
			char errmsg[30];
			strcpy(errmsg,"执行失败!!");

			errLog(LEVEL_ERROR,"",e.GetAppError(),errmsg,__FILE__,__LINE__,e);
			exit(1);
			}
		DBConn.Commit();
		 theLog<<"程序运行完毕"<<endi;
		}
