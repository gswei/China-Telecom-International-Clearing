/****************************************************************
  Project
  Copyright (c)	2000-2003. All Rights Reserved.

  SUBSYSTEM:
  FILE:			Cipher.cpp
  AUTHOR:		Jack Lee
  Create Time:  9/24/2000
==================================================================
  Description:

  UpdateRecord:
*****************************************************************/
#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <utime.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#define	O_BINARY	0
#define	SVMSG_MODE	(MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
#define SVSHM_MODE (SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6)
#else
#include <windows.h>
#include <io.h>
#include <conio.h>
#include <direct.h>
#include <sys/utime.h>

#define F_OK	0x00
#define W_OK	0x02
#define R_OK	0x04
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include "cipher.h"

static const UINT2 bytebit[8] = {
    0200, 0100, 040, 020, 010, 04, 02, 01
};

static const UINT4 bigbyte[24] = {
    0x800000L, 0x400000L, 0x200000L, 0x100000L,
    0x80000L,  0x40000L,  0x20000L,  0x10000L,
    0x8000L,   0x4000L,   0x2000L,   0x1000L,
    0x800L,    0x400L,    0x200L,    0x100L,
    0x80L,     0x40L,     0x20L,     0x10L,
    0x8L,      0x4L,      0x2L,      0x1L
};

static const unsigned char totrot[16] = {
    1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28
};

static const unsigned char pc1[56] = {
    56, 48, 40, 32, 24, 16,  8,      0, 57, 49, 41, 33, 25, 17,
    9,  1, 58, 50, 42, 34, 26,     18, 10,  2, 59, 51, 43, 35,
    62, 54, 46, 38, 30, 22, 14,      6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,     20, 12,  4, 27, 19, 11,  3
};

static const unsigned char bytKey[24] = {
    1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28, 99, 45, 23, 81, 102, 77, 39, 51
};

static const unsigned char pc2[48] = {
    13, 16, 10, 23,  0,  4,  2, 27, 14,  5, 20,  9,
    22, 18, 11,  3, 25,  7, 15,  6, 26, 19, 12,  1,
    40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
    43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31
};

static const unsigned char bytIv[8] = {
    30, 36, 46, 54, 29, 39, 50, 44
};

#ifndef DES386

const UINT4 Spbox[8][64] = {
    0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
    0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
    0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
    0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
    0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
    0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
    0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
    0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
    0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
    0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
    0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
    0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
    0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
    0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
    0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
    0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L,
    0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
    0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
    0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
    0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
    0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
    0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
    0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
    0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
    0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
    0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
    0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
    0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
    0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
    0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
    0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
    0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L,
    0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
    0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
    0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
    0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
    0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
    0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
    0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
    0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
    0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
    0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
    0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
    0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
    0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
    0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
    0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
    0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L,
    0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
    0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
    0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
    0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
    0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
    0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
    0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
    0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
    0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
    0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
    0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
    0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
    0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
    0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
    0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
    0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L,
    0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
    0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
    0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
    0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
    0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
    0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
    0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
    0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
    0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
    0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
    0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
    0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
    0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
    0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
    0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
    0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L,
    0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
    0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
    0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
    0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
    0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
    0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
    0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
    0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
    0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
    0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
    0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
    0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
    0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
    0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
    0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
    0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L,
    0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
    0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
    0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
    0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
    0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
    0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
    0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
    0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
    0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
    0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
    0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
    0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
    0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
    0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
    0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
    0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L,
    0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
    0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
    0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
    0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
    0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
    0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
    0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
    0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
    0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
    0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
    0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
    0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
    0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
    0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
    0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
    0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L
};

#else
// S box tables for assembler desfunc

