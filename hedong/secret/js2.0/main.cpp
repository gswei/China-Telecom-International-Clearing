#include<iostream>
#include "encrypt.h"

//using namespace std;
int main(int argc ,char* argv[])
{
        if(argc < 3)
        {
                cout<<"请选择加密或者解密方式："<<endl;
                cout<<"加密  sec + xxxx [xxxx2] [xxxx3] "<<endl;
                cout<<"解密      sec - xxxx     [xxxx2] [xxxx3]" <<endl;
                return -1;
        }

        CEncryptAsc encrypt;  
        char out[100];
        memset(out,0,sizeof(out));
        if(strcmp(argv[1],"+") == 0)            //加密
        {
                for(int i = 2;i<argc;i++)
                {
                         encrypt.Encrypt(argv[2],out);
                         cout<<"字符串："<<argv[i]<<    "加密后的结果："<<out<<endl;
                }
                
        }
        else if(strcmp(argv[1],"-") == 0)  //解密
        {
                for(int i = 2;i<argc;i++)
                {
                        encrypt.Decrypt(argv[2],out);
                        cout<<"字符串："<<argv[i]<<"    解密后的结果："<<out<<endl;
                }
        }
        else
        {
                cout<<"请选择加密或者解密方式："<<endl;
                cout<<"加密  sec + xxxx [xxxx2] [xxxx3] "<<endl;
                cout<<"解密      sec - xxxx     [xxxx2] [xxxx3]" <<endl;
        }

        return 0;

}





