/****************************************************************
 filename: Comquery.cpp
 module:
 created by:
 create date:
 version: 3.0.0
 description:

 update:

 *****************************************************************/
#include"Comquery.h"

/*
inline bool operator < (const tableVersion x,  const tableVersion y)
{
	return (x.tableName < y.tableName || (x.tableName == y.tableName && x.Version < y.Version));
};
*/

C_Comquery::C_Comquery()
{
	/*
	this->executeTime1 = 0;
	this->executeTime2 = 0;
	this->queryTime = 0;
	this->executeCounter = 0;
	this->queryCounter = 0;
	this->executeTimeP1= 0;
	this->executeTimeP2= 0;
	this->executeTimeP3= 0;
	this->executeTimeP4= 0;
	this->executeTimeP5= 0;*/

	table = NULL;
//	m_tableNum=0;
}

void C_Comquery::init(char *szSourceGroupID, char *szServiceID, int index)
{
	strcpy(m_szServiceId, "SERV221");
	strcpy(m_szIniPath, getenv("ZHJS_INI"));
	m_szIniPath[strlen(m_szIniPath)-8]='\0';
	strcpy(m_szSourceGroupId, szSourceGroupID);
	table = getMemPoint();
	if(table == NULL)
		throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
}

/*
Description: ͨ����������������е�Ψһ��¼
����������:
TABLENAME,SEARCH_TYPE,INDEX_ID,CDRTIME,DEALMETHOD,<LACKINFO_ID>,CONDITION_COLS...<DEFAULT_RESULT_COLS...>
*/
void C_Comquery::execute(PacketParser& pps,ResParser& retValue)
{
	/*
	struct timeval start, startP, finish, queryTs;
	timerclear(&start);
	timerclear(&finish);
	timerclear(&queryTs);
	timerclear(&startP);
	gettimeofday(&start, NULL);
	gettimeofday(&startP, NULL);
	*/

	//
	char errorMsg[ERROR_MSG_LEN+1];

	if ( pps.getItem_num() < 7 )
	{
		sprintf( errorMsg,"the params sent to Comquery is lack");
		throw jsexcp::CException(0,(char *)errorMsg,(char *)__FILE__,__LINE__);
	}
	
	//��һ�������Ǳ���
	char szTableName[RECORD_LENGTH+1];
	memset(szTableName, 0, sizeof(szTableName));
	pps.getFieldValue(1,szTableName);
	DeleteSpace( szTableName );
	//cout<<"szTableName="<<szTableName<<endl;

	/*gettimeofday(&finish, NULL);
	executeTimeP1 += (finish.tv_sec - startP.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - startP.tv_usec);
	gettimeofday(&startP, NULL);*/

	char temp[RECORD_LENGTH+1];

	//�ڶ��������Ǳ��version_id
	int iVersion=1;
	memset(temp, 0, sizeof(temp));
	pps.getFieldValue(2,temp);
	DeleteSpace( temp );
	iVersion = atoi(temp);
	//cout<<"iVersion="<<iVersion<<endl;

	/*gettimeofday(&finish, NULL);
		executeTimeP2 += (finish.tv_sec - startP.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - startP.tv_usec);
		gettimeofday(&startP, NULL);*/

	//�����������Ǳ��index_id
	int iIndex=1;
	memset(temp, 0, sizeof(temp));
	pps.getFieldValue(3,temp);
	DeleteSpace( temp );
	iIndex = atoi(temp);
	//cout<<"iIndex="<<iIndex<<endl;

	//��ѯ��ʽ
	//1��ʾ���ֶηֿ���
	//2��ʾ�Ѳ�ѯ�ֶ�ƴ������
	int iSearchType = table->getSearchType(szTableName, iIndex);
	//cout<<"��ѯ��ʽ��"<<iSearchType<<endl;
	if( iSearchType!=1 && iSearchType!=2 )
	{
		sprintf( errorMsg,"�� [%s] �Ĳ�ѯ��ʽ [%d] û��Comquery�ж���", szTableName, iSearchType);
		throw jsexcp::CException(0,(char *)errorMsg,(char *)__FILE__,__LINE__);
	}

	/*gettimeofday(&finish, NULL);
		executeTimeP3 += (finish.tv_sec - startP.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - startP.tv_usec);
		gettimeofday(&startP, NULL);*/

	//���ĸ�������ʱ���ֶ�
	char szTime[RECORD_LENGTH+1];
	memset(szTime, 0, sizeof(szTime));
	pps.getFieldValue(4,szTime);
	DeleteSpace( szTime );
	//cout<<"time:"<<szTime<<endl;

	/*gettimeofday(&finish, NULL);
		executeTimeP4 += (finish.tv_sec - startP.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - startP.tv_usec);
		gettimeofday(&startP, NULL);*/

	//�Ȱ�ʱ�丳ֵ
	inData.clear();
	sprintf(inData.startTime, "%s", szTime);

	//����������Ǵ���ʽ
	memset(temp, 0, sizeof(temp));
	pps.getFieldValue(5,temp);
	DeleteSpace( temp );
	if(strlen(temp) == 1)
	{
		dealMethod = atoi(temp);
	}
	else
	{
		sprintf( errorMsg,"�� [%s] �Ĵ���ʽ [%s] û��Comquery�ж���", szTableName, temp);
		throw jsexcp::CException(0,(char *)errorMsg,(char *)__FILE__,__LINE__);
	
	}
	/*gettimeofday(&finish, NULL);
	executeTimeP5 += (finish.tv_sec - startP.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - startP.tv_usec);
	gettimeofday(&startP, NULL);

	gettimeofday(&finish, NULL);
	executeTime1 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
	gettimeofday(&start, NULL);*/
	
	int iTableOffset = table->getTableOffset(szTableName);
	if(iTableOffset<0)
	cout<<"table name error!"<<endl;

	//����ʽ1���鲻���ͷ��ص���λ�������ϴ���
	//�����������������ϴ��룬�ӵ���λ��ʼ��Ϊ��ѯ���������
	if(dealMethod == 1)
	{
		int iCount = pps.getItem_num() -6; //��ѯ��������

		//inData.clear();
		for(int i=0; i<iCount; i++)
		{
			memset(temp, 0, sizeof(temp));
			pps.getFieldValue(6+i+1, temp);
			if( iSearchType==1 )
				inData.set( temp );
			else
				inData.set( temp, 0 );
		}
		if( iSearchType==1 )
			inData.itemNum=iCount;
		else
			inData.itemNum=1;

		/*gettimeofday(&queryTs, NULL);
		queryCounter ++;*/
		bool ret = table->getData(iTableOffset, &inData, &outData, iIndex)==0;
		/*gettimeofday(&finish, NULL);
		queryTime += (finish.tv_sec - queryTs.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - queryTs.tv_usec);
		*/
		if(ret)
		{
			//cout<<"���ҵ�"<<outData.itemNum<<"��"<<endl;
			for(int i=0; i<outData.itemNum; i++)
			{
				memset(temp, 0, sizeof(temp));
				strcpy(temp, outData.values[i]);
				//cout<<"Comquery->temp["<<i+1<<"]:"<<temp<<endl;
				retValue.setFieldValue(i+1, temp, strlen(temp));
			}
		}
		else
		{
			//�Ҳ���
			//�����������Ǵ������
			char szLackType[RECORD_LENGTH+1];
			memset(szLackType, 0, sizeof(szLackType));
			pps.getFieldValue(6, szLackType);
			DeleteSpace(szLackType);
			//cout<<"lack:"<<szLackType<<endl;
			pluginAnaResult result=eLackInfo;
			retValue.setAnaResult(result, szLackType, inData.values[0]);
		}
	}
	//����ʽ2����дĬ��ֵ
	//������������ʼ��Ϊ��ѯ��������������λ��Ĭ��ֵ
	else if(dealMethod == 2)
	{
		//cout<<"����ʽ2"<<endl;
		int iDefNum = table->getColNum(szTableName);
		int iCount = pps.getItem_num() - 5 - iDefNum;
		//cout<<"iCount="<<iCount<<endl;
		if(iCount < 1)
		{
			sprintf( errorMsg,"the params sent to Comquery is lack");
			throw jsexcp::CException(0,(char *)errorMsg,(char *)__FILE__,__LINE__);
		}
		//��ȡ����������ѯ����
		//inData.clear();
		for(int i=1; i<=iCount; i++)
		{
			memset(temp, 0, sizeof(temp));
			pps.getFieldValue(5+i, temp);//�ӵ�6��������ʼȡ
			DeleteSpace(temp);
			//cout<<"packet_in["<<5+i<<"]="<<temp<<endl;
			if( iSearchType==1 )
				inData.set( temp );
			else
				inData.set( temp, 0 );
		}
		//cout<<inData.values[0]<<endl;
		if( iSearchType==1 )
			inData.itemNum =  iCount;
		else
			inData.itemNum =  1;
		//cout<<"itemNum="<<inData.itemNum<<"\tIndex="<<iIndex<<endl;
		/*gettimeofday(&queryTs, NULL);
		queryCounter++;*/
		bool ret = table->getData(iTableOffset, &inData, &outData, iIndex)==0;
		/*gettimeofday(&finish, NULL);
		queryTime += (finish.tv_sec - queryTs.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - queryTs.tv_usec);
		*/
		if(ret)
		{
			//cout<<"���ҵ�"<<outData.itemNum<<"��"<<endl;
			//���������ҽ��
			for(int i=0; i<outData.itemNum; i++)
			{
				memset(temp, 0, sizeof(temp));
				strcpy(temp, outData.values[i]);
//				cout<<"temp["<<i+1<<"]:"<<temp<<endl;
				retValue.setFieldValue(i+1, temp, strlen(temp));
			}
		}
		//�޷��鵽�������Ĭ��ֵ�������
		else
		{
			for(int i=1; i<=iDefNum; i++)
			{
				memset(temp, 0, sizeof(temp));
				pps.getFieldValue(iCount+5+i, temp);
				DeleteSpace(temp);
				//cout<<"Ĭ��temp["<<i<<"]="<<temp<<endl;
				retValue.setFieldValue(i, temp, strlen(temp));
			}
		}
	}
	//����ʽδ����
	else
	{
		sprintf( errorMsg,"Comquery��δ�����������[%d] , ����[%s]", dealMethod, szTableName);
		throw jsexcp::CException(0,(char *)errorMsg,(char *)__FILE__,__LINE__);
	}

/*
	gettimeofday(&finish, NULL);
	executeTime2 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
	executeCounter ++;*/
	//table->detach();
	//delete table;
}

