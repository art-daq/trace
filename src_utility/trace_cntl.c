/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    */
#define TRACE_CNTL_REV "$Revision: 1432 $$Date: 2020-10-23 12:46:38 -0500 (Fri, 23 Oct 2020) $"
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
 show [opts] [file]...   # Note: -s option invalid with multiple files\n\
 info, tids\n\
 cntl <int>              # __func__ prepended to memory msg - 1=always, 0=TRACE_PRINT %%F, -1=never\n\
 mode <mode>\n\
 modeM <mode>\n\
 modeS <mode>\n\
 lvlmsk[M|S|T][g] <lvlmsk>         # M=memory, S=slow/console, T=trig\n\
 lvlmsk[g|G] <mskM> <mskS> <mskT>  # g=current and future name slots; G=only active (not empty)\n\
 lvlset[g|G] <mskM> <mskS> <mskT>  # g=current and future name slots; G=only active (not empty)\n\
 lvlclr[g|G] <mskM> <mskS> <mskT>  # g=current and future name slots; G=only active (not empty)\n\
 lvlstrs                           # print the lvlstrs arrary\n\
 trig <postEntries> [lvlmskTM+] [quiet] # opt 2nd arg is for specified/dflt TID\n\
 reset\n\
 limit_ms <cnt> <on_ms> <off_ms>\n\
 unlock\n\
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
 -t         print timing stats to stderr\n\
\n\
" USAGE_TESTS, basename(argv[0]), basename(argv[0]), DFLT_TEST_COMPARE_ITERS, /*USAGE_TESTS PARAMS*/ NUMTHREADS
#define USAGE_TESTS "\
 test1 [-lloops]  a single TRACE lvl 2 [or more if loops != 0]\n\
 test           various\n\
 test-ro        test first page of mmap read-only (for kernel module)\n\
 test-compare [-lloops] [test] [modes]  compare TRACE fmt+args vs. format+args converted (via sprintf). dflt loops=%d\n\
                for ff in '' ' %%F:';do TRACE_PRINT=\"%%T %%n %%*L$ff %%M\" tcntl test-compare 1 2;done\n\
 test-threads [-t] [-x<thread_opts>] [-lloops] [-bburst] [-ddly_ms] [num_threads]  Tests threading. loops\n\
                       of TRACEs: 2-info's, 1-debug, 1-dbg+1, 1-dbg+2  dflts: -x0 -l2 num_threads=%d\n\
  example:\n\
  for tt in `seq 0 6`;do export TRACE_NUMENTS=500000\\\n\
   printf \"$tt: \";rm -f /tmp/trace_buffer_$USER;toffS info -ntrace_cntl;tcntl test-threads $tt -tl3000000;\\\n\
  done\n\
 TRACE [-t,-l<loops>,--] <lvl> <fmt> [ulong or double]...   (double if contains any of \".INnPp\")\n\
"

// Started by copying macros from trace.h and adjusting (adding va_list ap = TRACE_VA_LIST_INIT(arrblk) in particular)
#define VTRACE(lvl, nargs, msg, arrblk)									\
	do{																	\
		struct { char tn[TRACE_TN_BUFSZ]; } _trc_;						\
		if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__FILE__,_trc_.tn,sizeof(_trc_.tn))){ \
			struct timeval lclTime;                                                                                                   \
			uint8_t lvl_ = (uint8_t)(lvl);								\
			TRACE_SBUFDECL;                                                                                                                \
			lclTime.tv_sec = 0;                                                                                                       \
			if (traceControl_rwp->mode.bits.M && (traceLvls_p[traceTID].M & TLVLMSK(lvl_))){ \
				/* Note: "...NARGS...+2" don't know if any args are long doubles, so support 2 on 64 and 4 on 32 bit archs */ \
				va_list ap = TRACE_VA_LIST_INIT(arrblk);				\
				vtrace(&lclTime, traceTID, lvl_, __LINE__, __func__, nargs, msg, ap); \
			}                                                                                                                         \
			if (traceControl_rwp->mode.bits.S && (traceLvls_p[traceTID].S & TLVLMSK(lvl_))){ \
				TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime){				\
					va_list ap = TRACE_VA_LIST_INIT(arrblk);			\
					vtrace_user(&lclTime, traceTID, lvl_, _insert, __FILE__, __LINE__, __func__, nargs, msg, ap); \
				}                                                                                                                     \
			}                                                                                                                         \
		}                                                                                                                             \
	} while (0)

#define VTRACEN(nam, lvl, nargs, msg, arrblk)								\
	do{																	\
		struct { char tn[TRACE_TN_BUFSZ];	} _trc_;					\
		if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__FILE__,_trc_.tn,sizeof(_trc_.tn))){ \
			static TRACE_THREAD_LOCAL int tid_ = -1;                                                                              \
			struct timeval lclTime;                                                                                               \
			uint8_t lvl_ = (uint8_t)(lvl);								\
			TRACE_SBUFDECL;													\
			if (tid_ == -1) tid_ = (int)trace_name2TID(&(nam)[0]);			\
			lclTime.tv_sec = 0;                                                                                                   \
			if (traceControl_rwp->mode.bits.M && (traceLvls_p[tid_].M & TLVLMSK(lvl_))){ \
				/* Note: "...NARGS...+2" don't know if any args are long doubles, so support 2 on 64 and 4 on 32 bit archs */ \
				va_list ap = TRACE_VA_LIST_INIT(arrblk);				\
				vtrace(&lclTime, tid_, lvl_, __LINE__, __func__, nargs, msg, ap); \
			}                                                                                                                     \
			if (traceControl_rwp->mode.bits.S && (traceLvls_p[tid_].S & TLVLMSK(lvl_))){ \
				TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime){				\
					va_list ap = TRACE_VA_LIST_INIT(arrblk);			\
					vtrace_user(&lclTime, tid_, lvl_, _insert, __FILE__, __LINE__, __func__, nargs, msg, ap); \
				}                                                                                                                 \
			}                                                                                                                     \
		}                                                                                                                         \
	} while (0)

unsigned long *add_ul_arg(unsigned long *args_ptr, unsigned long *end_ptr, unsigned long arg)
{
	unsigned long *ret_val=args_ptr;
	unsigned long *vp = (unsigned long*)args_ptr;
	if ((vp+1) <= (unsigned long*)end_ptr){
		*vp++ = arg;
		ret_val = (unsigned long*)vp;
	}
	return (ret_val);
}

unsigned long *add_double_arg(unsigned long *args_ptr, unsigned long *end_ptr, double arg)
{
	unsigned long *ret_val=args_ptr;
	double *vp = (double*)args_ptr;
	if ((vp+1) <= (double*)end_ptr){
		*vp++ = arg;
		ret_val = (unsigned long*)vp;
	}
	return (ret_val);
}


#define DFLT_TEST_COMPARE_ITERS 500000
#define minw(a,b) ((b)<(a)?a:b)     /* min width -- really max of the 2 numbers - argh */

#ifdef __linux__
//# define DFLT_SHOW         "HxNTPiCnLR"
# define DFLT_SHOW         "%H%x%N %T %P %i %C %n %.3L %m"
#else
//# define DFLT_SHOW         "HxNTPinLR"
# define DFLT_SHOW         "%H%x%N %T %P %i %n %.3L %m"
#endif

#define NUMTHREADS 4
static int trace_thread_option=0;

typedef struct {
	unsigned tidx;
	unsigned loops;
	unsigned dly_ms;
	unsigned burst;
} args_t;

void* thread_func(void *arg)
{
	args_t *argsp=(args_t*)arg;
	unsigned tidx  =argsp->tidx;
	unsigned dly_ms=argsp->dly_ms;
	unsigned burst =argsp->burst;
	unsigned loops =argsp->loops;
	unsigned lp;
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
		strcpy( tmp, "TEST_THR" );

	for (lp=0; lp<loops; ) {
		/* NOTE: be mindful of classic issue with macro -- args could be evaluted multiple times! */
		/* BUT -- when TRACE-ing a value, there you would not likely operate on it */
		++lp;
		TRACE( TLVL_INFO, "tidx=%u loop=%d of %d tid=%d I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message mangling", tidx, lp, loops, tid );
		TRACE( TLVL_INFO, "tidx=%u loop=%d of %d tid=%d this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9", tidx, lp, loops, tid );
		TRACE( TLVL_DEBUG, "tidx=%u loop=%d of %d tid=%d this is the third long message - third0 third1 third2 third3 third4 third5 third6 third7 third8 third9", tidx, lp, loops, tid );
		TRACE( TLVL_DBG+1, "tidx=%u loop=%d of %d tid=%d this is the fourth long message - fourth0 fourth1 fourth2 fourth3 fourth4 fourth5 fourth6 fourth7 fourth8 fourth9", tidx, lp, loops, tid );
		TRACEN(tmp,TLVL_DBG+2,"tidx=%u loop=%d of %d tid=%d - extra, if enabled", tidx, lp, loops, tid );
		if(dly_ms && (lp%burst)==0)
			usleep( dly_ms*1000 );
	}
	if (argsp->tidx)
		pthread_exit(NULL);
	else
		return (NULL);
} /* thread_func */


