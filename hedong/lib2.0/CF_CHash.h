/***********************************************************************
*CF_CCrc.h
*用于去重关键字的压缩，每10个字节压缩成一个unsigned short
*created by tanj 20050516
***********************************************************************/
#ifndef _CF_CHASH_H_
#define _CF_CHASH_H_
#include <string.h>

//#define HASH_CRC
//#define HASH_SHA1
//#define HASH_MD5
#define HASH_RIPEMD5

#ifdef HASH_MD5
#include "mddriver.h"
#endif

#ifdef HASH_RIPEMD5
#include "ripemd.h"
#endif


#ifdef HASH_SHA1

#define LITTLE_ENDIAN

typedef struct {
    unsigned long state[5];
    unsigned long count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(unsigned long state[5], unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef LITTLE_ENDIAN        //LITTLE_ENDIAN means 低位在先、高位在后 noted by tanj 2005.05.16
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

#endif 







class CF_CHash
{
private:
  int m_iHashStep;
#ifdef HASH_CRC
  unsigned short crc16(char *string ,int len);
#endif 

#ifdef HASH_RIPEMD5
  RipeMD128 md;
#endif

#ifdef HASH_SHA1
void SHA1Transform(unsigned long state[5], unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);
#endif 



public:
  CF_CHash(int iHashStep = 10);
  int getHashLen(int iSrcLen);
  void getHashStr(unsigned char *szDesStr,char *szSrcStr, int iSrcLen);
  
  
};






#endif 