const unsigned int Spbox[8][64] = {
    0x04041000, 0x00000000, 0x00040000, 0x04041010,
    0x04040010, 0x00041010, 0x00000010, 0x00040000,
    0x00001000, 0x04041000, 0x04041010, 0x00001000,
    0x04001010, 0x04040010, 0x04000000, 0x00000010,
    0x00001010, 0x04001000, 0x04001000, 0x00041000,
    0x00041000, 0x04040000, 0x04040000, 0x04001010,
    0x00040010, 0x04000010, 0x04000010, 0x00040010,
    0x00000000, 0x00001010, 0x00041010, 0x04000000,
    0x00040000, 0x04041010, 0x00000010, 0x04040000,
    0x04041000, 0x04000000, 0x04000000, 0x00001000,
    0x04040010, 0x00040000, 0x00041000, 0x04000010,
    0x00001000, 0x00000010, 0x04001010, 0x00041010,
    0x04041010, 0x00040010, 0x04040000, 0x04001010,
    0x04000010, 0x00001010, 0x00041010, 0x04041000,
    0x00001010, 0x04001000, 0x04001000, 0x00000000,
    0x00040010, 0x00041000, 0x00000000, 0x04040010,
    0x00420082, 0x00020002, 0x00020000, 0x00420080,
    0x00400000, 0x00000080, 0x00400082, 0x00020082,
    0x00000082, 0x00420082, 0x00420002, 0x00000002,
    0x00020002, 0x00400000, 0x00000080, 0x00400082,
    0x00420000, 0x00400080, 0x00020082, 0x00000000,
    0x00000002, 0x00020000, 0x00420080, 0x00400002,
    0x00400080, 0x00000082, 0x00000000, 0x00420000,
    0x00020080, 0x00420002, 0x00400002, 0x00020080,
    0x00000000, 0x00420080, 0x00400082, 0x00400000,
    0x00020082, 0x00400002, 0x00420002, 0x00020000,
    0x00400002, 0x00020002, 0x00000080, 0x00420082,
    0x00420080, 0x00000080, 0x00020000, 0x00000002,
    0x00020080, 0x00420002, 0x00400000, 0x00000082,
    0x00400080, 0x00020082, 0x00000082, 0x00400080,
    0x00420000, 0x00000000, 0x00020002, 0x00020080,
    0x00000002, 0x00400082, 0x00420082, 0x00420000,
    0x00000820, 0x20080800, 0x00000000, 0x20080020,
    0x20000800, 0x00000000, 0x00080820, 0x20000800,
    0x00080020, 0x20000020, 0x20000020, 0x00080000,
    0x20080820, 0x00080020, 0x20080000, 0x00000820,
    0x20000000, 0x00000020, 0x20080800, 0x00000800,
    0x00080800, 0x20080000, 0x20080020, 0x00080820,
    0x20000820, 0x00080800, 0x00080000, 0x20000820,
    0x00000020, 0x20080820, 0x00000800, 0x20000000,
    0x20080800, 0x20000000, 0x00080020, 0x00000820,
    0x00080000, 0x20080800, 0x20000800, 0x00000000,
    0x00000800, 0x00080020, 0x20080820, 0x20000800,
    0x20000020, 0x00000800, 0x00000000, 0x20080020,
    0x20000820, 0x00080000, 0x20000000, 0x20080820,
    0x00000020, 0x00080820, 0x00080800, 0x20000020,
    0x20080000, 0x20000820, 0x00000820, 0x20080000,
    0x00080820, 0x00000020, 0x20080020, 0x00080800,
    0x02008004, 0x00008204, 0x00008204, 0x00000200,
    0x02008200, 0x02000204, 0x02000004, 0x00008004,
    0x00000000, 0x02008000, 0x02008000, 0x02008204,
    0x00000204, 0x00000000, 0x02000200, 0x02000004,
    0x00000004, 0x00008000, 0x02000000, 0x02008004,
    0x00000200, 0x02000000, 0x00008004, 0x00008200,
    0x02000204, 0x00000004, 0x00008200, 0x02000200,
    0x00008000, 0x02008200, 0x02008204, 0x00000204,
    0x02000200, 0x02000004, 0x02008000, 0x02008204,
    0x00000204, 0x00000000, 0x00000000, 0x02008000,
    0x00008200, 0x02000200, 0x02000204, 0x00000004,
    0x02008004, 0x00008204, 0x00008204, 0x00000200,
    0x02008204, 0x00000204, 0x00000004, 0x00008000,
    0x02000004, 0x00008004, 0x02008200, 0x02000204,
    0x00008004, 0x00008200, 0x02000000, 0x02008004,
    0x00000200, 0x02000000, 0x00008000, 0x02008200,
    0x00000400, 0x08200400, 0x08200000, 0x08000401,
    0x00200000, 0x00000400, 0x00000001, 0x08200000,
    0x00200401, 0x00200000, 0x08000400, 0x00200401,
    0x08000401, 0x08200001, 0x00200400, 0x00000001,
    0x08000000, 0x00200001, 0x00200001, 0x00000000,
    0x00000401, 0x08200401, 0x08200401, 0x08000400,
    0x08200001, 0x00000401, 0x00000000, 0x08000001,
    0x08200400, 0x08000000, 0x08000001, 0x00200400,
    0x00200000, 0x08000401, 0x00000400, 0x08000000,
    0x00000001, 0x08200000, 0x08000401, 0x00200401,
    0x08000400, 0x00000001, 0x08200001, 0x08200400,
    0x00200401, 0x00000400, 0x08000000, 0x08200001,
    0x08200401, 0x00200400, 0x08000001, 0x08200401,
    0x08200000, 0x00000000, 0x00200001, 0x08000001,
    0x00200400, 0x08000400, 0x00000401, 0x00200000,
    0x00000000, 0x00200001, 0x08200400, 0x00000401,
    0x80000040, 0x81000000, 0x00010000, 0x81010040,
    0x81000000, 0x00000040, 0x81010040, 0x01000000,
    0x80010000, 0x01010040, 0x01000000, 0x80000040,
    0x01000040, 0x80010000, 0x80000000, 0x00010040,
    0x00000000, 0x01000040, 0x80010040, 0x00010000,
    0x01010000, 0x80010040, 0x00000040, 0x81000040,
    0x81000040, 0x00000000, 0x01010040, 0x81010000,
    0x00010040, 0x01010000, 0x81010000, 0x80000000,
    0x80010000, 0x00000040, 0x81000040, 0x01010000,
    0x81010040, 0x01000000, 0x00010040, 0x80000040,
    0x01000000, 0x80010000, 0x80000000, 0x00010040,
    0x80000040, 0x81010040, 0x01010000, 0x81000000,
    0x01010040, 0x81010000, 0x00000000, 0x81000040,
    0x00000040, 0x00010000, 0x81000000, 0x01010040,
    0x00010000, 0x01000040, 0x80010040, 0x00000000,
    0x81010000, 0x80000000, 0x01000040, 0x80010040,
    0x00800000, 0x10800008, 0x10002008, 0x00000000,
    0x00002000, 0x10002008, 0x00802008, 0x10802000,
    0x10802008, 0x00800000, 0x00000000, 0x10000008,
    0x00000008, 0x10000000, 0x10800008, 0x00002008,
    0x10002000, 0x00802008, 0x00800008, 0x10002000,
    0x10000008, 0x10800000, 0x10802000, 0x00800008,
    0x10800000, 0x00002000, 0x00002008, 0x10802008,
    0x00802000, 0x00000008, 0x10000000, 0x00802000,
    0x10000000, 0x00802000, 0x00800000, 0x10002008,
    0x10002008, 0x10800008, 0x10800008, 0x00000008,
    0x00800008, 0x10000000, 0x10002000, 0x00800000,
    0x10802000, 0x00002008, 0x00802008, 0x10802000,
    0x00002008, 0x10000008, 0x10802008, 0x10800000,
    0x00802000, 0x00000000, 0x00000008, 0x10802008,
    0x00000000, 0x00802008, 0x10800000, 0x00002000,
    0x10000008, 0x10002000, 0x00002000, 0x00800008,
    0x40004100, 0x00004000, 0x00100000, 0x40104100,
    0x40000000, 0x40004100, 0x00000100, 0x40000000,
    0x00100100, 0x40100000, 0x40104100, 0x00104000,
    0x40104000, 0x00104100, 0x00004000, 0x00000100,
    0x40100000, 0x40000100, 0x40004000, 0x00004100,
    0x00104000, 0x00100100, 0x40100100, 0x40104000,
    0x00004100, 0x00000000, 0x00000000, 0x40100100,
    0x40000100, 0x40004000, 0x00104100, 0x00100000,
    0x00104100, 0x00100000, 0x40104000, 0x00004000,
    0x00000100, 0x40100100, 0x00004000, 0x00104100,
    0x40004000, 0x00000100, 0x40000100, 0x40100000,
    0x40100100, 0x40000000, 0x00100000, 0x40004100,
    0x00000000, 0x40104100, 0x00100100, 0x40000100,
    0x40100000, 0x40004000, 0x40004100, 0x00000000,
    0x40104100, 0x00104000, 0x00104000, 0x00004100,
    0x00004100, 0x00100100, 0x40000000, 0x40104000,
};

