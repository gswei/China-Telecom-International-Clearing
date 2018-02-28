#include<iostream>
#include "cipher.h"
using namespace std;

//*******************************************************************
int main(int argc, char ** argv)
{
	Cipher cip;
	char key[CIPHER_CONST_LENGTH+1], code[CIPHER_CONST_LENGTH+1], *p;


	//����:
	if(argc > 2)
	{
		if(strcmp(argv[1],"+") == 0)
		{
				strcpy(key, argv[2]);
				p = cip.GetCipher(key);
				if( p==NULL )
				{
					cout<<"����ʧ��\n"<<endl;
					return -1;
				}
				else
				{
					cout<<"�ַ���["<<argv[2]<<"]�ļ��ܽ��:"<<p<<endl;
					return 0;
				}
		}
		else if(strcmp(argv[1],"-") == 0)
		{
				p = cip.GetPlainText(argv[2]);
				if( p==NULL )
				{
					cout<<"����ʧ��\n"<<endl;
					return -1;
				}
				else
				{
					cout<<"�ַ���["<<argv[2]<<"]�Ľ��ܽ��:"<<p<<endl;
					return 0;
				}
		}
		
	}

	cout<<"�����ʽ����:"<<endl;
	cout<<"\n��ʽ:"<<endl;
	cout<<"����: cipher + password"<<endl;
	cout<<"����: cipher - password"<<endl<<endl;

	return 0;
}

