/****************************************************************
filename: CF_CLzwCompressNoCatch.h
module: CF - Common Function
created by: wang hui
create date: 2001-04-16
update list: 
version: 1.1.1
description:
    the header file of the classes CF_CLzwCompressNoCatch, which
is the version without try...catch...throw
*****************************************************************/

#ifndef _CF_CLzwCompressNoCatch_H_
#define _CF_CLzwCompressNoCatch_H_ 1

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FALSE    	0
#define TRUE     	!FALSE
#define TABSIZE  	4096
#define STACKSIZE 	TABSIZE
#define NO_PRED  	0xFFFF
#define EMPTY    	0xFFFF
#define NOT_FND  	0xFFFF
#define SECTSIZE 	128
#define CTRLZ 		'\032' /* ascii end of file */
#define UPDATE 		TRUE
#define NO_UPDATE 	FALSE
#define UEOF ((unsigned)EOF)

#define FAIL 	-1
#define SUC 	0
#define MSGLEN	300

struct CF_CSlzwEntry
{
  char used;
  unsigned int next;                    /* index of next in collision list */
  unsigned int predecessor;             /* 12 bit code                  */
  unsigned char follower;
};


class CF_CLzwCompressNoCatch
{
  public :  	
  	int compress(char *inFileName, char *outFileName);
  	int uncompress(char *inFileName, char *outFileName);
  private:
  	void compressFile();
  	
  	void uncompressFile();

	  unsigned int h(unsigned int pred, unsigned char foll);
	
  	unsigned int eolist(unsigned int index);
	
    unsigned int hash(unsigned int pred, unsigned char foll, int update);
	
  	unsigned int unhash(unsigned int pred, unsigned char foll);
	
  	void init_tab();
	
  	void upd_tab(unsigned int pred, unsigned int foll);
	
	  int getcode();
	
	  void putcode(unsigned int code);
	
	  int readc();
	
	  void writec(int c);
	
	  void flushout();
	
	  int push(int c);
	
	  int pop();
	
	  int display_ratio(char *inFileName, char *outFileName);
	
  private:
  	
  	CF_CSlzwEntry string_tab[TABSIZE];
	 
	  unsigned int inbuf;
	  
	  unsigned int outbuf;
	
	  FILE *infp, *outfp;
	
	  int limit;
	  char insector[SECTSIZE];
	  int current;
	  int sector;
	
	  char outsector[SECTSIZE];
	  int outcurrent;
	
	  char stack[STACKSIZE];  /* stack for pushing and popping characters     */
	  int sp;             /* current stack pointer                        */

	  int hasError;
	  int osErrorNo;
};

#endif