#endif
// Secure memset routine
void R_memset( POINTER output, int value, unsigned int len )
{
    if ( len != 0 ) {
        do {
            *output++ = ( unsigned char )value;
        } while ( --len != 0 );
    }
}

// Secure memcpy routine
void R_memcpy( POINTER output, POINTER input, unsigned int len )
{
    if ( len != 0 ) {
        do {
            *output++ = *input++;
        } while ( --len != 0 );
    }
}

// Secure memcmp routine
int R_memcmp( POINTER Block1, POINTER Block2, unsigned int len )
{
    if ( len != 0 ) {
        // little trick in declaring vars
        register const unsigned char *p1 = Block1, *p2 = Block2;

        do {
            if ( *p1++ != *p2++ ) {
                return( *--p1 - *--p2 );
            }
        } while ( --len != 0 );
    }

    return( 0 );
}

static void cookey( UINT4 *subkeys, UINT4 *kn, int encrypt )
{
    UINT4 *cooked, *raw0, *raw1;
    int increment;
    unsigned int i;

    raw1 = kn;
    cooked = encrypt ? subkeys : &subkeys[30];
    increment = encrypt ? 1 : -3;

    for ( i = 0; i < 16; i++, raw1++ ) {
        raw0 = raw1++;
        *cooked    = ( *raw0 & 0x00fc0000L ) << 6;
        *cooked   |= ( *raw0 & 0x00000fc0L ) << 10;
        *cooked   |= ( *raw1 & 0x00fc0000L ) >> 10;
        *cooked++ |= ( *raw1 & 0x00000fc0L ) >> 6;
        *cooked    = ( *raw0 & 0x0003f000L ) << 12;
        *cooked   |= ( *raw0 & 0x0000003fL ) << 16;
        *cooked   |= ( *raw1 & 0x0003f000L ) >> 4;
        *cooked   |= ( *raw1 & 0x0000003fL );
        cooked += increment;
    }
}

