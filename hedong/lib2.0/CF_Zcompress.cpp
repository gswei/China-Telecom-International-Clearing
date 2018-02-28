#include "CF_Zcompress.h"
#define	min_a(a, b)	((a > b) ? b : a)		//比较大小的功能，之所以用这个名字为了区别某些系统函数min()
static void
cinterr(int hshift)
{
	/* we have exceeded the hash table */
	(void) fprintf(stderr,
		"internal error: hashtable exceeded - hsize = %ld\n", hsize);
	(void) fprintf(stderr, "hshift = %d, %d\n", hshift, (1 << hshift) -1);
	(void) fprintf(stderr, "maxbits = %d\n", maxbits);
	(void) fprintf(stderr, "n_bits = %d\n", n_bits);
	(void) fprintf(stderr, "maxcode = %ld\n", maxcode);
	longjmp(env, 1);
}

static code_int
adjusti(code_int i, code_int hsize_reg)
{
	while (i < 0) {
		i += hsize_reg;
	}

	while (i >= hsize_reg) {
		i -= hsize_reg;
	}
	return (i);
}

static void
Zcompress()
{
	long fcode;
	code_int i = 0;
	int c;
	code_int ent;
	int disp;
	code_int hsize_reg;
	int hshift;
	int probecnt;
	count_long in_count;
	unsigned long inchi, inclo;//2006-9-18（liuw） 将uint32_t改为unsigned long，以适应多平台
	int maxbits_reg;
	FILE *fin = inp;
#ifdef DEBUG
	count_long out_count = 0;
#endif

	if (nomagic == 0) {
		if ((putc(magic_header[0], outp) == EOF ||
		    putc(magic_header[1], outp) == EOF ||
		    putc((char)(maxbits | block_compress),
			outp) == EOF) &&
		    ferror(outp)) {
			ioerror();
		}
	}

	offset = 0;
	bytes_out = 3;		/* includes 3-byte header mojo */
	clear_flg = 0;
	ratio = 0;
	in_count = 1;
	inchi = 0;
	inclo = 1;
	checkpoint = CHECK_GAP;
	maxcode = MAXCODE(n_bits = INIT_BITS);
	free_ent = ((block_compress) ? FIRST : 256);

	if ((ent = getc(fin)) == EOF && ferror(fin)) {
		ioerror();
	}

	hshift = 0;

	for (fcode = (long)hsize;  fcode < 65536L; fcode *= 2L)
		hshift++;

	hshift = 8 - hshift;		/* set hash code range bound */

	hsize_reg = hsize;
	maxbits_reg = maxbits;

	cl_hash((count_int) hsize_reg);		/* clear hash table */

	while ((c = getc(fin)) != EOF) {
		if (++inclo == 0)
			inchi++;
		fcode = (long)(((long)c << maxbits_reg) + ent);
		i = ((c << hshift) ^ ent);	/* xor hashing */

		if ((unsigned int)i >= hsize_reg)
			i = adjusti(i, hsize_reg);

		if (htabof(i) == fcode) {
			ent = codetabof(i);
			continue;
		} else if ((long)htabof(i) < 0) {
			/* empty slot */
			goto nomatch;
		}

		/* secondary hash (after G. Knott) */
		disp = hsize_reg - i;

		if (i == 0) {
			disp = 1;
		}

		probecnt = 0;
	probe:
		if (++probecnt > hsize_reg)
			cinterr(hshift);

		if ((i -= disp) < 0) {
			while (i < 0)
				i += hsize_reg;
		}

		if (htabof(i) == fcode) {
			ent = codetabof(i);
			continue;
		}

		if ((long)htabof(i) > 0) {
			goto probe;
		}
	nomatch:
		output((code_int) ent);
#ifdef DEBUG
		out_count++;
#endif
		ent = c;
		if (free_ent < maxmaxcode) {
			codetabof(i) = free_ent++;
			/* code -> hashtable */
			htabof(i) = fcode;
		} else {
			in_count = ((long long)inchi<<32|inclo);
			if ((count_long)in_count >=
			    (count_long)checkpoint && block_compress) {
				cl_block(in_count);
			}
		}
	}

	in_count = ((long long)inchi<<32|inclo);

	if (ferror(fin) != 0) {
		ioerror();
	}

	/*
	 * Put out the final code.
	 */
	output((code_int)ent);
#ifdef DEBUG
	out_count++;
#endif

	output((code_int)-1);

	/*
	 * Print out stats on stderr
	 */
	if (!quiet) {
#ifdef DEBUG
		(void) fprintf(stderr,
			"%lld chars in, %lld codes (%lld bytes) out, "
			"compression factor: ",
			(count_long)in_count, (count_long)out_count,
			(count_long) bytes_out);
		prratio(stderr, (count_long)in_count,
			(count_long)bytes_out);
		(void) fprintf(stderr, "\n");
		(void) fprintf(stderr, "\tCompression as in compact: ");
		prratio(stderr,
			(count_long)in_count-(count_long)bytes_out,
			(count_long)in_count);
		(void) fprintf(stderr, "\n");
		(void) fprintf(stderr,
			"\tLargest code (of last block) was %d"
			" (%d bits)\n",
			free_ent - 1, n_bits);
#else /* !DEBUG */
		(void) fprintf(stderr, "Compression: ");
		prratio(stderr,
			(count_long)in_count-(count_long)bytes_out,
			(count_long)in_count);
#endif /* DEBUG */
	}
	/* report if no savings */
	if ((count_long)bytes_out > (count_long)in_count) {
		didnt_shrink = 1;
	}
}


