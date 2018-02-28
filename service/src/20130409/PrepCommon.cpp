/****************************************************************
 filename: PrepCommon.h
 module: 预处理共有的模块
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	通用查询
 update:

 *****************************************************************/

#include "PrepCommon.h"
using namespace std;
using namespace tpss;

C_DealCalculator::C_DealCalculator()
{
}

C_DealCalculator::~C_DealCalculator()
{
	clear();
}

int C_DealCalculator::clear()
{
	vRecordPacket.clear();

	m_TotalNum = 0;
	m_RightNum = 0;
	m_LackNum = 0;
	m_ErrorNum = 0;
	m_PickNum = 0;
	m_OtherNum = 0;

	m_TotalFee = 0;
	m_RightFee = 0;
	m_LackFee = 0;
	m_ErrorFee = 0;
	m_PickFee = 0;
	m_OtherFee = 0;

	m_TotalTime = 0;
	m_RightTime = 0;
	m_LackTime = 0;
	m_ErrorTime = 0;
	m_PickTime = 0;
	m_OtherTime = 0;

	return 0;
}

int C_DealCalculator::set(const char* servCat, int total, int right, int lack, int error, int pick, int other)
{
	int iMark = 0;
	/*for(it_Num=vRecordPacket.begin(); it_Num!=vRecordPacket.end(); it_Num++)
	{
		if(!strcmp(it_Num->szSerCatId, servCat))
		{
			it_Num->iTotalNum += total;
			it_Num->iRightNum += right;
			it_Num->iLackNum += lack;
			it_Num->iErrorNum += error;
			it_Num->iPickNum += pick;
			it_Num->iOtherNum += other;
			break;
		}
	}
	if(it_Num == vRecordPacket.end())
	{
		recordCount.iTotalNum=total;
		recordCount.iRightNum=right;
		recordCount.iLackNum=lack;
		recordCount.iErrorNum=error;
		recordCount.iPickNum=pick;
		recordCount.iOtherNum=other;
		strcpy(recordCount.szSerCatId, servCat);
		vRecordPacket.push_back(recordCount);
	}*/
	for(int i=0; i<vRecordPacket.size(); i++)
	{
		if(!strcmp(vRecordPacket[i].szSerCatId, servCat))
		{
			vRecordPacket[i].iTotalNum += total;
			vRecordPacket[i].iRightNum += right;
			vRecordPacket[i].iLackNum += lack;
			vRecordPacket[i].iErrorNum += error;
			vRecordPacket[i].iPickNum += pick;
			vRecordPacket[i].iOtherNum += other;
			iMark = 1;
			break;
		}
	}
	if(iMark == 0)
	{
		recordCount.iTotalNum=total;
		recordCount.iRightNum=right;
		recordCount.iLackNum=lack;
		recordCount.iErrorNum=error;
		recordCount.iPickNum=pick;
		recordCount.iOtherNum=other;
		strcpy(recordCount.szSerCatId, servCat);
		vRecordPacket.push_back(recordCount);
	}

	m_TotalNum += total;
	m_RightNum += right;
	m_LackNum += lack;
	m_ErrorNum += error;
	m_PickNum += pick;
	m_OtherNum += other;

	return 0;
}

int C_DealCalculator::setFee(long long total, long long right, long long lack, long long error, long long pick, long long other)
{
	m_TotalFee += total;
	m_RightFee += right;
	m_LackFee += lack;
	m_ErrorFee += error;
	m_PickFee += pick;
	m_OtherFee += other;

	return 0;
}

int C_DealCalculator::setTime(long long total, long long right, long long lack, long long error, long long pick, long long other)
{
	m_TotalTime += total;
	m_RightTime += right;
	m_LackTime += lack;
	m_ErrorTime += error;
	m_PickTime += pick;
	m_OtherTime += other;

	return 0;
}

