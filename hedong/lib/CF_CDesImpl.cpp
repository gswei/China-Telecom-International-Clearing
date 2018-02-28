#include "CF_CDesImpl.h"


/**
 *  FileName  : CF_CMD5Encoder.cpp
 *  Author    : --
 *  Date      : 2010-05-05
 *  Revision  :
 *  CopyRight : Copyright (C) YIXUN
 */

//#include "CF_CMD5Encoder.h"

/* Constants for MD5Transform routine.
 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init (MD5_CTX * context)
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void MD5Update (MD5_CTX *context, const unsigned char *input, unsigned int inputLen)
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
 MD5Transform (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (context->state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  MD5_memcpy
 ((POINTER)&context->buffer[index], (POINTER)&input[i],
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final (unsigned char digest[16],MD5_CTX *context)
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update (context, bits, 8);
  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform (UINT4 state[4], const unsigned char block[64])
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode (unsigned char *output, UINT4 *input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode (UINT4 *output, const unsigned char *input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */

static void MD5_memcpy (POINTER output, POINTER input, unsigned int len )
{
  unsigned int i;

  for (i = 0; i < len; i++)
 output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset (POINTER output, int value, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
 ((char *)output)[i] = (char)value;

}

/* Digests a string and prints the result.
 */
void md5_calc(unsigned char *output, const unsigned  char  *input, unsigned int len) 
{

  MD5_CTX context;

  MD5Init (&context);
  MD5Update (&context, input, len);
  MD5Final (output, &context);

}


void CF_CMD5Encoder::encodeUC16(const char* in, unsigned long inLen, unsigned char out[16]) 
{
    md5_calc (out, (const unsigned char*)in, inLen);
}

std::string CF_CMD5Encoder::encodeUC16(const char* in, unsigned long inLen)
{
	unsigned char tmp[32];
	memset(tmp, 0, sizeof(tmp));

	CF_CMD5Encoder::encodeUC16(in, inLen, tmp);

	char encode[64];
	memset(encode, 0, sizeof(encode));

	sprintf(encode,
			"%02X%02X%02X%02X"
		    "%02X%02X%02X%02X"
		    "%02X%02X%02X%02X"
		    "%02X%02X%02X%02X",
		    tmp[0],  tmp[1],  tmp[2],  tmp[3],
		    tmp[4],  tmp[5],  tmp[6],  tmp[7],
		    tmp[8],  tmp[9],  tmp[10], tmp[11],
		    tmp[12], tmp[13], tmp[14], tmp[15]);

	return std::string(encode);
}

void CF_CMD5Encoder::encodeUL4(const char* in, unsigned long inLen, unsigned long out[4]) 
{
	unsigned char tmp[16];
	md5_calc (tmp, (const unsigned char*)in, inLen);

	out[0] = tmp[3]  + (((unsigned long)tmp[2]) << 8)
			 + (((unsigned long)tmp[1]) << 16)
			 + (((unsigned long)tmp[0]) << 24);

	out[1] = tmp[7]  + (((unsigned long)tmp[6]) << 8)
			 + (((unsigned long)tmp[5]) << 16)
			 + (((unsigned long)tmp[4]) << 24);

	out[2] = tmp[11]  + (((unsigned long)tmp[10]) << 8)
			 + (((unsigned long)tmp[9]) << 16)
			 + (((unsigned long)tmp[8]) << 24);

	out[3] = tmp[15]  + (((unsigned long)tmp[14]) << 8)
			 + (((unsigned long)tmp[13]) << 16)
			 + (((unsigned long)tmp[12]) << 24);
}

//end of CF_CMD5Encoder.cpp

/**
 *  FileName  : CF_CDesImpl.cpp
 *  Author    : xiehp
 *  Date      : 2010-05-05
 *  Revision  :
 *  CopyRight : Copyright (C) YIXUN
 */

#include <stdio.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>


// initial permutation IP
const char CF_CDesImpl::IP_Table[64] = {
	58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
};

// final permutation IP^-1 
const char CF_CDesImpl::IPR_Table[64] = {
	40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25
};

// expansion operation matrix
const char CF_CDesImpl::E_Table[48] = {
	32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
};

// 32-bit permutation function P used on the output of the S-boxes 
const char CF_CDesImpl::P_Table[32] = {
	16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
	2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25
};

// permuted choice table (key) 
const char CF_CDesImpl::PC1_Table[56] = {
	57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
};

// permuted choice key (table) 
const char CF_CDesImpl::PC2_Table[48] = {
	14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};

// number left rotations of pc1 
const char CF_CDesImpl::LOOP_Table[16] = {
	1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

// The (in)famous S-boxes 
const char CF_CDesImpl::S_Box[8][4][16] = {
	// S1 
	14,	 4,	13,	 1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

	// S2 
    15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

	// S3 
    10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
     1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,

	// S4 
     7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
     3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,

	// S5 
     2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

	// S6 
    12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
     4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,

	// S7 
     4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

	// S8 
    13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
     2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

unsigned char CF_CDesImpl::Char2Hex(unsigned char ch)
{
	if(ch > 9)
	{
		ch = ch - 10 + 'A';
	}
	else
	{
		ch |= 0x30;
	}

	return ch;
}

unsigned char CF_CDesImpl::Hex2Char(unsigned char ch)
{
    if(!isxdigit(ch))
	{
        return 16;
    }

    if(ch < 'A')
	{
        ch -= 0x30;
    }
	else
	{
        ch = ch + 10 - 'A';
    }

    return ch;
}

void CF_CDesImpl::SetKey(const char* Key, int len)
{
	memset(deskey, 0, 16);
	memcpy(deskey, Key, len > 16 ? 16 : len);

	SetSubKey(&SubKey[0], &deskey[0]);
	Is3DES = len > 8 ? (SetSubKey(&SubKey[1], &deskey[8]), true) : false;
}

void CF_CDesImpl::SDES(char* out, const char* in, const PSubKey pSubKey, bool bEncrypt)
{
	//static bool M[64], tmp[32], *Li=&M[0], *Ri=&M[32];
	bool tmp[32], *Li=&m_M[0], *Ri=&m_M[32];

	ByteToBit(m_M, in, 64);
	Transform(m_M, m_M, IP_Table, 64);

	if(bEncrypt)
	{
		for(int i=0; i<16; ++i) 
		{
			memcpy(tmp, Ri, 32);
			F_func(Ri, (*pSubKey)[i]);
			Xor(Ri, Li, 32);
			memcpy(Li, tmp, 32);
		}
	}
	else
	{
		for(int i = 15; i >= 0; --i) 
		{
			memcpy(tmp, Li, 32);
			F_func(Li, (*pSubKey)[i]);
			Xor(Li, Ri, 32);
			memcpy(Ri, tmp, 32);
		}
	}

	Transform(m_M, m_M, IPR_Table, 64);
	BitToByte(out, m_M, 64);
}

void CF_CDesImpl::SetSubKey(PSubKey pSubKey, const char Key[8])
{
	//static bool K[64], *KL=&K[0], *KR=&K[28];
	bool *KL = &m_K[0];
	bool *KR = &m_K[28];

	ByteToBit(m_K, Key, 64);
	Transform(m_K, m_K, PC1_Table, 56);

	for(int i=0; i<16; ++i) 
	{
		RotateL(KL, 28, LOOP_Table[i]);
		RotateL(KR, 28, LOOP_Table[i]);
		Transform((*pSubKey)[i], m_K, PC2_Table, 48);
	}
}

void CF_CDesImpl::F_func(bool In[32], const bool Ki[48])
{
	//static bool MR[48];

	Transform(m_MR, In, E_Table, 48);
	Xor(m_MR, Ki, 48);
	S_func(In, m_MR);
	Transform(In, In, P_Table, 32);
}

void CF_CDesImpl::S_func(bool Out[32], const bool In[48])
{
	for(char i=0,j,k; i<8; ++i,In+=6,Out+=4) 
	{
		j = (In[0] << 1) + In[5];
		k = (In[1] << 3) + (In[2] << 2) + (In[3] << 1) + In[4];
		ByteToBit(Out, &S_Box[i][j][k], 4);
	}
}

void CF_CDesImpl::Transform(bool *Out, bool *In, const char *Table, int len)
{
	for(int i=0; i < len; ++i)
	{
		Tmp[i] = In[Table[i]-1];
	}

	memcpy(Out, Tmp, len);
}

void CF_CDesImpl::Xor(bool *InA, const bool *InB, int len)
{
	for(int i=0; i < len; ++i)
	{
		InA[i] ^= InB[i];
	}
}

void CF_CDesImpl::RotateL(bool *In, int len, int loop)
{
	memcpy(Tmp, In, loop);
	memcpy(In, In + loop, len - loop);
	memcpy(In + len - loop, Tmp, loop);
}

void CF_CDesImpl::ByteToBit(bool *Out, const char *In, int bits)
{
	for(int i=0; i < bits; ++i)
	{
		Out[i] = (In[i>>3]>>(i&7)) & 1;
	}
}

void CF_CDesImpl::BitToByte(char *Out, const bool *In, int bits)
{
	memset(Out, 0, bits>>3);

	for(int i=0; i < bits; ++i)
	{
		Out[i>>3] |= In[i]<<(i&7);
	}
}

int CF_CDesImpl::genKey(const char* key, std::string& desKey, std::string& desEncryKey)
{
	char tmpKey[32];
	memset(tmpKey, 0, sizeof(tmpKey));

	srand((unsigned int)time(0));
	sprintf(tmpKey, "@.#%d$*&", rand());

	if(0 == strcmp("", key))
	{
		SetKey("Des@gd.yixun.com", 15);
	}
	else
	{
		SetKey(key, strlen(key));
	}

	// 根据MD5算法生成密钥
	desKey = CF_CMD5Encoder::encodeUC16(tmpKey, 16);
	desKey = desKey.substr(0, 16);

	desKey[rand() % 16] = 3;
	desKey[rand() % 16] = 7;
	desKey[rand() % 16] = 10;

	// 将密钥加密,并将密钥加密后的密文存放在前16个字符
	encrypt(desKey.c_str(), 16, desEncryKey);

	return 0;
}

int CF_CDesImpl::decryptKey(const char* key, const char* desEncryKey, std::string& desKey)
{
	if(0 == strcmp("", key))
	{
		SetKey("Des@gzlingxun.com", 15);
	}
	else
	{
		SetKey(key, strlen(key));
	}

	// 解密密钥
	decrypt(desEncryKey, desKey);

	return 0;
}

int CF_CDesImpl::encrypt(const char* inStr, char* outStr)
{
	char tmpOut[9];
	tmpOut[8] = 0;

	if(!Is3DES) 
	{  
		SDES(tmpOut, inStr, &SubKey[0], true);
	} 
	else
	{   
		// 3次DES 
		// 加密:加(key0)-解(key1)-加(key0) 
		// 解密::解(key0)-加(key1)-解(key0)

		SDES(tmpOut, inStr,  &SubKey[0], true);
		SDES(tmpOut, tmpOut, &SubKey[1], false);
		SDES(tmpOut, tmpOut, &SubKey[0], true);
	}

	for(int i = 0; i < 8; i++)
	{
		*outStr = Char2Hex(((tmpOut[i] + 36) >> 4) & 0xF);
		outStr++;
		*outStr = Char2Hex((tmpOut[i] + 36) & 0xF);
		outStr++;
	}

	return 0;
}

int CF_CDesImpl::encrypt(const char* inStr, unsigned int inLen, std::string& outStr)
{
	char tmpOut[17];
	memset(tmpOut, 0, sizeof(tmpOut));

	int pos = 0;

	do
	{
		if(inLen >= 8)
		{
			encrypt(inStr + pos, tmpOut);
			inLen = inLen - 8;
			pos = pos + 8;
		}
		else
		{
			char tmp[9];
			memset(tmp, 0, sizeof(tmp));
			memcpy(tmp, inStr + pos, inLen);

			encrypt(tmp, tmpOut);
			inLen = 0;
		}

		outStr = outStr + tmpOut;

	}while(inLen > 0);

	return 0;
}

int CF_CDesImpl::encrypt(const char* inStr, unsigned int inLen, char* outStr, unsigned int outLen)
{
	int pos = 0;

	do
	{
		if(inLen >= 8)
		{
			encrypt(inStr + pos, outStr);
			inLen = inLen - 8;
			pos = pos + 8;
			outStr = outStr + 16;
		}
		else
		{
			char tmp[9];
			memset(tmp, 0, sizeof(tmp));
			memcpy(tmp, inStr + pos, inLen);

			encrypt(tmp, outStr);
			inLen = 0;
		}

	}while(inLen > 0);

	return 0;
}

int CF_CDesImpl::decrypt(const char* inStr, char* outStr)
{
	char tmpIn[9];
	tmpIn[8] = 0;

	for(int i = 0; i < 8; i++)
	{
        tmpIn[i] = (((Hex2Char(inStr[2*i]) << 4) & 0xf0) | Hex2Char(inStr[2*i+1])) - 36;
	}

	if(!Is3DES) 
	{  
		SDES(outStr, tmpIn, &SubKey[0], false);
	} 
	else
	{   
		// 3次DES 
		// 加密:加(key0)-解(key1)-加(key0) 
		// 解密::解(key0)-加(key1)-解(key0)

		SDES(outStr, tmpIn,  &SubKey[0], false);
		SDES(outStr, outStr, &SubKey[1], true);
		SDES(outStr, outStr, &SubKey[0], false);
	}

	return 0;
}

int CF_CDesImpl::decrypt(const char* inStr, std::string& outStr)
{
	char tmpOut[9];
	memset(tmpOut, 0, sizeof(tmpOut));

	unsigned int pos = 0;
	unsigned int inLen = strlen(inStr);

	do
	{
		if(inLen >= 16)
		{
			decrypt(inStr + pos, tmpOut);
			inLen = inLen - 16;
			pos = pos + 16;
		}
		else
		{
			char tmp[17];
			memset(tmp, 0, sizeof(tmp));
			memcpy(tmp, inStr + pos, inLen);

			decrypt(inStr + pos, tmpOut);
			inLen = 0;
		}

		outStr = outStr + tmpOut;

	}while(inLen > 0);

	return 0;
}

int CF_CDesImpl::decrypt(const char* inStr, char* outStr, unsigned int outLen)
{
	unsigned int pos = 0;
	unsigned int inLen = strlen(inStr);

	do
	{
		if(inLen >= 16)
		{
			decrypt(inStr + pos, outStr);
			inLen = inLen - 16;
			pos = pos + 16;
			outStr = outStr + 8;
		}
		else
		{
			char tmpIn[17];
			memset(tmpIn, 0, sizeof(tmpIn));
			memcpy(tmpIn, inStr + pos, inLen);

			decrypt(tmpIn, outStr);

			inLen = 0;
		}
	}while(inLen > 0);

	return 0;
}

char * CF_CDesImpl:: getVersion()
{
	return("3.0.0");
}


