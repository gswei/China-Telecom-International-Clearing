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
	 * 加密字符串
	 *
	 * @param inStr		待加密字符串			
	 * @param inLen		待加密字符串的长度	
	 * @param outStr	加密后的字符串	
	 * @param key		密钥	
	 *
	 * @return	加密结果
	 *
	 * @retval 0		加密成功
	 * @retval -1		加密失败
	 */
	static int encrypt(const char* inStr, unsigned int inLen, std::string& outStr, const char* key);

	/**
	 * @brief
	 *
	 * 解密字符串
	 *
	 * @param inStr		待解密字符串			
	 * @param outStr	解密后的字符串	
	 * @param key		密钥	
	 *
	 * @return	解密结果
	 *
	 * @retval 0		解密成功
	 * @retval -1		解密失败
	 */
	static int decrypt(const char* inStr, std::string& outStr, const char* key);

	/**
	 * @brief
	 *
	 * 解密字符串
	 *
	 * @param inStr		待解密字符串			
	 * @param outStr	解密后的字符串	
	 * @param outLen	存放解密后字符串的缓冲区大小	
	 * @param key		密钥	
	 *
	 * @return	解密结果
	 *
	 * @retval 0		解密成功
	 * @retval -1		解密失败
	 */
	static int decrypt(const char* inStr, char* outStr, unsigned int outLen, const char* key);

	/**
	 * @brief
	 *
	 * 加密字符串，此接口的密钥不能从外部传入
	 *
	 * @param inStr		待加密字符串			
	 * @param outStr	加密后的字符串	
	 *
	 * @return	加密结果
	 *
	 * @retval 0		加密成功
	 * @retval -1		加密失败
	 */
	static int encryptPwd(const char* inStr, std::string& outStr);

	/**
	 * @brief
	 *
	 * 解密字符串，此接口的密钥不能从外部传入 
	 *
	 * @param inStr		待解密字符串			
	 * @param outStr	解密后的字符串	
	 * @param licence	验证是否合法调用	
	 *
	 * @return	解密结果
	 *
	 * @retval 0		解密成功
	 * @retval -1		解密失败
	 */	
	static int decryptPwd(const char* inStr, std::string& outStr);

	/**
	 * @brief
	 *
	 * 文件加密 
	 *
	 * @param inStr		待加密的文件			
	 * @param outStr	加密后的文件	
	 * @param key		密钥		
	 *
	 * @return	加密结果
	 *
	 * @retval 0		加密成功
	 * @retval -1		加密失败
	 */	
	static int encryptFile(const char* inFile, const char* outFile, const char* key);

	/**
	 * @brief
	 *
	 * 文件加密 
	 *
	 * @param inStr		待加密的文件			
	 * @param key		密钥		
	 *
	 * @return	加密结果
	 *
	 * @retval 0		加密成功
	 * @retval -1		加密失败
	 */	
	static int encryptFile(const char* inFile, const char* key);

	/**
	 * @brief
	 *
	 * 文件解密 
	 *
	 * @param inStr		待解密的文件			
	 * @param outStr	解密后的文件	
	 * @param key		密钥		
	 *
	 * @return	解密结果
	 *
	 * @retval 0		解密成功
	 * @retval -1		解密失败
	 */	
	static int decryptFile(const char* inFile, const char* outFile, const char* key);

	/**
	 * @brief
	 *
	 * 文件解密 
	 *
	 * @param inStr		待解密的文件			
	 * @param key		密钥		
	 *
	 * @return	解密结果
	 *
	 * @retval 0		解密成功
	 * @retval -1		解密失败
	 */	
	static int decryptFile(const char* inFile, const char* key);

	/**
	 * @brief
	 *
	 * 加密字符串至文件 
	 *
	 * @param inStr		待加密字符串			
	 * @param inLen		待加密字符串的长度	
	 * @param outStr	保存加密后字符串的文件名	
	 * @param key		密钥	
	 *
	 * @return	加密结果
	 *
	 * @retval 0		加密成功
	 * @retval -1		加密失败
	 */	
	static int encryptBuffToFile(const char* inStr, unsigned int inLen, const char* outFile, const char* key);

	/**
	 * @brief
	 *
	 * 从文件中解密字符串至缓冲区 
	 *
	 * @param inFile	待解密的文件			
	 * @param outStr	解密输出的字符串	
	 * @param outLen	存放解密输出的缓冲区长度	
	 * @param key		密钥	
	 *
	 * @return	解密结果
	 *
	 * @retval 0		解密成功
	 * @retval -1		解密失败
	 */	
	static int decryptFileToBuff(const char* inFile, char* outStr, unsigned int outLen, const char* key);

	/**
	 * @brief
	 *
	 * 从文件中解密字符串至缓冲区 
	 *
	 * @param inFile	待解密的文件			
	 * @param outStr	解密输出的字符串	
	 * @param key		密钥	
	 *
	 * @return	解密结果
	 *
	 * @retval 0		解密成功
	 * @retval -1		解密失败
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
