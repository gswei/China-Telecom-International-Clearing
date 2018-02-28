#include "main.h"
using namespace std;
using namespace tpss;

CLog theJSLog;
bool bGetExitSig;
//MdrStatus syn_status;//����״̬
int pkgstatus; //������״ֵ̬
string auditkey;
string sessionid;
extern SParameter Param;
int isRedo; //�Ƿ�����̯��

SGW_RTInfo rtinfo;
short petri_status = -1; 
#define COMMIT_COUNT	1000	   // ���ݿ��ύ����������

C_MainFlow::C_MainFlow()
{
	//vecFile.clear();
	memManager = NULL;
	//chain = NULL;
	pps = NULL;
	res = NULL;
	pListSql = NULL;
	iRunTime = -1;
	conn.close();
	conn = NULL;
	//m_bDBLinkError = FALSE;
	//_DBConn = NULL;
}

C_MainFlow::~C_MainFlow()
{
	//vecFile.clear();
	//FreeFormula(Param.szDebugFlag,&Param.szformulastruct,Param.iFormulacount,Param.iParamcount);
}

int main(int argc, char** argv)
{
    C_MainFlow process;
	process.printVersion();

	if(!process.checkArg(argc, argv))
	{
		return -1;
	}

	if(!process.init(argc, argv))
	{
		goto Exit;
	}
	Exit:
		//theJSLog<<"��ȫ�˳�����"<<endi;
	
    BillRate billRate;

	for(int i=0;i<process.source_count;i++)
    {
       billRate.m_init(process.stparam[i].source_id,process.stparam[i],argv[3]);       
    }
	billRate.init(argv[3],argv[2]); // 20131119 add ���������ֶ� 
	strcpy(process.jobid,argv[3]);
	process.getSourceInfo();
	theJSLog <<"Init Success.."<<endi;

    /*isRedo = 0; //Ĭ�ϲ�����̯������
	if ( strcmp(argv[4] ,"-r")==0 )  // -f ��һ��̯�� -r ����̯��
	{
		isRedo = 1;
	} else 
		isRedo = 0; */

	//ȡ���������ļ��������ݿ�
	// ���������Ҫ����������е�����
	
    bool result_flag = process.dealAll();
	if (result_flag)
        theJSLog <<"JSRate Over Success.."<<endi;
	else
		theJSLog <<"JSRate Over Fail.."<<endi;

	//process.prcExit();
	//exit(-1);

	return 0;
}


void C_MainFlow::printVersion()
{
	/* ���ģ������������ơ��汾����Ϣ */
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    jsSettleRate               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2013-10-03 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;
};

bool C_MainFlow::checkArg(int argc, char** argv)
{	
	/* �����������Ƿ���ȷjsSettleRate   YYYYMM   jobid*/
	if (!(argc ==4))
	{
	    printf("Usage : %s -f0 YYYYMM JOBID \n",argv[0]);
		return false;
	}
	/*if(strlen(argv[2])!=6)
	{
	   printf("Please enter the right cycle like 201307!");
	   return false;
	}*/
 
	char szPathTmp[FILE_NAME_LEN+1];
	strcpy(szPathTmp, argv[0]);
	if(!strncmp(szPathTmp,"../",3))
	{
		getcwd(m_szExePath, PATH_NAME_LEN+1);
		sprintf(m_szExePath, "%s/%s", m_szExePath, szPathTmp);
		strncpy(m_szExeName, strrchr(szPathTmp,'/')+1, strlen(szPathTmp)-(strrchr(szPathTmp,'/')-szPathTmp)+1);
	}
	else if(!strncmp(szPathTmp,"./",2))
	{
		getcwd(m_szExePath, PATH_NAME_LEN+1);
		strncpy(m_szExeName, strrchr(szPathTmp,'/')+1, strlen(szPathTmp)-(strrchr(szPathTmp,'/')-szPathTmp)+1);
	}
	else if(!strncmp(szPathTmp,"/",1))
	{
		strncpy(m_szExeName, strrchr(szPathTmp,'/')+1, strlen(szPathTmp)-(strrchr(szPathTmp,'/')-szPathTmp)+1);
		strncpy(m_szExePath, szPathTmp, strlen(szPathTmp)-strlen(m_szExeName));
	}
	else
	{
		getcwd(m_szExePath, PATH_NAME_LEN+1);
		strcpy(m_szExeName, szPathTmp);
	}
	//theJSLog<<"szProgramPath="<<m_szExePath<<endi;
	//theJSLog<<"szProgramName="<<m_szExeName<<endi;
	return true;
};

