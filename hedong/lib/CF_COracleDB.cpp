// ���ݿ�ӿ���
// ���ߣ���ΰ(zhouwei@gsta.com;wilston1979@tom.com)
// ���°汾��1.09��2005-4-1
// ����������Ϊ��ѹ����࣬������Oracle9i���ϰ汾���ݿ�
// �����ʹ���ϵĲ���֮�����߷���bug���뼰ʱ��֪����
// ���ļ�¼

#include "CF_COracleDB.h"
#include <ctype.h>
#include <string>
#include <string.h>
//#include <algorithm>

//�Ƿ�ʹ�ô�ͳIO��,ȱʡʹ�õ��Ǳ�׼��
//ʵ��֤��,ʵ���ϸö�����û��ʹ����
//�Ӷ�ʹʹ�ñ��ӿڵ�Ӧ�ÿ�������ѡ��ʹ�ñ�׼���ߴ�ͳ��
#define USE_CLASSICAL_IOSTREAM_LIB

#ifdef USE_CLASSICAL_IOSTREAM_LIB

#include <stdio.h>

#else	// User Standard IO Stream Library

#include <iostream>
#include <fstream>
#include <strstream>

#endif

//�Ƿ�ʹ��CF_Error���������쳣��ȱʡʹ��CDBException��
//CF_Error�����ۺϽ�����Ŀ��ʹ�õ�ͳһ�쳣������
#define USE_CF_ERROR_EXCEPTION

#ifdef USE_CF_ERROR_EXCEPTION

//#include "CF_CError.h"
#include "CF_CException.h"
#define MAX_ERROR_MSG_LENGTH	2048
const int g_iErrorType = 'D';	//���ݿ����
const int g_iErrorLevel = 'H';	//���󼶱��
char g_szErrorMsg[MAX_ERROR_MSG_LENGTH];

// Linux gcc 2.95��֧�ֱ���\���з���Ҳ��֧��strlcpy����
//#define _EXCEPTION(iErrorCode, strErrorMsg)  \
//	strlcpy(g_szErrorMsg,  strErrorMsg.c_str(), MAX_ERROR_MSG_LENGTH), \
//	CF_CError( g_iErrorType,  g_iErrorLevel, iErrorCode, ZERO, g_szErrorMsg, __FILE__, __LINE__)

#define STRLCPY(sDest, sSrc, nSize)  strncpy(sDest, sSrc, nSize > strlen(sSrc) + 1 ? strlen(sSrc) + 1 : nSize)
//#define THROW_EXCEPTION(iErrorCode, strErrorMsg);  {DelSwitchLine(strErrorMsg); STRLCPY(g_szErrorMsg,  strErrorMsg.c_str(), MAX_ERROR_MSG_LENGTH); throw CF_CError( g_iErrorType,  g_iErrorLevel, iErrorCode, ZERO, g_szErrorMsg, __FILE__, __LINE__); }
#define THROW_EXCEPTION(iErrorCode, strErrorMsg);  {DelSwitchLine(strErrorMsg); STRLCPY(g_szErrorMsg,  strErrorMsg.c_str(), MAX_ERROR_MSG_LENGTH); throw CException( iErrorCode, g_szErrorMsg, __FILE__, __LINE__); }

#else
#define THROW_EXCEPTION(iErrorCode, strErrorMsg);  {DelSwitchLine(strErrorMsg); throw CDBException(iErrorCode, strErrorMsg); }
	
#endif

//�Ƿ���ȫ�ֵ�CDatabase����
#ifdef USE_GLOABLE_DB_CONNECTION
//CDatabase DBConn;
#endif

#ifdef USE_GLOABLE_POOL
CDatabase DBPool[MAX_POOL_NUM];  //Ĭ��MAX_POOL_NUM������
int iConnect[MAX_POOL_NUM];   //����״̬
int iState[MAX_POOL_NUM];     //����ʹ��״̬
pthread_mutex_t dbThreadLock;  //��
#endif

#ifdef USE_CLASSICAL_IOSTREAM_LIB

#define cerr  _mycerr
#define cout  _mycout

#define endl	_myendl

class MyStream
{
private:
	FILE*	  m_nFh;
	string	  m_strTrait;
public:
	MyStream(FILE* nFh, const char* pszParam) { m_nFh = nFh; m_strTrait.append(pszParam);}
	MyStream(FILE* nFh, const string& strParam) { m_nFh = nFh; m_strTrait = strParam; }
	MyStream(FILE* nFh)		{ m_nFh = nFh; m_strTrait = ""; }
	MyStream(const string& strParam)	{ m_nFh = stdout; m_strTrait = strParam; }
	MyStream(const char* pszParam)		{ m_nFh = stdout; m_strTrait.append(pszParam);}
	MyStream& operator <<(const int& iParam) { fprintf(m_nFh, "%d", iParam); return *this;}
	MyStream& operator <<(const unsigned int& uParam) { fprintf(m_nFh, "%u", uParam); return *this;}
	MyStream& operator <<(const long& lParam) { fprintf(m_nFh, "%ld", lParam); return *this;}
	MyStream& operator <<(const unsigned long& uParam) { fprintf(m_nFh, "%u", uParam); return *this;}
	MyStream& operator <<(const float& fParam) { fprintf(m_nFh, "%f", fParam); return *this;}
	MyStream& operator <<(const double& dParam) { fprintf(m_nFh, "%f", dParam); return *this;}
	MyStream& operator <<(const string& strParam) { fprintf(m_nFh, "%s", strParam.c_str()); return *this; }
	MyStream& operator <<(const char* pszParam) { fprintf(m_nFh, "%s", pszParam); return *this; }
	MyStream& operator <<(MyStream& stream) { if(stream.m_strTrait == "endl")	fprintf(m_nFh, "\n"); return *this;}
};

MyStream _mycout(stdout, "");
MyStream _mycerr(stderr, "");
MyStream _myendl("endl");

#endif

//ȡֵ1000���ϲ�𲻴�100Ч�ʱ�1000Ч�ʵ�10%
#define PREFETCH_ROW_COUNT 	1000
#define MAX_STRING_SIZE		1000

//������Ϣ�ָ���
#define C_ERROR_INFO_SEPERATOR_1 ':'
#define C_ERROR_INFO_SEPERATOR_2 ','
#define C_ERROR_INFO_SEPERATOR_3 ';'
#define C_ERROR_INFO_SEPERATOR_4 ' '


void DelSwitchLine(string &ss)
{
	for(int i=0;i<ss.size();i++)
	{
		if(ss[i]=='\n')	ss[i]=' ';
	}
}


int getMyEnv(const char *fname, const char *comp, char *ret )
{
 int i;
 FILE *tmpfp;
 char ss[2][1000], buf[1000];
   if( ( tmpfp = fopen( fname, "r" ) ) == NULL ) {
   	 //deleted by tanj 20050925
     //cout<<"open file Err!:"<<fname<<endl;    
     return(-1);
   }

   while( fgets( buf, 1000, tmpfp ) != NULL ) {
     if( buf[0] == '#' ) continue;
     ss[1][0]='\0';
     //add by zhanggq
     sscanf( buf, "%s %s", ss[0], ss[1]);
     if( !strcmp( ss[0], comp ) ) {
        strcpy( ret, ss[1] );
        fclose( tmpfp );
        return(0);
     }
   }
   fclose( tmpfp );
   //deleted by tanj 20050925
   //cout<<"Search File "<<fname<<" Nothing like "<<comp;
	return(-2);
}


/*************************************************
* ����    ���������ݿ�, ����ʱӦʹ��try_catch��ʽ
* ��ڲ�����pchEnvFile�����������ļ���
*           db�����ݿ����ӱ���
* ���ڲ�������
* ����    ���ɹ�����0
*           -1:δ�ҵ����ݿ�SID
*			-2:δ�ҵ�UNAME
*			-3:δ�ҵ�UPASS
*			-4:����ʧ��
**************************************************/
int createDBPool(char *pchEnvFile,int num)
{
	CEncryptAsc encrypt;
	char *pchValue;
	char pchDBname[50];
	char pchUserName[50];
	char pchUserPWD[50];

	char pchEncryptedUName[100];
	char pchEncryptedUPWD[100];
	//
	if(num>MAX_POOL_NUM)
	{
		int iErrorCode = 0;
		string strErrorMsg = "max connection limit!";
		throw CDBException(iErrorCode, strErrorMsg);
	}
	//��ȡ���ݿ�SID
	if(getMyEnv(pchEnvFile, "SID", pchDBname) != 0)
		return -1;

	//��ȡ���ݿ��û���
	if(getMyEnv(pchEnvFile, "UNAME", pchEncryptedUName) != 0)
		return -2;
	encrypt.Decrypt(pchEncryptedUName, pchUserName);
	
	//��ȡ���ݿ��û�����
	if(getMyEnv(pchEnvFile, "UPASS", pchEncryptedUPWD) != 0)
		return -3;
	encrypt.Decrypt(pchEncryptedUPWD, pchUserPWD);

	pthread_mutex_init(&dbThreadLock,NULL);
	//�������ݿ�
	for(int i=0;i<num;i++)
	{
		if(DBPool[i].Connect(pchUserName, pchUserPWD, pchDBname) != 0)
			return -4;
		else
		{
			iConnect[i]=1;  //��������1
			iState[i]=0;    //δʹ��
		}
	}
	return 0;
}

int GetConnectionId()
{
	int id=0;
	bool bGet=false;
	pthread_mutex_lock(&dbThreadLock);
	for(int i=0;i<MAX_POOL_NUM;i++)
	{
		if(iConnect[i]==1&&iState[i]==0)
		{
			id=i;//�ҵ�������
			iState[i]=1;
			bGet=true;
			break;
		}
	}
	pthread_mutex_unlock(&dbThreadLock);
	if(bGet)
		return id;
	else
		return -1;
}

int ReleaseId(int id)
{
	if(id>MAX_POOL_NUM)
		return -1;
	pthread_mutex_lock(&dbThreadLock);
	iState[id]=0;
	pthread_mutex_unlock(&dbThreadLock);
	return 0;
}

void DisconnectPool()
{
	for(int i=0;i<MAX_POOL_NUM;i++)
	{
		DBPool[i].Disconnect();
	}
}

// Class CFieldValue
//�ֶ�ֵ�࣬Ŀǰֻ֧��Number��String����
//����ͳһ�洢���ݿ���ֶκͰ󶨱������Լ�����ת��

CFieldValue::operator int()
{
	int iReturn = 0;

	if(m_iDataType == OCCIINT || m_iDataType == OCCI_SQLT_NUM
		|| m_iDataType == OCCIUNSIGNED_INT || m_iDataType == OCCIFLOAT)
		return (int)m_numValue; 
	
	int iErrorCode = 0;
	string strErrorMsg = "";
	switch(m_iDataType)
	{
		case OCCI_SQLT_STR:
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to int";
			throw CDBException(iErrorCode, strErrorMsg);
			iReturn = ZERO;
			break;
				
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = " datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			iReturn = ZERO;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to int";
			throw CDBException(iErrorCode, strErrorMsg);
			iReturn = ZERO;
			break;
	}

	return iReturn;
}	

CFieldValue::operator unsigned int()
{
	if(m_iDataType ==  OCCIUNSIGNED_INT || m_iDataType == OCCI_SQLT_NUM
		|| m_iDataType == OCCIINT || m_iDataType == OCCIFLOAT)
		return (unsigned int)m_numValue; 

	unsigned int nReturn = 0;
	int iErrorCode = 0;
	string strErrorMsg = "";
	switch(m_iDataType)
	{
		case OCCI_SQLT_STR:
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to unsigned int";
			throw CDBException(iErrorCode, strErrorMsg);
			nReturn = ZERO;
			break;
				
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			nReturn = ZERO;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to unsigned int";
			throw CDBException(iErrorCode, strErrorMsg);
			nReturn = ZERO;
			break;
	}

	return nReturn;
}	

CFieldValue::operator unsigned long()
{
	if(m_iDataType ==  OCCIUNSIGNED_INT || m_iDataType == OCCI_SQLT_NUM
		|| m_iDataType == OCCIINT || m_iDataType == OCCIFLOAT)
		return (unsigned long)m_numValue; 
	
	unsigned long ulReturn = 0;
	int iErrorCode = 0;
	string strErrorMsg = "";
	switch(m_iDataType)
	{
		case OCCI_SQLT_STR:
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to unsigned long";
			throw CDBException(iErrorCode, strErrorMsg);
			ulReturn = ZERO;
			break;
				
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			ulReturn = ZERO;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to unsigned int";
			throw CDBException(iErrorCode, strErrorMsg);
			ulReturn = ZERO;
			break;
	}

	return ulReturn;
}	

CFieldValue::operator long()
{
	if(m_iDataType ==  OCCIUNSIGNED_INT || m_iDataType == OCCI_SQLT_NUM
		|| m_iDataType == OCCIINT || m_iDataType == OCCIFLOAT)
		return (long)m_numValue; 
	
	long lReturn = 0;
	int iErrorCode = 0;
	string strErrorMsg = "";
	switch(m_iDataType)
	{
		case OCCI_SQLT_STR:
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to long";
			throw CDBException(iErrorCode, strErrorMsg);
			lReturn = ZERO;
			break;
				
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			lReturn = ZERO;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to long";
			throw CDBException(iErrorCode, strErrorMsg);
			lReturn = ZERO;
			break;
	}

	return lReturn;
}	

CFieldValue::operator float()
{
	if(m_iDataType ==  OCCIFLOAT || m_iDataType == OCCI_SQLT_NUM
		|| m_iDataType == OCCIINT || m_iDataType == OCCIUNSIGNED_INT)
		return (float)m_numValue; 
	
	float fReturn = 0;
	int iErrorCode = 0;
	string strErrorMsg = "";
	switch(m_iDataType)
	{
		case OCCI_SQLT_STR:
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to unsigned int";
			throw CDBException(iErrorCode, strErrorMsg);
			fReturn = ZERO;
			break;
				
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			fReturn = ZERO;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to float";
			throw CDBException(iErrorCode, strErrorMsg);
			fReturn = ZERO;
			break;
	}

	return fReturn;
}	

CFieldValue::operator double()
{
	if(m_iDataType ==  OCCIFLOAT || m_iDataType == OCCI_SQLT_NUM
		|| m_iDataType == OCCIINT || m_iDataType == OCCIUNSIGNED_INT)
		return (double)m_numValue; 
	
	double dReturn = 0;
	int iErrorCode = 0;
	string strErrorMsg = "";
	switch(m_iDataType)
	{
		case OCCI_SQLT_STR:
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to double";
			throw CDBException(iErrorCode, strErrorMsg);
			dReturn = ZERO;
			break;
				
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			dReturn = ZERO;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to double";
			throw CDBException(iErrorCode, strErrorMsg);
			dReturn = ZERO;
			break;
	}

	return dReturn;
}	

CFieldValue::operator string()
{
	if(m_iDataType == OCCI_SQLT_STR || m_iDataType == OCCI_SQLT_CHR
		|| m_iDataType == OCCI_SQLT_AFC || m_iDataType == OCCI_SQLT_VCS
		|| m_iDataType == OCCI_SQLT_AVC)
		return m_strValue;

	string strReturn;
	strReturn="";
	int iErrorCode = 0;
	string strErrorMsg = "";

	Environment* pEnv = NULL;
	
	if(m_numValue.isNull())
		return strReturn;
	switch(m_iDataType)
	{
		case OCCI_SQLT_NUM :
		case OCCIUNSIGNED_INT :
		case OCCIFLOAT :
		case OCCIINT :
			pEnv = Environment::createEnvironment (Environment::OBJECT);
			strReturn = m_numValue.toText(pEnv, "TM");
	    Environment::terminateEnvironment (pEnv);		
			break;

		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			strReturn = "";
			break;
				
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to string";
			throw CDBException(iErrorCode, strErrorMsg);
			strReturn = "";
			break;
		}
		
	return strReturn;
}