int C_DealCalculator::check()
{
	/*for(it_Num = vRecordPacket.begin(); it_Num != vRecordPacket.end(); it_Num++)
	{
		if(it_Num->iTotalNum != (it_Num->iLackNum + it_Num->iPickNum + it_Num->iRightNum + it_Num->iErrorNum))
		{
			cout<<"记录数不平"<<endl;
			theLog<<"记录数不平"<<endw;
		}
		if(it_Num->iLackNum > 0)
		{
			cout<<"无资料记录数："<<it_Num->iLackNum<<endl;
		}
	}*/
	for(int i=0;i < vRecordPacket.size(); i++)
	{
		if(vRecordPacket[i].iTotalNum != (vRecordPacket[i].iLackNum + vRecordPacket[i].iPickNum + vRecordPacket[i].iRightNum + vRecordPacket[i].iErrorNum))
		{
			//cout<<"记录数不平"<<endl;
			theLog<<"记录数不平"<<endw;
		}
		if(vRecordPacket[i].iLackNum > 0)
		{
			cout<<"无资料记录数："<<vRecordPacket[i].iLackNum<<endl;
		}
	}
	return 0;
}

//int C_DealCalculator::updateTable(const char* tableName, const char* sourceId, const char* fileName, const char* startTime)
int C_DealCalculator::updateTable(const char* tableName, struct SFileStruct &filestruct, int proc_index)
{
    char CurrentTime[DATETIME_LEN+1];
	getCurTime(CurrentTime);
	strcpy(filestruct.szDealEndTime, CurrentTime);
	theLog<<"---- "<<filestruct.szRealFileName<<" dealed "<<m_TotalNum<<" records"<<endi;
	
    try{			
	if (dbConnect(conn))
	 {
	        Statement stmt = conn.createStatement();
			string sql ;
	    if(vRecordPacket.size() == 1)
	     {
			sql = "update :v1 set serv_cat_id=':v2', dealendtime=':v3', input_count=:v4, mainflow_count=:v5, \
			lackinfo_count=:v6, error_count=:v7, pick_count=:v8, other_count=:v9, deal_flag='Y' where proc_index = :v10 \
			and filename = ':v11' and source_id = ':v12' and partid in(:v13,:v14,:v15)";		
			stmt.setSQLString(sql);
			stmt << tableName << vRecordPacket[0].szSerCatId << 
			CurrentTime << vRecordPacket[0].iTotalNum << vRecordPacket[0].iRightNum << vRecordPacket[0].iLackNum << 
			vRecordPacket[0].iErrorNum << vRecordPacket[0].iPickNum << vRecordPacket[0].iOtherNum << proc_index <<
			filestruct.szRealFileName << filestruct.szSourceId << getPrePartID(filestruct.iPartID) << filestruct.iPartID << 
			getNextPartID(filestruct.iPartID);
			if (stmt.execute())
			{
                conn.commit();
			}
			theLog<<"---- SERV_CAT "<<vRecordPacket[0].szSerCatId<<" with "<<vRecordPacket[0].iRightNum<<" items into mainflow, "
			<<vRecordPacket[0].iLackNum<<" lack_info items , "<<vRecordPacket[0].iErrorNum<<" abnormal items and "<<vRecordPacket[0].iPickNum
			<<" pick items!"<<endi;
		    theLog<<"---- and "<<vRecordPacket[0].iOtherNum<<" items in the pick path!"<<endi;

	    }
	    else
	   {
	      sql = "delete from :v1 where proc_index = :v2 and filename = ':v3' and source_id = ':v4' and \
			partid in(:v5,:v6,:v7)";		
			stmt.setSQLString(sql);
			stmt << tableName << proc_index << filestruct.szRealFileName << filestruct.szSourceId <<
			getPrePartID(filestruct.iPartID) << filestruct.iPartID << getNextPartID(filestruct.iPartID);
			if (stmt.execute())
			{
                conn.commit();
			}
			
		/*将输入日志表中的文件状态置为“Y” */
		for(int i=0; i<vRecordPacket.size(); i++)
		{
		    sql = "insert into :v1(source_id, serv_cat_id, filename, proc_index, dealstarttime, dealendtime, \
				input_count, mainflow_count, lackinfo_count, error_count, pick_count, other_count, deal_flag, validflag) \
				values(':v2', ':v3', ':v4', :v5, ':v6', ':v7', :v8, :v9, :v10, :v11, :v12, :v13, 'Y', 'Y')";		
			stmt.setSQLString(sql);
			stmt <<  tableName << filestruct.szSourceId << 
				vRecordPacket[i].szSerCatId << filestruct.szRealFileName << proc_index << filestruct.szDealStartTime << CurrentTime << 
				vRecordPacket[i].iTotalNum << vRecordPacket[i].iRightNum << vRecordPacket[i].iLackNum << vRecordPacket[i].iErrorNum <<
				vRecordPacket[i].iPickNum << vRecordPacket[i].iOtherNum;
			if (stmt.execute())
			{
                conn.commit();
			}
			theLog<<"---- SERV_CAT "<<vRecordPacket[i].szSerCatId<<" with "<<vRecordPacket[i].iRightNum<<" items into mainflow, "
				<<vRecordPacket[i].iLackNum<<" lack_info items , "<<vRecordPacket[i].iErrorNum<<" abnormal items and "<<vRecordPacket[i].iPickNum
				<<" pick items!"<<endi;
			theLog<<"---- and "<<vRecordPacket[i].iOtherNum<<" items in the pick path!"<<endi;
		}
	  }
	    
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
 } 	 
	theLog<<"Update in_schedule set to _Y_ successfully!"<<endi;
	return 0;
}

int C_DealCalculator::updateTable(const char* tableName, char *filename, char *dealstarttime, char *sourceID,int proc_index )
{
	//CBindSQL sql(DBConn);
	char CurrentTime[DATETIME_LEN+1];
	getCurTime(CurrentTime);

    int tem5 = CurrentTime[4] - '0';
    int tem6 = CurrentTime[5] - '0';
    int iPartID = tem5*10 + tem6;  //从当前时间获取数据库表的partid

    try
	{
	  if (dbConnect(conn))
	  {
	       Statement stmt = conn.createStatement();
		   string sql;
	     if(vRecordPacket.size() == 1)
	      {				
			sql = "insert into :v1(source_id,serv_cat_id,filename,deal_flag,dealstarttime,dealendtime,input_count, \
		      mainflow_count,error_count,lackinfo_count,pick_count,other_count,validflag,partid,proc_index) values\
		      (':v2', ':v3', ':v4', 'Y', ':v5', ':v6', :v7, :v8, :v9, :v10, :v11, :v12, 'Y' , :v13, :v14)";		
			stmt.setSQLString(sql);
			stmt << tableName<< sourceID << vRecordPacket[0].szSerCatId << filename << dealstarttime << 
				 CurrentTime << vRecordPacket[0].iTotalNum << 
		      vRecordPacket[0].iRightNum << vRecordPacket[0].iErrorNum << vRecordPacket[0].iLackNum << vRecordPacket[0].iPickNum<< 
		      vRecordPacket[0].iOtherNum << iPartID << proc_index;
			if (stmt.execute())
			{
                conn.commit();;
		    }
            theLog<<"filename:"<<filename<<"---- SERV_CAT "<<vRecordPacket[0].szSerCatId<<" with "<<vRecordPacket[0].iRightNum<<" items into mainflow, "
			    <<vRecordPacket[0].iLackNum<<" lack_info items , "<<vRecordPacket[0].iErrorNum<<" abnormal items and "<<vRecordPacket[0].iPickNum
			    <<" pick items!"<<endi;
		   theLog<<"---- and "<<vRecordPacket[0].iOtherNum<<" items in the pick path!"<<endi;	

		theLog<<"filename:"<<filename<<"---- SERV_CAT "<<vRecordPacket[0].szSerCatId<<" with "<<vRecordPacket[0].iRightNum<<" items into mainflow, "
			<<vRecordPacket[0].iLackNum<<" lack_info items , "<<vRecordPacket[0].iErrorNum<<" abnormal items and "<<vRecordPacket[0].iPickNum
			<<" pick items!"<<endi;
		theLog<<"---- and "<<vRecordPacket[0].iOtherNum<<" items in the pick path!"<<endi;
	}
	else
	{
		sql = "delete from :v1 where proc_index = :v2 and filename = ':v3' and source_id = ':v4' and \
			partid in(:v5,:v6,:v7)";		
			stmt.setSQLString(sql);
			stmt << tableName << proc_index << filename << sourceID <<
			getPrePartID(iPartID) << iPartID << getNextPartID(iPartID);
			if (stmt.execute())
			{
                conn.commit();;
		    }

		/*将输入日志表中的文件状态置为“Y” */
		for(int i=0; i<vRecordPacket.size(); i++)
		{
		    sql = "insert into :v1(source_id, serv_cat_id, filename, proc_index, dealstarttime, dealendtime, \
				input_count, mainflow_count, lackinfo_count, error_count, pick_count, other_count, deal_flag, validflag) \
				values(':v2', ':v3', ':v4', :v5, ':v6', ':v7', :v8, :v9, :v10, :v11, :V12, :v13, 'Y', 'Y')" ;		
			stmt.setSQLString(sql);
			stmt << tableName << sourceID <<  
				vRecordPacket[i].szSerCatId << filename << proc_index << dealstarttime << CurrentTime <<
				vRecordPacket[i].iTotalNum << vRecordPacket[i].iRightNum << vRecordPacket[i].iLackNum << vRecordPacket[i].iErrorNum<< 
				vRecordPacket[i].iPickNum << vRecordPacket[i].iOtherNum ;
			if (stmt.execute())
			{
                conn.commit();;
		    }
			theLog<<"---- SERV_CAT "<<vRecordPacket[i].szSerCatId<<" with "<<vRecordPacket[i].iRightNum<<" items into mainflow, "
				<<vRecordPacket[i].iLackNum<<" lack_info items , "<<vRecordPacket[i].iErrorNum<<" abnormal items and "<<vRecordPacket[i].iPickNum
				<<" pick items!"<<endi;
			theLog<<"---- and "<<vRecordPacket[i].iOtherNum<<" items in the pick path!"<<endi;
		  }  // end for
		
		}	
	     conn.commit();
	     conn.close();
	  	}
    }
	catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
    }
	theLog<<"insert in_schedule set to _Y_ successfully!"<<endi;
	return 0;
}

