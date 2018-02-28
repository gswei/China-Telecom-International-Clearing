
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
           char  filnam[11];
};
static const struct sqlcxp sqlfpn =
{
    10,
    "CF_CEnv.pc"
};


static unsigned int sqlctx = 73227;


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
   unsigned char  *sqhstv[3];
   unsigned long  sqhstl[3];
            int   sqhsts[3];
            short *sqindv[3];
            int   sqinds[3];
   unsigned long  sqharm[3];
   unsigned long  *sqharc[3];
   unsigned short  sqadto[3];
   unsigned short  sqtdso[3];
} sqlstm = {12,3};

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
5,0,0,1,76,0,4,237,0,0,3,2,0,1,0,2,97,0,0,1,97,0,0,1,97,0,0,
32,0,0,2,59,0,4,305,0,0,2,1,0,1,0,2,97,0,0,1,97,0,0,
};


/****************************************************************

filename: CF_CEnv.ec

module: CF - Common Function

created by: wang hui

create date: 2000-10-13

update list: 

	2000-12-25 by wh, updating the path of the env file 

version: 1.1.1

description:

    the header file of the classes for CF_CEnv

*****************************************************************/





// include system lib

#include <stdio.h>

#include <iostream.h>

#include <errno.h>

#include <time.h>

#include <string.h>

#include <strings.h>

#include <stdlib.h>



//#include <it.h>



#define SQLCA_STORAGE_CLASS extern
#define ORACA_STORAGE_CLASS extern
/* EXEC SQL INCLUDE sqlca;
 */ 
/*
 * $Header: sqlca.h 24-apr-2003.12:50:58 mkandarp Exp $ sqlca.h 
 */

/* Copyright (c) 1985, 2003, Oracle Corporation.  All rights reserved.  */
 
/*
NAME
  SQLCA : SQL Communications Area.
FUNCTION
  Contains no code. Oracle fills in the SQLCA with status info
  during the execution of a SQL stmt.
NOTES
  **************************************************************
  ***                                                        ***
  *** This file is SOSD.  Porters must change the data types ***
  *** appropriately on their platform.  See notes/pcport.doc ***
  *** for more information.                                  ***
  ***                                                        ***
  **************************************************************

  If the symbol SQLCA_STORAGE_CLASS is defined, then the SQLCA
  will be defined to have this storage class. For example:
 
    #define SQLCA_STORAGE_CLASS extern
 
  will define the SQLCA as an extern.
 
  If the symbol SQLCA_INIT is defined, then the SQLCA will be
  statically initialized. Although this is not necessary in order
  to use the SQLCA, it is a good pgming practice not to have
  unitialized variables. However, some C compilers/OS's don't
  allow automatic variables to be init'd in this manner. Therefore,
  if you are INCLUDE'ing the SQLCA in a place where it would be
  an automatic AND your C compiler/OS doesn't allow this style
  of initialization, then SQLCA_INIT should be left undefined --
  all others can define SQLCA_INIT if they wish.

  If the symbol SQLCA_NONE is defined, then the SQLCA variable will
  not be defined at all.  The symbol SQLCA_NONE should not be defined
  in source modules that have embedded SQL.  However, source modules
  that have no embedded SQL, but need to manipulate a sqlca struct
  passed in as a parameter, can set the SQLCA_NONE symbol to avoid
  creation of an extraneous sqlca variable.
 
MODIFIED
    lvbcheng   07/31/98 -  long to int
    jbasu      12/12/94 -  Bug 217878: note this is an SOSD file
    losborne   08/11/92 -  No sqlca var if SQLCA_NONE macro set 
  Clare      12/06/84 - Ch SQLCA to not be an extern.
  Clare      10/21/85 - Add initialization.
  Bradbury   01/05/86 - Only initialize when SQLCA_INIT set
  Clare      06/12/86 - Add SQLCA_STORAGE_CLASS option.
*/
 
#ifndef SQLCA
#define SQLCA 1
 
