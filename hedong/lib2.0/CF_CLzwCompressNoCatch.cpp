/****************************************************************
filename: CF_CLzwCompressNoCatch.cpp
module: CF - Common Function
created by: wang hui
create date: 2001-04-04
update list: 
version: 1.1.1
description:
    the program file of the classes for CF_CLzwCompressNoCatch, which
is the version without try...catch...throw
*****************************************************************/

// include system lib
#include <stdio.h>
#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// include self defined lib

#include "CF_CLzwCompressNoCatch.h"

// macro define

// declare internal function

// declare datatype

// declare global variable 

/***************************************************
description:
   compress the input file using LZW.
input:
  inFileName - the input filename
  outFileName - the output filename
output:
  none
return
  SUC/FAIL;
*****************************************************/
int CF_CLzwCompressNoCatch::compress(char *inFileName, char *outFileName)
{
  cout<<"compress "<<inFileName<<" to "<<outFileName<<endl;
  
  char msg[300];
  if ( NULL == (infp = fopen(inFileName, "rb" )) ) {
    sprintf(msg, "Cant open %s", inFileName);
    perror(msg);
    return FAIL;
   }
  
  if ( NULL == (outfp = fopen(outFileName, "wb")) ) {
    sprintf(msg, "Cant open %s", outFileName);
    perror(msg);
    return FAIL;
  }
  
  inbuf = EMPTY;
  outbuf = EMPTY;
  limit = 0;
  memset(insector, 0, SECTSIZE);
  current = 0;
  sector = 0;
  memset(outsector, 0, SECTSIZE);
  outcurrent = 0;
  memset(stack, 0, STACKSIZE);  
  sp = 0;            
  hasError = 0;
  osErrorNo = 0;   
  
  int ret=SUC;
  /*
   try {
   	compressFile();
    } catch (int isError) {
    	ret = FAIL;
     } */
   compressFile();   	
   if (hasError)
   	 ret=osErrorNo;	
        
   fclose(infp);
   fclose(outfp);
   // return ret;
   display_ratio(inFileName, outFileName);

   return ret;
   
}  // compress()



/***************************************************
description:
   uncompress the input file using LZW.
input:
  inFileName - the input filename
  outFileName - the output filename
output:
  none
return
  SUC/FAIL;
*****************************************************/
int CF_CLzwCompressNoCatch::uncompress(char *inFileName, char *outFileName)
{
  char msg[300];
  cout<<"uncompress from "<<inFileName<<" to "<<outFileName<<endl;

  if (NULL == (infp = fopen(inFileName, "rb" )) ) {
    sprintf(msg, "Cant open %s", inFileName);
    perror(msg);
    return FAIL;
   }
  
  if (NULL == (outfp = fopen(outFileName, "wb")) ) {
    sprintf(msg, "Cant open %s", outFileName);
    perror(msg);
    return FAIL;
  }
  
  inbuf = EMPTY;
  outbuf = EMPTY;
  limit = 0;
  memset(insector, 0, SECTSIZE);
  current = 0;
  sector = 0;
  memset(outsector, 0, SECTSIZE);
  outcurrent = 0;
  memset(stack, 0, STACKSIZE);  
  sp = 0;            
  hasError = 0;
  osErrorNo = 0;   

  int ret=SUC;    
  /*
   try {
   	uncompressFile();
    } catch (int isError) {
    	ret = FAIL;
     } */
   
   uncompressFile();	
   if (hasError)
   	 ret = osErrorNo;	

   fclose(infp);
   fclose(outfp);
   // return ret;
   
   display_ratio(inFileName, outFileName);

   return ret;
   
}  // uncompress()

/***************************************************
description:
   compress the input file using LZW.
input:
  inFileName - the input filename
  outFileName - the output filename
output:
  none
return
  SUC/FAIL;
*****************************************************/
void CF_CLzwCompressNoCatch::compressFile()
{
  unsigned int c, code, localcode;
  int code_count = TABSIZE - 256;

  init_tab();                           /* initialize code table        */
  
  c = readc();
  if (hasError)  return;	// may be error, may be empty
  	   
  code = unhash(NO_PRED,c);             /* initial code for table       */

  while (( UEOF != (c = readc()) ) && (!hasError))  {

       if ( NOT_FND != (localcode = unhash(code,c)) )    {
		code = localcode;
		continue;
        } // if no found
		  
	/* when the above clause comes false, you have found the last known code */
	putcode(code);      /* only update table if table isn't full */
   	if (hasError) return;	// error happened
		  
	if ( code_count )    {
	      upd_tab(code,c);
	      --code_count;
	  }  // if code count
	      	 
	/* start loop again with the char that didn't fit into last string      */
	 code = unhash(NO_PRED,c);
   }   // while
   if (hasError) return;	// error happened
   
   putcode(code);                 
   if (hasError) return;	// error happened
	                                        
   flushout();                      /* make sure everything's written */  
}  // compressFile()


