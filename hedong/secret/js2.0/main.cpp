#include<iostream>
#include "encrypt.h"

//using namespace std;
int main(int argc ,char* argv[])
{
        if(argc < 3)
        {
                cout<<"��ѡ����ܻ��߽��ܷ�ʽ��"<<endl;
                cout<<"����  sec + xxxx [xxxx2] [xxxx3] "<<endl;
                cout<<"����      sec - xxxx     [xxxx2] [xxxx3]" <<endl;
                return -1;
        }

        CEncryptAsc encrypt;  
        char out[100];
        memset(out,0,sizeof(out));
        if(strcmp(argv[1],"+") == 0)            //����
        {
                for(int i = 2;i<argc;i++)
                {
                         encrypt.Encrypt(argv[2],out);
                         cout<<"�ַ�����"<<argv[i]<<    "���ܺ�Ľ����"<<out<<endl;
                }
                
        }
        else if(strcmp(argv[1],"-") == 0)  //����
        {
                for(int i = 2;i<argc;i++)
                {
                        encrypt.Decrypt(argv[2],out);
                        cout<<"�ַ�����"<<argv[i]<<"    ���ܺ�Ľ����"<<out<<endl;
                }
        }
        else
        {
                cout<<"��ѡ����ܻ��߽��ܷ�ʽ��"<<endl;
                cout<<"����  sec + xxxx [xxxx2] [xxxx3] "<<endl;
                cout<<"����      sec - xxxx     [xxxx2] [xxxx3]" <<endl;
        }

        return 0;

}