struct   sqlca
         {
         /* ub1 */ char    sqlcaid[8];
         /* b4  */ int     sqlabc;
         /* b4  */ int     sqlcode;
         struct
           {
           /* ub2 */ unsigned short sqlerrml;
           /* ub1 */ char           sqlerrmc[70];
           } sqlerrm;
         /* ub1 */ char    sqlerrp[8];
         /* b4  */ int     sqlerrd[6];
         /* ub1 */ char    sqlwarn[8];
         /* ub1 */ char    sqlext[8];
         };

#ifndef SQLCA_NONE 
#ifdef   SQLCA_STORAGE_CLASS
SQLCA_STORAGE_CLASS struct sqlca sqlca
#else
         struct sqlca sqlca
#endif
 
#ifdef  SQLCA_INIT
         = {
         {'S', 'Q', 'L', 'C', 'A', ' ', ' ', ' '},
         sizeof(struct sqlca),
         0,
         { 0, {0}},
         {'N', 'O', 'T', ' ', 'S', 'E', 'T', ' '},
         {0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0}
         }
#endif
         ;
#endif
 
#endif
 
/* end SQLCA */

/* EXEC SQL INCLUDE oraca;
 */ 
/*
 * $Header: oraca.h 24-apr-2003.12:50:59 mkandarp Exp $ oraca.h 
 */

/* Copyright (c) 1985, 2003, Oracle Corporation.  All rights reserved.  */
 
/*
NAME
  ORACA : Oracle Communications Area.
FUNCTION
  Contains no code. Provides supplementary communications to/from
  Oracle (in addition to standard SQLCA).
NOTES
  **************************************************************
  ***                                                        ***
  *** This file is SOSD.  Porters must change the data types ***
  *** appropriately on their platform.  See notes/pcport.doc ***
  *** for more information.                                  ***
  ***                                                        ***
  **************************************************************

  oracchf : Check cursor cache consistency flag. If set AND oradbgf
            is set, then directs SQLLIB to perform cursor cache
            consistency checks before every cursor operation
            (OPEN, FETCH, SELECT, INSERT, etc.).
  oradbgf : Master DEBUG flag. Used to turn all DEBUG options
            on or off.
  orahchf : Check Heap consistency flag. If set AND oradbgf is set,
            then directs SQLLIB to perform heap consistency checks
            everytime memory is dynamically allocated/free'd via
            sqlalc/sqlfre/sqlrlc. MUST BE SET BEFORE 1ST CONNECT
            and once set cannot be cleared (subsequent requests
            to change it are ignored).
  orastxtf: Save SQL stmt text flag. If set, then directs SQLLIB
            to save the text of the current SQL stmt in orastxt
            (in VARCHAR format).
  orastxt : Saved len and text of current SQL stmt (in VARCHAR
            format).
  orasfnm : Saved len and text of filename containing current SQL
            stmt (in VARCHAR format).
  oraslnr : Saved line nr within orasfnm of current SQL stmt.
 
  Cursor cache statistics. Set after COMMIT or ROLLBACK. Each
  CONNECT'd DATABASE has its own set of statistics.
 
  orahoc  : Highest Max Open OraCursors requested. Highest value
            for MAXOPENCURSORS by any CONNECT to this DATABASE.
  oramoc  : Max Open OraCursors required. Specifies the max nr
            of OraCursors required to run this pgm. Can be higher
            than orahoc if working set (MAXOPENCURSORS) was set
            too low, thus forcing the PCC to expand the cache.
  oracoc  : Current nr of OraCursors used.
  oranor  : Nr of OraCursor cache reassignments. Can show the
            degree of "thrashing" in the cache. Optimally, this
            nr should be kept as low as possible (time vs space
            optimization).
  oranpr  : Nr of SQL stmt "parses".
  oranex  : Nr of SQL stmt "executes". Optimally, the relation-
            ship of oranex to oranpr should be kept as high as
            possible.
 
 
  If the symbol ORACA_NONE is defined, then there will be no ORACA
  *variable*, although there will still be a struct defined.  This
  macro should not normally be defined in application code.

  If the symbol ORACA_INIT is defined, then the ORACA will be
  statically initialized. Although this is not necessary in order
  to use the ORACA, it is a good pgming practice not to have
  unitialized variables. However, some C compilers/OS's don't
  allow automatic variables to be init'd in this manner. Therefore,
  if you are INCLUDE'ing the ORACA in a place where it would be
  an automatic AND your C compiler/OS doesn't allow this style
  of initialization, then ORACA_INIT should be left undefined --
  all others can define ORACA_INIT if they wish.
 
OWNER
  Clare
DATE
  10/19/85
MODIFIED
    apopat     05/08/02  - [2362423] MVS PE to make lines shorter than 79
    apopat     07/31/99 -  [707588] TAB to blanks for OCCS
    lvbcheng   10/27/98 -  change long to int for oraca
    pccint     10/03/96 -  Add IS_OSD for linting
    jbasu      12/12/94 -  Bug 217878: note this is an SOSD file
    losborne   09/04/92 -  Make oraca variable optional 
    Osborne    05/24/90 - Add ORACA_STORAGE_CLASS construct
  Clare      02/20/86 - PCC [10101l] Feature: Heap consistency check.
  Clare      03/04/86 - PCC [10101r] Port: ORACA init ifdef.
  Clare      03/12/86 - PCC [10101ab] Feature: ORACA cuc statistics.
*/
/* IS_OSD */ 
#ifndef  ORACA
#define  ORACA     1
 
