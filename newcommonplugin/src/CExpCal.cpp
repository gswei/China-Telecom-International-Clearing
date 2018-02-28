
  #include "CExpCal.h"


bool Calculate_Cla::IsData(char ch)      //�ж��������������Ƿ�Ϊ0-9
{
	return ((ch>='0'&&ch<='9')||ch=='.')?true:false;
}
  
bool Calculate_Cla::IsSym(char ch)      //�ж��Ƿ�����Ƿ������
{
	return (ch=='+'||ch=='-'||ch=='*'||ch=='/')?true:false;
}

int Calculate_Cla::IsPar(char ch)
{
	if(ch=='(')
		return 1;
	if(ch==')')
		return -1;
	return 0;
}

bool Calculate_Cla::Check(char *ch)
{
	int a=0;
	for(int i=0;i<strlen(ch);i++)
		if(ch[i]=='.')
			a++;
	if(a>1)
		return false;
	return true;
}

int Calculate_Cla::setPri(char ch)          //���ŵ����ȼ���
{
	switch(ch)
	{
	case '+':
		return 0;
	case '-':
		return 0;
	case '*':
		return 1;
	case '/':
		return 1;             
	default:
		return -1;
	}
} 

double Calculate_Cla::ToData(char* ch)   //������ת��Ϊ��ֵ
{
	int i,j,sumn=0;
	double sum=0.0;
	if(!Check(ch)) return 0.0;
	for(i=0;i<strlen(ch);i++)             //������������
	{
		if(ch[i]!='.')
			sumn=sumn*10+(ch[i]-'0');
		else break;
	}
	if(i<strlen(ch))
		for(j=i+1;j<strlen(ch);j++)        //С������
			sum=sum*10+(ch[j]-'0');
	sum /= pow(10.0,(double)(strlen(ch)-1-i));
	return (sum+sumn);                      //����ֵ
}

double Calculate_Cla::Call(double data,double sum,char ch)
{
	double ans=0.0;
	switch(ch)
	{
	case '+':
		ans=sum+data;        
		break;
	case '-':
		ans=sum-data;
		break;
	case '*':
		ans=sum*data;
		break;
	case '/':
		if( data != 0.0 )
			ans=sum/data;
		else
		{
			sprintf(ErrorMsg, "the express Appear number: 0 .");

			throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
		}
		break;              
	default:ans=0.0;
		break;    
	}
	return ans;
}

int Calculate_Cla::GetMatch(char* buffer,int pos)     //����ջ�ҵ�ƥ�������
{
	STACK<char> Temp;
	int i;
	for(i=pos;i<strlen(buffer);i++)
	{
		if(IsPar(buffer[i])==1)
			Temp.push('0');
		if(IsPar(buffer[i])==-1)
		{
			Temp.pop();
			if(Temp.size()==0) return i;
		}
	}
	return -1;
}

void Calculate_Cla::Opr(STACK<char>& symbol,STACK<double>& data,int& mark)
{
	double sum;
	while(symbol.size()!=0)
	{
		char tem=symbol.pop();
		int temp=setPri(tem);
		symbol.push(tem);
		if(temp<mark)
			break;
		else{
			sum=Call(data.pop(),data.pop(),symbol.pop());//��ջ��ȡ��ֵ���Ⱥ�˳��
			data.push(sum);
		}
	}
}

double Calculate_Cla::Calculate(char* buffer,double& sum)   //�ַ�������͸�����������
{
	STACK<double> data;
	STACK<char> symbol;
	double ans;
	char temp[MSG_LEN];
	int ct=0,mark=0,tp=0;
	data.push(sum);
	while(ct<=strlen(buffer))
	{
		if(IsData(buffer[ct]))            //��������ֻ�С����
		{
			while( ct < strlen(buffer) && IsData(buffer[ct]) )
				temp[tp++]=buffer[ct++];
			temp[tp]='\0';
			tp=0;                         //����������Ҳ��С��Ϊֹ
			ans=ToData(temp);             //�Ѷ������ַ���ת��Ϊ��
			data.push(ans);     

			if(ct==strlen(buffer))        //�Ѿ������ַ���ĩβ
			{
				mark=0;
				Opr(symbol,data,mark);    //����
				sum=data.pop();           //��ʱdataջ�л�ʣһ�����ݣ����ǽ��
				return sum;               //���ؽ��
			}
			else{
				int mark=setPri(buffer[ct]);
				Opr(symbol,data,mark);     //����
			}
		}
		else if(IsSym(buffer[ct]))         //����������
			symbol.push(buffer[ct++]);     //�������symbolջ
		else
		{
			char BF[100];int k=0;          //��������ǣ���ֻ��������
			while( IsPar( buffer[ct] ) != 1 && ct <= strlen(buffer) )
				BF[k++] = buffer[ct++];
			BF[k]='\0';     
			if(IsPar(buffer[ct])==1)       //һ�����������ţ�Ѱ����ƥ���������
			{
				int i,j;
				char Temp[100];
				for(i=ct+1,j=0;i<GetMatch(buffer,ct);i++,j++)
					Temp[j]=buffer[i];     //����������е��ַ�������Temp
				Temp[j]='\0';
				data.push(Calculate(Temp,sum)); //�ݹ����Calculateֱ��û������
				//Ȼ��ʼ���㣬ֵ��㷵��������ս������dataջ
				ct+=(strlen(Temp)+1);       //�����Ѿ���������ַ�
				if(ct+1==strlen(buffer))    //���￼���ַ��������Ž�β�����
				{
					mark=0;
					Opr(symbol,data,mark);
					sum=data.pop();
					return sum;
				}
				else
				{
					mark=setPri(buffer[ct+1]); //���ǵĻ���������
					Opr(symbol,data,mark);
				}
				ct++;                           //������һ���ַ�
			}
		}
	}
	return 0.;
}


  
CParaParser::CParaParser()
{
	

}

