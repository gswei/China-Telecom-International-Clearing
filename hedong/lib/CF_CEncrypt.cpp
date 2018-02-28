#include "CF_CEncrypt.h"

int CF_CDesEncrypt::encryptFile(const char* inFile, const char* outFile, const char* key)
{
	FILE* fpIn = NULL;
	FILE* fpOut = NULL;

	do
	{
		const int READ_LEN = 1*1024;
		const int WRITE_LEN = 2*1024;

		char readBuf[READ_LEN + 1];
		char writeBuf[WRITE_LEN + 1];

		fpIn = fopen(inFile, "r+b");
		if(NULL == fpIn)
		{
			break;
		}

		fpOut = fopen(outFile, "w+b");
		if(NULL == fpOut)
		{
			break;
		}

		std::string desKey, desEncryptKey;

		CF_CDesImpl desImpl;
		if(desImpl.genKey(key, desKey, desEncryptKey) < 0)
		{
			break;
		}

		int writeLen = fwrite(desEncryptKey.c_str(), sizeof(char), desEncryptKey.length(), fpOut);
		if(writeLen != desEncryptKey.length())
		{
			break;
		}

		desImpl.SetKey(desKey.c_str(), desKey.length());

		while(!feof(fpIn))
		{
			int readLen = fread(readBuf, sizeof(char), READ_LEN, fpIn);
			if(READ_LEN != readLen)
			{
				if(ferror(fpIn))
				{
					fclose(fpIn);
					fclose(fpOut);

					return -1;
				}
			}

			if(desImpl.encrypt(readBuf, readLen, writeBuf, WRITE_LEN) < 0)
			{
				fclose(fpIn);
				fclose(fpOut);

				return -1;
			}

			int outBufLen = 0;
			if(0 == (readLen % 8))
			{
				outBufLen = readLen * 2;
			}
			else
			{
				outBufLen = readLen / 8 + 1;
				outBufLen = outBufLen * 16;
			}

			writeLen = fwrite(writeBuf, sizeof(char), outBufLen, fpOut);
			if(outBufLen != writeLen)
			{
				fclose(fpIn);
				fclose(fpOut);

				return -1;
			}
		}

		fclose(fpIn);
		fclose(fpOut);

		return 0;

	}while(0);


	fclose(fpIn);
	fclose(fpOut);

	return -1;
}

int CF_CDesEncrypt::encryptFile(const char* inFile, const char* key)
{
	char outFile[1024];
	memset(outFile, 0, sizeof(outFile));

	sprintf(outFile, "%s.tmp", inFile);

	if(encryptFile(inFile, outFile, key) < 0)
	{
		return -1;
	}

	remove(inFile);
	rename(outFile, inFile);

	return 0;
}

int CF_CDesEncrypt::decryptFile(const char* inFile, const char* outFile, const char* key)
{
	FILE* fpIn = NULL;
	FILE* fpOut = NULL;

	do
	{
		const int READ_LEN = 2*1024;
		const int WRITE_LEN = 1*1024;

		char readBuf[READ_LEN + 1];
		char writeBuf[WRITE_LEN + 1];

		fpIn = fopen(inFile, "r+b");
		if(NULL == fpIn)
		{
			break;
		}

		int readLen = fread(readBuf, sizeof(char), 32, fpIn);
		if(32 != readLen)
		{
			break;
		}

		readBuf[32] = 0;

		std::string desKey;

		CF_CDesImpl desImpl;
		if(desImpl.decryptKey(key, readBuf, desKey) < 0)
		{
			break;
		}

		fpOut = fopen(outFile, "w+b");
		if(NULL == fpOut)
		{
			break;
		}

		desImpl.SetKey(desKey.c_str(), desKey.length());

		while(!feof(fpIn))
		{
			int readLen = fread(readBuf, sizeof(char), READ_LEN, fpIn);
			readBuf[readLen] = 0;

			if(desImpl.decrypt(readBuf, writeBuf, WRITE_LEN) < 0)
			{
				fclose(fpIn);
				fclose(fpOut);

				return -1;
			}

			int writeLen = fwrite(writeBuf, sizeof(char), readLen / 2, fpOut);
			if(writeLen != readLen / 2)
			{
				fclose(fpIn);
				fclose(fpOut);

				return -1;
			}
		}

		fclose(fpIn);
		fclose(fpOut);

		return 0;

	}while(0);

	fclose(fpIn);
	fclose(fpOut);

	return -1;
}

