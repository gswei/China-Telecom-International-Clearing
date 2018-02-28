// 数据库接口类
// 作者：周伟(zhouwei@gsta.com;wilston1979@tom.com)
// 最新版本：1.09，2005-4-1
// 声明：此类为免费共享类，适用于Oracle9i以上版本数据库
// 如果有使用上的不明之处或者发现bug，请及时告知本人
// 更改记录
//2004-9-29	  Version 1.0beta
//2004-10-11  Version 1.00 	应王辉要求，增加异常抛出和数据类型转换功能，
//							同时将接口中的const char*替换为string
//2004-10-14  Version 1.00		应孙华要求，为方便代码移植。仿照dbopertion.pc
//							增加CBindSQL类，重载operator输入输出符，支持输入参数，简化使用
//2004-10-15  Version 1.00 	使用ARRAY方式，即通过setDataBuffer传递参数，
//							从而调用executeArrayUpdate和Array Fetch，批量读写，提高效率
//							(AddIteration方式不适用于String对象，setMaxParamSize不知咋办，总有ORA-01459错误)
//2004-10-18  Version 1.00		考虑对于CBindSQL类，增加以下三个功能
//							1 自动判断输入绑定变量的数目
//							2 支持执行存储过程与函数
//							3 不需要Close，即可重复执行SQL(暂时放弃)
//							4 仿照Dboperation的DynamicSQL类，在输出每行记录的第一列时读取该行记录，
//							   这样的话，在读取只有一行的记录集时，读取一行之后，
//							   可以分清执行错误(IsError)和读取完毕(IsEnd)两种情况
//2004-10-20  Version 1.00		1 纠正了记录集不返回任何记录时输出列数值出现的underflow异常
//							2  CBindSQL::Execute异常时，输出SQL信息
//							3 输入字符类型参数改用OCCI_SQLT_CHR类型传递，这样不用多传一个NULL结束符
//							4 王辉提出要确保线程安全性
//							5 孙华提出执行不输入绑定变量的SQL(已经完成)
//							6 王冬升提出增加输入绑定变量为空串的处理(已经完成)
//2004-10-21  Version 1.00		1 为CBindSQL类增加DDL SQL处理功能
//2004-10-28  Version 1.01		1 周乐航发现输出纪录为空时流输出函数会抛出异常
//2004-11-2	  Version 1.02		1 为CBindSQL类开放CRecordSet接口
//2004-11-3	  Version 1.02		2 未能解决王冬升发现的无法批量插入的问题,原因分析
//2004-11-5 	  Version 1.02		3 解决CRecordSet::Open的Core Dump Bug
//2004-11-8	  Version 1.02		4 王辉要求CRecordSet::Fetch系列方法输出错误信息需要包含SQL
//							5 重新规划异常处理，CFieldValue异常全部使用CDBException做为内部异常，而不是_EXCEPTION宏
//							6 孙华要求，输出详细错误信息(绑定变量值)，方便调用者定位错误
//							7 重新规划错误信息,只有CDatabase类和CBindSQL类保存上一次的数据库错误信息和自定义异常信息
//2004-11-12  Version 1.02		8 吴磊发现CBindSQL Select 耗时是dbopeartion.pc的30 倍，
//							   发现主要是string类型错误信息初始化所导致，OCCI极限大概是dbopeartion.pc的1.5倍，
//							   调优后本接口效率大约是dbopeartion.pc的2-3倍
//							9 周乐航发现CBindSQL Insert单条插入一秒钟约300条，效率偏低
//							  调优后对于NONSELECT_DML类型SQL，允许不Close	，重新输入绑定变量并执行，可提高效率
//2004-11-17  Version 1.03		1 为提高效率将CRecordSet和CDatabase类中的常量string也用char[]代替
//							2 解决了周乐航/肖元红等发现的程序正常退出(exit, return)时会提交的问题
//							   解决方法是~CDatabase中增加回滚操作			
//							3 为CRecordSet、CBindSQL、CDatabase增加m_bOutputErrorMsg属性的Get/Set接口，缺省值为true
//2004-11-18  Version 1.03		4 字符串以及单个字符都采用OCCI_SQLT_STR传输，虽然多了一个null结束符，但是并不记入长度
//							   不象OCCI_SQLT_AFC会把NULL结束符也记入长度，产生过长无法插入问题，
//							   也不象OCCI_SQLT_CHR虽然少传输保存了一个null结束符，但是会产生ORA-01461号错误
//							    需要注意的是使用OCCI_SQLT_STR后面要追加8个NULL结束字符，这样才不会产生ORA-01461号错误
//								只加1个或者是2个仍然会产生ORA-01461号错误
//2004-11-23  Version 1.04		1 应吴磊要求,在CBindSQL::Open之前,先判断是否初始或者Close过的对象,如果不是,则先Close之
//2004-12-2	  Version 1.04		2 全面使用const string&代替string，提高参数传输效率
//							3 邹国栋发现CDatabase::Connect和CDatabase::Disconnect不够健壮，
//								因此为这两个方法增加对于m_bConnected连接标志的判断
//2004-12-20  Version 1.04a	吴磊修改1 对CBindSQL::Execute()和CBindSQL::Open()两个函数的处理修改，
//								在这两个函数中如果出现异常跑出的情况，
//								先调用CBindSQL::Close()函数关闭，然后再抛出异常，
//								避免多次出现数据库操作失败以后，没有调用CBindSQL::Close(),
//								导致打开游标数目超出限制，程序推出的情况.
//2004-12-30  Version 1.05		1 考虑到Exception或者是使用CBindSQL对象，忘记Close的情况
//							   在Open之前做Close操作; 在CBindSQL撤销时,也作Close操作. 自动处理关闭游标
//							避免ORA-01000号错误。另外一种办法是在CBindSQL类的所有抛出异常之前执行Close() 操作，
//							但是修改代码工作量太大
//2005-1-6	  Version 1.05		2 错误信息的分隔符改用宏定义代替，方便以后修改
//2005-3-3	  Version 1.06		1 吴磊发现，有的编译器不支持输入输出float类型。float数据类型不能自动转换为double类型
//							因此，增加CBindSQL流输入输出float类型参数的函数。
//2005-3-11	  Version 1.07		1 为支持gcc 2.95编译器，修改代码
//							2 为提高灵活性，不是使用Connection*  传递数据库连接，而是通过CDatabase对象指针
//2005-3-15	  Version 1.07		3 Oracle服务器管理连接时在一定时长之后可能踢连接
//								为了保持持久连接,修改所有接口实现, 定期做查询续上连接
//							4 异常情况，如网络故障、数据库服务器重启、网线拔断由使用接口的应用程序来处理
//							5 增加了CBindSQL::IsNull接口
//2005-3-16 	  Version 1.08		1 吴磊提出多进程支持的问题 	，暂时通过修改CDatabase::Connect函数来解决
//2005-3-22	  Version 1.08		2 周亚军建议将头文件中的USE_CLASSICAL_IOSTREAM_LIB宏定义块移到源文件中，解决编译问题
//2005-3-22	  Version 1.09		1 使用进程池支持多进程, 未实行
//2005-4-1	  Version 1.09		1 张郭强发现数据库连接刷新时间不是5分钟而是300毫秒
//2005-11-23	  Version 1.10		发现统计程序插入表，部分记录传入的字符串字段值非空，但实际插入时报不能插入NULL值，检查发现批量插入时,
//						出现NULL错误的字段,发生错误的同批数据第一个记录的值比后面的短，怀疑是setDataBuffer的size字段取
//						第一个字段的长度导致，在PrepareArrayUpdateData()中将存储同一字符串字段的一批记录的存储空间
//						改为等长,不再象原来那样按需分配,故障消失
//2005-12-31 Version 1.11 孙华发现数据库连接物理断开时，程序会core。在析构函数中先检测一把连接状态。
//2006-04-10 Version 1.12 谭杰发现从数字型字段转换成string类型输出时，如果字段值为空，则会抛出异常。增加判定条件后正常
//2006-4-21 Version1.13			含occi接口的程序在solaris2.6上会导致机器宕机，Sun说是2.6的一个bug：timer_create输入参数检查不够，
//					进而发现传入该函数第二个参数的sigev_signo为0，不在有效区内。估计是在solaris上静态变量的初始化
//					不能为函数（SIGRTMAX是sysconf()的宏定义）。在构造函数中加上付值，sigev_signo不再为0
//2006-4-30 Version1.14  清理了一个内存泄漏的问题
//2006-08-01 Version1.15 增加函数GetUpdateCount()，执行更新时返回收到影响的记录条数
//2007-10-19 Version1.16 zhouyj发现抛出异常时，错误信息是换了行的，修改成不换行方式。
//2007-10-31 Version1.17 zgq需要将CBindSQL bs作为一个私有变量，接口做相应修改，不会影响到现有的调用方式。但是这么申明后，必须调用初始化函数init(DBConn)。
//2007-11-20 Version1.18 修改调用存储过程时出现的一个问题。当无输入参数时，组合的sql语句有问题，多一个括号。
//2007-11-22 Version1.19 使用连接池的方式，用于多线程的操作，最多10个连接。同时开放CBindSQL的Commit/Rollback接口，可防止用户直接对数据库连接进行操作。
//2008-3-12  Version1.20 冲销程序特殊处理，需屏蔽一下信号量，故将timer的操作设为public.
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