CFieldValue::operator Number()
{
	if(m_iDataType == OCCI_SQLT_NUM || m_iDataType == OCCIINT
		|| m_iDataType == OCCIUNSIGNED_INT || m_iDataType == OCCIFLOAT)
		return m_numValue; 

	Number numReturn;
		
	int iErrorCode = 0;
	string strErrorMsg = "";

	switch(m_iDataType)
	{
		case OCCI_SQLT_STR:
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			numReturn = ZERO;
			break;
				
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			numReturn = ZERO;
			break;
				
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert data from datatype ";
			strErrorMsg += "CRecordSet::PrintType(m_iDataType)";
			strErrorMsg += " to string";
			throw CDBException(iErrorCode, strErrorMsg);
			numReturn = ZERO;
			break;
		}
		
	return numReturn;
}

// Class CRecordSet 

CRecordSet::CRecordSet()
{
	m_bOutputErrorMsg = true;
	m_pDBConn = NULL;
	m_pStmt = NULL;
	m_pResultSet = NULL;
}

CRecordSet::~CRecordSet()
{
	try
	{
		if(m_pStmt != NULL)
		{
			//�ͷ���Դ
			m_pStmt->closeResultSet(m_pResultSet);
			m_pDBConn->m_pConn->terminateStatement(m_pStmt);
			m_pStmt = NULL;
	
			m_vColDataSize.clear();
			m_vColName.clear();
			m_vColDataType.clear();
		}
	}
	catch(...)
	{
		m_pStmt=NULL;
	}
}

int CRecordSet::Open(CDatabase& db, const string& strSQL)
{
	int iErrorCode = 0;
	string strErrorMsg;
	//������Ч
	if(strSQL.empty())
	{
		string strMethodName = "int CRecordSet::Open(CDatabase& db, const string strSQL)";

		iErrorCode = INVALID_PARAMETER;
		strErrorMsg = strMethodName + " Call Invalid!SQL String is NULL";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return INVALID_PARAMETER;
	}
	
	//ǰ������������
	if(db.IsConnected() == -1) 	 
	{
		string strMethodName = "int CRecordSet::Open(CDatabase& db, const string strSQL)";
		
		iErrorCode = INVALID_PRE_CONDITION;
		strErrorMsg = strMethodName + " Call Invalid!Database Not Connected yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	int iReturn  = 0;

	try
	{
		//�÷�����Ȼ�򵥣����Ǹı���CDatabase��LastErrorCode������ʹ��
		//db.ExecuteQuery(strSQL, this);
	    	ResultSet* pResultSet = NULL;
    		Statement* pStmt = db.m_pConn->createStatement(strSQL);
		pResultSet = pStmt->executeQuery();
		//��߷��ش�������ʱ��Ч��
		pStmt->setPrefetchRowCount(PREFETCH_ROW_COUNT);
		this->Init(&db, pStmt, pResultSet);
	}
	catch(SQLException ex)
	{
		string strMethodName = "int CRecordSet::Open(CDatabase& db, const string strSQL)";
		
		if(m_bOutputErrorMsg)
		{
			cerr<<strMethodName<<" Failed! SQL String: "<<strSQL<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = ex.getErrorCode();
		//�����׳���׽�����쳣
		THROW_EXCEPTION(iReturn, strMethodName);		
	}
	
    	return iReturn;
}

int CRecordSet::Close()
{
	const char szMethodName[] = "int CRecordSet::Close()";
	if(m_pStmt != NULL)
	{
		//�ͷ���Դ
		m_pStmt->closeResultSet(m_pResultSet);
		m_pDBConn->m_pConn->terminateStatement(m_pStmt);
		m_pStmt = NULL;

		m_vColDataSize.clear();
		m_vColDataType.clear();
		m_vColName.clear();
		return EXECUTE_SUCCEED;
	}
	else 
	{
		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = szMethodName;
		strErrorMsg += " Call Invalid!recordset not initialized yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return INVALID_PRE_CONDITION;
	}
}

int CRecordSet::GetColumnCount()
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = "CRecordSet::GetColumnCount Call Invalid!recordset not available yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<"CRecordSet::GetColumnCount Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	return m_vColDataType.size();
}

//��ʼ����δ����ʼ����CRecordSet ������ʹ��
//pCon��������ڳ�ʼ������pStmt��pRs��Ҫ����ʹ��
//��ȡMetaData���ݣ�����ColIndex��ColName֮��� ӳ��
void CRecordSet::Init(CDatabase* pDB, Statement* pStmt, ResultSet* pRs)
{
	m_pResultSet = pRs;
	m_pDBConn = pDB;
	m_pStmt = pStmt;

	//��ȡ����Ϣ�����������������͡���С��
	vector<MetaData> v1;
	v1 = pRs->getColumnListMetaData();
	m_vColDataSize.clear();
	m_vColDataType.clear();
	m_vColName.clear();
	for(int i=0; i < v1.size(); i++)
      {
		MetaData md = v1[i];
      		m_vColName.push_back(md.getString(MetaData::ATTR_NAME));
      		m_vColDataType.push_back(md.getInt(MetaData::ATTR_DATA_TYPE));
      		m_vColDataSize.push_back(md.getInt(MetaData::ATTR_DATA_SIZE));
      }
}

string CRecordSet::PrintType (int type)
{
    switch (type)
    {
      case OCCI_SQLT_CHR : return "VARCHAR2";
                           break;
      case OCCI_SQLT_NUM : return "NUMBER";
                           break;
      case OCCIINT : return "INTEGER";
                           break;
      case OCCIFLOAT : return "FLOAT";
                           break;
      case OCCI_SQLT_STR : return "STRING";
                           break;
      case OCCI_SQLT_VNU : return "VARNUM";
                           break;
      case OCCI_SQLT_LNG : return "LONG";
                           break;
      case OCCI_SQLT_VCS : return "VARCHAR";
                           break;
      case OCCI_SQLT_RID : return "ROWID";
                           break;
      case OCCI_SQLT_DAT : return "DATE";
                           break;
      case OCCI_SQLT_VBI : return "VARRAW";
                           break;
      case OCCI_SQLT_BIN : return "RAW";
                           break;
      case OCCI_SQLT_LBI : return "LONG RAW";
                           break;
      case OCCIUNSIGNED_INT : return "UNSIGNED INT";
                           break;
      case OCCI_SQLT_LVC : return "LONG VARCHAR";
                           break;
      case OCCI_SQLT_LVB : return "LONG VARRAW";
                           break;
      case OCCI_SQLT_AFC : return "CHAR";
                           break;
      case OCCI_SQLT_AVC : return "CHARZ";
                           break;
      case OCCI_SQLT_RDD : return "ROWID";
                           break;
      case OCCI_SQLT_NTY : return "NAMED DATA TYPE";
                           break;
      case OCCI_SQLT_REF : return "REF";
                           break;
      case OCCI_SQLT_CLOB: return "CLOB";
                           break;
      case OCCI_SQLT_BLOB: return "BLOB";
                           break;
      case OCCI_SQLT_FILE: return "BFILE";
                           break;
      default: return "UNKNOWN TYPE";
    }
} // End of printType (int)

string CRecordSet::GetUpperString(const string& str)
{
	string strReturn;

	char cUpper;
	for(int i = 0; i < str.length(); i ++)
	{
		cUpper = toupper(str.at(i));
		strReturn.append(1, cUpper);
	}
	
	return strReturn;
}

int CRecordSet::GetColIndexByName(const string& strColName)
{
	// ǰ������
	if(m_pResultSet == NULL || m_vColName.size() <= 0)	
	{
		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = "CRecordSet::GetColIndexByName Call Invalid!recordset not available yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<"CRecordSet::GetColIndexByName Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	//��strColNameתΪ��д
	string strUpperColName = GetUpperString(strColName);
	
	for(int i = 0; i < m_vColName.size(); i++)
      {
		string strName = m_vColName[i];
      		if(strName == strUpperColName)
      			return i+1;
      }	
      // Not Found	
      return -1;
}

string CRecordSet::GetAdditionErrorMsg(const string& strColName)
{
	string strErrorMsgAdd = " SQL String: ";
	if(m_pStmt == NULL)
		strErrorMsgAdd += " NOT valid";
	else
		strErrorMsgAdd +=  m_pStmt->getSQL();
	strErrorMsgAdd += ";";
	strErrorMsgAdd +=	" Fetch Column Name: ";
	strErrorMsgAdd += strColName;

	return strErrorMsgAdd;
}

string CRecordSet::GetAdditionErrorMsg(int nColIndex)
{
	string strErrorMsgAdd = " SQL String: ";
	if(m_pStmt == NULL)
		strErrorMsgAdd += " NOT valid";
	else
		strErrorMsgAdd +=  m_pStmt->getSQL();
	strErrorMsgAdd += ";";
	strErrorMsgAdd += " Fetch Column Index: ";
	char szColumnIndex[10];
	memset(szColumnIndex, 0, 10);
	sprintf(szColumnIndex, "%d", nColIndex);	
	strErrorMsgAdd += szColumnIndex;

	return strErrorMsgAdd;
}

int CRecordSet::FetchFieldValue(int nColIndex, CFieldValue& fieldValue)
{
	int iColDataType = m_vColDataType[nColIndex-1];
	int iReturn = 0;

	int iErrorCode = 0;
	string strErrorMsg;
	switch(iColDataType)
	{
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_STR:
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			fieldValue = m_pResultSet->getString(nColIndex);
			break;
			
		case OCCI_SQLT_NUM :
		case OCCIUNSIGNED_INT :
		case OCCIFLOAT :
		case OCCIINT :
			fieldValue = m_pResultSet->getNumber(nColIndex);
			break;

		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "column datatype ";
			strErrorMsg += PrintType(iColDataType);
			strErrorMsg += " now NOT supported";
			throw CDBException(iErrorCode, strErrorMsg);
			iReturn = iErrorCode;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert column data from datatype ";
			strErrorMsg += PrintType(iColDataType);
			strErrorMsg += " to int";
			throw CDBException(iErrorCode, strErrorMsg);
			iReturn = iErrorCode;
			break;
	}

	return iReturn;
}

//��ȡһ��
//��ȷ��ȡ���ط�0ֵ����ȡʧ�ܻ����Ѿ����귵��0
int CRecordSet::ReadRecordAndNext()
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "int CRecordSet::ReadRecordAndNext ()";
	
		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	int iReturn = 0;
	try
	{
		return m_pResultSet->next();
		/*
		ResultSet::Status status = m_pResultSet->next();
		if(status == ResultSet::END_OF_FETCH)	//
			iReturn = 0;
		else
			iReturn = 1;
		*/	
	}
	catch(SQLException ex)
	{
		string strMethodName = "int CRecordSet::ReadRecordAndNext ()";
		
		if(m_bOutputErrorMsg)
		{
			cerr<<strMethodName<<" Failed! "<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}
		//�����񵽵��쳣�����׳�����Ҫ����LONG�ֶ��������쳣��ӦΪӦ���޷���׽
		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		THROW_EXCEPTION(iErrorCode, strErrorMsg);		
		return ZERO;
	}	
}

int CRecordSet::IsEnd()
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "int CRecordSet::IsEnd()";

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	int iReturn = 0;
	try
	{
		ResultSet::Status status = m_pResultSet->status();
		if(status == ResultSet::END_OF_FETCH)
			iReturn = 1;
		else if(status == ResultSet::DATA_AVAILABLE)	//�ݲ�֧������ȡ
			iReturn = -1;
		else		//�������ߵ�����
			iReturn = ZERO;
	}
	catch(SQLException ex)
	{
		string strMethodName = "int CRecordSet::IsEnd()";

		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<strMethodName<<" Failed! "<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = ZERO;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
	}
	return iReturn;
}

int CRecordSet::IsNull(int nColIndex)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "int CRecordSet::IsNull(int nColIndex)";
		
		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "int CRecordSet::IsNull(int nColIndex)";
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}
		
	if(m_pResultSet->isNull(nColIndex) == true)
		return 1;
	else
		return -1;
		
}

int CRecordSet::IsNull(const string& strColName)
{
	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "int CRecordSet::IsNull(const string& strColName)";
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}
	
	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "int CRecordSet::IsNull(const string& strColName)";
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}
	
	return IsNull(nColIndex);
}

string CRecordSet::FetchString(int nColIndex)
{
	string strReturn;	
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "string CRecordSet::FetchString(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		strReturn = "";
		return strReturn;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "string CRecordSet::FetchString(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
 		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		strReturn = "";
		return strReturn;
	}

	int iColDataType = m_vColDataType[nColIndex-1];
	try
	{
		if(iColDataType == OCCI_SQLT_CHR || iColDataType == OCCI_SQLT_AFC
			|| iColDataType == OCCI_SQLT_VCS || iColDataType == OCCI_SQLT_AVC || iColDataType == OCCI_SQLT_STR)
			return m_pResultSet->getString(nColIndex);
		
		//���������ʱ�����ͣ�ת��ΪYYYYMMDDHH24MISS��ʽ�ַ����󷵻�
		else if(iColDataType == OCCI_SQLT_DAT)
		{
			int nYear, nMonth, nDay, nHour, nMinute, nSecond;
			if(ZERO == FetchDateTime(nColIndex, nYear, nMonth, nDay, nHour, nMinute, nSecond))
				return ZERO;
			else
			{
				char szDateTime[15];
				memset(szDateTime, 0, 15);
				sprintf(szDateTime, "%4d%02d%02d%02d%02d%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
				strReturn = szDateTime;
				return strReturn;
			}
		}

		else
		{	
			CFieldValue fieldValue(0);
			FetchFieldValue(nColIndex, fieldValue);
			strReturn = (string)fieldValue;
			
		/*
			//Number numValue = 0;
			int iErrorCode = 0;
			string strErrorMsg;
			switch(iColDataType)
			{
				case OCCI_SQLT_CHR:
				case OCCI_SQLT_AFC:
				case OCCI_SQLT_VCS:
				case OCCI_SQLT_AVC:
				case OCCI_SQLT_STR:						
					return  m_pResultSet->getString(nColIndex);
					break;
			
				case OCCI_SQLT_NUM :
				case OCCIUNSIGNED_INT :
				case OCCIFLOAT :
				case OCCIINT :
					iErrorCode = DATA_TYPE_INVALID_CONVERSION;
					strErrorMsg = "Can NOT convert column data from datatype ";
					strErrorMsg += PrintType(iColDataType);
					strErrorMsg += " to datetime";
					strErrorMsg += strErrorMsgAdd;
					THROW_EXCEPTION(iErrorCode, strErrorMsg);
					strReturn = "";
					break;
										
				case OCCI_SQLT_VNU :
				case OCCI_SQLT_LNG :
				case OCCI_SQLT_RID :
				case OCCI_SQLT_VBI :
				case OCCI_SQLT_BIN :
				case OCCI_SQLT_LBI :
				case OCCI_SQLT_LVC :
				case OCCI_SQLT_LVB :		
				case OCCI_SQLT_RDD :
				case OCCI_SQLT_NTY :
				case OCCI_SQLT_REF :
				case OCCI_SQLT_CLOB :
				case OCCI_SQLT_BLOB :
				case OCCI_SQLT_FILE :
					iErrorCode = DATA_TYPE_NOT_SUPPORTED;
					strErrorMsg = "column datatype ";
					strErrorMsg += PrintType(iColDataType);
					strErrorMsg += " now NOT supported";
					strErrorMsg += strErrorMsgAdd;
					THROW_EXCEPTION(iErrorCode, strErrorMsg);
					strReturn = "";
					break;
					
				default:
					iErrorCode = DATA_TYPE_INVALID_CONVERSION;
					strErrorMsg = "Can NOT convert column data from datatype ";
					strErrorMsg += PrintType(iColDataType);
					strErrorMsg += " to datetime";
					strErrorMsg += strErrorMsgAdd;
					THROW_EXCEPTION(iErrorCode, strErrorMsg);
					strReturn = "";
					break;
			}
			*/
		}
	}
	catch(CDBException ex)
	{
		string strMethodName = "string CRecordSet::FetchString(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.GetErrorCode();
		string strErrorMsg = ex.GetErrorMsg();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		strReturn = "";
	}
	catch(SQLException ex)
	{
		string strMethodName = "string CRecordSet::FetchString(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		strReturn = "";
	}
	return strReturn;
}

