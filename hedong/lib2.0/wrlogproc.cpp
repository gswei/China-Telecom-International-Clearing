
/* Result Sets Interface */
#ifndef SQL_CRSR
#  define SQL_CRSR
  struct sql_cursor
  {
    unsigned int curocn;
    void *ptr1;
    void *ptr2;
    unsigned int magic;
  };
  typedef struct sql_cursor sql_cursor;
  typedef struct sql_cursor SQL_CURSOR;
#endif /* SQL_CRSR */

/* Thread Safety */
typedef void * sql_context;
typedef void * SQL_CONTEXT;

/* Object support */
struct sqltvn
{
  unsigned char *tvnvsn; 
  unsigned short tvnvsnl; 
  unsigned char *tvnnm;
  unsigned short tvnnml; 
  unsigned char *tvnsnm;
  unsigned short tvnsnml;
};
typedef struct sqltvn sqltvn;

struct sqladts
{
  unsigned int adtvsn; 
  unsigned short adtmode; 
  unsigned short adtnum;  
  sqltvn adttvn[1];       
};
typedef struct sqladts sqladts;

static struct sqladts sqladt = {
  1,1,0,
};

/* Binding to PL/SQL Records */
struct sqltdss
{
  unsigned int tdsvsn; 
  unsigned short tdsnum; 
  unsigned char *tdsval[1]; 
};
typedef struct sqltdss sqltdss;
static struct sqltdss sqltds =
{
  1,
  0,
};

/* File name & Package Name */
struct sqlcxp
{
  unsigned short fillen;
           char  filnam[13];
};
static const struct sqlcxp sqlfpn =
{
    12,
    "wrlogproc.pc"
};


static unsigned int sqlctx = 340323;


static struct sqlexd {
   unsigned long  sqlvsn;
   unsigned int   arrsiz;
   unsigned int   iters;
   unsigned int   offset;
   unsigned short selerr;
   unsigned short sqlety;
   unsigned int   occurs;
      const short *cud;
   unsigned char  *sqlest;
      const char  *stmt;
   sqladts *sqladtp;
   sqltdss *sqltdsp;
   unsigned char  **sqphsv;
   unsigned long  *sqphsl;
            int   *sqphss;
            short **sqpind;
            int   *sqpins;
   unsigned long  *sqparm;
   unsigned long  **sqparc;
   unsigned short  *sqpadto;
   unsigned short  *sqptdso;
   unsigned int   sqlcmax;
   unsigned int   sqlcmin;
   unsigned int   sqlcincr;
   unsigned int   sqlctimeout;
   unsigned int   sqlcnowait;
            int   sqfoff;
   unsigned int   sqcmod;
   unsigned int   sqfmod;
   unsigned char  *sqhstv[1];
   unsigned long  sqhstl[1];
            int   sqhsts[1];
            short *sqindv[1];
            int   sqinds[1];
   unsigned long  sqharm[1];
   unsigned long  *sqharc[1];
   unsigned short  sqadto[1];
   unsigned short  sqtdso[1];
} sqlstm = {12,1};

// Prototypes
extern "C" {
  void sqlcxt (void **, unsigned int *,
               struct sqlexd *, const struct sqlcxp *);
  void sqlcx2t(void **, unsigned int *,
               struct sqlexd *, const struct sqlcxp *);
  void sqlbuft(void **, char *);
  void sqlgs2t(void **, char *);
  void sqlorat(void **, unsigned int *, void *);
}

// Forms Interface
static const int IAPSUCC = 0;
static const int IAPFAIL = 1403;
static const int IAPFTL  = 535;
extern "C" { void sqliem(char *, int *); }

typedef struct { unsigned short len; unsigned char arr[1]; } VARCHAR;
typedef struct { unsigned short len; unsigned char arr[1]; } varchar;

/* cud (compilation unit data) array */
static const short sqlcud0[] =
{12,4128,852,26,0,
};


