/**
 *  FileName  : CF_CMD5Encoder.h
 *  Author    : xiehp
 *  Date      : 2010-05-05
 *  Revision  :
 *  CopyRight : Copyright (C) yixun
 */

#ifndef __YS_DES_IPML_H__
#define __YS_DES_IPML_H__

#include <string>

//#include "CF_CMD5Encoder.h"

#ifdef WIN32
	#ifdef LX_BUILD_DLL_MD5ENCODER
	    #define LX_MD5ENCODER_EXPORTS __declspec(dllexport)
	#else
	    #define LX_MD5ENCODER_EXPORTS __declspec(dllimport)
	#endif
#else
	#define LX_MD5ENCODER_EXPORTS
#endif

//#include <string>//重复,xiehp

#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif


/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void MD5Init PROTO_LIST ((MD5_CTX *));
static void MD5_memcpy (POINTER output, POINTER input, unsigned int len );

static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));
static void MD5Transform (UINT4 state[4],const unsigned char block[64]);
static void MD5_memset (POINTER output, int value, unsigned int len);
static void Encode(unsigned char *output, UINT4 *input, unsigned int len);
static void Decode (UINT4 *output, const unsigned char *input, unsigned int len);
void md5_calc(unsigned char *output, const unsigned  char  *input, unsigned int len);


class LX_MD5ENCODER_EXPORTS CF_CMD5Encoder
{
public:
    static void encodeUL4(const char* in, unsigned long inLen, unsigned long out[4]); 
    static void encodeUC16(const char* in, unsigned long inLen, unsigned char out[16]); 
	static std::string encodeUC16(const char* in, unsigned long inLen);
};


//end of CF_CMD5Encoder.h

class CF_CDesImpl
{
	typedef bool (*PSubKey)[16][48];

public:
	static char* getVersion();  

	/**
	* 设置密钥
	*/
	void SetKey(const char* Key, int len);

	/**
	 * 根据用户传入的密钥生成真正的加密密钥
	 */
	int genKey(const char* key, std::string& desKey, std::string& desEncryKey);
	int decryptKey(const char* key, const char* desEncryKey, std::string& desKey);

	int encrypt(const char* inStr, unsigned int inLen, std::string& outStr);
	int encrypt(const char* inStr, unsigned int inLen, char* outStr, unsigned int outLen);
	int decrypt(const char* inStr, std::string& outStr);
	int decrypt(const char* inStr, char* outStr, unsigned int outLen);

protected:

	unsigned char Char2Hex(unsigned char ch);
	unsigned char Hex2Char(unsigned char ch);

	int encrypt(const char* inStr, char* outStr);
	int decrypt(const char* inStr, char* outStr);

	/**
	 * 标准DES加/解密
	 */
	void SDES(char* out, const char* in, const PSubKey pSubKey, bool bEncrypt);

	/**
	 * 设置子密钥
	 */
	void SetSubKey(PSubKey pSubKey, const char Key[8]);

	/**
	 * F函数
	 */
	void F_func(bool In[32], const bool Ki[48]);

	/**
	 * S盒代替
	 */
	void S_func(bool Out[32], const bool In[48]);

	/**
	 * 变换
	 */
	void Transform(bool *Out, bool *In, const char *Table, int len);

	/**
	 * 异或
	 */
	void Xor(bool *InA, const bool *InB, int len);

	/**
	 * 循环左移
	 */
	void RotateL(bool *In, int len, int loop);

	/**
	 * 字节组转换成位组
	 */
	void ByteToBit(bool *Out, const char *In, int bits);

	/**
	 * 位组转换成字节组
	 */
	void BitToByte(char *Out, const bool *In, int bits);

private:

	/**
	 * 16圈子密钥
	 */
	bool SubKey[2][16][48];

	/**
	 * 3次DES标志
	 */
	bool Is3DES;

	char Tmp[256];

	char deskey[16];

	bool m_MR[48];
	bool m_K[64];
	bool m_M[64];

	static const char IP_Table[64];
	static const char IPR_Table[64];
	static const char E_Table[48];
	static const char P_Table[32];
	static const char PC1_Table[56];
	static const char PC2_Table[48];
	static const char LOOP_Table[16];
	static const char S_Box[8][4][16];
};

#endif // __LX_DES_IPML_H__