/***************************************************
description:
   uncompress the input file compress by LZW.
input:
  inFileName - the input filename
  outFileName - the output filename
output:
  none
return
  SUC/FAIL;
*****************************************************/

void CF_CLzwCompressNoCatch::uncompressFile()
{
  unsigned int c, tempc, code, oldcode, incode, finchar,lastchar;
  char unknown = FALSE;
  int code_count = TABSIZE - 256;
  CF_CSlzwEntry *ep;

  init_tab();                           /* set up atomic code definitions*/  
  
  code = oldcode = getcode();
  if (hasError) return;	// error happened

  c = string_tab[code].follower;        /* first code always known      */
  writec(c);
  if (hasError) return;	// error happened

  finchar = c;

  while (( UEOF != (code = incode = getcode()) )&&(!hasError)) {	
  	ep = &string_tab[code];             /* initialize pointer           */
  	if ( !ep->used ) {                  /* if code isn't known          */
	      	lastchar = finchar;
	      	code = oldcode;
	      	unknown = TRUE;
        	ep = &string_tab[code];           /* re-initialize pointer        */
   	  }   // if not used
 
  	while (NO_PRED != ep->predecessor)  {
      		push( ep->follower);              /* decode string backwards into */
                                  		  /* stack                        */
                if (hasError) return;		// error happened
                
  		code = ep->predecessor;
  		ep = &string_tab[code];
   	  }  // while  no pred

  	finchar = ep->follower;
  	/* above loop terminates, one way or another, with                  */
  	/* string_tab[code].follower = first char in string                 */

  	writec(finchar);
   	if (hasError) return;	// error happened
  
  	/* pop anything stacked during code parsing                         */
  	while ( EMPTY != (tempc = pop()) )  {
  		writec(tempc);
  		if (hasError) return;	// error happened

   	 }  // while no empty
   
  	if ( unknown ) {                  /* if code isn't known          */
  		writec(finchar = lastchar); /* the follower char of last    */
  		if (hasError) return;	// error happened

	  	unknown = FALSE;
	 }  // if unknown
	 
  	if ( code_count )  {
  		upd_tab(oldcode,finchar);
     		--code_count;
    	 }  // if code_count
    	 
   	oldcode = incode;
    }   // while 
   if (hasError) return;	// error happened
     
  flushout();      /* write out fractional buffer, if any          */
   
}  // uncompressFile()


/***************************************************
description:
   push character into stack
input:
  c - character
output:
  none
return
  SUC/FAIL(throw out)
*****************************************************/
int CF_CLzwCompressNoCatch::push(int c)
{
   stack[sp] = ((char) c);/* coerce passed integer into a character      */
   ++sp;
   if (sp >= STACKSIZE) {
     cerr<<"Stack overflow, aborting...\n"<<endl;
     hasError = 1;
     osErrorNo = FAIL;
   }
   
   return SUC;
}  // pop

/***************************************************
description:
   pop character from stack
input:
  c - character
output:
  none
return
  c - character
*****************************************************/
int CF_CLzwCompressNoCatch::pop() 
{
  if (sp > 0)   {
    --sp;               /* push leaves sp pointing to next empty slot   */
    return ( (int) stack[sp] ); /* make sure pop returns char           */
  }
  else
    return EMPTY;
}   // pop()


/***************************************************
description:
   uses the 'mid-square' algorithm. I.E. for a hash val of n bits     
hash = middle binary digits of (key * key).  Upon collision, hash    
searches down linked list of keys that hashed to that key already.  
It will NOT notice if the table is full. This must be handled       
elsewhere
input:
  c - character
output:
  none
return
  he mid square of pred + foll
*****************************************************/
unsigned int CF_CLzwCompressNoCatch::h(unsigned int pred, unsigned char foll)
{
  long temp, local;             /* 32 bit receiving field for local^2   */
  
  local = (pred + foll) | 0x0800;
  temp = local * local;
  local = (temp >> 6) & 0x0FFF;
  
  return local;                 /* middle 12 bits of result             */
}   // h()