/*******************************************************************
int main(int argc, char ** argv)
{
	Cipher cip;
	char key[CIPHER_CONST_LENGTH+1], code[CIPHER_CONST_LENGTH+1], *p;

	strcpy(key, "lixu0071889");
	p = cip.GetCipher(key);
	if( p==NULL )
	{
		printf("Error from GetCipher\n");
		return -1;
	} else
	{
		printf("CIPHER£º---%s---\n", p);
		strcpy(code, p);
	}

	p = cip.GetPlainText(code);
	if( p==NULL )
	{
		printf("Error from GetPlainText\n");
		return -1;
	} else
	{
		printf("PlainText£º---%s---\n", p);
		strcpy(code, p);
	}
	return 0;
}
*******************************************************************/

//////////////////////////////////////////////////////////////////////
// Implementation of OP_Des classe
//////////////////////////////////////////////////////////////////////
OP_Des::OP_Des()
{
}

OP_Des::~OP_Des()
{
    R_memset( ( POINTER )( &context ), 0x00, sizeof( DES_CBC_CTX ) );
    R_memset( ( POINTER )( &context3 ), 0x00, sizeof( DES3_CBC_CTX ) );
    R_memset( ( POINTER )( &contextX ), 0x00, sizeof( DESX_CBC_CTX ) );
}

// Initialize context.  Caller must zeroize the context when finished.
void OP_Des::DES_CBCInit( unsigned char *key, unsigned char *iv, int encrypt )
{
    // Save encrypt flag to context.
    context.encrypt = encrypt;

    // Pack initializing vector into context.
    scrunch( context.iv, iv );
    scrunch( context.originalIV, iv );

    // Precompute key schedule
    deskey( context.subkeys, key, encrypt );
}

// DES-CBC block update operation. Continues a DES-CBC encryption
//	 operation, processing eight-byte message blocks, and updating
//	 the context.
//
//	 This requires len to be a multiple of 8.
int OP_Des::DES_CBCUpdate( unsigned char *output, unsigned char *input, unsigned int len )
{
    UINT4 inputBlock[2], work[2];
    unsigned int i;

    if ( len % 8 ) {                                                                        /* block size check */
        return( RE_LEN );
    }

    for ( i = 0; i < len / 8; i++ ) {
        scrunch( inputBlock, &input[8 * i] );

        // Chain if encrypting.

        if ( context.encrypt == 0 ) {
            *work = *inputBlock;
            *( work + 1 ) = *( inputBlock + 1 );
        } else {
            *work = *inputBlock ^ *( context.iv );
            *( work + 1 ) = *( inputBlock + 1 ) ^ *( context.iv + 1 );
        }

        desfunc( work, context.subkeys );

        // Chain if decrypting, then update IV.
        if ( context.encrypt == 0 ) {
            *work ^= *context.iv;
            *( work + 1 ) ^= *( context.iv + 1 );
            *context.iv = *inputBlock;
            *( context.iv + 1 ) = *( inputBlock + 1 );
        } else {
            *context.iv = *work;
            *( context.iv + 1 ) = *( work + 1 );
        }

        unscrunch ( &output[8 * i], work );
    }

    // Clear sensitive information.
    R_memset( ( POINTER )inputBlock, 0, sizeof( inputBlock ) );
    R_memset( ( POINTER )work, 0, sizeof( work ) );

    return( ID_OK );
}