bool C_MainFlow::init(int argc, char **argv)
{
    //��ܳ�ʼ��
   if(!PS_Process::init(argc,argv))
   {
      return false;
   }

   if(!(rtinfo.connect()))		//�����ڴ���
	{
		 return false;
	}
	rtinfo.getDBSysMode(petri_status);		//��ȡ״̬
	cout<<"petri_status:"<< petri_status <<endl;

	if(petri_status==304)
	{
		return false;
	} 

	 //DBConnection conn;//���ݿ�����
  try{			
	if (dbConnect(conn))
	 {
	       //��ȡ����Դ����
			Statement stmt = conn.createStatement();
	        string sql = "select distinct(source_group_id) from C_SETTLE_JOB where job_id = :v1 and valid_flag = 'Y'";
			stmt.setSQLString(sql);
			stmt << argv[3] ;
			stmt.execute();
			stmt>>source_group;	
			//cout << source_group <<endl;
	       
	        sql = "select count(*) from C_SETTLE_JOB where job_id = :v1 and valid_flag = 'Y'";
			stmt.setSQLString(sql);
			stmt << argv[3] ;
			stmt.execute();
			stmt>>source_count;
			//cout << source_count <<endl;
			if( source_count == 0 )
			{
			   theJSLog << "û��ƥ�������Դ�������־������־��ΪY " << endw;
			   return false;
			}


			sql = "select source_id from C_SETTLE_JOB where job_id = :v1 and valid_flag = 'Y'";
			stmt.setSQLString(sql);
			stmt << argv[3] ;
			stmt.execute();
			//cout << sql <<endl;
			
			for(int i=0;i<source_count;i++)
			{
			   stmt >> stparam[i].source_id;
			   
			}
	 }else{
	 	theJSLog<<"connect error."<<endi;
	 }
	    //conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "��ȡ����Դ��Ϣʧ��", __FILE__, __LINE__);
    }

	 // дsql �ļ�
	 //isWriteSQLFile(); 
	//��ʼ������ƽ̨,ͬʱ��ȡ������ϵͳ״̬
    //syn_status = syncInit(); 
	bGetExitSig = false;
	// �������� jsSettleRate   YYYYMM   jobid

	//char szLogPath[] = {"/mboss/jtcbs/zbjs1_a/log"};
	//char szLogLevel[] = {"1"};
	theJSLog.setLog(PS_Process::szLogPath, PS_Process::szLogLevel, "RATE", source_group, 1);
	strcpy(billmonth,argv[2]);	   
	strcpy(Param.ratecycle,argv[2]);
	strcpy(jobid,argv[3]);
	strcpy(Param.szSourceGroup,source_group);
}