/***************************************************
description:
   return last element in a collision list
input:
  index - the index of first element in the collision list
output:
  none
return
  last element in a collision list
*****************************************************/
unsigned int CF_CLzwCompressNoCatch::eolist(unsigned int index)
{
  int temp;
  while ( 0 != (temp = string_tab[index].next) )
    	index = temp;
  return index;	
}  // eolist()


/***************************************************
description:
   hash function
input:
  pred - the predecessor
  foll - the follower
  update - the update flag
output:
  none
return
  hash result
*****************************************************/
unsigned int CF_CLzwCompressNoCatch::hash(unsigned int pred, unsigned char foll, int update)
{
  unsigned int local, tempnext;
  CF_CSlzwEntry *ep;
  
  local = h(pred,foll);
  if(!string_tab[local].used)
    return local;
  else
  {
  	/* if collision has occured    */
    local = eolist(local);

  	/* search for free entry from local + 101 */
    tempnext = (local + 101) & 0x0FFF; 
    ep = &string_tab[tempnext];                 /* initialize pointer   */
	  while ( ep->used )
	  {
	    ++tempnext;
	    if ( tempnext == TABSIZE )
	    {
	      tempnext = 0;           /* handle wrap to beginning of table    */
	      ep = string_tab;        /* address of first element of table    */
	    }
	    else
	      ++ep;                   /* point to next element in table       */
	  }  // while 
    	/* put new tempnext into last element in collision list             */ 
    if (update)               /* if update requested                  */
      string_tab[local].next = tempnext; 
    return tempnext;
  } 	// else   
}   //hash()

/***************************************************
description:
   unhash uses the 'next' field to go down the collision tree to find 
the entry corresponding to the passed key passed key 
 
input:          
  pred - the predecessor
  foll - the follower
output:
  none
return
  either the matching entry # or NOT_FND
*****************************************************/
unsigned int CF_CLzwCompressNoCatch::unhash(unsigned int pred, unsigned char foll)
{
  unsigned int local;
  CF_CSlzwEntry *ep;    /* pointer to current entry             */
  
  local = h(pred,foll);         /* initial hash                         */

  for (;;) {
    ep = &string_tab[local];
    if ( (ep->predecessor == pred) && (ep->follower == foll) ) {	
      	return local;
     }

    if ( ep->next == 0 ) {
      	return NOT_FND;
     }
     
    local = ep->next;
  }  // for
  	
}  // unhash()


/***************************************************
description:
   unhash uses the 'next' field to go down the collision tree to find 
the entry corresponding to the passed key passed key 
 
input:          
  pred - the predecessor
  foll - the follower
output:
  none
return
  either the matching entry # or NOT_FND
*****************************************************/
void CF_CLzwCompressNoCatch::init_tab()
{
  unsigned int i;
  
  memset((char *)string_tab, 0, sizeof(string_tab));  
  for (i = 0; i <= 255; i++)
  {
    upd_tab(NO_PRED,i);
  }
}  // init_tab()


/***************************************************
description:
   
input:          
  pred - the predecessor
  foll - the follower
output:
  none
return
  none
*****************************************************/
void CF_CLzwCompressNoCatch::upd_tab(unsigned int pred, unsigned int foll)
{
  CF_CSlzwEntry *ep;    /* pointer to current entry     */
  
  /* calculate offset just once */
  ep = &string_tab[ hash(pred,foll,UPDATE) ];
  ep->used = TRUE;
  ep->next = 0;
  ep->predecessor = pred;
  ep->follower = foll;
  	
}  // upd_tab()

/***************************************************
description:
   get code from file
input:          
  pred - the predecessor
  foll - the follower
output:
  none
return
  none
*****************************************************/
int CF_CLzwCompressNoCatch::getcode()
{
  unsigned int localbuf, returnval;
  if(EMPTY == inbuf)
  {         /* On code boundary                     */
    if((EOF == (localbuf = readc()))||(hasError))
    {                                 /* H L1 byte - on code boundary         */
      return EOF;
    }
    localbuf&=0xFF;
    if ((EOF==(inbuf=readc()))||(hasError))
    {  /* L0 Hnext                     */
      return EOF;       /* The file should always end on code boundary  */
    }
    inbuf&=0xFF;
    returnval=((localbuf<<4)&0xFF0)+((inbuf>>4)&0x00F);
    inbuf &= 0x000F;
  }
  else
  {                      /* buffer contains nibble H             */
    if((EOF==(localbuf=readc()))||(hasError))
      return EOF;
    localbuf &= 0xFF;
    returnval = localbuf + ((inbuf << 8) & 0xF00);
    inbuf = EMPTY;
  }
  return returnval;
}  // getcode()