void C_Comquery::message(MessageParser&  pMessage)
{
	int message=pMessage.getMessageType();
	switch(message)
	{
		case MESSAGE_NEW_BATCH:
			/*
			for(int i =0; i<m_tableNum; i++)
			{
				if(table[i] != NULL)
				{
					table[i]->reBind();
				}
			}
			*/
			break;
		case MESSAGE_END_BATCH_END_DATA:
			/*
			theLog << "----------Comquery performance: execute counter:" << this->executeCounter << endi;
			theLog << "----------Comquery performance: query counter:" << this->queryCounter << endi;
			theLog << "----------Comquery performance: execute1:" << this->executeTime1 << endi;
			theLog << "----------Comquery performance: execute2 + query:" << this->executeTime2 << endi;
			theLog << "----------Comquery performance: query time:" << this->queryTime << endi;
			theLog << "----------Comquery performance: execute ---- p1:" << this->executeTimeP1 << endi;
			theLog << "----------Comquery performance: execute ---- p1:" << this->executeTimeP2 << endi;
			theLog << "----------Comquery performance: execute ---- p1:" << this->executeTimeP3 << endi;
			theLog << "----------Comquery performance: execute ---- p1:" << this->executeTimeP4 << endi;
			theLog << "----------Comquery performance: execute ---- p1:" << this->executeTimeP5 << endi;*/
			break;
		case MESSAGE_BREAK_BATCH:
			break;
		default:
			break;
	}
}

void C_Comquery::printMe()
{
	printf("\t�������:Comquery,�汾�ţ�3.0.0 \n");
}

std::string C_Comquery::getPluginName()
{
	return "Comquery";
}

std::string C_Comquery::getPluginVersion(){
	return "3.0.0";
}

C_Comquery::~C_Comquery()
{
	/*
	tableMap.clear();
	tableIndex.clear();
	for(int i =0; i<MAX_TABLE_NUM; i++)
	{
		if(table[i] != NULL)
		{
			table[i]->detach();
			delete table[i];
			table[i] = 0;
		}
	}
	*/
}



