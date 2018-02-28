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
		//��ѯconfig_id��Ӧ�ı���
		stmt = conn.createStatement();
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select Table_Name from C_STAT_TABLE_DEFINE where config_id=%d",config_id);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>m_SStat_Table.SzTable_Name;
		
		//��ѯͳ�Ʊ���ֶζ�Ӧ��ʽ
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
			sprintf(erro_msg,"init() ͳ�Ʊ���ֶζ�Ӧ����%d����������%d",m_SStat_Table.NStat_Item_Count,STAT_ITEM_COUNT);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

			return false;
		}
		
		//��ѯͳ�Ʊ��Ӧ���ֶι�ϵ
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
		//��ʼ���ļ���ʽ
		outrcd.Init(m_szOutTypeId);	
		
		stmt.close();
		conn.close();
	}
	catch(SQLException e)
	{
  		conn.close();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ���ݿ����%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	}

	 char sKeyName[100];
	 CString sKeyVal;
	 IBC_ParamCfgMng param_cfg;	//���Ĳ�����ȡ��

	//2013-08-19 ����sql·��,����дsql�ļ�,��־·��,��־����
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
			cerr<<"SQLĿ¼:"<<sql_path<<"��ʧ��"<<endl;
			return false ;
		 }else  closedir(dirptr);

	}	

	//cout<<"ok..."<<endl;
	return true;
}

//2013-11-16����file_id,����ɾ��
int CF_CNewError_Table::setConfig(long file_id,char* rate_cycle)
{
	mFileid = file_id;
	memset(mRateCycle,0,sizeof(mRateCycle));
	strcpy(mRateCycle,rate_cycle);

	return 1;
}

//2013-12-06 ������дsql�ļ��Ĳ����ŵ��ýӿ���
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

//����¼ֵ���뵽��Ӧ���ֶ���ȥ
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
	if((m_count%COMMIT_COUNT) == 0)			//ÿ��500���ύ
	{
		//theJSLog<<"�ύ����:"<<m_count<<endi;
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
		//cout<<"���ڴ�sqlд��ʱ�ļ�:"<<tmp<<endl;
		ofstream sqlout;

		char tmp[512];
		memset(tmp,0,sizeof(tmp));

		strcpy(tmp,mFullSqlName);
		strcat(tmp,".tmp");
		
		//cout<<"д��ʱsql�ļ�"<<tmp<<" "<<m_count<<endl;
		//sleep(10);

		if(m_count == 0)	//���оͲ�д�ļ�,�п����ļ�������
		{
			return 0;
		}

		sqlout.open(tmp,ios::app);
		if(!sqlout)
		{
			cerr<<"�ļ�["<<tmp<<"]��ʧ��! �޷�д��sql���ļ�"<<endl;
			return -1;
		}
		
		for(int i = 0;i<m_count;i++)
		{
			sqlout<<vsql[i]<<endl;
		}
		sqlout.close();
		
		//cout<<"��ʱsql�ļ�����"<<mFullSqlName<<endl;
		//sleep(30);

		delete[] vsql;			//�ͷ��ڴ�
		vsql = NULL;

		//rename(tmp,mFullSqlName);	�ò�����process.cppȥ����ʱ�ļ�����
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

			if(m_count >= COMMIT_COUNT)			//�ع� ɾ���ύ������
			{
				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from %s_%s where FILEID = %ld",m_SStat_Table.SzTable_Name,mRateCycle,mFileid);
				theJSLog<<"ɾ��ǰ���ύ������... sql="<<sql<<endi;
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
		//vector<string>().swap(vsql);	//2013-12-05 vector<int>().swap(nums)����nums.swap(vector<int>())
	}

	m_count = 0;

	return 1;
}

//ƴ��sql
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