/***************************************************
description:
   put code into file
input:          
  pred - the predecessor
  foll - the follower
output:
  none
return
  none
*****************************************************/
void CF_CLzwCompressNoCatch::putcode(unsigned int code)
{
  if (EMPTY == outbuf)
  {
    writec( ((code >> 4) & 0xFF));    /* H L1                        */
    if (hasError) return;	// error happened
       	
    outbuf = code & 0x000F;     /* L0                                   */
  } 
  else
  {
    writec(( ((outbuf << 4) & 0xFF0) + ((code >> 8) & 0x00F) ) );
     /* L0prev H                             */
    if (hasError) return;	// error happened                
    writec((code & 0x00FF));        /* L1 L0                        */
    if (hasError) return;	// error happened
    outbuf = EMPTY;
  }
}   // putcode()

/***************************************************
description:
   read a charactor from buffer, if buffer is empty 
read something from file to buffer
input:          
  none
output:
  none
return
  none
*****************************************************/
int CF_CLzwCompressNoCatch::readc()
{
  int returnval;
  if (current == 0)
  {
  	errno = 0;
  	limit = fread(insector, sizeof(char), SECTSIZE, infp);

  	if(ferror(infp))
  	{
  		perror("read file error!");
  	  osErrorNo=errno;
  	  hasError=1;
  	  return EOF;
  	}  
  	  
    if (0==limit)
    {
      return(EOF);
    }
  }

  returnval=(insector[current++]);
  if(current == limit)
  {
    current = 0;
  }
  return(returnval & 0xFF);
}  // readc()


/***************************************************
description:
   write a charactor to buffer, when the buffer is 
full, write all in buffer into file
input:          
  none
output:
  none
return
  none
*****************************************************/
void CF_CLzwCompressNoCatch::writec(int c)
{
  outsector[outcurrent++]=((char)c);
  if(outcurrent == SECTSIZE)
  {
    outcurrent = 0;
    int ret = fwrite(outsector, sizeof(char), SECTSIZE, outfp);
    if (ferror(outfp))
    {
    	perror("write file fail !");
    	hasError = 1;
    	osErrorNo = errno;
    }  
  }
}  // write


/***************************************************
description:
   write the left in the buffer to file
input:          
  none
output:
  none
return
  none
*****************************************************/
void CF_CLzwCompressNoCatch::flushout()
{
  /* if there's still a byte waiting to be packed */
  /* stuff it in the buffer */
  if (EMPTY != outbuf)	
	outsector[outcurrent++] = (outbuf << 4) & 0xFF0;	

  int ret = fwrite(outsector, sizeof(char), outcurrent, outfp);
  if (ferror(outfp)) {
  	perror("write file fail !");
  	hasError = 1;
  	osErrorNo = errno;
    }
}  // flush out() 

/***************************************************
description:
  display the ration of compress/uncompress
input:
  inFileName - input file name
  outFileName - output file name
output:
  none
return
  TRUE / FALSE 
*****************************************************/
int CF_CLzwCompressNoCatch::display_ratio(char *inFileName, char *outFileName)
{
    FILE *fp;
    char msg[300];
    
    if ((fp = fopen(inFileName, "rb")) == NULL) {
    	  sprintf(msg, "open input file %s fail", inFileName);
    	  perror(msg);
          fclose(fp);
    	  return FALSE;
     }
    
    long inputSize;
    fseek(fp, 0L, SEEK_END);
    inputSize = ftell(fp);  
    fclose(fp);
    
    if ((fp = fopen(outFileName, "rb")) == NULL) {
    	  sprintf(msg, "open output file %s fail", outFileName);
    	  perror(msg);
          fclose(fp);
    	  return FALSE;
     }
     
    long outputSize;
    fseek(fp, 0L, SEEK_END);
    outputSize = ftell(fp);  
    fclose(fp);
     
    float ratio;
    ratio = (float)outputSize/inputSize*100;
    cout<<endl<<"file size of "<<inFileName<<" is "<<inputSize
    	<<", file size of "<<outFileName<<" is "<<outputSize<<", ratio is "<<ratio<<"%"<<endl;
    return TRUE;
}

