#include "Formula.h"

#include <cfloat>
#include <stdio.h>
#include <iostream.h>
const double INF = DBL_MIN;
const double PREC = 0;

Formula::Formula()
{
  // err_ = false;
}

Formula::~Formula()
{
 // err_ = false;
}

bool Formula::Calc(double& sum) {
	int index = 0;
	char sign = '+';
	sum = 0;
	err_ = false;
	//cout <<index<<sign<<sum<<endl;
	if (IsExpr(index, sign, 0, sum) || '\0' == expr_[index]) {
		//cout << "Calc true " <<endl;
		return true;
	}
	return false;
}

bool Formula::IsVar(int& index, double& number) {
	bool res = false;
	string var = "$";
	map<string, double>::iterator iter = vars_.end();

	if (err_ || '$' != expr_[index]) goto RES;


	++ index;
	while(isalpha(expr_[index]) || expr_[index]=='_') {
		var.append(1, expr_[index]);
        ++ index;
	}

	if (1 == var.size()) goto ERR;
	iter = vars_.find(var);
	if (vars_.end() == iter) goto ERR;

	number = iter->second;
	
	res = true;
	goto RES;

ERR:
	err_ = true;
RES:
	return res && (!err_);
}


bool Formula::IsNumber(int& index, double& number) {
	bool res = false;

	if (err_ || !isdigit(expr_[index])) goto ERR;

	number = 0;
	while(isdigit(expr_[index])) {
		number = number * 10 + (expr_[index] - '0');
		++ index;
	}

	if ('.' == expr_[index]) {
		++ index;
		double decimal = 0.1;
		while (isdigit(expr_[index])) {
			number += decimal * (expr_[index] - '0');
			decimal *= 0.1;
			++ index;
		}
	}

	res = true;
	goto RES;

ERR:
	err_ = true;
RES:
	return res && (!err_);
}

bool Formula::IsFactor(int& index, char sign, double factor, double& product) {
	bool res = false;
	if (err_) goto RES;
	
	product = factor;
	do {
		Space(index);
		if ('(' == expr_[index]) {
			++ index;
			if (IsExpr(index, '+', 0, factor)) {
				product = Sign(sign, product, factor);
			} else {
				goto ERR;
			}
			if (')' != expr_[index]) {
				goto ERR;
			}
			++ index;
		} else if (IsVar(index, factor) || IsNumber(index, factor)) {
			product = Sign(sign, product, factor);
		} else {
			goto ERR;
		}
		Space(index);
		sign = ' ';
		if ('*' == expr_[index] || '/' == expr_[index]) {
			sign = expr_[index ++];
		}
	} while (('*' == sign || '/' == sign) && !err_);

	res = true;
	goto RES;

ERR:
	err_ = true;
RES:
	return res && (!err_);
}

bool Formula::IsExpr(int& index, char sign, double addend, double& addtion) {
	bool res = false;
	
	addtion = addend;

	sign = GetPrefix(index);

	while (!err_) {
		if ('(' == expr_[index]) {
			++ index;
			if (!IsExpr(index, '+', 0, addend)) {
				goto ERR;
			}
			if (')' != expr_[index]) {
				goto ERR;
			}
			++ index;
		} else if (false == IsVar(index, addend) && false == IsNumber(index, addend)) {
			goto ERR;
		}

		Space(index);
		if ('*' == expr_[index] || '/' == expr_[index]) {
			char tmp_sign = expr_[index];
			++ index;
			if (!IsFactor(index, tmp_sign, addend, addend)) {
				goto ERR;
			}
		}

		if ('+' == expr_[index] || '-' == expr_[index]) {
			addtion = Sign(sign, addtion, addend);
			sign = expr_[index ++];
			Space(index);
		} else if (')' == expr_[index] || '\0' == expr_[index]) {
			addtion = Sign(sign, addtion, addend);
			break;
		}
	}

	res = true;
	goto RES;
ERR:
	err_ = true;
RES:
	return res && (!err_);
}

double Formula::Sign(char sign, double operand, double operanded) {
    double sum = 0;
    switch (sign) {
        case '+':
            sum = operand + operanded;
            break;
        case '-':
            sum = operand - operanded;
            break;
        case '*':
        	  sum = operand * operanded;
            break;
        case '/':
            sum = (IsZero(operanded) ? (err_ = true, INF) : (operand / operanded));
            break;  
    }
    /*
    printf("in Formula sum = %f ,%f,%c  \n",operand,operanded,sign);
     printf("in Formula sum = %f   \n",sum);
   
     double f1 = 88000.000000;
     double f2 = 614520.000000;
      printf("in Formula sum double = %f   \n",f1*f2);
      
    long l1 =  88000;
    long l2 =  614520;
      printf("in Formula sum long = %ld   \n",l1*l2);  
      
      double d1 =  88000.000000;
    double d2 =  614520.000000;
      printf("in Formula sum double = %f  \n",d1*d2); 
      */
    return sum;	
}

void Formula::Space(int& index) {
	while (' ' == expr_[index]) {
		++ index;
	}
}

bool Formula::IsZero(double number) {
	return number  < PREC && number > -PREC;  
}

char Formula::GetPrefix(int& index) {
	char sign = '+';
	Space(index);
	if ('-' == expr_[index]) {
		sign = '-';
		++ index;
	}
	Space(index);
	return sign;
}

double Formula::Round(double d, int iPrecision) {
        double dResult;
        double dTmp = pow((double) 10, iPrecision);

        double integerPart; //整数部分
        double decimalPart;//小数部分
        double tmp = 0.500001;
        double tmp2 = -0.500001;
        //double tmp2;
       // double tmp3;
        //int tmp4;

        decimalPart = modf(d, &integerPart);

       // cout << "decimalPart = " << decimalPart <<endl;
      //  tmp2 = decimalPart * dTmp;
      //  tmp3 = tmp2+tmp;
        
       // cout << "tmp2 = " << tmp2 <<endl;
       // cout << "tmp3 = " << tmp3 <<endl;
       // tmp4 = floor( tmp3 );
       // cout << "tmp4 = " << tmp4 <<endl;
       if(integerPart>=0)
        	decimalPart = floor(decimalPart * dTmp + tmp) / dTmp;
       else
       	  decimalPart = ceil(decimalPart * dTmp + tmp2) / dTmp;
        //decimalPart = tmp4 / dTmp;
        
       // cout << "floor = " << tmp3 <<endl;
       // cout << "integerPart = " << integerPart <<endl;
       // cout << "decimalPart = " << decimalPart <<endl;
      //  cout << "dTmp = " << dTmp <<endl;

       // cout << " floor(5) = " << floor(5.0) <<endl;
       // cout << " floor(4) = " << floor(4.0) <<endl;
       // cout << "int(1.6) = " << int(1.6) <<endl;
       // cout << " int(1.3)  = " << int(1.3) <<endl;
        
        dResult = integerPart + decimalPart;

        return (dResult);
}
