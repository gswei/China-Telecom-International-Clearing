// ���ݿ�ӿ���
// ���ߣ���ΰ(zhouwei@gsta.com;wilston1979@tom.com)
// ���°汾��1.09��2005-4-1
// ����������Ϊ��ѹ����࣬������Oracle9i���ϰ汾���ݿ�
// �����ʹ���ϵĲ���֮�����߷���bug���뼰ʱ��֪����
// ���ļ�¼
//2004-9-29	  Version 1.0beta
//2004-10-11  Version 1.00 	Ӧ����Ҫ�������쳣�׳�����������ת�����ܣ�
//							ͬʱ���ӿ��е�const char*�滻Ϊstring
//2004-10-14  Version 1.00		Ӧ�ﻪҪ��Ϊ���������ֲ������dbopertion.pc
//							����CBindSQL�࣬����operator�����������֧�������������ʹ��
//2004-10-15  Version 1.00 	ʹ��ARRAY��ʽ����ͨ��setDataBuffer���ݲ�����
//							�Ӷ�����executeArrayUpdate��Array Fetch��������д�����Ч��
//							(AddIteration��ʽ��������String����setMaxParamSize��֪զ�죬����ORA-01459����)
//2004-10-18  Version 1.00		���Ƕ���CBindSQL�࣬����������������
//							1 �Զ��ж�����󶨱�������Ŀ
//							2 ֧��ִ�д洢�����뺯��
//							3 ����ҪClose�������ظ�ִ��SQL(��ʱ����)
//							4 ����Dboperation��DynamicSQL�࣬�����ÿ�м�¼�ĵ�һ��ʱ��ȡ���м�¼��
//							   �����Ļ����ڶ�ȡֻ��һ�еļ�¼��ʱ����ȡһ��֮��
//							   ���Է���ִ�д���(IsError)�Ͷ�ȡ���(IsEnd)�������
//2004-10-20  Version 1.00		1 �����˼�¼���������κμ�¼ʱ�������ֵ���ֵ�underflow�쳣
//							2  CBindSQL::Execute�쳣ʱ�����SQL��Ϣ
//							3 �����ַ����Ͳ�������OCCI_SQLT_CHR���ʹ��ݣ��������öഫһ��NULL������
//							4 �������Ҫȷ���̰߳�ȫ��
//							5 �ﻪ���ִ�в�����󶨱�����SQL(�Ѿ����)
//							6 �����������������󶨱���Ϊ�մ��Ĵ���(�Ѿ����)
//2004-10-21  Version 1.00		1 ΪCBindSQL������DDL SQL������
//2004-10-28  Version 1.01		1 ���ֺ����������¼Ϊ��ʱ������������׳��쳣
//2004-11-2	  Version 1.02		1 ΪCBindSQL�࿪��CRecordSet�ӿ�
//2004-11-3	  Version 1.02		2 δ�ܽ�����������ֵ��޷��������������,ԭ�����
//2004-11-5 	  Version 1.02		3 ���CRecordSet::Open��Core Dump Bug
//2004-11-8	  Version 1.02		4 ����Ҫ��CRecordSet::Fetchϵ�з������������Ϣ��Ҫ����SQL
//							5 ���¹滮�쳣����CFieldValue�쳣ȫ��ʹ��CDBException��Ϊ�ڲ��쳣��������_EXCEPTION��
//							6 �ﻪҪ�������ϸ������Ϣ(�󶨱���ֵ)����������߶�λ����
//							7 ���¹滮������Ϣ,ֻ��CDatabase���CBindSQL�ౣ����һ�ε����ݿ������Ϣ���Զ����쳣��Ϣ
//2004-11-12  Version 1.02		8 ���ڷ���CBindSQL Select ��ʱ��dbopeartion.pc��30 ����
//							   ������Ҫ��string���ʹ�����Ϣ��ʼ�������£�OCCI���޴����dbopeartion.pc��1.5����
//							   ���ź󱾽ӿ�Ч�ʴ�Լ��dbopeartion.pc��2-3��
//							9 ���ֺ�����CBindSQL Insert��������һ����Լ300����Ч��ƫ��
//							  ���ź����NONSELECT_DML����SQL������Close	����������󶨱�����ִ�У������Ч��
//2004-11-17  Version 1.03		1 Ϊ���Ч�ʽ�CRecordSet��CDatabase���еĳ���stringҲ��char[]����
//							2 ��������ֺ�/ФԪ��ȷ��ֵĳ��������˳�(exit, return)ʱ���ύ������
//							   ���������~CDatabase�����ӻع�����			
//							3 ΪCRecordSet��CBindSQL��CDatabase����m_bOutputErrorMsg���Ե�Get/Set�ӿڣ�ȱʡֵΪtrue
//2004-11-18  Version 1.03		4 �ַ����Լ������ַ�������OCCI_SQLT_STR���䣬��Ȼ����һ��null�����������ǲ������볤��
//							   ����OCCI_SQLT_AFC���NULL������Ҳ���볤�ȣ����������޷��������⣬
//							   Ҳ����OCCI_SQLT_CHR��Ȼ�ٴ��䱣����һ��null�����������ǻ����ORA-01461�Ŵ���
//							    ��Ҫע�����ʹ��OCCI_SQLT_STR����Ҫ׷��8��NULL�����ַ��������Ų������ORA-01461�Ŵ���
//								ֻ��1��������2����Ȼ�����ORA-01461�Ŵ���
//2004-11-23  Version 1.04		1 Ӧ����Ҫ��,��CBindSQL::Open֮ǰ,���ж��Ƿ��ʼ����Close���Ķ���,�������,����Close֮
//2004-12-2	  Version 1.04		2 ȫ��ʹ��const string&����string����߲�������Ч��
//							3 �޹�������CDatabase::Connect��CDatabase::Disconnect������׳��
//								���Ϊ�������������Ӷ���m_bConnected���ӱ�־���ж�
//2004-12-20  Version 1.04a	�����޸�1 ��CBindSQL::Execute()��CBindSQL::Open()���������Ĵ����޸ģ�
//								����������������������쳣�ܳ��������
//								�ȵ���CBindSQL::Close()�����رգ�Ȼ�����׳��쳣��
//								�����γ������ݿ����ʧ���Ժ�û�е���CBindSQL::Close(),
//								���´��α���Ŀ�������ƣ������Ƴ������.
//2004-12-30  Version 1.05		1 ���ǵ�Exception������ʹ��CBindSQL��������Close�����
//							   ��Open֮ǰ��Close����; ��CBindSQL����ʱ,Ҳ��Close����. �Զ�����ر��α�
//							����ORA-01000�Ŵ�������һ�ְ취����CBindSQL��������׳��쳣֮ǰִ��Close() ������
//							�����޸Ĵ��빤����̫��
//2005-1-6	  Version 1.05		2 ������Ϣ�ķָ������ú궨����棬�����Ժ��޸�
//2005-3-3	  Version 1.06		1 ���ڷ��֣��еı�������֧���������float���͡�float�������Ͳ����Զ�ת��Ϊdouble����
//							��ˣ�����CBindSQL���������float���Ͳ����ĺ�����
//2005-3-11	  Version 1.07		1 Ϊ֧��gcc 2.95���������޸Ĵ���
//							2 Ϊ�������ԣ�����ʹ��Connection*  �������ݿ����ӣ�����ͨ��CDatabase����ָ��
//2005-3-15	  Version 1.07		3 Oracle��������������ʱ��һ��ʱ��֮�����������
//								Ϊ�˱��ֳ־�����,�޸����нӿ�ʵ��, ��������ѯ��������
//							4 �쳣�������������ϡ����ݿ���������������߰ζ���ʹ�ýӿڵ�Ӧ�ó���������
//							5 ������CBindSQL::IsNull�ӿ�
//2005-3-16 	  Version 1.08		1 ������������֧�ֵ����� 	����ʱͨ���޸�CDatabase::Connect���������
//2005-3-22	  Version 1.08		2 ���Ǿ����齫ͷ�ļ��е�USE_CLASSICAL_IOSTREAM_LIB�궨����Ƶ�Դ�ļ��У������������
//2005-3-22	  Version 1.09		1 ʹ�ý��̳�֧�ֶ����, δʵ��
//2005-4-1	  Version 1.09		1 �Ź�ǿ�������ݿ�����ˢ��ʱ�䲻��5���Ӷ���300����
//2005-11-23	  Version 1.10		����ͳ�Ƴ����������ּ�¼������ַ����ֶ�ֵ�ǿգ���ʵ�ʲ���ʱ�����ܲ���NULLֵ����鷢����������ʱ,
//						����NULL������ֶ�,���������ͬ�����ݵ�һ����¼��ֵ�Ⱥ���Ķ̣�������setDataBuffer��size�ֶ�ȡ
//						��һ���ֶεĳ��ȵ��£���PrepareArrayUpdateData()�н��洢ͬһ�ַ����ֶε�һ����¼�Ĵ洢�ռ�
//						��Ϊ�ȳ�,������ԭ�������������,������ʧ
//2005-12-31 Version 1.11 �ﻪ�������ݿ���������Ͽ�ʱ�������core���������������ȼ��һ������״̬��
//2006-04-10 Version 1.12 ̷�ܷ��ִ��������ֶ�ת����string�������ʱ������ֶ�ֵΪ�գ�����׳��쳣�������ж�����������
//2006-4-21 Version1.13			��occi�ӿڵĳ�����solaris2.6�ϻᵼ�»���崻���Sun˵��2.6��һ��bug��timer_create���������鲻����
//					�������ִ���ú����ڶ���������sigev_signoΪ0��������Ч���ڡ���������solaris�Ͼ�̬�����ĳ�ʼ��
//					����Ϊ������SIGRTMAX��sysconf()�ĺ궨�壩���ڹ��캯���м��ϸ�ֵ��sigev_signo����Ϊ0
//2006-4-30 Version1.14  ������һ���ڴ�й©������
//2006-08-01 Version1.15 ���Ӻ���GetUpdateCount()��ִ�и���ʱ�����յ�Ӱ��ļ�¼����
//2007-10-19 Version1.16 zhouyj�����׳��쳣ʱ��������Ϣ�ǻ����еģ��޸ĳɲ����з�ʽ��
//2007-10-31 Version1.17 zgq��Ҫ��CBindSQL bs��Ϊһ��˽�б������ӿ�����Ӧ�޸ģ�����Ӱ�쵽���еĵ��÷�ʽ��������ô�����󣬱�����ó�ʼ������init(DBConn)��
//2007-11-20 Version1.18 �޸ĵ��ô洢����ʱ���ֵ�һ�����⡣�����������ʱ����ϵ�sql��������⣬��һ�����š�
//2007-11-22 Version1.19 ʹ�����ӳصķ�ʽ�����ڶ��̵߳Ĳ��������10�����ӡ�ͬʱ����CBindSQL��Commit/Rollback�ӿڣ��ɷ�ֹ�û�ֱ�Ӷ����ݿ����ӽ��в�����
//2008-3-12  Version1.20 �����������⴦��������һ���ź������ʽ�timer�Ĳ�����Ϊpublic.
#ifndef CORACLE_DATABASE_ZHOUWEI_H
#define CORACLE_DATABASE_ZHOUWEI_H