static void
output(code_int code)
{
#ifdef DEBUG
	static int col = 0;
#endif /* DEBUG */

	int r_off = offset, bits = n_bits;
	char *bp = buf;

#ifdef DEBUG
	if (verbose)
		(void) fprintf(stderr, "%5d%c", code,
			(col += 6) >= 74 ? (col = 0, '\n') : ' ');
#endif /* DEBUG */
	if (code >= 0) {
		/*
		 * byte/bit numbering on the VAX is simulated
		 * by the following code
		 */
		/*
		 * Get to the first byte.
		 */
		bp += (r_off >> 3);
		r_off &= 7;
		/*
		 * Since code is always >= 8 bits, only need to mask the first
		 * hunk on the left.
		 */
		*bp = (*bp & rmask[r_off]) | (code << r_off) & lmask[r_off];
		bp++;
		bits -= (8 - r_off);
		code >>= 8 - r_off;
		/*
		 * Get any 8 bit parts in the middle (<=1 for up to 16
		 * bits).
		 */
		if (bits >= 8) {
			*bp++ = code;
			code >>= 8;
			bits -= 8;
		}
		/* Last bits. */
		if (bits)
			*bp = code;
		offset += n_bits;
		if (offset == (n_bits << 3)) {
			bp = buf;
			bits = n_bits;
			bytes_out += bits;
			do {
				if (putc(*bp, outp) == EOF &&
				    ferror(outp)) {
					ioerror();
				}
				bp++;
			} while (--bits);
			offset = 0;
		}

		/*
		 * If the next entry is going to be too big for the code size,
		 * then increase it, if possible.
		 */
		if (free_ent > maxcode || (clear_flg > 0)) {
			/*
			 * Write the whole buffer, because the input
			 * side won't discover the size increase until
			 * after it has read it.
			 */
			if (offset > 0) {
				if (fwrite(buf, 1, n_bits, outp) != n_bits) {
					longjmp(env, 3);
				}
				bytes_out += n_bits;
			}
			offset = 0;

			if (clear_flg) {
				maxcode = MAXCODE(n_bits = INIT_BITS);
				clear_flg = 0;
			} else {
				n_bits++;
				if (n_bits == maxbits)
					maxcode = maxmaxcode;
				else
					maxcode = MAXCODE(n_bits);
			}
#ifdef DEBUG
			if (debug) {
				(void) fprintf(stderr,
					"\nChange to %d bits\n", n_bits);
				col = 0;
			}
#endif /* DEBUG */
		}
	} else {
		/*
		 * At EOF, write the rest of the buffer.
		 */
		if (offset > 0) {
			if (fwrite(buf, 1, (offset + 7) / 8, outp) == 0 &&
			    ferror(outp)) {
				ioerror();
			}
			bytes_out += (offset + 7) / 8;
		}
		offset = 0;
		(void) fflush(outp);
#ifdef DEBUG
		if (verbose)
			(void) fprintf(stderr, "\n");
#endif /* DEBUG */
		if (ferror(outp))
			ioerror();
	}
}

static int
Zdecompress()							//解压到stdout，调用putchar，打印在屏幕上
{
	char_type *stackp, *stack_lim;
	int finchar;
	code_int code, oldcode, incode;
	
	maxcode = MAXCODE(n_bits = INIT_BITS);
	for (code = 255; code >= 0; code--) {
		tab_prefixof(code) = 0;
		tab_suffixof(code) = (char_type)code;
	}
	free_ent = ((block_compress) ? FIRST : 256);

	finchar = oldcode = getcode();
	if (oldcode == -1)	/* EOF already? */
		return -1;			/* Get out of here */
	putchar((char)finchar);
	stackp = de_stack;
	stack_lim = stack_max;

	while ((code = getcode()) > -1) {

		if ((code == CLEAR) && block_compress) {
			for (code = 255; code >= 0; code--)
			tab_prefixof(code) = 0;
			clear_flg = 1;
			free_ent = FIRST - 1;
			if ((code = getcode()) == -1)	/* O, untimely death! */
				break;
		}
		incode = code;
		/*
		 * Special case for KwKwK string.
		 */
		if (code >= free_ent) {
			if (stackp < stack_lim) {
				*stackp++ = (char_type) finchar;
				code = oldcode;
			} else {
				/* badness */
				longjmp(env, 2);
			}
		}

		/*
		 * Generate output characters in reverse order
		 */
		while (code >= 256) {
			if (stackp < stack_lim) {
				*stackp++ = tab_suffixof(code);
				code = tab_prefixof(code);
			} else {
				/* badness */
				longjmp(env, 2);
			}
		}
		*stackp++ = finchar = tab_suffixof(code);

		/*
		 * And put them out in forward order
		 */
		do {
			stackp--;
			//(void) putc(*stackp, cout);
			putchar(*stackp);
		} while (stackp > de_stack);


		/*
		 * Generate the new entry.
		 */
		if ((code = free_ent) < maxmaxcode) {
			tab_prefixof(code) = (unsigned short) oldcode;
			tab_suffixof(code) = (char_type) finchar;
			free_ent = code+1;
		}
		/*
		 * Remember previous code.
		 */
		oldcode = incode;
	}
	
	return -1;
}

int
Zdecompress(const char* filename)						//解压到指定文件
{
	char_type *stackp, *stack_lim;
	int finchar;
	code_int code, oldcode, incode;
  FILE* fp;
		fp=fopen(filename,"w");
	
  if(fp==NULL)
			return -1;
	
	maxcode = MAXCODE(n_bits = INIT_BITS);
	for (code = 255; code >= 0; code--) {
		tab_prefixof(code) = 0;
		tab_suffixof(code) = (char_type)code;
	}
	free_ent = ((block_compress) ? FIRST : 256);

	finchar = oldcode = getcode();
	if (oldcode == -1)	/* EOF already? */
		return -1;			/* Get out of here */
	if((fprintf( fp,"%c",finchar ))<0)
				return -1;
	stackp = de_stack;
	stack_lim = stack_max;

	while ((code = getcode()) > -1) {

		if ((code == CLEAR) && block_compress) {
			for (code = 255; code >= 0; code--)
			tab_prefixof(code) = 0;
			clear_flg = 1;
			free_ent = FIRST - 1;
			if ((code = getcode()) == -1)	/* O, untimely death! */
				break;
		}
		incode = code;
		/*
		 * Special case for KwKwK string.
		 */
		if (code >= free_ent) {
			if (stackp < stack_lim) {
				*stackp++ = (char_type) finchar;
				code = oldcode;
			} else {
				/* badness */
				longjmp(env, 2);
			}
		}

		/*
		 * Generate output characters in reverse order
		 */
		while (code >= 256) {
			if (stackp < stack_lim) {
				*stackp++ = tab_suffixof(code);
				code = tab_prefixof(code);
			} else {
				/* badness */
				longjmp(env, 2);
			}
		}
		*stackp++ = finchar = tab_suffixof(code);

		/*
		 * And put them out in forward order
		 */
		do {
			stackp--;
			//(void) putc(*stackp, cout);
			//putchar(*stackp);
			if((fprintf( fp,"%c",*stackp ))<0)
				return -1;
		} while (stackp > de_stack);
		

		/*
		 * Generate the new entry.
		 */
		if ((code = free_ent) < maxmaxcode) {
			tab_prefixof(code) = (unsigned short) oldcode;
			tab_suffixof(code) = (char_type) finchar;
			free_ent = code+1;
		}
		/*
		 * Remember previous code.
		 */
		oldcode = incode;
	}
	fclose(fp);		
	return 0;
}

