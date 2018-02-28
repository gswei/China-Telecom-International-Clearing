/*
* Copyright (c) 2003, 广东普信科技有限公司
* All rights reserved.
*
* filename：CF_CErrorLackRec.cpp
* version：1.1.2
* update list: 
* discription：修改错单中ANARES字段，只存放业务属性分析的结果
*/
//无资料临时文件名改为YYYYMMDDhhmmssYYYYMM
//20060809   修改文件出错在Rollback中没有清空count的错误，改为在open中初始化
//20080603 by zhoulh 删除临时文件只删除该流水的

#include "CF_CErrorLackRec.h"
//#include "CF_CError.h"
//#include "CF_CFscan.h"

CF_CErrorLackRec::CF_CErrorLackRec()
{
	fp = NULL;
	Count = 0;
	proc_id = -1;
	type = abenum_lackinfo;		//默认为无资料类型处理
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
		/*这里没有提交操作,管理员看到有临时文件可以再次提交*/
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
	写入异常话单头记录
input:	
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_FILE_WRITE 	文件写入出错
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
	char s_len[10 + 1];               //总字节数10位
	sprintf(s_len, "%02d", 6);	  //话单字段记录部分的字段数,记录标准话单那个记录开始
	if((r = fwrite(s_len, 2, 1,fp)) != 1)
	{
		sprintf(msg,"Write temporary file %s header note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	sprintf(s_len,"%010d\n",16);      //头记录16位(包括'\n')
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
	写入异常话单数据源记录
input:	
	id		本异常话单的话单类型（比如：无资料话单的无资料类型）
	filename	本话单出自哪个文件
	change		保存话单记录的类
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_GET_RECORD	读取change的数据出错
  ERR_FILE_WRITE 	文件写入出错
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
	写入异常话单尾记录
input:	
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_UNKOWN		出现未知错误
  ERR_FILE_WRITE 	文件写入出错
*****************************************************/
int CF_CErrorLackRec::process_end()
{
	char msg[MSGLEN];
	if(fwrite("wEND",4,1,fp) != 1)
	{
		sprintf(msg,"Write temporary file %s end note error\n",temp_file);
		throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,ERR_FILE_WRITE, errno, msg,__FILE__,__LINE__ -3);
	}
	char ch[10];				//总记录数6位
	sprintf(ch,"%06d",Count);
	if(fwrite(ch,6,1,fp) != 1)		//总记录6位加'\n'一位
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
	初始化异常话单接口
input:	
	rootpath	异常话单文件根目录
	type_id		异常话单类型（所处理的异常话单是无资料，错单，重单等）
	process_id	处理过程代码
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_DIR_CREATE	生成(子)目录失败
  ERR_DIR_CHANGE	无法进入(子)目录
  ERR_DIR_NULLITY	无效的根目录
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
	初始化异常话单接口
input:	
	rootpath	异常话单文件根目录
	type_id		异常话单类型（所处理的异常话单是无资料，错单，重单等）
	process_id	处理过程代码
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_DIR_CREATE	生成(子)目录失败
  ERR_DIR_CHANGE	无法进入(子)目录
  ERR_DIR_NULLITY	无效的根目录
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
	}	//提示自己*****重单的生成方式的路径命名不是这个格式！记得看字典*****
	return SUCCESS;
  }
  return SUCCESS;
}
/***************************************************
function
	int CF_CErrorLackRec::Open()
description:
	打开一个新的异常话单临时文件
input:	
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_UNKOWN		出现未知错误
  ERR_FILE_OPEN 	文件创建或打开时为空
  ERR_DIR_CREATE	生成(子)目录失败
  ERR_DIR_CHANGE	无法进入(子)目录
  ERR_DIR_NULLITY	无效的根目录
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
	打开一个新的异常话单临时文件
input:	
	id		异常话单类型, 如果是无资料话单则是无资料类型，如果是错单则是错单类型；如果是重单，则是重单类型。
	filename	本话单出自哪个文件
	change		保存话单记录的类
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_NOT_COMMIT	前一个话单文件处理完成，但没有提交或回退
  ERR_UNKOWN		出现未知错误
  ERR_FILE_OPEN 	文件创建或打开时为空
  ERR_DIR_CREATE	生成(子)目录失败
  ERR_DIR_CHANGE	无法进入(子)目录
  ERR_DIR_NULLITY	无效的根目录
  ERR_GET_RECORD	读取change的数据出错
  ERR_FILE_WRITE 	文件写入出错
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
	关闭异常话单文件
input:	
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_UNKOWN		出现未知错误
  ERR_FILE_CLOSE 	关闭文件失败
  ERR_FILE_WRITE 	写头，尾记录出错
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
	提交异常话单文件（将临时文件名改为正式文件名）
	注意:在CF_CErrorLackRec::proc_TMPFile(char* filename)过程中提交的查找到的filename
input:	
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
  DO_CLOSE_FIRST	成功，但在提交前，自动作了关闭文件操作(Close)
throw
  ERR_UNKOWN		出现未知错误
  ERR_FILE_CLOSE 	关闭文件失败
  ERR_FILE_WRITE 	写头，尾记录出错
  ERR_RENAME_FILE	将临时文件名改为正式文件名时出错
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
	回退当前异常话单文件(把临时文件删除)
	注意:在CF_CErrorLackRec::Commit(char* filename)过程中删除的是查找到的filename
input:	
output:
return
  SUCCESS              	函数执行成功
  FAIL			函数执行失败
throw
  ERR_FILE_CLOSE	关闭文件时出错
  ERR_REMOVE_FILE	删除临时文件时出错
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
	检查异常话单文件(*.TMP),把处理过程不正常(没有尾记录)的文件删除,把正常处理过的文件改名为正式
	注意：此函数的调用只能在CF_CErrorLackRec::init()之后,必须处理函数输出
input:	
output:
  filename				查找到的话单文件名
return
  SUCCESS              	函数执行成功
  FAIL					函数执行失败
throw
  ERR_DIR_OPEN		打开工作目录失败
  ERR_FILE_OPEN		文件打开时为空
  ERR_FILE_CLOSE	关闭文件时出错
  ERR_REMOVE_FILE	删除临时文件时出错
  ERR_RENAME_FILE	将临时文件名改为正式文件名时出错
  ERR_UNKOWN		未知错误
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
	while((dirp = readdir(dp)) != NULL)	//年月
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
	初始化异常话单入库函数
input:	
	type_id		异常话单类型（所处理的异常话单是无资料，错单，重单等）
	process_id	处理过程代码
	tablename	异常类型如库存储对应的表名
output:
return
  SUCCESS              	函数执行成功
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
	保存异常话单记录（先保存在本地，存储到一定数目后再一起保存到数据库）
input:	
	filename		本话单出自哪个文件
	error_type		错误类型代码
	source_id		数据源代码
	change			文件记录格式（清单格式）
output:
return
  SUCCESS              	函数执行成功
  ERR_NOT_COMMIT		（没有存储成功）存储在本地的数据记录必须先保存到数据库了。
  SUC_DO_COMMIT			成功，但必须保持数据到数据库了。
  FAIL					失败
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
			throw CF_CError('O','M',ERR_UNKOWN,errno,"申请不到空间",__FILE__,__LINE__ -3);
		}
		current = header;
	}
	else
	{
		current->next = new ErrorLackRec_Link;
		if(!current->next)
		{
			throw CF_CError('O','M',ERR_UNKOWN,errno,"申请不到空间",__FILE__,__LINE__ -3);
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
	把存放在本地的异常话单记录入库，没有对数据库进行提交，对数据库的提交在接口外面进行！
input:	
output:
return
  SUCCESS              	函数执行成功
  FAIL					失败
throw
*****************************************************/
int CF_CErrorLackRec::CommitToDB()
{
	//记得判断是属于那一个异常类型的
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
		ds.Open(scur,NONSELECT_DML);//暂时放在这里，等OCCI修改后必须改过
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
				ds.Execute();//因为接口问题所以不可以一次插入多行，以后必须修盖
				if(ds.IsError())
				{
					throw CF_CError('D','H',FAIL, ds.GetLastDBErrorCode(),"打开游标写入数据库是出错",__FILE__,__LINE__ -3);
				}
				ds.Close();//关闭连接
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
	把从上次保存后存储的数据记录删除
input:	
output:
return
  SUCCESS              	函数执行成功
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
