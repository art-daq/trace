/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    */
#define TRACE_CNTL_REV "$Revision: 1194 $$Date: 2019-09-27 10:32:52 -0500 (Fri, 27 Sep 2019) $"
/*
NOTE: This is a .c file instead of c++ mainly because C is friendlier when it
      comes to extended initializer lists.
gxx_standards.sh trace_cntl.c
for std in $standards;do\
  for ll in 0 1 2 3;do echo $std $ll:;TRACE_LVL=$ll ./Trace_test-$std;done;\
done

*/
#include <stdio.h>		/* printf */
#ifndef __USE_MISC
# define __USE_MISC
#endif
#include <sched.h>
#ifndef __USE_BSD
# define __USE_BSD
#endif
#include <stdlib.h>		/* setenv */
#include <stdint.h>		/* uint64_t */
#include <libgen.h>		/* basename */
#include <unistd.h>		/* getopt */
#include <getopt.h>		/* getopt */
#include <sys/time.h>           /* gettimeofday, struct timeval */
#include <pthread.h>		/* pthread_self */
#include <locale.h>				/* setlocale */
#include <string.h>				/* strnlen */

#include "TRACE/trace.h"
//#define TRACE_NAME basename(__FILE__)  // don't define TRACE_NAME, so ton* (w/o -n or -N) works on default TRACE_NAME "TRACE"

#define USAGE "\
%s [opts] <cmd> [command opt/args]\n\
commands:\n\
 info, tids, reset\n\
 show [opts] [file]...         # Note: -s option invalid with multiple files\n\
 mode <mode>\n\
 modeM <mode>\n\
 modeS <mode>\n\
 lvlmsk[M|S|T][g] <lvlmsk>              # M=memory, S=slow/console, T=trig\n\
 lvlmsk[g]  <mskM> <mskS> <mskT>\n\
 lvlset[g] <mskM> <mskS> <mskT>\n\
 lvlclr[g] <mskM> <mskS> <mskT>\n\
 trig <postEntries> [lvlmskTM+] [quiet] # opt 2nd arg is for specified/dflt TID\n\
 reset\n\
 limit_ms <cnt> <on_ms> <off_ms>\n\
 sleep [seconds]     # dlft:10 - allows time to look at /proc/fd/ and /proc/maps\n\
opts:\n\
 -?, -h       show this help\n\
 -f<file>\n\
 -n<name>     name. For all commands - not wildcard, created if non-existent\n\
 -N<name>     name. For lvl*, could be wildcard match spec.\n\
 -V           print version and exit\n\
 -a           for tids, show all\n\
show opts:\n\
 -H           no header\n\
 -L<LC_NUMERIC val>  i.e. -Len_US\n\
 -q           silence format errors\n\
 -F           show forward, poll for new entries. \"count\" (each files) avoids older entries\n\
 -c<count>    \n\
 -s<startSlotIndex> \n\
 other options encoded in TRACE_SHOW env.var.\n\
tests:  (use %s show after test)\n\
 -x<thread_options_mask>    b0=TRACE_CNTL\"file\", b1=TRACE_CNTL\"name\", b2=count mappings\n\
 -l<loops>\n\
\n\
" USAGE_TESTS, basename(argv[0]), basename(argv[0]), DFLT_TEST_COMPARE_ITERS, /*USAGE_TESTS PARAMS*/ NUMTHREADS
#define USAGE_TESTS "\
 test1 [-lloops]  a single TRACE lvl 2 [or more if loops != 0]\n\
 test           various\n\
 test-ro        test first page of mmap read-only (for kernel module)\n\
 test-compare [-lloops]  compare TRACE fmt+args vs. format+args converted (via sprintf). dflt loops=%d\n\
 test-threads [-x<thread_opts>] [-lloops] [-ddly_ms] [num_threads]  Tests threading. loops\n\
                         of TRACEs: 2 lvl2's, lvl3, lvl4, lvl5  dflts: -x0 -l2 num_threads=%d\n\
 TRACE <lvl> <fmt> [ulong]...   (just ulong args are supported)\n\
"
#define DFLT_TEST_COMPARE_ITERS 1000000
#define minw(a,b) (b<a?a:b)     /* min width -- really max of the 2 numbers - argh */

#ifdef __linux__
//# define DFLT_SHOW         "HxNTPiCnLR"
# define DFLT_SHOW         "%H%x%N %T %P %i %C %n %L %R %m"
#else
//# define DFLT_SHOW         "HxNTPinLR"
# define DFLT_SHOW         "%H%x%N %T %P %i %n %L %R %m"
#endif
#define NUMTHREADS 4
static int trace_thread_option=0;

typedef struct {
	unsigned tidx;
	int      loops;
	unsigned dly_ms;
	unsigned burst;
} args_t;

void* thread_func(void *arg)
{
	args_t *argsp=(args_t*)arg;
	unsigned tidx  =argsp->tidx;
	unsigned dly_ms=argsp->dly_ms;
	unsigned burst =argsp->burst;
	int      loops =argsp->loops;
	int      lp;
	char tmp[PATH_MAX];
	pid_t tid=trace_gettid();

	if (trace_thread_option & 1) {
		/* IF -std=c11 is NOT used, a seg fault usually occurs if default file does not exit */
		snprintf( tmp, sizeof(tmp),"/tmp/trace_buffer_%u",tidx );
		TRACE_CNTL( "file", tmp );
	}
	if (trace_thread_option & 2) {
		snprintf( tmp, sizeof(tmp), "TEST_THR_%u", tidx ); /* a name per thread...*/
		/*...-- may fill up namLvlTbl (w/ "larger" number of threads), but that should be OK */
		TRACE_CNTL( "name", tmp );
	} else
		strcat( tmp, "TEST_THR" );

	for (lp=0; lp<loops; ) {
		/* NOTE: be mindful of classic issue with macro -- args could be evaluted multiple times! */
		/* BUT -- when TRACE-ing a value, there you would not likely operate on it */
		++lp;
		TRACE( 2, "tidx=%u loop=%d of %d tid=%d I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message mangling", tidx, lp, loops, tid );
		TRACE( 2, "tidx=%u loop=%d of %d tid=%d this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9", tidx, lp, loops, tid );
		TRACE( 3, "tidx=%u loop=%d of %d tid=%d this is the third long message - third0 third1 third2 third3 third4 third5 third6 third7 third8 third9", tidx, lp, loops, tid );
		TRACE( 4, "tidx=%u loop=%d of %d tid=%d this is the fourth long message - fourth0 fourth1 fourth2 fourth3 fourth4 fourth5 fourth6 fourth7 fourth8 fourth9", tidx, lp, loops, tid );
		TRACEN(tmp,5,"tidx=%u loop=%d of %d tid=%d - extra, if enabled", tidx, lp, loops, tid );
		if(dly_ms && (lp%burst)==0 && loops)
			usleep( dly_ms*1000 );
	}
	if (argsp->tidx)
		pthread_exit(NULL);
	else
		return (NULL);
}


uint64_t get_us_timeofday()
{	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (uint64_t)tv.tv_sec*1000000+tv.tv_usec;
}

//                  32bit      64bit
//                --------   --------
//    char         1 byte     1 bytes   * always pushed as long - 4 on 32, 8 on 64
//    short        2 bytes    2 bytes   * always pushed as long - 4 on 32, 8 on 64
//    int          4 bytes    4 bytes 
//    float        4 bytes    4 bytes   * always pushed as double/8 on both 32 and 64
//    long         4 bytes    8 bytes
//    void *       4 bytes    8 bytes
//    double       8 bytes    8 bytes
//    long long    8 bytes    8 bytes
//    long double 12 bytes   16 bytes   * 80 bit extended precision in 96; 128 bits
//
// used to direct the copying of the trace entry parameters block as a part of "show"
struct sizepush
{	unsigned size:16;    // size (number of bytes) an arg is when it's not pushed as determined by a combination of the traced fmt and fundamental param size of the executed TRACE statement (myEnt_p->param_bytes)
	unsigned push:16;    // size (number of bytes) an arg was pushed onto stack and copied into the trace entry parameters block
	unsigned positive;       // 1=%d%p 0=%u
};// For example, '%hd' w/param_bytes=4  size=2, push=4
//                '%hd' w/param_bytes=8  size=2, push=8
// usually args are ints size=sizeof(int) and pushed as long (the fundamental
// size of the arch addresses) push=sizeof(long) (which is usually sizeof (void*))

enum show_opts_e {
	filter_newline_=0x1,
	quiet_         =0x2,
	indent_        =0x4,
    forward_       =0x8
};