int
Zdecompress(std::vector<char*> &fileLine)					//解压到一个vector中
{
	char_type *stackp, *stack_lim;
	int finchar;
	code_int code, oldcode, incode;
	char szBuf[10000];
	memset(szBuf, 0, 10000);
	
	maxcode = MAXCODE(n_bits = INIT_BITS);
	for (code = 255; code >= 0; code--) {
		tab_prefixof(code) = 0;
		tab_suffixof(code) = (char_type)code;
	}
	free_ent = ((block_compress) ? FIRST : 256);

	finchar = oldcode = getcode();
	if (oldcode == -1)	/* EOF already? */
		return -1;			/* Get out of here */
	szBuf[0] = (char)finchar;
	stackp = de_stack;
	stack_lim = stack_max;
	
	int iIndex = 1, i=0;
	while ( (code = getcode()) > -1) {
		
		if ((code == CLEAR) && block_compress) {
			for (code = 255; code >= 0; code--)
			tab_prefixof(code) = 0;
			clear_flg = 1;
			free_ent = FIRST - 1;
			if ((code = getcode()) == -1)	/* O, untimely death! */
				break;
		}
		incode = code;
		/*
		 * Special case for KwKwK string.
		 */
		if (code >= free_ent) {
			if (stackp < stack_lim) {
				*stackp++ = (char_type) finchar;
				code = oldcode;
			} else {
				/* badness */
				longjmp(env, 2);
			}
		}

		/*
		 * Generate output characters in reverse order
		 */
		while (code >= 256) {
			if (stackp < stack_lim) {
				*stackp++ = tab_suffixof(code);
				code = tab_prefixof(code);
			} else {
				/* badness */
				longjmp(env, 2);
			}
		}
		*stackp++ = finchar = tab_suffixof(code);

		/*
		 * And put them out in forward order
		 */
		do {
			/////////////////////////////
			if(szBuf[iIndex-1] == '\n')
			{				
				char *szNewLine = new char[iIndex+1];
				sprintf(szNewLine, "%s", (char*)szBuf);				
				fileLine.push_back(szNewLine);
				
				/*i++;
				printf("%d]%s", i, (char*)szBuf);*/
				
				memset(szBuf, 0, 10000);				
				iIndex = 0;
			}
			
			/////////////////////////////
			
			stackp--;
			//(void) putc(*stackp, fout);
			//(void) putc(*stackp, cout);
			szBuf[iIndex] = *stackp;
			iIndex++;
		} while (stackp > de_stack);


		/*
		 * Generate the new entry.
		 */
		if ((code = free_ent) < maxmaxcode) {
			tab_prefixof(code) = (unsigned short) oldcode;
			tab_suffixof(code) = (char_type) finchar;
			free_ent = code+1;
		}
		/*
		 * Remember previous code.
		 */
		oldcode = incode;
		
		
	}
	
//	i++;
//	printf("%d]%s", i, (char*)szBuf);
	char *szNewLine = new char[iIndex+1];
	sprintf(szNewLine, "%s", (char*)szBuf);	
	fileLine.push_back(szNewLine);
	
	fclose(inp);
	
	free(yesstr);
	free(nostr);
	free(yesorno);
	
	return 0;
}

code_int
getcode() {
	code_int code;
	static int offset = 0, size = 0;
	static char_type buf[BITS];
	int r_off, bits;
	char_type *bp = buf;

	if (clear_flg > 0 || offset >= size || free_ent > maxcode) 
	{
		
		if (free_ent > maxcode) 
		{
			n_bits++;
			if (n_bits == maxbits)
				/* won't get any bigger now */
				maxcode = maxmaxcode;
			else
				maxcode = MAXCODE(n_bits);
		}
		if (clear_flg > 0) {
			maxcode = MAXCODE(n_bits = INIT_BITS);
			clear_flg = 0;
		}
		size = fread(buf, 1, n_bits, inp);

		if (size <= 0) {
			if (feof(inp)) {
				/* end of file */
				return (-1);
			} else if (ferror(inp)) {
				ioerror();
			}
		}

		offset = 0;
		/* Round size down to integral number of codes */
		size = (size << 3) - (n_bits - 1);
	}
	r_off = offset;
	bits = n_bits;
	/*
	 * Get to the first byte.
	 */
	bp += (r_off >> 3);
	r_off &= 7;
	/* Get first part (low order bits) */
	code = (*bp++ >> r_off);
	bits -= (8 - r_off);
	r_off = 8 - r_off;		/* now, offset into code word */
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if (bits >= 8) {
		code |= *bp++ << r_off;
		r_off += 8;
		bits -= 8;
	}
	/* high order bits. */
	code |= (*bp & rmask[bits]) << r_off;
	offset += n_bits;

	return (code);
}