string CRecordSet::FetchString(const string& strColName)
{	
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "string CRecordSet::FetchString(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "string CRecordSet::FetchString(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "string CRecordSet::FetchString(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	return FetchString(nColIndex);
}

int CRecordSet::FetchInt(int nColIndex)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "int CRecordSet::FetchInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);
	
		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<< strMethodName <<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "int CRecordSet::FetchInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{	
			cerr<< strMethodName << " Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	int iReturn = 0;
	try
	{
		//����ת����ֱ��ȡ
		//���ܳ���������Ϣ:Code: 32109 Msg: ORA-32109: invalid column or parameter position
		int iColDataType = m_vColDataType[nColIndex-1];
		if(iColDataType == OCCI_SQLT_NUM || iColDataType == OCCIINT
			|| iColDataType == OCCIUNSIGNED_INT || iColDataType == OCCIFLOAT)
		{	
			//Number num = m_pStmt->getNumber(nColIndex);
			//iReturn = (int)num;
			iReturn = m_pResultSet->getInt(nColIndex);
		}
		//��Ҫת����ʹ��CFieldValue
		else
		{
			CFieldValue fieldValue(0);
			FetchFieldValue(nColIndex, fieldValue);
			iReturn = (int)fieldValue;
		}
	}
	catch(CDBException ex)
	{
		string strMethodName = "int CRecordSet::FetchInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.GetErrorCode();
		string strErrorMsg = ex.GetErrorMsg();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		iReturn = ZERO;
	}
	catch(SQLException ex)
	{
		string strMethodName = "int CRecordSet::FetchInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		iReturn = ZERO;
	}
	
	return iReturn;
}

int CRecordSet::FetchInt(const string& strColName)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "int CRecordSet::FetchInt(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);
		
		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "int CRecordSet::FetchInt(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "int CRecordSet::FetchInt(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	return FetchInt(nColIndex);
}

long CRecordSet::FetchLong(int nColIndex)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "long CRecordSet::FetchLong(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName << " Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "long CRecordSet::FetchLong(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<< strMethodName <<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	long lReturn = 0;
	int iColDataType = m_vColDataType[nColIndex-1];
	try
	{
		if(iColDataType == OCCI_SQLT_NUM || iColDataType == OCCIINT
			||iColDataType == OCCIUNSIGNED_INT || iColDataType == OCCIFLOAT)
			lReturn = (long)m_pResultSet->getNumber(nColIndex);
		else
		{
			CFieldValue fieldValue(0);
			FetchFieldValue(nColIndex, fieldValue);
			lReturn = (long)fieldValue;
		}
	}
	catch(CDBException ex)
	{
		string strMethodName = "long CRecordSet::FetchLong(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.GetErrorCode();
		string strErrorMsg = ex.GetErrorMsg();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		lReturn = ZERO;
	}
	catch(SQLException ex)
	{
		string strMethodName = "long CRecordSet::FetchLong(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		lReturn = ZERO;
	}

	return lReturn;
}

long CRecordSet::FetchLong(const string& strColName)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "long CRecordSet::FetchLong(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "long CRecordSet::FetchLong(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "long CRecordSet::FetchLong(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);
		
		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	return FetchLong(nColIndex);
}

unsigned int CRecordSet::FetchUInt(int nColIndex)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "unsigned int CRecordSet::FetchUInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "unsigned int CRecordSet::FetchUInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	unsigned int nReturn = 0;
	int iColDataType = m_vColDataType[nColIndex-1];
	try
	{
		if(iColDataType == OCCIUNSIGNED_INT || iColDataType == OCCI_SQLT_NUM
			||iColDataType == OCCIINT || iColDataType == OCCIFLOAT)
			nReturn = m_pResultSet->getUInt(nColIndex);
		else
		{
			CFieldValue fieldValue(0);
			FetchFieldValue(nColIndex, fieldValue);
			nReturn = (unsigned int)fieldValue;
		}
	}
	catch(CDBException ex)
	{
		string strMethodName = "unsigned int CRecordSet::FetchUInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.GetErrorCode();
		string strErrorMsg = ex.GetErrorMsg();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		nReturn = ZERO;
	}
	catch(SQLException ex)
	{
		string strMethodName = "unsigned int CRecordSet::FetchUInt(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		nReturn = ZERO;
	}

	return nReturn;
}

unsigned int CRecordSet::FetchUInt(const string& strColName)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "unsigned int CRecordSet::FetchUInt(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "unsigned int CRecordSet::FetchUInt(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "unsigned int CRecordSet::FetchUInt(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	return FetchUInt(nColIndex);
}

float CRecordSet::FetchFloat(int nColIndex)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "float CRecordSet::FetchFloat(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "float CRecordSet::FetchFloat(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	float fReturn = 0;
	int iColDataType = m_vColDataType[nColIndex-1];
	try
	{
		if(iColDataType == OCCIFLOAT || iColDataType == OCCI_SQLT_NUM
			||iColDataType == OCCIINT || iColDataType == OCCIUNSIGNED_INT)
			fReturn = m_pResultSet->getFloat(nColIndex);	
		else
		{
			CFieldValue fieldValue(0);
			FetchFieldValue(nColIndex, fieldValue);
			fReturn = (float)fieldValue;
		}
	}
	catch(CDBException ex)
	{
		string strMethodName = "float CRecordSet::FetchFloat(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.GetErrorCode();
		string strErrorMsg = ex.GetErrorMsg();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		fReturn = ZERO;
	}
	catch(SQLException ex)
	{
		string strMethodName = "float CRecordSet::FetchFloat(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		fReturn = ZERO;
	}

	return fReturn;
}

float CRecordSet::FetchFloat(const string& strColName)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "float CRecordSet::FetchFloat(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "float CRecordSet::FetchFloat(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "float CRecordSet::FetchFloat(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	return FetchFloat(nColIndex);
}

double CRecordSet::FetchDouble(int nColIndex)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "double CRecordSet::FetchDouble(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "double CRecordSet::FetchDouble(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	double dReturn = 0;
	int iColDataType = m_vColDataType[nColIndex-1];
	try
	{
		if(iColDataType == OCCIFLOAT || iColDataType == OCCI_SQLT_NUM
			||iColDataType == OCCIINT || iColDataType == OCCIUNSIGNED_INT)
			dReturn = m_pResultSet->getDouble(nColIndex);
		else
		{
			CFieldValue fieldValue(0);
			FetchFieldValue(nColIndex, fieldValue);
			dReturn = (double)fieldValue;
		}
	}
	catch(CDBException ex)
	{
		string strMethodName = "double CRecordSet::FetchDouble(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.GetErrorCode();
		string strErrorMsg = ex.GetErrorMsg();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		dReturn = ZERO;
	}
	catch(SQLException ex)
	{
		string strMethodName = "double CRecordSet::FetchDouble(int nColIndex)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = ex.getErrorCode();
		string strErrorMsg = ex.getMessage();
		strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< iErrorCode <<" Msg: "<< strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		dReturn = ZERO;
	}

	return dReturn;
}

double CRecordSet::FetchDouble(const string& strColName)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "double CRecordSet::FetchDouble(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "double CRecordSet::FetchDouble(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "double CRecordSet::FetchDouble(const string& strColName)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	return FetchDouble(nColIndex);
}

int CRecordSet::FetchDateTime(int nColIndex, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "int CRecordSet::FetchDateTime(int nColIndex, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(nColIndex <= 0 || nColIndex >m_vColName.size())	
	{
		string strMethodName = "int CRecordSet::FetchDateTime(int nColIndex, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)";
		string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColIndex too small or too large";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	int iColDataType = m_vColDataType[nColIndex-1];
	if(iColDataType == OCCI_SQLT_DAT)
	{
		Date date = m_pResultSet->getDate(nColIndex);
		int year;
		unsigned int month, day, hour, minute, second;
		date.getDate(year, month, day, hour, minute, second);

		nYear = year;
		nMonth = month;
		nDay = day;
		nHour = hour;
		nMinute = minute;
		nSecond = second;
		
		return EXECUTE_SUCCEED;
	}
	
	int iReturn = 0;
	string strMethodName = "int CRecordSet::FetchDateTime(int nColIndex, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)";
	string strErrorMsgAdd = GetAdditionErrorMsg(nColIndex);
	
	Number numValue = 0;
	int iErrorCode = 0;
	string strErrorMsg = "";
	switch(iColDataType)
	{
		case OCCI_SQLT_CHR:
		case OCCI_SQLT_STR:	
		case OCCI_SQLT_VCS:
		case OCCI_SQLT_AFC:
		case OCCI_SQLT_AVC:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert column data from datatype ";
			strErrorMsg += PrintType(iColDataType);
			strErrorMsg += " to datetime";
			strErrorMsg += strErrorMsgAdd;
			THROW_EXCEPTION(iErrorCode, strErrorMsg);
			iReturn = ZERO;
			break;

		case OCCI_SQLT_NUM :
		case OCCIUNSIGNED_INT :
		case OCCIFLOAT :
		case OCCIINT :
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert column data from datatype ";
			strErrorMsg += PrintType(iColDataType);
			strErrorMsg += " to datetime";
			strErrorMsg += strErrorMsgAdd;
			THROW_EXCEPTION(iErrorCode, strErrorMsg);
			iReturn = ZERO;
			break;
			
		case OCCI_SQLT_VNU :
		case OCCI_SQLT_LNG :
		case OCCI_SQLT_RID :
		case OCCI_SQLT_VBI :
		case OCCI_SQLT_BIN :
		case OCCI_SQLT_LBI :
		case OCCI_SQLT_LVC :
		case OCCI_SQLT_LVB :		
		case OCCI_SQLT_RDD :
		case OCCI_SQLT_NTY :
		case OCCI_SQLT_REF :
		case OCCI_SQLT_CLOB :
		case OCCI_SQLT_BLOB :
		case OCCI_SQLT_FILE :
			iErrorCode = DATA_TYPE_NOT_SUPPORTED;
			strErrorMsg = "column datatype ";
			strErrorMsg += PrintType(iColDataType);
			strErrorMsg += " now NOT supported";
			strErrorMsg += strErrorMsgAdd;
			THROW_EXCEPTION(iErrorCode, strErrorMsg);
			iReturn = ZERO;
			break;
			
		default:
			iErrorCode = DATA_TYPE_INVALID_CONVERSION;
			strErrorMsg = "Can NOT convert column data from datatype ";
			strErrorMsg += PrintType(iColDataType);
			strErrorMsg += " to datetime";
			strErrorMsg += strErrorMsgAdd;
			THROW_EXCEPTION(iErrorCode, strErrorMsg);
			iReturn = ZERO;
			break;
	}
	
	return iReturn;
}

int CRecordSet::FetchDateTime(const string& strColName, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)
{
	// ǰ������
	if(m_pResultSet == NULL)	
	{
		string strMethodName = "int CRecordSet::FetchDateTime(const string& strColName, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PRE_CONDITION;
		string strErrorMsg = strMethodName + " Call Invalid!recordset not available yet";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;
	}

	//��Ч����
	if(strColName.empty())
	{
		string strMethodName = "int CRecordSet::FetchDateTime(const string& strColName, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName is NULL";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}

	int nColIndex = GetColIndexByName(strColName);

	if(nColIndex < 0)	// Not Found
	{
		string strMethodName = "int CRecordSet::FetchDateTime(const string& strColName, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond)";
		string strErrorMsgAdd = GetAdditionErrorMsg(strColName);

		int iErrorCode = INVALID_PARAMETER;
		string strErrorMsg = strMethodName + " Call Invalid!ColName: ";
		strErrorMsg += strColName;
		strErrorMsg += " NOT found in recordset";
		strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<iErrorCode<<" Msg: "<<strErrorMsg<<endl;
		}

		THROW_EXCEPTION(iErrorCode, strErrorMsg);
		return ZERO;		
	}
	
	return FetchDateTime(nColIndex, nYear, nMonth, nDay, nHour, nMinute, nSecond);
}

CBindSQL::CBindSQL(CDatabase& db)
{
	// ���캯���������׳��쳣
	m_pDBConn = &db;

	m_pStmt = NULL;
	m_pRecordSet = NULL;
	
	m_bOutputErrorMsg = true;
	m_eSQLType = INVALID_TYPE;

	m_nInputParamPos = 0;
	m_nOutputColumnPos = 0;

	m_iErrorCode = 0;
	m_strErrorMsg = "Success";	

	m_nInputParamCount = 0;

	m_nOutputColumnIndex	 = 0;
}

CBindSQL::CBindSQL()
{	
}

CBindSQL::CBindSQL(int id)
{
	// ���캯���������׳��쳣
	m_pDBConn = &DBPool[id];

	m_pStmt = NULL;
	m_pRecordSet = NULL;
	
	m_bOutputErrorMsg = true;
	m_eSQLType = INVALID_TYPE;

	m_nInputParamPos = 0;
	m_nOutputColumnPos = 0;

	m_iErrorCode = 0;
	m_strErrorMsg = "Success";	

	m_nInputParamCount = 0;

	m_nOutputColumnIndex	 = 0;
}


void CBindSQL::Init(CDatabase& db)
{
	// ���캯���������׳��쳣
	m_pDBConn = &db;

	m_pStmt = NULL;
	m_pRecordSet = NULL;
	
	m_bOutputErrorMsg = true;
	m_eSQLType = INVALID_TYPE;

	m_nInputParamPos = 0;
	m_nOutputColumnPos = 0;

	m_iErrorCode = 0;
	m_strErrorMsg = "Success";	

	m_nInputParamCount = 0;

	m_nOutputColumnIndex	 = 0;
}
int CBindSQL::Commit()
{
	return m_pDBConn->Commit();
}
int CBindSQL::Rollback()
{
	return m_pDBConn->Rollback();
}

CBindSQL::~CBindSQL()
{
	//������ʱ��ִ��Close������ȷ����Դ�ͷ�
	Close();
	/*
	m_varInputParam.clear();
	if(m_pRecordSet != NULL)	//SELECT QUERY
	{
	    	delete m_pRecordSet;
	    	// CRecordSet�����������ͷ������Դ
	}
	else		// DELETE ��UPDARE��INSERT DML Statements
	{
		if(m_eSQLType != SELECT_QUERY && m_eSQLType != INVALID_TYPE)
		    	m_pConn->terminateStatement (m_pStmt);	
	}
	*/
}

