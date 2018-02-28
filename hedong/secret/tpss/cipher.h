/****************************************************************
  Project
  Copyright (c)	2000-2005. All Rights Reserved.

  SUBSYSTEM:
  FILE:			Cipher.h
  AUTHOR:		Jack Lee
  Create Time:  9/24/2000
==================================================================
  Description:

  UpdateRecord:
*****************************************************************/
#ifndef _CIPHER_H_
#define _CIPHER_H_

// PROTOTYPES should be set to one if and only if the compiler supports
//	function argument prototyping.
// The following makes PROTOTYPES default to 1 if it has not already been
//	defined as 0 with C compiler flags.
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

typedef unsigned char *POINTER;			// POINTER defines a generic pointer type
typedef unsigned short int UINT2;		// UINT2 defines a two byte word
typedef unsigned int UINT4;				// UINT4 defines a four byte word
typedef unsigned char BYTE;				// BYTE defines a unsigned character
typedef signed int signeddigit;			// internal signed value

#ifndef NULL_PTR
#define NULL_PTR ((POINTER)0)
#endif

#ifndef UNUSED_ARG
#define UNUSED_ARG(x) x = *(&x);
#endif

// PROTO_LIST is defined depending on how PROTOTYPES is defined above.
//	If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
//	returns an empty list.
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

#define ID_OK    0
#define ID_ERROR 1

//Error codes
#define RE_CONTENT_ENCODING 0x0400
#define RE_DATA 0x0401
#define RE_DIGEST_ALGORITHM 0x0402
#define RE_ENCODING 0x0403
#define RE_KEY 0x0404
#define RE_KEY_ENCODING 0x0405
#define RE_LEN 0x0406
#define RE_MODULUS_LEN 0x0407
#define RE_NEED_RANDOM 0x0408
#define RE_PRIVATE_KEY 0x0409
#define RE_PUBLIC_KEY 0x040a
#define RE_SIGNATURE 0x040b
#define RE_SIGNATURE_ENCODING 0x040c
#define RE_ENCRYPTION_ALGORITHM 0x040d
#define RE_FILE 0x040e

typedef struct {
    UINT4 subkeys[32];		// subkeys
    UINT4 iv[2];				// initializing vector
    UINT4 originalIV[2];		// for restarting the context
    int encrypt;				// encrypt flag
} DES_CBC_CTX;

typedef struct {
    UINT4 subkeys[32];		// subkeys
    UINT4 iv[2];				// initializing vector
    UINT4 inputWhitener[2];	// input whitener
    UINT4 outputWhitener[2];	// output whitener
    UINT4 originalIV[2];		// for restarting the context
    int encrypt;				// encrypt flag
} DESX_CBC_CTX;

typedef struct {
    UINT4 subkeys[3][32];		// subkeys for three operations
    UINT4 iv[2];				// initializing vector
    UINT4 originalIV[2];		// for restarting the context
    int encrypt;				// encrypt flag
} DES3_CBC_CTX;


class OP_Des
{
    // Attributes
private:
    DES_CBC_CTX context;
    DES3_CBC_CTX context3;
    DESX_CBC_CTX contextX;

    // Construction
public:
    OP_Des();
    ~OP_Des();

    // Operations
private:
    void scrunch ( UINT4 *into, unsigned char *outof );
    void unscrunch( unsigned char *into, UINT4 *outof );
    void deskey( UINT4 subkeys[32], unsigned char key[8], int encrypt );
#ifndef DES386
    void desfunc( UINT4 *block, UINT4 *ks );
#endif

public:
    void DES_CBCInit( unsigned char *key, unsigned char *iv, int encrypt );
    int DES_CBCUpdate( unsigned char *output, unsigned char *input, unsigned int len );
    void DES_CBCRestart();

    void DESX_CBCInit( unsigned char *key, unsigned char *iv, int encrypt );
    int DESX_CBCUpdate ( unsigned char *output, unsigned char *input, unsigned int len );
    void DESX_CBCRestart();

    void DES3_CBCInit( unsigned char *key, unsigned char *iv, int encrypt );
    int DES3_CBCUpdate( unsigned char *output, unsigned char *input, unsigned int len );
    void DES3_CBCRestart();
};

const int CIPHER_CONST_LENGTH = 32;

class Cipher : public OP_Des
{
private:
    BYTE m_bytKey[24];
    BYTE m_bytIv[8];
    char m_cipher[CIPHER_CONST_LENGTH * 2 + 1];
    char m_plaintext[CIPHER_CONST_LENGTH + 1];

    // Operation
private:
    int hextoascii( const char* data, int dataLen, char * OutBuf );
    int asciitohex( const char* data, int dataLen, char * OutBuf );
    char *trimrightsp( char *data );
#ifdef _WIN32
    char *getpass( const char *disp_msg );
#endif
public:
    char *GetCipher( const char *data );
    char *GetPlainText( const char *data );
    Cipher();
    virtual ~Cipher();
};

#endif