#ifdef DEBUG
static void
printcodes()
{
	/*
	 * Just print out codes from input file.  For debugging.
	 */
	code_int code;
	int col = 0, bits;

	bits = n_bits = INIT_BITS;
	maxcode = MAXCODE(n_bits);
	free_ent = ((block_compress) ? FIRST : 256);
	while ((code = getcode()) >= 0) {
		if ((code == CLEAR) && block_compress) {
			free_ent = FIRST - 1;
			clear_flg = 1;
		} else if (free_ent < maxmaxcode)
			free_ent++;
		if (bits != n_bits) {
			(void) fprintf(stderr, "\nChange to %d bits\n", n_bits);
			bits = n_bits;
			col = 0;
		}
		(void) fprintf(stderr, "%5d%c",
			code, (col += 6) >= 74 ? (col = 0, '\n') : ' ');
	}
	(void) putc('\n', stderr);
}

#endif /* DEBUG */

#ifdef DEBUG
static void
dump_tab()	/* dump string table */
{
	int i, first;
	int ent;
	int stack_top = STACK_SIZE;
	int c;

	if (do_decomp == 0) {	/* compressing */
		int flag = 1;

		for (i = 0; i < hsize; i++) {	/* build sort pointers */
			if ((long)htabof(i) >= 0) {
				sorttab[codetabof(i)] = i;
			}
		}
		first = block_compress ? FIRST : 256;
		for (i = first; i < free_ent; i++) {
			(void) fprintf(stderr, "%5d: \"", i);
			de_stack[--stack_top] = '\n';
			de_stack[--stack_top] = '"';
			stack_top =
				in_stack((htabof(sorttab[i]) >> maxbits) & 0xff,
					stack_top);
			for (ent = htabof(sorttab[i]) & ((1 << maxbits) -1);
				ent > 256;
				ent = htabof(sorttab[ent]) & ((1<<maxbits)-1)) {
				stack_top = in_stack(
					htabof(sorttab[ent]) >> maxbits,
					stack_top);
			}
			stack_top = in_stack(ent, stack_top);
			(void) fwrite(&de_stack[stack_top], 1,
				STACK_SIZE - stack_top, stderr);
			stack_top = STACK_SIZE;
		}
	} else if (!debug) {	/* decompressing */

		for (i = 0; i < free_ent; i++) {
			ent = i;
			c = tab_suffixof(ent);
			if (isascii(c) && isprint(c))
				(void) fprintf(stderr, "%5d: %5d/'%c'  \"",
					ent, tab_prefixof(ent), c);
			else
				(void) fprintf(stderr, "%5d: %5d/\\%03o \"",
					ent, tab_prefixof(ent), c);
			de_stack[--stack_top] = '\n';
			de_stack[--stack_top] = '"';
			for (; ent != NULL;
				ent = (ent >= FIRST ? tab_prefixof(ent) :
						NULL)) {
				stack_top = in_stack(tab_suffixof(ent),
								stack_top);
			}
			(void) fwrite(&de_stack[stack_top], 1,
				STACK_SIZE - stack_top, stderr);
			stack_top = STACK_SIZE;
		}
	}
}

#endif /* DEBUG */
#ifdef DEBUG
static int
in_stack(int c, int stack_top)
{
	if ((isascii(c) && isprint(c) && c != '\\') || c == ' ') {
		de_stack[--stack_top] = c;
	} else {
		switch (c) {
		case '\n': de_stack[--stack_top] = 'n'; break;
		case '\t': de_stack[--stack_top] = 't'; break;
		case '\b': de_stack[--stack_top] = 'b'; break;
		case '\f': de_stack[--stack_top] = 'f'; break;
		case '\r': de_stack[--stack_top] = 'r'; break;
		case '\\': de_stack[--stack_top] = '\\'; break;
		default:
			de_stack[--stack_top] = '0' + c % 8;
			de_stack[--stack_top] = '0' + (c / 8) % 8;
			de_stack[--stack_top] = '0' + c / 64;
			break;
		}
		de_stack[--stack_top] = '\\';
	}
	return (stack_top);
}

#endif /* DEBUG */
static void
ioerror()
{
	longjmp(env, 1);
}

static void
copystat(char *ifname, struct stat *ifstat, char *ofname)
{
	mode_t mode;
	struct utimbuf timep;
	int error;

	if (fclose(outp)) {
		perror(ofname);
		if (!quiet) {
			(void) fprintf(stderr, " -- file unchanged");
			newline_needed = 1;
		}
		perm_stat = 1;
	} else if (ifstat == NULL) {	/* Get stat on input file */
		perror(ifname);
		return;
	} else if ((ifstat->st_mode &
			S_IFMT /* 0170000 */) != S_IFREG /* 0100000 */) {
		if (quiet) {
			(void) fprintf(stderr, "%s: ", ifname);
		}
		(void) fprintf(stderr, 
			" -- not a regular file: unchanged");
		newline_needed = 1;
		perm_stat = 1;
	} else if (ifstat->st_nlink > 1) {
		if (quiet) {
			(void) fprintf(stderr, "%s: ", ifname);
		}
		(void) fprintf(stderr, 
			" -- has %d other links: unchanged",
			(unsigned char)ifstat->st_nlink - 1);//2006-9-18（liuw） 将uint8_t改为unsigned char，以适应多平台
		newline_needed = 1;
		perm_stat = 1;
	} else if (didnt_shrink && !force) {
		/* No compression: remove file.Z */
		if (!quiet) {
			(void) fprintf(stderr, 
				" -- file unchanged");
			newline_needed = 1;
		}
	}else {	/* ***** Successful Compression ***** */
		mode = ifstat->st_mode & 07777;
		if (chmod(ofname, mode))	 /* Copy modes */
			perror(ofname);

		/* Copy ownership */
		(void) chown(ofname, ifstat->st_uid, ifstat->st_gid);
		timep.actime = ifstat->st_atime;
		timep.modtime = ifstat->st_mtime;
		/* Update last accessed and modified times */
		(void) utime(ofname, &timep);
		if (!quiet) {
			(void) fprintf(stderr, 
				" -- replaced with %s", ofname);
			newline_needed = 1;
		}
		return;		/* Successful return */
	}

	/* Unsuccessful return -- one of the tests failed */
	if (ofname[0] != '\0') {
		if (unlink(ofname)) {
			perror(ofname);
		}

		ofname[0] = '\0';
	}
}

