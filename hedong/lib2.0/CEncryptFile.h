/****************************************************************
filename: CEncryptFile.h
module: 
created by: Panwj
create date: 2005-09-20
update list: 
version: 1.0.0
description:
    Encrypt the file.
*****************************************************************/

#ifndef _CEncryptFile_H_
#define _CEncryptFile_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>

#define MAX_BLOCK_SIZE  4000
#define BUFFER_LEN      4096
#define SUCC    	0
#define FAIL     	-1

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
