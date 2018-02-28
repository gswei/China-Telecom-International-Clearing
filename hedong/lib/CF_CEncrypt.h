#ifndef _ENCRYPT_H
#define _ENCRYPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <ctype.h>

#include "CF_CDesImpl.h"

#define MAX 1000
#define MARK  ('0'-1)

#define MAX_BLOCK_SIZE  4000
#define BUFFER_LEN      4096
#define SUCC    	0
#define FAIL     	1

#ifdef WIN32
	#ifdef YS_BUILD_DLL_DES
	    #define YS_DES_EXPORTS __declspec(dllexport)
	#else
	    #define YS_DES_EXPORTS __declspec(dllimport)
	#endif
#else
	#define YS_DES_EXPORTS
#endif


class YS_DES_EXPORTS CF_CDesEncrypt
{
public:

	/**
	 * @brief
	 *
	 * �����ַ���
	 *
	 * @param inStr		�������ַ���			
	 * @param inLen		�������ַ����ĳ���	
	 * @param outStr	���ܺ���ַ���	
	 * @param key		��Կ	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */
	static int encrypt(const char* inStr, unsigned int inLen, std::string& outStr, const char* key);

	/**
	 * @brief
	 *
	 * �����ַ���
	 *
	 * @param inStr		�������ַ���			
	 * @param outStr	���ܺ���ַ���	
	 * @param key		��Կ	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */
	static int decrypt(const char* inStr, std::string& outStr, const char* key);

	/**
	 * @brief
	 *
	 * �����ַ���
	 *
	 * @param inStr		�������ַ���			
	 * @param outStr	���ܺ���ַ���	
	 * @param outLen	��Ž��ܺ��ַ����Ļ�������С	
	 * @param key		��Կ	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */
	static int decrypt(const char* inStr, char* outStr, unsigned int outLen, const char* key);

	/**
	 * @brief
	 *
	 * �����ַ������˽ӿڵ���Կ���ܴ��ⲿ����
	 *
	 * @param inStr		�������ַ���			
	 * @param outStr	���ܺ���ַ���	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */
	static int encryptPwd(const char* inStr, std::string& outStr);

	/**
	 * @brief
	 *
	 * �����ַ������˽ӿڵ���Կ���ܴ��ⲿ���� 
	 *
	 * @param inStr		�������ַ���			
	 * @param outStr	���ܺ���ַ���	
	 * @param licence	��֤�Ƿ�Ϸ�����	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int decryptPwd(const char* inStr, std::string& outStr);

	/**
	 * @brief
	 *
	 * �ļ����� 
	 *
	 * @param inStr		�����ܵ��ļ�			
	 * @param outStr	���ܺ���ļ�	
	 * @param key		��Կ		
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int encryptFile(const char* inFile, const char* outFile, const char* key);

	/**
	 * @brief
	 *
	 * �ļ����� 
	 *
	 * @param inStr		�����ܵ��ļ�			
	 * @param key		��Կ		
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int encryptFile(const char* inFile, const char* key);

	/**
	 * @brief
	 *
	 * �ļ����� 
	 *
	 * @param inStr		�����ܵ��ļ�			
	 * @param outStr	���ܺ���ļ�	
	 * @param key		��Կ		
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int decryptFile(const char* inFile, const char* outFile, const char* key);

	/**
	 * @brief
	 *
	 * �ļ����� 
	 *
	 * @param inStr		�����ܵ��ļ�			
	 * @param key		��Կ		
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int decryptFile(const char* inFile, const char* key);

	/**
	 * @brief
	 *
	 * �����ַ������ļ� 
	 *
	 * @param inStr		�������ַ���			
	 * @param inLen		�������ַ����ĳ���	
	 * @param outStr	������ܺ��ַ������ļ���	
	 * @param key		��Կ	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int encryptBuffToFile(const char* inStr, unsigned int inLen, const char* outFile, const char* key);

	/**
	 * @brief
	 *
	 * ���ļ��н����ַ����������� 
	 *
	 * @param inFile	�����ܵ��ļ�			
	 * @param outStr	����������ַ���	
	 * @param outLen	��Ž�������Ļ���������	
	 * @param key		��Կ	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int decryptFileToBuff(const char* inFile, char* outStr, unsigned int outLen, const char* key);

	/**
	 * @brief
	 *
	 * ���ļ��н����ַ����������� 
	 *
	 * @param inFile	�����ܵ��ļ�			
	 * @param outStr	����������ַ���	
	 * @param key		��Կ	
	 *
	 * @return	���ܽ��
	 *
	 * @retval 0		���ܳɹ�
	 * @retval -1		����ʧ��
	 */	
	static int decryptFileToBuff(const char* inFile, std::string& outStr, const char* key);
};

/////////////////////////////////////////////////////////////
//class CEncryptAsc
/////////////////////////////////////////////////////////////
class CEncryptAsc {
private:
	static unsigned char m_KeyAsc[128];
public:
	CEncryptAsc() {}
	~CEncryptAsc() {}
	static char* getVersion();  
	  
	int Encrypt(char* strInput, char* strOutput);
	int Decrypt(char* strInput, char* strOutput); 
	
	int UltraEncrypt(char* strInput, char* strOutput);
	int UltraDecrypt(char* strInput, char* strOutput); 
	
};

/////////////////////////////////////////////////////////////
//class CEncryptDes
/////////////////////////////////////////////////////////////

class CEncryptFile
{
public:
  CEncryptFile();
  ~CEncryptFile();
   
  int Encrypt(char *inFileName, char *outFileName, char *Password);
  int unEncrypt(char *inFileName, char *outFileName, char *Password);         
private:
  FILE *infp, *outfp;    
};


#endif