CParaParser::~CParaParser()
{

}

//��������
bool CParaParser::parseFormular(char * exp)
{
	_index = 0;
	_expr = exp;
	string stmp;
	SFieldValue tmpFieldValue;
	_vStack.clear();//modify
	
	while(_index < _expr.length())
	{
		
		Space(_index);
		
		if(isFieldNumber(_index,stmp))
		{
			tmpFieldValue.iFlag = NUMB;
			tmpFieldValue.sField = stmp;
			_vStack.push_back(tmpFieldValue);
		}
		else if(isVariable(_index,stmp))
		{
			tmpFieldValue.iFlag = VARI;
			tmpFieldValue.sField = stmp;
			_vStack.push_back(tmpFieldValue);
		}
		else if(isSymbol(_index,stmp))
		{
			tmpFieldValue.iFlag = SYMB;
			tmpFieldValue.sField = stmp;
			_vStack.push_back(tmpFieldValue);
		}
		/*if( isFieldNumber(_index,stmp) || isVariable(_index,stmp) || isSymbol(_index,stmp) )
		{
			_vStack.push_back(stmp);
			_index++;
		}*/
		else
		{
			sprintf(ErrorMsg, " input invalid expression !");
			
			throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
		}
		
	}
	
	return true;
}

//�ж��Ƿ�Ϊ����.
//pos�������ִ���βλ��
//svalue�������ֵ��ַ�����
bool CParaParser::isFieldNumber(int& pos,string& svalue)
{
	int iBeginPos = pos;
	Space(pos);

	//�״�ֻ�ж��Ƿ�Ϊ����
	if(!isdigit(_expr[pos])) return false;
	
	_target.clear();
	_target.append(1,_expr[pos]);
	pos++;
	
	while(isdigit(_expr[pos]) || '.' == _expr[pos])
	{
		_target.append(1,_expr[pos]);
		pos++;
	}
	svalue = _target;

	return true;
}

//�ж��Ƿ�Ϊ����
//pos���ر���βλ��
//svalue���ز���$�ı�����
bool CParaParser::isVariable(int & pos,string& svalue)
{
	
	//�жϱ����ĳ�ʼ�ַ��Ƿ�Ϊ$
	if('$' != _expr[pos]) return false;
	Space(pos);

	_target.clear();
	pos++;
	//�ж��Ƿ�����ĸ���ֻ����»���
	while( (isalnum(_expr[pos]) || _expr[pos]=='_') ) 
	{

		_target.append( 1,_expr[pos] );
        	pos++;

	}
	svalue = _target;

	return true;
}

//�ж��Ƿ�Ϊ��Ч�������
bool CParaParser::isSymbol(int& pos,string& svalue)
{
	Space(pos);

	if(  !(_expr[pos]=='(' || _expr[pos]==')' || _expr[pos]=='*'
			|| _expr[pos]=='+' || _expr[pos]=='-' || _expr[pos]=='/' || _expr[pos]=='.' )  )  //����Ƿ�Ϊ�����
	{
		return false;
	}

	svalue = es::str(_expr[pos]);
	pos++;
	return true;
}

void CParaParser::Space(int& pos)
{

	while (' ' == _expr[pos])
	{
		pos++;
	}
	
}

//���������ջ
vector<SFieldValue> CParaParser::getParseVec()
{
	return _vStack;
}