static uint64_t gettimeofday_us()
{	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (uint64_t)(tv.tv_sec*1000000+tv.tv_usec);
}

/* This function searches the enum spec. One may think - just search the trace_lvlstrs[] array,
   but the issue is, some levels in the enum may have alternate designations.
   In the end, search both -- 1st, the enum spec and 2nd, the trace_lvlstrs
   which could be filled from the enum spec or the TRACE_LVLSTRS env.var.
 */
int str2enum(const char *ss)
{
	char  lvlstr_a[]=TRACE_STR(TRACE_LVL_ENUM_0_9,TRACE_LVL_ENUM_10_63); /* the code below assumes the input compiles as an enum spec */
	char *lvlstr=lvlstr_a;
	int   retidx;
	char *endptr, *savptr, *cp;
	if (strncmp(ss,"TLVL_",5)==0 && (cp=getenv(ss)) && (*cp)){
		int val=(int)strtoul(cp,&endptr,0);
		if (*endptr == '\0')
			return (val);
	}
	for (retidx=0; lvlstr; ++retidx,lvlstr=endptr) {
		savptr = endptr = strpbrk(lvlstr,",= ");
		if (endptr){
			if(*endptr == '=')
				retidx=(int)strtoul(endptr+1,(char**)&endptr,0); /* works only if value is not a previous enum enumerator */
			while (isspace(*endptr)) ++endptr; /* space before ',' */
			++endptr;  /* must be ',' */
			while (isspace(*endptr))++endptr; /* space after ',' */
			*savptr='\0';		/* terminate to allow strcmp */
		}
		if (strcasecmp(ss,lvlstr) == 0) return (retidx);
	}
	for (retidx=0; retidx<64; ++retidx){
		if (strcasecmp(ss,trace_lvlstrs[0][retidx]) == 0)
			break;
	}
	if (retidx == 64)
		retidx = TLVL_ERROR;
	return (retidx);
}/* str2enum */

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
	unsigned numArgs=0;
	unsigned maxArgs=(nargs<traceControl_p->num_params?nargs:traceControl_p->num_params);
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
			case -2: sizes_out[numArgs].push=(unsigned short)param_bytes;sizes_out[numArgs].size=4; break; /* char */
			case -1: sizes_out[numArgs].push=(unsigned short)param_bytes;sizes_out[numArgs].size=4;break;/* short */
			case 0:	 sizes_out[numArgs].push=(unsigned short)param_bytes;sizes_out[numArgs].size=4;break;/* int */
			case 1:	 sizes_out[numArgs].push=(unsigned short)param_bytes;sizes_out[numArgs].size=(unsigned short)param_bytes;/* long */
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
			sizes_out[numArgs].push=(unsigned short)param_bytes; sizes_out[numArgs].size=(unsigned short)param_bytes;
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
			sizes_out[numArgs].push=(unsigned short)param_bytes;sizes_out[numArgs].size=4/*int*/;
			sizes_out[numArgs].positive = 1;
			if (++numArgs == maxArgs) { ofmt[slen++] = *in++; break; }
			ofmt[slen++] = *in++;
			goto chkChr;
		default:
			if( !(opts&quiet_))
				printf("tshow: unknown format spec char \"%c\" encountered with nargs=%u.\n", *in, nargs);
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