int C_DealCalculator::getTotal()
{
	return m_TotalNum;
}

int C_DealCalculator::getRight()
{
	return m_RightNum;
}

int C_DealCalculator::getLack()
{
	return m_LackNum;
}

int C_DealCalculator::getError()
{
	return m_ErrorNum;
}

int C_DealCalculator::getPick()
{
	return m_PickNum;
}

int C_DealCalculator::getOther()
{
	return m_OtherNum;
}

long long C_DealCalculator::getTotalFee()
{
	return m_TotalFee;
}

long long C_DealCalculator::getRightFee()
{
	return m_RightFee;
}

long long C_DealCalculator::getLackFee()
{
	return m_LackFee;
}

long long C_DealCalculator::getErrorFee()
{
	return m_ErrorFee;
}

long long C_DealCalculator::getPickFee()
{
	return m_PickFee;
}

long long C_DealCalculator::getOtherFee()
{
	return m_OtherFee;
}

long long C_DealCalculator::getTotalTime()
{
	return m_TotalTime;
}

long long C_DealCalculator::getRightTime()
{
	return m_RightTime;
}

long long C_DealCalculator::getLackTime()
{
	return m_LackTime;
}

long long C_DealCalculator::getErrorTime()
{
	return m_ErrorTime;
}

long long C_DealCalculator::getPickTime()
{
	return m_PickTime;
}