void OP_Des::DES_CBCRestart()
{
    // Restore the original IV
    *context.iv = *context.originalIV;
    *( context.iv + 1 ) = *( context.originalIV + 1 );
}

// Initialize context.  Caller should clear the context when finished.
//	 The key has the DES key, input whitener and output whitener concatenated.
void OP_Des::DESX_CBCInit( unsigned char *key, unsigned char *iv, int encrypt )
{
    // Save encrypt flag to context.
    contextX.encrypt = encrypt;

    // Pack initializing vector and whiteners into context.
    scrunch( contextX.iv, iv );
    scrunch( contextX.inputWhitener, key + 8 );
    scrunch( contextX.outputWhitener, key + 16 );
    // Save the IV for use in Restart
    scrunch( contextX.originalIV, iv );

    // Precompute key schedule.
    deskey ( contextX.subkeys, key, encrypt );
}

// DESX-CBC block update operation. Continues a DESX-CBC encryption
//	 operation, processing eight-byte message blocks, and updating
//	 the context.
//
//	 Requires len to a multiple of 8.
int OP_Des::DESX_CBCUpdate ( unsigned char *output, unsigned char *input, unsigned int len )
{
    UINT4 inputBlock[2], work[2];
    unsigned int i;

    if ( len % 8 ) {	// Length check
        return( RE_LEN );
    }

    for ( i = 0; i < len / 8; i++ )  {
        scrunch( inputBlock, &input[8 * i] );

        // Chain if encrypting, and xor with whitener.
        if ( contextX.encrypt == 0 ) {
            *work = *inputBlock ^ *contextX.outputWhitener;
            *( work + 1 ) = *( inputBlock + 1 ) ^ *( contextX.outputWhitener + 1 );
        } else {
            *work = *inputBlock ^ *contextX.iv ^ *contextX.inputWhitener;
            *( work + 1 ) = *( inputBlock + 1 ) ^ *( contextX.iv + 1 ) ^ *( contextX.inputWhitener + 1 );
        }

        desfunc( work, contextX.subkeys );

        // Xor with whitener, chain if decrypting, then update IV.
        if ( contextX.encrypt == 0 ) {
            *work ^= *contextX.iv ^ *contextX.inputWhitener;
            *( work + 1 ) ^= *( contextX.iv + 1 ) ^ *( contextX.inputWhitener + 1 );
            *( contextX.iv ) = *inputBlock;
            *( contextX.iv + 1 ) = *( inputBlock + 1 );
        } else {
            *work ^= *contextX.outputWhitener;
            *( work + 1 ) ^= *( contextX.outputWhitener + 1 );
            *contextX.iv = *work;
            *( contextX.iv + 1 ) = *( work + 1 );
        }

        unscrunch( &output[8 * i], work );
    }

    R_memset( ( POINTER )inputBlock, 0, sizeof( inputBlock ) );
    R_memset( ( POINTER )work, 0, sizeof( work ) );

    return( ID_OK );
}

void OP_Des::DESX_CBCRestart()
{
    // Restore the original IV
    *contextX.iv = *contextX.originalIV;
    *( contextX.iv + 1 ) = *( contextX.originalIV + 1 );
}

// Initialize context.  Caller must zeroize the context when finished.
void OP_Des::DES3_CBCInit( unsigned char *key, unsigned char *iv, int encrypt )
{
    // Copy encrypt flag to context.
    context3.encrypt = encrypt;

    // Pack initializing vector into context.
    scrunch( context3.iv, iv );

    // Save the IV for use in Restart
    scrunch( context3.originalIV, iv );

    // Precompute key schedules.
    deskey( context3.subkeys[0], encrypt ? key : &key[16], encrypt );
    deskey( context3.subkeys[1], &key[8], !encrypt );
    deskey( context3.subkeys[2], encrypt ? &key[16] : key, encrypt );
}

