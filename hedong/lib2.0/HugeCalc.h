// HugeCalc.h: 
//
//////////////////////////////////////////////////////////////////////

#if !defined(_HUGECALC_H_)
#define _HUGECALC_H_

#include <stdio.h>
//#include <conio.h>
#include <ctype.h>
#include <string.h>

#define MAX 1000
#define MARK  ('0'-1)

//The Prototypes of the functions
char* Initialiaze(char [],int);
int  Read(char [],char [],char * );
int  Print(char [],char [],char [],char []);
int  Calculate(char[],char[],char[],char []);
char* Addition(char[],char[]);
char* Substraction(char[],char[]);
char* Multiplication(char[],char[]);
char* Division(char[],char[]);
char Div_per_bit(char [],int , char []);
int Sub_per_bit(char [],char [],int *);
int Copy(char [],int ,char []);
int Compare(char [],char []);
int  Data_Process(char []);
int  Value(char);
int  Check(char []);
int  Compare(char [],char []);
int Judge(int);
int Convert(char [],char [],int);

#endif // !defined(_HUGECALC_H_)