//#define NULL 0	//用于指针
#define ZERO 0	//用于数值

#define EXECUTE_SUCCEED 			1

class CDatabase;
#define USE_GLOABLE_POOL
#define MAX_POOL_NUM 10

#ifdef USE_GLOABLE_POOL
extern CDatabase DBPool[MAX_POOL_NUM];  //默认MAX_POOL_NUM个连接
#endif

//是否定义全局的CDatabase对象
#define USE_GLOABLE_DB_CONNECTION

#ifdef USE_GLOABLE_DB_CONNECTION
extern CDatabase DBConn;
#endif

//删除抛出信息的换行符  add by yangh 2007-10-19
void DelSwitchLine(string &ss);
int getMyEnv(const char *fname, const char *comp, char *ret );
int createDBPool(char *pchEnvFile,int num=10);
int GetConnectionId();    //获取连接id
int ReleaseId(int);    //释放连接
void DisconnectPool();

//字段值类，目前只支持Number和String类型
//用于统一存储数据库表字段和绑定变量，以及类型转换
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
				//内联函数无法使用CRecordSet::PrintType和CDBException构造函数，
				//放弃输出详细错误信息
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

//记录集
class CRecordSet 
{
	friend class CDatabase;
	friend class CBindSQL;
	static string GetUpperString(const string& str);
  private:
  	bool				m_bOutputErrorMsg;
  	//使用CDatabase对象管理数据库连接，代替Connection*指针，更加灵活
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