int OP_Des::DES3_CBCUpdate( unsigned char *output, unsigned char *input, unsigned int len )
{
    UINT4 inputBlock[2], work[2];
    unsigned int i;

    if ( len % 8 ) {	// length check
        return( RE_LEN );
    }

    for ( i = 0; i < len / 8; i++ ) {
        scrunch( inputBlock, &input[8 * i] );

        // Chain if encrypting.
        if ( context3.encrypt == 0 ) {
            *work = *inputBlock;
            *( work + 1 ) = *( inputBlock + 1 );
        } else {
            *work = *inputBlock ^ *context3.iv;
            *( work + 1 ) = *( inputBlock + 1 ) ^ *( context3.iv + 1 );
        }

        desfunc( work, context3.subkeys[0] );
        desfunc( work, context3.subkeys[1] );
        desfunc( work, context3.subkeys[2] );

        // Chain if decrypting, then update IV.
        if ( context3.encrypt == 0 ) {
            *work ^= *context3.iv;
            *( work + 1 ) ^= *( context3.iv + 1 );
            *context3.iv = *inputBlock;
            *( context3.iv + 1 ) = *( inputBlock + 1 );
        } else {
            *context3.iv = *work;
            *( context3.iv + 1 ) = *( work + 1 );
        }

        unscrunch( &output[8 * i], work );
    }

    R_memset( ( POINTER )inputBlock, 0, sizeof( inputBlock ) );
    R_memset( ( POINTER )work, 0, sizeof( work ) );

    return ( 0 );
}

void OP_Des::DES3_CBCRestart()
{
    // Restore the original IV
    *context3.iv = *context3.originalIV;
    *( context3.iv + 1 ) = *( context3.originalIV + 1 );
}

void OP_Des::scrunch( UINT4 *into, unsigned char *outof )
{
    *into    = ( *outof++ & 0xffL ) << 24;
    *into   |= ( *outof++ & 0xffL ) << 16;
    *into   |= ( *outof++ & 0xffL ) << 8;
    *into++ |= ( *outof++ & 0xffL );
    *into    = ( *outof++ & 0xffL ) << 24;
    *into   |= ( *outof++ & 0xffL ) << 16;
    *into   |= ( *outof++ & 0xffL ) << 8;
    *into   |= ( *outof   & 0xffL );
}

void OP_Des::unscrunch( unsigned char *into, UINT4 *outof )
{
    *into++ = ( unsigned char )( ( *outof >> 24 ) & 0xffL );
    *into++ = ( unsigned char )( ( *outof >> 16 ) & 0xffL );
    *into++ = ( unsigned char )( ( *outof >>  8 ) & 0xffL );
    *into++ = ( unsigned char )( *outof++      & 0xffL );
    *into++ = ( unsigned char )( ( *outof >> 24 ) & 0xffL );
    *into++ = ( unsigned char )( ( *outof >> 16 ) & 0xffL );
    *into++ = ( unsigned char )( ( *outof >>  8 ) & 0xffL );
    *into   = ( unsigned char )( *outof        & 0xffL );
}

// Compute DES Subkeys
void OP_Des::deskey( UINT4 subkeys[32], unsigned char key[8], int encrypt )
{
    UINT4 kn[32];
    int i, j, l, m, n;
    unsigned char pc1m[56], pcr[56];

    for ( j = 0; j < 56; j++ ) {
        l = pc1[j];
        m = l & 07;
        pc1m[j] = ( unsigned char )( ( key[l >> 3] & bytebit[m] ) ? 1 : 0 );
    }

    for ( i = 0; i < 16; i++ ) {
        m = i << 1;
        n = m + 1;
        kn[m] = kn[n] = 0L;

        for ( j = 0; j < 28; j++ ) {
            l = j + totrot[i];

            if ( l < 28 ) {
                pcr[j] = pc1m[l];
            } else {
                pcr[j] = pc1m[l - 28];
            }
        }

        for ( j = 28; j < 56; j++ ) {
            l = j + totrot[i];

            if ( l < 56 ) {
                pcr[j] = pc1m[l];
            } else {
                pcr[j] = pc1m[l - 28];
            }
        }

        for ( j = 0; j < 24; j++ ) {
            if ( pcr[pc2[j]] ) {
                kn[m] |= bigbyte[j];
            }

            if ( pcr[pc2[j + 24]] ) {
                kn[n] |= bigbyte[j];
            }
        }
    }

    cookey( subkeys, kn, encrypt );

#ifdef DES386

    for ( i = 0; i < 32; i++ ) {
        subkeys[i] <<= 2;
    }

#endif

    R_memset( ( POINTER )pc1m, 0, sizeof( pc1m ) );
    R_memset( ( POINTER )pcr, 0, sizeof( pcr ) );
    R_memset( ( POINTER )kn, 0, sizeof( kn ) );
}

