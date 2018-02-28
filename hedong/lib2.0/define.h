#ifndef Goodzzp_Define_05_10
#define  Goodzzp_Define_05_10

#include "string.h"	//Ìá¹©memsetº¯Êý

//define variable types
typedef unsigned char UI8;
typedef char I8;
typedef unsigned short UI16;
typedef short I16;
typedef unsigned int UI32;
typedef int I32;
typedef unsigned long long UI64;
typedef long long I64;
//typedef unsigned __int64 UI64; 
//typedef __int64 I64; 


//abcd --->  dcba
#define InverseByte32(x)	((x<<24)|(x>>24)|((x<<8)&0xff0000)|((x>>8)&0xff00))	
//abcdefgh ---> hgfedcba
#define InverseByte64(n)	((n&0xff) << 56) | ((n&0xff00) << 40) |\
							((n&0xff0000) << 24) | ((n&0xff000000) << 8) |\
							((n&0xff00000000) >> 8)| ((n&0xff0000000000) >> 24) |\
							((n&0xff000000000000) >> 40) | ((n&0xff00000000000000) >> 56) 

// ************** rotate functions ***************
template <class T> inline T rotlFixed(T x, unsigned int y)
{
	return (x<<y) | (x>>(sizeof(T)*8-y));
}

template <class T> inline T rotrFixed(T x, unsigned int y)
{
	return (x>>y) | (x<<(sizeof(T)*8-y));
}

template <class T> inline T rotlVariable(T x, unsigned int y)
{
	return (x<<y) | (x>>(sizeof(T)*8-y));
}

template <class T> inline T rotrVariable(T x, unsigned int y)
{
	return (x>>y) | (x<<(sizeof(T)*8-y));
}

template <class T> inline T rotlMod(T x, unsigned int y)
{
	y %= sizeof(T)*8;
	return (x<<y) | (x>>(sizeof(T)*8-y));
}

template <class T> inline T rotrMod(T x, unsigned int y)
{
	y %= sizeof(T)*8;
	return (x>>y) | (x<<(sizeof(T)*8-y));
}

#endif

