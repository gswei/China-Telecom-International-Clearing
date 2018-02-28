#ifndef Z_COMPRESS_H
#define Z_COMPRESS_H 1

#pragma ident	"@(#)compress.c	1.41	05/10/30 SMI"

#define	min_a(a, b)	((a > b) ? b : a)

#ifndef SACREDMEM
#define	SACREDMEM	0
#endif

#ifndef USERMEM
#define	USERMEM 	450000	/* default user memory */
#endif

#ifdef USERMEM
#if USERMEM >= (433484+SACREDMEM)
#define	PBITS	16
#else
#if USERMEM >= (229600+SACREDMEM)
#define	PBITS	15
#else
#if USERMEM >= (127536+SACREDMEM)
#define	PBITS	14
#else
#if USERMEM >= (73464+SACREDMEM)
#define	PBITS	13
#else
#define	PBITS	12
#endif
#endif
#endif
#endif
#undef USERMEM
#endif /* USERMEM */

#ifdef PBITS		/* Preferred BITS for this memory size */
#ifndef BITS
#define	BITS PBITS
#endif /* BITS */
#endif /* PBITS */

#if BITS == 16
#define	HSIZE	69001		/* 95% occupancy */
#endif
#if BITS == 15
#define	HSIZE	35023		/* 94% occupancy */
#endif
#if BITS == 14
#define	HSIZE	18013		/* 91% occupancy */
#endif
#if BITS == 13
#define	HSIZE	9001		/* 91% occupancy */
#endif
#if BITS <= 12
#define	HSIZE	5003		/* 80% occupancy */
#endif

#define	OUTSTACKSIZE	(2<<BITS)

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
#if BITS > 15
typedef long int	code_int;
#else
typedef int		code_int;
#endif

typedef long int	count_int;
typedef long long	count_long;

typedef	unsigned char	char_type;

static char_type magic_header[] = { "\037\235" }; /* 1F 9D */

/* Defines for third byte of header */
#define	BIT_MASK	0x1f
#define	BLOCK_MASK	0x80
/*
 * Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
 * a fourth header byte(for expansion).
 */
#define	INIT_BITS 9			/* initial number of bits/code */

/*
 * compress.c - File compression ala IEEE Computer, June 1984.
 */
static char rcs_ident[] =
	"$Header: compress.c,v 4.0 85/07/30 12:50:00 joe Release $";

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/param.h>
#include <stdlib.h>			/* XCU4 */
#include <limits.h>
//#include <libintl.h>
#include <locale.h>
#include <langinfo.h>
#include <string.h>
#include <sys/acl.h>
#include <utime.h>
#include <libgen.h>
#include <setjmp.h>
#include <strings.h>
#include <fcntl.h>
#include <dirent.h>
//#include <aclutils.h>
#include <sys/acl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <errno.h>

/*
 * Multi-byte handling for 'y' or 'n'
 */
static char	*yesstr;		/* string contains int'l for "yes" */
static char	*nostr;			/* string contains int'l for "yes" */
static int	ynsize = 0;		/* # of (multi)bytes for "y" */
static char	*yesorno;		/* int'l input for 'y' */

static int n_bits;			/* number of bits/code */
static int maxbits = BITS;	/* user settable max # bits/code */
static code_int maxcode;	/* maximum code, given n_bits */
			/* should NEVER generate this code */
static code_int maxmaxcode = 1 << BITS;
#define	MAXCODE(n_bits)	((1 << (n_bits)) - 1)

static count_int htab [OUTSTACKSIZE];
static unsigned short codetab [OUTSTACKSIZE];

#define	htabof(i)	htab[i]
#define	codetabof(i)	codetab[i]
static code_int hsize = HSIZE; /* for dynamic table sizing */
static off_t	fsize;	/* file size of input file */

#define	tab_prefixof(i)		codetabof(i)
#define	tab_suffixof(i)		((char_type *)(htab))[i]
#define	de_stack		((char_type *)&tab_suffixof(1<<BITS))
#define	stack_max		((char_type *)&tab_suffixof(OUTSTACKSIZE))

static code_int free_ent = 0; /* first unused entry */
static int newline_needed = 0;
static int didnt_shrink = 0;
static int perm_stat = 0;	/* permanent status */