#ifndef DES386 // ignore C version in favor of 386 ONLY desfunc

#define	F(l,r,key) {work=((r>>4)|(r<<28))^*key;l^=Spbox[6][work&0x3f];l^=Spbox[4][(work>>8)&0x3f];l^=Spbox[2][(work>>16)&0x3f];l^=Spbox[0][(work>>24)&0x3f];work=r^*(key+1);l^=Spbox[7][work&0x3f];l^=Spbox[5][(work>>8)&0x3f];l^=Spbox[3][(work>>16)&0x3f];l^=Spbox[1][(work>>24)&0x3f];}

//This desfunc code is marginally quicker than that uses in
//	 RSAREF(tm)
void OP_Des::desfunc( UINT4 *block, UINT4 *ks )
{
    unsigned int left, right, work;

    left = block[0];
    right = block[1];

    work = ( ( left >> 4 ) ^ right ) & 0x0f0f0f0f;
    right ^= work;
    left ^= work << 4;
    work = ( ( left >> 16 ) ^ right ) & 0xffff;
    right ^= work;
    left ^= work << 16;
    work = ( ( right >> 2 ) ^ left ) & 0x33333333;
    left ^= work;
    right ^= ( work << 2 );
    work = ( ( right >> 8 ) ^ left ) & 0xff00ff;
    left ^= work;
    right ^= ( work << 8 );
    right = ( right << 1 ) | ( right >> 31 );
    work = ( left ^ right ) & 0xaaaaaaaa;
    left ^= work;
    right ^= work;
    left = ( left << 1 ) | ( left >> 31 );

    // Now do the 16 rounds
    F( left, right, &ks[0] );
    F( right, left, &ks[2] );
    F( left, right, &ks[4] );
    F( right, left, &ks[6] );
    F( left, right, &ks[8] );
    F( right, left, &ks[10] );
    F( left, right, &ks[12] );
    F( right, left, &ks[14] );
    F( left, right, &ks[16] );
    F( right, left, &ks[18] );
    F( left, right, &ks[20] );
    F( right, left, &ks[22] );
    F( left, right, &ks[24] );
    F( right, left, &ks[26] );
    F( left, right, &ks[28] );
    F( right, left, &ks[30] );

    right = ( right << 31 ) | ( right >> 1 );
    work = ( left ^ right ) & 0xaaaaaaaa;
    left ^= work;
    right ^= work;
    left = ( left >> 1 ) | ( left  << 31 );
    work = ( ( left >> 8 ) ^ right ) & 0xff00ff;
    right ^= work;
    left ^= work << 8;
    work = ( ( left >> 2 ) ^ right ) & 0x33333333;
    right ^= work;
    left ^= work << 2;
    work = ( ( right >> 16 ) ^ left ) & 0xffff;
    left ^= work;
    right ^= work << 16;
    work = ( ( right >> 4 ) ^ left ) & 0x0f0f0f0f;
    left ^= work;
    right ^= work << 4;

    *block++ = right;
    *block = left;
}

#endif // DES386 endif

//////////////////////////////////////////////////////////////////////
// Implementation of Cipher classe
//////////////////////////////////////////////////////////////////////
Cipher::Cipher(): OP_Des()
{

}

Cipher::~Cipher()
{

}

char *Cipher::trimrightsp( char *data )
{
    size_t i;

    for ( i = strlen( data ) - 1; i >= 0; i-- ) {
        if ( data[i] != 0x20 ) {
            break;
        } else {
            data[i] = 0x00;
        }
    }

    return data;
}

char *Cipher::GetCipher( const char *data )
{
    int ii;
    size_t size;
    char temp[CIPHER_CONST_LENGTH + 1];

    for ( ii = 0; ii < sizeof( m_bytKey ); ii++ ) {
        m_bytKey[ii] = bytKey[ii];
    }

    for ( ii = 0; ii < sizeof( m_bytIv ); ii++ ) {
        m_bytIv[ii] = bytIv[ii];
    }

    DES3_CBCInit( m_bytKey, m_bytIv, 1 );
    size = strlen( data );

    if ( size >= sizeof( m_plaintext ) ) {
        return NULL;
    }

    memset( m_plaintext, 0x20, sizeof( m_plaintext ) );
    m_plaintext[sizeof( m_plaintext ) - 1] = 0;
    memcpy( m_plaintext, data, size );

    if ( DES3_CBCUpdate( ( BYTE * )temp, ( BYTE * )m_plaintext, CIPHER_CONST_LENGTH ) != ID_OK ) {
        printf( "DES3_CBCUpdate error\n" );
        return NULL;
    }

    size = hextoascii( temp, CIPHER_CONST_LENGTH, m_cipher );
    m_cipher[size] = 0;
    return trimrightsp( m_cipher );
}