static void
onintr()
{
	if (!precious && !use_stdout && ofname[0] != '\0')
		(void) unlink(ofname);
	exit(1);
}

static void
oops()	/* wild pointer -- assume bad input */
{
	if (do_decomp) {
		(void) fprintf(stderr, "uncompress: corrupt input\n");
	}

	if (!use_stdout && ofname[0] != '\0') {
		(void) unlink(ofname);
	}

	exit(1);
}

static void
cl_block(count_long in_count)	/* table clear for block compress */
{
	count_long rat;

	checkpoint = (count_long)in_count + (count_long)CHECK_GAP;
#ifdef DEBUG
	if (debug) {
		(void) fprintf(stderr, "count: %lld, ratio: ",
			(count_long)in_count);
		prratio(stderr, (count_long)in_count, (count_long)bytes_out);
		(void) fprintf(stderr, "\n");
	}
#endif /* DEBUG */

	/* shift will overflow */
	if ((count_long)in_count > 0x007fffffffffffffLL) {
		rat = (count_long)bytes_out >> 8;
		if (rat == 0) {		/* Don't divide by zero */
			rat = 0x7fffffffffffffffLL;
		} else {
			rat = (count_long)in_count / (count_long)rat;
		}
	} else {
		/* 8 fractional bits */
		rat = ((count_long)in_count << 8) /(count_long)bytes_out;
	}
	if (rat > ratio) {
		ratio = rat;
	} else {
		ratio = 0;
#ifdef DEBUG
		if (verbose)
			dump_tab();	/* dump string table */
#endif
		cl_hash((count_int) hsize);
		free_ent = FIRST;
		clear_flg = 1;
		output((code_int) CLEAR);
#ifdef DEBUG
		if (debug)
			(void) fprintf(stderr, "clear\n");
#endif /* DEBUG */
	}
}

static void
cl_hash(count_int hsize)		/* reset code table */
{
	count_int *htab_p = htab+hsize;
	long i;
	long m1 = -1;

	i = hsize - 16;
	do {				/* might use Sys V memset(3) here */
		*(htab_p-16) = m1;
		*(htab_p-15) = m1;
		*(htab_p-14) = m1;
		*(htab_p-13) = m1;
		*(htab_p-12) = m1;
		*(htab_p-11) = m1;
		*(htab_p-10) = m1;
		*(htab_p-9) = m1;
		*(htab_p-8) = m1;
		*(htab_p-7) = m1;
		*(htab_p-6) = m1;
		*(htab_p-5) = m1;
		*(htab_p-4) = m1;
		*(htab_p-3) = m1;
		*(htab_p-2) = m1;
		*(htab_p-1) = m1;
		htab_p -= 16;
	} while ((i -= 16) >= 0);
		for (i += 16; i > 0; i--)
			*--htab_p = m1;
}

static void
prratio(FILE *stream, count_long num, count_long den)
{
	int q;  /* store percentage */

	q = (int)(10000LL * (count_long)num / (count_long)den);
	if (q < 0) {
		(void) putc('-', stream);
		q = -q;
	}
	(void) fprintf(stream, "%d%s%02d%%", q / 100,
			localeconv()->decimal_point, q % 100);
}

static void
version()
{
	(void) fprintf(stderr, "%s, Berkeley 5.9 5/11/86\n", rcs_ident);
	(void) fprintf(stderr, "Options: ");
#ifdef DEBUG
	(void) fprintf(stderr, "DEBUG, ");
#endif
	(void) fprintf(stderr, "BITS = %d\n", BITS);
}

static void
Usage()
{
	(void) fprintf(stderr,
	"Usage: Init(char* filename,char flag),filename->文件名，flag：D->uncompress,C-compress");
}

static char *
local_basename(char *path)
{
	char *p;
	char *ret = (char *)path;

	while ((p = (char *)strpbrk(ret, "/")) != NULL)
		ret = p + 1;
	return (ret);
}

static int
addDotZ(char *fn, size_t fnsize)
{
	char *fn_dup;
	char *dir;
	long int max_name;
	long int max_path;

	fn_dup = strdup(fn);
	dir = dirname(fn_dup);
	max_name = pathconf(dir, _PC_NAME_MAX);
	max_path = pathconf(dir, _PC_PATH_MAX);
	free(fn_dup);

	/* Check for component length too long */
	if ((strlen(local_basename(fn)) + 2) > (size_t)max_name) {
		(void) fprintf(stderr,
			"%s: filename too long to tack on .Z:"
				" %s\n", progname, fn);
		return (-1);
	}

	/* Check for path length too long */

	if ((strlen(fn) + 2) > (size_t)max_path - 1) {
		(void) fprintf(stderr,
			"%s: Pathname too long to tack on .Z:"
				" %s\n", progname, fn);
		return (-1);
	}

	if (strlen(strncat(fn, ".Z", fnsize)) >= fnsize) {
		(void) fprintf(stderr,
			"%s: Buffer overflow adding .Z to %s\n",
				progname, fn);
		return (-1);
	}

	return (0);
}

