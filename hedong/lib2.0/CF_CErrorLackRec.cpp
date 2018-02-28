/*
* Copyright (c) 2003, �㶫���ſƼ����޹�˾
* All rights reserved.
*
* filename��CF_CErrorLackRec.cpp
* version��1.1.2
* update list: 
* discription���޸Ĵ���ANARES�ֶΣ�ֻ���ҵ�����Է����Ľ��
*/
//��������ʱ�ļ�����ΪYYYYMMDDhhmmssYYYYMM
//20060809   �޸��ļ�������Rollback��û�����count�Ĵ��󣬸�Ϊ��open�г�ʼ��
//20080603 by zhoulh ɾ����ʱ�ļ�ֻɾ������ˮ��

#include "CF_CErrorLackRec.h"
//#include "CF_CError.h"
//#include "CF_CFscan.h"

CF_CErrorLackRec::CF_CErrorLackRec()
{
	fp = NULL;
	Count = 0;
	proc_id = -1;
	type = abenum_lackinfo;		//Ĭ��Ϊ���������ʹ���
	memset(path_name, 0, PATHLEN);
	memset(file_name, 0, FILELEN);
	memset(temp_file, 0, FILELEN);
	header = NULL;
	current = NULL;
	LinkCount = 0;
	memset(table, 0, TABLELEN);
	sELParam.Is_Save_Table=0;
    sELParam.Save_Table_Config=0;
    sELParam.Is_Stat_Table=0;
    memset(sELParam.pipe_id,0,6);

}

