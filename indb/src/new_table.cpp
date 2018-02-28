#include "new_table.h"

//vector<string>  vsql;
DBConnection m_conn;

CF_CNewError_Table::CF_CNewError_Table()
{
	mRWFlag = true;
	m_count = 0;
	mFileid = -1;
	memset(m_szOutTypeId,0,sizeof(m_szOutTypeId));
	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));

	vsql = NULL;
}

CF_CNewError_Table::~CF_CNewError_Table()
{

}

bool CF_CNewError_Table::init(int config_id,char* OutTypeId)
{
	int ret = 0;
	
	DBConnection conn;
	if(!dbConnect(conn))
	{
		return false;
	}
		
	//string sql;
	Statement stmt ;
	try
	{
		//查询config_id对应的表名
		stmt = conn.createStatement();
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select Table_Name from C_STAT_TABLE_DEFINE where config_id=%d",config_id);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>m_SStat_Table.SzTable_Name;
		
		//查询统计表的字段对应格式
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*) from C_STAT_TABLE_FMT where config_id=%d",config_id);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>m_SStat_Table.NStat_Item_Count;
		
		theJSLog<<"SzTable_Name="<<m_SStat_Table.SzTable_Name<<"  NStat_Item_Count="<<m_SStat_Table.NStat_Item_Count<<endi;

		if(m_SStat_Table.NStat_Item_Count > STAT_ITEM_COUNT)
		{
			conn.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() 统计表的字段对应个数%d大于最大个数%d",m_SStat_Table.NStat_Item_Count,STAT_ITEM_COUNT);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

			return false;
		}
		
		//查询统计表对应的字段关系
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select Table_Item,Item_Type,Field_Name,Field_Begin,Field_End from C_STAT_TABLE_FMT where config_id=:1",config_id);
		stmt.setSQLString(sql);
		stmt<<config_id;
		stmt.execute();

		for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
		{
			stmt>>m_SStat_Table.SzStat_Item[i]>>m_SStat_Table.NItem_Type[i]>>m_SStat_Table.SzStat_FieldName[i]
				>>m_SStat_Table.NStat_Item_Begin[i]>>m_SStat_Table.NStat_Item_End[i];
		}
		
		memset(m_SStat_Table.NSql,0,sizeof(m_SStat_Table.NSql));
		//sprintf(m_SStat_Table.NSql,"insert into %s(",m_SStat_Table.SzTable_Name);
		strcpy(m_SStat_Table.NSql,"(");
		for(int i = 0;i<m_SStat_Table.NStat_Item_Count;i++)
		{
			sprintf(m_SStat_Table.NSql,"%s %s,",m_SStat_Table.NSql,m_SStat_Table.SzStat_Item[i]);	
		}

		//cout<<"m_SStat_Table.NSql = "<<m_SStat_Table.NSql<<endl;

		m_SStat_Table.NSql[strlen(m_SStat_Table.NSql)-1] = '\0';
		strcat(m_SStat_Table.NSql,") values");

		theJSLog<<"m_SStat_Table.NSql = "<<m_SStat_Table.NSql<<endi;
		
		memset(m_szOutTypeId,0,sizeof(m_szOutTypeId));
		strcpy(m_szOutTypeId,OutTypeId);
		//初始化文件格式
		outrcd.Init(m_szOutTypeId);	
		
		stmt.close();
		conn.close();
	}
	catch(SQLException e)
	{
  		conn.close();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 数据库出错：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	}

	 char sKeyName[100];
	 CString sKeyVal;
	 IBC_ParamCfgMng param_cfg;	//核心参数读取类

	//2013-08-19 新增sql路径,用来写sql文件,日志路径,日志级别
	sprintf(sKeyName, "sql.path");
	if(param_cfg.bGetMem(sKeyName, sKeyVal))
	{
		memset(sql_path,0,sizeof(sql_path));
		strcpy(sql_path,(const char*)sKeyVal);
		if (sql_path[strlen(sql_path) - 1] != '/')
		{
			strcat(sql_path, "/");
		}
		
		//theJSLog<<"sql_path:"<<sql_path<<endi;

		DIR *dirptr = NULL;
	    if((dirptr=opendir(sql_path)) == NULL)
		{		
			cerr<<"SQL目录:"<<sql_path<<"打开失败"<<endl;
			return false ;
		 }else  closedir(dirptr);

	}	

	//cout<<"ok..."<<endl;
	return true;
}

//2013-11-16新增file_id,便于删除
int CF_CNewError_Table::setConfig(long file_id,char* rate_cycle)
{
	mFileid = file_id;
	memset(mRateCycle,0,sizeof(mRateCycle));
	strcpy(mRateCycle,rate_cycle);

	return 1;
}

//2013-12-06 新增将写sql文件的操作放到该接口中
int CF_CNewError_Table::setSQLconf(char* name,int count)
{
	memset(mFullSqlName,0,sizeof(mFullSqlName));
	strcpy(mFullSqlName,sql_path);
	strcat(mFullSqlName,name);

	vsql = new string[count];

	return 1;
}

int CF_CNewError_Table::setBegin(DBConnection& conn)
{
	setRWFlag(true);
	m_conn = conn;
	stmt = conn.createStatement();
	
	return 1;
}

int CF_CNewError_Table::setRWFlag(bool flag)
{
	mRWFlag = flag;
	
	m_count = 0;
	
	return 1;
}