#include <occi.h>
#include "CF_Config.h"
#include <pthread.h>
#include "CF_CEncrypt.h"

using namespace oracle::occi;
/*using namespace std;
using std::string;
using std::vector;
*/
#include <signal.h>

//#define NULL 0	//����ָ��
#define ZERO 0	//������ֵ

#define EXECUTE_SUCCEED 			1

class CDatabase;
#define USE_GLOABLE_POOL
#define MAX_POOL_NUM 10

#ifdef USE_GLOABLE_POOL
extern CDatabase DBPool[MAX_POOL_NUM];  //Ĭ��MAX_POOL_NUM������
#endif

//�Ƿ���ȫ�ֵ�CDatabase����
#define USE_GLOABLE_DB_CONNECTION

#ifdef USE_GLOABLE_DB_CONNECTION
extern CDatabase DBConn;
#endif

//ɾ���׳���Ϣ�Ļ��з�  add by yangh 2007-10-19
void DelSwitchLine(string &ss);
int getMyEnv(const char *fname, const char *comp, char *ret );
int createDBPool(char *pchEnvFile,int num=10);
int GetConnectionId();    //��ȡ����id
int ReleaseId(int);    //�ͷ�����
void DisconnectPool();

//�ֶ�ֵ�࣬Ŀǰֻ֧��Number��String����
//����ͳһ�洢���ݿ���ֶκͰ󶨱������Լ�����ת��
class CFieldValue
{
private:
	int m_iDataType;
	Number m_numValue;
	string m_strValue;

public:
	CFieldValue(const int& iParam) { m_numValue = iParam;	m_iDataType = OCCIINT;	}
	CFieldValue(const unsigned int& uParam) { m_numValue = uParam;	m_iDataType = OCCIUNSIGNED_INT;	}
	CFieldValue(const long& lParam) { m_numValue = lParam;	m_iDataType = OCCIINT;	}
	CFieldValue(const float& fParam) { m_numValue = fParam;	m_iDataType = OCCIFLOAT;	}
	CFieldValue(const double& dParam) { m_numValue = dParam;	m_iDataType = OCCIFLOAT;	}
	CFieldValue(const string& strParam) { m_strValue = strParam;	m_iDataType = OCCI_SQLT_STR;	}
	CFieldValue(const Number& numParam) { m_numValue = numParam;		m_iDataType = OCCI_SQLT_NUM;	}
public:
	CFieldValue& operator =(const CFieldValue& fieldValue) 
	{
			m_iDataType = fieldValue.m_iDataType;
			if(m_iDataType == OCCI_SQLT_NUM || m_iDataType == OCCIINT
				|| m_iDataType == OCCIUNSIGNED_INT || m_iDataType == OCCIFLOAT)
				m_numValue = fieldValue.m_numValue; 
			else if(m_iDataType == OCCI_SQLT_STR || m_iDataType == OCCI_SQLT_CHR
				|| m_iDataType == OCCI_SQLT_AFC || m_iDataType == OCCI_SQLT_VCS
				|| m_iDataType == OCCI_SQLT_AVC)
				m_strValue = fieldValue.m_strValue;
			else 
			{
				//���������޷�ʹ��CRecordSet::PrintType��CDBException���캯����
				//���������ϸ������Ϣ
				/*
				int iErrorCode = DATA_TYPE_NOT_SUPPORTED;
				string strErrorMsg = " datatype ";
				strErrorMsg += CRecordSet::PrintType(m_iDataType);
				strErrorMsg += " now NOT supported";
				throw CDBException(iErrorCode, strErrorMsg);
				*/
			}
			return *this;
	}
	