size_t get_arg_sizes(  char            *ofmt
                     , char            *ifmt
                     , int              opts
                     , int              param_bytes
                     , struct sizepush *sizes_out
                     , unsigned         nargs )
{	char    *in;
	char    *percent_sav;
	int	     numArgs=0;
	int	     maxArgs=(nargs<traceControl_p->num_params?nargs:traceControl_p->num_params);
	int	     modifier=0;
	int	     filter_newline=opts&filter_newline_;
	size_t   slen=0;

	/*strcpy( ofmt, ifmt );*/
	in = ifmt;
	/* strchrnul would be nice (_GNU_SOURCE, ref. man strchr) */
	/*while ((in=strchr(in,'%')))*/
	while (*in)
	{
		if (*in != '%')
		{	if (*in != '\n') ofmt[slen++] = *in++;
			else
			{	if (filter_newline) { ofmt[slen++] = ';'; in++; }
				else                { ofmt[slen++] = *in++; }
			}
			continue;
		}
		/* found '%' - process it */
		ofmt[slen] = *in;
		percent_sav = ofmt+slen;	/* save in case we need to modify it (too many args) */
		++in; ++slen;       /* point to next char */
		sizes_out[numArgs].positive = 0;
		/*printf("next char=%c\n",*in);*/
		if ((*in=='%')||(*in=='m')) { ofmt[slen++] = *in++; continue; }/* ingore %% which specified a % char */
		if (numArgs == maxArgs) { *percent_sav = '*'; continue; }  /* SAFETY - no args beyond max */
		while (	 ((*in>='0')&&(*in<='9'))
			   ||(*in=='.')||(*in=='-')||(*in=='#')||(*in=='+')||(*in=='\'')||(*in=='I')||(*in=='"'))
			ofmt[slen++] = *in++;
 chkChr:
		switch (*in)
		{
			/* Basic conversion specifiers */
		case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': case 'c':
			switch (modifier)
			{
			case -2: sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4; break; /* char */
			case -1: sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4;break;/* short */
			case 0:	 sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4;break;/* int */
			case 1:	 sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=param_bytes;/* long */
				if ((param_bytes==8)&&(sizeof(long)==4)) ofmt[slen++] = 'l';
				break;
			case 2:	 sizes_out[numArgs].push=8;          sizes_out[numArgs].size=8;break;/* long long */
			default: printf("error\n");
			}
			if (strchr("di",*in))
				sizes_out[numArgs].positive = 1;
			ofmt[slen++] = *in++;
			modifier=0;
			break;
		case 'e': case 'E': case 'f': case 'F': case 'g': case 'G': case 'a': case 'A':
			if (modifier)
			{	/* long double */
				sizes_out[numArgs].push=(param_bytes==4)?12:16;
				sizes_out[numArgs].size=(param_bytes==4)?12:16;
				modifier=0;
			} else
			{	/* double */
				sizes_out[numArgs].push=8;
				sizes_out[numArgs].size=8;
			}
			ofmt[slen++] = *in++;
			modifier=0;
			break;

			/* length modifiers */
		case 'h':  --modifier; ofmt[slen++] = *in++; goto chkChr;
		case 'l':  ++modifier; ofmt[slen++] = *in++; goto chkChr;
		case 'L':  ++modifier; ofmt[slen++] = *in++; goto chkChr;
		case 'z':  ++modifier; ofmt[slen++] = *in++; goto chkChr;

		case 's': case 'n': case 'p':       /* SAFETY -- CONVERT %s to %p */
			// for 's', first need to remove "-.0123456789" b/c %p take no flags
			if (*in=='s')
				while (strchr("-.0123456789",ofmt[slen-1]))
					--slen;
			sizes_out[numArgs].push=param_bytes; sizes_out[numArgs].size=param_bytes;
			if ((param_bytes==8)&&(sizeof(long)==4)) { // traced on 64, showing on 32
				strcpy(&ofmt[slen-1],"0x%llx"); // overwrite the original '%'
				slen+=5;
			} else 
				ofmt[slen++]='p';
			in++;
			break;
		case 'm':    // (Glibc extension.)  Print output of strerror(errno).  No argument is required.
			ofmt[slen++] = *in++;
			continue;
			break;
		case '*':   /* special -- "arg" in an arg */
			sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4/*int*/;
			sizes_out[numArgs].positive = 1;
			if (++numArgs == maxArgs) { ofmt[slen++] = *in++; break; }
			ofmt[slen++] = *in++;
			goto chkChr;
		default:
			if( !(opts&quiet_)) printf("tshow: unknown format spec char \"%c\" encountered.\n",*in);
			ofmt[slen++] = *in++;
		}
		++numArgs;
	}
	while (*in) ofmt[slen++] = *in++;
	ofmt[slen]='\0';
	//while (*ofmt == '\n') *ofmt-- = '\0';
	if (numArgs < maxArgs) sizes_out[numArgs].push=0;
	return (slen);
}	/* get_arg_sizes */

int countDigits(int n)
{	return (n >= 10)
		? 1 + countDigits(n/10)
		: 1;
}

/* Return 1 if found, else 0
   This handles the case of %%<flag> which would be false positive if just
   searching for %<flag>.
 */
int strflg( const char *ospec, char flag )
{
	for ( ; *ospec; ++ospec)
	{
		if (*ospec == '%')
		{
			++ospec;
			if (*ospec == '\0')
				break;
			if (*ospec == flag)
				return (1);
		}
	}
	return (0);
}

void printEnt(  const char *ospec, int opts, struct traceEntryHdr_s* myEnt_p
              , char *local_msg, uint8_t *local_params, struct sizepush *params_sizes
              , int bufSlot_width, int N_width, int name_width, unsigned printed, uint32_t rdIdx
              , char *tfmt, int tfmt_len, unsigned longest_lvlstr
              )
{
	unsigned                ii;
	char                  * msg_p;
	unsigned long         * params_p;
	size_t                  slen;
	time_t                  seconds;
	unsigned                useconds;
	void                  * param_va_ptr;
	uint8_t               * ent_param_ptr;
	uint8_t               * lcl_param_ptr;
	const char            * sp; /*spec ptr*/
	int                     msg_spec_included;
	char                    tbuf[0x100], *cp;
	char                  * indent="                                                                ";

		msg_p	 = (char*)(myEnt_p+1);
		params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);
		
		msg_p[traceControl_p->siz_msg - 1] = '\0';
		if (myEnt_p->nargs)
			slen = get_arg_sizes(	local_msg, msg_p, opts
			                     , myEnt_p->param_bytes, params_sizes, myEnt_p->nargs );
		else {
			if (opts&filter_newline_) {
				for (slen=0; slen<traceControl_p->siz_msg && msg_p[slen]!='\0'; ++slen)
					local_msg[slen] = (msg_p[slen]!='\n')?msg_p[slen]:';';
			}
			else {
				/* strncpy( local_msg, msg_p, traceControl_p->siz_msg ); */
				for (slen=0; slen<traceControl_p->siz_msg && msg_p[slen]!='\0'; ++slen)
					local_msg[slen] = msg_p[slen];
			}
			if (slen < traceControl_p->siz_msg)
				local_msg[slen] = '\0';
			else if (traceControl_p->siz_msg > 0) // if no term in 1st n bytes
				local_msg[--slen] = '\0';  // adjust strlen and terminate
		}

		/* determine if args need to be copied */
		if        (	 ((myEnt_p->param_bytes==4) && (sizeof(long)==4))
		           ||((myEnt_p->param_bytes==8) && (sizeof(long)==8)) )
		{	seconds	 = myEnt_p->time.tv_sec;
			useconds = myEnt_p->time.tv_usec;
			param_va_ptr = (void*)params_p;
		}
		else if (  ((myEnt_p->param_bytes==4) && (sizeof(long)==8)) )
		{	// Entry made by 32bit program, being readout by 64bit
			void *xx = &(myEnt_p->time);
			unsigned *ptr=(unsigned*)xx;
			seconds	 = *ptr++;
			useconds = *ptr;
			ent_param_ptr = (uint8_t*)params_p;
			lcl_param_ptr = local_params;
			for (ii=0; ii<myEnt_p->nargs && params_sizes[ii].push!=0; ++ii)
			{
				if      (params_sizes[ii].push == 4)
				{	if (params_sizes[ii].positive) {
						*(long*)lcl_param_ptr = (long)*(int*)ent_param_ptr;
						//printf( "converting from 0x%x\n", *(int*)ent_param_ptr );
					} else {
						*(unsigned long*)lcl_param_ptr = (unsigned long)*(unsigned*)ent_param_ptr;
						//printf( "converting to 0x%lx\n", *(unsigned long*)lcl_param_ptr );
					}
					lcl_param_ptr += sizeof(long);
				}
				else if (params_sizes[ii].push == 12) // i.e. long double
				{
					typedef _Complex float __attribute__((mode(XC))) _float80;
					*(long double*)lcl_param_ptr =		 *(long double*)(_float80*)ent_param_ptr;
					lcl_param_ptr += sizeof(long double);
				}
				else /* (params_sizes[ii].push == 8) */
				{	*(long*)lcl_param_ptr =		 *(long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long);
				}
				ent_param_ptr += params_sizes[ii].push;
			}
			param_va_ptr = (void*)local_params;
		}
		else /* (  ((myEnt_p->param_bytes==8) && (sizeof(long)==4)) ) */
		{	// Entry made by 64bit program, being shown by 32bit
			void *xx=&myEnt_p->time;
			long long *ptr=(long long*)xx;
			seconds	 = (unsigned)*ptr++;
			useconds = (unsigned)*ptr;
			ent_param_ptr = (uint8_t*)params_p;
			lcl_param_ptr = local_params;
			for (ii=0; ii<myEnt_p->nargs && params_sizes[ii].push!=0; ++ii)
			{
				if      (params_sizes[ii].size == 4) // i.e. char, short, int
				{	*(unsigned*)lcl_param_ptr = (unsigned)*(unsigned long long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long);
				}
				else if (params_sizes[ii].size == 16) // i.e. long double
				{	*(long double*)lcl_param_ptr = *(long double*)(__float128*)ent_param_ptr;
					lcl_param_ptr += sizeof(long double);
				}
				else // (params_sizes[ii].size == 8) i.e. 
				{	*(unsigned long long*)lcl_param_ptr = *(unsigned long long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long long);
				}
				ent_param_ptr += params_sizes[ii].push;
			}
			param_va_ptr = (void*)local_params;
		}

		msg_spec_included = 0;
		for (sp=ospec; *sp; ++sp)
		{
			if (*sp != '%')
			{
				printf("%c",*sp );
				continue;
			}
			if (*++sp =='\0')
			{
				printf("%%"); // print the trailing/ending %
				break;
			}
			switch (*sp)
			{
			case '%': printf("%%"); break;
			case 'a': printf("%4u", myEnt_p->nargs); break;
			case 'B': printf("%u", myEnt_p->param_bytes); break;
			case 'C': printf("%" MM_STR(TRACE_CPU_WIDTH) "u", myEnt_p->cpu); break;
			case 'D': /* ignore this "control" */ break;
			case 'e': snprintf(tbuf,sizeof(tbuf),"%s:%d",traceNamLvls_p[myEnt_p->TrcId].name,myEnt_p->linenum);
				      printf("%*.*s",name_width+1+TRACE_LINENUM_WIDTH,name_width+1+TRACE_LINENUM_WIDTH,tbuf);break;
			case 'f': printf("%s",local_msg); msg_spec_included=1; break;
			case 'H': /* ignore this "control" */ break;
			case 'I': printf("%4u", myEnt_p->TrcId); break;
			case 'i': printf("%*d", TRACE_TID_WIDTH, myEnt_p->tid); break; /* darwin 16 tids are routinely 7 digit */
			case 'L': printf("%*s", longest_lvlstr, _lvlstr[(myEnt_p->lvl)&LVLBITSMSK]); break;
			case 'l': printf("%2d", myEnt_p->lvl); break;
			case 'm':
				if (local_msg[slen-1] == '\n')
					local_msg[--slen] = '\0'; // strip off the trailing newline
				if (opts&indent_)
					printf("%s",&indent[LVLBITSMSK-(myEnt_p->lvl&LVLBITSMSK)]);
				if (myEnt_p->nargs) {
					va_list ap_=TRACE_VA_LIST_INIT(param_va_ptr);
					vprintf( local_msg, ap_ );
				} else
					printf("%s",local_msg);				
				msg_spec_included=1;
				break;
			case 'N': printf("%*u", N_width, printed ); break;
			case 'n': printf("%*.*s",name_width,name_width,traceNamLvls_p[myEnt_p->TrcId].name);break;
			case 'P': printf("%6d", myEnt_p->pid); break; /* /proc/sys/kernel/pid_max has 32768 or 458752 (on mu2edaq13) */
			case 'R':
				if (myEnt_p->get_idxCnt_retries) printf( "%u", myEnt_p->get_idxCnt_retries );
				else							 printf( "." );
				break;
			case 's': printf("%*u", bufSlot_width, rdIdx%traceControl_p->num_entries ); break;
			case 'T': if(tfmt_len){ strftime(tbuf,sizeof(tbuf),tfmt,localtime(&seconds));// strcat(tbuf," ");
					  if ((cp=strstr(tbuf,"%0")) && *(cp+2)>='1' && *(cp+2)<='5' && *(cp+3)=='d') {
						  // NOTE: if *(cp+2)==6, don't do anything.
						  // Example: fmt originally had "%%04d" which got changed by strftime to "%04d";
						  // grab the character '4' and adjust useconds accordingly.
						  int div=1;
						  switch (*(cp+2)){
						  case '1':div=100000;break;
						  case '2':div=10000;break;
						  case '3':div=1000;break;
						  case '4':div=100;break;
						  case '5':div=10;break;
						  }
						  useconds = (unsigned)((double)useconds/div+0.5); // div, round and cast back to unsigned
					  }
					  printf(tbuf, useconds); } break;
			case 't': printf("%10u", (unsigned)myEnt_p->tsc); break;
			case 'u': printf("%" MM_STR(TRACE_LINENUM_WIDTH) "u", myEnt_p->linenum); break;
			case 'x': /* ignore this "control" */ break;
			default:  printf("%%%c",*sp);break; // print unknown %<blah> sequence
			}
		}

		/*typedef unsigned long parm_array_t[1];*/
		/*va_start( ap, params_p[-1] );*/
		if (!msg_spec_included) {
			if (local_msg[slen-1] == '\n')
				local_msg[--slen] = '\0'; // strip off the trailing newline
			if (opts&indent_)
				printf("%s",&indent[LVLBITSMSK-(myEnt_p->lvl&LVLBITSMSK)]);
			if (ospec[0]!='\0' && !strchr(" \t:-|", *(sp-1))) // if the specification does not end in a "separator"
				printf(" ");
			if (myEnt_p->nargs)
			{	/* Ref. http://andrewl.dreamhosters.com/blog/variadic_functions_in_amd64_linux/index.html
				   Here, I need an initializer so I must have this inside braces { } */
				va_list ap_=TRACE_VA_LIST_INIT(param_va_ptr);
				vprintf( local_msg, ap_ );
			}
			else
				printf("%s",local_msg);
		}
		printf("\n");
		fflush(stdout);
} /* printEnt */