struct   oraca
         {
    /* text */ char oracaid[8];      /* Reserved                            */
    /* ub4  */ int oracabc;          /* Reserved                            */
 
    /*       Flags which are setable by User. */
 
   /* ub4 */ int  oracchf;           /* <> 0 if "check cur cache consistncy"*/
   /* ub4 */ int  oradbgf;           /* <> 0 if "do DEBUG mode checking"    */
   /* ub4 */ int  orahchf;           /* <> 0 if "do Heap consistency check" */
   /* ub4 */ int  orastxtf;          /* SQL stmt text flag                  */
#define  ORASTFNON 0                 /* = don't save text of SQL stmt       */
#define  ORASTFERR 1                 /* = only save on SQLERROR             */
#define  ORASTFWRN 2                 /* = only save on SQLWARNING/SQLERROR  */
#define  ORASTFANY 3                 /* = always save                       */
         struct
           {
  /* ub2  */ unsigned short orastxtl;
  /* text */ char  orastxtc[70];
           } orastxt;                /* text of last SQL stmt               */
         struct
           {
  /* ub2  */   unsigned short orasfnml;
  /* text */   char       orasfnmc[70];
           } orasfnm;                /* name of file containing SQL stmt    */
  /* ub4 */ int   oraslnr;           /* line nr-within-file of SQL stmt     */

  /* ub4 */ int   orahoc;            /* highest max open OraCurs requested  */
  /* ub4 */ int   oramoc;            /* max open OraCursors required        */
  /* ub4 */ int   oracoc;            /* current OraCursors open             */
  /* ub4 */ int   oranor;            /* nr of OraCursor re-assignments      */
  /* ub4 */ int   oranpr;            /* nr of parses                        */
  /* ub4 */ int   oranex;            /* nr of executes                      */
         };

#ifndef ORACA_NONE

#ifdef ORACA_STORAGE_CLASS
ORACA_STORAGE_CLASS struct oraca oraca
#else
struct oraca oraca
#endif
#ifdef ORACA_INIT
         =
         {
         {'O','R','A','C','A',' ',' ',' '},
         sizeof(struct oraca),
         0,0,0,0,
         {0,{0}},
         {0,{0}},
         0,
         0,0,0,0,0,0
         }
#endif
         ;

#endif

#endif
/* end oraca.h */




// include self defined lib

#include "CF_CEnv.h"

#include "wrlog.h"



// macro define



// declare internal function



// declare datatype



// declare global variable 