	inline operator int();	
	inline operator unsigned int();
	inline operator unsigned long();
	inline operator long();
	inline operator float();
	inline operator double();
	inline operator string();
	inline operator Number();
};

//��¼��
class CRecordSet 
{
	friend class CDatabase;
	friend class CBindSQL;
	static string GetUpperString(const string& str);
  private:
  	bool				m_bOutputErrorMsg;
  	//ʹ��CDatabase����������ݿ����ӣ�����Connection*ָ�룬�������
  	CDatabase*		m_pDBConn;
  	Statement*		m_pStmt;
	ResultSet*	 	m_pResultSet;

  	vector<string> 	m_vColName;
  	vector<int>	  	m_vColDataType;
  	vector<int>		m_vColDataSize;

  protected:
	int GetColIndexByName(const string& strColName);
	int GetColumnCount();
    	void Init(CDatabase* pDB, Statement* pStmt, ResultSet* pRs);

	// ͨ��CFieldValue���󴫵��ֶ�ֵ
	int FetchFieldValue(int nColIndex, CFieldValue& fieldValue);

	//��ȡFetchϵ�к����ĸ��Ӵ�����Ϣ
	string GetAdditionErrorMsg(int nColIndex);
	string GetAdditionErrorMsg(const string& strColName);
	
public:
	static string PrintType (int type);