int ZInit(const char* filename,char flag)
{
	char tempname[MAXPATHLEN];
	char line[LINE_MAX];
	const char **fileptr;
	char *cp;
	struct stat statbuf;
	struct stat ostatbuf;
	int ch;				/* XCU4 */
	char	*p, *yptr, *nptr;
	extern int optind, optopt;
	extern char *optarg;
	int dash_count = 0;		/* times "-" is on cmdline */
	do_decomp = 0;
	
	/* XCU4 changes */
	(void) setlocale(LC_ALL, "");
#if !defined(TEXT_DOMAIN)	/* Should be defined by cc -D */
#define	TEXT_DOMAIN "SYS_TEST"	/* Use this only if it weren't */
#endif
//	(void) textdomain(TEXT_DOMAIN);
	/* Build multi-byte char for 'y' char */
	if ((yptr = nl_langinfo(YESSTR)) == NULL)
		yptr = "y";

	yesstr = (char *)malloc(strlen(yptr) + 1);
	(void) strcpy(yesstr, yptr);
	/* Build multi-byte char for 'n' char */
	if ((nptr = nl_langinfo(NOSTR)) == NULL)
		nptr = "n";

	nostr = (char *)malloc(strlen(nptr) + 1);
	(void) strcpy(nostr, nptr);

	/* Build multi-byte char for input char */
	yesorno = (char *)malloc((size_t)ynsize + 1);
	ynsize = mblen(yesstr, strlen(yesstr));

	fileptr = &filename;

		switch (flag) {
			case 'D':
				do_decomp = 1;
				dflg++;
				break;
			case 'C':
				Cflg++;
				block_compress = 0;
				break;
			default:
				(void) fprintf(stderr, 
					"Unknown flag: '%c'\n", flag);
				Usage();
				exit(1);
		}

	if (maxbits < INIT_BITS)
		maxbits = INIT_BITS;
	if (maxbits > BITS)
		maxbits = BITS;
	maxmaxcode = 1 << maxbits;

	/* Need to open something to close with freopen later */

	if ((infile = fopen("/dev/null", "r")) == NULL) {
		(void) fprintf(stderr, "Error opening /dev/null for "
			"input\n");
		exit(1);
	}

	if ((outfile = fopen("/dev/null", "w")) == NULL) {
		(void) fprintf(stderr, "Error opening /dev/null for "
			"output\n error=%d\n",errno);
			perror("errno");
		exit(1);
	}
	int jmpval = 0;


		if (do_decomp) {
				/* DECOMPRESSION */

				/* process the named file */

				inp = infile;
				outp = outfile;
				use_stdout = 0;

				/* Check for .Z suffix */

				if (strcmp(*fileptr +
				    strlen(*fileptr) - 2, ".Z") != 0) {
					/* No .Z: tack one on */

					if (strlen(strncpy(tempname, *fileptr,
						sizeof (tempname))) >=
						sizeof (tempname)) {
						(void) fprintf(stderr,
						    "%s: filename "
							"too long\n",
							*fileptr);
						perm_stat = 1;
						return -1;
					}

					if (addDotZ(tempname,
					    sizeof (tempname)) < 0) {
						perm_stat = 1;
						return -1;
					}

					*fileptr = tempname;
				}

				/* Open input file */

				if (stat(*fileptr, &statbuf) < 0) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				if ((freopen(*fileptr, "r", inp)) == NULL) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}
			

			/* Check the magic number */

			if (nomagic == 0) {
				if ((getc(inp) !=
				    (magic_header[0] & 0xFF)) ||
				    (getc(inp) !=
				    (magic_header[1] & 0xFF))) {
					(void) fprintf(stderr, 
						"%s: not in compressed "
						"format\n",
						*fileptr);
					perm_stat = 1;
					return -1;
				}

				/* set -b from file */
				if ((maxbits = getc(inp)) == EOF &&
				    ferror(inp)) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				block_compress = maxbits & BLOCK_MASK;
				maxbits &= BIT_MASK;
				maxmaxcode = 1 << maxbits;

				if (maxbits > BITS) {
					(void) fprintf(stderr,
						"%s: compressed "
							"with %d bits, "
							"can only handle"
							" %d bits\n",
						*fileptr, maxbits, BITS);
					perm_stat = 1;
					return -1;
				}
			}

		} else {
			/* COMPRESSION */

				/* process the named file */

				inp = infile;
				outp = outfile;
				use_stdout = 0;

				if (strcmp(*fileptr +
				    strlen(*fileptr) - 2, ".Z") == 0) {
					(void) fprintf(stderr, 
						"%s: already has .Z "
						"suffix -- no change\n",
						*fileptr);
					perm_stat = 1;
					return -1;
				}
				/* Open input file */

				if (stat(*fileptr, &statbuf) < 0) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				if ((freopen(*fileptr, "r", inp)) == NULL) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				fsize = (off_t)statbuf.st_size;

				/*
				 * tune hash table size for small
				 * files -- ad hoc,
				 * but the sizes match earlier #defines, which
				 * serve as upper bounds on the number of
				 * output codes.
				 */
				hsize = HSIZE;
				if (fsize < (1 << 12))
					hsize = min_a(5003, HSIZE);
				else if (fsize < (1 << 13))
					hsize = min_a(9001, HSIZE);
				else if (fsize < (1 << 14))
					hsize = min_a(18013, HSIZE);
				else if (fsize < (1 << 15))
					hsize = min_a(35023, HSIZE);
				else if (fsize < 47000)
					hsize = min_a(50021, HSIZE);

				if (!use_stdout) {
					/* Generate output filename */

					if (strlen(strncpy(ofname, *fileptr,
						sizeof (ofname))) >=
						sizeof (ofname)) {
						(void) fprintf(stderr,
						    "%s: filename "
							"too long\n",
							*fileptr);
						perm_stat = 1;
						return -1;
					}

					if (addDotZ(ofname,
						sizeof (ofname)) < 0) {
						perm_stat = 1;
						return -1;
					}
				}
		}	/* if (do_decomp) */

		if ((!use_stdout)&&(!do_decomp)) {
			/* Open output file */
			if (freopen(ofname, "w", outp) == NULL) {
				perror(ofname);
				perm_stat = 1;
				return -1;
			}
		}

		/* Actually do the compression/decompression */

		if ((jmpval = setjmp(env)) == 0) {

			if (do_decomp == 0)  {
				Zcompress();
				stat(ofname,&statbuf);
				fsize = (off_t)statbuf.st_size;
				fclose(outfile);
				fclose(outp);
				return fsize;
			} 
		} else {
			/*
			 * Things went badly - clean up and go on.
			 * jmpval's values break down as follows:
			 *   1 == message determined by ferror() values.
			 *   2 == input problem message needed.
			 *   3 == output problem message needed.
			 */
			return -1;
		}
		return 0;
}