bool C_MainFlow::dealFile(STparam &szstparam)
{
    /*BillRate BillRate;
	CF_MemFileI _infile;
	CF_MemFileO _outfile;
	CFmt_Change _inrcd,_outrcd;
	MessageParser  pMessage;
	
	char szinFiletype[10];
	char szFileouttype[10];    
	memset(tmp,0,sizeof(tmp));
	strcpy(szinFiletype,Param.infmt_type); //��ڸ�ʽ
	strcpy(szFileouttype,Param.outfmt_type);
	
	
	_infile.Init(szinFiletype);
	_outfile.Init(szFileouttype);
	_inrcd.Init(szinFiletype);
	_outrcd.Init(szFileouttype);*/
	char fileName[1024];
    char tmp[512];

    map<string,string>::iterator it; 
    int i=0;
    //cout << "szstparam.source_id = " << szstparam.source_id<<endl;
   if( (it = sourcemes.find(szstparam.source_id)) != sourcemes.end() )
   	{
   	//}
    
    //for(it = sourcemes.begin();it != sourcemes.end();++it)
    //{
      strcpy(m_szSourceId,it->first.c_str());  //����Դ
      strcpy(m_szinFilePath,it->second.c_str());
      strcat(m_szinFilePath,szstparam.invalue);
      //strcat(m_szinFilePath,stparam[i].invalue); //ʵ��·��
      strcpy(m_szoutFilePath,it->second.c_str());
      strcat(m_szoutFilePath,szstparam.outvalue);//ʵ�����·��
      //strcat(m_szoutFilePath,stparam[i].outvalue);
      i++;
      theJSLog << "����Դ��ȡ��·��Ϊ : " << m_szSourceId << "  m_szinFilePath :" << m_szinFilePath << endi;

      if(scan.openDir(m_szinFilePath))
		{
			theJSLog << "�򿪻����ļ�Ŀ¼ʧ��" << m_szinFilePath << endi;
			return ;	
		}
      //ѭ����ȡĿ¼��ɨ���ļ��У���ȡ�ļ�
		int rett = -1 ;
		int counter = 0;
		char filter[256] ="*txt";  //�ļ���ʽ��Ŀǰ��txt ��βΪ��׼   
		while(1) 
		{				   
			memset(fileName,0,sizeof(fileName));
			rett = scan.getFile(filter,fileName);  
			
			if(rett == 100)
			{		
				cout<<"��ʱ�ļ�Ŀ¼����û���ļ���ɨ���¸�����Դ"<<endl;
				break ;  
			}
			if(rett == -1)
			{
				continue;			//��ʾ��ȡ�ļ���Ϣʧ��
			}
            theJSLog << "fileName = " << fileName << endi;
					counter++;				//ɨ��һ���ļ�������+1				
					if(counter > 10)
					{
										//cout<<"ɨ��10�κ������¸�����Դ"<<endl;
						break;
					}

					/*�����ļ�*.tmp,*TMP,~* */			
					char* p = strrchr(fileName,'/');
					//char tmp[512];
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,p+1);
					//cout<<"�ļ�����"<<tmp<<endl;

					if(tmp[0] == '~' ) continue;
					if(strlen(tmp) > 2)
					{		
						int pos = strlen(tmp)-3;
						if((strcmp(tmp+pos,"tmp") && strcmp(tmp+pos,"TMP")) == 0) 
						{
							continue;
						}
					}				
					cout<<"ɨ�赽�ļ���"<<fileName<<endl;
					strcpy(m_szFileName,p+1);  //�����ļ���,ȥ·��				
					/*memset(tmp,0,sizeof(tmp));
					strcpy(tmp,fileName);
					strcat(tmp,".tmp");
					if(rename(fileName,tmp) != 0) //�ļ���������ʱ�ļ�
					{
						cout<<"�ļ�����ʧ��!"<<fileName<<endl;
						continue ;
					}	*/	
					//strcat(m_szFileName,m_szinFilePath); //ʵ���ļ�������·��			
					//strcat(m_szOutFileName,m_szoutFilePath); //ʵ���ļ�������·��		
					strcat(m_szinFilePath,m_szFileName); //ʵ���ļ�������·��			
					strcat(m_szoutFilePath,m_szFileName); //ʵ���ļ�������·��
					//dealFile();
					//dealRecord(stparam[i]);					
					 if( dealRecord(szstparam) )
	               	{
	               	    insertLog(szstparam.source_id,"Y");
	               	} else {
                        insertLog(szstparam.source_id,"E");
	               	}					
				}//while(1)
				scan.closeDir();				
    }
		sleep(5);
		return true;
}