/********************************************************************

description :

      the construction method of the class CF_CEnv 

input:

   localnet_abbr - the abbr of the localnet 

   module_id - the id of the calling module

   env_name - the name of the env variable  

output

   none

return 

   none

*********************************************************************/

CF_CEnv::CF_CEnv(char *exchange_code, int module_id, char *env_name)

{

  strcpy(localcode,exchange_code);

  moduleID = module_id;

  strcpy(envName,env_name);

  memset(envValue,0,80);

} 



CF_CEnv::CF_CEnv(int module_id, char *env_name)

{

  moduleID = module_id;

  strcpy(envName,env_name);

  memset(envValue,0,80);

} 

/********************************************************************

description :

      the construction method of the class CF_CEnv 

input:

   fileName - the filename of the enviroment  

output

   none

return 

   none

*********************************************************************/

CF_CEnv::CF_CEnv(char *fileName, char *env_name)

{

  

  strcpy(envName,env_name);

  memset(envValue,0,80);

  



  // 2000-12-25 by wh

  // sprintf(buf, "/usr/net/env/%s.env", fileName.get_string()); 

  // file = buf;

  strcpy(file,fileName);

}





/********************************************************************

description :

      get the env value from env_var table 

input:

   localnet_abbr - the abbr of the localnet 

   module_id - the id of the calling module

   env_name - the name of the env variable    

output

   env variable value

return 

   SUC - success; FAIL - fail

*********************************************************************/

int CF_CEnv::loadEnvVar()

{

   /* EXEC SQL BEGIN DECLARE SECTION; */ 


   	char local[8];

   	char name[32];

   	char value[80];

   	char curTime[TIME_LEN+1];

   /* EXEC SQL END DECLARE SECTION; */ 


   long sqlcode, isam;

   

   strcpy(local,localcode);

   strcpy(name,envName);

   CF_get_current_time(curTime);



   /* EXEC SQL select var_value 

   	     into :value

   	     from PIPE_ENV

   	     where PIPE_ID = :local

   	     	and varname = :name; */ 

{
   struct sqlexd sqlstm;
   sqlstm.sqlvsn = 12;
   sqlstm.arrsiz = 3;
   sqlstm.sqladtp = &sqladt;
   sqlstm.sqltdsp = &sqltds;
   sqlstm.stmt = "select var_value into :b0  from PIPE_ENV where (PIPE_ID=:\
b1 and varname=:b2)";
   sqlstm.iters = (unsigned int  )1;
   sqlstm.offset = (unsigned int  )5;
   sqlstm.selerr = (unsigned short)1;
   sqlstm.cud = sqlcud0;
   sqlstm.sqlest = (unsigned char  *)&sqlca;
   sqlstm.sqlety = (unsigned short)256;
   sqlstm.occurs = (unsigned int  )0;
   sqlstm.sqhstv[0] = (unsigned char  *)value;
   sqlstm.sqhstl[0] = (unsigned long )80;
   sqlstm.sqhsts[0] = (         int  )0;
   sqlstm.sqindv[0] = (         short *)0;
   sqlstm.sqinds[0] = (         int  )0;
   sqlstm.sqharm[0] = (unsigned long )0;
   sqlstm.sqadto[0] = (unsigned short )0;
   sqlstm.sqtdso[0] = (unsigned short )0;
   sqlstm.sqhstv[1] = (unsigned char  *)local;
   sqlstm.sqhstl[1] = (unsigned long )8;
   sqlstm.sqhsts[1] = (         int  )0;
   sqlstm.sqindv[1] = (         short *)0;
   sqlstm.sqinds[1] = (         int  )0;
   sqlstm.sqharm[1] = (unsigned long )0;
   sqlstm.sqadto[1] = (unsigned short )0;
   sqlstm.sqtdso[1] = (unsigned short )0;
   sqlstm.sqhstv[2] = (unsigned char  *)name;
   sqlstm.sqhstl[2] = (unsigned long )32;
   sqlstm.sqhsts[2] = (         int  )0;
   sqlstm.sqindv[2] = (         short *)0;
   sqlstm.sqinds[2] = (         int  )0;
   sqlstm.sqharm[2] = (unsigned long )0;
   sqlstm.sqadto[2] = (unsigned short )0;
   sqlstm.sqtdso[2] = (unsigned short )0;
   sqlstm.sqphsv = sqlstm.sqhstv;
   sqlstm.sqphsl = sqlstm.sqhstl;
   sqlstm.sqphss = sqlstm.sqhsts;
   sqlstm.sqpind = sqlstm.sqindv;
   sqlstm.sqpins = sqlstm.sqinds;
   sqlstm.sqparm = sqlstm.sqharm;
   sqlstm.sqparc = sqlstm.sqharc;
   sqlstm.sqpadto = sqlstm.sqadto;
   sqlstm.sqptdso = sqlstm.sqtdso;
   sqlcxt((void **)0, &sqlctx, &sqlstm, &sqlfpn);
}



   	     

   	  sqlcode = sqlca.sqlcode;

      isam = sqlca.sqlerrd[1];





      if(sqlca.sqlcode){	

	wrlog(local, moduleID, (char *)"env_var", 'D', 'S', sqlcode, isam, curTime,

		(char *)"get env variable value from env_var fail!");

	return (-1);

      }	



    strcpy(envValue,value);

    // cout<<"get env variable for ("<<local<<")from database: "<<name<<"="<<CF_trim(value)<<endl;

    cout<<"get env variable for ("<<local<<")from database: "<<name<<endl;

    return 0;  

}  // loadEnvVar()