long long C_DealCalculator::getOtherTime()
{
	return m_OtherTime;
}

bool SearchAllFiles(const char *Pathname, const char *Format, vector<string> &vecFileSet)
{
	CF_FileScan fscan;

	char tempPath[PATH_NAME_LEN+1];
	sprintf(tempPath, "%s", Pathname);
	char tempFormat[256];
	sprintf(tempFormat, "%s", Format);
	fscan.scan_file(tempPath,3,tempFormat); 
	char szFile[256];
	while(fscan.get_file(szFile) != 100)
	{
		char subFileName[FILE_NAME_LEN+1];
		strcpy(subFileName, szFile+strlen(Pathname));
		vecFileSet.push_back(string(subFileName));
	}
	return true;
}

int getPrePartID(int PartID)
{
	if(PartID == 1)
		return 12;
	else
		return (PartID-1);
}

int getNextPartID(int PartID)
{
	if(PartID == 12)
		return 1;
	else
		return (PartID+1);
}


int getPrePartID(char* PartID)
{
	int iPID = atoi(PartID);
	if(iPID == 1)
		return 12;
	else
		return (iPID-1);
}
int getNextPartID(char* PartID)
{
	
	int iPID = atoi(PartID);
	if(iPID == 12)
		return 1;
	else
		return (iPID+1);
}
