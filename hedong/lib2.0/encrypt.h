#ifndef _ENCRYPT_H
#define _ENCRYPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>

/////////////////////////////////////////////////////////////
//class CEncryptAsc
/////////////////////////////////////////////////////////////
class CEncryptAsc {
private:
	static unsigned char m_KeyAsc[128];
public:
	CEncryptAsc() {}
	~CEncryptAsc() {}
	
	int Encrypt(char* strInput, char* strOutput);
	int Decrypt(char* strInput, char* strOutput); 
	
	int UltraEncrypt(char* strInput, char* strOutput);
	int UltraDecrypt(char* strInput, char* strOutput); 
	
};

/////////////////////////////////////////////////////////////
//class CEncryptDes
/////////////////////////////////////////////////////////////

#endif
