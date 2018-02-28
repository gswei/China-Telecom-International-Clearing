#include<iostream>
#include "cipher.h"
using namespace std;

//*******************************************************************
int main(int argc, char ** argv)
{
	Cipher cip;
	char key[CIPHER_CONST_LENGTH+1], code[CIPHER_CONST_LENGTH+1], *p;


	//加密:
	if(argc > 2)
	{
		if(strcmp(argv[1],"+") == 0)
		{
				strcpy(key, argv[2]);
				p = cip.GetCipher(key);
				if( p==NULL )
				{
					cout<<"加密失败\n"<<endl;
					return -1;
				}
				else
				{
					cout<<"字符串["<<argv[2]<<"]的加密结果:"<<p<<endl;
					return 0;
				}
		}
		else if(strcmp(argv[1],"-") == 0)
		{
				p = cip.GetPlainText(argv[2]);
				if( p==NULL )
				{
					cout<<"解密失败\n"<<endl;
					return -1;
				}
				else
				{
					cout<<"字符串["<<argv[2]<<"]的解密结果:"<<p<<endl;
					return 0;
				}
		}
		
	}

	cout<<"输入格式不对:"<<endl;
	cout<<"\n格式:"<<endl;
	cout<<"加密: cipher + password"<<endl;
	cout<<"解密: cipher - password"<<endl<<endl;

	return 0;
}

