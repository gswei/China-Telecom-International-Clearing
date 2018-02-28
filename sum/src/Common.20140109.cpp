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
		 
		 //��������Ҫ��Ŀ��������
	     sprintf(sqltmp,"select a.table_item,a.field_name,a.item_type,a.field_begin,a.field_end from C_STAT_TABLE_FMT a where a.config_id = %d order by a.seq ",dconfig);
         stmt.setSQLString(sqltmp);	
	     stmt.execute(); 
		 SItemPair item ;
	  
	    while(stmt>>item.toItem.szItemName>>item.fromItem.szItemName>>item.toItem.iItemType>>item.toItem.iBeginPos>>item.toItem.iEndPos)
		{
				if(item.toItem.iBeginPos > 0)
				{
					item.toItem.iBeginPos--;
					item.toItem.iEndPos = item.toItem.iEndPos - item.toItem.iBeginPos;	//��ʾ����
					//char tmp[50];
					//memset(tmp,0,sizeof(tmp));
					//sprintf(tmp,"SUBSTR(%s,%d,%d)",item.fromItem.szItemName,item.fromItem.iBeginPos,item.toItem.iEndPos);
					//strcpy(item.fromItem.szItemName,tmp);
					//cout<<"item.fromItem.szItemName = "<<item.fromItem.szItemName<<endl;
				}
				vItemInfo.push_back(item);
		}

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

/**
  ����˵��
  1 sql�����ص�sql���
  2 type�� 1 ��ʾ�ջ���,2�»���,3̯�ֻ���
  3 fromDateValue ���ջ��ܱ�ʾͳ�Ƶ�����,�»���̯�ֻ��ܱ�ʾ����,Ҫͳ�Ƶ������ڱ�
  4 rate_cycle ���ջ�������,��ʾ�ջ��ܵı�
  5 position  ���̯�ֻ���,��ʾҪͳ�Ƶ����config_id��ʱָ����λ��
*/

int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char *sql,int type,char* rate_cycle,int position)
{
  //ͨ������vItemInfo��������ͳ���ͳ��ά�ȡ�����ͳ��ֵ
  //where������szOrgSumtCol=fromDateValue
  //group by ͳ��ά��
  //����sql
  //0Ϊ�ɹ���-1Ϊʧ�ܡ�
 
 try
 {

  memset(sql,0,sizeof(sql));
  if(type == 1)		//�ջ���,�ջ��ܵ�ԭʼ��û�к�׺����
  {
  	  sprintf(sql,"insert into %s_%s (",szSCom.iDestTableName,rate_cycle);
  }
  else				//�»���,̯�ֻ���
  {
	  if((type == 3) && (szSCom.count > 1) )
	  {
		//cout<<"xxx: "<<szSCom.mscontion[position].iDestTableName<<endl;
		sprintf(sql,"insert into %s_%s (",szSCom.mscontion[position].iDestTableName,fromDateValue);
	  }
  	  else
	  {
		  sprintf(sql,"insert into %s_%s (",szSCom.iDestTableName,fromDateValue);
	  }
  }

  string sqlA1,sqlA2,sqlB1,sqlB2;				//�ֱ����� ԭ��ͳͳ��ά�ȣ�ԭ��ͳ���Ŀ���ͳ����
  char currTime[15],time_flag;
 
  char time_col[30];
  memset(time_col,0,sizeof(time_col));

  for(int i = 0;i<vItemInfo.size();i++)			//��ѯ���ͳ����ͳ��ά�ȵ�
  {
	SItem fromItem = vItemInfo[i].fromItem;
	SItem toItem = vItemInfo[i].toItem;
	switch(toItem.iItemType)					//����ͳ�����ͳ��ά�ȣ���Ϊ�˱�֤ԭ���Ŀ����ֶζ���
	{
		case 0:							
				sqlB2.append(toItem.szItemName).append(",");
				sqlA2.append(" sum(").append(fromItem.szItemName).append("),");
				break;
		case 1:						//����ͳ��ά��
		case 2:						
				sqlB1.append(toItem.szItemName).append(",");
				if(toItem.iBeginPos == -1)
				{
					sqlA1.append(fromItem.szItemName).append(",");
				}
				else
				{
					char tmp[50];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"SUBSTR(%s,%d,%d)",fromItem.szItemName,fromItem.iBeginPos,toItem.iEndPos);
					sqlA1.append(tmp).append(",");
				}
				break ;
		case 12:					//��־ȡ��ǰʱ��
				time_flag = 'Y';
				strcpy(time_col,toItem.szItemName);  //����ʱ���ֶ�
				break;
		default :
				break ;
   }

  }

  if(time_flag == 'Y')
  {
		sqlB2.append(time_col) ; 							  //��ӵ�Ŀ����ʱ���ջ��ܱ���������ֶ�����ֵ��д���ܵ�����
		sqlA2.append("to_char(sysdate,'yyyymmdd')");		  //ȡ���ݿ�ϵͳʱ��
  }
  else
  {
	sqlB2 = sqlB2.substr(0, sqlB2.length()-1);  //���Ŀ����ֶ�
	sqlA2 = sqlA2.substr(0, sqlA2.length()-1);  //���ԭʼ��ͳ����sum()
  }
  
  sqlA1 = sqlA1.substr(0, sqlA1.length()-1);  //���ԭʼ��ͳ��ά��
 
  //2013-11-18 �ջ��ܵ�ԭʼ��Ҳ�ӷ�������,Ҳ�����ļ����Ҳ�ǰ��������벻ͬ�ı�(��Ҫ�����ڹ�����������̫��)
  //sprintf(sql,"%s%s%s) select %s,%s from %s where %s like '%s%s' group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,szSCom.szOrgSumtCol,fromDateValue,"%",sqlA1);
  if(type == 1)
  {
	  sprintf(sql,"%s%s%s) select %s,%s from %s_%s where %s ='%s' group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,rate_cycle,szSCom.szOrgSumtCol,fromDateValue,sqlA1);
  }
  else
  {
	  if((type == 3) && (szSCom.count > 1))		//̯�ֻ��ܲ����ж��config_id ���˸�where����
	  {
		  string scond  = "where ";
		  scond.append(szSCom.mscontion[position].condition);
		  sprintf(sql,"%s%s%s) select %s,%s from %s_%s %s group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,fromDateValue,scond,sqlA1);
	  }
  	  else
	  {
		  sprintf(sql,"%s%s%s) select %s,%s from %s_%s group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,fromDateValue,sqlA1);
	  }
  }

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



//����һ��ʱ��,�жϵ�������
int getDays(char* time)
{
	int year = -1;
	int month = -1;
	int days= -1;

	if(strlen(time) != 6) 
	{
		theJSLog<<"����:"<<time<<"���Ϸ�, YYYYMM"<<endi;
		return -1;
	}
	char tmp[5];
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,time,4);
	year = atoi(tmp);

	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,time+4,2);
	month = atoi(tmp);
	switch(month)
	{
		case 2: /* �ж��Ƿ�Ϊ���겢�Ҹ�2�·ݸ�ֵ*/
			days=((year%4==0 && year%100!=0) || year%400==0)?29:28;
			break;
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			days=31;
			break;
		default:
			days=30;
   }

   return days;
}


#endif