//将记录值插入到对应的字段中去
bool CF_CNewError_Table::insertData(char* data)
{
	char sql2[2048];
try
{
	outrcd.Set_record(data);
	char value[256];
	memset(sql,0,sizeof(sql));
	strcpy(sql,"(");

	for(int i = 0;i<m_SStat_Table.NStat_Item_Count;i++)
	{	
		memset(value,0,sizeof(value));
		outrcd.Get_Field(m_SStat_Table.SzStat_FieldName[i],value);
		if(m_SStat_Table.NItem_Type[i] == 2)
		{
			sprintf(sql,"%s'%s',",sql,value);
		}
		else
		{
			sprintf(sql,"%s%s,",sql,value);
		}
	}
	sql[strlen(sql)-1] = '\0';
	strcat(sql,")");
	
	memset(sql2,0,sizeof(sql2));
	sprintf(sql2,"insert into %s_%s%s%s",m_SStat_Table.SzTable_Name,mRateCycle,m_SStat_Table.NSql,sql);
	//sprintf(sql2,"%s%s",m_SStat_Table.NSql,sql);
	
	m_count++;

	stmt.setSQLString(sql2);
	stmt.execute();
	if((m_count%COMMIT_COUNT) == 0)			//每隔500条提交
	{
		//theJSLog<<"提交条数:"<<m_count<<endi;
		stmt.commit();
		stmt.close();
		stmt = m_conn.createStatement();
	}

}catch (jsexcp::CException e)
{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"insertData error %s ",e.GetErrMessage());
		//theJSLog.writeLog(0,erro_msg);

		throw jsexcp::CException(0,erro_msg,__FILE__,__LINE__);

		return false;
}
catch(SQLException e)
{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"insertData() sql error %s (%s)",e.what(),sql2);
		//theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);

		throw jsexcp::CException(0,erro_msg,__FILE__,__LINE__);
		return false;
}
	

	return true;
}

int CF_CNewError_Table::Commit()
{
	if(mRWFlag)
	{
		stmt.commit();
		stmt.close();
	}
	else
	{
		//cout<<"将内存sql写临时文件:"<<tmp<<endl;
		ofstream sqlout;

		char tmp[512];
		memset(tmp,0,sizeof(tmp));

		strcpy(tmp,mFullSqlName);
		strcat(tmp,".tmp");
		
		//cout<<"写临时sql文件"<<tmp<<" "<<m_count<<endl;
		//sleep(10);

		if(m_count == 0)	//咩有就不写文件,有可能文件不存在
		{
			return 0;
		}

		sqlout.open(tmp,ios::app);
		if(!sqlout)
		{
			cerr<<"文件["<<tmp<<"]打开失败! 无法写入sql到文件"<<endl;
			return -1;
		}
		
		for(int i = 0;i<m_count;i++)
		{
			sqlout<<vsql[i]<<endl;
		}
		sqlout.close();
		
		//cout<<"临时sql文件改名"<<mFullSqlName<<endl;
		//sleep(30);

		delete[] vsql;			//释放内存
		vsql = NULL;

		//rename(tmp,mFullSqlName);	该步骤有process.cpp去改临时文件到正
	}

	m_count = 0;

	return 1;
}

int CF_CNewError_Table::RollBack()		
{
	if(mRWFlag)
	{
		try
		{
			stmt.rollback();

			if(m_count >= COMMIT_COUNT)			//回滚 删除提交的数据
			{
				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from %s_%s where FILEID = %ld",m_SStat_Table.SzTable_Name,mRateCycle,mFileid);
				theJSLog<<"删除前面提交的数据... sql="<<sql<<endi;
				stmt.setSQLString(sql);
				stmt.execute();
			}

			stmt.close();

		}catch(SQLException e)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"RollBack() sql error %s (%s)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
			//throw jsexcp::CException(0,erro_msg,__FILE__,__LINE__);
		}
	}
	else
	{
		m_count = 0;
		if(vsql)  
		{
			delete[] vsql;
			vsql = NULL;
		}

		//vsql.clear();
		//vector<string>().swap(vsql);	//2013-12-05 vector<int>().swap(nums)或者nums.swap(vector<int>())
	}

	m_count = 0;

	return 1;
}

//拼接sql
void CF_CNewError_Table::writeSQL(char* data)
{
	char sql2[2048];
try
{
	outrcd.Set_record(data);
	char value[256];
	memset(sql,0,sizeof(sql));
	strcpy(sql,"(");

	for(int i = 0;i<m_SStat_Table.NStat_Item_Count;i++)
	{	
		memset(value,0,sizeof(value));
		outrcd.Get_Field(m_SStat_Table.SzStat_FieldName[i],value);
		if(m_SStat_Table.NItem_Type[i] == 2)
		{
			sprintf(sql,"%s'%s',",sql,value);
		}
		else
		{
			sprintf(sql,"%s%s,",sql,value);
		}
	}
	sql[strlen(sql)-1] = '\0';
	strcat(sql,")");
	
	memset(sql2,0,sizeof(sql2));
	//sprintf(sql2,"%s%s",m_SStat_Table.NSql,sql);
	sprintf(sql2,"insert into %s_%s%s%s",m_SStat_Table.SzTable_Name,mRateCycle,m_SStat_Table.NSql,sql);
	
	vsql[m_count] = "";
	vsql[m_count].append(sql2) ;
	m_count++;
	//vsql.push_back(sql2);

}catch (jsexcp::CException e)
{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"writeSQL error %s ",e.GetErrMessage());
		//theJSLog.writeLog(0,erro_msg);

		throw jsexcp::CException(0,erro_msg,__FILE__,__LINE__);
}

}

//vector<string> CF_CNewError_Table::getvSQL()
//{	
//	return vsql;
//}