typedef struct trace_ptrs {
	struct trace_ptrs      *next;
	struct trace_ptrs      *prev;
	const char             *file; /* for debugging */
	struct traceControl_s  *ro_p;
	struct traceControl_rw *rw_p;
	struct traceNamLvls_s  *namlvls_p;
	struct traceEntryHdr_s *entries_p;
	uint32_t                rdIdx;
	int32_t                 max;
} trace_ptrs_t;

void trace_ptrs_store( int idx, trace_ptrs_t *trace_ptrs, const char * file )
{
	if (idx == 0)
		trace_ptrs[idx].prev = NULL;
	else {
		trace_ptrs[idx].prev = &trace_ptrs[idx-1];
		trace_ptrs[idx-1].next = &trace_ptrs[idx];
	}
	trace_ptrs[idx].file = file;
	trace_ptrs[idx].ro_p = traceControl_p;
	trace_ptrs[idx].rw_p = traceControl_rwp;
	trace_ptrs[idx].namlvls_p = traceNamLvls_p;
	trace_ptrs[idx].entries_p = traceEntries_p;
	
	trace_ptrs[idx].next = NULL;
} /* trace_ptrs_store */

/* Designed to work in the code:
   for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next )
       trace_ptrs_discard( t_ptrs, &trace_ptrs_list_start );
 */
void trace_ptrs_discard( trace_ptrs_t *tptr, trace_ptrs_t **list_start )
{
	trace_ptrs_t *prev=tptr->prev;
	trace_ptrs_t *next=tptr->next;
	if(next)
		next->prev = prev;
	if(prev)
		prev->next = next;
	if(prev==NULL && next==NULL)
		*list_start=NULL;
} /* trace_ptrs_discard */

void trace_ptrs_restore( trace_ptrs_t *tptr )
{
	traceControl_p   = tptr->ro_p;
	traceControl_rwp = tptr->rw_p;
	traceNamLvls_p   = tptr->namlvls_p;
	traceEntries_p   = tptr->entries_p;
} /* trace_ptrs_restore */

/*  rdIdx and wrIdxCnt are BOTH non-modulo.
	For forward read: simple delta
	For reverse read: num_entries(or wrIdxCnt) - delta

NOT WORRYING ABOUT wrIdxCnt,rdIdx wrap yet -- Work-In-Progress!!!
 */
uint32_t rdIdx_has_lines( uint32_t wrIdxCnt, uint32_t rdIdx, int for_rev )
{
    uint32_t lines;
	if (for_rev == -1) {
		// READING in reverse -- from large numbers to smaller; reading and writing are on head on collision course
		if (wrIdxCnt <= traceControl_p->num_entries) {
			//     0*********<-rw->_______End
			lines = wrIdxCnt - IDXCNT_DELTA(wrIdxCnt,rdIdx+1);
			//return ((lines<=wrIdxCnt)?lines:0);
			return (lines);
		} else {
			lines = traceControl_p->num_entries - IDXCNT_DELTA(wrIdxCnt,rdIdx+1);
			//return ((lines<=traceControl_p->num_entries)?lines:0);
			return (lines);
		}
	} else {
		// READING forward (i.e from 0 up; read chasing writing; writing may overrun reading)
		if (wrIdxCnt <= traceControl_p->num_entries) {
			lines = IDXCNT_DELTA(wrIdxCnt,rdIdx);
			//return ((lines<=wrIdxCnt)?lines:0);
			return (lines);
		} else {
			lines = IDXCNT_DELTA(wrIdxCnt,rdIdx);
			//return ((lines<=traceControl_p->num_entries)?lines:0);
			return (lines);
		}
	}
} /* rdIdx_has_lines */


int tvcmp( struct timeval *t1, struct timeval *t2)
{
	if        (t1->tv_sec  < t2->tv_sec) {
		return (-1);
	} else if (t1->tv_sec == t2->tv_sec) {
		if        (t1->tv_usec < t2->tv_usec) {
			return (-1);
		} else if (t1->tv_usec == t2->tv_usec) {
			return (0);
		} else
			return (1);
	} else
		return (1);
} /* tvcmp */

/* count==-1, slotStart==-1  ==> DEFAULT - reverse print from wrIdx to zero (if not
                                 full) or for num_entrie
   count>=0,  slotStart==-1  ==> reverse print "count" entries (incuding 0, which
                                 doesn't seem to useful
   count>=0,  slotStart>=0   ==> reverse print count entries starting at "slot" idx
                                 "slotStart"  I.e. if 8 traces have occurred, these 2
                                 are equivalent:
                                 traceShow( ospec, -1, -1, 0 ) and
                                 traceShow( ospec,  8,  7, 0 )
 */