int CF_CDesEncrypt::decryptFile(const char* inFile, const char* key)
{
	char outFile[1024];
	memset(outFile, 0, sizeof(outFile));

	sprintf(outFile, "%s.tmp", inFile);

	if(decryptFile(inFile, outFile, key) < 0)
	{
		return -1;
	}

	remove(inFile);
	rename(outFile, inFile);

	return 0;
}

int CF_CDesEncrypt::encryptPwd(const char* inStr, std::string& outStr)
{
	return encrypt(inStr, strlen(inStr), outStr, "{229C-408C#AABF}");
}

int CF_CDesEncrypt::decryptPwd(const char* inStr, std::string& outStr)
{
/*	if(0 != strcmp(licence, "{5512D7BE-A09B-46A5-A37B-3E11567CAD20}"))
	{
		return -1;
	}
*/
	return decrypt(inStr, outStr, "{229C-408C#AABF}");
}

int CF_CDesEncrypt::encrypt(const char* inStr, unsigned int inLen, std::string& outStr, const char* key)
{
	std::string desKey;

	CF_CDesImpl desImpl;
	desImpl.genKey(key, desKey, outStr);

	if(0 == inLen)
	{
		return 0;
	}

	desImpl.SetKey(desKey.c_str(), desKey.length());
	desImpl.encrypt(inStr, inLen, outStr);

	return 0;
}

int CF_CDesEncrypt::decrypt(const char* inStr, std::string& outStr, const char* key)
{
	int len = strlen(inStr);

	if(len < 32)
	{
		return -1;
	}

	std::string desKey;

	CF_CDesImpl desImpl;
	desImpl.decryptKey(key, inStr, desKey);

	if(32 == len)
	{
		outStr = "";
		return 0;
	}

	desImpl.SetKey(desKey.c_str(), desKey.length());
	desImpl.decrypt(inStr + 32, outStr);

	return 0;
}

int CF_CDesEncrypt::decrypt(const char* inStr, char* outStr, unsigned int outLen, const char* key)
{
	unsigned int inLen = strlen(inStr);

	if(inLen < 32)
	{
		return -1;
	}

	// 避免解密输出越界
	if(outLen < inLen / 2)
	{
		return -1;
	}

	std::string desKey;

	CF_CDesImpl desImpl;
	desImpl.decryptKey(key, inStr, desKey);

	if(32 == inLen)
	{
		outStr = "";
		return 0;
	}

	desImpl.SetKey(desKey.c_str(), desKey.length());
	desImpl.decrypt(inStr + 32, outStr, outLen);

	return 0;
}