int CF_CEnv::loadEnvVarDefault()

{

   /* EXEC SQL BEGIN DECLARE SECTION; */ 


   	char name[32];

   	char value[80];

   	char curTime[TIME_LEN+1];

   /* EXEC SQL END DECLARE SECTION; */ 


   long sqlcode, isam;

   

   strcpy(name,envName);

   CF_get_current_time(curTime);



   /* EXEC SQL select varvalue 

   	     into :value

   	     from GLOBAL_ENV

   	     where varname = :name; */ 

{
   struct sqlexd sqlstm;
   sqlstm.sqlvsn = 12;
   sqlstm.arrsiz = 3;
   sqlstm.sqladtp = &sqladt;
   sqlstm.sqltdsp = &sqltds;
   sqlstm.stmt = "select varvalue into :b0  from GLOBAL_ENV where varname=:\
b1";
   sqlstm.iters = (unsigned int  )1;
   sqlstm.offset = (unsigned int  )32;
   sqlstm.selerr = (unsigned short)1;
   sqlstm.cud = sqlcud0;
   sqlstm.sqlest = (unsigned char  *)&sqlca;
   sqlstm.sqlety = (unsigned short)256;
   sqlstm.occurs = (unsigned int  )0;
   sqlstm.sqhstv[0] = (unsigned char  *)value;
   sqlstm.sqhstl[0] = (unsigned long )80;
   sqlstm.sqhsts[0] = (         int  )0;
   sqlstm.sqindv[0] = (         short *)0;
   sqlstm.sqinds[0] = (         int  )0;
   sqlstm.sqharm[0] = (unsigned long )0;
   sqlstm.sqadto[0] = (unsigned short )0;
   sqlstm.sqtdso[0] = (unsigned short )0;
   sqlstm.sqhstv[1] = (unsigned char  *)name;
   sqlstm.sqhstl[1] = (unsigned long )32;
   sqlstm.sqhsts[1] = (         int  )0;
   sqlstm.sqindv[1] = (         short *)0;
   sqlstm.sqinds[1] = (         int  )0;
   sqlstm.sqharm[1] = (unsigned long )0;
   sqlstm.sqadto[1] = (unsigned short )0;
   sqlstm.sqtdso[1] = (unsigned short )0;
   sqlstm.sqphsv = sqlstm.sqhstv;
   sqlstm.sqphsl = sqlstm.sqhstl;
   sqlstm.sqphss = sqlstm.sqhsts;
   sqlstm.sqpind = sqlstm.sqindv;
   sqlstm.sqpins = sqlstm.sqinds;
   sqlstm.sqparm = sqlstm.sqharm;
   sqlstm.sqparc = sqlstm.sqharc;
   sqlstm.sqpadto = sqlstm.sqadto;
   sqlstm.sqptdso = sqlstm.sqtdso;
   sqlcxt((void **)0, &sqlctx, &sqlstm, &sqlfpn);
}



   	     

   	  sqlcode = sqlca.sqlcode;

      isam = sqlca.sqlerrd[1];





      if(sqlca.sqlcode){	

	wrlog((char *)"_", moduleID, (char *)"env_var_default", 'D', 'S', sqlcode, isam, curTime,

		(char *)"get env variable value from env_var fail!");

	return (-1);

      }	



    strcpy(envValue,value);

    // cout<<"get env variable for ("<<local<<")from database: "<<name<<"="<<CF_trim(value)<<endl;

    cout<<"get env variable for ("<<name<<")from database: env_var_default"<<endl;

    return 0;  

}  // loadEnvVar()