bool C_MainFlow::dealRecord(STparam &szstparam)
{
    BillRate billRate;
	CF_MemFileI _infile;
	CF_MemFileO _outfile;
	CFmt_Change _inrcd,_outrcd;
	MessageParser  pMessage;
	
	char szinFiletype[10];
	char szFileouttype[10];
    char fileName[1024];
    char tmp[512];
	memset(tmp,0,sizeof(tmp));

	strcpy(szinFiletype,szstparam.intxt_type); //��ڸ�ʽ
	strcpy(szFileouttype,szstparam.outtxt_type);
	//cout << " szinFiletype = " <<szinFiletype<<endl;
	//cout << " szFileouttype = " <<szFileouttype<<endl;
	
	_infile.Init(szinFiletype);
	_outfile.Init(szFileouttype);
	_inrcd.Init(szinFiletype);
	_outrcd.Init(szFileouttype);

    cout << " m_szFileName = " <<m_szFileName<<endl;
    cout << " szstparam.source_id = " <<szstparam.source_id<<endl;
	/*pMessage.setMessage(MESSAGE_NEW_BATCH, szstparam.source_id, m_szFileName,1111);
	billRate.message(pMessage);
	theJSLog <<"Batch Success.."<<endi;
	theJSLog <<"start file.."<<endi;
	pMessage.setMessage(MESSAGE_NEW_FILE, szstparam.source_id, m_szFileName,1111);	//m_szSourceId	
	billRate.message(pMessage);*/
	theJSLog <<"record .."<<endi;

    //cout << "m_szinFilePath = " << m_szinFilePath<<endl;
    //cout << "m_szoutFilePath = " << m_szoutFilePath<<endl;
    _infile.Open(m_szinFilePath);
    _outfile.Open(m_szoutFilePath);
    
	//_infile.Open("/mboss/home/zhjs/work/vivi/NOG.0001");
	//_outfile.Open("/mboss/home/zhjs/work/vivi/NOG.0001.result");	
	PacketParser ps;
	ResParser retValue;
	while( _infile.readRec(_inrcd) != READ_AT_END)
	{
		ps.setRcd(_inrcd);
		_outrcd.Copy_Record(_inrcd);
		retValue.setRcd(_outrcd);
		billRate.execute(ps,retValue);
		_outfile.writeRec(retValue.m_outRcd);
				
	}
	/*pMessage.setMessage(MESSAGE_END_FILE, m_szSourceId, m_szOutFileName,1111);			
	billRate.message(pMessage);
	theJSLog <<"end file.."<<endi;
	pMessage.setMessage(MESSAGE_END_BATCH_END_DATA, m_szSourceId, m_szOutFileName,1111);						
	billRate.message(pMessage);
	theJSLog <<"end batch .."<<endi;
	pMessage.setMessage(MESSAGE_NEW_BATCH, m_szSourceId, m_szOutFileName,1111);
	billRate.message(pMessage);
	theJSLog <<"Batch Success.."<<endi;
	pMessage.setMessage(MESSAGE_END_BATCH_END_DATA, m_szSourceId, m_szOutFileName,1111);						
	billRate.message(pMessage);
	theJSLog <<"end batch .."<<endi;*/
	_outfile.Close();
	_infile.Close();
	cout << "deal Record over" << endl;
    return true;
}
bool C_MainFlow::dealTable(STparam &szstparam)
{
    BillRate billRate;
	CFmt_Change _inrcd,_outrcd;
	MessageParser  pMessage;
	
	char szinTabletype[10];
	char szoutTabletype[10];
    char tableName[1024];
    char tmp[512];
	memset(tmp,0,sizeof(tmp));
	strcpy(szinTabletype,szstparam.intxt_type); //��ڸ�ʽ
	strcpy(szoutTabletype,szstparam.outtxt_type); //�����ݿ�����һ��	

	_inrcd.Init(szinTabletype);
	_outrcd.Init(szoutTabletype);

	PacketParser ps;
	ResParser retValue;	
	
    /////////////////////////////////////////////////////////
    //�����ݿ���ж�ȡ���ݣ�һ��������
    //try{	
    //DBConnection conn;//���ݿ�����
	//if (dbConnect(conn))
	 //{
			m_stmt = conn.createStatement();
			Statement stmt = conn.createStatement();
			char szSqlTmp[SQL_LEN+1];

            char column_name[15];
            char all_column_name[1024];
            char column_value[15];
            char all_column_value[1024];
            int column_no;
            int value_num=0;
            int value_len=0;
            char column_out_value[1024];
            char sql[5000] ={0};
            record_num =0;
           try{
           	// ������ʱ��������ԭ������
           	//cout << "isRedo = " <<isRedo<<endl;
           //	if(isRedo == 1)
           	//{
           	theJSLog << "ɾ��ԭ����¼" <<endi;
           	    sprintf(szSqlTmp,"delete from %s_%s where %s like '%s%s'",szstparam.outvalue,billmonth,szstparam.settle_month,billmonth,"%");		
			    stmt.setSQLString(szSqlTmp);		
			    stmt.execute();
			    stmt.commit();
			    
           // ��������־��Ϊ�����Ӻ�update
		   insertLog(szstparam.source_id,"W");
           	//}   
            //��ȡ����������
            sprintf(szSqlTmp,"select count(*) from C_TXTFILE_FMT where filetype_id = '%s'",szstparam.intxt_type);
			stmt.setSQLString(szSqlTmp);
			stmt.execute();
			stmt>> column_no;

			sprintf(szSqlTmp,"select COLNAME from C_TXTFILE_FMT where filetype_id = '%s' order by col_index",szstparam.intxt_type);
			stmt.setSQLString(szSqlTmp);
			stmt.execute();
			while(stmt>>column_name)
			{
			   strcat(all_column_name,column_name);
	           strcat(all_column_name,",");
			}
			all_column_name[strlen(all_column_name)-1] = 0;  //��ȡSQL ����еı���ֵ
			//cout << "all_column_name = " << all_column_name<<endl;

			//ʵ��ֵ������
			sprintf(szSqlTmp,"select count(*) from %s_%s where %s like '%s%s'",szstparam.invalue,billmonth,szstparam.settle_month,billmonth,"%");		
            //theJSLog <<"szSqlTmp="<<szSqlTmp<<endl;
			stmt.setSQLString(szSqlTmp);
			stmt.execute();
			stmt >> value_num;	
			//theJSLog << "record_num="<<value_num<<endl;
            
			//��ȡֵ
			sprintf(szSqlTmp,"select %s from %s_%s where %s like '%s%s'",all_column_name,szstparam.invalue,billmonth,szstparam.settle_month,billmonth,"%");		
			stmt.setSQLString(szSqlTmp);
		    //theJSLog<<"szSqlTmp="<<szSqlTmp<<endl;			
			stmt.execute();
			int i=0;
			
			while(stmt >> column_value)
			{
			   strcat(all_column_value,column_value);
	           strcat(all_column_value,",");
	           i++;

	           if(i==column_no)  //��ɵ�һ����¼
	           	{
	           	   all_column_value[strlen(all_column_value)-1]=0;
	           	   //cout <<"all_column_value = "<< all_column_value << endl;
	           	   i=0; //  i ������λ	           	   
	           	   _inrcd.Set_record(all_column_value,strlen(all_column_value));
	           	   ps.setRcd(_inrcd);
		           _outrcd.Copy_Record(_inrcd);
		           retValue.setRcd(_outrcd);
		          // cout << "billrate.execute " << endl;
		           
		           billRate.execute(ps,retValue); 
		           int dealFlag=billRate.dealFlag;
		          // cout<<"dealFlag:"<<dealFlag<<endl;
		          if(dealFlag!=0 && dealFlag!=-1 && dealFlag!=-2 && dealFlag!=-3 && dealFlag !=-4)
		          	continue;
		           
		           //��ȡ���ؽ��ֵ   int CFmt_Change::Get_Field(int count, char *array)
	               char *tmp_value = retValue.m_outRcd.Get_record();
		           sprintf(column_out_value,"%s",tmp_value);
	               //cout << "column_out_value = "<< column_out_value << endl; 

                   //char *sql = getSql(&column_out_value);
                   
	               memset(sql,0,sizeof(sql));
                   getSql(column_out_value,all_column_name,sql,szstparam.outvalue);
	               //cout << "�����SQL  " << sql<<endl;
	              // insertTableData(sql); 
	               //insertLog(szstparam.source_id,"Y");
	               if( insertTableData(sql) )
	               	{
	               	    record_num++;
				    if(record_num%500==0)
	               	   	 theJSLog << "record_seq="<<record_num<<endd;
	               	    //insertLog(szstparam.source_id,"Y");
	               	} else {
                        //insertLog(szstparam.source_id,"E");
                        theJSLog << "�������ݳ���" <<endd;
	               	}
	              memset(all_column_value,0,sizeof(all_column_value));
	              memset(column_out_value,0,sizeof(column_out_value));

	              //�ﵽһ���������ύ
	              if( record_num%COMMIT_COUNT == 0 )
	              	{
		               theJSLog<<"�ύ����:"<<record_num<<endi;
		               m_stmt.commit();
		               m_stmt.close();
		               m_stmt = conn.createStatement();
	                }
	              // end һ��������¼
	           	}
			}	
			
		     //��������������
	         if(value_num != record_num)
	         {
				theJSLog<<"failed,�����ļ�¼��["<<record_num<<"]������ļ�¼��["<<value_num<<"]��һ��!"<<endw;
				m_stmt.rollback();
				insertLog(szstparam.source_id,"E");
				//ͬʱ��Ҫ����ʼ���ύ����ɾ��
				sprintf(szSqlTmp,"delete from %s_%s where %s like '%s%s'",szstparam.outvalue,billmonth,szstparam.settle_month,billmonth,"%");		
			    stmt.setSQLString(szSqlTmp);		
			    stmt.execute();
			    stmt.commit();
	            return false;
	         } 
	         else
	         {
	            insertLog(szstparam.source_id,"Y");
				m_stmt.commit();
				m_stmt.close();	
	            conn.close();	            
	            cout << "Success ,dealTable over ,record_num="<<record_num<<endl;
	            return true;
	         }
			
			//cout << "dealTable over 2" <<endl;
		  
	       } catch( SQLException e ){

				m_stmt.rollback();
				theJSLog<<"dealTable() ���ݿ��쳣:"<<e.what()<<endw;

				sprintf(szSqlTmp,"delete from %s_%s where %s like '%s%s'",szstparam.outvalue,billmonth,szstparam.settle_month,billmonth,"%");		
			    stmt.setSQLString(szSqlTmp);		
			    stmt.execute();
			    stmt.commit();
				conn.close();
				return false;
		    } 
	       catch (jsexcp::CException ce) {
				
				m_stmt.rollback();
				theJSLog<<"dealTable() �쳣:"<<ce.GetErrMessage()<<endw;

		        sprintf(szSqlTmp,"delete from %s_%s where %s like '%s%s'",szstparam.outvalue,billmonth,szstparam.settle_month,billmonth,"%");		
			    stmt.setSQLString(szSqlTmp);			
			    stmt.execute();
			    stmt.commit();
				conn.close();
				//throw jsexcp::CException(errno, "��ȡ����������Ϣʧ��", __FILE__, __LINE__);
				return false;
		    }
				
	 //}
	//else
	 //{
	 	//theJSLog<<"connect error."<<endi;
	 	//return false;
	// }
	             
	    //conn.close();
	 
    return true;
}