int ZInit(const char *in_filename,const char *out_filename,char flag)
{
	char tempname[MAXPATHLEN];
	char line[LINE_MAX];
	const char **fileptr;
	char *cp;
	struct stat statbuf;
	struct stat ostatbuf;
	int ch;				/* XCU4 */
	char	*p, *yptr, *nptr;
	extern int optind, optopt;
	extern char *optarg;
	int dash_count = 0;		/* times "-" is on cmdline */
	do_decomp = 0;
	
	/* XCU4 changes */
	(void) setlocale(LC_ALL, "");
#if !defined(TEXT_DOMAIN)	/* Should be defined by cc -D */
#define	TEXT_DOMAIN "SYS_TEST"	/* Use this only if it weren't */
#endif
//	(void) textdomain(TEXT_DOMAIN);
	/* Build multi-byte char for 'y' char */
	if ((yptr = nl_langinfo(YESSTR)) == NULL)
		yptr = "y";

	yesstr = (char *)malloc(strlen(yptr) + 1);
	(void) strcpy(yesstr, yptr);
	/* Build multi-byte char for 'n' char */
	if ((nptr = nl_langinfo(NOSTR)) == NULL)
		nptr = "n";

	nostr = (char *)malloc(strlen(nptr) + 1);
	(void) strcpy(nostr, nptr);

	/* Build multi-byte char for input char */
	yesorno = (char *)malloc((size_t)ynsize + 1);
	ynsize = mblen(yesstr, strlen(yesstr));

	fileptr = &in_filename;

		switch (flag) {
			case 'D':
				do_decomp = 1;
				dflg++;
				break;
			case 'C':
				Cflg++;
				block_compress = 0;
				break;
			default:
				(void) fprintf(stderr, 
					"Unknown flag: '%c'\n", flag);
				Usage();
				exit(1);
		}

	if (maxbits < INIT_BITS)
		maxbits = INIT_BITS;
	if (maxbits > BITS)
		maxbits = BITS;
	maxmaxcode = 1 << maxbits;

	/* Need to open something to close with freopen later */

	if ((infile = fopen("/dev/null", "r")) == NULL) {
		(void) fprintf(stderr, "Error opening /dev/null for "
			"input\n");
		exit(1);
	}

	if ((outfile = fopen("/dev/null", "w")) == NULL) {
		(void) fprintf(stderr, "Error opening /dev/null for "
			"output\n error=%d\n",errno);
			perror("errno");
		exit(1);
	}
	int jmpval = 0;


		if (do_decomp) {
				/* DECOMPRESSION */

				/* process the named file */

				inp = infile;
				outp = outfile;
				use_stdout = 0;

				/* Check for .Z suffix */

				if (strcmp(*fileptr +
				    strlen(*fileptr) - 2, ".Z") != 0) {
					/* No .Z: tack one on */

					if (strlen(strncpy(tempname, *fileptr,
						sizeof (tempname))) >=
						sizeof (tempname)) {
						(void) fprintf(stderr,
						    "%s: filename "
							"too long\n",
							*fileptr);
						perm_stat = 1;
						return -1;
					}

					if (addDotZ(tempname,
					    sizeof (tempname)) < 0) {
						perm_stat = 1;
						return -1;
					}

					*fileptr = tempname;
				}

				/* Open input file */

				if (stat(*fileptr, &statbuf) < 0) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				if ((freopen(*fileptr, "r", inp)) == NULL) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}
			

			/* Check the magic number */

			if (nomagic == 0) {
				if ((getc(inp) !=
				    (magic_header[0] & 0xFF)) ||
				    (getc(inp) !=
				    (magic_header[1] & 0xFF))) {
					(void) fprintf(stderr, 
						"%s: not in compressed "
						"format\n",
						*fileptr);
					perm_stat = 1;
					return -1;
				}

				/* set -b from file */
				if ((maxbits = getc(inp)) == EOF &&
				    ferror(inp)) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				block_compress = maxbits & BLOCK_MASK;
				maxbits &= BIT_MASK;
				maxmaxcode = 1 << maxbits;

				if (maxbits > BITS) {
					(void) fprintf(stderr,
						"%s: compressed "
							"with %d bits, "
							"can only handle"
							" %d bits\n",
						*fileptr, maxbits, BITS);
					perm_stat = 1;
					return -1;
				}
			}

		} else {
			/* COMPRESSION */

				/* process the named file */

				inp = infile;
				outp = outfile;
				use_stdout = 0;

				if (strcmp(*fileptr +
				    strlen(*fileptr) - 2, ".Z") == 0) {
					(void) fprintf(stderr, 
						"%s: already has .Z "
						"suffix -- no change\n",
						*fileptr);
					perm_stat = 1;
					return -1;
				}
				/* Open input file */

				if (stat(*fileptr, &statbuf) < 0) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				if ((freopen(*fileptr, "r", inp)) == NULL) {
					perror(*fileptr);
					perm_stat = 1;
					return -1;
				}

				fsize = (off_t)statbuf.st_size;

				/*
				 * tune hash table size for small
				 * files -- ad hoc,
				 * but the sizes match earlier #defines, which
				 * serve as upper bounds on the number of
				 * output codes.
				 */
				hsize = HSIZE;
				if (fsize < (1 << 12))
					hsize = min_a(5003, HSIZE);
				else if (fsize < (1 << 13))
					hsize = min_a(9001, HSIZE);
				else if (fsize < (1 << 14))
					hsize = min_a(18013, HSIZE);
				else if (fsize < (1 << 15))
					hsize = min_a(35023, HSIZE);
				else if (fsize < 47000)
					hsize = min_a(50021, HSIZE);

				if (!use_stdout) {
					/* Generate output filename */

					if (strlen(strncpy(ofname, (char*)out_filename,
						sizeof (ofname))) >=
						sizeof (ofname)) {
						(void) fprintf(stderr,
						    "%s: filename "
							"too long\n",
							*out_filename);
						perm_stat = 1;
						return -1;
					}

					if (addDotZ(ofname,
						sizeof (ofname)) < 0) {
						perm_stat = 1;
						return -1;
					}
				}
		}	/* if (do_decomp) */

		if ((!use_stdout)&&(!do_decomp)) {
			/* Open output file */
			if (freopen(ofname, "w", outp) == NULL) {
				perror(ofname);
				perm_stat = 1;
				return -1;
			}
		}

		/* Actually do the compression/decompression */

		if ((jmpval = setjmp(env)) == 0) {

			if (do_decomp == 0)  {
				Zcompress();
				stat(ofname,&statbuf);
				fsize = (off_t)statbuf.st_size;
				fclose(outfile);
				fclose(outp);
				return fsize;
			} 
		} else {
			/*
			 * Things went badly - clean up and go on.
			 * jmpval's values break down as follows:
			 *   1 == message determined by ferror() values.
			 *   2 == input problem message needed.
			 *   3 == output problem message needed.
			 */
			return -1;
		}
		return 0;
}
/*
int
main(int argc, char *argv[])
{
	unsigned char* szBuffer;//[1001];
	szBuffer = new unsigned char[20000];
	int ret = Init(argv[1]);
	if(ret != 0)
		exit(-1);
	int i = 0;
	while(1)
	{
		i++;
		memset(szBuffer, 0, 20000);
		ret = decompress(szBuffer);
	//	ret = decompress();
	//	printf("%s", (char*)szBuffer);
	//	getchar();
		if(ret == -1)
			break;		
	}
	printf("\n");
	
	delete[] szBuffer;
}*/