char *Cipher::GetPlainText( const char *data )
{
    int ii, size;
    char temp[CIPHER_CONST_LENGTH + 1];

    for ( ii = 0; ii < sizeof( m_bytKey ); ii++ ) {
        m_bytKey[ii] = bytKey[ii];
    }

    for ( ii = 0; ii < sizeof( m_bytIv ); ii++ ) {
        m_bytIv[ii] = bytIv[ii];
    }

    DES3_CBCInit( m_bytKey, m_bytIv, 0 );
    size = static_cast<int>( strlen( data ) );

    if ( size != CIPHER_CONST_LENGTH * 2 ) {
        return NULL;
    }

    size = asciitohex( data, size, temp );
    memset( m_cipher, 0x20, sizeof( m_cipher ) );
    m_cipher[sizeof( m_cipher ) - 1] = 0;
    memcpy( m_cipher, temp, size );

    if ( DES3_CBCUpdate( ( BYTE * )m_plaintext, ( BYTE * )m_cipher, CIPHER_CONST_LENGTH ) != ID_OK ) {
        printf( "DES3_CBCUpdate error\n" );
        return NULL;
    }

    m_plaintext[CIPHER_CONST_LENGTH] = 0;
    return trimrightsp( m_plaintext );
}

#ifdef _WIN32
char *Cipher::getpass( const char *disp_msg )
{
    int ii;
    char c;
    static char buf[256];

    ii = 0;
    buf[ii] = 0;
    _cputs( disp_msg );

    while ( ( c = _getch() ) != '\r' ) {
        if ( c == '\b' ) {
            if ( ii > 0 ) {
                ii--;
            }

            buf[ii] = 0;
        } else {
            buf[ii] = c;
            ii++;
            buf[ii] = 0;
        }
    }

    _cputs( "\n" );
    return buf;
}
#endif

int Cipher::hextoascii( const char* data, int dataLen, char * OutBuf )
{
    int OutLen = 0;
    BYTE cTemp, c1, c2;

    for ( int i = 0; i < dataLen; i++ ) {
        cTemp = ( unsigned char )( *( data + i ) );
        c1 = ( cTemp & 0xF0 ) >> 4;

        if ( c1 >= 0 && c1 < 0x0A ) {
            OutBuf[OutLen ++] = c1 + '0';
        } else if ( c1 >= 0x0A && c1 < 0x10 ) {
            OutBuf[OutLen ++] = c1 + 'A' - 0x0A;
        } else {
            OutBuf[OutLen ++] = 0;
        }

        c2 = cTemp & 0x0F;

        if ( c2 >= 0 && c2 < 0x0A ) {
            OutBuf[OutLen ++] = c2 + '0';
        } else if ( c2 >= 0x0A && c2 < 0x10 ) {
            OutBuf[OutLen ++] = c2 + 'A' - 0x0A;
        } else {
            OutBuf[OutLen ++] = 0;
        }
    }

    return OutLen;
}

int Cipher::asciitohex( const char* data, int dataLen, char * OutBuf )
{
    int retLen = 0, counter = 0;
    BYTE b1, b2;

    do {
        if ( data[counter] >= '0' && data[counter] <= '9' ) {
            b1 = data[counter] - '0';
        } else if ( data[counter] >= 'A' && data[counter] <= 'F' ) {
            b1 = data[counter] - 'A' + 0x0A;
        } else {
            b1 = 0;
        }

        counter ++;

        if ( data[counter] >= '0' && data[counter] <= '9' ) {
            b2 = data[counter] - '0';
        } else if ( data[counter] >= 'A' && data[counter] <= 'F' ) {
            b2 = data[counter] - 'A' + 0x0A;
        } else {
            b2 = 0;
        }

        counter ++;

        OutBuf[retLen ++] = ( b1 << 4 ) | b2;
    } while ( counter < dataLen );

    return retLen;
}