void printEnt(  const char *ospec, int opts, struct traceEntryHdr_s* myEnt_p
              , char *local_msg, uint8_t *local_params, struct sizepush *params_sizes
              , int bufSlot_width, int N_width, int name_width, unsigned printed, uint32_t rdIdx
              , char *tfmt, int tfmt_len
              )
{
	unsigned                uu;
	char                  * msg_p;
	uint64_t              * params_p;
	size_t                  slen;
	time_t                  seconds;
	int                     useconds;
	void                  * param_va_ptr;
	uint8_t               * ent_param_ptr;
	uint8_t               * lcl_param_ptr;
	const char            * sp; /*spec ptr*/
	int                     msg_spec_included;
	char                    *cp;
	char                  * indent="                                                                ";

		msg_p	 = (char*)(myEnt_p+1);
		params_p = (uint64_t*)(msg_p+traceControl_p->siz_msg);
		
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
			useconds = (int)myEnt_p->time.tv_usec;
			param_va_ptr = (void*)params_p;
		}
		else if (  ((myEnt_p->param_bytes==4) && (sizeof(long)==8)) )
		{	// Entry made by 32bit program, being readout by 64bit
			void *xx = &(myEnt_p->time);
			int *ptr=(int*)xx;
			seconds	 = *ptr++;
			useconds = (int)*ptr;
			ent_param_ptr = (uint8_t*)params_p;
			lcl_param_ptr = local_params;
			for (uu=0; uu<myEnt_p->nargs && params_sizes[uu].push!=0; ++uu)
			{
				if      (params_sizes[uu].push == 4)
				{	if (params_sizes[uu].positive) {
						*(long*)lcl_param_ptr = (long)*(int*)ent_param_ptr;
						//printf( "converting from 0x%x\n", *(int*)ent_param_ptr );
					} else {
						*(unsigned long*)lcl_param_ptr = (unsigned long)*(unsigned*)ent_param_ptr;
						//printf( "converting to 0x%lx\n", *(unsigned long*)lcl_param_ptr );
					}
					lcl_param_ptr += sizeof(long);
				}
				else if (params_sizes[uu].push == 12) // i.e. long double
				{
# if defined(__arm__) || defined(__powerpc__)
					*(long double*)lcl_param_ptr = 0.0; // __arm__: error: unable to emulate 'XC'; 
# else
					typedef _Complex float __attribute__((mode(XC))) _float80;
					*(long double*)lcl_param_ptr = *(long double*)(_float80*)ent_param_ptr;
# endif
					lcl_param_ptr += sizeof(long double);
				}
				else /* (params_sizes[uu].push == 8) */
				{	*(long*)lcl_param_ptr =		 *(long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long);
				}
				ent_param_ptr += params_sizes[uu].push;
			}
			param_va_ptr = (void*)local_params;
		}
		else /* (  ((myEnt_p->param_bytes==8) && (sizeof(long)==4)) ) */
		{	// Entry made by 64bit program, being shown by 32bit
			void *xx=&myEnt_p->time;
			long long *ptr=(long long*)xx;
			seconds	 = (time_t)*ptr++;
			useconds = (int)*ptr;
			ent_param_ptr = (uint8_t*)params_p;
			lcl_param_ptr = local_params;
			for (uu=0; uu<myEnt_p->nargs && params_sizes[uu].push!=0; ++uu)
			{
				if      (params_sizes[uu].size == 4) // i.e. char, short, int
				{	*(unsigned*)lcl_param_ptr = (unsigned)*(unsigned long long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long);
				}
				else if (params_sizes[uu].size == 16) // i.e. long double
				{
# if defined(__APPLE__) || defined(__arm__) || defined(__powerpc__)  // Basically mixed 32/64 env w/ long double is not supported on Mac/clang; shouldn't be a big deal
					*(long double*)lcl_param_ptr = 0.0;
# else
					*(long double*)lcl_param_ptr = *(long double*)(__float128*)ent_param_ptr;
# endif
					lcl_param_ptr += sizeof(long double);
				}
				else // (params_sizes[uu].size == 8) i.e. 
				{	*(unsigned long long*)lcl_param_ptr = *(unsigned long long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long long);
				}
				ent_param_ptr += params_sizes[uu].push;
			}
			param_va_ptr = (void*)local_params;
		}

		msg_spec_included = 0;
		for (sp=ospec; *sp; ++sp) {
			char tbuf[0x100];
			int width_state, width_ia[2]= {0}; /* allow up to 3 widths w[.w] */
			char width_ca[2][9];               /* need to save the string for leading "0" (e.g. "-04"*/
			char flags_ca[4];                  /*  */
			size_t flags_sz;
			//const char *default_unknown_sav;
			char *endptr;
			
			if (*sp != '%') {
				printf("%c",*sp );
				continue;
			}
			if (*++sp =='\0') {
				printf("%%"); // print the trailing/ending %
				break;
			}

			/* look for a width/flag spec - see trace.h:vtrace_user(...)  */
			flags_sz= strspn(sp, "#*."); /* originally:"#-+ '*."      %#n.mf#/src#  */
			if (flags_sz) {
				snprintf(flags_ca, TRACE_MIN(flags_sz + 1, sizeof(flags_ca)), "%s", sp);  // snprintf always terminates
				sp+= flags_sz;                                                            // use just flags_sz here to ignore wacky +++++++
			} else flags_ca[0]='\0';
			for (width_state= 0; width_state < 2; ++width_state) {
				width_ia[width_state]= (int)TRACE_STRTOL(sp, &endptr, 10);
				if (endptr == sp || endptr > (sp + 4)) { /* check if no num or num too big (allow "-099") */
					width_ca[width_state][0] = '\0';
					break;
				}
				snprintf(width_ca[width_state], sizeof(width_ca[0]), "%.*s", (int)(endptr - sp), sp);
				sp= endptr;
				if (*sp == '.')
					++sp;
			}

			switch (*sp) {
			case '%': printf("%%"); break;
			case 'a': printf("%4u", myEnt_p->nargs); break;
			case 'B': printf("%u", myEnt_p->param_bytes); break;
			case 'C': printf("%" TRACE_STR(TRACE_CPU_WIDTH) "u", myEnt_p->cpu); break;
			case 'D': /* ignore this "control" */ break;
			case 'e':
				snprintf(tbuf,sizeof(tbuf),"%s:%d",TRACE_TID2NAME(myEnt_p->TrcId),myEnt_p->linenum);
				if (!width_state) printf("%*.*s",name_width+1+TRACE_LINENUM_WIDTH,name_width+1+TRACE_LINENUM_WIDTH,tbuf);
				else {
					if (width_ia[0]<TRACE_LINENUM_WIDTH) width_ia[0]=TRACE_LINENUM_WIDTH;
					printf("%*.*s",name_width+1+width_ia[0],name_width+1+width_ia[0],tbuf);
				}
				break;
			case 'f': printf("%s",local_msg); msg_spec_included=1; break;
			case 'H': /* ignore this "control" */ break;
			case 'I': printf("%4u", myEnt_p->TrcId); break;
			case 'i': printf("%*d", TRACE_TID_WIDTH, myEnt_p->tid); break; /* darwin 16 tids are routinely 7 digit */
			case 'L': {
				char altbuf[0x100], l3buf[4], *lvlcp=trace_lvlstrs[0][(myEnt_p->lvl) & TLVLBITSMSK];
				int  vwidth;
				if (trace_build_L_fmt( tbuf, altbuf, l3buf, &lvlcp, &vwidth, flags_ca, width_ia, width_ca ))
					printf(tbuf, vwidth, lvlcp);
				else
					printf(tbuf, lvlcp);
			} break;
			case 'l': printf("%2d", myEnt_p->lvl); break;
			case 'm':
				if (local_msg[slen-1] == '\n')
					local_msg[--slen] = '\0'; // strip off the trailing newline
				if (opts&indent_)
					printf("%s",&indent[TLVLBITSMSK-(myEnt_p->lvl&TLVLBITSMSK)]);
				if (myEnt_p->nargs) {
					va_list ap_=TRACE_VA_LIST_INIT(param_va_ptr);
					vprintf( local_msg, ap_ );
				} else
					printf("%s",local_msg);				
				msg_spec_included=1;
				break;
			case 'N': printf("%*u", N_width, printed ); break;
			case 'n': printf("%*.*s",name_width,name_width,TRACE_TID2NAME(myEnt_p->TrcId));break;
			case 'O': printf("%s",trace_lvlcolors[(myEnt_p->lvl)&TLVLBITSMSK][0]); break;
			case 'o': printf("%s",trace_lvlcolors[(myEnt_p->lvl)&TLVLBITSMSK][1]); break;
			case 'P': printf("%*d", TRACE_TID_WIDTH, myEnt_p->pid); break; /* /proc/sys/kernel/pid_max has 32768 or 458752 (on mu2edaq13) */
			case 'R':
				if (myEnt_p->get_idxCnt_retries) printf( "%u", myEnt_p->get_idxCnt_retries );
				else							 printf( "." );
				break;
			case 'S': printf("%c", trace_lvlstrs[0][(myEnt_p->lvl)&TLVLBITSMSK][0]); break;
			case 's': printf("%*u", bufSlot_width, rdIdx%traceControl_p->num_entries); break;
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
						  useconds = (int)((double)useconds/div+0.5); // div, round and cast back to unsigned
					  }
					  printf(tbuf, useconds); } break;
			case 't': printf("%10u", (unsigned)myEnt_p->tsc); break;
			case 'u':
				if (!width_state) printf("%" TRACE_STR(TRACE_LINENUM_WIDTH) "u", myEnt_p->linenum);
				else              printf("%*u", width_ia[0], myEnt_p->linenum);
				break;
			case 'x': /* ignore this "control" */ break;
			case 'X': params_p=(uint64_t*)param_va_ptr;
				      for (uu=0; uu<myEnt_p->nargs; ++uu) printf( "0x%016llx ", (unsigned long long)params_p[uu] );
					  msg_spec_included=1;
					  break;
			default:  printf("%%%c",*sp);break; // print unknown %<blah> sequence
			}
		}

		if (!msg_spec_included) {
			if (local_msg[slen-1] == '\n')
				local_msg[--slen] = '\0'; // strip off the trailing newline
			if (opts&indent_)
				printf("%s",&indent[TLVLBITSMSK-(myEnt_p->lvl&TLVLBITSMSK)]);
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
	struct traceLvls_s     *lvls_p;
	char                   *nams_p;
	struct traceEntryHdr_s *entries_p;
	uint32_t                rdIdx;
	uint32_t                max;
	struct timeval          ref_tv;
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
	trace_ptrs[idx].lvls_p = traceLvls_p;
	trace_ptrs[idx].nams_p = traceNams_p;
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
	traceLvls_p      = tptr->lvls_p;
	traceNams_p      = tptr->nams_p;
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
			lines = wrIdxCnt - TRACE_IDXCNT_DELTA(wrIdxCnt,rdIdx+1);
			//return ((lines<=wrIdxCnt)?lines:0);
			return (lines);
		} else {
			lines = traceControl_p->num_entries - TRACE_IDXCNT_DELTA(wrIdxCnt,rdIdx+1);
			//return ((lines<=traceControl_p->num_entries)?lines:0);
			return (lines);
		}
	} else {
		// READING forward (i.e from 0 up; read chasing writing; writing may overrun reading)
		if (wrIdxCnt <= traceControl_p->num_entries) {
			lines = TRACE_IDXCNT_DELTA(wrIdxCnt,rdIdx);
			//return ((lines<=wrIdxCnt)?lines:0);
			return (lines);
		} else {
			lines = TRACE_IDXCNT_DELTA(wrIdxCnt,rdIdx);
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
	uint32_t max;
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
	uint32_t                name_width=0;
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
	opts |= (trace_strflg(ospec,'D')?indent_:0);
	opts |= (trace_strflg(ospec,'x')?filter_newline_:0);

	if (argc == 0) {
		traceInit(NULL,1); /* init traceControl_p, traceControl_rwp, etc. */
		trace_ptrs_list_start = (trace_ptrs_t*)malloc(sizeof(trace_ptrs_t)*1);
		trace_ptrs_store( files_to_show++, trace_ptrs_list_start, getenv("TRACE_FILE")?getenv("TRACE_FILE"):traceFile );
	} else {
		trace_ptrs_list_start = (trace_ptrs_t*)malloc(sizeof(trace_ptrs_t)*(unsigned)argc);
		memset( trace_ptrs_list_start, 0, sizeof(trace_ptrs_t)*(unsigned)argc );
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
		tfmt=(char*)"%10s%%06d"; // %s - The number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
	strftime(ttbuf,sizeof(ttbuf),tfmt,localtime(&tt));
	tfmt_len=snprintf( tbuf,sizeof(tbuf),ttbuf, 0 );  /* possibly (probably) add usecs (i.e. FMT has %%06d) */

	if (trace_strflg(ospec,'n') || trace_strflg(ospec,'e'))
		for (name_width=0, t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next )
			if (name_width < t_ptrs->rw_p->longest_name)
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
		{	slotStart = (int)num_entries_total;
			printf("specified slotStart index too large, adjusting to %d\n",slotStart );
		}
		trace_ptrs_list_start->rdIdx = (uint32_t)slotStart; // in "startSlot mode" rdIdx is not realative to wrIdxCnt
		trace_ptrs_list_start->ref_tv.tv_sec = 0;
	}
	else
	{	/* here, rdIdx starts as a count (see max = rdIdx below), but it is
		   used as an index after first being decremented (assuming reverse
		   printing) */
		for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next) {
			t_ptrs->rdIdx = TRACE_ATOMIC_LOAD(&t_ptrs->rw_p->wrIdxCnt); // % t_ptrs->ro_p->num_entries;
			t_ptrs->ref_tv.tv_sec = 0;
		}
	}

	if ((count>=0) && (slotStart>=0)) {   // only applicable with one file (the slotStart>=0 part)
		t_ptrs=trace_ptrs_list_start;
		if ((unsigned)count > num_entries_total) {
			max = t_ptrs->max = t_ptrs->ro_p->num_entries;
			count = (int)max;
		} else {
			max = t_ptrs->max = (uint32_t)count;
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

	if ((count>=0) && (slotStart<0) && (count<(int)max))
		for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next)
			t_ptrs->max=(uint32_t)count;
	if (opts&forward_) {
		for (t_ptrs=trace_ptrs_list_start; t_ptrs!=NULL; t_ptrs=t_ptrs->next ) {
			int32_t add = -(int32_t)t_ptrs->max;
			t_ptrs->rdIdx = TRACE_IDXCNT_ADD( TRACE_ATOMIC_LOAD(&t_ptrs->rw_p->wrIdxCnt), add );
			/* subtract 1 b/c during loop (below) rdIdx is incremented first */
			t_ptrs->rdIdx = TRACE_IDXCNT_ADD( t_ptrs->rdIdx, -1 );
		}
		forward_continuous =1;
		for_rev=1;
	}

	//printf("for_rev=%d slotStart=%d count=%d max=%d siz_msg_largest=%u num_params_largest=%u files_to_show=%d\n"
	//       ,for_rev,slotStart,count,max,siz_msg_largest,num_params_largest, files_to_show);

	N_width = minw(3,countDigits((int32_t)max-1));
	bufSlot_width= minw( 3, countDigits((int32_t)num_entries_total-1) );

	myEnt_p       = (struct traceEntryHdr_s *)malloc( siz_entry_largest );
	local_msg     =	                   (char*)malloc( siz_msg_largest * 3 );/* in case an %ld needs change to %lld */
	local_params  =	                (uint8_t*)malloc( num_params_largest*sizeof(uint64_t) );
	params_sizes  =         (struct sizepush*)malloc( num_params_largest*sizeof(struct sizepush) );

	if (trace_strflg(ospec,'H'))
	{
		msg_spec_included=0;
		/*                  123456789112345678921234*/
# define TRACE_MSG_DASHES  "------------------------"
# define TRACE_LONG_DASHES "------------------------------------------------"
		for (sp=ospec; *sp; ++sp) {
			char tbuf[0x100];
			int width_state, width_ia[2]= {0}; /* allow up to 3 widths w[.w] */
			char width_ca[2][9];               /* need to save the string for leading "0" (e.g. "-04"*/
			char flags_ca[4];                  /*  */
			size_t flags_sz;
			//const char *default_unknown_sav;
			char *endptr;

			if (*sp != '%') {
				printf("%c",*sp );
				continue;
			}
			if (*++sp =='\0') {
				printf("%%"); // print the trailing/ending %
				break;
			}

			/* look for a width/flag spec - see trace.h:vtrace_user(...)  */
			flags_sz= strspn(sp, "#*."); /* originally:"#-+ '*."      %#n.mf#/src#  */
			if (flags_sz) {
				snprintf(flags_ca, TRACE_MIN(flags_sz + 1, sizeof(flags_ca)), "%s", sp);  // snprintf always terminates
				sp+= flags_sz;                                                            // use just flags_sz here to ignore wacky +++++++
			} else flags_ca[0]='\0';
			for (width_state= 0; width_state < 2; ++width_state) {
				width_ia[width_state]= (int)TRACE_STRTOL(sp, &endptr, 10);
				if (endptr == sp || endptr > (sp + 4)) { /* check if no num or num too big (allow "-099") */
					width_ca[width_state][0] = '\0';
					break;
				}
				snprintf(width_ca[width_state], sizeof(width_ca[0]), "%.*s", (int)(endptr - sp), sp);
				sp= endptr;
				if (*sp == '.')
					++sp;
			}

			switch (*sp) {
			case '%': printf("%%"); break;
			case 'a': printf("args");break;
			case 'B': printf("B"); break;
			case 'C': printf("cpu"); break;
			case 'D': /* ignore this "control" */ break;
			case 'e':
				if (!width_state) printf("%*s", name_width+1+TRACE_LINENUM_WIDTH,"trcname:ln#");
				else {
					if (width_ia[0]<TRACE_LINENUM_WIDTH) width_ia[0]=TRACE_LINENUM_WIDTH;
					printf("%*s", (int)name_width+1+width_ia[0],"trcname:ln#");
				}
				break; // +1 for ':'
			case 'f': printf("%-*s", (int)strlen(TRACE_MSG_DASHES),"msg"); msg_spec_included=1; break;
			case 'H': /* ignore this "control" */ break;
			case 'I': printf(" TID"); break;
			case 'i': printf("%" TRACE_STR(TRACE_TID_WIDTH) "s", "tid" ); break; /* darwin 16 tids are routinely 7 digit */
			case 'L':
				if (strchr(flags_ca,'*'))
					printf("%*s",trace_lvlwidth, "lvl");
				else {
					strcpy(tbuf, "%");
					if (strchr(flags_ca,'.')) strcpy(&tbuf[1], ".");
					if (width_state >= 1)     strcat(&tbuf[1], width_ca[0]);
					if (width_state >= 2) {
						strcat(&tbuf[1], ".");
						strcat(&tbuf[2], width_ca[1]);
					}
					strcat(&tbuf[1], "s");
					printf(tbuf, "lvl");
				}
				break;
			case 'l': printf("lv"); break;
			case 'm': printf("%-*s", (int)strlen(TRACE_MSG_DASHES),"msg"); msg_spec_included=1; break;
			case 'N': printf("%*s", N_width, "idx" ); break;
			case 'n': printf("%*s", name_width,"trcname");break;
			case 'O': /* ignore here */ break;
			case 'o': /* ignore here */ break;
			case 'P': printf("%" TRACE_STR(TRACE_TID_WIDTH) "s","pid"); break;
			case 'R': printf("r"); break;
			case 'S': printf("%c", 'S' ); break; /* Severity (1st character of lvlstr) */
			case 's': printf("%*s", bufSlot_width, "slt" ); break;
			case 'T': if(tfmt_len)printf("%*.*s", tfmt_len,tfmt_len,&("us_tod"[tfmt_len>=6?0:6-tfmt_len])); break;
			case 't': printf("       tsc"); break;
			case 'u': 
				if (!width_state) printf("%" TRACE_STR(TRACE_LINENUM_WIDTH) "s", "ln#");
				else              printf("%*s", width_ia[0], "ln#");
				break;
			case 'x': /* ignore this "control" */ break;
			case 'X': printf("%-*s", (int)strlen(TRACE_MSG_DASHES),"msg"); msg_spec_included=1; break;
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
			char tbuf[0x100];
			int width_state, width_ia[2]= {0}; /* allow up to 3 widths w[.w] */
			char width_ca[2][9];               /* need to save the string for leading "0" (e.g. "-04"*/
			char flags_ca[4];                  /*  */
			size_t flags_sz;
			//const char *default_unknown_sav;
			char *endptr;
			
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

			/* look for a width/flag spec - see trace.h:vtrace_user(...)  */
			flags_sz= strspn(sp, "#*."); /* originally:"#-+ '*."      %#n.mf#/src#  */
			if (flags_sz) {
				snprintf(flags_ca, TRACE_MIN(flags_sz + 1, sizeof(flags_ca)), "%s", sp);  // snprintf always terminates
				sp+= flags_sz;                                                            // use just flags_sz here to ignore wacky +++++++
			} else flags_ca[0]='\0';
			for (width_state= 0; width_state < 2; ++width_state) {
				width_ia[width_state]= (int)TRACE_STRTOL(sp, &endptr, 10);
				if (endptr == sp || endptr > (sp + 4)) { /* check if no num or num too big (allow "-099") */
					width_ca[width_state][0] = '\0';
					break;
				}
				snprintf(width_ca[width_state], sizeof(width_ca[0]), "%.*s", (int)(endptr - sp), sp);
				sp= endptr;
				if (*sp == '.')
					++sp;
			}

			switch (*sp) {
			case '%': printf("-"); break;
			case 'a': printf("----");break;
			case 'B': printf("-"); break;
			case 'C': printf("---"); break;
			case 'D': /* ignore this "control" */ break;
			case 'e':
				if (!width_state) printf("%.*s", name_width+1+TRACE_LINENUM_WIDTH,TRACE_LONG_DASHES);
				else {
					if (width_ia[0]<TRACE_LINENUM_WIDTH) width_ia[0]=TRACE_LINENUM_WIDTH;
					printf("%.*s", (int)name_width+1+width_ia[0],TRACE_LONG_DASHES);
				}
				break;
			case 'f': printf(TRACE_MSG_DASHES); msg_spec_included=1; break;
			case 'H': /* ignore this "control" */ break;
			case 'I': printf("----"); break;
			case 'i': printf("%." TRACE_STR(TRACE_TID_WIDTH) "s", TRACE_LONG_DASHES); break; /* darwin 16 tids are routinely 7 digit */
			case 'L':
				if (strchr(flags_ca,'*'))
					printf("%.*s",trace_lvlwidth, TRACE_LONG_DASHES);
				else {
					strcpy(tbuf, "%");
					if (strchr(flags_ca,'.')) strcpy(&tbuf[1], ".");
					if (width_state >= 1)     strcat(&tbuf[1], width_ca[0]);
					if (width_state >= 2) {
						strcat(&tbuf[1], ".");
						strcat(&tbuf[2], width_ca[1]);
					}
					strcat(&tbuf[1], "s");
					printf(tbuf, TRACE_LONG_DASHES);
				}
				break;
			case 'l': printf("--"); break;
			case 'm': printf(TRACE_MSG_DASHES); msg_spec_included=1; break;
			case 'N': printf("%.*s", N_width, TRACE_LONG_DASHES); break;
			case 'n': printf("%.*s", name_width,TRACE_LONG_DASHES);break;
			case 'O': /* ignore here */ break;
			case 'o': /* ignore here */ break;
			case 'P': printf("%." TRACE_STR(TRACE_TID_WIDTH) "s", TRACE_LONG_DASHES); break;
			case 'R': printf("-"); break;
			case 'S': printf("-"); break; /* Severity (1st character of lvlstr) */
			case 's': printf("%.*s", bufSlot_width, TRACE_LONG_DASHES); break;
			case 'T': if(tfmt_len)printf("%.*s", tfmt_len, TRACE_LONG_DASHES); break;
			case 't': printf("----------"); break;
			case 'u':
				if (!width_state) printf("%." TRACE_STR(TRACE_LINENUM_WIDTH) "s", TRACE_LONG_DASHES);
				else              printf("%.*s", width_ia[0], TRACE_LONG_DASHES);
				break;

			case 'x': /* ignore this "control" */ break;
			case 'X': printf(TRACE_MSG_DASHES); msg_spec_included=1; break;
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
		rdIdx = TRACE_IDXCNT_ADD( t_ptrs->rdIdx, for_rev );
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
					t_ptrs->ref_tv.tv_sec=0;
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
			} else if (forward_continuous) { /* and lines==0 (obviously) */
				/* special check where, e.g.: treset; tcntl test */
				if (t_ptrs->ref_tv.tv_sec) {
					uint32_t prvIdx=TRACE_IDXCNT_ADD(t_ptrs->rdIdx, -1);
					if (tvcmp(&t_ptrs->ref_tv,&idxCnt2entPtr(prvIdx)->time)) {
						t_ptrs->rdIdx = 0;
						t_ptrs->ref_tv.tv_sec=0;
						goto reset_check;
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
		         , bufSlot_width, N_width, (int)name_width, printed, t_ptrs_use->rdIdx
		         , tfmt, tfmt_len );
		++printed;
		t_ptrs_use->ref_tv = myEnt_p->time;
		t_ptrs_use->rdIdx = TRACE_IDXCNT_ADD( t_ptrs_use->rdIdx, for_rev );
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
	int            memlen=(int)traceMemLen( TRACE_cntlPagesSiz()
	                                  ,traceControl_p->num_namLvlTblEnts
	                                  ,traceControl_p->nam_arr_sz
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
	       "fast/mem __func__ = %-8d    1=force on, 0=TRACE_PRINT, -1=force off\n"
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
	       "lvls offset       = 0x%lx\n"
	       "nams offset       = 0x%lx\n"
	       "buffer_offset     = 0x%lx\n"
	       "memlen            = 0x%x          %s\n"
	       "default TRACE_TIME_FMT=\"%s\"\n"
	       "default TRACE_SHOW=\"%s\" others: a:nargs B:paramBytes D:inDent e:nam:ln# f:convertedMsgfmt_only I:trcId l:lvlNum O/o:color R:retry S:severity s:slot t:tsc u:line X:examineArgData\n"
	       "default TRACE_PRINT=\"%s\" others: C:core e:nam:ln# [n]f:file F:func I:trcId i:threadID l:lvlNum m:msg-insert N:unpadded_trcName O/o:color P:pid S:severity t:insert u:line\n"
	       , TRACE_REV
	       , traceControl_p->version_string
	       , outstr
	       , traceControl_p->trace_initialized
	       , traceControl_rwp->mode.bits.func
	       , traceControl_rwp->mode.words.mode, traceControl_rwp->mode.bits.S?"Slow:ON, ":"Slow:off", traceControl_rwp->mode.bits.M?" Mem:ON":" Mem:off"
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
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceLvls_p[traceTID].M /* sizeof(uint64_t)*2 is "nibbles" */
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceLvls_p[traceTID].S
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceLvls_p[traceTID].T
	       , traceControl_p->num_entries
	       , traceControl_p->siz_msg
	       , traceControl_p->num_params
	       , traceControl_p->siz_entry
	       , traceControl_p->num_namLvlTblEnts
	       , (int)traceControl_p->nam_arr_sz-1
	       , traceControl_rwp->longest_name
	       , (void*)&((struct traceControl_s*)0)->rw.wrIdxCnt
	       , (unsigned long)traceLvls_p - (unsigned long)traceControl_rwp
	       , (unsigned long)traceNams_p - (unsigned long)traceControl_rwp
	       , (unsigned long)traceEntries_p - (unsigned long)traceControl_rwp
	       , traceControl_p->memlen
	       , (traceControl_p->memlen != (uint32_t)memlen)?"not for mmap":""
	       , TRACE_DFLT_TIME_FMT
	       , DFLT_SHOW
	       , TRACE_PRINT__
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
	int         opt_loops=-1, opt_timing_stats=0;
	unsigned    opt_dly_ms=0;
	unsigned    opt_burst=1;
	int         opt_all=0;
	int         opt_count=-1, opt_start=-1;
	const char *opt_Name=NULL;	/* -N<wild> */
	uint64_t t0_us=0;
	uint32_t tdelta_us;

	opterr=0;					/* turn of getopt (automatic) error output */
	while ((opt=getopt(argc,argv,"?hab:c:d:Ff:HL:l:N:n:qs:tVx:")) != -1) {
		switch (opt) {
		/*   '?' is also what you get w/ "invalid option -- -"   */
		case '?': case 'h': if (!strchr("?h",optopt))printf("Invalid option: -%c\n",optopt);printf(USAGE);exit(0);break;
		case 'a': opt_all=1;                                       break;
		case 'b': opt_burst=(unsigned)strtoul(optarg,NULL,0);      break;
		case 'c': opt_count=(int)strtoul(optarg,NULL,0);           break;
		case 'd': opt_dly_ms=(unsigned)strtoul(optarg,NULL,0);     break;
        case 'F': show_opts|=forward_;                             break;
		case 'f': setenv("TRACE_FILE",optarg,1);                   break;
		case 'H': do_heading=0;                                    break;
		case 'L': setlocale(LC_NUMERIC,optarg);                    break;
		case 'l': opt_loops=(int)strtoul(optarg,NULL,0);           break;
		case 'N': opt_Name=optarg;                                 break;
		case 'n': setenv("TRACE_NAME",optarg,1);                   break;/*note: TRACE_CNTL "file" or "name" doesn't allow setting the other (order becomes dependent)*/
		case 'q': show_opts|=quiet_;                               break;
		case 's': opt_start=(int)strtoul(optarg,NULL,0);           break;
		case 't': opt_timing_stats=1;                              break;
		case 'V': printf("%s\n",TRACE_CNTL_REV);if(argc==2)exit(0);break;
		case 'x': trace_thread_option=(int)strtoul(optarg,NULL,0); break;
		default:
			fprintf(stderr,"Invalid option\n");
			printf( USAGE ); exit( 0 );
		}
	}
	if (argc - optind < 1) {
		printf( "Need cmd\n" );
		printf( USAGE ); exit( 0 );
	}
	cmd = argv[optind++];

	if (opt_Name && !(strncmp(cmd,"lvl",3)==0)) { // a name (which may be wildcard) will get added to lvl command below
		setenv("TRACE_NAME",opt_Name,1);
		//traceInit(opt_Name,0);
	}

	if(getenv("LC_NUMERIC"))                        // IFF LC_NUMERIC is set in the environment (i.e to "en_US.UTF-8")...
		setlocale(LC_NUMERIC,getenv("LC_NUMERIC")); // this is needed for (e.g.) %'d to produce output with thousands grouped

	if		(strcmp(cmd,"test1") == 0)
	{	int loops=1, trace_cnt;
		char msg_str[200];
		char addr_str[100];
		void *vp=addr_str;
		setenv("TRACE_NAME","TRACE",0); // allow this to be overridden by -n and env.var. (and define TRACE_NAME)
		if (opt_loops > -1) loops=opt_loops;
		sprintf( addr_str, "%p", vp );
		sprintf( msg_str,"Hi %%d. \"c 2.5 -5 5000000000 0x87654321 2.6 %p\" ", vp );
		strcat( msg_str, "should be repeated here: %c %.1f %hd %lld %p %.1Lf %s" );
		for (trace_cnt=1; trace_cnt<=loops; ++trace_cnt)
		{	TRACE( TLVL_INFO, msg_str, trace_cnt
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

		for (ii=0; ii<sizeof(ff)/sizeof(ff[0]); ++ii)  ff[ii]=(float)(2.5*ii);

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

		TRACE( TLVL_INFO, "hello" );

		myIdx = traceControl_p->largest_multiple - 3;
		printf("myIdx=0x%08x\n", myIdx );
		for (ii=0; ii<6; ++ii)
		{	desired = TRACE_IDXCNT_ADD(myIdx,1);
			printf( "myIdx==>TRACE_IDXCNT_ADD(myIdx,1): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		for (ii=0; ii<6; ++ii)
		{	desired = TRACE_IDXCNT_ADD(myIdx,-1);
			printf( "myIdx==>TRACE_IDXCNT_ADD(myIdx,-1): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		printf("myIdx=0x%08x\n", myIdx );

		xx=5;
		myIdx = traceControl_p->largest_multiple - (uint32_t)(3*xx);
		printf("myIdx=0x%08x\n", myIdx );
		for (ii=0; ii<6; ++ii)
		{	desired = TRACE_IDXCNT_ADD(myIdx,xx);
			printf( "myIdx==>TRACE_IDXCNT_ADD(myIdx,5): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		for (ii=0; ii<6; ++ii)
		{	desired = TRACE_IDXCNT_ADD(myIdx,-xx);  /* NOTE: this will not work if xx is of type uint32_t; it must be of int32_t */
			printf( "myIdx==>TRACE_IDXCNT_ADD(myIdx,-5): 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		printf("myIdx=0x%08x\n", myIdx );

		printf("For forward read... simple delta\n");
		printf("TRACE_IDXCNT_DELTA(wr=5,rd=4)=%u\n",TRACE_IDXCNT_DELTA(5,4));
		printf("TRACE_IDXCNT_DELTA(wr=5,rd=0)=%u\n",TRACE_IDXCNT_DELTA(5,0));
		printf("TRACE_IDXCNT_DELTA(wr=5,rd=TRACE_IDXCNT_ADD(0,-1)=0x%08x)=%u\n",TRACE_IDXCNT_ADD(0,-1),TRACE_IDXCNT_DELTA(5,TRACE_IDXCNT_ADD(0,-1)));
		printf("For reverse read... numents(or wrIdxCnt) - delta\n");
		printf("(num_entries=%u) - TRACE_IDXCNT_DELTA(wr=TRACE_IDXCNT_ADD(num_entries,10),rd=TRACE_IDXCNT_ADD(num_entries,5)) = %u\n"
		       ,traceControl_p->num_entries
		       ,traceControl_p->num_entries - TRACE_IDXCNT_DELTA(TRACE_IDXCNT_ADD(traceControl_p->num_entries,10),TRACE_IDXCNT_ADD(traceControl_p->num_entries,5)) );
		printf("rdIdx_has_lines(TRACE_IDXCNT_ADD(num_entries,10),TRACE_IDXCNT_ADD(num_entries,5),-1) = %u\n"
		       ,rdIdx_has_lines( TRACE_IDXCNT_ADD(traceControl_p->num_entries,10)
			                    ,TRACE_IDXCNT_ADD(traceControl_p->num_entries,5),-1) );
		printf("(num_entries=%u) - TRACE_IDXCNT_DELTA(wr=TRACE_IDXCNT_ADD(num_entries,10+num_entries)=%u,rd=TRACE_IDXCNT_ADD(num_entries,5)) = %u\n"
		       ,traceControl_p->num_entries
		       ,TRACE_IDXCNT_ADD(traceControl_p->num_entries,(int)(10+traceControl_p->num_entries))
		       ,traceControl_p->num_entries - TRACE_IDXCNT_DELTA( TRACE_IDXCNT_ADD(traceControl_p->num_entries,(int)(10+traceControl_p->num_entries))
			                                               ,TRACE_IDXCNT_ADD(traceControl_p->num_entries,5) ) );
		printf("rdIdx_has_lines(TRACE_IDXCNT_ADD(num_entries,10+num_entries),TRACE_IDXCNT_ADD(num_entries,5),-1) = %u\n"
		       ,rdIdx_has_lines( TRACE_IDXCNT_ADD(traceControl_p->num_entries,(int)(10+traceControl_p->num_entries))
			                    ,TRACE_IDXCNT_ADD(traceControl_p->num_entries,5),-1) );

		printf("\n");
		TRACE( TLVL_WARNING, "hello %d\n\tthere\n", 1 );
		TRACE( TLVL_INFO, "hello %d %d", 1, 2 );
		TRACE( TLVL_DEBUG, "hello %d %d %d", 1,2,3 );
		TRACE( TLVL_DEBUG, "hello %d %d %d %d %d %d %d %d %d %d %d"
			  , 1,2,3,4,5,6,7,8,9,10, 11 );	  /* extra param does not get saved in buffer */
		TRACE( TLVL_DEBUG, "hello %f %f %f %f %f %f %f %f %f %f"
			  , 1.0,2.0,3.0,4.0, ff[5],6.0,7.0,8.0,9.0,10.0 );
		TRACE( TLVL_DBG+1, "hello %d %d %f  %d %d %f	 %d %d there is tabs after 2nd float"
			  ,	          1, 2,3.3,4, 5, 6.6, 7, 8 );
		TRACE( TLVL_DBG+1, TSPRINTF("%s:%%d- int=%%d __FILE__=%s",strrchr(__FILE__,'/')?strrchr(__FILE__,'/')+1:__FILE__,__FILE__)
		      , __LINE__, 5 );
		TRACE( TLVL_DBG+1, TSPRINTF("%s%s%s",strrchr(__FILE__,'/')?strrchr(__FILE__,'/')+1:__FILE__,":%d- int=%d __FILE__=",__FILE__)
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
			TRACE( TLVL_INFO, "ii=%u", ii );
		}
	}
	else if (strcmp(cmd,"test-ro") == 0)
	{
		/* To test normal userspace, can use: TRACE_FILE= trace_cntl test-ro
		 */
		setenv("TRACE_FILE","/proc/trace/buffer",0);
		printf("TRACE_FILE=%s\n", getenv("TRACE_FILE"));
		traceInit(NULL,0);
		printf("try write to (presumably kernel memory) write-protected 1st page...\n");
		traceControl_p->trace_initialized = 2;
		printf("write succeeded.\n");
#	   if defined(TEST_WRITE_PAST_END)
		*(((uint8_t*)traceControl_p)+traceControl_p->memlen) = 6;
#	   endif
	} else if (strcmp(cmd,"test-compare") == 0) {
		char      buffer[200];
		uint64_t  mark;
		uint32_t  delta;
		unsigned  loops=DFLT_TEST_COMPARE_ITERS;
		unsigned  test_mask=0x7f; /* all tests */
		unsigned  modes_msk=0xf; /* all mode combinations */
		int       fd;
		int       jj;

		if (opt_loops > -1) loops=(unsigned)opt_loops;
		opt_loops = (int)loops;		/* save */

		fd = open( "/dev/null", O_WRONLY );
		dup2( fd, 1 );   /* redirect stdout to /dev/null */
        setlocale(LC_NUMERIC,"en_US");  /* make ' printf flag work -- setting LC_NUMERIC in env does not seem to work */

		TRACE_CNTL("mode",3);
		traceControl_rwp->mode.bits.M = 1;		   // NOTE: TRACE_CNTL("modeM",1) hardwired to NOT enable when not mapped!

#		define STRT_PRN( s1, sA, sB ) sprintf(buffer,s1,sA,sB);fprintf(stderr,"%-46s",buffer);fflush(stderr)
		// ELF 6/6/18: GCC v6_3_0 does not like %', removing the '...
#		define END_FMT  "%10u us, %5.3f us/TRACE, %7.3f Mtraces/s\n",delta,(double)delta/loops,(double)loops/delta
#		define CONTINUE fprintf(stderr,"Continuing.\n");continue
		if (argc - optind >= 1) test_mask=(unsigned)strtoul(argv[optind],NULL,0);
		if (argc - optind >= 2) modes_msk=(unsigned)strtoul(argv[optind+1],NULL,0);
		for (jj=0; jj<4; ++jj) {
			unsigned tstmod=(1U<<jj)&modes_msk;
			switch (tstmod) {
			case 1: { TRACE_CNTL("lvlclrM",1LL<<TLVL_INFO); TRACE_CNTL("lvlclrS",1LL<<TLVL_INFO);
					loops = (unsigned)opt_loops*10;
					fprintf(stderr,"0x1 M0S0 - Testing with M and S lvl disabled. loops=%u\n", loops );
			}		break;
			case 2: { TRACE_CNTL("lvlsetM",1LL<<TLVL_INFO); TRACE_CNTL("lvlclrS",1LL<<TLVL_INFO);
					loops = (unsigned)opt_loops*5;
					fprintf(stderr,"0x2 M1S0 - First testing with S lvl disabled (mem only). loops=%u\n", loops ); break;
			}		break;
			case 4: { TRACE_CNTL("lvlsetM",1LL<<TLVL_INFO); TRACE_CNTL("lvlsetS",1LL<<TLVL_INFO);
					loops = (unsigned)opt_loops;
					fprintf(stderr,"0x4 M1S1 - Testing with M and S lvl enabled (stdout>/dev/null). loops=%u\n", loops ); break;
			}		break;
			case 8: { TRACE_CNTL("lvlclrM",1LL<<TLVL_INFO); TRACE_CNTL("lvlsetS",1LL<<TLVL_INFO);
					loops = (unsigned)opt_loops;
					fprintf(stderr,"0x8 M0S1 - Testing with just S lvl enabled. Unusual (freeze). loops=%u\n", loops ); break;
			}		break;
			case 0: continue;
			}

			if (1 & test_mask) {
				STRT_PRN(" 0x01 -%s const short msg %s","",(tstmod&0xc)?"(NO snprintf)":"");
				//fprintf(stderr,STRT_FMT," 0x01 - const short msg (NO snprintf)");fflush(stderr);
				TRACE_CNTL("reset"); mark = gettimeofday_us();
				for (ii=0; ii<loops; ++ii) {
					TRACE( TLVL_INFO, "any msg" );
				} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
			}

			if (2 & test_mask) {
				STRT_PRN(" 0x02 - 1 arg%s%s","","");
				TRACE_CNTL("reset"); mark = gettimeofday_us();
				for (ii=0; ii<loops; ++ii) {
					TRACE( TLVL_INFO, "this is one small param: %u", 12345678 );
				} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
			}

			if (4 & test_mask) {
				STRT_PRN(" 0x04 - 2 args%s%s","","");
				TRACE_CNTL("reset"); mark = gettimeofday_us();
				for (ii=0; ii<loops; ++ii) {
					TRACE( TLVL_INFO, "this is 2 params: %u %u", 12345678, ii );
				} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
			}

			if (8 & test_mask) {
				STRT_PRN(" 0x08 - 8 args (7 ints, 1 float)%s%s","","");
				TRACE_CNTL("reset"); mark = gettimeofday_us();
				for (ii=0; ii<loops; ++ii) {
					TRACE( TLVL_INFO, "this is 8 params: %u %u %u %u %u %u %u %g"
					      , 12345678, ii, ii*2, ii+6
					      , 12345679, ii, ii-7, (float)ii*1.5 );
				} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
			}

			if (0x10 & test_mask) {
				STRT_PRN(" 0x10 - 8 args (1 ints, 7 float)%s%s","","");
				TRACE_CNTL("reset"); mark = gettimeofday_us();
				for (ii=0; ii<loops; ++ii) {
					TRACE( TLVL_INFO, "this is 8 params: %u %g %g %g %g %g %g %g"
					      , 12345678, (float)ii, (float)ii*2.5, (float)ii+3.14
					      , (float)12345679, (float)ii/.25, (float)ii-2*3.14, (float)ii*1.5 );
				} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
			}

			if (0x20 & test_mask) {
				STRT_PRN(" 0x20 - snprintf of same 8 args%s%s","","");
				TRACE_CNTL("reset"); mark = gettimeofday_us();
				for (ii=0; ii<loops; ++ii) {
					snprintf( buffer, sizeof(buffer)
					         , "this is 8 params: %u %g %g %g %g %g %g %g"
					         , 12345678, (float)ii, (float)ii*2.5, (float)ii+3.14
					         , (float)12345679, (float)ii/.25, (float)ii-2*3.14, (float)ii*1.5
					         );
					TRACE( TLVL_INFO, buffer );
				} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
			}

			if (0x40 & test_mask) {
				STRT_PRN(" 0x40 -%s const short msg %s",(1&test_mask)?" (repeat)":"",(tstmod&0xc)?"(NO snprintf)":"");
				TRACE_CNTL("reset"); mark = gettimeofday_us();
				for (ii=0; ii<loops; ++ii) {
					TRACE( TLVL_INFO, "any msg" );
				} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
			}
		}
	}
#	ifdef DO_THREADS   /* requires linking with -lpthreads */
	else if (strcmp(cmd,"test-threads") == 0)
	{	pthread_t *threads;
		unsigned   loops=2, num_threads=NUMTHREADS;
		args_t    *argsp, args0;
		int        sts=0;
		pid_t      main_tid=trace_gettid();
		unsigned   total_traces=0;
		if (opt_loops > -1)
			loops=(unsigned)opt_loops;
		if (opt_burst == 0) {
			printf("adjusting burst==0 to min burst=1\n");
			opt_burst=1;
		}
		if ((argc-optind)==1)
		{	num_threads=(unsigned)strtoul(argv[optind],NULL,0);
			if (num_threads > 4095) num_threads=4095;
		}
		threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
		argsp   =    (args_t*)malloc(num_threads*sizeof(args_t));

		TRACE( TLVL_INFO, "before pthread_create - main_tid=%u loops=%d, threads=%u, dly_ms=%u traceControl_p=%p"
		      , main_tid, loops, num_threads, opt_dly_ms, (void*)traceControl_p );
		if (opt_timing_stats){
			TRACE_CNTL("lvlsetMg", 0xfLL<<TLVL_INFO); /* set INFO,DBG,DBG+1, and DBG+2 */
			t0_us = gettimeofday_us();
		}
		for (ii=0; ii<num_threads; ii++) {
			argsp[ii].tidx   = ii+1;
			argsp[ii].loops  = (unsigned)loops;
			argsp[ii].dly_ms = opt_dly_ms;
			argsp[ii].burst  = opt_burst;
			pthread_create(&(threads[ii]),NULL,thread_func,(void*)&argsp[ii] );
		}

		args0.tidx   = 0;
		args0.loops  = (unsigned)loops;
		args0.dly_ms = opt_dly_ms;
		args0.burst  = opt_burst;
		thread_func( &args0 );

		if (trace_thread_option & 4)
		{   char          cmd[200];
		    sprintf( cmd, "echo trace_buffer mappings before join '(#1)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    sts = system( cmd );
		    sprintf( cmd, "echo trace_buffer mappings before join '(#2)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    sts = system( cmd );
		}
		for (ii=0; ii<num_threads; ii++)
		{	pthread_join(threads[ii], NULL);
		}
		if (opt_timing_stats){
			tdelta_us=(uint32_t)(gettimeofday_us()-t0_us);
			total_traces = loops*(num_threads+1)*5;
			fprintf(stderr,"%lld usec, %.3f usec/TRACE, %.3f Mtraces/s\n",
			        (unsigned long long)tdelta_us, (double)tdelta_us/total_traces,(double)total_traces/tdelta_us);
		}
		if (trace_thread_option & 4)
		{   char          cmd[200];
		    sprintf( cmd, "echo trace_buffer mappings after join '(#1)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    sts = system( cmd );
		    sprintf( cmd, "echo trace_buffer mappings after join '(#2)' = `cat /proc/%d/maps | grep trace_buffer | wc -l`", getpid() );
		    sts = system( cmd );
		}
		TRACE( TLVL_INFO, "after pthread_join - main_tid=%u traceControl_p=%p sts=%d"
		      , main_tid, (void*)traceControl_p, sts );
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
		traceInit(NULL,1);
		traceInfo();
	}
	else if (strcmp(cmd,"tids") == 0)
	{	uint32_t longest_name;
		int      namLvlTblEnts_digits;
		traceInit(NULL,1);
		longest_name = traceControl_rwp->longest_name;
		/*printf("longest_name=%d\n",longest_name);*/
		namLvlTblEnts_digits=countDigits((int)traceControl_p->num_namLvlTblEnts-1);
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
			if (opt_all || TRACE_TID2NAME((int32_t)ii)[0]!='\0')
			{	printf("%*d %*.*s 0x%016llx 0x%016llx 0x%016llx\n"
				       , minw(3,namLvlTblEnts_digits), ii
					   , longest_name
					   , longest_name
				       , TRACE_TID2NAME((int32_t)ii)
				       , (unsigned long long)traceLvls_p[ii].M
				       , (unsigned long long)traceLvls_p[ii].S
				       , (unsigned long long)traceLvls_p[ii].T
				       );
			}
		}
	} else if (strcmp(cmd,"unlock") == 0) {
		traceInit(NULL,0);
		trace_unlock( &traceControl_rwp->namelock );
	} else if (strcmp(cmd,"lvlstrs") == 0) {
		TRACE_CNTL("init");		/* for potential TRACE_LVLSTRS */
		for (ii=0; ii<64; ++ii) {
			for (int aa=0; aa<trace_lvlstrs_aliases; ++aa)
				printf("%*s ",trace_lvlwidth, trace_lvlstrs[aa][ii]);
			printf("\n");
		}
	} else if (strncmp(cmd,"TRACE",5) == 0) {
#		define ARGS_SZ 35*sizeof(uint64_t)/sizeof(unsigned long) /* works for 32 and 64 bit archs */
		unsigned long args[ARGS_SZ] __attribute__ ((aligned (16))); // needed for delayed long double formatting
		unsigned long *args_ptr=args;
		unsigned long *end_ptr=&args[ARGS_SZ];
		unsigned long *nxt_ptr;
		uint8_t nargs=0, lvl;
		int    cc, required=(cmd[5]=='N')?3:2;
		char *eptr, *lvlptr=(cmd[5]=='N')?argv[optind+1]:argv[optind];
		if (opt_loops == -1) opt_loops=1;
		if (argc - optind < required) {
			printf( "\"TRACE\" cmd requires at least lvl and fmt arguments.");
			printf( "\"TRACE\" cmd requires at least name, lvl and fmt arguments.");
			printf( USAGE ); exit( 0 );
		}
		for (cc=optind+required; cc<argc; ++cc){
			/* do a quick check for certain character that would only be in a double - see man strtod */
			if (strpbrk(argv[cc],".INnPp"))  nxt_ptr = add_double_arg(args_ptr,end_ptr,strtod(argv[cc], NULL));
			else {
				/* then actually see if a full unsigned long parse occurs */
				unsigned long uu=strtoul(argv[cc],&eptr,0);
				if (eptr == (argv[cc]+strlen(argv[cc])))
					nxt_ptr = add_ul_arg(    args_ptr,end_ptr,uu);
				else
					nxt_ptr = add_double_arg(args_ptr,end_ptr,strtod(argv[cc], NULL));
			}
			if (nxt_ptr==args_ptr) break;
			args_ptr = nxt_ptr;
			++nargs;
		}

		TRACE_CNTL("init");		/* for potential TRACE_LVLSTRS */
		lvl = (uint8_t)strtoul(lvlptr,&eptr,0); /* traceInit should be called before this to pickup any trace_lvlstrs changes */
		if (lvlptr == eptr)
			lvl = (uint8_t)str2enum(lvlptr);
		if (opt_timing_stats)
			t0_us = gettimeofday_us();
		if (cmd[5]=='N')
			for (int ii=0; ii<opt_loops; ++ii)
				VTRACEN(argv[optind], lvl, nargs, argv[optind+2], args);
		else
			for (int ii=0; ii<opt_loops; ++ii)
				VTRACE(lvl, nargs, argv[optind+1], args);
		if (opt_timing_stats){
			tdelta_us=(uint32_t)(gettimeofday_us()-t0_us);
			fprintf(stderr,"%lld usec, %.3f usec/TRACE, %.3f Mtraces/s\n",
			        (unsigned long long)tdelta_us, (double)tdelta_us/opt_loops, (double)opt_loops/tdelta_us);
		}
	} else if (strncmp(cmd,"sleep",4) == 0) { /* this allows time to look at /proc/fd/ and /proc/maps */
		TRACE( TLVL_INFO, "starting sleep" );
		if ((argc-optind) == 1)
			sleep( (unsigned)strtoul(argv[optind],NULL,0) );
		else sleep( 10 );
		TRACE( TLVL_INFO, "done sleeping" );
	}
	/* now deal with traceCntl commands using TRACE_CNTL */
	else if (strncmp(cmd,"mode",4) == 0) { /* mode is special -- may _return_ old mode */
		if ((argc-optind) == 0) {
			ret=(int)TRACE_CNTL( cmd );
			printf( "%d\n",ret ); /* print the old mode */
		}
		else ret=(int)TRACE_CNTL( cmd, strtoul(argv[optind],NULL,0) );
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
		size_t  slen=strlen(&cmd[6]);
		long long msk=0;
		if (opt_Name) {			/* this is the Name given via the -N which may be a WILDCARDed name */
			char cmdn[8];
			if (slen>1 || (slen==1&&!strpbrk(&cmd[6],"MST"))) {
				printf("invalid command: %s\n", cmd ); printf( USAGE );
			}
			strncpy(cmdn,cmd,6);
			cmdn[6]='n';strcpy(&cmdn[7],&cmd[6]); /* insert the 'n' */
			switch (argc - optind) {
			case 0: msk=TRACE_CNTL( cmdn, opt_Name );
				printf("mask=0x%llx\n",msk);
				break;
			case 1: msk=TRACE_CNTL( cmdn, opt_Name, strtoull(argv[optind],NULL,0) );
				/*printf("previous mask=0x%x\n",sts);*/
				break;
			case 3: TRACE_CNTL( cmdn, opt_Name, strtoull(argv[optind],NULL,0)
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
			case 0: msk=TRACE_CNTL( cmd );
				printf("mask=0x%llx\n",msk);
				break;
			case 1: msk=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0) );
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
		long long sts=0;
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
		} else if (sts==0)    /* WHY?? */
			ret=1;
		else
			ret=0;
	}
	return (ret);
}	/* main */