/***************************************************************************
Zcompress类成员函数定义：ZCompress，~ZCompress构造和析构函数
												ZGetline，ZOpenf，error为私有成员函数，分别为读取一行，
																							打开文件，输出错误信息。
												Getline（读取一行），Openf（打开文件），Compress（压缩）为公有成员函数
***************************************************************************/
ZCompress::ZCompress()
{
	//memset(m_infile, 0, MAX_NAME_LEN);
		m_infile = NULL;
//    m_in = NULL;
    memset(m_buffer, 0, BUFLEN);
    m_endFlag = FILENOTEND;
    
    vec_fileLine.reserve(30000);
		m_iCurLine = 0;
		m_cSfxFlag = 0;
}

ZCompress::~ZCompress()
{
	//if(m_in != NULL)
		//if (gzclose(m_in) != Z_OK) error("failed gzclose");		
	fclose(inp);
	fclose(outp);
	fclose(infile);
	fclose(outfile);
	vec_fileLine.clear();
}



int ZCompress::ZGetLine(char* szOutRes)
{
	if(m_iCurLine == vec_fileLine.size())
		return -1;
	strcpy(szOutRes, vec_fileLine[m_iCurLine]);
	delete[] vec_fileLine[m_iCurLine];
	m_iCurLine++;
	return 0;
}

int ZCompress::GetLine(char* szOutRes)
{
	char tmpRes[1000];
	if(m_cSfxFlag == 'Z')
		return ZGetLine(szOutRes);
	else if(m_cSfxFlag == 'N')
		if(NULL==fgets(tmpRes,sizeof(tmpRes),mfp))
			return -1;
	else;
	strcpy(szOutRes,tmpRes);
	return 0;	
}


void ZCompress::SeekLine(long off_set,char* szOutRes,int till)//移动到某处读取一行
{
	if(m_cSfxFlag == 'Z')
	{
		for(int il=m_iCurLine;il<vec_fileLine.size();il++)
		{
			if(curOffset>off_set) break;//
			curOffset+=strlen(vec_fileLine[m_iCurLine]);
			if(curOffset==off_set)
			{
				m_iCurLine++;
				strcpy(szOutRes, vec_fileLine[m_iCurLine]);
				break;
			}
			else
				m_iCurLine++;
		}
	}
	else if(m_cSfxFlag == 'N')
	{
		fseek(mfp, off_set, till);
		fgets(szOutRes,1000,mfp);
	}	
	else;
}

void ZCompress::Puts(char *szLine)
{
	fputs(szLine, mfp);
}


void ZCompress::error(const char *msg)
{
    fprintf(stderr, " %s\n", msg);
    exit(1);
}

int ZCompress::ZOpenf(const char* szFileName)
{
	int ret = ZInit(szFileName,'D');
	if(ret != 0)
		return NULL;
	Zdecompress(vec_fileLine);
	fclose(inp);
	fclose(infile);
	return 0;
}

int ZCompress::Openf(const char* file,char *auth)
{
	const char*szSuffix = strrchr(file, '.');
	if(strcmp(szSuffix, ".Z") == 0)
	{
		m_cSfxFlag = 'Z';//标准.Z文件
		ZOpenf(file);
		curOffset=0;
		return 0;
	}
	else
	{
		m_cSfxFlag='N';//正常文件
		mfp=fopen(file,auth);
		return 0;
	}
	return -1;
}

void ZCompress::Close()
{
	if(m_cSfxFlag == 'Z')
	{
		fclose(inp);
		fclose(outp);
		fclose(infile);
		fclose(outfile);
		vec_fileLine.clear();
	}
	else if(m_cSfxFlag=='N')
	{
		fclose(mfp);
	}
	else;
}


int ZCompress::Compress(const char *filename,const char delflag)
{
	int filesize = ZInit(filename,'C');
	if(delflag=='Y')//delflag为删除标记，标识是否删除源文件,默认删除。
	unlink(filename);
	
	return filesize;
}

int ZCompress::Compress(const char *in_filename,const char* out_filename,const char delflag)
{
	int filesize = ZInit(in_filename,out_filename,'C');
	
	if(delflag=='Y')//delflag为删除标记，标识是否删除源文件,默认删除。
	unlink(in_filename);
	
	return filesize;
}

int ZCompress::DeCompress(const char *filename,const char* out_filename,const char delflag)
{
	struct stat statbuf;
	stat(filename, &statbuf);
	int ret=ZInit(filename,'D');
	if(ret != 0)
		return -1;
	ret=Zdecompress(out_filename);
	if(ret==-1)
		{
		if(statbuf.st_size==3)//判斷0字節文件時壓縮后為3字節的情況
			{
				fclose(inp);
				fclose(infile);
				if(delflag=='Y')//delflag为删除标记，标识是否删除源文件,默认删除。
				unlink(filename);
	
				return 0;
			}
		printf("error");
	return -1;}
	fclose(inp);
	fclose(infile);
	if(delflag=='Y')//delflag为删除标记，标识是否删除源文件,默认删除。
	unlink(filename);
	
	return 0;
}