int CF_CDesEncrypt::encryptBuffToFile(const char* inStr, unsigned int inLen, const char* outFile, const char* key)
{
	std::string outStr;
	if(encrypt(inStr, inLen, outStr, key) < 0)
	{
		return -1;
	}

	FILE* fp = fopen(outFile, "w+b");
	if(NULL == fp)
	{
		return -1;
	}

	int writeLen = fwrite(outStr.c_str(), sizeof(char), outStr.length(), fp);
	if(outStr.length() != writeLen)
	{
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int CF_CDesEncrypt::decryptFileToBuff(const char* inFile, char* outStr, unsigned int outLen, const char* key)
{
	FILE* fpIn = NULL;

	const int READ_LEN = 2*1024;
	char readBuf[READ_LEN + 1];

	fpIn = fopen(inFile, "r+b");
	if(NULL == fpIn)
	{
		return -1;
	}

	int readLen = fread(readBuf, sizeof(char), 32, fpIn);
	if(32 != readLen)
	{
		fclose(fpIn);
		return -1;
	}

	readBuf[32] = 0;

	std::string desKey;

	CF_CDesImpl desImpl;
	if(desImpl.decryptKey(key, readBuf, desKey) < 0)
	{
		fclose(fpIn);
		return -1;
	}

	desImpl.SetKey(desKey.c_str(), desKey.length());

	while(!feof(fpIn))
	{
		readLen = fread(readBuf, sizeof(char), READ_LEN, fpIn);
		if(readLen != READ_LEN)
		{
			if(ferror(fpIn))
			{
				fclose(fpIn);
				return -1;
			}
		}

		readBuf[readLen] = 0;

		if(outLen < readLen * 2)
		{
			fclose(fpIn);
			return -1;
		}

		if(desImpl.decrypt(readBuf, outStr, outLen) < 0)
		{
			fclose(fpIn);
			return -1;
		}

		outStr = outStr + readLen / 2;
		outLen = outLen - readLen / 2;
	}

	fclose(fpIn);
	return 0;
}

int CF_CDesEncrypt::decryptFileToBuff(const char* inFile, std::string& outStr, const char* key)
{
	FILE* fpIn = NULL;

	const int READ_LEN = 2*1024;
	const int WRITE_LEN = 1024;

	char readBuf[READ_LEN + 1];

	fpIn = fopen(inFile, "r+b");
	if(NULL == fpIn)
	{
		return -1;
	}

	int readLen = fread(readBuf, sizeof(char), 32, fpIn);
	if(32 != readLen)
	{
		fclose(fpIn);
		return -1;
	}

	readBuf[32] = 0;

	std::string desKey;

	CF_CDesImpl desImpl;
	if(desImpl.decryptKey(key, readBuf, desKey) < 0)
	{
		fclose(fpIn);
		return -1;
	}

	desImpl.SetKey(desKey.c_str(), desKey.length());

	while(!feof(fpIn))
	{
		readLen = fread(readBuf, sizeof(char), READ_LEN, fpIn);
		if(readLen != READ_LEN)
		{
			if(ferror(fpIn))
			{
				fclose(fpIn);
				return -1;
			}
		}

		readBuf[readLen] = 0;

		if(desImpl.decrypt(readBuf, outStr) < 0)
		{
			fclose(fpIn);
			return -1;
		}	
	}

	fclose(fpIn);
	return 0;
}


////////////////////////////////////////////////////////////////////////////
//class CEncryptAsc
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////
//static key
////////////////////////////////////////
unsigned char CEncryptAsc::m_KeyAsc[128] = {
	0x48, 0xEE, 0x76, 0x1D, 0x67, 0x69, 0xA1, 0x1B, 
	0x7A, 0x86, 0x47, 0xF8, 0x54, 0x95, 0x97, 0x5F, 
 	0x78, 0xD9, 0xDA, 0x6C, 0x59, 0xD7, 0x6B, 0x35, 
 	0xC5, 0x77, 0x85, 0x18, 0x2A, 0x0E, 0x52, 0xFF, 
	0x00, 0xE3, 0x1B, 0x71, 0x8D, 0x34, 0x63, 0xEB, 
  	0x91, 0xC3, 0x24, 0x0F, 0xB7, 0xC2, 0xF8, 0xE3, 
  	0xB6, 0x54, 0x4C, 0x35, 0x54, 0xE7, 0xC9, 0x49, 
  	0x28, 0xA3, 0x85, 0x11, 0x0B, 0x2C, 0x68, 0xFB, 
  	0xEE, 0x7D, 0xF6, 0x6C, 0xE3, 0x9C, 0x2D, 0xE4, 
  	0x72, 0xC3, 0xBB, 0x85, 0x1A, 0x12, 0x3C, 0x32, 
  	0xE3, 0x6B, 0x4F, 0x4D, 0xF4, 0xA9, 0x24, 0xC8, 
  	0xFA, 0x78, 0xAD, 0x23, 0xA1, 0xE4, 0x6D, 0x9A, 
  	0x04, 0xCE, 0x2B, 0xC5, 0xB6, 0xC5, 0xEF, 0x93, 
  	0x5C, 0xA8, 0x85, 0x2B, 0x41, 0x37, 0x72, 0xFA, 
  	0x57, 0x45, 0x41, 0xA1, 0x20, 0x4F, 0x80, 0xB3, 
  	0xD5, 0x23, 0x02, 0x64, 0x3F, 0x6C, 0xF1, 0x0F };

/********************************************************************
函数说明： 加密函数
输入： sInput -- 待加密的密码串, 长度最大不能超过128
	  sOutput -- 密码串加密后的ascii字符串输出缓冲区, 应保证大于257个字节
返回： 0--成功 !0--失败
********************************************************************/

char * CEncryptAsc:: getVersion()
{
	return("3.0.0");
}


int CEncryptAsc::Encrypt(char* sInput, char* sOutput)
{
	int iBuf[256], i;
	char sTemp[256] = "";
       char szMindOutput[256]="";
	sOutput[0] = 0x00;
	
	int iLen = strlen(sInput);
	//if( iLen == 0 || iLen > 128 )
	if (iLen > 128)
		return -1; //长度错误
	
	for (i = 0; i < iLen; i++)
		iBuf[i] = (m_KeyAsc[i]^sInput[i]);
	
	for (i=0; i < iLen; i++)
	{
		sprintf(sTemp, "%02x", iBuf[i]);
		strcat(szMindOutput, sTemp);
	}			
	std::string szDesOutput;
	szDesOutput="";
	CF_CDesEncrypt::encryptPwd(szMindOutput,szDesOutput);
	//printf("szDesOutput=%s",szDesOutput.c_str());
	//长度应该小于256
	sprintf(sOutput,"%s",szDesOutput.c_str());
	
	return 0;
}

/******************************************************************
函数说明： 解密函数
输入： sInput -- 待解密的密码串, 长度最大不能超过256
	  sOutput -- 密码串解密后的ascii字符串输出缓冲区, 应保证大于129个字节
返回： 0--成功 !0--失败
*******************************************************************/
int CEncryptAsc::Decrypt(char *sInput, char *sOutput)
{
	 char szMindOutput[256]="";
	 std::string szDesOutput="";
	CF_CDesEncrypt::decryptPwd(sInput,szDesOutput);
	sprintf(szMindOutput,"%s",szDesOutput.c_str());

       //printf("szDesOutput=%s",szDesOutput.c_str());
	
	char sBuf[256] = "";
	char sTemp[256] = "";
	int i = 0, j = 0;
	int nTemp;

	int iLen = strlen(szMindOutput);
	//if( iLen == 0 || iLen > 256 ) 
	if (iLen > 500)
		return -1; //长度错误	

	while (szMindOutput[i]!=0x00)
	{
		memcpy(sTemp, szMindOutput+i, 2);
		sTemp[2] = 0x00;
		sscanf(sTemp, "%x",&nTemp);
		sBuf[j] = nTemp;
		sOutput[j] = (sBuf[j]^m_KeyAsc[j]);
		j++;
		i +=2;
	}

	sOutput[j] = 0x00;

	return 0;	
}

CEncryptFile::CEncryptFile()
{
   
}

CEncryptFile::~CEncryptFile()
{

}

/*****************************************************************************
description: 
  Encrypt the input file. 
input: 
  inFileName - the input filename
  outFileName - the output filename  
output:
  
return: 
  SUCC/FAIL
*****************************************************************************/ 

int CEncryptFile::Encrypt(char *inFileName, char *outFileName, char *Password)
{
  char szMsg[BUFFER_LEN] = "";
  char szPwd[BUFFER_LEN] = "";
  char szBuffer[BUFFER_LEN] = "";
  char szTempBuffer[BUFFER_LEN] = "";
  char szRslBuffer[BUFFER_LEN] = "";  
  char szTemp[BUFFER_LEN] = "";

  int  iLenPwd;
  int  iLenByte;
  int  i;
  int  j;
  int  k;
  int  iTemp;
    
  cout<<"Encrypt from "<<inFileName<<" to "<<outFileName<<endl;

  if (NULL == (infp = fopen(inFileName, "rb" ))) 
  {
    sprintf(szMsg, "Cant open %s", inFileName);
    perror(szMsg);
    return FAIL;
   }
  
  if (NULL == (outfp = fopen(outFileName, "wb")))
  {
    sprintf(szMsg, "Cant open %s", outFileName);
    perror(szMsg);
    return FAIL;
  }

  if (Password[strlen(Password)-1] == '\n')
    Password[strlen(Password)-1] = 0;
  iLenPwd = strlen(Password);
  memcpy(szPwd, Password, iLenPwd);
  
  for ( ; ; )
  {
    memset(szBuffer,0,BUFFER_LEN);
    memset(szTempBuffer,0,BUFFER_LEN);
    memset(szRslBuffer,0,BUFFER_LEN);
    memset(szTemp,0,BUFFER_LEN);

    j = -1;
    i = 0;

    iLenByte = fread(szBuffer, sizeof(char), MAX_BLOCK_SIZE, infp);
    if (iLenByte <= 0)
      break;
   
    for (i=0; i<iLenByte; i++)
    {
      if (j == iLenPwd-1) j=-1;
      j++;
      szRslBuffer[i] = szBuffer[i]^szPwd[j];
    }

    fwrite(szRslBuffer, sizeof(char), iLenByte, outfp);
  }

  fclose(infp);
  fclose(outfp);
  return SUCC;
}
	
/*****************************************************************************
description: 
  Encrypt the input file. 
input: 
  inFileName - the input filename
  outFileName - the output filename  
output:
  
return: 
  SUCC/FAIL
*****************************************************************************/  

int CEncryptFile::unEncrypt(char *inFileName, char *outFileName, char *Password)
{
  char szMsg[BUFFER_LEN] = "";
  char szPwd[BUFFER_LEN] = "";
  char szBuffer[BUFFER_LEN] = "";
  char szTempBuffer[BUFFER_LEN] = "";
  char szTemp[BUFFER_LEN] = "";
  char szRslBuffer[BUFFER_LEN] = "";
  int  iLenPwd;
  int  iLenByte;
  int  iLenRsl;
  int i;
  int j;
  int k;
  int iTemp;    
    
  cout<<"unEncrypt from "<<inFileName<<" to "<<outFileName<<endl;

  if (NULL == (infp = fopen(inFileName, "rb" ))) 
  {
    sprintf(szMsg, "Cant open %s", inFileName);
    perror(szMsg);
    return FAIL;
   }
  
  if (NULL == (outfp = fopen(outFileName, "wb")))
  {
    sprintf(szMsg, "Cant open %s", outFileName);
    perror(szMsg);
    return FAIL;
  }

  if (Password[strlen(Password)-1] == '\n')
    Password[strlen(Password)-1] = 0;
  iLenPwd = strlen(Password);
  memcpy(szPwd, Password, iLenPwd);
  
  for ( ; ; )
  {
    memset(szBuffer,0,BUFFER_LEN);
    memset(szTempBuffer,0,BUFFER_LEN);
    memset(szRslBuffer,0,BUFFER_LEN);

    i = 0;
    j = -1;
    k = 0;
    
    iLenByte = fread(szBuffer, sizeof(char), MAX_BLOCK_SIZE, infp);
    if (iLenByte <= 0)
      break;

    for (i=0; i<iLenByte; i++)
    {
      if (j == iLenPwd-1) j=-1;
      j++;
      szRslBuffer[i] = szPwd[j]^szBuffer[i];
    }
    
    fwrite(szRslBuffer, sizeof(char), iLenByte, outfp);
  }

  fclose(infp);
  fclose(outfp);
  return SUCC;	
}