int CBindSQL::GetUpdateCount()
{
	return m_effectCount;
}
int CBindSQL::Open(const string& strSQL, int eSQLType)
{	
	//���ǵ����ϰ�߲�ִ��Close()�������Լ�Exception֮��
	//�޷�ִ��Close()����
	//Ϊ���ӳ���׳�ԣ���Open֮ǰ����ִ��Close()����
	Close();
	
	//��Ч����
	if(strSQL.empty())
	{
		string strMethodName = "int CBindSQL::Open  "+strSQL;
		
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = strMethodName + " Call Invalid!SQL String is NULL";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName + " Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}
                //2004-12-20
                //add by wulei
                Close();
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}

	// ǰ������
	if(m_pDBConn->m_pConn == NULL)	
	{
		string strMethodName = "int CBindSQL::Open  "+strSQL;
		
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = strMethodName + " Call Invalid!BindSQL not connected to database yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}
                //2004-12-20
                //add by wulei
                Close();
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	//��Ч����
	if(eSQLType != SELECT_QUERY && eSQLType != NONSELECT_DML 
		&& eSQLType != FUNCTION_OR_PROCEDURE && eSQLType != SQL_DDL)
	{
		string strMethodName = "int CBindSQL::Open "+strSQL;
		
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = strMethodName + " Call Invalid!parameter sql type not valid ";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}
                //2004-12-20
                //add by wulei
                Close();
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}

	//�����Close֮ǰOpen,����Close֮
	//������Ϊǰ������������,�ӳ��쳣?
	if(m_eSQLType != INVALID_TYPE)
	{
	//	Close();
		string strMethodName = "int CBindSQL::Open  "+strSQL;
		
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = strMethodName + " Call Invalid!parameter sql type not initialized, Maybe Re-Open before Closed, Please Close First!";

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}
                //2004-12-20
                //add by wulei
                Close();
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}
	
	m_eSQLType = eSQLType;
	m_effectCount=0;
	try
	{
		//����Ѿ������ȹر�֮
		//if(m_pStmt == NULL)
		//	Close();

		//			Ϊ���ظ�ִ��
		//			m_pStmt->setSQL(strSQL);
		if(m_eSQLType == SELECT_QUERY)
		{
			m_pStmt = m_pDBConn->m_pConn->createStatement(strSQL);
			//For SELECT Query to enhance performance, set the number of rows to prefetch in each round-trip to the server
			m_pStmt->setPrefetchRowCount(PREFETCH_ROW_COUNT);
		}
		else if(m_eSQLType == NONSELECT_DML)
		{
			m_pStmt = m_pDBConn->m_pConn->createStatement(strSQL);
			//AutoCommit��Ĭ��ֵ����false������ע�͵�����
			//m_pStmt->setAutoCommit(false);
		}
		else if(m_eSQLType == SQL_DDL)
		{
			m_pStmt = m_pDBConn->m_pConn->createStatement(strSQL);
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			if(GetProcInfo(strSQL) == 0)
			{
				string strProcSQL = GetExecuteProcSQL(strSQL);
				m_pStmt = m_pDBConn->m_pConn->createStatement(strProcSQL);
			}
		}
		else;

		int nParamCount = FigureInputParamCount(strSQL, eSQLType);
		SetInputParamCount(nParamCount);
		
		m_iErrorCode = 0;
		m_strErrorMsg = "Success";
	}
	catch(SQLException ex)
	{
		string strMethodName = "int CBindSQL::Open  "+strSQL;
		
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<strMethodName<<" Failed! SQL String: "<<strSQL<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg << endl;
		}
                //2004-12-20
                //add by wulei
                Close();
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	return m_iErrorCode;

}