void C_MainFlow::getSourceInfo()
{
  // int path_num;
  //DBConnection conn2;//���ݿ�����
   try{			
	//if (dbConnect(conn2))
	 {
	       //�ɿ�ʼ��ȡ��������Դ�����������Դ��Ϊͨ��jobid����ȡ����Դ
	        Statement stmt1 = conn.createStatement();
	        string sourcepath;
	        for(int i=0;i<source_count;i++)
	        {
	           string sql = "select source_path from I_SOURCE_DEFINE where source_id = :v1";		
			   stmt1.setSQLString(sql);
			   stmt1 << stparam[i].source_id ;
			   stmt1.execute();
			   stmt1 >> sourcepath;
			    if ( sourcepath[strlen(sourcepath.c_str())-1] != '/' )
                	strcat(sourcepath.c_str(),"/");
			   sourcemes.insert(map<string,string>::value_type(stparam[i].source_id,sourcepath));
			   theJSLog << "����Դ�������û�ȡ�ɹ�" <<endi;
	        }
			
	       //��ȡ����Դ��·��
			/*Statement stmt = conn.createStatement();	        			
			string sql = "select count(*) from C_SOURCE_GROUP_CONFIG a ,I_SOURCE_DEFINE b where a.source_group = :v1 and A.SOURCE_ID = B.SOURCE_ID";		
			stmt.setSQLString(sql);
			stmt << source_group ;
			stmt.execute();
			stmt >> path_num;

			if ( path_num > 0) {
			string sql = "select a.source_id,b.source_path from C_SOURCE_GROUP_CONFIG a ,I_SOURCE_DEFINE b where a.source_group = :v1 and A.SOURCE_ID = B.SOURCE_ID";		
			stmt.setSQLString(sql);
			stmt << source_group ;
			stmt.execute();
			for(int i=0;i<path_num;i++){
				std::string sourceid,sourcepath;
             while ( stmt >> sourceid >> sourcepath )
              {
                if ( sourcepath[strlen(sourcepath.c_str())-1] != '/' )
                	strcat(sourcepath.c_str(),"/");
                sourcemes.insert(map<string,string>::value_type(sourceid,sourcepath));
               theJSLog << "����Դ�������û�ȡ�ɹ�" <<endi;
               // if(source_path[i].szSourcePath[strlen(source_path[i].szSourcePath)-1] != '/')
			         //strcat(source_path[i].szSourcePath, "/");
               }
			 }
			} else {
              theJSLog << "����Դ�������ô���" <<endi;
			}*/
	 }
	//else{
	    //conn2.close();
	 	//theJSLog<<"connect error."<<endi;
	 //}
	    //conn2.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		//conn2.close();
		throw jsexcp::CException(errno, "��ȡ����Դ��Ϣʧ��", __FILE__, __LINE__);
    }
}