	//�ƻ��޸�Fetchϵ�к�����ʹ��ͳһ���CFieldValue����
	//Ȼ����CFieldValue����ת��ΪC++����
	//CFieldValue& FetchValue(int nColIndex) {}
  public:
    //## Constructors (generated)
      CRecordSet();
	
    //## Destructor (generated)
      ~CRecordSet();

   //	����ErrorCode
      int ReadRecordAndNext();
      int IsEnd();

      int Open(CDatabase& db, const string& strSQL);
      int Close();

      //	����ֵ
      //	1��Ϊ�գ�0���쳣��-1���ǿ�
      int IsNull (int nColIndex);

      //	���ز�����
      //	1��Ϊ�գ�0���쳣��-1���ǿ�
      int IsNull (const string& strColName);

      //	�ɹ������ַ�����ʧ�ܷ��ؿ�ָ��
      //	���ڷ��ַ������ͣ����Զ�ת��Ϊ�ַ�������
      //	����ʱ������ת����ʽ��YYYYMMDDHH24MISS
      string FetchString (int nColIndex);

      string FetchString (const string& strColName);

      unsigned int FetchUInt (int nColIndex);

      unsigned int FetchUInt (const string& strColName);

      int FetchInt (int nColIndex);

      int FetchInt (const string& strColName);