//  �洢�������
int CBindSQL::GetProcInfo(const string& strProcName)
{
	m_vProcParamDataType.clear();
	m_vProcParamMode.clear();
	m_vProcParamName.clear();

	try
	{
		MetaData metaData = m_pDBConn->m_pConn->getMetaData(strProcName);
		int iObjectType = metaData.getInt(MetaData::ATTR_PTYPE);
	    	if ( iObjectType != MetaData::PTYPE_PROC && iObjectType != MetaData::PTYPE_FUNC)
	    	{
			m_iErrorCode = INVALID_PARAMETER;
			m_strErrorMsg = "CBindSQL::GetProcInfo Call Invalid!";
			m_strErrorMsg +=strProcName;
			m_strErrorMsg +=" is NOT a function or stored procedure";

			if(m_bOutputErrorMsg)
			{	
				cerr<<"CBindSQL::GetProcInfo Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}

			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return INVALID_PARAMETER;
	    	}

	    	vector<MetaData> v1 =  metaData.getVector ( MetaData::ATTR_LIST_ARGUMENTS );
	    	int iSize = v1.size();
	    	m_vProcParamDataType.resize(iSize);
	    	m_vProcParamMode.resize(iSize);
	    	m_vProcParamName.resize(iSize);

	    	for(int i=0; i < v1.size(); i++)
	      {
		      MetaData md = v1[i];
		      m_vProcParamName[i] = md.getString(MetaData::ATTR_NAME);
		      m_vProcParamDataType[i] = md.getInt(MetaData::ATTR_DATA_TYPE);
		      m_vProcParamMode[i] = md.getInt (MetaData::ATTR_IOMODE);
/*		      int mode = md.getInt (MetaData::ATTR_IOMODE);
		      if (mode == 0)
		        cout << "IN" << endl;
		      if (mode == 1)
		        cout << "OUT" << endl;
		      if (mode == 2)
		        cout << "IN/OUT" << endl;
*/		        	        
	      }
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<"CBindSQL::GetProcInfo Failed! Proc Name: "<<strProcName<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
		
	return m_iErrorCode;
}
string CBindSQL::GetExecuteProcSQL(const string& strProcName)
{
	// ����������BEGIN PROC_NAME(:v1, :v2, :v3); END;���ַ�����ִ�д洢����
	string strReturn = "BEGIN ";
	strReturn += strProcName;
	strReturn +="(";

	for(int i=0; i< m_vProcParamName.size(); i ++)
	{
		strReturn +=	":";
		strReturn += m_vProcParamName[i];
		if(i ==  m_vProcParamName.size() -1 ) // ���һ��
			strReturn += "); ";
		else
			strReturn += ", ";
	}
	if(0==m_vProcParamName.size())
		strReturn += "); END;";
	else
		strReturn += "END;";
	return strReturn;
}

int CBindSQL::FigureInputParamCount(const string& strSQL, int eSQLType)
{
	int nCount = 0;
	if(eSQLType == SELECT_QUERY || eSQLType == NONSELECT_DML || eSQLType == SQL_DDL)
	//ͨ������SQL�ַ����е�ð�Ÿ�����ȷ������󶨱����ĸ���	
	{
		for(int nIndex = 0; nIndex < strSQL.length(); nIndex ++)
		{
			if(strSQL[nIndex] == ':')
				nCount ++;
		}
		return nCount;
	}

	else if(eSQLType == FUNCTION_OR_PROCEDURE)
	{
		for(int nIndex = 0; nIndex < m_vProcParamDataType.size(); nIndex ++)
		{	
			printf("m_vInputParamDataType.size()=%d\n",m_vProcParamDataType.size());
			int iMode = m_vProcParamDataType[nIndex];
			if( iMode == IN || iMode == INOUT)
				nCount ++;
		}
	}
	
	else;

	return nCount;
}

//���SQLֵ��������Ϣ��
string CBindSQL::GetInputAdditionErrorMsg(int nColIndex)
{
	string strErrorMsgAdd = " SQL String ";
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;
	
	if(m_pStmt == NULL)
		strErrorMsgAdd += " NOT valid";
	else
		strErrorMsgAdd +=  m_pStmt->getSQL();

	//ָ���к���Ϣ
	if(nColIndex > 0)
	{
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_3;

		strErrorMsgAdd += " Param Index ";
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;
		char szColumnIndex[10];
		memset(szColumnIndex, 0, 10);
		sprintf(szColumnIndex, "%d", nColIndex);	
		strErrorMsgAdd += szColumnIndex;
	}

	return strErrorMsgAdd;

}

//���SQL�Ͱ󶨱���ֵ��������Ϣ��
string CBindSQL::GetInputAdditionErrorMsg(CFieldValue& paramValue,int nColIndex)
{
	string strErrorMsgAdd = " SQL String ";
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;
	
	if(m_pStmt == NULL)
		strErrorMsgAdd += " NOT valid";
	else
		strErrorMsgAdd +=  m_pStmt->getSQL();

	//ָ���к���Ϣ
	if(nColIndex > 0)
	{
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_3;

		strErrorMsgAdd += " Param Index ";
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;

		char szColumnIndex[10];
		memset(szColumnIndex, 0, 10);
		sprintf(szColumnIndex, "%d", nColIndex);	
		strErrorMsgAdd += szColumnIndex;
	}

	//����󶨱�����Ϣ
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_3;
	strErrorMsgAdd += " Input Bind Variable ";
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;

	string strParamValue = (string)paramValue;
	strErrorMsgAdd += strParamValue;
	
	return strErrorMsgAdd;

}

//���SQL�Ͱ󶨱���ֵ��������Ϣ��
string CBindSQL::GetOutputAdditionErrorMsg(int nColIndex)
{
	string strErrorMsgAdd = " SQL String ";
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
	strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;

	if(m_pStmt == NULL)
		strErrorMsgAdd += " NOT valid";
	else
		strErrorMsgAdd +=  m_pStmt->getSQL();

	//ָ���к���Ϣ
	if(nColIndex > 0)
	{
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_3;

		strErrorMsgAdd += " Column Index ";
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;

		char szColumnIndex[10];
		memset(szColumnIndex, 0, 10);
		sprintf(szColumnIndex, "%d", nColIndex);	
		strErrorMsgAdd += szColumnIndex;
	}

	//����󶨱�����Ϣ
	if(m_varInputParam.size() > 0)
	{
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_3;
		strErrorMsgAdd += " Input Bind Variables ";
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_1;
		strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_4;

		int i = 0;
		while(i < m_varInputParam.size())
		{
			string strParamValue = (string)m_varInputParam[i];
			strErrorMsgAdd += strParamValue;

			i ++;
			strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_2;
			/*
			if(i % GetInputParamCount() == 0)
				strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_3;
			else
				strErrorMsgAdd += C_ERROR_INFO_SEPERATOR_2;
			*/	
		}
	}
	
	return strErrorMsgAdd;
}
	
//CRecordSet�ӿڣ�������SELECT ��ѯ��
//����ͨ���˽ӿ�ѡ��ʹ��CRecordSet�������������
//ע��:ͨ�����ӿڻ�õ�ָ�벻��Ҫdelete
CRecordSet* CBindSQL::GetRecordSet()
{
	if(m_pRecordSet == NULL || m_eSQLType != SELECT_QUERY)
	{
		return ZERO;
	}
	else
		return m_pRecordSet;
}

//�������DataBuffer ���ݣ����ڲ���
int CBindSQL::DumpOut()
{
	int nIndex = 0, nRow = 0, iDataType = 0;
	int nRows = m_nInputParamPos/GetInputParamCount() + 1;
	if(m_nInputParamPos % GetInputParamCount()  == 0) 
		nRows = nRows -1;
	
	long* pLong = NULL;
	double* pDouble = NULL;
	unsigned long* pULong = NULL;
	char* pChar = NULL;
	
	// Set Data Buffer for IN bind Variables
	for(nIndex = 0; nIndex < m_vpData.size(); nIndex ++)
	{
		void* pData = m_vpData[nIndex];
		iDataType = m_vInputParamDataType[nIndex];
		ub2* pLength = m_vpLength[nIndex];
		if(iDataType == OCCIINT)
		{
			pLong = (long*)pData;
			for(nRow = 0; nRow < nRows; nRow ++)
			{
				if(pLong[nRow]==NULL)
					cout<<"Index : " <<nIndex<< " Row: "<<nRow<<"is NULL:OCCIINT"<<endl;
				//cout<<"Index : " <<nIndex<< " Row: "<<nRow<<" Value: " << pLong[nRow]  << endl;
				//cout<<"Size: " << pLength[nRow] << endl;
			}
		}
			
		else if(iDataType == OCCIFLOAT)
		{
			pDouble = (double*)pData;
			for(nRow = 0; nRow < nRows; nRow ++)
			{
				if(pDouble[nRow]==NULL)
					cout<<"Index : " <<nIndex<< " Row: "<<nRow<<"is NULL:OCCIFLOAT"<<endl;
				//cout<<"Index : " <<nIndex<< " Row: "<<nRow<<" Value: " << pDouble[nRow]  << endl;
				//cout<<"Size: " << pLength[nRow] << endl;	
			}
		}

		else if(iDataType == OCCIUNSIGNED_INT)
		{
			pULong = (unsigned long*)pData;
			for(nRow = 0; nRow < nRows; nRow ++)
			{
				if(pULong[nRow]==NULL)
					cout<<"Index : " <<nIndex<< " Row: "<<nRow<<"is NULL:OCCIUNSIGNED_INT"<<endl;	
				//cout<<"Index : " <<nIndex<< " Row: "<<nRow<<" Value: " << pULong[nRow]  << endl;
				//cout<<"Size: " << pLength[nRow] << endl;
			}
		}

		else if(iDataType == OCCI_SQLT_AFC)
		{
			pChar = (char*)pData;
			for(nRow = 0; nRow < nRows; nRow ++)
			{
				if(pChar==NULL)
					cout<<"Index : " <<nIndex<< " Row: "<<nRow<<"is NULL:OCCI_SQLT_AFC"<<endl;	
				//cout<<"Index : " <<nIndex<< " Row: "<<nRow<<" Value: " << pChar  << endl;
				int iSize = pLength[nRow];
				//cout<<" String Size is: " << iSize << endl;
				pChar += iSize;
			}
		}

		else if(iDataType == OCCI_SQLT_STR)
		{
			pChar = (char*)pData;
			for(nRow = 0; nRow < nRows; nRow ++)
			{
				if(pChar==NULL)
					cout<<"Index : " <<nIndex<< " Row: "<<nRow<<"is NULL:OCCI_SQLT_STR"<<endl;
				//cout<<"Index : " <<nIndex<< " Row: "<<nRow<<" Value: " << pChar  << endl;
				int iSize = pLength[nRow];
				//cout<<" String Size is: " << iSize << endl;
				pChar += iSize;
			}  
		}    
		
		else if(iDataType == OCCI_SQLT_CHR)
		{
			pChar = (char*)pData;
			for(nRow = 0; nRow < nRows; nRow ++)
			{
				int iSize = pLength[nRow];
				char *pStr = new char[iSize+1];
				memset(pStr, 0, iSize+1);
				strncpy(pStr, pChar, iSize);
				if(pStr==NULL)
					cout<<"Index : " <<nIndex<< " Row: "<<nRow<<"is NULL:OCCI_SQLT_CHR"<<endl;
				//cout<<"Index : " <<nIndex<< " Row: "<<nRow<<" Value: " << pStr  << endl;
				//cout<<" String Size is: " << iSize << endl;
				pChar += iSize;
				delete pStr;
				pStr=NULL;
			}
		}

		else
			return -1;
	}
	return 0;
}

//������󶨱���ת������䵽DataBuffer�У�Ϊִ��executeArrayUpdate��׼��
int CBindSQL::PrepareArrayUpdateData()
{
		m_vpData.clear();
		m_vpLength.clear();
		m_vpData.resize(GetInputParamCount());
		m_vpLength.resize(GetInputParamCount());

		int nIndex = 0, nPos = 0, nRow = 0, iDataType = 0;
		string strValue;
		// ����ִ�еĴ���
		int nRows = m_nInputParamPos/GetInputParamCount() + 1;
		if(m_nInputParamPos % GetInputParamCount()  == 0) 
			nRows = nRows -1;
			
		for(nIndex = 0; nIndex < m_vpLength.size(); nIndex ++)
		{
			m_vpLength[nIndex] = new ub2[nRows];
			memset(m_vpLength[nIndex], 0, sizeof(ub2)*nRows);
		}

		//Caculate Buffer Length
		for(nPos = 0; nPos < m_varInputParam.size(); nPos ++)
		{
			nIndex = nPos % GetInputParamCount();
			nRow = nPos / GetInputParamCount();
			iDataType = m_vInputParamDataType[nIndex];
			switch(iDataType)
			{
				case OCCIINT:
					m_vpLength[nIndex][nRow] = sizeof(long);
					break;

				case OCCIFLOAT:
					m_vpLength[nIndex][nRow] = sizeof(double);
					break;

				case OCCIUNSIGNED_INT:
					m_vpLength[nIndex][nRow] = sizeof(unsigned long);						
					break;

				case OCCI_SQLT_AFC: // Char
					strValue = (string)m_varInputParam[nPos];
					m_vpLength[nIndex][nRow] = strValue.size() + 1;
					break;
					
				case OCCI_SQLT_AVC: // CharZ
					strValue = (string)m_varInputParam[nPos];
					m_vpLength[nIndex][nRow] = strValue.size() + 1;
					break;

				case OCCI_SQLT_CHR: // Varchar2
					strValue = (string)m_varInputParam[nPos];
					if(strValue.size() == 0)
						m_vpLength[nIndex][nRow] = 1;
					else
						m_vpLength[nIndex][nRow] = strValue.size();						
					break;

				case OCCI_SQLT_STR:	// String
					strValue = (string)m_varInputParam[nPos];
					//ע�⣬����������8��NULL������
					//2004-11-18 Ѫ�Ĵ���!
					m_vpLength[nIndex][nRow] = strValue.size() + 8;
					break;

				case OCCI_SQLT_VCS:	// Varchar
					strValue = (string)m_varInputParam[nPos];
					m_vpLength[nIndex][nRow] = strValue.size() + sizeof(short);
					break;

				default:
					m_iErrorCode = DATA_TYPE_NOT_SUPPORTED;
					m_strErrorMsg = "param datatype ";
					m_strErrorMsg += CRecordSet::PrintType(iDataType);
					m_strErrorMsg += " now NOT supported";
					THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
					return m_iErrorCode;
					break;
			}
		}
			
		//Allocate Data Buffer
		for(nIndex = 0; nIndex < m_vpData.size(); nIndex ++)
		{
			iDataType = m_vInputParamDataType[nIndex];

			//�ַ������͵�������
			if(iDataType == OCCI_SQLT_AFC || iDataType == OCCI_SQLT_CHR || iDataType == OCCI_SQLT_STR
			||iDataType == OCCI_SQLT_AVC || iDataType == OCCI_SQLT_VCS)
			{
				int iSize = 0;
				int maxLen=0;
				for(nRow = 0; nRow < nRows; nRow ++)
				{
					// 2005-11-23 by wh ȡ�������Ϊһ��Ԫ�صĳ���
					// iSize += m_vpLength[nIndex][nRow];
					if (maxLen < m_vpLength[nIndex][nRow]) 
						maxLen = m_vpLength[nIndex][nRow];
					 
				}
				
				// 2005-11-23 by wh ȡ�������Ϊһ��Ԫ�صĳ���
				for(nRow = 0; nRow < nRows; nRow ++)
				{
					m_vpLength[nIndex][nRow] = maxLen;					 
				}
				iSize = maxLen*nRow;
				 
				m_vpData[nIndex] = new char[iSize];
				memset(m_vpData[nIndex], 0, iSize);
			}

			else
			switch(iDataType)
			{
				case OCCIINT:
					m_vpData[nIndex] = new long[nRows];
					memset(m_vpData[nIndex], 0, sizeof(long)*nRows);
					break;

				case OCCIFLOAT:
					m_vpData[nIndex] = new double[nRows];
					memset(m_vpData[nIndex], 0, sizeof(double)*nRows);
					break;

				case OCCIUNSIGNED_INT:
					m_vpData[nIndex] = new unsigned long[nRows];						
					memset(m_vpData[nIndex], 0, sizeof(unsigned long)*nRows);
					break;

				default:
					break;
			}
									
		}

		// Copy Data to Buffer from vecotr
		for(nPos = 0; nPos < m_varInputParam.size(); nPos ++)
		{
			nIndex = nPos % GetInputParamCount();
			nRow = nPos / GetInputParamCount();
			iDataType = m_vInputParamDataType[nIndex];
				
			void* pData = m_vpData[nIndex];
			CFieldValue& fieldValue = m_varInputParam[nPos];

			long* pLong = NULL;
			double* pDouble = NULL;
			unsigned long* pULong = NULL;
			char* pChar = NULL;

			if(iDataType == OCCI_SQLT_AFC || iDataType == OCCI_SQLT_CHR || iDataType == OCCI_SQLT_STR
			||iDataType == OCCI_SQLT_AVC || iDataType == OCCI_SQLT_VCS )
			{
				strValue = (string)fieldValue;
				pChar = (char*)pData;
				for(int i = 0; i < nRow; i ++)
					pChar += m_vpLength[nIndex][i];
				//��Ϊ�·����Buffer����NULL(0)��䣬���Բ��ùܸ���null������������
				strValue.copy(pChar, strValue.size());
			}

			else
			switch(iDataType)
			{
				case OCCIINT:
					pLong = (long*)pData;
					pLong[nRow] = (long)fieldValue;
					break;

				case OCCIFLOAT:
					pDouble = (double*)pData;
					pDouble[nRow] = (double)fieldValue;
					break;

				case OCCIUNSIGNED_INT:
					pULong = (unsigned long*)pData;
					pULong[nRow] = (unsigned long)fieldValue;
					break;
						
				default:
					break;
			}
		}
	return 0;
}

int CBindSQL::Execute()
{
	// ǰ������
	m_effectCount=0;
	if(m_pStmt == NULL)
	{
		string strMethodName = "int CBindSQL::Execute()";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg();
		
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = strMethodName + " Call Invalid!BindSQL statement not initialized yet";
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}
                //2004-12-20
                //add by wulei
                Close();
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	// For Re-execution, wipe off this check
	/*
	Statement::Status status = m_pStmt->status();
	if(status != Statement::PREPARED)
	{
		string strMethodName = "int CBindSQL::Execute()";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg();
		
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = strMethodName + " Call Invalid!BindSQL statement not prepared or already executed";
		m_strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<<strMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}
	*/
	
	try
	{
		if(m_eSQLType == SELECT_QUERY)	// SELECT QUERY
		{
			m_pRecordSet = new CRecordSet();
			ResultSet* pRs = m_pStmt->executeQuery();
			m_pRecordSet->Init(m_pDBConn,m_pStmt, pRs);
		}
		//ִ�в�����󶨱�����SQL 
		else if(m_eSQLType == NONSELECT_DML )
		{
			// ����ִ��
			if(m_nInputParamPos > GetInputParamCount())
			{
				int nIndex =0, iDataType = 0;
				int nRows = m_nInputParamPos/GetInputParamCount() + 1;
				if(m_nInputParamPos % GetInputParamCount() == 0) 
					nRows = nRows - 1;

				PrepareArrayUpdateData();

				//�������DataBuffer ���ݣ����ڲ���
				//DumpOut();
					
				// Set Data Buffer for IN bind Variables
				for(nIndex = 0; nIndex < m_vpData.size(); nIndex ++)
				{
					void* pData = m_vpData[nIndex];
					iDataType = m_vInputParamDataType[nIndex];
					ub2* pLength = m_vpLength[nIndex];
					switch(iDataType)
					{
						case OCCIINT:
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCIINT, sizeof(long), pLength);
							break;

						case OCCIFLOAT:
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCIFLOAT, sizeof(double), pLength);
							break;

						case OCCIUNSIGNED_INT:
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCIUNSIGNED_INT, sizeof(unsigned long), pLength);
							break;

						case OCCI_SQLT_AFC: // Char
							//m_pStmt->setDataBuffer(nIndex+1, pData, OCCI_SQLT_AFC, sizeof((char*)pData), pLength);
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCI_SQLT_AFC, pLength[0], pLength);
							break;

						case OCCI_SQLT_AVC: // CharZ
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCI_SQLT_AVC, pLength[0], pLength);
							break;
							
						case OCCI_SQLT_VCS: // Varchar
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCI_SQLT_VCS, pLength[0], pLength);
							break;
							
						case OCCI_SQLT_CHR: // Varchar2
							//m_pStmt->setDataBuffer(nIndex+1, pData, OCCI_SQLT_CHR, sizeof((char*)pData), pLength);
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCI_SQLT_CHR, pLength[0], pLength);
							break;

						case OCCI_SQLT_STR: // String
							m_pStmt->setDataBuffer(nIndex+1, pData, OCCI_SQLT_STR, pLength[0], pLength);
							break;

						default:
							break;
					}
				}

				m_pStmt->executeArrayUpdate(nRows);
				m_effectCount=m_pStmt->getUpdateCount();//add by yangh

				// Free Memory Allocated
				for(nIndex = 0; nIndex < m_vpData.size(); nIndex++)
				{
					delete[] m_vpData[nIndex];	
					m_vpData[nIndex]=NULL;					
				}
				//�������Ŀռ�m_vpLength add by yangh 2006-04-30
				for(nIndex = 0; nIndex < m_vpLength.size(); nIndex++)
				{
					delete[] m_vpLength[nIndex];	
					m_vpLength[nIndex]=NULL;
				}				
				// For Re-execution
				m_pStmt->setSQL(m_pStmt->getSQL());

				//��λ
				m_vpData.clear();
				m_vpLength.clear();
				m_varInputParam.clear();
				m_vInputParamDataType.clear();

				m_nInputParamPos = 0;
				m_nOutputColumnPos = 0;
				m_nOutputColumnIndex	 = 0;
			}

			//�����󶨱������ߵ���ִ��
			else
			{
				for(int nIndex = 0; nIndex < m_varInputParam.size(); nIndex ++)
				{
					switch(m_vInputParamDataType[nIndex])
					{
						case OCCIINT:
						case OCCIFLOAT:
						case OCCIUNSIGNED_INT:
						case OCCI_SQLT_NUM:		
							{
								Number numValue = m_varInputParam[nIndex].operator Number();
								m_pStmt->setNumber(nIndex+1, numValue);
							}
							break;

						case OCCI_SQLT_STR:
						case OCCI_SQLT_AFC:
						case OCCI_SQLT_CHR:
						case OCCI_SQLT_VCS:
						case OCCI_SQLT_AVC:
							m_pStmt->setString(nIndex+1, (string)m_varInputParam[nIndex]);
							break;

						default:
							//�����ܵ����
							break;
					}		
				}
				
				m_pStmt->executeUpdate();
				m_effectCount=m_pStmt->getUpdateCount();//add by yangh
				// For Re-execution
				m_pStmt->setSQL(m_pStmt->getSQL());
				
				//��λ
				m_varInputParam.clear();
				m_vInputParamDataType.clear();

				m_nInputParamPos = 0;
				m_nOutputColumnPos = 0;
				m_nOutputColumnIndex	 = 0;
			}	
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			int nIndex =0, iDataType = 0;
			int nInputCount = 0;
			for(nIndex = 0; nIndex < m_vProcParamMode.size(); nIndex ++)
			{
				int iMode = m_vProcParamMode[nIndex];
				iDataType = m_vProcParamDataType[nIndex];
				CFieldValue fieldValue("");
				Number numValue;
				string strValue;
				switch(iDataType)
				{
					case OCCIINT:
					case OCCIFLOAT:
					case OCCIUNSIGNED_INT:
					case OCCI_SQLT_NUM:
						if(iMode == IN || iMode == INOUT)
						{
							fieldValue = m_varInputParam[nInputCount];
							//numValue = (Number)fieldValue;
							numValue = fieldValue.operator Number();
							m_pStmt->setNumber(nIndex+1, numValue);
							nInputCount ++;
						}
						if(iMode == OUT || iMode == INOUT)
						{
							m_pStmt->registerOutParam(nIndex+1, OCCINUMBER);
							//�޸�����������ͣ������Ժ��ȡ����ֵ
							m_vProcParamDataType[nIndex] = OCCINUMBER;
						}	
						break;

					case OCCI_SQLT_CHR:
					case OCCI_SQLT_VCS:
					case OCCI_SQLT_AFC:
					case OCCI_SQLT_AVC:
					case OCCI_SQLT_STR:
						strValue = (string)fieldValue;
						if(iMode == IN || iMode == INOUT)
						{
							fieldValue = m_varInputParam[nInputCount];
							strValue = (string)fieldValue;
							m_pStmt->setString(nIndex+1, strValue);
							nInputCount ++;
						}
						if(iMode == OUT || iMode == INOUT)
						{
							m_pStmt->registerOutParam(nIndex+1, OCCISTRING, MAX_STRING_SIZE, "");
							//�޸�����������ͣ������Ժ��ȡ����ֵ
							m_vProcParamDataType[nIndex] = OCCISTRING;
						}
						break;

					default:
						m_iErrorCode = DATA_TYPE_NOT_SUPPORTED;
						m_strErrorMsg = "param datatype ";
						m_strErrorMsg += CRecordSet::PrintType(iDataType);
						m_strErrorMsg += " now NOT supported";
						THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
						break;
				}
			}
			m_pStmt->executeUpdate();
			m_effectCount=m_pStmt->getUpdateCount();//add by yangh
		}
		else if(m_eSQLType == SQL_DDL)
		{
			m_pStmt->executeUpdate();		
		}
		
		else;
		
		m_vInputParamDataType.clear();
		m_varInputParam.clear();
		
		m_iErrorCode = 0;
		m_strErrorMsg = "Success";
	}
	catch(SQLException ex)
	{
		string strMethodName = "int CBindSQL::Execute()";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg();
		
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			string strSQL = m_pStmt->getSQL();
			cerr<<strMethodName<<" Failed! "<<"SQL String: "<<strSQL<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg << endl;
		}
                //2004-12-20
                //add by wulei
                Close();
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return m_iErrorCode;
}

int CBindSQL::Close()
{
	if(m_pRecordSet == NULL && m_pStmt == NULL)
	{
		// Not Opened yet!
		return ZERO;
	}

	if(m_pRecordSet != NULL)	//SELECT QUERY
	{
		delete m_pRecordSet;
		m_pRecordSet=NULL;
	    	// CRecordSet�����������ͷ������Դ
	}
	else		// DELETE ��UPDARE��INSERT DML Statements
	{
		if(m_eSQLType != SELECT_QUERY)
		    	m_pDBConn->m_pConn->terminateStatement (m_pStmt);	
	}
	
	m_pStmt = NULL;
	m_pRecordSet = NULL;

	m_nInputParamPos = 0;
	m_nOutputColumnPos = 0;
	m_nOutputColumnIndex	 = 0;
	m_nInputParamCount = 0;
	
	m_eSQLType = INVALID_TYPE;

	m_varInputParam.clear();
	
	return EXECUTE_SUCCEED;
}
	