	// 通过CFieldValue对象传递字段值
	int FetchFieldValue(int nColIndex, CFieldValue& fieldValue);

	//获取Fetch系列函数的附加错误信息
	string GetAdditionErrorMsg(int nColIndex);
	string GetAdditionErrorMsg(const string& strColName);
	
public:
	static string PrintType (int type);

	//计划修改Fetch系列函数，使先统一获得CFieldValue对象，
	//然后由CFieldValue对象转换为C++对象
	//CFieldValue& FetchValue(int nColIndex) {}
  public:
    //## Constructors (generated)
      CRecordSet();
	
    //## Destructor (generated)
      ~CRecordSet();

   //	返回ErrorCode
      int ReadRecordAndNext();
      int IsEnd();

      int Open(CDatabase& db, const string& strSQL);
      int Close();

      //	返回值
      //	1：为空；0：异常；-1：非空
      int IsNull (int nColIndex);

      //	返回布尔型
      //	1：为空；0：异常；-1：非空
      int IsNull (const string& strColName);

      //	成功返回字符串，失败返回空指针
      //	对于非字符串类型，则自动转换为字符串类型
      //	日期时间类型转换格式是YYYYMMDDHH24MISS
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

      //	返回布尔型
      //	1：成功；0：失败
      int FetchDateTime (int nColIndex, int& nYear, int& nMonth, int& nDay, int& nHour, int& nMinute, int& nSecond);

      //	返回布尔型
      //	1：成功；0：失败
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

// 错误处理
  bool			m_bOutputErrorMsg;
  int				m_iErrorCode;
  string			m_strErrorMsg;

// 连接数据库状态标志
  bool			m_bConnected;

// 保存登录用户名/密码等，用于Copy Constructor
 string			m_strDatabase;
 string			m_strUserName;
 string			m_strPassword;

  //OCCI 对象
  Environment *	m_pEnv;
  Connection *	m_pConn;

	//	返回ErrorCode                                               
	int ExecuteCmd (const string& strSQL);                          
	                                                                
	//	返回ErrorCode                                               
	int ExecuteQuery (const string& strSQL, CRecordSet* pRecordSet);
	

	//保持长连接的计时器
	bool m_bKeepLongConnection;
	static bool m_bTimerStarted;
	static timer_t	m_nTimerID;	//定时器编号
  	static int m_nElapse;	//时钟周期
  	static int m_nSignalID;

	//用于保存所有CDatabase对象指针
	int m_nSeq;		// 用于标识this对象的编号
  	static int m_nCurrentSeq;
  	static vector<CDBObj> m_vObj;
  //2008-3-12 update by yh
	//static bool StartTimer();
	//static bool KillTimer();
	static void TimerRoutine(int signo, siginfo_t* info, void* context);

  protected:
	//版本号
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
     //	返回ErrorCode
      int Connect (const string& strUserName, const string& strPassword, const string& pstrDatabase);