bool C_MainFlow::insertTableData(char *sql)
{
  //cout << "sql 1= " << sql << endl;
  //char allsql[2048];
  //memset(allsql,0,sizeof(allsql));
  //strcpy(allsql,sql);

// try{		
    //DBConnection conn3;//���ݿ�����
	//if (dbConnect(conn3))
	 //{
			//Statement stmt3 = conn.createStatement();
			//char *szSqlTmp = getSql(&*sql);
			//cout << "allsql 2= " << sql << endl;
			
	        //rtinfo.getDBSysMode(petri_status);		//��ȡ״̬
	        //cout<<"petri_status:"<< petri_status <<endl;

			   m_stmt.setSQLString(sql);
			//cout<<"petri_status:"<< petri_status <<endl;
			//if(petri_status==303)
			//{
			   //cout<<"petri_status:"<< petri_status <<endl;
			   m_stmt.execute();
			   //cout<<"petri_status:"<< petri_status <<endl;
			   //m_stmt.commit();
			   //cout<<"petri_status:"<< petri_status <<endl;
			   //conn3.close();
		    //} else
		    //{
		    //������־
		     // conn3.close();
		    //  return false;
		    //}
			//sql ���ִ�е�ͬʱдsql�ļ�
			//writeSQL(sql);
	 //}
	//else
	// {
	   // conn3.close();
	 	//theJSLog<<"connect error."<<endi;
	 //}
	    //conn3.close();
	// } 
   /* catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn3.close();
		throw jsexcp::CException(errno, "��ȡ����������Ϣʧ��", __FILE__, __LINE__);
    }*/
	 //cout << "insert Table data over" <<endl;
   return true;
}