void traceShow( const char *ospec, int count, int slotStart, int show_opts, int argc, char *argv[] )
{
	uint32_t rdIdx;
	int32_t max;
	unsigned printed=0;
	int      ii;
	int	   	 bufSlot_width;
	int	   	 opts=0;
	struct traceEntryHdr_s* myEnt_p;

	uint8_t               * local_params;
	char                  * local_msg;
	struct sizepush       * params_sizes;
	const char            * sp; /*spec ptr*/
	char                  * tfmt;
	int                     tfmt_len;
	time_t                  tt=time(NULL);
	char                    tbuf[0x100], ttbuf[0x100];
	int                     for_rev=-1; /* default is reverse print */
	int                     forward_continuous=0; /* continuous==1 will force for_rev=1 */
	int                     msg_spec_included;
	int						files_to_show=0;
	int                     memlen_out_unused __attribute__((__unused__));
	trace_ptrs_t		  * t_ptrs, * trace_ptrs_list_start, *t_ptrs_use;
	struct timeval        * tv_p_use;
	int                     name_width=0;
	unsigned                longest_lvlstr=0;
	uint32_t                num_entries_total;
	uint32_t                siz_msg_largest;
	uint32_t                siz_entry_largest=0;
	uint32_t                num_params_largest;
	int                     N_width;

	if ((slotStart != -1) && (argc >= 2)) {
		fprintf( stderr, "-s<startSlotIndex> invalid with multiple files\n" );
		exit(EXIT_FAILURE);
	} else if ((slotStart != -1) && (show_opts&forward_)) {
		fprintf( stderr, "-s<startSlotIndex> invalid with -F (Forward/poll mode)\n" );
		exit(EXIT_FAILURE);
	}
	opts |= show_opts; /* see enum show_opts_e above */
	opts |= (strflg(ospec,'D')?indent_:0);
	opts |= (strflg(ospec,'x')?filter_newline_:0);

	if (argc == 0) {
		traceInit(NULL,1); /* init traceControl_p, traceControl_rwp, etc. */
		trace_ptrs_list_start = (trace_ptrs_t*)malloc(sizeof(trace_ptrs_t)*1);
		trace_ptrs_store( files_to_show++, trace_ptrs_list_start, getenv("TRACE_FILE")?getenv("TRACE_FILE"):traceFile );
	} else {
		trace_ptrs_list_start = (trace_ptrs_t*)malloc(sizeof(trace_ptrs_t)*argc);
		memset( trace_ptrs_list_start, 0, sizeof(trace_ptrs_t)*argc );
		trace_ptrs_list_start[0].prev = NULL;
		for (ii=0; ii<argc; ++ii) {
			if (access(argv[ii],R_OK) != 0) {
				fprintf( stderr, "Warning: cannot access %s\n", argv[ii] );
				continue;
			}
			setenv("TRACE_FILE", argv[ii], 1 );
# if 1
			traceControl_p = traceControl_p_static = NULL; // trick to allow traceInit to (re)mmap (another) file
			traceInit(NULL,1); /* init traceControl_p, traceControl_rwp, etc. NOTE: multiple "register_atfork" will occur, but should be tolerated be this non-forking app */
# else
			trace_mmap_file( argv[ii],&memlen_out_unused,&traceControl_p, &traceControl_rwp // NOTE: traceNamLvls_p, traceEntries_p are iniitalized
			                , 0,0,0,0, 1 );
# endif
			if (TRACE_CNTL("mapped")) {
				int jj;
				for (jj=0; jj<files_to_show; ++jj) {
					if (traceControl_p == trace_ptrs_list_start[jj].ro_p)
						// found
						break;
				}
				if (jj==files_to_show) {
					trace_ptrs_store( files_to_show++, trace_ptrs_list_start, argv[ii] );
				}
			}
		}
	}

	/* get the time format and the length of the format (using example time (now) */
	if((tfmt=getenv("TRACE_TIME_FMT"))==NULL)
		//tfmt=(char*)TRACE_DFLT_TIME_FMT;
		tfmt=(char*)"%s%%06d"; // %s - The number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
	strftime(ttbuf,sizeof(ttbuf),tfmt,localtime(&tt));
	tfmt_len=snprintf( tbuf,sizeof(tbuf),ttbuf, 0 );  /* possibly (probably) add usecs (i.e. FMT has %%06d) */

	if (strflg(ospec,'L')) { /* level number 0-63 is 2 digits, 'L' is level string -- need to find longest str */
		unsigned ll;
		for (ii=0; ii<64; ++ii)
			if (_lvlstr[ii]!=NULL && (ll=strlen(_lvlstr[ii]))>longest_lvlstr)
				longest_lvlstr = ll;
	}

	if (strflg(ospec,'n') || strflg(ospec,'e'))
		for (name_width=0, t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next )
			if (name_width < (int)t_ptrs->rw_p->longest_name)
				name_width = t_ptrs->rw_p->longest_name;
	for (num_entries_total=0, t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next )
		num_entries_total += t_ptrs->ro_p->num_entries;
	for (siz_msg_largest=0, num_params_largest=0, t_ptrs=trace_ptrs_list_start;
		 t_ptrs!=NULL; t_ptrs=t_ptrs->next ) {
		if (siz_msg_largest < t_ptrs->ro_p->siz_msg)
			siz_msg_largest = t_ptrs->ro_p->siz_msg;
		if (num_params_largest < t_ptrs->ro_p->num_params)
			num_params_largest = t_ptrs->ro_p->num_params;
		if (siz_entry_largest < t_ptrs->ro_p->siz_entry)
			siz_entry_largest = t_ptrs->ro_p->siz_entry;
	}

	/* If just a count is given, it will be a way to limit the number printed;
	   a short hand version of "... | head -count". (so if there are not entries,
	   none will be printed.
	   If however, count and slotStart are given, the count (up to numents) will be
	   printed regardless of how many entries are actually filled with real
	   data. This gives a way to look at data after a "treset".
	*/
	if (slotStart >= 0) // only applicable with one file
	{	slotStart++; /* startSlot index needs to be turned into a "count" */
		if ((unsigned)slotStart > num_entries_total)
		{	slotStart = num_entries_total;
			printf("specified slotStart index too large, adjusting to %d\n",slotStart );
		}
		trace_ptrs_list_start->rdIdx = slotStart; // in "startSlot mode" rdIdx is not realative to wrIdxCnt
	}
	else
	{	/* here, rdIdx starts as a count (see max = rdIdx below), but it is
		   used as an index after first being decremented (assuming reverse
		   printing) */
		for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next)
			t_ptrs->rdIdx = TRACE_ATOMIC_LOAD(&t_ptrs->rw_p->wrIdxCnt); // % t_ptrs->ro_p->num_entries;
	}

	if ((count>=0) && (slotStart>=0)) {   // only applicable with one file (the slotStart>=0 part)
		t_ptrs=trace_ptrs_list_start;
		if ((unsigned)count > num_entries_total) {
			count = max = t_ptrs->max = t_ptrs->ro_p->num_entries;
		} else {
			max = t_ptrs->max = count;
		}
	} else {
		for (max=0, t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next)
			if (TRACE_ATOMIC_LOAD(&t_ptrs->rw_p->wrIdxCnt)>=t_ptrs->ro_p->num_entries)
				//max = traceControl_p->num_entries;
				max += t_ptrs->max = t_ptrs->ro_p->num_entries;
			else
				//max = rdIdx;
				max += t_ptrs->max = t_ptrs->rdIdx;
	}

	if ((count>=0) && (slotStart<0) && (count<max))
		for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next)
			t_ptrs->max=count;
	if (opts&forward_) {
		for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next ) {
			t_ptrs->rdIdx = IDXCNT_ADD( TRACE_ATOMIC_LOAD(&t_ptrs->rw_p->wrIdxCnt), -t_ptrs->max );
			/* subtract 1 b/c during loop (below) rdIdx is incremented first */
			t_ptrs->rdIdx = IDXCNT_ADD( t_ptrs->rdIdx, -1 );
		}
		forward_continuous =1;
		for_rev=1;
	}

	//printf("for_rev=%d slotStart=%d count=%d max=%d siz_msg_largest=%u num_params_largest=%u files_to_show=%d\n"
	//       ,for_rev,slotStart,count,max,siz_msg_largest,num_params_largest, files_to_show);

	N_width = minw(3,countDigits(max-1));
	bufSlot_width= minw( 3, countDigits(num_entries_total-1) );

	myEnt_p       = (struct traceEntryHdr_s *)malloc( siz_entry_largest );
	local_msg     =	                   (char*)malloc( siz_msg_largest * 3 );/* in case an %ld needs change to %lld */
	local_params  =	                (uint8_t*)malloc( num_params_largest*sizeof(uint64_t) );
	params_sizes  =         (struct sizepush*)malloc( num_params_largest*sizeof(struct sizepush) );

	if (strflg(ospec,'H'))
	{
		msg_spec_included=0;
# define TRACE_MSG_DASHES  "--------------------------"
# define TRACE_LONG_DASHES "------------------------------------------------"
		for (sp=ospec; *sp; ++sp) {
			if (*sp != '%') {
				printf("%c",*sp );
				continue;
			}
			if (*++sp =='\0') {
				printf("%%"); // print the trailing/ending %
				break;
			}
			switch (*sp) {
			case '%': printf("%%"); break;
			case 'a': printf("args");break;
			case 'B': printf("B"); break;
			case 'C': printf("cpu"); break;
			case 'D': /* ignore this "control" */ break;
			case 'e': printf("%*s", name_width+1+TRACE_LINENUM_WIDTH,"trcname:ln#");break; // +1 for ':'
			case 'f': printf("%-*s", (int)strlen(TRACE_MSG_DASHES),"msg"); msg_spec_included=1; break;
			case 'H': /* ignore this "control" */ break;
			case 'I': printf(" TID"); break;
			case 'i': printf("%" MM_STR(TRACE_TID_WIDTH) "s", "tid" ); break; /* darwin 16 tids are routinely 7 digit */
			case 'L': printf("lvl"); break;
			case 'l': printf("lv"); break;
			case 'm': printf("%-*s", (int)strlen(TRACE_MSG_DASHES),"msg"); msg_spec_included=1; break;
			case 'N': printf("%*s", N_width, "idx" ); break;
			case 'n': printf("%*s", name_width,"trcname");break;
			case 'P': printf("%" MM_STR(TRACE_TID_WIDTH) "s","pid"); break;
			case 'R': printf("r"); break;
			case 's': printf("%*s", bufSlot_width, "slt" ); break;
			case 'T': if(tfmt_len)printf("%*.*s", tfmt_len,tfmt_len,&("us_tod"[tfmt_len>=6?0:6-tfmt_len])); break;
			case 't': printf("       tsc"); break;
			case 'u': printf("%" MM_STR(TRACE_LINENUM_WIDTH) "s", "ln#");break;
			case 'x': /* ignore this "control" */ break;
			default:  printf("%%%c",*sp);break; // print unknown %<blah> sequence
			}
		}
		if (!msg_spec_included) {
			if (!strchr(" \t:-|", *(sp-1))) // if the specification does not end in a "separator"
				printf(" ");
			printf("%-*s\n", (int)strlen(TRACE_MSG_DASHES),"msg");
		}
		else if (*(sp-1) != '\n')
			printf("\n");

		msg_spec_included=0; // reset for the ---- processing
		for (sp=ospec; *sp; ++sp) {
			if (*sp != '%') {
				if (strchr(" \t\n",*sp))
					printf("%c",*sp );
				else
					printf("-");
				continue;
			}
			if (*++sp =='\0') {
				printf("%%"); // print the trailing/ending %
				break;
			}
			switch (*sp) {
			case '%': printf("-"); break;
			case 'a': printf("----");break;
			case 'B': printf("-"); break;
			case 'C': printf("---"); break;
			case 'D': /* ignore this "control" */ break;
			case 'e': printf("%.*s", name_width+1+TRACE_LINENUM_WIDTH,TRACE_LONG_DASHES);break;
			case 'f': printf(TRACE_MSG_DASHES); msg_spec_included=1; break;
			case 'H': /* ignore this "control" */ break;
			case 'I': printf("----"); break;
			case 'i': printf("%." MM_STR(TRACE_TID_WIDTH) "s", TRACE_LONG_DASHES); break; /* darwin 16 tids are routinely 7 digit */
			case 'L': printf("---"); break;
			case 'l': printf("--"); break;
			case 'm': printf(TRACE_MSG_DASHES); msg_spec_included=1; break;
			case 'N': printf("%.*s", N_width, TRACE_LONG_DASHES); break;
			case 'n': printf("%.*s", name_width,TRACE_LONG_DASHES);break;
			case 'P': printf("%." MM_STR(TRACE_TID_WIDTH) "s", TRACE_LONG_DASHES); break;
			case 'R': printf("-"); break;
			case 's': printf("%.*s", bufSlot_width, TRACE_LONG_DASHES); break;
			case 'T': if(tfmt_len)printf("%.*s", tfmt_len, TRACE_LONG_DASHES); break;
			case 't': printf("----------"); break;
			case 'u': printf("%." MM_STR(TRACE_LINENUM_WIDTH) "s", TRACE_LONG_DASHES);break;
			case 'x': /* ignore this "control" */ break;
			default:  printf("%%%c",*sp);break; // print unknown %<blah> sequence
			}
		}
		if (!msg_spec_included) {
			if (!strchr(" \t:-|", *(sp-1))) // if the specification does not end in a "separator"
				printf(" ");
			printf(TRACE_MSG_DASHES "\n");
		}
		else if (*(sp-1) != '\n')
			printf("\n");

	}

	// Change count to idx
	for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next ) {
		rdIdx = IDXCNT_ADD( t_ptrs->rdIdx, for_rev );
		t_ptrs->rdIdx = rdIdx;
	}

	printed=0;
	while (1) {
		uint32_t lines;
		/* loop trhough the files looking for
		   a) ones that have something to print and
		   b) the one that is earliest or latest depending on for_rev
		 */
		if (count>=0 && (int)printed>=count && !forward_continuous) /* yes, can elect to print zero lines w/ -c0 */
			break;
		
 forward_check:
		t_ptrs_use = 0; tv_p_use = 0;
		for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next ) {
			traceControl_p = t_ptrs->ro_p; // used in rdIdx_has_lines
 reset_check:
			if (slotStart >= 0) {
				// "startSlot mode"
				t_ptrs_use=t_ptrs;
			} else if ((lines=rdIdx_has_lines(t_ptrs->rw_p->wrIdxCnt,t_ptrs->rdIdx,for_rev))) {
				if (lines > traceControl_p->num_entries) {  // reset detect
					t_ptrs->rdIdx = 0;
					goto reset_check;
				}
				trace_ptrs_restore( t_ptrs ); // for idxCnt2entPtr to get time
				if(!t_ptrs_use) {
					t_ptrs_use=t_ptrs;
					tv_p_use = &idxCnt2entPtr(t_ptrs->rdIdx)->time;
				} else {
					// Compare differently depending on whether forward or reverse.
					// Race condition -- rdIdx might have just changed and time could be being overwritten :(
					// Frozen trace buffers are the safest!
					if (tvcmp(tv_p_use,&idxCnt2entPtr(t_ptrs->rdIdx)->time) == for_rev) {
						t_ptrs_use=t_ptrs;
						tv_p_use = &idxCnt2entPtr(t_ptrs->rdIdx)->time;
					}
				}
			} else if (!forward_continuous)  // not strictly needed, but a little efficiency gain
				trace_ptrs_discard( t_ptrs, &trace_ptrs_list_start );
		}
		if (t_ptrs_use == NULL && !forward_continuous) {
			break;
		} else if (t_ptrs_use==NULL && forward_continuous) {
			usleep( 100000 );
			goto forward_check;
		}

		// had (possibly just changed) t_ptrs_use->rdIdx for a while -- race condition -
		// could be being written to as we are copying :( -- frozen buffers are the safest
		trace_ptrs_restore( t_ptrs_use );
		memcpy( myEnt_p, idxCnt2entPtr(t_ptrs_use->rdIdx), traceControl_p->siz_entry );

		printEnt(  ospec, opts, myEnt_p
		         , local_msg, local_params, params_sizes
		         , bufSlot_width, N_width, name_width, printed, t_ptrs_use->rdIdx
		         , tfmt, tfmt_len, longest_lvlstr );
		++printed;
		t_ptrs_use->rdIdx = IDXCNT_ADD( t_ptrs_use->rdIdx, for_rev );
	}
}	/*traceShow*/


