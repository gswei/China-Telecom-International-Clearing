 /****************************************************************

 filename: CExpCal.h

 module: SC;

 created by:	daiw

 create date:	2014-06-13

 version: 3.0.0

 description: 

	M(SC2iQ/

 update:



 *****************************************************************/



#ifndef _CEXPCAL_H_

#define _CEXPCAL_H_  1

 

#include <sys/time.h>

#include "es/util/StringUtil.h"

#include "CF_CPlugin.h"

#include "CF_CException.h"

#include "CF_Common.h"

#include "ComFunction.h"

#include "CF_Lack_Abn_Code.h"

#include "CF_PREP_Error.h"

#include "CommonPluginFactory.h"

class SFieldValue
{
public:
	int iFlag;
	string sField;
};


template <class Type>       
class STACK{                 //����ջ��
private:
	Type base[MSG_LEN];
	int Size;
public:
	STACK(){Size=0;};
	void push(Type a)     //��ջ
	{
		base[Size]=a;
		Size++;
	}
	Type pop()            //��ջ
	{
		return base[--Size];
	}
	int size()
	{return Size;}
};



class Calculate_Cla
{
public:
	Calculate_Cla()
	{}

	~Calculate_Cla()
	{}
	
	bool IsData(char);
	
	bool IsSym(char);
	
	int IsPar(char);
	
	bool Check(char *);
	
	int setPri(char);                 //�жϷ��ŵ����ȼ���
	
	double ToData(char*);               //���ַ���ת��Ϊ��ֵ
	
	double Call(double data,double sum,char ch);    //���尴���ż���
	
	int GetMatch(char* buffer,int pos); //����ջ�ҵ�ƥ�������
	
	void Opr( STACK<char>&, STACK<double>&, int& ); //����ջ����
	
	double Calculate(char*, double& );   //�ַ����Ķ��뼰����

};



//����������
class CParaParser
{

public:
	CParaParser();

	~CParaParser();

	bool parseFormular(char* exp);

	bool isVariable(int& pos,string& svalue);

	bool isFieldNumber(int& pos,string& svalue);

	bool isSymbol(int& pos,string& svalue);

	void Space(int& pos);

	vector<SFieldValue> getParseVec();

	string getParseExpress(PacketParser& pps);

	string transform(int n,int imethod,double dres);

	enum ExpType
	{
		NUMB = 0,//����
		VARI = 1,//����
		SYMB = 2,//�����
	};

private:

	int _index;
	
	string _expr,_target;

	vector<SFieldValue> _vStack;



};


/**
  ���ʽ������ExpCal(express)
  -----------------------------------------------------------------------
  ���ʽ��Ϊ�������
  ���ʽ������$���ŵĲ������
  ������ֻ֧����������,��֧��λ����,֧��С��,����������,��Ҫ�����Ž���������
  ��������ı��ʽ������$���ſ�ʼ(�Ƿ��޸Ĵ���)
  -----------------------------------------------------------------------
**/
class CExpCal: public BasePlugin

{

public:	

	CExpCal();

	void init(char *szSourceGroupID, char *szServiceID, int index);

	void execute(PacketParser& pps,ResParser& retValue);

	void message(MessageParser&  pMessage);

	std::string getPluginName();

	std::string getPluginVersion();

	void printMe();

	~CExpCal();


private:

	int dealMethod;

	CParaParser _parser;

	bool bHavParse;//�Ƿ��Ѿ�������ʶ

	vector<SFieldValue> _vParseField;

	string _needParseOperat,_result;

	char _cbuf[FIELD_LEN];
	char _cpos[FIELD_LEN];
	char _cmethod[FIELD_LEN];
	char _cfrontfield[FIELD_LEN];

	Calculate_Cla _cl;//����������ʽ����������

};


#endif



