#ifndef __SUM_COMMON_CPP__
#define __SUM_COMMON_CPP__

#include "Common.h"

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <list>
#include <set>

#include "CF_CException.h" 

//CLog theJSLog;

int getItemInfo(SCom szSCom,  vector<SItemPair> &vItemInfo)
{
   char sqltmp[SQL_LEN+1];
   int iconfig,dconfig;
   iconfig = szSCom.iOrgSumt;
   dconfig = szSCom.iDestSumt;
   int noitem;
   
   DBConnection conn;//���ݿ�����
   try{			
	if (dbConnect(conn))
	 {
	     Statement stmt = conn.createStatement(); 
	     //�ȼ���Ƿ�������
	     sprintf(sqltmp,"select count(*) from C_STAT_TABLE_FMT where config_id = %d ",iconfig);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
	     stmt>>noitem;     
		 if(noitem == 0)
		 {
			cout<<"ԭ��["<<szSCom.iOrgTableName<<"]û�������ֶε�C_STAT_TABLE_FMT"<<endl;
			return -1 ;
		 }
		 sprintf(sqltmp,"select count(*) from C_STAT_TABLE_FMT where config_id = %d ",dconfig);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
	     stmt>>noitem;
		 if(noitem == 0)
		 {
			cout<<"Ŀ���["<<szSCom.iDestTableName<<"]û�������ֶε�C_STAT_TABLE_FMT"<<endl;
			return -1;
		 }
		 
		 //ȫ����
	     sprintf(sqltmp,"select a.table_item,a.item_type,a.field_begin,a.field_end ,b.table_item,b.item_type,b.field_begin,b.field_end from (select * from C_STAT_TABLE_FMT where config_id = %d ) a full join (select * from C_STAT_TABLE_FMT b where config_id = %d) b on a.seq = b.seq",iconfig,dconfig);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
		 SItemPair item ;
	  
	    while(stmt>>item.fromItem.szItemName>>item.fromItem.iItemType>>item.fromItem.iBeginPos>>item.fromItem.iEndPos
			 >>item.toItem.szItemName>>item.toItem.iItemType>>item.toItem.iBeginPos>>item.toItem.iEndPos)
		{
				vItemInfo.push_back(item);
		}

		//cout<<"vItemInfo.size() = "<<vItemInfo.size()<<endl;

		stmt.close();
		conn.close();
	     
	     
	 }else
	 {
  	 	cout<<"connect error."<<endl;
  	 	return -1;
	 }
	    conn.close();

		return 0;

	 }catch( SQLException e )
	 {
  		cout<<e.what()<<endl;
  		theJSLog << "getItemInfo ���ݿ����" <<e.what()<< endi;
  		//throw jsexcp::CException(0, "ɾ�����ݿ����", __FILE__, __LINE__);
  		conn.close();
  		return -1;
     }
	 
  //���ݻ����������ṹ���vector
  //��ȡ��Ҫ�������������ԴID
  //��ȡ����ԴID��Ӧ��ԭʼ��configid��Ŀ���configid�����szSCom
  //��������ԴID��ȡԴͷ��ṹ��Ŀ���ṹ�����vItemInfo
}
int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char *sql)
{
  //ͨ������vItemInfo��������ͳ���ͳ��ά�ȡ�����ͳ��ֵ
  //where������szOrgSumtCol=fromDateValue
  //group by ͳ��ά��
  //����sql
  //0Ϊ�ɹ���-1Ϊʧ�ܡ�
 
 try
 {

  memset(sql,0,sizeof(sql));
  sprintf(sql,"insert into %s (",szSCom.iDestTableName);
  string sqlA1,sqlA2,sqlB1,sqlB2;				//�ֱ����� ԭ��ͳͳ��ά�ȣ�ԭ��ͳ���Ŀ���ͳ����
  char currTime[15],time_flag;
 
  char time_col[20];
  memset(time_col,0,sizeof(time_col));

  //sqlA1.append("select  "); 
  for(int i = 0;i<vItemInfo.size();i++)		//��ѯ���ͳ����ͳ��ά�ȵ�
  {
	SItem fromItem = vItemInfo[i].fromItem;
	SItem toItem = vItemInfo[i].toItem;
	
	if(strcmp("",toItem.szItemName))							//Ŀ�������ͳ��ά��
	{
		switch(toItem.iItemType)		//����ͳ�����ͳ��ά�ȣ���Ϊ�˱�֤ԭ���Ŀ����ֶζ���
		{
			case 0:							
				//break;
				sqlB2.append(toItem.szItemName).append(",");
				break;
			case 1:						//����ͳ��ά��
				//break ;
			case 2:						
				sqlB1.append(toItem.szItemName).append(",");
				break ;
			case 12:					//��־ȡ��ǰʱ��
				time_flag = 'Y';
				strcpy(time_col,toItem.szItemName);  //����ʱ���ֶ�
				break;
			default :
				break ;
		}
	}
	if(strcmp("",fromItem.szItemName))					//ԭ���ֶ�����
	{
		switch(fromItem.iItemType)
		{
			case 0:					//��־ͳ����
			    sqlA2.append(" sum(").append(fromItem.szItemName).append("),");
				break;
			case 1:					//����ͳ��ά��
				//break ;
			case 2:					//�ַ���ͳ��ά��
				sqlA1.append(fromItem.szItemName).append(",");
				break ;
			case 12:
				//??????????????????
				break;
			default :
				break ;
		}
	}
  }
  if(time_flag == 'Y')
  {
		sqlB2.append(time_col) ; 							  //��ӵ�Ŀ����ʱ���ջ��ܱ���������ֶ�����ֵ��д���ܵ�����
		sqlA2.append("to_char(sysdate,'yyyymmddhh24miss')");  //ȡ���ݿ�ϵͳʱ��
  }
  else
  {
	sqlB2 = sqlB2.substr(0, sqlB2.length()-1);  //���Ŀ����ֶ�
	sqlA2 = sqlA2.substr(0, sqlA2.length()-1);  //���ԭʼ��ͳ����sum()
  }
  
  //sqlB1 = sqlB1.substr(0, sqlB1.length()-1);
  sqlA1 = sqlA1.substr(0, sqlA1.length()-1);  //���ԭʼ��ͳ��ά��
 
  sprintf(sql,"%s%s%s) select %s,%s from %s where %s like '%s%s' group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,szSCom.szOrgSumtCol,fromDateValue,"%",sqlA1);
  
  

 }catch(jsexcp::CException e)
 {
	char tmp[1024];
	memset(tmp,0,sizeof(tmp));
	sprintf("getSql()  sqlƴ��ʧ��: %s",e.GetErrMessage());
	theJSLog.writeLog(0,tmp);
	return -1;
 } 

  return 0;
}

// ��������ƴSQL���ɾ����Ӧ����
bool redo(char *sumdate,char *tablename,char*sumitem)
{
   char sqltmp[SQL_LEN+1];
   int inum;
   DBConnection conn;//���ݿ�����
   try{			
	if (dbConnect(conn))
	 {
	     Statement stmt = conn.createStatement(); 
	     //�ȼ���Ƿ�������
	      sprintf(sqltmp,"select count(*) from %s where %s = '%s' ",tablename,sumitem,sumdate);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
	     stmt>>inum;
	     if(inum>0)
	     {
	        theJSLog<< "����ɾ�����ݿ���ڵ�����" <<endi;
	        return false;
	     }else
	     {
	       //ɾ������
	       sprintf(sqltmp,"delete from %s where %s = '%s' ",tablename,sumitem,sumdate);
           stmt.setSQLString(sqltmp);	
	       stmt.execute();  
	     }
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "ɾ�����ݿ����" << endi;
  		throw jsexcp::CException(0, "ɾ�����ݿ����", __FILE__, __LINE__);
  		conn.close();
  		return false;
     }
	 return true; 
}

//ֱ�ӽ�sql �����뵽����
int insertSql(char *sql)
{
   DBConnection conn;//���ݿ�����
   //int ret = 0;
   try
   {			
		if (dbConnect(conn))
		{
			Statement stmt = conn.createStatement();        
			stmt.setSQLString(sql);	
			stmt.execute();    
			stmt.close();
		}
		else
		{
  	 		cout<<"connect error."<<endl;
  	 		return 0;
		}

		conn.close();
	 }
	 catch( SQLException e ) 
	 {
		char tmp[1024];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"�������ݿ����: %s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,tmp);

  		//theJSLog << "�������ݿ����"<<e.what()<< endi;
  		//throw jsexcp::CException(0, "�������ݿ����", __FILE__, __LINE__);
  		conn.close();
  		return 0;
     }

	 //cout<<"�����¼������"<<ret<<endl;

	 return 1 ; //
}


#endif