      long FetchLong (int nColIndex);

      long FetchLong (const string& strColName);

      float FetchFloat (int nColIndex);

      float FetchFloat (const string& strColName);

      double FetchDouble (int nColIndex);

      double FetchDouble (const string& strColName);

      //	���ز�����
      //	1���ɹ���0��ʧ��
      int FetchDateTime (int nColIndex, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond);

      //	���ز�����
      //	1���ɹ���0��ʧ��
      int FetchDateTime (const string& strColName, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond);

	bool GetOutputErrorMsg() { return m_bOutputErrorMsg; };
	void SetOutputErrorMsg(bool bValue) { m_bOutputErrorMsg = bValue; }
};

struct CDBObj
{
	int nID;
	CDatabase* pObj;

	CDBObj(int nID, CDatabase* pObj)
	{
		this->nID = nID;
		this->pObj = pObj;
	}
};

class CDatabase 
{
	friend class CBindSQL;
  	friend class CRecordSet;
  private:

// ������
  bool			m_bOutputErrorMsg;
  int				m_iErrorCode;
  string			m_strErrorMsg;

// �������ݿ�״̬��־
  bool			m_bConnected;

// �����¼�û���/����ȣ�����Copy Constructor
 string			m_strDatabase;
 string			m_strUserName;
 string			m_strPassword;

  //OCCI ����
  Environment *	m_pEnv;
  Connection *	m_pConn;

	//	����ErrorCode                                               
	int ExecuteCmd (const string& strSQL);                          
	                                                                
	//	����ErrorCode                                               
	int ExecuteQuery (const string& strSQL, CRecordSet* pRecordSet);
	

	//���ֳ����ӵļ�ʱ��
	bool m_bKeepLongConnection;
	static bool m_bTimerStarted;
	static timer_t	m_nTimerID;	//��ʱ�����
  	static int m_nElapse;	//ʱ������
  	static int m_nSignalID;

	//���ڱ�������CDatabase����ָ��
	int m_nSeq;		// ���ڱ�ʶthis����ı��
  	static int m_nCurrentSeq;
  	static vector<CDBObj> m_vObj;
  //2008-3-12 update by yh
	//static bool StartTimer();
	//static bool KillTimer();
	static void TimerRoutine(int signo, siginfo_t* info, void* context);

  protected:
	//�汾��
	static float		m_fVersion;

  public:
    //## Constructors (generated)
      CDatabase();