/********************************************************************

description :

      get the env value from env file

input:

  none 

output

   env variable value

return 

   SUC - success; FAIL - fail

*********************************************************************/

int CF_CEnv::readEnvVarFromFile()

{

    int i;

    FILE *tmpfp;

    char ss[10][80],buf[200];

    char fileName[80];

    strcpy(fileName,file);

    if( ( tmpfp = fopen( fileName,"r" ) ) == NULL ) {

       	sprintf( buf,"open file %s Error when get env %s!",

       		fileName, envName);

       	

cerr<<"**************** errno = "<<errno<<endl;

       		

        perror(buf);

        fclose(tmpfp);

	return (-1);

    }

    

    while( fgets( buf,200,tmpfp ) != NULL ) {

       memset(ss, 0, sizeof(ss));

       

       if( buf[0] == '#' ) continue;

       sscanf( buf,"%s %s %s %s %s %s",ss[1],ss[2],ss[3],ss[4],ss[5],ss[6] );

       if( !strcmp( ss[1], envName) ) {

           strcpy(envValue,ss[2]);

           fclose(tmpfp);

           

    	   // cout<<"read env variable from ("<<fileName<<"): "<<name<<"="<<envValue<<endl;

    	   cout<<"read env variable from ("<<fileName<<")"<<endl;

           return 0;

          }

      }   // while

 

    fclose(tmpfp);

    

    return (-1);

}   // readEnvVarFromFile()



/********************************************************************

description :

      get the env value from env_var table 

input:

  none 

output

   env variable value

return 

   SUC - success; FAIL - fail

*********************************************************************/

void CF_CEnv::getenv(long &env_value)

{DeleteSpace( envValue );

   env_value = atol(envValue);

}  // getenv()



/********************************************************************

description :

      get the env value from env_var table 

input:

   none   

output

   env variable value

return 

   SUC - success; FAIL - fail

*********************************************************************/

void CF_CEnv::getenv(char *env_value)

{DeleteSpace( envValue );

    strcpy(env_value, envValue);

    

}  // getenv()



/********************************************************************

description :

      get the system current time 

input:

   none

   

output

   current time

return 

   none

*********************************************************************/

void CF_CEnv::CF_get_current_time(char* curtime)

{

  	time_t		time1;

	struct tm	*time2;

	

	time(&time1);

	time2 = localtime(&time1);	

	sprintf(curtime, "%4d%02d%02d%02d%02d%02d", time2->tm_year+1900, 

		time2->tm_mon+1, time2->tm_mday, time2->tm_hour, time2->tm_min, time2->tm_sec);



}



int CF_CEnv::DeleteSpace( char *ss )





{





 int i;





    i = strlen(ss)-1;





    while ( i && ss[i] == ' ' ) i--;





    ss[i+1] = 0;





 return(0);



}