void C_MainFlow::getSql(char *sqltmp,char *tableitem,char *result,char *tablename)
{
   //cout<<"getsql: sqltmp="<<sqltmp<<" tableitem="<<tableitem<<endl;

   char total_sql[10000]={0};
   char alltableitem[500]={0};
   //cout << "sql = " << *sqltmp <<endl;
   //string_replace(string(sqltmp),",","','");
   ReplaceStr(sqltmp,",","'|'");
   //cout << "sql = " << sqltmp <<endl;
   ReplaceStr(sqltmp,"|",",");
   //cout << "sql = " << sqltmp <<endl;   
   //cout << tableitem<<endl;

  /* strcat(total_sql,"insert into ");
   strcat(total_sql,tablename);
   //strcat(total_sql,"(");
   //strcat(total_sql,tableitem);
   strcat(total_sql,")");
   strcat(total_sql," values('");
   strcat(total_sql,sqltmp);
   strcat(total_sql,"')");*/
   strcat(alltableitem,tableitem);
   
   //cout << "total_sql2 = " << total_sql <<endl;
   //sprintf(total_sql,"insert into %s values('%s",stparam.outpath,*sqltmp);
   //cout << "total_sql = " << total_sql <<endl;
   sprintf(total_sql, "insert into %s_%s (%s) values('%s')",tablename,billmonth,alltableitem,rtrim_lc(sqltmp));		
   //cout << "total_sql = " << total_sql <<endl;
   strcpy(result,total_sql);
   //return true;
}