/**********************************************************************

filename: wrlogproc.pc

module: WL -write log

created by: xiaowei

creat date:2002-10-10

update list:

  by xiaowei 2002-10-10 change the wrlogproc.ec esql code from esql-c to pro*c 

version: 1.1.1

description:

complete the write log operation

********************************************************************/



//include system lib

#include<stdio.h>

#include <iostream.h>



#include<string.h>



#include<dirent.h>



#include<sys/types.h>



#include<sys/stat.h>



//include self defined lib

#include "wrlog.h"



//include the sql communication areas

//EXEC SQL INCLUDE sqlca;

//EXEC SQL INCLUDE oraca;

//macro define





//declare  internal function

int wrlogproc(int flag,char* buffer);

//declare  datatype



//declare global variable

// 2000-12-27 by wh

extern char EnvFilePath[ENVFILEPATHLEN+1];

extern char LOGFILEPATH[256];



/********************************************************************

description:

  Call by wrlog function to write a log to log table in database

or a errorlog file.

input:

  a flag to identify the log type and a long string containing the

log information.

output:

  none

return:

 operation result or error code

******************************************************************/ 





int wrlogproc(int flag, char* buffer)



{

 /* EXEC SQL BEGIN DECLARE SECTION; */ 


 char temp[9][1000];

 long integer[5];

 //char SQLSTATE[6];

 //long SQLCODE;



 /* EXEC SQL END DECLARE SECTION; */ 




 int m,k;

 int j=0;	

 char ch;

 int first=0;

 char hour[3]; 

 int i=0;

 int pos;

 int u;

 int c;

 int o;

 int e;

 int l;

 int r;

 int x;

 int flagcmp=0;

 char filename[25];

 char addfile[150];

 char path[9];

 char desc[80];

 char obpath[150];

 char temptm[17];

 char tollmid[17];

 FILE *fp;

 int dirflag=0;

 int dbsflag=0;



 //read log file path 

 // 2000-12-27 by wh

 // if((fp=fopen("/usr/net/env/filelog.env","r"))==NULL) 



 //initialize the array,break the buffer into parameters,store them in the array   

  memset(temp, 0, sizeof(temp));

  

  for(i=0;i<strlen(buffer);i++)

    {

      if((buffer[i]==',' )&&(j<8))

        {

	  for(l=0;l<i-first;l++) temp[j][l]=buffer[l+first];

          first=i+1;

	  j++;

	}

      if(i==strlen(buffer)-1)

	{ 

	   int g;

	   int itmp=i;

	   if(i>1000)itmp=1000;

	   

	   for(g=0;g<itmp-first+1;g++) 

	   	{

	     	temp[j][g]=buffer[g+first];

	   	 }		

	}

    }//end for



 // process three types of log

 switch(flag)

 

 {

    case 1:
/*
     {

       

       

       EXEC SQL insert into operatelog values(:temp[0],:temp[1],

       :temp[2],:temp[3],:temp[4],:temp[5],:temp[6],:temp[7],:temp[8]);

   

       if( sqlca.sqlcode ) 

          {

            m=sqlca.sqlcode; 

            return(m);

          }

        EXEC SQL COMMIT WORK;

        if( sqlca.sqlcode ) 

          {

            m=sqlca.sqlcode; 

            return(m);

          }

      }//end case1

	  */

   break;





    case 2:
/*
     {

       

       

       EXEC SQL insert into gatherlog values(:temp[0],:temp[1],:temp[2],

        :temp[3],:temp[4],:temp[5]);

   

       if( sqlca.sqlcode ) 

         {

           k=sqlca.sqlcode; 

           return(k);

         }

	    EXEC SQL COMMIT WORK;

	   if( sqlca.sqlcode ) 

          {

            m=sqlca.sqlcode; 

            return(m);

          }

     }//end case2
*/
	  break;



   case 3:

     {  



        if ( strlen(temp[7])!=14) 

          return TIMEPARAMETERERROR;

        

        for( i=0;i<8;i++)

          path[i]=temp[7][i];

        path[8]='\0';

       

       /* 2000-12-27 by wh

        if (strlen(temp[0])==0) 

           strcpy(temp[0],"--"); */

        if (strlen(temp[0])==0) 

           strcpy(temp[0],"__"); 

           

        if(strlen(temp[0])>6)  

          return TOLLCODEERROR;

        if((strlen(temp[1])<1)||(strlen(temp[1])>10)) 

          return MODUL_IDERROR;

  

        for(e=0;e<strlen(temp[0])+strlen(temp[1])+4;e++)

 	 {	

 	    if(e<strlen(temp[0]))           

 	      filename[e]=temp[0][e];

 	    if((e==strlen(temp[0]))||(e==(strlen(temp[0])+strlen(temp[1])+1)))           

 	      filename[e]='.';

	    if((e>strlen(temp[0]))&&(e<strlen(temp[0])+strlen(temp[1])+1))

	      filename[e]=temp[1][e-strlen(temp[0])-1];



	    if(e==strlen(temp[0])+strlen(temp[1])+2)

	      filename[e]=temp[7][8];

            if(e==strlen(temp[0])+strlen(temp[1])+3)

              filename[e]=temp[7][9];

//        printf("%s::%c\n",filename,filename[e]);    



         }

        filename[strlen(temp[0])+strlen(temp[1])+4]='\0';



       hour[0]=temp[7][8];

       hour[1]=temp[7][9];

       hour[2]='\0';



       strcpy(obpath,LOGFILEPATH);

       strcat(obpath,"/");

       strcat(obpath,path);

       

       mkdir(obpath,0777);

       chmod(obpath,0777);

       strcat(obpath,"/");

       strcpy(addfile,obpath);

       strcat(addfile,hour);

       strcat(addfile,".ind");

       strcat(obpath,filename);

       strcat(obpath,".log");

  

       if((fp=fopen(obpath,"r"))==NULL)

      { if((fp=fopen(obpath,"w+"))==NULL)

          return OPENLOGFILEERROR;

               

         chmod(obpath,0777);}

        else  { fclose(fp);

        if((fp=fopen(obpath,"a+"))==NULL)

          return OPENLOGFILEERROR;}



        

      /* 2000-12-27 by wh 

      if(strlen(temp[2])==0)  

         strcpy(temp[2],"--");

       if(temp[3][0]==' ')  

         temp[3][0]='-';

       if(temp[4][0]==' ')  

         temp[4][0]='-'; */

       if(strlen(temp[2])==0)  

         strcpy(temp[2],"__");

       if(temp[3][0]==' ')  

         temp[3][0]='_';

       if(temp[4][0]==' ')  

         temp[4][0]='_';   

 

       for( o=0;o<9;o++)

         {

          fwrite(&temp[o],strlen(temp[o]),1,fp);          

          fputc(' ',fp);				

         }

       fputc('\n',fp);

       fclose(fp);

 

       if((fp=fopen(addfile,"r"))==NULL)

       {   

         if((fp=fopen(addfile,"w+"))==NULL)

         return OPENLOGFILEERROR;

         chmod(addfile,0777);

       } 

       else 

       {  fclose(fp);

       if((fp=fopen(addfile,"a+"))==NULL)

          return OPENLOGFILEERROR;

        }   

       strcpy(tollmid,temp[0]);

       strcat(tollmid,":");

       strcat(tollmid,temp[1]);

   

       while(!feof(fp))

         {

           fscanf(fp,"%s",temptm);

           if(strcmp(temptm,tollmid)==0)

             { flagcmp=1; break;}

         } 

       if(!flagcmp)

         {

            fprintf(fp,"%s",tollmid);

            fputc('\n',fp);

         }



       fclose(fp);

	

      }//end case3

    break;

   

   }//end case



 return 0;
}

 