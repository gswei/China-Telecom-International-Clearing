
  #include "CExpCal.h"


bool Calculate_Cla::IsData(char ch)      //判断输入计算的数字是否为0-9
{
	return ((ch>='0'&&ch<='9')||ch=='.')?true:false;
}
  
bool Calculate_Cla::IsSym(char ch)      //判断是否输入非法运算符
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

int Calculate_Cla::setPri(char ch)          //符号的优先极别
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

double Calculate_Cla::ToData(char* ch)   //将数字转化为数值
{
	int i,j,sumn=0;
	double sum=0.0;
	if(!Check(ch)) return 0.0;
	for(i=0;i<strlen(ch);i++)             //读入整数部分
	{
		if(ch[i]!='.')
			sumn=sumn*10+(ch[i]-'0');
		else break;
	}
	if(i<strlen(ch))
		for(j=i+1;j<strlen(ch);j++)        //小数部分
			sum=sum*10+(ch[j]-'0');
	sum /= pow(10.0,(double)(strlen(ch)-1-i));
	return (sum+sumn);                      //返回值
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

int Calculate_Cla::GetMatch(char* buffer,int pos)     //利用栈找到匹配的括号
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
			sum=Call(data.pop(),data.pop(),symbol.pop());//从栈中取数值的先后顺序
			data.push(sum);
		}
	}
}

double Calculate_Cla::Calculate(char* buffer,double& sum)   //字符串读入和各个函数调配
{
	STACK<double> data;
	STACK<char> symbol;
	double ans;
	char temp[MSG_LEN];
	int ct=0,mark=0,tp=0;
	data.push(sum);
	while(ct<=strlen(buffer))
	{
		if(IsData(buffer[ct]))            //如果是数字或小数点
		{
			while( ct < strlen(buffer) && IsData(buffer[ct]) )
				temp[tp++]=buffer[ct++];
			temp[tp]='\0';
			tp=0;                         //读到非数字也非小数为止
			ans=ToData(temp);             //把读到的字符串转化为数
			data.push(ans);     

			if(ct==strlen(buffer))        //已经独到字符串末尾
			{
				mark=0;
				Opr(symbol,data,mark);    //计算
				sum=data.pop();           //此时data栈中还剩一个数据，即是结果
				return sum;               //返回结果
			}
			else{
				int mark=setPri(buffer[ct]);
				Opr(symbol,data,mark);     //计算
			}
		}
		else if(IsSym(buffer[ct]))         //如果是运算符
			symbol.push(buffer[ct++]);     //运算符入symbol栈
		else
		{
			char BF[100];int k=0;          //如果都不是，则只能是括号
			while( IsPar( buffer[ct] ) != 1 && ct <= strlen(buffer) )
				BF[k++] = buffer[ct++];
			BF[k]='\0';     
			if(IsPar(buffer[ct])==1)       //一旦读到左括号，寻找它匹配的右括号
			{
				int i,j;
				char Temp[100];
				for(i=ct+1,j=0;i<GetMatch(buffer,ct);i++,j++)
					Temp[j]=buffer[i];     //把这对括号中的字符串存入Temp
				Temp[j]='\0';
				data.push(Calculate(Temp,sum)); //递归调用Calculate直到没有括号
				//然后开始计算，值层层返回最后将最终结果放入data栈
				ct+=(strlen(Temp)+1);       //跳过已经处理完的字符
				if(ct+1==strlen(buffer))    //这里考虑字符串以括号结尾的情况
				{
					mark=0;
					Opr(symbol,data,mark);
					sum=data.pop();
					return sum;
				}
				else
				{
					mark=setPri(buffer[ct+1]); //不是的话继续计算
					Opr(symbol,data,mark);
				}
				ct++;                           //读入下一个字符
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

//参数解析
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

//判断是否为数字.
//pos返回数字串的尾位置
//svalue返回数字的字符串型
bool CParaParser::isFieldNumber(int& pos,string& svalue)
{
	int iBeginPos = pos;
	Space(pos);

	//首次只判断是否为数字
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

//判断是否为变量
//pos返回变量尾位置
//svalue返回不带$的变量名
bool CParaParser::isVariable(int & pos,string& svalue)
{
	
	//判断变量的初始字符是否为$
	if('$' != _expr[pos]) return false;
	Space(pos);

	_target.clear();
	pos++;
	//判断是否是字母数字或者下划线
	while( (isalnum(_expr[pos]) || _expr[pos]=='_') ) 
	{

		_target.append( 1,_expr[pos] );
        	pos++;

	}
	svalue = _target;

	return true;
}

//判断是否为有效的运算符
bool CParaParser::isSymbol(int& pos,string& svalue)
{
	Space(pos);

	if(  !(_expr[pos]=='(' || _expr[pos]==')' || _expr[pos]=='*'
			|| _expr[pos]=='+' || _expr[pos]=='-' || _expr[pos]=='/' || _expr[pos]=='.' )  )  //检测是否为运算符
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

//返回运算符栈
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
		if(_vStack[i].iFlag == 1)//参数类型(不带$的参数名)
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


//自定义格式转换
string CParaParser::transform(int n,int imethod,double dres)
{
	if(n < 0)
	{
		sprintf(ErrorMsg, "保留小数位不能小于0.");

		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}

	ostringstream oss;
	oss<<setiosflags(ios::fixed) <<setprecision(n);
	
	double dpower = pow((double)10,(double)n);
	double transresult;
	
	switch(imethod)
	{
		
		//n+1位截断
		case 1 :
			oss<<floor(dres*dpower)/dpower;
			break;
		//保留小数位n，n+1值大于0进位
		case 2 :
			oss<<ceil(dres*dpower)/dpower;
			break;
		//n+1位四舍五入
		case 3 :
			oss<<dres;
			break;
		default :
			sprintf(ErrorMsg, "无效的转换类型id。");
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

	//如果与上一个表达式不等则重新解析表达式
	if(strcmp(_cfrontfield,_cbuf) != 0)
	{
		strcpy(_cfrontfield,_cbuf);
		bHavParse = _parser.parseFormular(_cbuf);

		//_vParseField = _parser.getParseVec();
	}


	/*for(int i =0;i < _vParseField.size();i++)
	{
		if(_vParseField[i].iFlag == 1)//参数类型(不带$的参数名)
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

	//参数个数为3则开始转格式
	if(pps.getItem_num() == 3)
	{
		pps.getFieldValue(2,_cpos);//保留小数位
		pps.getFieldValue(3,_cmethod);//转换类型

		int ipos       = atoi(_cpos);
		int imethod = atoi(_cmethod);

		_result = _parser.transform(ipos,imethod,dresult);
	}
	else//不输入格式参数则按原值输出
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

  

  