string CParaParser::getParseExpress(PacketParser& pps)
{
	string res;
	char cfield[FIELD_LEN];
	memset(cfield,0x00,FIELD_LEN);
	
	for(int i =0;i < _vStack.size();i++)
	{
		if(_vStack[i].iFlag == 1)//��������(����$�Ĳ�����)
		{
			if(-1 == pps.getField(_vStack[i].sField.c_str(), cfield))
			{
				sprintf(ErrorMsg, "get params  to ExpCal is Error");

				throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
			}
			res = res + cfield;
		}
		else
		{
			res = res + _vStack[i].sField;
		}
		
	}

	return res;
}


//�Զ����ʽת��
string CParaParser::transform(int n,int imethod,double dres)
{
	if(n < 0)
	{
		sprintf(ErrorMsg, "����С��λ����С��0.");

		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}

	ostringstream oss;
	oss<<setiosflags(ios::fixed) <<setprecision(n);
	
	double dpower = pow((double)10,(double)n);
	double transresult;
	
	switch(imethod)
	{
		
		//n+1λ�ض�
		case 1 :
			oss<<floor(dres*dpower)/dpower;
			break;
		//����С��λn��n+1ֵ����0��λ
		case 2 :
			oss<<ceil(dres*dpower)/dpower;
			break;
		//n+1λ��������
		case 3 :
			oss<<dres;
			break;
		default :
			sprintf(ErrorMsg, "��Ч��ת������id��");
			throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}

	string stmp = oss.str();
	return stmp;
	
}



  CExpCal::CExpCal()
  {
  
  }


  CExpCal::~CExpCal()
  {

  }

  void CExpCal::init(char *szSourceGroupID, char *szServiceID, int index)
  {
  
  	bHavParse = false;
	_needParseOperat = "";
	
	memset(_cbuf,0x00,FIELD_LEN),memset(_cfrontfield,0x00,FIELD_LEN);
	memset(_cpos,0x00,FIELD_LEN),memset(_cmethod,0x00,FIELD_LEN);
  }

  void CExpCal::execute(PacketParser& pps,ResParser& retValue)
  {
  	if(pps.getItem_num() != 1 && pps.getItem_num() != 3)
  	{
  		sprintf(ErrorMsg, "the params sent to ExpCal is Error");

		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
  	}

	_needParseOperat.clear();
	
	pps.getFieldValue(1,_cbuf);

	//�������һ�����ʽ���������½������ʽ
	if(strcmp(_cfrontfield,_cbuf) != 0)
	{
		strcpy(_cfrontfield,_cbuf);
		bHavParse = _parser.parseFormular(_cbuf);

		//_vParseField = _parser.getParseVec();
	}


	/*for(int i =0;i < _vParseField.size();i++)
	{
		if(_vParseField[i].iFlag == 1)//��������(����$�Ĳ�����)
		{
			//strcpy(_cfield,_vParseField[i].sField.c_str());
			if(-1 == pps.getField(_vParseField[i].sField.c_str(), _cfield))
			{
				sprintf(ErrorMsg, "get params  to ExpCal is Error");

				throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
			}
			_needParseOperat = _needParseOperat + _cfield;
		}
		else
		{
			_needParseOperat = _needParseOperat + _vParseField[i].sField;
		}
		
	}

	for(int i = 0;i < _vParseField.size();i++)
	{
		cout<<"_vParseField["<<i<<"].iFlag:"<<_vParseField[i].iFlag<<endl;
		cout<<"_vParseField["<<i<<"].sField:"<<_vParseField[i].sField<<endl;
	}*/

	_needParseOperat = _parser.getParseExpress(pps);

	double dsum = 0.0,dresult;
	dresult = _cl.Calculate(_needParseOperat.c_str(), dsum);

	//��������Ϊ3��ʼת��ʽ
	if(pps.getItem_num() == 3)
	{
		pps.getFieldValue(2,_cpos);//����С��λ
		pps.getFieldValue(3,_cmethod);//ת������

		int ipos       = atoi(_cpos);
		int imethod = atoi(_cmethod);

		_result = _parser.transform(ipos,imethod,dresult);
	}
	else//�������ʽ������ԭֵ���
	{
		_result = es::str(dresult);
	}

	retValue.setFieldValue(1,_result.c_str(),_result.length());
	
  }


void CExpCal::printMe()

{

	printf("\t2e<~C{3F:ExpCal,0f1>:E#:3.0.0 \n");

}

void CExpCal::message(MessageParser&  pMessage)
{
}

std::string CExpCal::getPluginName()

{

	return "ExpCal";

}



std::string CExpCal::getPluginVersion(){

	return "3.0.0";

}

  

  