static code_int getcode();

	/* Use a 3-byte magic number header, unless old file */
static int nomagic = 0;
	/* Write output on stdout, suppress messages */
static int zcat_flg = 0;	/* use stdout on all files */
static int zcat_cmd = 0;	/* zcat cmd */
static int use_stdout = 0;	/* set for each file processed */
	/* Don't unlink output file on interrupt */
static int precious = 1;
static int quiet = 1;	/* don't tell me about compression */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int block_compress = BLOCK_MASK;
static int clear_flg = 0;
static long int ratio = 0;
#define	CHECK_GAP 10000	/* ratio check interval */
static count_long checkpoint = CHECK_GAP;
/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */
#define	FIRST	257	/* first free entry */
#define	CLEAR	256	/* table clear output code */

static int force = 0;
static char ofname [MAXPATHLEN];

static int dflg = 0;
static int Cflg = 0;

#ifdef DEBUG
int verbose = 0;
int debug = 0;
#endif /* DEBUG */

static void (*oldint)();
static int bgnd_flag;

static int do_decomp = 0;

static char *progname;
static char *optstr;
/*
 * Fix lint errors
 */

static char *local_basename(char *);

static int  addDotZ(char *, size_t);

static void Usage(void);
static void cl_block(count_long);
static void cl_hash(count_int);
static void Zcompress(void);
static void copystat(char *, struct stat *, char *);
static int Zdecompress(void);
int Zdecompress(const char* filename);
int Zdecompress(std::vector<char*> &fileLine);
int ZInit(const char* filename,char flag);
int ZInit(const char *in_filename,const char *out_filename,char flag);
static void ioerror(void);
static void onintr();
static void oops();
static void output(code_int);
static void prratio(FILE *, count_long, count_long);
static void version(void);
//static int mv_xattrs(char *, char *, int);

#ifdef DEBUG
static int in_stack(int, int);
static void dump_tab(void);
static void printcodes(void);
#endif

/* For error-handling */

static jmp_buf env;

/* For input and ouput */

static FILE *inp;		/* the current input file */
static FILE *infile;		/* disk-based input stream */
static FILE *outp;		/* current output file */
static FILE *outfile;		/* disk-based output stream */

/* For output() */

static char buf[BITS];

static char_type lmask[9] =
	{0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
static char_type rmask[9] =
	{0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

/* For compress () */

static int offset;
static count_long bytes_out;	/* length of compressed output */
	/* # of codes output (for debugging) */

/* For dump_tab() */

#define	STACK_SIZE	15000
#ifdef DEBUG
code_int sorttab[1<<BITS];	/* sorted pointers into htab */
#endif


#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(WIN32)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

/*
#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ifdef RISCOS
#  define unlink remove
#  define GZ_SUFFIX "-gz"
#  define fileno(file) file->__file
#endif
#if defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> 
#endif

#ifndef WIN32 
  extern int unlink OF((const char *));
#endif
*/
/**********************************************************************************
定义ZCompress类
**********************************************************************************/
#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local static
   /* Needed for systems with limitation on stack size. */
#else
#  define local
#endif

#define	FILENOTEND	0

class ZCompress
{
public:	
	
	ZCompress();
	virtual ~ZCompress();
	
	virtual int Openf(const char* file,char *auth);//打开文件，获取文件指针
	virtual int GetLine(char* szOutRes);//依次读取一行
	virtual void Close();//close
	virtual void Puts(char *);//
	virtual void SeekLine(long off_set,char* szOutRes,const int till=SEEK_SET);//移动到某处读取一行
	virtual int Compress(const char *filename,const char delflag='Y');
	virtual int Compress(const char *in_filename,const char* out_filename,const char delflag='Y');
	virtual int DeCompress(const char *filename,const char* out_filename,const char delflag='N');
private:	

	void error(const char *msg);
	int ZGetLine(char* szOutRes);
	int ZOpenf(const char* file);	

  char *m_infile;
  char m_buffer[BUFLEN+2];
  int m_endFlag;
  
  FILE *mfp; 
	std::vector<char*>	vec_fileLine;
	int m_iCurLine;
	long curOffset;
	
	char m_cSfxFlag;	//='Z'表示.Z后缀的文件; ='g'表示.gz后缀的文件
};



#endif
