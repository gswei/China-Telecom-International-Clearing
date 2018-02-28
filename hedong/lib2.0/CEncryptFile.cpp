/****************************************************************
filename: CEncryptFile.cpp
module: 
created by: Panwj
create date: 2005-09-20
update list: 
version: 1.0.0
description:
    Encrypt the file.
*****************************************************************/

#include "CEncryptFile.h"

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
      szRslBuffer[i] = szBuffer[i]^szPwd[j++];
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
      szRslBuffer[i] = szPwd[j++]^szBuffer[i];
    }
    
    fwrite(szRslBuffer, sizeof(char), iLenByte, outfp);
  }

  fclose(infp);
  fclose(outfp);
  return SUCC;	
}
