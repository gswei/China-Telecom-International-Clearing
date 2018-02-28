/***********************************************************************
*FILTERTM_CCrc.cpp
*����ȥ�عؼ��ֵ�ѹ����ÿ10���ֽ�ѹ����һ��unsigned short
*created by tanj 20050516
***********************************************************************/
#include "CF_CHash.h"

CF_CHash::CF_CHash(int iHashStep)
{
	m_iHashStep = iHashStep;
}

#ifdef HASH_CRC
/***************************************************************
int crc16(char *string ,int len)
DESCRIPTION  : �������봮��crcֵ��������ѹ�����������봮��Ҫ̫��(<=20)
INPUT   : string    :���봮����Ϊascii��16���ƣ�
	  len	    :������
OUTPUT  : None
RETURN  : 2���ֽڵ�crc16�Ľ��ֵ
****************************************************************/
unsigned short CF_CHash::crc16(char *string ,int len)
{
	unsigned short crc =0;
	int i ,j;
	
	for (i =0 ;i <len ;i++) {
		crc ^=( (int)string[i] <<8); //���ֽڲ��� �����ֽ����
		for (j =0 ;j <8 ;j++) 
			if ( (crc &0x8000) !=0)
				crc = (crc <<1) ^ 0x1021;
			else
				crc <<=1;
	}
	return crc &0xffff;
}
#endif 


#ifdef HASH_SHA1
#define SHA1HANDSOFF
/* Hash a single 512-bit block. This is the core of the algorithm. */
void CF_CHash::SHA1Transform(unsigned long state[5], unsigned char buffer[64])
{
    unsigned long a, b, c, d, e;
    typedef union 
    {
        unsigned char c[64];
        unsigned long l[16];
    } CHAR64LONG16;
    CHAR64LONG16* block;
#ifdef SHA1HANDSOFF
static unsigned char workspace[64];
    block = (CHAR64LONG16*)workspace;
    memcpy(block, buffer, 64);
#else
    block = (CHAR64LONG16*)buffer;
#endif
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
}


/* SHA1Init - Initialize new context */

void CF_CHash::SHA1Init(SHA1_CTX* context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void CF_CHash::SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len)
{
unsigned int i, j;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
    context->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
        SHA1Transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void CF_CHash::SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
unsigned long i, j;
unsigned char finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    SHA1Update(context, (unsigned char *)"\200", 1);
    while ((context->count[0] & 504) != 448) {
        SHA1Update(context, (unsigned char *)"\0", 1);
    }
    SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
         ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
    /* Wipe variables */
    i = j = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(&finalcount, 0, 8);
#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
    SHA1Transform(context->state, context->buffer);
#endif
}

#endif 





int CF_CHash::getHashLen(int iSrcLen)
{
#ifdef HASH_CRC
	return ((iSrcLen + m_iHashStep - 1)/m_iHashStep)*sizeof(unsigned short);
#endif 

#ifdef HASH_SHA1
  return 20;
#endif 

#ifdef HASH_MD5
  return 16;
#endif 

#ifdef HASH_RIPEMD5
	return 16;
#endif 
}

void CF_CHash::getHashStr(unsigned char *szDesStr, char *szSrcStr, int iSrcLen)
{
#ifdef HASH_CRC
	for (int i = 0; i < getHashLen(iSrcLen); i++)
	{
		unsigned short crc_code;
		crc_code = crc16(szSrcStr + i*m_iCrcStep, m_iCrcStep);
		memcpy(szDesStr + i*sizeof(unsigned short), &crc_code, sizeof(unsigned short));
	}
	return;
#endif 

#ifdef HASH_SHA1
  SHA1_CTX context;
  SHA1Init(&context);
  SHA1Update(&context, (unsigned char *)szSrcStr, iSrcLen);
  SHA1Final((unsigned char *)szDesStr, &context);
  return;
#endif 

#ifdef HASH_MD5
  MDString(szSrcStr,szDesStr,iSrcLen);
  return;
#endif

#ifdef HASH_RIPEMD5
  md.Hash(szDesStr,(unsigned char *)szSrcStr, iSrcLen);
#endif

}