    //## Copy Constructor (generated)
    	CDatabase(CDatabase& db);
    
    //## Destructor (generated)
      ~CDatabase();
      CDatabase& operator =(const CDatabase &right);

			bool StartTimer();
			bool KillTimer(); 
     //	����ErrorCode
      int Connect (const string& strUserName, const string& strPassword, const string& pstrDatabase);

      //	����ErrorCode
      //## Preconditions:
      //	Connect
      int Disconnect ();
      int Reconnect ();

	//�����������ݿ��ʶ
      int IsConnected();
      
      int GetLastDBErrorCode ();

      string GetLastDBErrorMsg ();

      //	����ErrorCode
      int Commit ();

      //	����ErrorCode
      int Rollback ();
	static char* getVersion();
      ////	����ErrorCode
      //int ExecuteCmd (const string& strSQL);
      //
      ////	����ErrorCode
      //int ExecuteQuery (const string& strSQL, CRecordSet* pRecordSet);

	bool GetOutputErrorMsg() { return m_bOutputErrorMsg; };
	void SetOutputErrorMsg(bool bValue) { m_bOutputErrorMsg = bValue; }

	bool GetKeepDBLongConnection() { return m_bKeepLongConnection; };
	void SetKeepDBLongConnection(bool bValue) { m_bKeepLongConnection = bValue; }

};


//֧�ְ󶨱�����DML SQL
//֧��������д(����Pro*C���α귽ʽ)
//��֧������󶨱���(OUT BIND Variable)������SELECT...INTO...�ʹ洢���̵��õ��ݲ�֧��
enum		SQLType
{
	SELECT_QUERY = 0,						// SELECT ��ѯ
	FUNCTION_OR_PROCEDURE = 1,			// �������ߴ洢����
	NONSELECT_DML = 2,		// DELETE . INSERT. UPDATE DML���
	SQL_DDL = 3,			// Create Or Alter��DDL SQL���
	INVALID_TYPE 						// ��֧�ֵ�SQL����
};

// �洢���̵�IO���Ͷ���
enum 		IOMode
{
	IN	= 0,
	OUT = 1,
	INOUT = 2
};

class CBindSQL
{
private:
  	//ʹ��CDatabase����������ݿ����ӣ�����Connection*ָ�룬�������
  	CDatabase*		m_pDBConn;
	Statement*	m_pStmt;
	CRecordSet*	m_pRecordSet;

	vector<int>			m_vInputParamDataType;
	vector<CFieldValue>	m_varInputParam;
	vector<void*>		m_vpData;
	vector<ub2*>			m_vpLength;
	
	// ������Ϣ
	int			m_iErrorCode;
	string		m_strErrorMsg;

	int			m_eSQLType;	
	int    m_effectCount;//��Ӱ������
	bool			m_bOutputErrorMsg;

	// IN Bind Variables position, the first is 1
	int			m_nInputParamPos;	

	// Output Columns position, the first is 2. do not consider OUT bind parameter now
	int			m_nOutputColumnPos;
	int			m_nOutputColumnIndex;

	int			m_nInputParamCount;

	vector<int>		m_vProcParamDataType;
	vector<string>	m_vProcParamName;
	vector<int>		m_vProcParamMode;
protected:

	//���operator << ϵ�к������õĺϷ���
	int CheckValid_InputParam(const char* pszMethodName);
	//���operator >> ϵ�к������õĺϷ���
	int CheckValid_OutputParam(const char* pszMethodName);
	
	int Before_InputParam(const char* pszMethodName, int nParamIndex);	// Add Iteration
	int Before_OutputParam();	// ׼������
	int After_OutputParam(const string& strMethodName, int nParamIndex, const string& strErrorMsgAdd);	// ����һ��

	int Account_InputParamIndex(int nPos);
	int Account_OutputParamIndex(int nPos);

	int PrepareArrayUpdateData();
	//�������DataBuffer ���ݣ����ڲ���
	int DumpOut();