CF_CErrorLackRec::~CF_CErrorLackRec()
{
	if(fp)
	{
		fclose(fp);
		//Close();
		/*����û���ύ����,����Ա��������ʱ�ļ������ٴ��ύ*/
	}
	ErrorLackRec_Link* tmp = header;
	while(header)
	{
		header = header->next;
		delete tmp;
		tmp = header;
	}
}
/***************************************************
function
	int CF_CErrorLackRec::process_header();
description:
	д���쳣����ͷ��¼
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_FILE_WRITE 	�ļ�д�����
*****************************************************/
int CF_CErrorLackRec::process_header()
{
	char msg[MSGLEN];
	int r;
	if((r = fwrite("SOF", 3, 1, fp)) != 1)
	{
		sprintf(msg,"Write temporary file %s header note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	char s_len[10 + 1];               //���ֽ���10λ
	sprintf(s_len, "%02d", 6);	  //�����ֶμ�¼���ֵ��ֶ���,��¼��׼�����Ǹ���¼��ʼ
	if((r = fwrite(s_len, 2, 1,fp)) != 1)
	{
		sprintf(msg,"Write temporary file %s header note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	sprintf(s_len,"%010d\n",16);      //ͷ��¼16λ(����'\n')
	if((fwrite(s_len, 11, 1,fp)) != 1)
	{
		sprintf(msg,"Write temporary file %s header note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	return SUCCESS;
}
/***************************************************
function
	int CF_CErrorLackRec::process_column(int id, char* filename, CFmt_Change* change);
description:
	д���쳣��������Դ��¼
input:	
	id		���쳣�����Ļ������ͣ����磺�����ϻ��������������ͣ�
	filename	�����������ĸ��ļ�
	change		���滰����¼����
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_GET_RECORD	��ȡchange�����ݳ���
  ERR_FILE_WRITE 	�ļ�д�����
*****************************************************/
int CF_CErrorLackRec::process_column(int id, char* filename, CFmt_Change* change)
{
	char msg[MSGLEN];
	char ch[LACKLEN];
	char chSep;
	memset(ch, 0, LACKLEN);
	int r;
    chSep = change->Get_FieldSep(1);
    if(chSep == 0) chSep = ' ';
	if((r = change->Get_record(ch, LACKLEN)) != 0)
	{
		sprintf(msg,"CFmt_Change::Get_record() read infomation error\n");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,ERR_GET_RECORD, r, msg,__FILE__,__LINE__ -3);
	}
	//rewrite by wulei
	//int i = 20 + strlen(filename) + sizeof(char)*2 + strlen(ch) + 5 + 1;
	//char* arr = new char[i];
	char* arr = new char[500];
	switch(type)
	{
		case abenum_lackinfo:
		case abenum_error:
#ifdef _SHCJ_
  		    sprintf(arr,"%c%c%d%c%d%c%s%c%c%c%s\n", 'Y',chSep, id,chSep, proc_id,chSep, filename,chSep, 'N',chSep, ch);
#else
			sprintf(arr,"%d%c%d%c%s%c%c%c%c%c%s\n", id,chSep, proc_id, chSep,filename,chSep, 'Y',chSep, 'N', chSep,ch);
#endif
			break;
		case abenum_nonStand:
		case abenum_FixDup:
			sprintf(arr,"%d%c%s\n", id,chSep, ch);
			break;			
		default:
			break;
	}
	//end rewrite
	if((r = fwrite(arr, strlen(arr), 1, fp)) != 1)
	{
		delete[] arr;
		sprintf(msg,"Write temporary file %s source note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	delete[] arr;
	Count++;
	return SUCCESS;
}
/***************************************************
function
	int CF_CErrorLackRec::process_end();
description:
	д���쳣����β��¼
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_UNKOWN		����δ֪����
  ERR_FILE_WRITE 	�ļ�д�����
*****************************************************/
int CF_CErrorLackRec::process_end()
{
	char msg[MSGLEN];
	if(fwrite("wEND",4,1,fp) != 1)
	{
		sprintf(msg,"Write temporary file %s end note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	char ch[10];				//�ܼ�¼��6λ
	sprintf(ch,"%06d",Count);
	if(fwrite(ch,6,1,fp) != 1)		//�ܼ�¼6λ��'\n'һλ
	{
		sprintf(msg,"Write temporary file %s end note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	long length = ftell(fp);
	if(fseek(fp,5,SEEK_SET) !=0 )
	{
		sprintf(msg,"fseek temporary file %s error\n",temp_file);
		throw CF_CError('O','H',ERR_UNKOWN, errno, msg,__FILE__,__LINE__ -3);
	}
	sprintf(ch,"%010d",length);
	if((fwrite(ch,10,1,fp)) != 1)
	{
		sprintf(msg,"Rewrite temporary file %s header note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	return SUCCESS;
}

/***************************************************
function
	int CF_CErrorLackRec::init(char* rootpath,abnormity_type type_id,int process_id);
description:
	��ʼ���쳣�����ӿ�
input:	
	rootpath	�쳣�����ļ���Ŀ¼
	type_id		�쳣�������ͣ���������쳣�����������ϣ������ص��ȣ�
	process_id	������̴���
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_DIR_CREATE	����(��)Ŀ¼ʧ��
  ERR_DIR_CHANGE	�޷�����(��)Ŀ¼
  ERR_DIR_NULLITY	��Ч�ĸ�Ŀ¼
*****************************************************/
int CF_CErrorLackRec::init_Table(char *Pipe_id,int process_id)
{
//  CBindSQL ds(DBConn);
    char sqltmp[400];

    if(getEnvFromDB(DBConn,Pipe_id,process_id, "COMMON_LACKINFO_NEED_2_TAB",sqltmp)<0)
    {
    	sprintf(sqltmp,"can't find COMMON_LACKINFO_NEED_2_TAB in pipe_env");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,0, errno, sqltmp,__FILE__,__LINE__);
    }
    if(sqltmp[0] == 'Y') 
    {	
      sELParam.Is_Save_Table=1;
      if(getEnvFromDB(DBConn,Pipe_id,process_id, "COMMON_LACKINFO_2_TAB_CONFIG",sqltmp)<0)
      {
    	sprintf(sqltmp,"can't find COMMON_LACKINFO_2_TAB_CONFIG in pipe_env");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,0, errno, sqltmp,__FILE__,__LINE__);
      }
      sELParam.Save_Table_Config = atoi(sqltmp);
    }
    else sELParam.Is_Save_Table = 0;
    if(getEnvFromDB(DBConn,Pipe_id,process_id, "COMMON_LACKINFO_NEED_STAT",sqltmp)<0)
    {
    	sprintf(sqltmp,"can't find COMMON_LACKINFO_NEED_STAT in pipe_env");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,0, errno, sqltmp,__FILE__,__LINE__);
    }
    if(sqltmp[0] == 'Y') 
    {	
      sELParam.Is_Stat_Table=1;
      if(getEnvFromDB(DBConn,Pipe_id,process_id, "COMMON_LACKINFO_STAT_CONFIG",sqltmp)<0)
      {
    	sprintf(sqltmp,"can't find COMMON_LACKINFO_STAT_CONFIG in pipe_env");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,0, errno, sqltmp,__FILE__,__LINE__);
      }
      sprintf(sELParam.Stat_Table_Config,"%s",sqltmp);
      //sELParam.Save_Table_Config = atoi(sqltmp);
    }
    else sELParam.Is_Stat_Table = 0;

  sprintf(sELParam.pipe_id,"%s",Pipe_id);

  if(sELParam.Is_Stat_Table)
  {
    OStat_Table.Init(sELParam.pipe_id,process_id,sELParam.Stat_Table_Config,'N');
//    OStat_Table.Init(sELParam.pipe_id,process_id,"COMMON_LACKINFO_STAT_CONFIG");
  }

	proc_id = process_id;
    Is_Commit = 1;
  if(sELParam.Is_Save_Table)
  {
    OSave_Table.Init(sELParam.pipe_id,process_id,sELParam.Save_Table_Config);
    return SUCCESS;
  }

//20080603 by zhoulh
    CBindSQL ds(DBConn);
    int iInputId;
    ds.Open("select input_id from workflow where process_id=:iProcessId \
      and workflow_id=(select workflow_id from pipe where pipe_id=:szPipeId)", SELECT_QUERY );
    ds<<proc_id<<sELParam.pipe_id;
    if (!(ds>>iInputId))
    {
    	sprintf(sqltmp,"select input_id from workflow is NULL");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,0, errno, sqltmp,__FILE__,__LINE__);
    }
    ds.Close();
    ds.Open("select ctl_tabname from model_interface where \
      interface_id=:iInputId", SELECT_QUERY );
    ds<<iInputId;
    if (!(ds>>szInCtlTabname))
    {
    	sprintf(sqltmp,"select ctl_tabname from model_interface is NULL");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,0, errno, sqltmp,__FILE__,__LINE__);
    }
    ds.Close();

    ds.Open("select count(*) from source where pipe_id=:szPipeId", SELECT_QUERY);
    ds<<sELParam.pipe_id;
    ds>>iSourceCount;
    ds.Close();
    if (iSourceCount <= 0)
    {
    	sprintf(sqltmp,"select count(*) from source is Zero");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,0, errno, sqltmp,__FILE__,__LINE__);
    }

    ds.Open("select source_id from source where pipe_id=:szPipeId", SELECT_QUERY );
    ds<<sELParam.pipe_id;
    /* Repeat reading all record */
    for (int i=0; i<iSourceCount; i++)
    {
      ds>>szSourceId[i];
    }
    ds.Close();


    
  return SUCCESS;
}

/***************************************************
function
	int CF_CErrorLackRec::init(char* rootpath,abnormity_type type_id,int process_id);
description:
	��ʼ���쳣�����ӿ�
input:	
	rootpath	�쳣�����ļ���Ŀ¼
	type_id		�쳣�������ͣ���������쳣�����������ϣ������ص��ȣ�
	process_id	������̴���
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_DIR_CREATE	����(��)Ŀ¼ʧ��
  ERR_DIR_CHANGE	�޷�����(��)Ŀ¼
  ERR_DIR_NULLITY	��Ч�ĸ�Ŀ¼
*****************************************************/
int CF_CErrorLackRec::init(char* rootpath,abnormity_type type_id,int process_id)
{

	type = type_id;
	proc_id = process_id;
    Is_Commit = 1;
  if(!sELParam.Is_Save_Table)
  {
	char msg[MSGLEN];
	sprintf(path_name,"%s",rootpath);
	DelDir(path_name);
	switch(type_id)
	{
		case abenum_lackinfo:
			sprintf(path_name,"%s/lackinfo/%d",path_name,process_id);
			break;
		case abenum_error:
			sprintf(path_name,"%s/%s/%d",path_name,"error",process_id);
			break;
		case abenum_nonStand:
			sprintf(path_name,"%s/nonStand",path_name);
			break;
                case abenum_FixDup:
			sprintf(path_name,"%s/FixDup",path_name);
			break;			
		default:
			break;
	}
	int j;
	j = CheDir(path_name);
	if(j)
	{
		sprintf(msg,"CheDir(%s) error\n",path_name);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,j, errno, msg,__FILE__,__LINE__ -4);
	}	//��ʾ�Լ�*****�ص������ɷ�ʽ��·���������������ʽ���ǵÿ��ֵ�*****
	return SUCCESS;
  }
  return SUCCESS;
}
/***************************************************
function
	int CF_CErrorLackRec::Open()
description:
	��һ���µ��쳣������ʱ�ļ�
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_UNKOWN		����δ֪����
  ERR_FILE_OPEN 	�ļ��������ʱΪ��
  ERR_DIR_CREATE	����(��)Ŀ¼ʧ��
  ERR_DIR_CHANGE	�޷�����(��)Ŀ¼
  ERR_DIR_NULLITY	��Ч�ĸ�Ŀ¼
*****************************************************/
int CF_CErrorLackRec::Open()
{
	char msg[MSGLEN];
	char stime[15];
	char year_month[7];
	char day[3];
	char path[PATHLEN];

#ifdef _SHCJ_
	char *char1;
	char1 = strchr(file_name,'.');
	memcpy(stime,char1+1,8);
#else
	CurTime(stime);
#endif

	strncpy(year_month,stime,6);
	year_month[6] = 0;
	strncpy(day,stime+6,2);
	day[2] = 0;
	//add by wulei 2005-02-23
	switch(type)
	{
	        case abenum_lackinfo:
	                sprintf(path,"%s/%s/%s",path_name,year_month,day);
		        break;
            case abenum_error:
			//sprintf(path,"%s/%s/%s",path_name,year_month,day);
				break;
			case abenum_nonStand:
				sprintf(path,"%s/%s",path_name,year_month);
				break;
            case abenum_FixDup:
                sprintf(path,"%s/%s",path_name,year_month);
                break;		        
            default:
		        break;
        }
	//end add
	int j;
	j = CheDir(path);
	if(j)
	{
		sprintf(msg,"CheDir(%s) error\n",path);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_MID,j, errno, msg,__FILE__,__LINE__ -4);
	}
//	sprintf(temp_file,"%s/~%s.TMP",path,stime);
	sprintf(temp_file,"%s/~%s.TMP",path,file_name);

	fp = fopen(temp_file,"w");
	if(!fp)
	{
		memset(temp_file, 0, FILELEN);
		memset(file_name, 0, FILELEN);
		{
			sprintf(msg,"fopen(%s) error\n",temp_file);
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_LOW,ERR_FILE_OPEN, errno, msg,__FILE__,__LINE__ -7);
		}
	}
	else
	{
		Count = 0;
		return SUCCESS;
	}
}
/***************************************************
function
	int CF_CErrorLackRec::saveErrLackRec(int id, char* filename, CFmt_Change* change);
description:
	��һ���µ��쳣������ʱ�ļ�
input:	
	id		�쳣��������, ����������ϻ����������������ͣ�����Ǵ����Ǵ����ͣ�������ص��������ص����͡�
	filename	�����������ĸ��ļ�
	change		���滰����¼����
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_NOT_COMMIT	ǰһ�������ļ�������ɣ���û���ύ�����
  ERR_UNKOWN		����δ֪����
  ERR_FILE_OPEN 	�ļ��������ʱΪ��
  ERR_DIR_CREATE	����(��)Ŀ¼ʧ��
  ERR_DIR_CHANGE	�޷�����(��)Ŀ¼
  ERR_DIR_NULLITY	��Ч�ĸ�Ŀ¼
  ERR_GET_RECORD	��ȡchange�����ݳ���
  ERR_FILE_WRITE 	�ļ�д�����
*****************************************************/
int CF_CErrorLackRec::saveErrLackRec(int id, char* filename, CFmt_Change* change,char* source_id)
{
  int r;
  r=Is_Commit;

  if(sELParam.Is_Stat_Table)
  {
    if(Is_Commit)
    {
//      r=Is_Commit;
      Is_Commit =0;
      OStat_Table.setFileName(filename,source_id);
    }
    OStat_Table.dealRedoRec(*change,id);
  }
  if(!sELParam.Is_Save_Table)
  {
    if(fp)
	{
		if(strcmp(file_name,filename))
			return ERR_NOT_COMMIT;
		if(process_column(id, filename, change))
			return FAIL;
		return SUCCESS;
	}
	else
	{
		if(!fp)
		{
			if(strlen(temp_file))
				return ERR_NOT_COMMIT;
			//strcpy(file_name,filename);
			sprintf(file_name,"%s",filename);
			if(Open())
				return FAIL;
			//add by wulei 2005-02-23
			switch(type)
	                {
		                case abenum_lackinfo:
			                if(process_header())
				                return FAIL;                
				        break;
		                default:
			                break;
	                }
	                //end add
			
		}
		if(process_column(id, filename, change))
			return FAIL;
		return SUCCESS;
	}
  }
  else
  {
    if(r)
    {
      Is_Commit = 0;
      OSave_Table.setFileName(filename,source_id);
    }
    OSave_Table.dealInsertRec(*change,id);
    return SUCCESS;
  }
}
/***************************************************
function
	int CF_CErrorLackRec::Close();
description:
	�ر��쳣�����ļ�
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_UNKOWN		����δ֪����
  ERR_FILE_CLOSE 	�ر��ļ�ʧ��
  ERR_FILE_WRITE 	дͷ��β��¼����
*****************************************************/
int CF_CErrorLackRec::Close()
{
	if(!fp)
		return SUCCESS;
        //add by wulei 2005-02-23
	switch(type)
	{
                case abenum_lackinfo:
	                if(process_end())
		                return FAIL;
                        break;
                case abenum_nonStand:
		case abenum_FixDup:
		case abenum_error:
		        break;
        }		
	//end add

	if(fclose(fp))
	{
		char msg[MSGLEN];
		sprintf(msg,"fclose(%s) error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_CLOSE, errno, msg,__FILE__,__LINE__ -4);
	}
	fp = NULL;
	//memset(file_name, 0, FILELEN);
	Count = 0;
	return SUCCESS;
}
/***************************************************
function
	int CF_CErrorLackRec::Commit();
description:
	�ύ�쳣�����ļ�������ʱ�ļ�����Ϊ��ʽ�ļ�����
	ע��:��CF_CErrorLackRec::proc_TMPFile(char* filename)�������ύ�Ĳ��ҵ���filename
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
  DO_CLOSE_FIRST	�ɹ��������ύǰ���Զ����˹ر��ļ�����(Close)
throw
  ERR_UNKOWN		����δ֪����
  ERR_FILE_CLOSE 	�ر��ļ�ʧ��
  ERR_FILE_WRITE 	дͷ��β��¼����
  ERR_RENAME_FILE	����ʱ�ļ�����Ϊ��ʽ�ļ���ʱ����
*****************************************************/
int CF_CErrorLackRec::Commit()
{
  Is_Commit =1;
  if(sELParam.Is_Stat_Table)
  {
    char stime[16];
    CurTime(stime);
    expTrace("Y", __FILE__, __LINE__,"before OStat_Table.update_commit:%s;", stime);
    OStat_Table.update_commit();
    CurTime(stime);
    expTrace("Y", __FILE__, __LINE__,"after OStat_Table.update_commit:%s;", stime);
  }
  if(!sELParam.Is_Save_Table)
  {
	int b = 0;
	if(fp)
	{
		if(Close())
			return FAIL;
		b = 1;
	}
	char msg[MSGLEN];
	if(strlen(temp_file))
	{
		char new_name[FILELEN];
		memset(new_name, 0, FILELEN);
		char* ch = strrchr(temp_file, '/');
		int i = ch - temp_file + 1;	//+'/'
		strncpy(new_name,temp_file,i);
		switch(type)
		{
			case abenum_lackinfo:
			case abenum_nonStand:
			case abenum_FixDup:
			case abenum_error:
				strcat(new_name,file_name);
				break;
                        default:
                                break;
			/*case abenum_duplation:
				char* chr = strrchr(file_name,'.');
				strncpy(file_name,file_name,chr - file_name);
				sprintf(new_name,"%s%s.rep",new_name,file_name);
				break;
			*/	
		}
		//strcat(new_name,file_name);
    char stime[16];
    CurTime(stime);
    expTrace("Y", __FILE__, __LINE__,"before OStat_Table.update_commit:%s;", stime);
		if((i=rename(temp_file,new_name)) != 0)
		{
			sprintf(msg,"rename(%s,%s) error\n",temp_file,new_name);
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_RENAME_FILE, errno, msg,__FILE__,__LINE__ -3);
		}
    CurTime(stime);
    expTrace("Y", __FILE__, __LINE__,"after OStat_Table.update_commit:%s;", stime);
		memset(file_name, 0, FILELEN);
		memset(temp_file, 0, FILELEN);
	}
	if(b)
		return DO_CLOSE_FIRST;
	else
		return SUCCESS;
  }
  else
  {
    OSave_Table.commit();
    return SUCCESS;
  }
}
/***************************************************
function
	int CF_CErrorLackRec::RollBack();
description:
	���˵�ǰ�쳣�����ļ�(����ʱ�ļ�ɾ��)
	ע��:��CF_CErrorLackRec::Commit(char* filename)������ɾ�����ǲ��ҵ���filename
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL			����ִ��ʧ��
throw
  ERR_FILE_CLOSE	�ر��ļ�ʱ����
  ERR_REMOVE_FILE	ɾ����ʱ�ļ�ʱ����
*****************************************************/
int CF_CErrorLackRec::RollBack()
{
  Is_Commit =1;
  if(sELParam.Is_Stat_Table)
  {
    OStat_Table.rollback();
  }
  if(!sELParam.Is_Save_Table)
  {
	if(!(strlen(temp_file)))
		return SUCCESS;
	char msg[MSGLEN];
	if(fp)
		if(fclose(fp))
		{
			sprintf(msg,"fclose(%s) error",temp_file);
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_FILE_CLOSE, errno, msg,__FILE__,__LINE__ -3);	
		}
	fp = NULL;
	Count = 0;

	memset(file_name, 0, FILELEN);
	int j;
	if((j=remove(temp_file)) != 0)
	{
		sprintf(msg,"remove(%s) error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_REMOVE_FILE, errno, msg,__FILE__,__LINE__ -3);
	}
	memset(temp_file, 0, FILELEN);
	return SUCCESS;
  }
  else
  {
    OSave_Table.rollback();
    return SUCCESS;
  }
}
/***************************************************
function
	int CF_CErrorLackRec::Proc_TMPFile(char* filename);
description:
	����쳣�����ļ�(*.TMP),�Ѵ�����̲�����(û��β��¼)���ļ�ɾ��,��������������ļ�����Ϊ��ʽ
	ע�⣺�˺����ĵ���ֻ����CF_CErrorLackRec::init()֮��,���봦�������
input:	
output:
  filename				���ҵ��Ļ����ļ���
return
  SUCCESS              	����ִ�гɹ�
  FAIL					����ִ��ʧ��
throw
  ERR_DIR_OPEN		�򿪹���Ŀ¼ʧ��
  ERR_FILE_OPEN		�ļ���ʱΪ��
  ERR_FILE_CLOSE	�ر��ļ�ʱ����
  ERR_REMOVE_FILE	ɾ����ʱ�ļ�ʱ����
  ERR_RENAME_FILE	����ʱ�ļ�����Ϊ��ʽ�ļ���ʱ����
  ERR_UNKOWN		δ֪����
*****************************************************/
int CF_CErrorLackRec::Proc_TMPFile(char* filename)
{
	if(sELParam.Is_Save_Table)
		return SEARCH_EOF;
	DIR* dp;
	char msg[MSGLEN];
	dp = opendir(path_name);
	if(!dp)
	{
		sprintf(msg,"opendir(%s) error",path_name);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_DIR_OPEN, errno, msg,__FILE__,__LINE__ -4);
	}
	struct dirent* dirp;
	struct stat buf;
	while((dirp = readdir(dp)) != NULL)	//����
	{
		if(strcmp(dirp->d_name,".") == 0 || strcmp(dirp->d_name,"..") == 0)
			continue;
		char path_YYYYMM[FILELEN];
		sprintf(path_YYYYMM, "%s/%s",path_name,dirp->d_name);
		if(lstat(path_YYYYMM, &buf)<0)
			continue;
		if(S_ISDIR(buf.st_mode))
		{
			DIR* dp_YYYYMM;
			dp_YYYYMM = opendir(path_YYYYMM);
			if(!dp_YYYYMM)
			{
				sprintf(msg,"opendir(%s) error",path_YYYYMM);
				throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_DIR_OPEN, errno, msg,__FILE__,__LINE__ -4);
			}
			struct dirent* dirp_YYYYMM;
			struct stat buf_YYYYMM;
			while((dirp_YYYYMM = readdir(dp_YYYYMM)) != NULL)
			{
				if(strcmp(dirp_YYYYMM->d_name,".") == 0 || strcmp(dirp_YYYYMM->d_name,"..") == 0)
					continue;
				char path_DD[FILELEN];
				sprintf(path_DD,"%s/%s",path_YYYYMM,dirp_YYYYMM->d_name);
				if(lstat(path_DD,&buf_YYYYMM)<0)
					continue;
				if(S_ISDIR(buf_YYYYMM.st_mode))
				{
					CF_CFscan scan;
					char* TMPFile = new char[FILELEN];
					memset(TMPFile, 0, FILELEN);
					scan.openDir(path_DD);
					while(!scan.getFile("*.TMP",TMPFile))
					{

//20080603 by zhoulh
                      if(!(strlen(szInCtlTabname)==0))
                      {
	                   char *char1;
                       char tmpfilename[256];
	                   char1 = strrchr(TMPFile,'/');
                       strcpy(tmpfilename,char1+2);
                       tmpfilename[strlen(tmpfilename)-4]=0;
                       CBindSQL ds(DBConn);
                       char szSqlTmp[2000];
                       int iFileCount;
                       sprintf(szSqlTmp, "select count(*) from %s where filename=:szPipeId ", szInCtlTabname);

                       ds.Open(szSqlTmp, SELECT_QUERY );
                       ds<<tmpfilename;
                       ds>>iFileCount;
                       ds.Close();

                       if (iFileCount <= 0)
                       {
                         continue;
                       }

                       char szsource_id[6];
                       sprintf(szSqlTmp, "select source_id from %s where filename=:szPipeId ", szInCtlTabname);

                       ds.Open(szSqlTmp, SELECT_QUERY );
                       ds<<tmpfilename;
                       ds>>szsource_id;
                       ds.Close();
                       int isourceIndex;
                       for(isourceIndex=0;isourceIndex<iSourceCount;isourceIndex++)
                       {
                         if(!strcmp(szsource_id,szSourceId[isourceIndex]))
                         break;
                       }
                       if(isourceIndex>=iSourceCount) continue;
                      }
//20080603 end by zhoulh
						FILE* fp_tmp;
						if((fp_tmp = fopen(TMPFile,"r")) == NULL)
						{
							sprintf(msg,"fopen(%s) error",TMPFile);
							throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_LOW,ERR_FILE_OPEN, errno, msg,__FILE__,__LINE__ -3);
						}
						fseek(fp_tmp, 0, SEEK_END);
						if(ftell(fp_tmp)<16)
						{
							if(fclose(fp_tmp))
							{
								sprintf(msg,"fclose(%s) error",TMPFile);
								throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_LOW,ERR_FILE_CLOSE, errno, msg,__FILE__,__LINE__ -3);	
							}
							if(remove(TMPFile))
							{
								sprintf(msg,"remove(%s) error\n",TMPFile);
								throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_REMOVE_FILE, errno, msg,__FILE__,__LINE__ -3);
							}
							memset(TMPFile, 0, FILELEN);
							continue;
						}
						if(fseek(fp_tmp,5,SEEK_SET) !=0 )
						{
							sprintf(msg,"fseek(%s,5,SEEK_SET) error\n",TMPFile);
							throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_UNKOWN, errno, msg,__FILE__,__LINE__ -3);
						}
						char sLen[11];
						memset(sLen, 0, 11);
						int Len;
						fread(sLen,10,1,fp_tmp);
						sscanf(sLen,"%d",&Len);
						if(Len==16)
						{
							if(fclose(fp_tmp))
							{
								sprintf(msg,"fclose(%s) error",TMPFile);
								throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_LOW,ERR_FILE_CLOSE, errno, msg,__FILE__,__LINE__ -3);	
							}
							if(remove(TMPFile))
							{
								sprintf(msg,"remove(%s) error\n",TMPFile);
								throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_REMOVE_FILE, errno, msg,__FILE__,__LINE__ -3);
							}
						}
						else
						{
							fseek(fp_tmp,0,SEEK_SET);
							char sbuf[COLLEN];
							if(fgets(sbuf, COLLEN, fp_tmp) == NULL)
							{
								printf("fgets %s error\n",sbuf);
								sprintf(msg,"read the file %s column error(1)\n",TMPFile);
								throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_UNKOWN, errno, msg,__FILE__,__LINE__ -4);
							}
							if(fgets(sbuf, COLLEN, fp_tmp) == NULL)
							{
								if(fclose(fp_tmp))
								{
									sprintf(msg,"fclose(%s) error(2)",TMPFile);
									throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_LOW,ERR_FILE_CLOSE, errno, msg,__FILE__,__LINE__ -3);	
								}
								if(remove(TMPFile))
								{
									sprintf(msg,"remove(%s) error\n",TMPFile);
									throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_REMOVE_FILE, errno, msg,__FILE__,__LINE__ -3);
								}
							}
							if(fclose(fp_tmp))
							{
								sprintf(msg,"fclose(%s) error",TMPFile);
								throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_LOW,ERR_FILE_CLOSE, errno, msg,__FILE__,__LINE__ -3);	
							}
							char* cc=sbuf;
							char* ccc;
							int dd=0;
							do{
								ccc = cc;
								if((cc=strchr(cc,';')) == NULL)
								{
									if(fclose(fp_tmp))
									{
										sprintf(msg,"fclose(%s) error",TMPFile);
										throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_LOW,ERR_FILE_CLOSE, errno, msg,__FILE__,__LINE__ -3);	
									}
									if(remove(TMPFile))
									{
										sprintf(msg,"remove(%s) error\n",TMPFile);
										throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_MID,ERR_REMOVE_FILE, errno, msg,__FILE__,__LINE__ -3);
									}
								}
								cc++;
								dd++;
							}while(dd<3);
							char sfilename[250];
							memset(sfilename, 0, 250);
							int ddd = cc - ccc - 1;
							strncpy(sfilename, ccc, ddd);
							sprintf(file_name,"%s",sfilename);
							sprintf(filename,"%s",sfilename);
							sprintf(temp_file,"%s",TMPFile);
							delete[] TMPFile;
							scan.closeDir();
							closedir(dp_YYYYMM);
							closedir(dp);
							return SUCCESS;
							
						}
						memset(TMPFile, 0, FILELEN);
					}
					scan.closeDir();
					delete[] TMPFile;	
				}
			}
			closedir(dp_YYYYMM);
		}
	}
	closedir(dp);
	return SEARCH_EOF;
}
/***************************************************
function
	int CF_CErrorLackRec::initToDB(abnormity_type type_id, int process_id, char* tablename);
description:
	��ʼ���쳣������⺯��
input:	
	type_id		�쳣�������ͣ���������쳣�����������ϣ������ص��ȣ�
	process_id	������̴���
	tablename	�쳣�������洢��Ӧ�ı���
output:
return
  SUCCESS              	����ִ�гɹ�
throw
*****************************************************/
int CF_CErrorLackRec::initToDB(abnormity_type type_id, int process_id, char* tablename)
{
	type = type_id;
	proc_id = process_id;
	sprintf(table,"%s",tablename);
	return SUCCESS;
}
/***************************************************
function
	int CF_CErrorLackRec::saveErrLackRecToDB(char* filename, int error_type, char* source_id, CFmt_Change* change)
description:
	�����쳣������¼���ȱ����ڱ��أ��洢��һ����Ŀ����һ�𱣴浽���ݿ⣩
input:	
	filename		�����������ĸ��ļ�
	error_type		�������ʹ���
	source_id		����Դ����
	change			�ļ���¼��ʽ���嵥��ʽ��
output:
return
  SUCCESS              	����ִ�гɹ�
  ERR_NOT_COMMIT		��û�д洢�ɹ����洢�ڱ��ص����ݼ�¼�����ȱ��浽���ݿ��ˡ�
  SUC_DO_COMMIT			�ɹ��������뱣�����ݵ����ݿ��ˡ�
  FAIL					ʧ��
throw
*****************************************************/
int CF_CErrorLackRec::saveErrLackRecToDB(char* filename, int error_type, char* source_id, CFmt_Change* change)
{
	if(LinkCount>=CURCOUNT)
		return ERR_NOT_COMMIT;
	if(!header)
	{
		header = new ErrorLackRec_Link;
		if(!header)
		{
			throw CF_CError('O','M',ERR_UNKOWN,errno,"���벻���ռ�",__FILE__,__LINE__ -3);
		}
		current = header;
	}
	else
	{
		current->next = new ErrorLackRec_Link;
		if(!current->next)
		{
			throw CF_CError('O','M',ERR_UNKOWN,errno,"���벻���ռ�",__FILE__,__LINE__ -3);
		}
		current = current->next;
	}
	sprintf(current->filename, "%s", filename);
	current->error_type = error_type;
	sprintf(current->source_id,"%s",source_id);
	current->change = *change;
	current->next = NULL;
	LinkCount++;
	if(LinkCount == CURCOUNT)
		return SUC_DO_COMMIT;
	return SUCCESS;
}

