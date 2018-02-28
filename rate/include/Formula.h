#ifndef FORMULA_H_

#define FORMULA_H_

#include <string>

#include <map>
#include <math.h>

using namespace std;

class Formula {
public:
	string* GetExpr() {
		return &expr_;
	}

	map<string, double>* GetVars() {
		return &vars_;
	}

	bool Calc(double& sum);
	double Round(double d, int iPrecision);
   
        void Clearmap(){
           vars_.clear();
           return;
        }
  Formula();
  ~Formula();
private:
	bool IsVar(int& index, double& number);

	bool IsNumber(int& index, double& number);

	bool IsFactor(int& index, char sign, double factor, double& product);

	bool IsExpr(int& index, char sign, double addend, double& addtion);

	double Sign(char sign, double operand, double operanded);

	void Space(int& index);

	static bool IsZero(double number);

	char GetPrefix(int& index);
	

private:
	string expr_;
	bool err_;
	map<string, double> vars_;
};


#endif