	int GetInputParamCount() { if (m_nInputParamCount <= 0) return m_nInputParamPos; else return m_nInputParamCount; }
	void SetInputParamCount(int nCount) { m_nInputParamCount = nCount; m_vInputParamDataType.resize(nCount); }
	//����SQL��ȡ����󶨱����ĸ���
	int FigureInputParamCount(const string& strSQL, int eSQLType);

	//  �洢�������
	int GetProcInfo(const string& strProcName);
	string GetExecuteProcSQL(const string& strProcName);

	//��ȡ���ݿ���������ĸ��Ӵ�����Ϣ
	string GetInputAdditionErrorMsg(CFieldValue& paramValue,int nColIndex = 0);
	string GetInputAdditionErrorMsg(int nColIndex = 0);
	string GetOutputAdditionErrorMsg(int nColIndex = 0);
public:
	CBindSQL(CDatabase& db);
	CBindSQL();
	CBindSQL(int);
	void Init(CDatabase& db);
	~CBindSQL();
	
	int Open(const string& strSQL, int eSQLType = SELECT_QUERY);
	int Close();
	int Execute();
	int Commit();
	int Rollback();
	//CRecordSet�ӿڣ�������SELECT ��ѯ��
	//����ͨ���˽ӿ�ѡ��ʹ��CRecordSet�������������
	CRecordSet* GetRecordSet();
	
	//����д
	// For input parameters
	CBindSQL& operator <<(const long& lParam);
	CBindSQL& operator <<(const int& iParam);
	CBindSQL& operator <<(const double& dParam);
	CBindSQL& operator <<(const string& strParam);
	CBindSQL& operator <<(const char* pszParam);
	CBindSQL& operator <<(const char& cParam);
	CBindSQL& operator <<(const float& fParam);
	
	CBindSQL& InLong(const long& lParam);
	CBindSQL& InInt(const int& iParam);
	CBindSQL& InDouble(const double& dParam);
	CBindSQL& InString(const string& strParam);
	CBindSQL& InStrPointer(const char* pszParam);
	CBindSQL& InChar(const char& cParam);
	CBindSQL& InFloat(const float& fParam);
	
	// For result-set output
	CBindSQL& operator >>(long& lParam);
	CBindSQL& operator >>(int& iParam);
	CBindSQL& operator >>(double& dParam);
	CBindSQL& operator >>(string& strParam);
	CBindSQL& operator >>(char* pszParam);
	CBindSQL& operator >>(char& cParam);
	CBindSQL& operator >>(float& fParam);
	
	CBindSQL& OutLong(long& lParam);
	CBindSQL& OutInt(int& iParam);
	CBindSQL& OutDouble(double& dParam);
	CBindSQL& OutString(string& strParam);
	CBindSQL& OutStrPointer(char* pszParam);
	CBindSQL& OutChar(char& cParam);
	CBindSQL& OutFloat(float& fParam);

	operator bool();
	bool IsEnd();	//��¼���Ƿ������һ��
	bool IsError();	//���ж��ϴ�DB �������Ƿ����
	bool IsNull(int nColIndex);		// �ж�ĳ���Ƿ�Ϊ��
	bool IsNull(const string& strColName);	// �ж�ĳ���Ƿ�Ϊ��
	int GetLastDBErrorCode ();
       string GetLastDBErrorMsg ();
  int GetUpdateCount();//�����ܵ�Ӱ��ļ�¼���������»�ɾ��ʱΪӰ��ļ�¼��
	bool GetOutputErrorMsg() { return m_bOutputErrorMsg; };
	void SetOutputErrorMsg(bool bValue) { m_bOutputErrorMsg = bValue; }
       
};


//CDBException:��������������CDatabase��CRecordSet���쳣����
//���ݿ����ͨ����װSQLException��������
//��Ч��������ǰ�����������㵥������������ʵ��
class CDBException
{
protected:
	int m_iErrorCode;
	string m_strErrorMsg;
	
public:
	CDBException();
	CDBException(int iCode, const string& strMsg);
	~CDBException();
	
	int GetErrorCode();
	string GetErrorMsg();
};

#endif