void traceInfo()
{
	uint32_t       used;
	uint32_t       wrCopy;
	uint32_t       nameLockCopy;
	char           outstr[200];
	struct tm      *tmp;
	time_t         tt=(time_t)traceControl_p->create_tv_sec;
	int            memlen=traceMemLen( cntlPagesSiz()
	                                  ,traceControl_p->num_namLvlTblEnts
	                                  ,traceControl_p->siz_msg
	                                  ,traceControl_p->num_params
	                                  ,traceControl_p->num_entries); /* for when buffer is vmalloc (kernel) or file mmapped (user), but not userspace inactive */
	tmp = localtime( &tt );
	if (tmp == NULL) { perror("localtime"); exit(EXIT_FAILURE); }
	if (strftime(outstr, sizeof(outstr),"%a %b %d %H:%M:%S %Z %Y",tmp) == 0)
	{   perror("strftime"); exit(EXIT_FAILURE);
	}
	nameLockCopy = TRACE_ATOMIC_LOAD(&traceControl_rwp->namelock);
	wrCopy       = TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
	used = ((traceControl_rwp->full)
		?traceControl_p->num_entries
		:wrCopy ); /* Race Condition - if not full this shouldn't be > traceControl_p->num_entries */
	printf("trace.h rev       = %s\n"
	       "revision          = %s\n"
	       "create time       = %s\n"
	       "trace_initialized = %d\n"
	       "mode              = 0x%-8x  %s%s\n"
	       "writeIdxCount     = 0x%08x  entries used: %u\n"
	       "full              = %d\n"
	       "nameLock          = %u\n"
	       "largestMultiple   = 0x%08x\n"
	       "largestZeroOffset = 0x%08x\n"
	       "trigIdxCnt        = 0x%08x\n"
	       "triggered         = %d\n"
	       "trigActivePost    = %u\n"
	       "limit_cnt_limit   = %u\n"
	       "limit_span_on_ms  = %llu\n"
	       "limit_span_off_ms = %llu\n"
	       "traceLevel        = 0x%0*llx 0x%0*llx 0x%0*llx\n"
	       "num_entries       = %u\n"
	       "max_msg_sz        = %u  includes system enforced terminator\n"
	       "max_params        = %u\n"
	       "entry_size        = %u\n"
	       "namLvlTbl_ents    = %u\n"
	       "namLvlTbl_name_sz = %u         not including null terminator\n"
	       "longest_name      = %u\n"
	       "wrIdxCnt offset   = %p\n"
	       "namLvls offset    = 0x%lx\n"
	       "buffer_offset     = 0x%lx\n"
	       "memlen            = 0x%x          %s\n"
	       "default TRACE_SHOW=\"%s\" others: a:nargs B:paramBytes D:inDent e:nam:ln# f:convertedMsgfmt_only I:trcId l:lvlNum s:slot t:tsc u:line\n"
	       "default TRACE_PRINT=\"%s\" others: C:core e:nam:ln# f:file I:trcId i:threadID l:lvlNum m:msg-insert N:unpadded_trcName P:pid s:severity t:insert u:line\n"
	       , TRACE_REV
	       , traceControl_p->version_string
	       , outstr
	       , traceControl_p->trace_initialized
	       , traceControl_rwp->mode.mode, traceControl_rwp->mode.bits.S?"Slow:ON, ":"Slow:off", traceControl_rwp->mode.bits.M?" Mem:ON":" Mem:off"
	       , wrCopy, used
	       , traceControl_rwp->full
	       , nameLockCopy
	       , traceControl_p->largest_multiple
	       , traceControl_p->largest_zero_offset
	       , traceControl_rwp->trigIdxCnt
	       , traceControl_rwp->triggered
	       , traceControl_rwp->trigActivePost
	       , traceControl_rwp->limit_cnt_limit
	       , (unsigned long long)traceControl_rwp->limit_span_on_ms
	       , (unsigned long long)traceControl_rwp->limit_span_off_ms
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceNamLvls_p[traceTID].M /* sizeof(uint64_t)*2 is "nibbles" */
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceNamLvls_p[traceTID].S
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceNamLvls_p[traceTID].T
	       , traceControl_p->num_entries
	       , traceControl_p->siz_msg
	       , traceControl_p->num_params
	       , traceControl_p->siz_entry
	       , traceControl_p->num_namLvlTblEnts
	       , (int)sizeof(traceNamLvls_p[0].name)-1
	       , traceControl_rwp->longest_name
	       , (void*)&((struct traceControl_s*)0)->rw.wrIdxCnt
	       , (unsigned long)traceNamLvls_p - (unsigned long)traceControl_rwp
	       , (unsigned long)traceEntries_p - (unsigned long)traceControl_rwp
	       , traceControl_p->memlen
	       , (traceControl_p->memlen != (uint32_t)memlen)?"not for mmap":""
	       , DFLT_SHOW
	       , TRACE_PRINT
	       );
}   /* traceInfo */



/* for "test-compare" printf buffer (could use write) and (more so) "TRACE" */
#pragma GCC diagnostic ignored "-Wformat-security"