//����д
int CBindSQL::CheckValid_InputParam(const char* pszMethodName)
{
	// ǰ������
	if(m_pStmt == NULL)
	{
		string strErrorMsgAdd = GetInputAdditionErrorMsg();
		
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = pszMethodName;
		m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
		m_strErrorMsg += strErrorMsgAdd;

		if(m_bOutputErrorMsg)
		{	
			cerr<< pszMethodName <<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	if(m_eSQLType == SQL_DDL)
	{
		string strErrorMsgAdd = GetInputAdditionErrorMsg();

		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = pszMethodName;
		m_strErrorMsg += " Call Invalid!DDL SQL can not use bind variables!";
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{	
			cerr<< pszMethodName <<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}
	
	return 0;
}

int CBindSQL::Before_InputParam(const char* pszMethodName, int nParamIndex)
{
	// SELECT QUERY������������ͬһ�󶨱�������ֵ		
	if(m_eSQLType == SELECT_QUERY && m_nInputParamPos > GetInputParamCount())
	{
		string strErrorMsgAdd = GetInputAdditionErrorMsg(nParamIndex);

		m_iErrorCode = INVALID_INPUT_BIND_VAR_COUNT;
		m_strErrorMsg = pszMethodName;
		m_strErrorMsg += " Call Invalid!Too Many Input Bind Variables!";
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{	
			cerr<<"CBindSQL::Before_InputParam Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_INPUT_BIND_VAR_COUNT;
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE && m_nInputParamPos > GetInputParamCount())
	{
		string strErrorMsgAdd = GetInputAdditionErrorMsg(nParamIndex);
		
		m_iErrorCode = INVALID_INPUT_BIND_VAR_COUNT;
		m_strErrorMsg = pszMethodName;
		m_strErrorMsg += " Call Invalid!Too Many Input Bind Variables!";
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{	
			cerr<<"CBindSQL::Before_InputParam Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_INPUT_BIND_VAR_COUNT;
	}
	
	else
		return 0;
}

int CBindSQL::Account_InputParamIndex(int nPos)
{
	int nParamCount = 0, nParamIndex = 0;
	nParamCount = GetInputParamCount();

	nParamIndex = nPos % nParamCount;
	if(nParamIndex == 0) 	nParamIndex = nParamCount;		// The first ColIndex is 1 

	return nParamIndex;
}

// For input parameters
CBindSQL& CBindSQL::InLong(const long& lParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator <<(const long& lParam)";
	if(CheckValid_InputParam(szMethodName) != 0)
		return *this;		//Do Nothing

	m_nInputParamPos ++;
	int nParamIndex = Account_InputParamIndex(m_nInputParamPos);
	if(nParamIndex <= 0)	return *this;

	if(Before_InputParam(szMethodName, nParamIndex) != 0)
		return *this;		//Do Nothing

	//��������󶨱���ֵ
	CFieldValue fieldValue(lParam);
	m_varInputParam.push_back(fieldValue);

	if(m_eSQLType == SELECT_QUERY)
	{
		Number numValue(lParam);
		try
		{
			m_pStmt->setNumber(nParamIndex, numValue);
		}
		catch(SQLException ex)
		{
			string strErrorMsgAdd = GetInputAdditionErrorMsg(fieldValue, nParamIndex);
			
			m_iErrorCode = INVALID_PRE_CONDITION;
			m_strErrorMsg = szMethodName;
			m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
			m_strErrorMsg += strErrorMsgAdd;
			
			if(m_bOutputErrorMsg)
			{	
				cerr<< szMethodName <<" Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}
			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return *this;	
		}
	}
	
	else if(m_eSQLType == NONSELECT_DML)
	{
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
	}
	
	else return *this;
	
	if(m_vInputParamDataType.size() < GetInputParamCount())
		m_vInputParamDataType.resize(GetInputParamCount());
	m_vInputParamDataType[nParamIndex-1] = OCCIINT;
	return *this;
	
}

CBindSQL& CBindSQL::InInt(const int& iParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator <<(const int& iParam)";
	if(CheckValid_InputParam(szMethodName) != 0)
		return *this;		//Do Nothing

	m_nInputParamPos ++;
	int nParamIndex = Account_InputParamIndex(m_nInputParamPos);
	if(nParamIndex <= 0)	return *this;
	
	if(Before_InputParam(szMethodName, nParamIndex) != 0)
		return *this;		//Do Nothing

	//��������󶨱���ֵ
	CFieldValue fieldValue(iParam);
	m_varInputParam.push_back(fieldValue);

	if(m_eSQLType == SELECT_QUERY)
	{		
		Number numValue(iParam);
		try
		{
			m_pStmt->setNumber(nParamIndex, numValue);
		}
		catch(SQLException ex)
		{
			string strErrorMsgAdd = GetInputAdditionErrorMsg(fieldValue, nParamIndex);
			
			m_iErrorCode = INVALID_PRE_CONDITION;
			m_strErrorMsg = szMethodName;
			m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
			m_strErrorMsg += strErrorMsgAdd;

			if(m_bOutputErrorMsg)
			{	
				cerr<< szMethodName <<" Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}
			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return *this;	
		}
	}

	else if(m_eSQLType == NONSELECT_DML)
	{
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
	}

	else return *this;

	if(m_vInputParamDataType.size() < GetInputParamCount())
		m_vInputParamDataType.resize(GetInputParamCount());
	m_vInputParamDataType[nParamIndex-1] = OCCIINT;	
	return *this;
}

CBindSQL& CBindSQL::InFloat(const float& fParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator <<(const float& fParam)";
	if(CheckValid_InputParam(szMethodName) != 0)
		return *this;		//Do Nothing

	m_nInputParamPos ++;
	int nParamIndex = Account_InputParamIndex(m_nInputParamPos);
	if(nParamIndex <= 0)	return *this;
	
	if(Before_InputParam(szMethodName, nParamIndex) != 0)
		return *this;		//Do Nothing

	//��������󶨱���ֵ
	CFieldValue fieldValue(fParam);
	m_varInputParam.push_back(fieldValue);

	if(m_eSQLType == SELECT_QUERY)
	{		
		Number numValue(fParam);
		try
		{
			m_pStmt->setNumber(nParamIndex, numValue);
		}
		catch(SQLException ex)
		{
			string strErrorMsgAdd = GetInputAdditionErrorMsg(fieldValue, nParamIndex);			
		
			m_iErrorCode = INVALID_PRE_CONDITION;
			m_strErrorMsg = szMethodName;
			m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
			m_strErrorMsg += strErrorMsgAdd;

			if(m_bOutputErrorMsg)
			{	
				cerr<< szMethodName <<" Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}
			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return *this;	
		}
	}

	else if(m_eSQLType == NONSELECT_DML)
	{
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
	}

	else return *this;

	if(m_vInputParamDataType.size() < GetInputParamCount())
		m_vInputParamDataType.resize(GetInputParamCount());
	m_vInputParamDataType[nParamIndex-1] = OCCIFLOAT;	
	return *this;
}

CBindSQL& CBindSQL::InDouble(const double& dParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator <<(const double& dParam)";
	if(CheckValid_InputParam(szMethodName) != 0)
		return *this;		//Do Nothing

	m_nInputParamPos ++;
	int nParamIndex = Account_InputParamIndex(m_nInputParamPos);
	if(nParamIndex <= 0)	return *this;
	
	if(Before_InputParam(szMethodName, nParamIndex) != 0)
		return *this;		//Do Nothing

	//��������󶨱���ֵ
	CFieldValue fieldValue(dParam);
	m_varInputParam.push_back(fieldValue);

	if(m_eSQLType == SELECT_QUERY)
	{		
		Number numValue(dParam);
		try
		{
			m_pStmt->setNumber(nParamIndex, numValue);
		}
		catch(SQLException ex)
		{
			string strErrorMsgAdd = GetInputAdditionErrorMsg(fieldValue, nParamIndex);			
		
			m_iErrorCode = INVALID_PRE_CONDITION;
			m_strErrorMsg = szMethodName;
			m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
			m_strErrorMsg += strErrorMsgAdd;

			if(m_bOutputErrorMsg)
			{	
				cerr<< szMethodName <<" Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}
			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return *this;	
		}
	}

	else if(m_eSQLType == NONSELECT_DML)
	{
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
	}

	else return *this;

	if(m_vInputParamDataType.size() < GetInputParamCount())
		m_vInputParamDataType.resize(GetInputParamCount());
	m_vInputParamDataType[nParamIndex-1] = OCCIFLOAT;	
	return *this;
}

CBindSQL& CBindSQL::InString(const string& strParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator <<(const string& strParam)";
	if(CheckValid_InputParam(szMethodName) != 0)
		return *this;		//Do Nothing

	m_nInputParamPos ++;
	int nParamIndex = Account_InputParamIndex(m_nInputParamPos);
	if(nParamIndex <= 0)	return *this;
	
	if(Before_InputParam(szMethodName, nParamIndex) != 0)
		return *this;		//Do Nothing

	//��������󶨱���ֵ
	CFieldValue fieldValue(strParam);
	m_varInputParam.push_back(fieldValue);

	if(m_eSQLType == SELECT_QUERY)
	{
		try
		{
			m_pStmt->setString(nParamIndex, strParam);
		}
		catch(SQLException ex)
		{
			string strErrorMsgAdd = GetInputAdditionErrorMsg(fieldValue, nParamIndex);
			
			m_iErrorCode = INVALID_PRE_CONDITION;
			m_strErrorMsg = szMethodName;
			m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
			m_strErrorMsg += strErrorMsgAdd;

			if(m_bOutputErrorMsg)
			{	
				cerr<< szMethodName <<" Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}
			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return *this;	
		}
	}

	else if(m_eSQLType ==  NONSELECT_DML)
	{
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
	}

	if(m_vInputParamDataType.size() < GetInputParamCount())
		m_vInputParamDataType.resize(GetInputParamCount());
	
	if(strParam.size() > 0)
		m_vInputParamDataType[nParamIndex-1] = OCCI_SQLT_STR;
	else //���Ϊ�գ�����Ҫ����һ��NULL�ַ���
		m_vInputParamDataType[nParamIndex-1] = OCCI_SQLT_STR;
	return *this;
}

CBindSQL& CBindSQL::InStrPointer(const char* pszParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator <<(const char* pszParam)";
	if(CheckValid_InputParam(szMethodName) != 0)
		return *this;		//Do Nothing

	m_nInputParamPos ++;
	int nParamIndex = Account_InputParamIndex(m_nInputParamPos);
	if(nParamIndex <= 0)	return *this;
	
	if(Before_InputParam(szMethodName, nParamIndex) != 0)
		return *this;		//Do Nothing

	string strParam;
	strParam.append(pszParam);

	//��������󶨱���ֵ
	CFieldValue fieldValue(strParam);
	m_varInputParam.push_back(fieldValue);
	
	if(m_eSQLType == SELECT_QUERY)
	{
		try
		{
			m_pStmt->setString(nParamIndex, strParam);
		}
		catch(SQLException ex)
		{
			string strErrorMsgAdd = GetInputAdditionErrorMsg(fieldValue, nParamIndex);

			m_iErrorCode = INVALID_PRE_CONDITION;
			m_strErrorMsg = szMethodName;
			m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
			m_strErrorMsg += strErrorMsgAdd;

			if(m_bOutputErrorMsg)
			{	
				cerr<< szMethodName <<" Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}
			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return *this;	
		}
	}

	else if(m_eSQLType == NONSELECT_DML)
	{
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
	}

	if(m_vInputParamDataType.size() < GetInputParamCount())
		m_vInputParamDataType.resize(GetInputParamCount());
	if(pszParam != NULL)
		m_vInputParamDataType[nParamIndex-1] = OCCI_SQLT_STR;
	else //���Ϊ�գ�����Ҫ����һ��NULL�ַ���
		m_vInputParamDataType[nParamIndex-1] = OCCI_SQLT_STR;
	return *this;
}

CBindSQL& CBindSQL::InChar(const char& cParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator <<(const char& cParam)";
	if(CheckValid_InputParam(szMethodName) != 0)
		return *this;		//Do Nothing

	m_nInputParamPos ++;
	int nParamIndex = Account_InputParamIndex(m_nInputParamPos);
	if(nParamIndex <= 0)	return *this;
	
	if(Before_InputParam(szMethodName, nParamIndex) != 0)
		return *this;		//Do Nothing

	string strParam;
	strParam.append(1, cParam);

	//��������󶨱���ֵ
	CFieldValue fieldValue(strParam);
	m_varInputParam.push_back(fieldValue);

	if(m_eSQLType == SELECT_QUERY)
	{
		try
		{
			m_pStmt->setString(nParamIndex, strParam);
		}
		catch(SQLException ex)
		{
			string strErrorMsgAdd = GetInputAdditionErrorMsg(fieldValue, nParamIndex);
			
			m_iErrorCode = INVALID_PRE_CONDITION;
			m_strErrorMsg = szMethodName;
			m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
			m_strErrorMsg += strErrorMsgAdd;

			if(m_bOutputErrorMsg)
			{	
				cerr<< szMethodName <<" Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}
			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return *this;	
		}
	}

	else if(m_eSQLType == NONSELECT_DML)
	{
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
	}

	if(m_vInputParamDataType.size() < GetInputParamCount())
		m_vInputParamDataType.resize(GetInputParamCount());
	m_vInputParamDataType[nParamIndex-1] = OCCI_SQLT_STR;	
	return *this;
}

CBindSQL& CBindSQL::operator <<(const long& lParam)
{
	return InLong(lParam);
}

CBindSQL& CBindSQL::operator <<(const int& iParam)
{
	return InInt(iParam);
}

CBindSQL& CBindSQL::operator <<(const double& dParam)
{
	return InDouble(dParam);
}

CBindSQL& CBindSQL::operator <<(const float& fParam)
{
	return InFloat(fParam);
}

CBindSQL& CBindSQL::operator <<(const string& strParam)
{
	return InString(strParam);
}

CBindSQL& CBindSQL::operator <<(const char* pszParam)
{
	return InStrPointer(pszParam);
}

CBindSQL& CBindSQL::operator <<(const char& cParam)
{
	return InChar(cParam);
}

int CBindSQL::CheckValid_OutputParam(const char* pszMethodName)
{
	// ǰ������
	if(m_pStmt == NULL)
	{
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = pszMethodName;
		m_strErrorMsg += pszMethodName;
		m_strErrorMsg += " Call Invalid!BindSQL statement not prepared yet";
		m_strErrorMsg += GetOutputAdditionErrorMsg();
		
		if(m_bOutputErrorMsg)
		{	
			cerr<< pszMethodName << " Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	//ǰ������
	if(m_eSQLType != SELECT_QUERY && m_eSQLType != FUNCTION_OR_PROCEDURE)
	//	Ŀǰ�������ֻ������������֧�ַ��ز���
	{
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = pszMethodName;
		m_strErrorMsg += " Call Invalid!only Select Query allowed to call this function";
		m_strErrorMsg += GetOutputAdditionErrorMsg();
		
		if(m_bOutputErrorMsg)
		{	
			cerr<< pszMethodName << " Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	return 0;
}

int CBindSQL::Before_OutputParam()
{
	//���û��ִ�в�ѯ����ִ��֮
	if(m_eSQLType == SELECT_QUERY )
	{
		if(m_pRecordSet != NULL)
		{
			//����Ѿ�����¼β�����Ѿ���������ִ���������
			if(true == IsEnd() || true == IsError())
				return -1;
			else
				return 0;
		}
		else
			return Execute();	
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	{
		int nOutParam = 0;
		for(int nIndex = 0; nIndex < m_vProcParamMode.size(); nIndex ++)
		{	
			int iMode = m_vProcParamMode[nIndex];
			if(iMode == OUT || iMode == INOUT)
				nOutParam ++;
		}
		//���������Ŀ������OUT �󶨱���������
		if(m_nOutputColumnPos > nOutParam)
			return -1;
		else
			return 0;
	}

	//����������������
	else return -1;
}

// For result-set output
CBindSQL& CBindSQL::OutLong(long& lParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator >>(long& lParam)";
	if(CheckValid_OutputParam(szMethodName) != 0)
		return *this;	// Do Nothing

	if(Before_OutputParam() != 0)
		return *this;
	
	m_nOutputColumnPos ++;
	int nColIndex = 0;
		
	try
	{
		if(m_eSQLType == SELECT_QUERY)
		{
			m_nOutputColumnIndex ++;
			if(m_nOutputColumnIndex > m_pRecordSet->GetColumnCount())
				m_nOutputColumnIndex = 1;

			nColIndex = m_nOutputColumnIndex;
			if(nColIndex == 1)		
			//�������ʱ����ȡ���м�¼
			{	
				//�Ѿ�����β���߳���
				if(ZERO == m_pRecordSet->ReadRecordAndNext())
					return *this;
			}
			lParam = m_pRecordSet->FetchLong(nColIndex);
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			CFieldValue fieldValue("");
			int iDataType = m_vProcParamDataType[nColIndex-1];
			if(iDataType == OCCINUMBER)
				fieldValue = m_pStmt->getNumber(nColIndex);
			else if(iDataType == OCCISTRING)
				fieldValue = m_pStmt->getString(nColIndex);
			else;
			lParam = (long)fieldValue;
		}
		else ;
	}
	catch(CDBException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(long& lParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
		
		m_iErrorCode = ex.GetErrorCode();
		m_strErrorMsg = ex.GetErrorMsg();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(SQLException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(long& lParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
	
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	
	return *this;
}

CBindSQL& CBindSQL::OutInt(int& iParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator >>(int& iParam)";
	if(CheckValid_OutputParam(szMethodName) != 0)
		return *this;	// Do Nothing
		
	if(Before_OutputParam() != 0)
		return *this;
	
	m_nOutputColumnPos ++;
	int nColIndex = 0;
	
	try
	{
		if(m_eSQLType == SELECT_QUERY)
		{	
			m_nOutputColumnIndex ++;
			if(m_nOutputColumnIndex > m_pRecordSet->GetColumnCount())
				m_nOutputColumnIndex = 1;

			nColIndex = m_nOutputColumnIndex;
			if(nColIndex == 1)		
			//�������ʱ����ȡ���м�¼
			{	
				//�Ѿ�����β���߳���
				if(ZERO == m_pRecordSet->ReadRecordAndNext())
					return *this;
			}
			iParam = m_pRecordSet->FetchInt(nColIndex);
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			CFieldValue fieldValue("");
			int iDataType = m_vProcParamDataType[nColIndex-1];
			if(iDataType == OCCINUMBER)
				fieldValue = m_pStmt->getNumber(nColIndex);
			else if(iDataType == OCCISTRING)
				fieldValue = m_pStmt->getString(nColIndex);
			else;
			iParam = (int)fieldValue;
		}
		else;
	}
	catch(CDBException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(int& iParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
		
		m_iErrorCode = ex.GetErrorCode();
		m_strErrorMsg = ex.GetErrorMsg();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(SQLException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(int& iParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
		
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return *this;
}

CBindSQL& CBindSQL::OutFloat(float& fParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator >>(float& fParam)";
	if(CheckValid_OutputParam(szMethodName) != 0)
		return *this;	// Do Nothing
		
	if(Before_OutputParam() != 0)
		return *this;
	
	m_nOutputColumnPos ++;
	int nColIndex = 0;
	
	try
	{
		if(m_eSQLType == SELECT_QUERY)
		{
			m_nOutputColumnIndex ++;
			if(m_nOutputColumnIndex > m_pRecordSet->GetColumnCount())
				m_nOutputColumnIndex = 1;

			nColIndex = m_nOutputColumnIndex;
			if(nColIndex == 1)		
			//�������ʱ����ȡ���м�¼
			{	
				//�Ѿ�����β���߳���
				if(ZERO == m_pRecordSet->ReadRecordAndNext())
					return *this;
			}
			fParam = m_pRecordSet->FetchFloat(nColIndex);
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			CFieldValue fieldValue("");
			int iDataType = m_vProcParamDataType[nColIndex-1];
			if(iDataType == OCCINUMBER)
				fieldValue = m_pStmt->getNumber(nColIndex);
			else if(iDataType == OCCISTRING)
				fieldValue = m_pStmt->getString(nColIndex);
			else;
			fParam = (double)fieldValue;
		}
		else;

	}
	catch(CDBException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(float& fParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);	
		
		m_iErrorCode = ex.GetErrorCode();
		m_strErrorMsg = ex.GetErrorMsg();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(SQLException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(float& fParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);	
		
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return *this;
}

CBindSQL& CBindSQL::OutDouble(double& dParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator >>(double& dParam)";
	if(CheckValid_OutputParam(szMethodName) != 0)
		return *this;	// Do Nothing
		
	if(Before_OutputParam() != 0)
		return *this;
	
	m_nOutputColumnPos ++;
	int nColIndex = 0;
	
	try
	{
		if(m_eSQLType == SELECT_QUERY)
		{
			m_nOutputColumnIndex ++;
			if(m_nOutputColumnIndex > m_pRecordSet->GetColumnCount())
				m_nOutputColumnIndex = 1;

			nColIndex = m_nOutputColumnIndex;
			if(nColIndex == 1)		
			//�������ʱ����ȡ���м�¼
			{	
				//�Ѿ�����β���߳���
				if(ZERO == m_pRecordSet->ReadRecordAndNext())
					return *this;
			}
			dParam = m_pRecordSet->FetchDouble(nColIndex);
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			CFieldValue fieldValue("");
			int iDataType = m_vProcParamDataType[nColIndex-1];
			if(iDataType == OCCINUMBER)
				fieldValue = m_pStmt->getNumber(nColIndex);
			else if(iDataType == OCCISTRING)
				fieldValue = m_pStmt->getString(nColIndex);
			else;
			dParam = (double)fieldValue;
		}
		else;

	}
	catch(CDBException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(double& dParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);	
		
		m_iErrorCode = ex.GetErrorCode();
		m_strErrorMsg = ex.GetErrorMsg();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(SQLException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(double& dParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);	
		
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return *this;
}

CBindSQL& CBindSQL::OutString(string& strParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator >>(string& strParam)";
	if(CheckValid_OutputParam(szMethodName) != 0)
		return *this;	// Do Nothing
		
	if(Before_OutputParam() != 0)
		return *this;
	
	m_nOutputColumnPos ++;
	int nColIndex = 0;
	
	try
	{
		if(m_eSQLType == SELECT_QUERY)
		{
			m_nOutputColumnIndex ++;
			if(m_nOutputColumnIndex > m_pRecordSet->GetColumnCount())
				m_nOutputColumnIndex = 1;

			nColIndex = m_nOutputColumnIndex;
			if(nColIndex == 1)		
			//�������ʱ����ȡ���м�¼
			{	
				//�Ѿ�����β���߳���
				if(ZERO== m_pRecordSet->ReadRecordAndNext())
					return *this;
			}
			strParam = m_pRecordSet->FetchString(nColIndex);
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			CFieldValue fieldValue("");
			int iDataType = m_vProcParamDataType[nColIndex-1];
			if(iDataType == OCCINUMBER)
				fieldValue = m_pStmt->getNumber(nColIndex);
			else if(iDataType == OCCISTRING)
				fieldValue = m_pStmt->getString(nColIndex);
			else;
			strParam = (string)fieldValue;
		}
		else;
	}
	catch(CDBException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(string& strParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
	
		m_iErrorCode = ex.GetErrorCode();
		m_strErrorMsg = ex.GetErrorMsg();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(SQLException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(string& strParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
	
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return *this;
}

CBindSQL& CBindSQL::OutStrPointer(char* pszParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator >>(char* pszParam)";
	if(CheckValid_OutputParam(szMethodName) != 0)
		return *this;	// Do Nothing
		
	if(Before_OutputParam() != 0)
		return *this;
	
	m_nOutputColumnPos ++;
	int nColIndex = 0;
	
	try
	{
		if(m_eSQLType == SELECT_QUERY)
		{
			m_nOutputColumnIndex ++;
			if(m_nOutputColumnIndex > m_pRecordSet->GetColumnCount())
				m_nOutputColumnIndex = 1;

			nColIndex = m_nOutputColumnIndex;
			if(nColIndex == 1)		
			//�������ʱ����ȡ���м�¼
			{	
				//�Ѿ�����β���߳���
				if(ZERO == m_pRecordSet->ReadRecordAndNext())
					return *this;
			}

			//strcpy(pszParam,(m_pRecordSet->m_pResultSet->getString(nColIndex)).c_str());
			strcpy(pszParam,(m_pRecordSet->FetchString(nColIndex)).c_str());
			
			/*
			string strParam = m_pRecordSet->FetchString(nColIndex);
			//strParam.copy(pszParam, strParam.size());	//�ú�������copy���ַ����������Կ��Ƕ�ȡ����ֵ
			strcpy(pszParam, strParam.c_str());
			*/
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			nColIndex = Account_OutputParamIndex(m_nOutputColumnPos);
			if(nColIndex <= 0)		return *this;
		
			CFieldValue fieldValue("");
			int iDataType = m_vProcParamDataType[nColIndex-1];
			if(iDataType == OCCINUMBER)
				fieldValue = m_pStmt->getNumber(nColIndex);
			else if(iDataType == OCCISTRING)
				fieldValue = m_pStmt->getString(nColIndex);
			else;
			string strParam = (string)fieldValue;
			//strParam.copy(pszParam, strParam.size());
			strcpy(pszParam, strParam.c_str());			
		}
		else;
	
	}
	catch(CDBException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(char* pszParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
		
		m_iErrorCode = ex.GetErrorCode();
		m_strErrorMsg = ex.GetErrorMsg();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(SQLException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(char* pszParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);
		
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return *this;
}

CBindSQL& CBindSQL::OutChar(char& cParam)
{
	const char szMethodName[] = "CBindSQL& CBindSQL::operator >>(char& cParam)";
	if(CheckValid_OutputParam(szMethodName) != 0)
		return *this;	// Do Nothing
		
	if(Before_OutputParam() != 0)
		return *this;
	
	m_nOutputColumnPos ++;
	int nColIndex = 0;
	
	try
	{
		if(m_eSQLType == SELECT_QUERY)
		{
			m_nOutputColumnIndex ++;
			if(m_nOutputColumnIndex > m_pRecordSet->GetColumnCount())
				m_nOutputColumnIndex = 1;

			nColIndex = m_nOutputColumnIndex;
			if(nColIndex == 1)		
			//�������ʱ����ȡ���м�¼
			{	
				//�Ѿ�����β���߳���
				if(ZERO == m_pRecordSet->ReadRecordAndNext())
					return *this;
			}
			string strParam = m_pRecordSet->FetchString(nColIndex);
			// �������ַ�
			if(strParam.size() == 0)
				cParam = ZERO;
			else
				cParam = strParam.at(0);
		}
		else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
		{
			CFieldValue fieldValue("");
			int iDataType = m_vProcParamDataType[nColIndex-1];
			if(iDataType == OCCINUMBER)
				fieldValue = m_pStmt->getNumber(nColIndex);
			else if(iDataType == OCCISTRING)
				fieldValue = m_pStmt->getString(nColIndex);
			else;
			string strParam = (string)fieldValue;
			if(strParam.size() == 0)
				cParam = ZERO;
			else
				cParam = strParam.at(0);
		}
		else;
	
	}
	catch(CDBException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(char& cParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);	
		
		m_iErrorCode = ex.GetErrorCode();
		m_strErrorMsg = ex.GetErrorMsg();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(SQLException ex)
	{
		string strMethodName = "CBindSQL& CBindSQL::operator >>(char& cParam)";
		string strErrorMsgAdd = GetOutputAdditionErrorMsg(nColIndex);	
		
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return *this;
}

CBindSQL& CBindSQL::operator >>(long& lParam)
{
	return OutLong(lParam);
}

CBindSQL& CBindSQL::operator >>(int& iParam)
{
	return OutInt(iParam);
}

CBindSQL& CBindSQL::operator >>(double& dParam)
{
	return OutDouble(dParam);
}

CBindSQL& CBindSQL::operator >>(float& fParam)
{
	return OutFloat(fParam);
}

CBindSQL& CBindSQL::operator >>(string& strParam)
{
	return OutString(strParam);
}

CBindSQL& CBindSQL::operator >>(char* pszParam)
{
	return OutStrPointer(pszParam);
}

CBindSQL& CBindSQL::operator >>(char& cParam)
{
	return OutChar(cParam);
}

int CBindSQL::Account_OutputParamIndex(int nPos)
{
	if(m_eSQLType == SELECT_QUERY)
	{
		int iColCount = 0, nColIndex = 0;
		iColCount = m_pRecordSet->GetColumnCount();

		nColIndex = nPos % iColCount;
		if(nColIndex == 0) 	nColIndex = iColCount;		// The first ColIndex is 1 

		if(nColIndex == 1)		
		//�������ʱ����ȡ���м�¼
		{	
			//�Ѿ�����β���߳���
			if(ZERO == m_pRecordSet->ReadRecordAndNext())
				nColIndex = -1;
		}
		
		return nColIndex;
	}

	else if(m_eSQLType == FUNCTION_OR_PROCEDURE)
	//�����λ��˳���(nPos)ת��Ϊ�����в����е�λ�ú�(ParamIndex)
	{
		int nOutParam = 0;
		for(int nIndex = 0; nIndex < m_vProcParamMode.size(); nIndex ++)
		{	
			int iMode = m_vProcParamMode[nIndex];
			if(iMode == OUT || iMode == INOUT)
				nOutParam ++;
			if(nOutParam == nPos)	return nIndex+1;
		}
		if(nPos > nOutParam)
		{
			string strErrorMsgAdd = GetOutputAdditionErrorMsg();
	
			m_iErrorCode = INVALID_OUTPUT_BIND_VAR_COUNT;
			m_strErrorMsg = "int CBindSQL::Account_OutputParamIndex(int nPos)";
			m_strErrorMsg += " Call Invalid!Too Many Output Bind Variables!";
			m_strErrorMsg += strErrorMsgAdd;
			
			if(m_bOutputErrorMsg)
			{	
				cerr<<"CBindSQL::Account_OutputParamIndex Call Invalid!"<<endl;
				cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
			}

			THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
			return INVALID_OUTPUT_BIND_VAR_COUNT;
		}
	}

	else 
		return ZERO;
}

int CBindSQL::After_OutputParam(const string& strMethodName, int nParamIndex, const string& strErrorMsgAdd)
{
	int iReturn = 0;
	try
	{
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		m_strErrorMsg += strErrorMsgAdd;
		
		if(m_bOutputErrorMsg)
		{
			cerr<< strMethodName << " Failed! "<<endl;
			cerr<<"Code: "<< m_iErrorCode <<" Msg: "<< m_strErrorMsg << endl;
		}

		iReturn  = m_iErrorCode;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

	return iReturn;
}
	
int CBindSQL::GetLastDBErrorCode()
{
	return m_iErrorCode;
}

string CBindSQL::GetLastDBErrorMsg()
{
	return m_strErrorMsg;
}

bool CBindSQL::IsError()
{
	if(m_iErrorCode == 0)
		return false;
	else
		return true;
}

bool CBindSQL::IsEnd()
{
	if(m_pRecordSet == NULL)
		return false;

	if(m_pRecordSet->IsEnd() == 1)
		return true;
	else
		return false;
}

//ֻ�и���Ϊ��ʱ,�ŷ�����,���򷵻ؼ� 
bool CBindSQL::IsNull(int nColIndex)
{
	if(m_pRecordSet == NULL)
		return false;

	if(m_pRecordSet->IsNull(nColIndex) == 1)
		return true;
	else
		return false;
}

//ֻ�и���Ϊ��ʱ,�ŷ�����,���򷵻ؼ� 
bool CBindSQL::IsNull(const string& strColName)
{
	if(m_pRecordSet == NULL)
		return false;

	if(m_pRecordSet->IsNull(strColName) == 1)
		return true;
	else
		return false;
}

CBindSQL::operator bool()
{
	return !IsError() && !IsEnd();
}

// Class CDatabase 
float CDatabase::m_fVersion = 1.09;
bool CDatabase::m_bTimerStarted = false;
int CDatabase::m_nElapse = 5 * 60 * 1000;			// 5����,��ע�⣬�Ǻ���
int CDatabase::m_nSignalID = SIGRTMAX;	//��������źţ���solaris�£�SIGRTMAX���궨��Ϊ_sysconf(...)���ﲻ������ֵ��Ч�����ڹ��캯������Ӹ�ֵ���
timer_t CDatabase::m_nTimerID;

int CDatabase::m_nCurrentSeq = 0;
vector<CDBObj> CDatabase::m_vObj;

char * CDatabase:: getVersion()
{
	return("3.0.0");
}

CDatabase::CDatabase()
{
	// added by wh 2006-04-20
	m_nSignalID = SIGRTMAX;
	
	//ȱʡָ����Ҫ���������Ϣ
	m_bOutputErrorMsg = true;

	m_iErrorCode = 0;
	m_strErrorMsg = "Success";

	//OCCI�����ʼ��Ϊ��ָ��
	m_pConn = NULL;
	m_pEnv = NULL;

	// ��ʼ�����ӱ�־Ϊ0
	m_bConnected = false;

	m_strDatabase = "";
	m_strUserName = "";
	m_strPassword = "";
	
	// Ĭ����Ҫ���ֳ�����
	m_bKeepLongConnection = true;

	//�ǼǶ���
	m_nCurrentSeq ++;
	m_nSeq = m_nCurrentSeq;

	m_vObj.push_back(CDBObj(m_nSeq, this));

	if(m_bKeepLongConnection == true && CDatabase::m_bTimerStarted == false)
		StartTimer();
	
}

CDatabase::CDatabase(CDatabase& db)
{
	//cout<<"CDatabase Copy Constructor Called!"<<endl;
	// added by wh 2006-04-20
	m_nSignalID = SIGRTMAX;
	
	//ȱʡָ����Ҫ���������Ϣ
	m_bOutputErrorMsg = db.m_bOutputErrorMsg;

	m_iErrorCode = db.m_iErrorCode;
	m_strErrorMsg = db.m_strErrorMsg;

	m_bConnected = db.m_bConnected;
	m_bKeepLongConnection = db.m_bKeepLongConnection;	
	//OCCI�����ʼ��Ϊ��ָ��
	if(m_bConnected == false)
	{
		m_strDatabase = "";
		m_strUserName = "";
		m_strPassword = "";

		m_pConn = NULL;
		m_pEnv = NULL;
	}
	else
	{
		//���캯���Ƿ������׳��쳣
		//Connect(m_strDatabase, m_strUserName, m_strPassword);
		m_strDatabase = db.m_strDatabase;
		m_strUserName = db.m_strUserName;
		m_strPassword = db.m_strPassword;

		m_pEnv = Environment::createEnvironment (Environment::OBJECT);
		m_pConn = m_pEnv->createConnection (m_strUserName, m_strPassword, m_strDatabase);
	}

	//�ǼǶ���
	m_nCurrentSeq ++;
	m_nSeq = m_nCurrentSeq;

	m_vObj.push_back(CDBObj(m_nSeq, this));
	if(m_bKeepLongConnection == true && CDatabase::m_bTimerStarted == false)
		StartTimer();	

}

CDatabase::~CDatabase()
{
	//add by yangh 2005-12-31
	try
	{
		if(m_bConnected)
		{
			Statement *pStmt = NULL;	  
			ResultSet* pRset = NULL;
	  	string sqlStmt = "SELECT TO_CHAR(SYSDATE,'YYYYMMDDHH24MISS') FROM DUAL";
	  	if(m_pConn != NULL)
	  	{	pStmt = m_pConn->createStatement(sqlStmt);
	  		pRset = pStmt->executeQuery();
	  		pRset->next();
			}
		}
	}
	catch(SQLException ex)
	{
		m_bConnected=false;
	}
	if(m_bConnected)
	{
		//��������Disconnect�����е�m_pEnv->terminateConnection (m_pConn)�ᵼ���ύ
		//������Environment::createEnvironmentѡ�õ�ģʽ�޹�
		//�����Ȼع�
		//��Ϊ�Ѿ����˽⹹�����������޸����ݿ������Ϣm_iErrorCodeû�ж��Ӱ��
		Rollback();
		Disconnect();
	}	

	//ע������
	vector<CDBObj>::iterator it;// = NULL;
	for(it = m_vObj.begin(); it!= m_vObj.end(); it ++)
	{
		CDBObj obj = *it;
		if(obj.nID == m_nSeq)
		{
			m_vObj.erase(it);
			break;
		}
	}

	if(m_vObj.size() == 0)
		KillTimer();
}
CDatabase& CDatabase::operator =(const CDatabase &right)
{
	if (this == &right) 
		return *this;

	m_nSignalID = SIGRTMAX;
	
	//ȱʡָ����Ҫ���������Ϣ
	m_bOutputErrorMsg = right.m_bOutputErrorMsg;

	m_iErrorCode = right.m_iErrorCode;
	m_strErrorMsg = right.m_strErrorMsg;

	m_bConnected = right.m_bConnected;
	m_bKeepLongConnection = right.m_bKeepLongConnection;	
	//OCCI�����ʼ��Ϊ��ָ��
	if(m_bConnected == false)
	{
		m_strDatabase = "";
		m_strUserName = "";
		m_strPassword = "";

		m_pConn = NULL;
		m_pEnv = NULL;
	}
	else
	{
		//���캯���Ƿ������׳��쳣
		//Connect(m_strDatabase, m_strUserName, m_strPassword);
		m_strDatabase = right.m_strDatabase;
		m_strUserName = right.m_strUserName;
		m_strPassword = right.m_strPassword;

		m_pEnv = Environment::createEnvironment (Environment::OBJECT);
		m_pConn = m_pEnv->createConnection (m_strUserName, m_strPassword, m_strDatabase);
	}

	//�ǼǶ���
	m_nCurrentSeq ++;
	m_nSeq = m_nCurrentSeq;

	m_vObj.push_back(CDBObj(m_nSeq, this));
	if(m_bKeepLongConnection == true && CDatabase::m_bTimerStarted == false)
		StartTimer();	
	
	return *this;
}

int CDatabase::Connect(const string& strUserName, const string& strPassword, const string& strDatabase)
{
	const char szMethodName[] = "int CDatabase::Connect(const string& strUserName, const string& strPassword, const string& strDatabase)";
	//ǰ������
	if(strUserName.empty() || strPassword.empty() || strDatabase.empty())
	{
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg +=  " Call Invalid!Username or Password or Database Name is NULL";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}

	//ǰ������
	if(m_bConnected == true)
	{
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg +=  " Call Invalid!Database Already Connected, Please Disconnect First!";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}
	
	int iReturn = 0;
	try
	{
		//m_pEnv = Environment::createEnvironment (Environment::DEFAULT);
		//Ϊ��ʹ�� Date����ʹ��OBJECTģʽ
		//Ϊ���̰߳�ȫ��ʹ��THREADED_MUTEXEDģʽ
		m_pEnv = Environment::createEnvironment (Environment::OBJECT);
		m_pConn = m_pEnv->createConnection (strUserName, strPassword, strDatabase);

		m_bConnected = true;

		m_iErrorCode = 0;
		m_strErrorMsg = "Success";		
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<szMethodName<<" Failed! "<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = m_iErrorCode;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	catch(...)
	{
		m_strErrorMsg = "asdfdsa";
		cerr<<"catch err!"<<endl;
		THROW_EXCEPTION(2344,m_strErrorMsg);
	}

	if(iReturn == 0)	//Connect Succeeded!
	{
		m_bConnected = true;
		m_strDatabase = strDatabase;
		m_strUserName = strUserName;
		m_strPassword = strPassword;
	}
	
	return iReturn;
}

int CDatabase::Disconnect()
{
	const char szMethodName[] = "int CDatabase::Disconnect()";
	int iReturn  = 0;

	//ǰ������
	if(m_bConnected == false)
	{
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg +=  " Call Invalid!Database NOT Connected, Please Connect First!";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}

	try
	{
	    	m_pEnv->terminateConnection(m_pConn);
    		Environment::terminateEnvironment(m_pEnv);

		m_bConnected = false;

		m_iErrorCode = 0;
		m_strErrorMsg = "Success";
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<szMethodName<<" Failed! "<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = m_iErrorCode;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
		
	return iReturn;
}
int CDatabase::Reconnect()
{
	Disconnect();
	Connect(m_strUserName,m_strPassword,m_strDatabase);
	return 0;
}

int CDatabase::IsConnected()
{
	if(m_bConnected == true)
		return 1;
	else
		return -1;
}

int CDatabase::GetLastDBErrorCode()
{
	return m_iErrorCode;
}

string CDatabase::GetLastDBErrorMsg()
{
	return m_strErrorMsg;
}

int CDatabase::Commit()
{
	const char szMethodName[] = "int CDatabase::Commit()";
	//ǰ������������
	if(m_bConnected == 0 || m_pConn == NULL) 	 
	{
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg +=  " Call Invalid!Database Not Connected yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}
	
	int iReturn  = 0;
	try
	{
	  m_pConn->commit();
		m_iErrorCode = 0;
		m_strErrorMsg = "Success";
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<szMethodName<<" Failed! "<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = m_iErrorCode;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	return iReturn;
}

int CDatabase::Rollback()
{
	const char szMethodName[] = "int CDatabase::Rollback()";
	//ǰ������������
	if(m_bConnected == 0 || m_pConn == NULL) 	 
	{
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg += " Call Invalid!Database Not Connected yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	int iReturn  = 0;
	try
	{
	    	m_pConn->rollback();

		m_iErrorCode = 0;
		m_strErrorMsg = "Success";
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<szMethodName<<" Failed! "<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = m_iErrorCode;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}
	return iReturn;
}

//ִ�з�SELECT DML��������DDL���
//���ش�����룬�ɹ�����0
int CDatabase::ExecuteCmd(const string& strSQL)
{
	const char szMethodName[] = "int CDatabase::ExecuteCmd(const string& strSQL)";
	//������Ч
	if(strSQL.empty())
	{
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg += " Call Invalid!SQL String is NULL";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}

	//ǰ������������
	if(m_bConnected == 0 || m_pConn == NULL) 	 
	{
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg += " Call Invalid!Database Not Connected yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	int iReturn  = 0;

    	string strSqlStmt = strSQL;
	try
	{
		Statement* pStmt = NULL;
	    	pStmt = m_pConn->createStatement(strSqlStmt);
		pStmt->executeUpdate();
	    	m_pConn->terminateStatement(pStmt);

		m_iErrorCode = 0;
		m_strErrorMsg = "Success";
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<szMethodName<<" Failed! SQL String: "<<strSQL<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = m_iErrorCode;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
	}

    	return iReturn;
}

// ִ��SELECT ��ѯ
// pResultSet���������������ֵ��һ��ͨ��new��������Ķ���ָ��
int CDatabase::ExecuteQuery(const string& strSQL, CRecordSet* pRecordSet)
{
	const char szMethodName[] = "int CDatabase::ExecuteQuery(const string& strSQL, CRecordSet* pRecordSet)";
	//������Ч
	if(strSQL.empty() || pRecordSet == NULL)
	{
		m_iErrorCode = INVALID_PARAMETER;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg += " Call Invalid!SQL String or RecordSet pointer is NULL";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PARAMETER;
	}
	
	//ǰ������������
	if(m_bConnected == false || m_pConn == NULL ) 	 
	{
		m_iErrorCode = INVALID_PRE_CONDITION;
		m_strErrorMsg = szMethodName;
		m_strErrorMsg += " Call Invalid!Database Not Connected yet";

		if(m_bOutputErrorMsg)
		{	
			cerr<<szMethodName<<" Call Invalid!"<<endl;
			cerr<<"Code: "<<m_iErrorCode<<" Msg: "<<m_strErrorMsg<<endl;
		}

		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);
		return INVALID_PRE_CONDITION;
	}

	int iReturn  = 0;

    	string strSqlStmt = strSQL;
    	ResultSet* pResultSet = NULL;
    	Statement* pStmt = NULL;
	try
	{
	    	pStmt = m_pConn->createStatement(strSqlStmt);
		pResultSet = pStmt->executeQuery();

		// ����CRecordSet
		pRecordSet->Init(this, pStmt, pResultSet);

		m_iErrorCode = 0;
		m_strErrorMsg = "Success";
		//add by yangh 2005-10-28
		m_pConn->terminateStatement(pStmt);
	}
	catch(SQLException ex)
	{
		m_iErrorCode = ex.getErrorCode();
		m_strErrorMsg = ex.getMessage();
		
		if(m_bOutputErrorMsg)
		{
			cerr<<szMethodName<<" Failed! SQL String: "<<strSQL<<endl;
			cerr<<"Code: "<<ex.getErrorCode()<<" Msg: "<<ex.getMessage() << endl;
		}

		iReturn = m_iErrorCode;
		//�����׳���׽�����쳣
		THROW_EXCEPTION(m_iErrorCode, m_strErrorMsg);		
	}
	   	
    	return iReturn;
}

bool CDatabase::StartTimer()
{
	struct sigaction sysact ;
	
	sigemptyset(&sysact.sa_mask);
	sysact.sa_flags = SA_SIGINFO;
	sysact.sa_sigaction = CDatabase::TimerRoutine;
	sigaction(m_nSignalID, &sysact, NULL);

	struct sigevent evp;
	
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = m_nSignalID;
	evp.sigev_value.sival_ptr = &m_nTimerID;
	int nCreate = timer_create(CLOCK_REALTIME, &evp, &m_nTimerID);

	if (nCreate == 0) //success
	{
		struct itimerspec value;
		struct itimerspec ovalue;

		value.it_value.tv_sec = m_nElapse / 1000; 
		value.it_value.tv_nsec = (m_nElapse % 1000) * (1000 * 1000);
		value.it_interval.tv_sec = value.it_value.tv_sec;
		value.it_interval.tv_nsec = value.it_value.tv_nsec;

		if (timer_settime(m_nTimerID, 0, &value, &ovalue) == 0) //success
		{
			//cout<<"Timer id:"<<m_nTimerID<<" nElaspe:"<<m_nElapse<<endl;
			m_bTimerStarted = true;
			return true;
		}
		else
			return false;
	}
	else
	{
		//cout<<"create timer error"<<endl;
		timer_delete(m_nTimerID);
		return false;
	}
}

bool CDatabase::KillTimer()
{
	if(timer_delete(m_nTimerID) == 0)
	{
		m_bTimerStarted = false;
		return true;
	}
	else
		return false;
}	

void CDatabase::TimerRoutine(int signo, siginfo_t* info, void* context)
{
	if (signo != m_nSignalID) return;

	CDatabase* pObj = NULL;
	Connection* pConn = NULL;
	for(int i = 0; i < m_vObj.size(); i ++)
	{
	    pObj = m_vObj[i].pObj;
	    if(pObj->m_bKeepLongConnection && pObj->m_pConn != NULL )
	    {
		    Statement *pStmt = NULL;	  
		    ResultSet* pRset = NULL;

	    	    pConn = pObj->m_pConn;
	    	    try
	    	    {
			    string sqlStmt = "SELECT TO_CHAR(SYSDATE,'YYYYMMDDHH24MISS') FROM DUAL";
			    pStmt = pConn->createStatement(sqlStmt);
			    pRset = pStmt->executeQuery();
			    pRset->next();

			    //�����ʱ����Ϣ
			/*	
	 	          while (pRset->next ())
			    {
			      cout << "  DB Sysdate: " << pRset->getString (1) << endl;
			    }
			*/    
		    }
	    	    catch(SQLException ex)
		    {
		    	     //�������쳣
		    		/*
			     cout<<"Exception thrown for display DB System Date"<<endl;
			     cout<<"Error number: "<<  ex.getErrorCode() << endl;
			     cout<<ex.getMessage() << endl;
			     */
		    }
	    	    //�������׳��쳣
		    /*	
		    catch(...)
		    {
		    }
		    */
		    pStmt->closeResultSet (pRset);
		    pConn->terminateStatement(pStmt);
	    }
	}
}

// class CDBException

CDBException::CDBException()
{
	m_iErrorCode = 0;
	m_strErrorMsg = "Success";
}

CDBException::CDBException(int iCode, const string& strMsg)
{
	m_iErrorCode = iCode;
	m_strErrorMsg = strMsg;
}

CDBException::~CDBException()
{
}
	
int CDBException::GetErrorCode()
{
	return m_iErrorCode;
}

string CDBException::GetErrorMsg()
{
	return m_strErrorMsg;
}