/***************************************************
function
	int CF_CErrorLackRec::CommitToDB();
description:
	�Ѵ���ڱ��ص��쳣������¼��⣬û�ж����ݿ�����ύ�������ݿ���ύ�ڽӿ�������У�
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
  FAIL					ʧ��
throw
*****************************************************/
int CF_CErrorLackRec::CommitToDB()
{
	//�ǵ��ж���������һ���쳣���͵�
	if(!header)
		return SUCCESS;
	if(type != abenum_error)
		return FAIL;
	char msg[MSGLEN];
	char scur[CURLEN];
	memset(scur, 0, CURLEN);

	/*sprintf(scur,"insert into %s(FILENAME,ERROR_TYPE,SOURCE_ID,FILE_ID,OFFSET,MESSAGEID",table);
	sprintf(scur,"%s,MSGTYPE,MSGDIRECTION,CHARGTERMID,CPID,SRCTERMID,DESTTERMID,SERVICEID,FEETYPE,FEECOM,SETTFEE_A,SETTFEE_B",scur);
	sprintf(scur,"%s,SMGNO,NXTNWKND,PRENWKND,RECVTIME,DONETIME,MSGID,MSGLENTH,PROCESS_ID,INVALID_FLAG,REDO_FLAG,ANARES)",scur);
	sprintf(scur,"%s values(:filename, :error_type, :source_id, :file_id, :offset, :message_id,"
	":msgtype, :msgdirection, :changtermid, :cpid, :stctermid, :desttermid, :serviceid, :feetype, :feecom, :settfee_a, :settfee_b,"
	":smgno, :nxtnwknd, :prenwknd, :recvtime, :donetime, :msgid, :msglenth, :process_id, :invalid_flag, :redo_flag, :anares)",scur);
	*/
	sprintf(scur,"insert into %s(FILENAME,ERROR_TYPE,SOURCE_ID,FILE_ID,OFFSET,MESSAGEID",table);
	sprintf(scur,"%s,MSGTYPE,MSGDIRECTION,CHARGTERMID,CPID,DESTTERMID,SERVICEID,FEETYPE,FEECOM,SETTFEE_A,SETTFEE_B",scur);
	sprintf(scur,"%s,SMGNO,FwdSMGNo,SMCNo,RECVTIME,DONETIME,MSGID,MSGLENTH,PROCESS_ID,INVALID_FLAG,REDO_FLAG,ANARES)",scur);
	sprintf(scur,"%s values(:filename, :error_type, :source_id, :file_id, :offset, :message_id,"
	":msgtype, :msgdirection, :changtermid, :cpid, :desttermid, :serviceid, :feetype, :feecom, :settfee_a, :settfee_b,"
	":smgno, :fwdno, :smcno, :recvtime, :donetime, :msgid, :msglenth, :process_id, :invalid_flag, :redo_flag, :anares)",scur);
	CBindSQL ds( DBConn );
        
        
        //char cntqry[CURLEN];
        
        //sprintf(cntqry,"select max(COL_INDEX) from TXTFILE_FMT where FILETYPE_ID='SMSSB'");
        //ds.Open(cntqry,SELECT_QUERY);
        //if (!(ds>>item_cnt))
        //{
        //        throw CF_CError('D','H',FAIL, ds.GetLastDBErrorCode(),"select max(COL_INDEX) from TXTFILE_FMT where FILETYPE_ID='SMSSB'",__FILE__,__LINE__ -3);
        //}
        //ds.Close();
        
	//ds.Open(scur,NONSELECT_DML);
	ErrorLackRec_Link* tmp = header;
	//add by wulei 2005-01-05
	int item_cnt=tmp->change.Get_fieldcount();
	char rec[LACKLEN];
	while(tmp)
	{
		ds.Open(scur,NONSELECT_DML);//��ʱ���������OCCI�޸ĺ����Ĺ�
		int file_id;
		int offset;
		char inte[10];
		memset(inte, 0, 10);
		sprintf(inte,"%s",tmp->change.Get_Field(1));
		sscanf(inte,"%d",&file_id);
		memset(inte, 0, 10);
		sprintf(inte,"%s",tmp->change.Get_Field(2));
		sscanf(inte,"%d",&offset);
		//write by wulei 2004-12-16
		//rewrite following the formation of the txtfile_fmt table
		//tmp->change.Get_record(rec,LACKLEN);
		//write by wulei 2005-01-05
		sprintf(rec,"%s",tmp->change.Get_Field(21));
		for (int i=22;i<item_cnt+1;i++)
		{
                        sprintf(rec,"%s;%s",rec,tmp->change.Get_Field(i));
                }
		/*sprintf(rec,"%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s",tmp->change.Get_Field(21),tmp->change.Get_Field(22),
		        tmp->change.Get_Field(23),
		        tmp->change.Get_Field(24),tmp->change.Get_Field(25),tmp->change.Get_Field(26),tmp->change.Get_Field(27),
		        tmp->change.Get_Field(28),tmp->change.Get_Field(29),tmp->change.Get_Field(30),tmp->change.Get_Field(31),
		        tmp->change.Get_Field(32),tmp->change.Get_Field(33),tmp->change.Get_Field(34),tmp->change.Get_Field(35),
		        tmp->change.Get_Field(36),tmp->change.Get_Field(37));
		*/
		if(ds<<tmp->filename<<tmp->error_type<<tmp->source_id<<file_id<<offset
			<<tmp->change.Get_Field(3)<<tmp->change.Get_Field(4)<<tmp->change.Get_Field(5)
			<<tmp->change.Get_Field(6)<<tmp->change.Get_Field(7)<<tmp->change.Get_Field(8)
			<<tmp->change.Get_Field(9)<<tmp->change.Get_Field(10)<<tmp->change.Get_Field(11)
			<<tmp->change.Get_Field(12)<<tmp->change.Get_Field(13)<<tmp->change.Get_Field(14)
			<<tmp->change.Get_Field(15)<<tmp->change.Get_Field(16)<<tmp->change.Get_Field(17)
			<<tmp->change.Get_Field(18)<<tmp->change.Get_Field(19)<<tmp->change.Get_Field(20)
			<<proc_id<<'Y'<<'N'<<rec)
		//end write
			{
				ds.Execute();//��Ϊ�ӿ��������Բ�����һ�β�����У��Ժ�����޸�
				if(ds.IsError())
				{
					throw CF_CError('D','H',FAIL, ds.GetLastDBErrorCode(),"���α�д�����ݿ��ǳ���",__FILE__,__LINE__ -3);
				}
				ds.Close();//�ر�����
			}
		tmp = tmp->next;
	}

	tmp = header;
	while(header)
	{
		header = header->next;
		delete tmp;
		tmp = header;
	}
	current = NULL;
	LinkCount = 0;
	return SUCCESS;
}
/***************************************************
function
	int CF_CErrorLackRec::RollBackToDB();
description:
	�Ѵ��ϴα����洢�����ݼ�¼ɾ��
input:	
output:
return
  SUCCESS              	����ִ�гɹ�
throw
*****************************************************/
int CF_CErrorLackRec::RollBackToDB()
{
	ErrorLackRec_Link* tmp = header;
	while(header)
	{
		header = header->next;
		delete tmp;
		tmp = header;
	}
	current = NULL;
	LinkCount = 0;
	return SUCCESS;	
}