int main(  int	argc
         , char	*argv[] )
{
	int         ret=0;
	const char *cmd;
extern  char       *optarg;        /* for getopt */
extern  int        optind;         /* for getopt */
        int        opt;            /* for how I use getopt */
	int	        do_heading=1;
	int	        show_opts=0;
	unsigned    ii=0;
	char        test_name[0x100];
	int         opt_loops=-1;
	unsigned    opt_dly_ms=0;
	unsigned    opt_burst=1;
	int         opt_all=0;
	int         opt_count=-1, opt_start=-1;
	const char *opt_name=NULL;

	while ((opt=getopt(argc,argv,"?hn:N:f:x:HqVL:Fl:d:b:ac:s:")) != -1) {
		switch (opt) {
		/*   '?' is also what you get w/ "invalid option -- -"   */
		case '?': case 'h': printf(USAGE);exit(0);           break;
		case 'N': opt_name=optarg;                           break;
		case 'n': setenv("TRACE_NAME",optarg,1);             break;/*note: TRACE_CNTL "file" or "name" doesn't allow setting the other (order becomes dependent)*/
		case 'f': setenv("TRACE_FILE",optarg,1);             break;
		case 'x': trace_thread_option=strtoul(optarg,NULL,0);break;
		case 'H': do_heading=0;                              break;
		case 'q': show_opts|=quiet_;                         break;
		case 'V': printf( "%s\n",TRACE_CNTL_REV ); exit(0);  break;
		case 'L': setlocale(LC_NUMERIC,optarg);              break;
        case 'F': show_opts|=forward_;                       break;
		case 'l': opt_loops=strtoul(optarg,NULL,0);          break;
		case 'd': opt_dly_ms=strtoul(optarg,NULL,0);         break;
		case 'b': opt_burst=strtoul(optarg,NULL,0);          break;
		case 'a': opt_all=1;                                 break;
		case 'c': opt_count=strtoul(optarg,NULL,0);          break;
		case 's': opt_start=strtoul(optarg,NULL,0);          break;
		}
	}
	if (argc - optind < 1) {
		printf( "Need cmd\n" );
		printf( USAGE ); exit( 0 );
	}
	cmd = argv[optind++];

	if (opt_name && !(strncmp(cmd,"lvl",3)==0)) // name (which may be wildcard) will get added to lvl command below
		setenv("TRACE_NAME",opt_name,1);

	if(getenv("LC_NUMERIC"))                        // IFF LC_NUMERIC is set in the environment (i.e to "en_US.UTF-8")...
		setlocale(LC_NUMERIC,getenv("LC_NUMERIC")); // this is needed for (e.g.) %'d to produce output with thousands grouped

	if		(strcmp(cmd,"test1") == 0)
	{	int loops=1, trace_cnt;
		char msg_str[200];
		char addr_str[100];
		void *vp=addr_str;
		setenv("TRACE_NAME",basename(__FILE__),0); // allow this to be overridden by -n and env.var. (and define TRACE_NAME)
		if (opt_loops > -1) loops=opt_loops;
		sprintf( addr_str, "%p", vp );
		sprintf( msg_str,"Hi %%d. \"c 2.5 -5 5000000000 0x87654321 2.6 %p\" ", vp );
		strcat( msg_str, "should be repeated here: %c %.1f %hd %lld %p %.1Lf %s" );
		for (trace_cnt=1; trace_cnt<=loops; ++trace_cnt)
		{	TRACE( 2, msg_str, trace_cnt
				  , 'c',2.5,(short)-5,(long long)5000000000LL,(void*)0x87654321,(long double)2.6, addr_str );
		}
	}
	else if (strcmp(cmd,"test") == 0)
	{	float	 ff[10];
		pid_t	 tid;
		uint32_t desired, myIdx;
		int32_t  xx;

		tid = trace_gettid();
		if (tid == -1) perror("trace_gettid");
		printf("tid=%ld\n", (long int)tid );

		printf("sizeof: int:%u long:%u pid_t:%u pthread_t:%u timeval:%u "
		       "double:%u traceControl_s:%u traceEntryHdr_s:%u traceControl(for disabled):%u\n"
		       , (unsigned)sizeof(unsigned), (unsigned)sizeof(long), (unsigned)sizeof(pid_t)
		       , (unsigned)sizeof(pthread_t), (unsigned)sizeof(struct timeval)
		       , (unsigned)sizeof(double)
		       , (unsigned)sizeof(struct traceControl_s)
		       , (unsigned)sizeof(struct traceEntryHdr_s)
		       , (unsigned)sizeof(traceControl) );
		printf("offset: trigIdxCount   =%p\n"
			   "        trigActivePost =%p\n"
			   "        full           =%p\n"
			   "        lvl            =%p\n"
			   "        pid            =%p\n"
			   "        TID            =%p\n"
			   "        get_retries    =%p\n"
			   "        param_bytes    =%p\n"
			   "        tsc            =%p\n"
			   , (void*)&((struct traceControl_s*)0)->rw.trigIdxCnt
			   , (void*)&((struct traceControl_s*)0)->rw.trigActivePost
			   , (void*)&((struct traceControl_s*)0)->rw.full
			   , (void*)&((struct traceEntryHdr_s*)0)->lvl
			   , (void*)&((struct traceEntryHdr_s*)0)->pid
			   , (void*)&((struct traceEntryHdr_s*)0)->TrcId
			   , (void*)&((struct traceEntryHdr_s*)0)->get_idxCnt_retries
			   , (void*)&((struct traceEntryHdr_s*)0)->param_bytes
			   , (void*)&((struct traceEntryHdr_s*)0)->tsc
			   );

		for (ii=0; ii<sizeof(ff)/sizeof(ff[0]); ++ii)  ff[ii]=2.5*ii;

		/* NOTE: using setenv method works in threading env where as additional
		   threads initializing via TRACE will disable/undo the levels
		   set by the initialization via TRACE_CNTL("lvlsetS",0xffLL) */
		setenv("TRACE_LVLS","0xff",0);/*does TRACE_CNTL("lvlsetS",0xffLL);TRACE_CNTL("modeS",1);*/
		/* NOTE/Recall - _LVLS does not "activate" like TRACE_{FILE,NAME,MSGMAX,NUMENTS,NAMTBLENTS} */

		/* _at_least_ set bit 0 (lvl=0) in the "M" mask and turn on the "M" mode
		   bit -- this is what is on by default when the file is created */
		/*                    Mem    Slow Trig */
		TRACE_CNTL( "lvlset", 0xfLL, 0LL, 0LL );
		TRACE_CNTL( "modeM", 1 );

		TRACE( 2, "hello" );

		myIdx = traceControl_p->largest_multiple - 3;
		printf("myIdx=0x%08x\n", myIdx );
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,1);
			printf( "myIdx==>IDXCNT_ADD(myIdx,1): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,-1);
			printf( "myIdx==>IDXCNT_ADD(myIdx,-1): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		printf("myIdx=0x%08x\n", myIdx );

		xx=5;
		myIdx = traceControl_p->largest_multiple - (3*xx);
		printf("myIdx=0x%08x\n", myIdx );
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,xx);
			printf( "myIdx==>IDXCNT_ADD(myIdx,5): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,-xx);  /* NOTE: this will not work if xx is of type uint32_t; it must be of int32_t */
			printf( "myIdx==>IDXCNT_ADD(myIdx,-5): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		printf("myIdx=0x%08x\n", myIdx );

		printf("For forward read... simple delta\n");
		printf("IDXCNT_DELTA(wr=5,rd=4)=%u\n",IDXCNT_DELTA(5,4));
		printf("IDXCNT_DELTA(wr=5,rd=0)=%u\n",IDXCNT_DELTA(5,0));
		printf("IDXCNT_DELTA(wr=5,rd=IDXCNT_ADD(0,-1)=0x%08x)=%u\n",IDXCNT_ADD(0,-1),IDXCNT_DELTA(5,IDXCNT_ADD(0,-1)));
		printf("For reverse read... numents(or wrIdxCnt) - delta\n");
		printf("(num_entries=%u) - IDXCNT_DELTA(wr=IDXCNT_ADD(num_entries,10),rd=IDXCNT_ADD(num_entries,5)) = %u\n"
		       ,traceControl_p->num_entries
		       ,traceControl_p->num_entries - IDXCNT_DELTA(IDXCNT_ADD(traceControl_p->num_entries,10),IDXCNT_ADD(traceControl_p->num_entries,5)) );
		printf("rdIdx_has_lines(IDXCNT_ADD(num_entries,10),IDXCNT_ADD(num_entries,5),-1) = %u\n"
		       ,rdIdx_has_lines( IDXCNT_ADD(traceControl_p->num_entries,10)
			                    ,IDXCNT_ADD(traceControl_p->num_entries,5),-1) );
		printf("(num_entries=%u) - IDXCNT_DELTA(wr=IDXCNT_ADD(num_entries,10+num_entries)=%u,rd=IDXCNT_ADD(num_entries,5)) = %u\n"
		       ,traceControl_p->num_entries
		       ,IDXCNT_ADD(traceControl_p->num_entries,(int)(10+traceControl_p->num_entries))
		       ,traceControl_p->num_entries - IDXCNT_DELTA( IDXCNT_ADD(traceControl_p->num_entries,(int)(10+traceControl_p->num_entries))
			                                               ,IDXCNT_ADD(traceControl_p->num_entries,5) ) );
		printf("rdIdx_has_lines(IDXCNT_ADD(num_entries,10+num_entries),IDXCNT_ADD(num_entries,5),-1) = %u\n"
		       ,rdIdx_has_lines( IDXCNT_ADD(traceControl_p->num_entries,(int)(10+traceControl_p->num_entries))
			                    ,IDXCNT_ADD(traceControl_p->num_entries,5),-1) );

		printf("\n");
		TRACE( 1, "hello %d\n\tthere\n", 1 );
		TRACE( 2, "hello %d %d", 1, 2 );
		TRACE( 3, "hello %d %d %d", 1,2,3 );
		TRACE( 3, "hello %d %d %d %d %d %d %d %d %d %d %d"
			  , 1,2,3,4,5,6,7,8,9,10, 11 );	  /* extra param does not get saved in buffer */
		TRACE( 3, "hello %f %f %f %f %f %f %f %f %f %f"
			  , 1.0,2.0,3.0,4.0, ff[5],6.0,7.0,8.0,9.0,10.0 );
		TRACE( 4, "hello %d %d %f  %d %d %f	 %d %d there is tabs after 2nd float"
			  ,	          1, 2,3.3,4, 5, 6.6, 7, 8 );
		TRACE( 4, TSPRINTF("%s:%%d- int=%%d __FILE__=%s",strrchr(__FILE__,'/')?strrchr(__FILE__,'/')+1:__FILE__,__FILE__)
		      , __LINE__, 5 );
		TRACE( 4, TSPRINTF("%s%s%s",strrchr(__FILE__,'/')?strrchr(__FILE__,'/')+1:__FILE__,":%d- int=%d __FILE__=",__FILE__)
		      , __LINE__, 5 );

#	   ifndef TEST_UNUSED_FUNCTION
		TRACE_CNTL( "trig", -1, 5LL );
#	   endif
		for (ii=0; ii<20; ++ii) {
			if (ii%5 == 0) {
				/* this shows how TRACE (which does not have a "name" argument)
				   could "TRACE with different names." The other trace macros
				   (TRACEN,TRACEN_,TLOG) can't do this. (They can have a name per thread.)
				   This is just a technical detail -- I have yet to think of a
				   real use for this feature/functionality.
				*/
				sprintf(test_name,"%s_%d","TRACE",ii);
				TRACE_CNTL("name", test_name); /* this is a realatively slow operation */
			}
			TRACE( 2, "ii=%u", ii );
		}
	}
	else if (strcmp(cmd,"test-ro") == 0)
	{
		/* To test normal userspace, can use: TRACE_FILE= trace_cntl test-ro
		 */
		setenv("TRACE_FILE","/proc/trace/buffer",0);
		traceInit(NULL,0);
		printf("try write to (presumably kernel memory) write-protected 1st page...\n");
		traceControl_p->trace_initialized = 2;
		printf("write succeeded.\n");
#	   if defined(TEST_WRITE_PAST_END)
		*(((uint8_t*)traceControl_p)+traceControl_p->memlen) = 6;
#	   endif
	}
	else if (strcmp(cmd,"test-compare") == 0)
	{	char      buffer[200];
		uint64_t  mark;
		unsigned long long delta;
		unsigned  loops=DFLT_TEST_COMPARE_ITERS;
		unsigned  compares=1000; /* some big number */
		int       fd;
		int       jj;

		if (opt_loops > -1) loops=(unsigned)opt_loops;

		fd = open( "/dev/null", O_WRONLY );
		dup2( fd, 1 );   /* redirect stdout to /dev/null */
        setlocale(LC_NUMERIC,"en_US");  /* make ' printf flag work -- setting LC_NUMERIC in env does not seem to work */

		TRACE_CNTL("mode",3);
		traceControl_rwp->mode.bits.M = 1;		   // NOTE: TRACE_CNTL("modeM",1) hardwired to NOT enable when not mapped!

#      define STRT_FMT "%-50s"
		// ELF 6/6/18: GCC v6_3_0 does not like %', removing the '...
#	   define END_FMT  "%10llu us (%5.3f us/TRACE)\n",delta,(double)delta/loops
#      define CONTINUE fprintf(stderr,"Continuing with S lvl enabled (stdout redirected to /dev/null).\n");TRACE_CNTL("lvlsetS",4LL);continue
		for (jj=0; jj<4; ++jj) {
			if (argc - optind == 1) compares=strtoul(argv[optind],NULL,0);
			switch (jj) {
			case 0: TRACE_CNTL("lvlsetM",4LL); TRACE_CNTL("lvlclrS",4LL);
				fprintf(stderr,"First testing with S lvl disabled (mem only). loops=%u\n", loops ); break;
			case 1: TRACE_CNTL("lvlsetM",4LL); TRACE_CNTL("lvlsetS",4LL);
				fprintf(stderr,"Testing with M and S lvl enabled (stdout>/dev/null). loops=%u\n", loops ); break;
			case 2: TRACE_CNTL("lvlclrM",4LL); TRACE_CNTL("lvlclrS",4LL);
				fprintf(stderr,"Testing with M and S lvl disabled. loops=%u\n", loops ); break;
			case 3: TRACE_CNTL("lvlclrM",4LL); TRACE_CNTL("lvlsetS",4LL);
				fprintf(stderr,"Testing with just S lvl enabled. Unusual. loops=%u\n", loops ); break;
			}

			fprintf(stderr,STRT_FMT,"const short msg (NO snprintf)");fflush(stderr);
			TRACE_CNTL("reset"); mark = get_us_timeofday();
			for (ii=0; ii<loops; ++ii) {
				TRACE( 2, "any msg" );
			} delta=get_us_timeofday()-mark; fprintf(stderr,END_FMT);

			fprintf(stderr,STRT_FMT,"1 arg");fflush(stderr);
			TRACE_CNTL("reset"); mark = get_us_timeofday();
			for (ii=0; ii<loops; ++ii) {
				TRACE( 2, "this is one small param: %u", 12345678 );
			} delta=get_us_timeofday()-mark; fprintf(stderr,END_FMT);
			if (--compares == 0) { CONTINUE; }

			fprintf(stderr,STRT_FMT,"2 args");fflush(stderr);
			TRACE_CNTL("reset"); mark = get_us_timeofday();
			for (ii=0; ii<loops; ++ii) {
				TRACE( 2, "this is 2 params: %u %u", 12345678, ii );
			} delta=get_us_timeofday()-mark; fprintf(stderr,END_FMT);
			if (--compares == 0) { CONTINUE; }

			fprintf(stderr,STRT_FMT,"8 args (7 ints, 1 float)");fflush(stderr);
			TRACE_CNTL("reset"); mark = get_us_timeofday();
			for (ii=0; ii<loops; ++ii) {
				TRACE( 2, "this is 8 params: %u %u %u %u %u %u %u %g"
				         , 12345678, ii, ii*2, ii+6
				         , 12345679, ii, ii-7, (float)ii*1.5 );
			} delta=get_us_timeofday()-mark; fprintf(stderr,END_FMT);
			if (--compares == 0) { CONTINUE; }

			fprintf(stderr,STRT_FMT,"8 args (1 ints, 7 float)");fflush(stderr);
			TRACE_CNTL("reset"); mark = get_us_timeofday();
			for (ii=0; ii<loops; ++ii) {
				TRACE( 2, "this is 8 params: %u %g %g %g %g %g %g %g"
						 , 12345678, (float)ii, (float)ii*2.5, (float)ii+3.14
						 , (float)12345679, (float)ii/.25, (float)ii-2*3.14, (float)ii*1.5 );
			} delta=get_us_timeofday()-mark; fprintf(stderr,END_FMT);
			if (--compares == 0) { CONTINUE; }

			fprintf(stderr,STRT_FMT,"snprintf of same 8 args");fflush(stderr);
			TRACE_CNTL("reset"); mark = get_us_timeofday();
			for (ii=0; ii<loops; ++ii) {
				snprintf( buffer, sizeof(buffer)
						 , "this is 8 params: %u %g %g %g %g %g %g %g"
						 , 12345678, (float)ii, (float)ii*2.5, (float)ii+3.14
						 , (float)12345679, (float)ii/.25, (float)ii-2*3.14, (float)ii*1.5
						 );
				TRACE( 2, buffer );
			} delta=get_us_timeofday()-mark; fprintf(stderr,END_FMT);
			if (--compares == 0) { CONTINUE; }

			fprintf(stderr,STRT_FMT,"(repeat) 1st test - const short msg (NO snprintf)");fflush(stderr);
			TRACE_CNTL("reset"); mark = get_us_timeofday();
			for (ii=0; ii<loops; ++ii) {
				TRACE( 2, "any msg" );
			} delta=get_us_timeofday()-mark; fprintf(stderr,END_FMT);
		}
	}
#	ifdef DO_THREADS   /* requires linking with -lpthreads */
	else if (strcmp(cmd,"test-threads") == 0)
	{	pthread_t *threads;
		unsigned   num_threads=NUMTHREADS;
		args_t    *argsp, args0;
		int        loops=2;
		pid_t      main_tid=trace_gettid();
		if (opt_loops > -1)
			loops=opt_loops;
		if (opt_burst == 0) {
			printf("adjusting burst==0 to min burst=1\n");
			opt_burst=1;
		}
		if ((argc-optind)==1)
		{	num_threads=strtoul(argv[optind],NULL,0);
			if (num_threads > 4095) num_threads=4095;
		}
		threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
		argsp   =    (args_t*)malloc(num_threads*sizeof(args_t));

		TRACE( 1, "before pthread_create - main_tid=%u loops=%d, threads=%u, dly_ms=%u traceControl_p=%p"
		      , main_tid, loops, num_threads, opt_dly_ms, (void*)traceControl_p );
		for (ii=0; ii<num_threads; ii++) {
			argsp[ii].tidx   = ii+1;
			argsp[ii].loops  = loops;
			argsp[ii].dly_ms = opt_dly_ms;
			argsp[ii].burst  = opt_burst;
			pthread_create(&(threads[ii]),NULL,thread_func,(void*)&argsp[ii] );
		}

		args0.tidx   = 0;
		args0.loops  = loops;
		args0.dly_ms = opt_dly_ms;
		args0.burst  = opt_burst;
		thread_func( &args0 );

		if (trace_thread_option & 4)
		{   char          cmd[200];
		    sprintf( cmd, "echo trace_buffer mappings before join '(#1)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    system( cmd );
		    sprintf( cmd, "echo trace_buffer mappings before join '(#2)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    system( cmd );
		}
		for (ii=0; ii<num_threads; ii++)
		{	pthread_join(threads[ii], NULL);
		}
		if (trace_thread_option & 4)
		{   char          cmd[200];
		    sprintf( cmd, "echo trace_buffer mappings after join '(#1)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    system( cmd );
		    sprintf( cmd, "echo trace_buffer mappings after join '(#2)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    system( cmd );
		}
		TRACE( 1, "after pthread_join - main_tid=%u traceControl_p=%p", main_tid, (void*)traceControl_p );
		free( threads );
	}
#	endif
	else if (strcmp(cmd,"show") == 0)
	{
		const char *ospec=getenv("TRACE_SHOW");
		if (!ospec) ospec=DFLT_SHOW;
		if ((do_heading==0) && (strncmp("%H",ospec,2)==0)) ospec+=2;
		traceShow(ospec,opt_count,opt_start,show_opts, argc-optind, &argv[optind]);
	}
	else if (strncmp(cmd,"info",4) == 0)
	{
		traceInit(NULL,0);
		traceInfo();
	}
	else if (strcmp(cmd,"unlock") == 0)
	{	traceInit(NULL,0);
		trace_unlock( &traceControl_rwp->namelock );
	}
	else if (strcmp(cmd,"tids") == 0)
	{	uint32_t longest_name;
		int      namLvlTblEnts_digits;
		traceInit(NULL,0);
		longest_name = traceControl_rwp->longest_name;
		/*printf("longest_name=%d\n",longest_name);*/
		namLvlTblEnts_digits=countDigits(traceControl_p->num_namLvlTblEnts-1);
		if (do_heading) {
			printf( "mode:%*s%*s            M=%d                S=%d\n"
			       , minw(3,namLvlTblEnts_digits), ""
			       , longest_name, ""
			       , traceControl_rwp->mode.bits.M, traceControl_rwp->mode.bits.S );
			printf("%*s %*s %*s %*s %*s\n"
			       , minw(3,namLvlTblEnts_digits), "TID"
			       , longest_name, "NAME"
			       , 18, "maskM", 18, "maskS", 18, "maskT" );
			printf("%.*s %.*s %.*s %.*s %.*s\n"
			       , minw(3,namLvlTblEnts_digits), TRACE_LONG_DASHES
			       , longest_name, TRACE_LONG_DASHES
			       , 18, TRACE_LONG_DASHES, 18, TRACE_LONG_DASHES
			       , 18, TRACE_LONG_DASHES );
		}
		for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
		{
			if (opt_all || traceNamLvls_p[ii].name[0]!='\0')
			{	printf("%*d %*.*s 0x%016llx 0x%016llx 0x%016llx\n"
				       , minw(3,namLvlTblEnts_digits), ii
					   , longest_name
					   , longest_name
				       , traceNamLvls_p[ii].name
				       , (unsigned long long)traceNamLvls_p[ii].M
				       , (unsigned long long)traceNamLvls_p[ii].S
				       , (unsigned long long)traceNamLvls_p[ii].T
				       );
			}
		}
	}
	else if (strcmp(cmd,"TRACE") == 0) {
#   define ARG(n) strchr(argv[optind+n],'.')?strtod(argv[optind+n],0):strtoul(argv[optind+n],0,0)
		switch (argc - optind) {
		case 0: printf("\"trace\" cmd requires at least lvl and fmt arguments."); break;
		case 1: printf("\"trace\" cmd requires at least lvl and fmt arguments."); break;
		case 2: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1] );
			break;
		case 3: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0) );
			break;
		case 4: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0) );
			break;
		case 5: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0) );
			break;
		case 6: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0) );
			break;
		case 7: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0)
		              ,strtoul(argv[optind+6],NULL,0) );
			break;
		case 8: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0)
		              ,strtoul(argv[optind+6],NULL,0),strtoul(argv[optind+7],NULL,0) );
			break;
		case 9: TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0)
		              ,strtoul(argv[optind+6],NULL,0),strtoul(argv[optind+7],NULL,0)
		              ,strtoul(argv[optind+8],NULL,0) );
			break;
		case 10:TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0)
		              ,strtoul(argv[optind+6],NULL,0),strtoul(argv[optind+7],NULL,0)
		              ,strtoul(argv[optind+8],NULL,0),strtoul(argv[optind+9],NULL,0) );
			break;
		case 11:TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0)
		              ,strtoul(argv[optind+6],NULL,0),strtoul(argv[optind+7],NULL,0)
		              ,strtoul(argv[optind+8],NULL,0),strtoul(argv[optind+9],NULL,0)
		              ,strtoul(argv[optind+10],NULL,0) );
			break;
		case 12:TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0)
		              ,strtoul(argv[optind+6],NULL,0),strtoul(argv[optind+7],NULL,0)
		              ,strtoul(argv[optind+8],NULL,0),strtoul(argv[optind+9],NULL,0)
		              ,strtoul(argv[optind+10],NULL,0),strtoul(argv[optind+11],NULL,0) );
			break;
		case 13:TRACE( strtoul(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoul(argv[optind+2],NULL,0),strtoul(argv[optind+3],NULL,0)
		              ,strtoul(argv[optind+4],NULL,0),strtoul(argv[optind+5],NULL,0)
		              ,strtoul(argv[optind+6],NULL,0),strtoul(argv[optind+7],NULL,0)
		              ,strtoul(argv[optind+8],NULL,0),strtoul(argv[optind+9],NULL,0)
		              ,strtoul(argv[optind+10],NULL,0),strtoul(argv[optind+11],NULL,0)
		              ,strtoul(argv[optind+12],NULL,0) );
			break;
		default:
			printf( "oops - only able to test/handle up to 11 TRACE params/args - sorry.\n" );
		}
	}
	else if (strcmp(cmd,"TRACEN") == 0) {
		switch (argc - optind) {
		case 0: printf("\"trace\" cmd requires at least name, lvl and fmt arguments."); break;
		case 1: printf("\"trace\" cmd requires at least name, lvl and fmt arguments."); break;
		case 2: printf("\"trace\" cmd requires at least name, lvl and fmt arguments."); break;
		case 3: TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2] );
			break;
		case 4: TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		               ,strtoul(argv[optind+3],NULL,0) );
			break;
		case 5: TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		               ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0) );
			break;
		case 6: TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0) );
			break;
		case 7: TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0) );
			break;
		case 8: TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0)
		              ,strtoul(argv[optind+7],NULL,0) );
			break;
		case 9: TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0)
		              ,strtoul(argv[optind+7],NULL,0),strtoul(argv[optind+8],NULL,0) );
			break;
		case 10:TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0)
		              ,strtoul(argv[optind+7],NULL,0),strtoul(argv[optind+8],NULL,0)
		              ,strtoul(argv[optind+9],NULL,0) );
			break;
		case 11:TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0)
		              ,strtoul(argv[optind+7],NULL,0),strtoul(argv[optind+8],NULL,0)
		              ,strtoul(argv[optind+9],NULL,0),strtoul(argv[optind+10],NULL,0) );
			break;
		case 12:TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0)
		              ,strtoul(argv[optind+7],NULL,0),strtoul(argv[optind+8],NULL,0)
		              ,strtoul(argv[optind+9],NULL,0),strtoul(argv[optind+10],NULL,0)
		              ,strtoul(argv[optind+11],NULL,0) );
			break;
		case 13:TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0)
		              ,strtoul(argv[optind+7],NULL,0),strtoul(argv[optind+8],NULL,0)
		              ,strtoul(argv[optind+9],NULL,0),strtoul(argv[optind+10],NULL,0)
		              ,strtoul(argv[optind+11],NULL,0),strtoul(argv[optind+12],NULL,0) );
			break;
		case 14:TRACEN( argv[optind],strtoul(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoul(argv[optind+3],NULL,0),strtoul(argv[optind+4],NULL,0)
		              ,strtoul(argv[optind+5],NULL,0),strtoul(argv[optind+6],NULL,0)
		              ,strtoul(argv[optind+7],NULL,0),strtoul(argv[optind+8],NULL,0)
		              ,strtoul(argv[optind+9],NULL,0),strtoul(argv[optind+10],NULL,0)
		              ,strtoul(argv[optind+11],NULL,0),strtoul(argv[optind+12],NULL,0)
		              ,strtoul(argv[optind+13],NULL,0) );
			break;
		default:
			printf( "oops - only able to test/handle up to 11 TRACEN params/args - sorry.\n" );
		}
	}
	else if (strncmp(cmd,"sleep",4) == 0) { /* this allows time to look at /proc/fd/ and /proc/maps */
		TRACE( 1, "starting sleep" );
		if ((argc-optind) == 1)
			sleep( strtoul(argv[optind],NULL,0) );
		else sleep( 10 );
		TRACE( 1, "done sleeping" );
	}
	/* now deal with traceCntl commands using TRACE_CNTL */
	else if (strncmp(cmd,"mode",4) == 0) { /* mode is special -- may _return_ old mode */
		if ((argc-optind) == 0) {
			ret=TRACE_CNTL( cmd );
			printf( "%d\n",ret ); /* print the old mode */
		}
		else ret=TRACE_CNTL( cmd, strtoul(argv[optind],NULL,0) );
		if (ret == -1) ret=1; else ret=0;
	} else if (  (strncmp(cmd,"lvlmsk",6)==0)
	           ||(strncmp(cmd,"lvlset",6)==0)
	           ||(strncmp(cmd,"lvlclr",6)==0)) {
		/* lvl msk,set,clr are special primarily in that the -n<name> will no
		   longer create a non-existent name b/c it could be a wildcard spec.
		   Way to create a non-existent name:
		   trace_cntl mode -n<name>
		   TRACE_NAME=<name> trace_cntl lvlsetM <msk>
           others? */
		int sts=0, slen=strlen(&cmd[6]);
		if (opt_name) {
			char cmdn[8];
			if (slen>1 || (slen==1&&!strpbrk(&cmd[6],"MST"))) {
				printf("invalid command: %s\n", cmd ); printf( USAGE );
			}
			strncpy(cmdn,cmd,6);
			cmdn[6]='n';strcpy(&cmdn[7],&cmd[6]); /* insert the 'n' */
			switch (argc - optind) {
			case 0: sts=TRACE_CNTL( cmdn, opt_name );
				printf("mask=0x%x\n",sts);
				break;
			case 1: sts=TRACE_CNTL( cmdn, opt_name, strtoull(argv[optind],NULL,0) );
				/*printf("previous mask=0x%x\n",sts);*/
				break;
			case 3: TRACE_CNTL( cmdn, opt_name, strtoull(argv[optind],NULL,0)
			                   , strtoull(argv[optind+1],NULL,0)
			                   , strtoull(argv[optind+2],NULL,0) );
				break;
			default:
				printf("invalid command: %s\n", cmd ); printf( USAGE );
				break;
			}
		} else {
			if (slen>2 || (slen>=1&&!strpbrk(&cmd[6],"MSTgG"))) {
				printf("invalid command: %s\n", cmd );printf( USAGE );
			}
			switch (argc - optind) {
			case 0: sts=TRACE_CNTL( cmd );
				printf("mask=0x%x\n",sts);
				break;
			case 1: sts=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0) );
				break;
			case 3: TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0)
			                   , strtoull(argv[optind+1],NULL,0)
			                   , strtoull(argv[optind+2],NULL,0) );
				break;
			default:
				printf("invalid command: %s\n", cmd ); printf( USAGE ); break;
			}
		}
	}
	else { /* all other traceCntl commands */
		int sts=0;
		/*printf("argc - optind = %d\n", argc - optind );*/
		switch (argc - optind) {
		case 0: sts=TRACE_CNTL( cmd ); break;
		case 1: sts=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0) ); break;
		case 2: sts=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0)
		                       , strtoull(argv[optind+1],NULL,0) ); break;
		case 3: sts=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0)
		                       , strtoull(argv[optind+1],NULL,0)
		                       , strtoull(argv[optind+2],NULL,0) ); break;
		}
		if (sts < 0) {
			printf("invalid command: %s\n", cmd );
			printf( USAGE );
			ret=1;
		} else if (sts==0)
			ret=1;
		else
			ret=0;
	}
	return (ret);
}	/* main */