      //	返回ErrorCode
      //## Preconditions:
      //	Connect
      int Disconnect ();
      int Reconnect ();

	//返回连接数据库标识
      int IsConnected();
      
      int GetLastDBErrorCode ();

      string GetLastDBErrorMsg ();

      //	返回ErrorCode
      int Commit ();

      //	返回ErrorCode
      int Rollback ();
	static char* getVersion();
      ////	返回ErrorCode
      //int ExecuteCmd (const string& strSQL);
      //
      ////	返回ErrorCode
      //int ExecuteQuery (const string& strSQL, CRecordSet* pRecordSet);

	bool GetOutputErrorMsg() { return m_bOutputErrorMsg; };
	void SetOutputErrorMsg(bool bValue) { m_bOutputErrorMsg = bValue; }

	bool GetKeepDBLongConnection() { return m_bKeepLongConnection; };
	void SetKeepDBLongConnection(bool bValue) { m_bKeepLongConnection = bValue; }

};


//支持绑定变量的DML SQL
//支持批量读写(类似Pro*C的游标方式)
//不支持输出绑定变量(OUT BIND Variable)，即如SELECT...INTO...和存储过程调用等暂不支持
enum		SQLType
{
	SELECT_QUERY = 0,						// SELECT 查询
	FUNCTION_OR_PROCEDURE = 1,			// 函数或者存储过程
	NONSELECT_DML = 2,		// DELETE . INSERT. UPDATE DML语句
	SQL_DDL = 3,			// Create Or Alter等DDL SQL语句
	INVALID_TYPE 						// 不支持的SQL类型
};

// 存储过程的IO类型定义
enum 		IOMode
{
	IN	= 0,
	OUT = 1,
	INOUT = 2
};

class CBindSQL
{
private:
  	//使用CDatabase对象管理数据库连接，代替Connection*指针，更加灵活
  	CDatabase*		m_pDBConn;
	Statement*	m_pStmt;
	CRecordSet*	m_pRecordSet;

	vector<int>			m_vInputParamDataType;
	vector<CFieldValue>	m_varInputParam;
	vector<void*>		m_vpData;
	vector<ub2*>			m_vpLength;
	
	// 错误信息
	int			m_iErrorCode;
	string		m_strErrorMsg;

	int			m_eSQLType;	
	int    m_effectCount;//受影响行数
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

	//检查operator << 系列函数调用的合法性
	int CheckValid_InputParam(const char* pszMethodName);
	//检查operator >> 系列函数调用的合法性
	int CheckValid_OutputParam(const char* pszMethodName);
	
	int Before_InputParam(const char* pszMethodName, int nParamIndex);	// Add Iteration
	int Before_OutputParam();	// 准备数据
	int After_OutputParam(const string& strMethodName, int nParamIndex, const string& strErrorMsgAdd);	// 读下一行

	int Account_InputParamIndex(int nPos);
	int Account_OutputParamIndex(int nPos);

	int PrepareArrayUpdateData();
	//输出填充的DataBuffer 数据，用于测试
	int DumpOut();

	int GetInputParamCount() { if (m_nInputParamCount <= 0) return m_nInputParamPos; else return m_nInputParamCount; }
	void SetInputParamCount(int nCount) { m_nInputParamCount = nCount; m_vInputParamDataType.resize(nCount); }
	//根据SQL获取输入绑定变量的个数
	int FigureInputParamCount(const string& strSQL, int eSQLType);

	//  存储过程相关
	int GetProcInfo(const string& strProcName);
	string GetExecuteProcSQL(const string& strProcName);

	//获取数据库操作函数的附加错误信息
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
	//CRecordSet接口，适用于SELECT 查询，
	//可以通过此接口选择使用CRecordSet输出，替代流输出
	CRecordSet* GetRecordSet();
	
	//流读写
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
	bool IsEnd();	//记录集是否到了最后一行
	bool IsError();	//即判断上次DB 操作，是否出错
	bool IsNull(int nColIndex);		// 判断某列是否为空
	bool IsNull(const string& strColName);	// 判断某列是否为空
	int GetLastDBErrorCode ();
       string GetLastDBErrorMsg ();
  int GetUpdateCount();//返回受到影响的记录条数。更新或删除时为影响的记录数
	bool GetOutputErrorMsg() { return m_bOutputErrorMsg; };
	void SetOutputErrorMsg(bool bValue) { m_bOutputErrorMsg = bValue; }
       
};


//CDBException:可用来处理所有CDatabase和CRecordSet的异常错误
//数据库错误通过包装SQLException类来处理，
//无效参数或者前置条件不满足单独开发代码来实现
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