int C_MainFlow::ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr)
{
        int  StringLen;
        char caNewString[1024];
        char caOldString[1024];
        sprintf(caOldString,"%s",sSrc);
        //cout << "sMatchStr " << sMatchStr<< endl;
        //cout << "sReplaceStr " <<sReplaceStr << endl;

        char *FindPos = strstr(caOldString, sMatchStr);  //sSrc Դ�ַ���,sMatchStr ƥ���ַ�����sReplaceStr �滻�ַ���
        if( (!FindPos) || (!sMatchStr) )
                return -1;

        while( FindPos )
        {
                memset(caNewString, 0, sizeof(caNewString));
                StringLen = FindPos - caOldString;
                strncpy(caNewString, caOldString, StringLen);
                strcat(caNewString, sReplaceStr);
                strcat(caNewString, FindPos + strlen(sMatchStr));
                strcpy(caOldString, caNewString);
                FindPos = strstr(caOldString, sMatchStr);
        }
        strcpy(sSrc,caOldString);
        return 0;
}

void C_MainFlow::insertLog(char *source_id,char *dealflag)
{
   //SGW_RTInfo rtinfo;
   //DBConnection conn2;//���ݿ�����
   char tmpflag[2];
   strcpy(tmpflag,dealflag);
   //cout << "tmpflag = " << tmpflag <<endl;
 try{			
	        char sql[1024];
			memset(sql,0,sizeof(sql));
	        //string sql = "insert into D_SETTLE_LOG(source_id,deal_time,billmonth,deal_flag,job_id) values(:v1,to_char(SYSDATE,'yyyymmddhh24miss'),:v2,:v3)";
            if ( strcmp(tmpflag , "W") ==0 )
           {
              sprintf(sql, "insert into D_SETTLE_LOG(source_id,deal_time,billmonth,deal_flag,job_id) values('%s',to_char(SYSDATE,'yyyymmddhh24miss'),'%s','%s','%s')",
	                  source_id,billmonth,dealflag,jobid);		
			  m_stmt.setSQLString(sql);
			  theJSLog << sql <<endd;
			  m_stmt.execute();
			  m_stmt.commit();
           } else 
           {
               sprintf(sql, "update D_SETTLE_LOG set deal_flag = '%s' where deal_flag ='W' and job_id = '%s'",tmpflag,jobid);		
			   m_stmt.setSQLString(sql);
			   theJSLog << sql <<endd;
			   m_stmt.execute();
			   m_stmt.commit();
           }
	        

	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		//conn2.close();
		throw jsexcp::CException(errno, "��ȡ����Դ��Ϣʧ��", __FILE__, __LINE__);
    }
	 //cout<< "insertLog over" <<endl;
	 return ;
}

bool C_MainFlow::dealAll()
{
    bool result;
	for(int i=0;i<source_count;i++)
	{
	    //char flag[5];
	   
	 if ( strcmp(stparam[i].intype, "F") ==0 )  //Ĭ�ϴ��ļ���ȡ��������
		{
			 result = dealFile(stparam[i]);				
				
		} else if (strcmp(stparam[i].intype, "T") ==0 ) //�����ݿ���л�ȡ��������
		{
		    result = dealTable(stparam[i]);
		    //cout << "result = " <<result<<endl;
		}
		if (result == false)
		{
		  break;
		}
		
	     // ������֮���ύsql�ļ�
         //commitSQLFile();
		//cout << "flag = " <<flag<<endl;
		//cout << "process.stparam[i].source_id = " <<process.stparam[i].source_id<<endl;
		//process.insertLog(process.stparam[i].source_id,flag);
	}
	return result;
}
char* rtrim_lc(char* s)  
{  
    //char* s_s=new char[strlen(s)+1];  
	
	char d_s[1024];
	memset(d_s,0,sizeof(1024));
    strcpy(d_s,s);    
    int d_len=strlen(d_s);  

    for(int i=d_len-1;i>=0;i--)  
    {  
         if(d_s[i]==' ')  
         {  
             d_s[i]='\0';  
         }  
    }  
    //char* d_s=new char[strlen(s_s)];  
    //strcpy(d_s,s_s);  	
	//strcpy(d_s,s_s);
	//delete[] s_s;

    return d_s;   
}
void dealSignal(int sig)
{
	if(sig == SIGTERM)
	{
		theJSLog<<"�յ���ֹ������źţ�"<<sig<<endi;
		bGetExitSig = true;
	}
	else
		bGetExitSig = false;
}