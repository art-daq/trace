/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    */
#define TRACE_CNTL_REV "$Revision: 1045 $$Date: 2019-02-19 11:36:30 -0600 (Tue, 19 Feb 2019) $"
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

#include "trace.h"

#ifdef __APPLE__
# define TRACE_TID_WIDTH 7
#else
# define TRACE_TID_WIDTH 5
#endif

#define USAGE "\
%s [opts] <cmd> [command opt/args]\n\
commands:\n\
 info, tids, reset\n\
 show [count [startSlotIndex]]\n\
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
 -F           show forward, poll for new entries. \"count\" avoids older entries\n\
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
#define minw(a,b) (b<a?a:b)

#ifdef __linux__
# define DFLT_SHOW         "HxNTPiCnLR"
#else
# define DFLT_SHOW         "HxNTPinLR"
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


struct sizepush
{	unsigned size:16;
	unsigned push:16;
};

enum show_opts_e {
	filter_newline_=0x1,
	quiet_         =0x2,
	indent_        =0x4,
    forward_       =0x8
};

void get_arg_sizes(	 char            *ofmt
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

	/*strcpy( ofmt, ifmt );*/
	in = ifmt;
	/* strchrnul would be nice (_GNU_SOURCE, ref. man strchr) */
	/*while ((in=strchr(in,'%')))*/
	while (*in)
	{
		if (*in != '%')
		{	if (*in != '\n') *ofmt++ = *in++;
			else
			{	if (filter_newline) { *ofmt++ = ';'; in++; }
				else                { *ofmt++ = *in++; }
			}
			continue;
		}
		/* found '%' - process it */
		*ofmt = *in;
		percent_sav = ofmt;	/* save in case we need to modify it (too many args) */
		++in; ++ofmt;       /* point to next char */
		/*printf("next char=%c\n",*in);*/
		if ((*in=='%')||(*in=='m')) { *ofmt++ = *in++; continue; }/* ingore %% which specified a % char */
		if (numArgs == maxArgs) { *percent_sav = '*'; continue; }  /* SAFETY - no args beyond max */
		while (	 ((*in>='0')&&(*in<='9'))
			   ||(*in=='.')||(*in=='-')||(*in=='#')||(*in=='+')||(*in=='\'')||(*in=='I')||(*in=='"'))
			*ofmt++ = *in++;
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
				if ((param_bytes==8)&&(sizeof(long)==4)) *ofmt++ = 'l';
				break;
			case 2:	 sizes_out[numArgs].push=8;          sizes_out[numArgs].size=8;break;/* long long */
			default: printf("error\n");
			}
			*ofmt++ = *in++;
			modifier=0;
			break;
		case 'e': case 'E': case 'f': case 'F': case 'g': case 'G': case 'a': case 'A':
			if (modifier)
			{	sizes_out[numArgs].push=(param_bytes==4)?12:16;
				sizes_out[numArgs].size=(param_bytes==4)?12:16;
				modifier=0;
			}/* long double */
			else
			{	sizes_out[numArgs].push=8;
				sizes_out[numArgs].size=8;
			} /* double */
			*ofmt++ = *in++;
			break;

			/* length modifiers */
		case 'h':  --modifier; *ofmt++ = *in++; goto chkChr;
		case 'l':  ++modifier; *ofmt++ = *in++; goto chkChr;
		case 'L':  ++modifier; *ofmt++ = *in++; goto chkChr;
		case 'z':  ++modifier; *ofmt++ = *in++; goto chkChr;

		case 's': case 'n': case 'p':       /* SAFETY -- CONVERT %s to %p */
			*ofmt++='p'; in++;
			break;
		case 'm':
			*ofmt++ = *in++;
			continue;
			break;
		case '*':   /* special -- "arg" in an arg */
			sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4;
			if (++numArgs == maxArgs) { *ofmt++ = *in++; break; }
			*ofmt++ = *in++;
			goto chkChr;
		default:
			if( !(opts&quiet_)) printf("tshow: unknown format spec char \"%c\" encountered.\n",*in);
			*ofmt++ = *in++;
		}
		++numArgs;
	}
	while (*in) *ofmt++ = *in++;
	*ofmt--='\0';
	while (*ofmt == '\n') *ofmt-- = '\0';
	if (numArgs < maxArgs) sizes_out[numArgs].push=0;
}	/* get_arg_sizes */

int countDigits(int n)
{	return (n >= 10)
		? 1 + countDigits(n/10)
		: 1;
}

/* count==-1, start==-1  ==> DEFAULT - reverse print from wrIdx to zero (if not
                             full) or for num_entrie
   count>=0,  start==-1  ==> reverse print "count" entries (incuding 0, which
                             doesn't seem to useful
   count>=0,  start>=0   ==> reverse print count entries starting at "slot" idx
                             "start"  I.e. if 8 traces have occurred, these 2
                             are equivalent:
                             traceShow( ospec, -1, -1, 0 ) and
                             traceShow( ospec,  8,  7, 0 )
 */
void traceShow( const char *ospec, int count, int start, int show_opts )
{
	uint32_t rdIdx;
	int32_t max;
	unsigned printed=0;
	unsigned ii;
	int	   	 buf_slot_width;
	int	   	 opts=(strchr(ospec,'x')?filter_newline_:0);
	struct traceEntryHdr_s* myEnt_p;
	struct traceEntryHdr_s  myEnt_cpy; /* I'll copy */
	char                  * msg_p;
	char                  * msg_cpy; /* I'll malloc space to copy the msg into */
	unsigned long         * params_p;
	unsigned long         * params_cpy; /* I'll malloc space to copy the params into */
	uint8_t               * local_params;
	char                  * local_msg;
	void                  * param_va_ptr;
	struct sizepush       * params_sizes;
	uint8_t               * ent_param_ptr;
	uint8_t               * lcl_param_ptr;
	const char            * sp; /*spec ptr*/
	char                  * indent="                                                                ";
	char                  * tfmt;
	int                     tfmt_len;
	time_t                  tt=time(NULL);
	char                    tbuf[0x100], ttbuf[0x100];
	unsigned                longest_name=0;
	unsigned                longest_lvlstr=0;
	int                     for_rev=-1; /* default is reverse print */
	int                     forward_continuous=0; /* continuous==1 will force for_rev=1 */
	
	opts |= show_opts; /* see enum show_opts_e above */
	opts |= (strchr(ospec,'D')?indent_:0);

	traceInit(NULL); /* init traceControl_p, etc. */

	/* get the time format and the length of the format (using example time (now) */
	if((tfmt=getenv("TRACE_TIME_FMT"))==NULL)
		//tfmt=(char*)TRACE_DFLT_TIME_FMT;
		tfmt=(char*)"%s%%06d";
	strftime(ttbuf,sizeof(ttbuf),tfmt,localtime(&tt));
	tfmt_len=snprintf( tbuf,sizeof(tbuf),ttbuf, 0 );  /* possibly (probably) add usecs (i.e. FMT has %%06d) */

	if (strchr(ospec,'n'))
		for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii) {
			unsigned ll=0;
			while (traceNamLvls_p[ii].name[ll]!='\0' && ++ll < sizeof(traceNamLvls_p->name)) ;
			if (longest_name<ll)
				longest_name=ll;
		}
	if (strchr(ospec,'L')) {
		unsigned ll;
		for (ii=0; ii<64; ++ii)
			if (_lvlstr[ii]!=NULL && (ll=strlen(_lvlstr[ii]))>longest_lvlstr)
				longest_lvlstr = ll;
	}

	/* If just a count is given, it will be a way to limit the number printed;
	   a short hand version of "... | head -count". (so if there are not entries,
	   none will be printed.
	   If however, count and start are given, the count (up to numents) will be
	   printed regardless of how many entries are actually filled with real
	   data. This gives a way to look at data after a "treset".
	*/
	if (start >= 0)
	{	start++; /* start slot index needs to be turned into a "count" */
		if ((unsigned)start > traceControl_p->num_entries)
		{	start = traceControl_p->num_entries;
			printf("specified start index too large, adjusting to %d\n",start );
		}
		rdIdx=start;
	}
	else
	{	/* here, rdIdx stars as a count (see max = rdIdx below), but it is
		   used as an index after first being decremented (assuming reverse
		   printing) */
		rdIdx = TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt) % traceControl_p->num_entries;
	}
	if ((count>=0) && (start>=0))
	{
		if ((unsigned)count > traceControl_p->num_entries)
		{	max = traceControl_p->num_entries;
			printf("specified count > num_entrie, adjusting to %d\n",max);
		} else
			max = count;
	} else if (TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt)>=traceControl_p->num_entries) { /* traceControl_p-> may not be being used */
		max = traceControl_p->num_entries;
	} else
		max = rdIdx;
	if ((count>=0) && (start<0) && (count<max)) max=count;
	if (opts&forward_) {
		rdIdx = IDXCNT_ADD( TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt), -max );
		/* subtract 1 b/c during loop (below) rdIdx is incremented first */
		rdIdx = IDXCNT_ADD( rdIdx, -1 );
		forward_continuous =1;
		for_rev=1;
	}

	buf_slot_width= minw( 3, countDigits(traceControl_p->num_entries-1) );
	local_msg     =	           (char*)malloc( traceControl_p->siz_msg * 3 );/* in case an %ld needs change to %lld */
	msg_cpy       =	           (char*)malloc( traceControl_p->siz_msg * 3 );/* in case an %ld needs change to %lld */
	params_cpy    =	  (unsigned long*)malloc( traceControl_p->num_params*sizeof(uint64_t) );
	local_params  =	        (uint8_t*)malloc( traceControl_p->num_params*sizeof(uint64_t) );
	params_sizes  = (struct sizepush*)malloc( traceControl_p->num_params*sizeof(struct sizepush) );

	if (ospec[0] == 'H')
	{	++ospec; /* done with Heading flag */
		for (sp=ospec; *sp; ++sp)
		{
			switch (*sp)
			{
			case 'N': printf("%*s ", minw(3,countDigits(max-1)), "idx" ); break;
			case 's': printf("%*s ", buf_slot_width, "slt" ); break;
			case 'T': if(tfmt_len)printf("%*.*s ", tfmt_len,tfmt_len,&("us_tod"[tfmt_len>=6?0:6-tfmt_len])); break;
			case 't': printf("       tsc "); break;
			case 'i': printf("%*s ", TRACE_TID_WIDTH, "tid" ); break; /* darwin 16 tids are routinely 7 digit */
			case 'I': printf("TID "); break;
			case 'n': printf("%*s ", longest_name,"name");break;
			case 'C': printf("cpu "); break;
			case 'l': printf("lv "); break;
			case 'L': printf("lvl "); break;
			case 'B': printf("B "); break;
			case 'P': printf("  pid "); break;
			case 'R': printf("r "); break;
			case '#': printf("args ");break;
				/* ignore other unknown chars in ospec */
			}
		}
		printf("msg\n");
		for (sp=ospec; *sp; ++sp)
		{
# define TRACE_LONG_DASHES "------------------------------------------------"
			switch (*sp)
			{
			case 'N': printf("%.*s ", minw(3,countDigits(max-1)), TRACE_LONG_DASHES); break;
			case 's': printf("%.*s ", buf_slot_width, TRACE_LONG_DASHES); break;
			case 'T': if(tfmt_len)printf("%.*s ", tfmt_len, TRACE_LONG_DASHES); break;
			case 't': printf("---------- "); break;
			case 'i': printf("%.*s ", TRACE_TID_WIDTH, TRACE_LONG_DASHES); break; /* darwin 16 tids are routinely 7 digit */
			case 'I': printf("--- "); break;
			case 'n': printf("%.*s ", longest_name,TRACE_LONG_DASHES);break;
			case 'C': printf("--- "); break;
			case 'l': printf("-- "); break;
			case 'L': printf("--- "); break;
			case 'B': printf("-- "); break;
			case 'P': printf("----- "); break;
			case 'R': printf("- "); break;
			case '#': printf("---- ");break;
				/* ignore other unknown chars in ospec */
			}
		}
		printf("-----------------------------\n");
	}

	for (printed=0; printed<(unsigned)max || forward_continuous; ++printed)
	{	time_t seconds; unsigned useconds;
		int print_just_converted_ofmt=0;
		rdIdx = IDXCNT_ADD( rdIdx, for_rev );
		if (opts&forward_) {
			uint32_t now, ini;
 forward_check:
			ini=TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
			while (rdIdx == (now=TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt)))
				usleep( 100000 );
			/* attempt to detect a treset which occurred _while_sleeping_ */
			uint32_t delta=now-ini;
			/*fprintf(stderr,"rdIdx=%u now=%u old=%u delta=%u\n",rdIdx,now,ini,delta);*/
			if (delta > 0x80000000) {
				rdIdx = 0;
				goto forward_check;
			}
		}
		myEnt_p = idxCnt2entPtr( rdIdx );
		msg_p	 = (char*)(myEnt_p+1);
		params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);
		/* not do the copy and reset ptrs to the copies. This avoids */
		myEnt_cpy = *myEnt_p; /* potential issues with overwrite */
		myEnt_p = &myEnt_cpy; /* due to the fact that entries can to created much faster than they can be printed */
		strncpy( msg_cpy, msg_p, traceControl_p->siz_msg );
		msg_p = msg_cpy;
		memcpy( params_cpy, params_p, myEnt_p->nargs*sizeof(uint64_t) );
		params_p = params_cpy;
		
		msg_p[traceControl_p->siz_msg - 1] = '\0';

		if (myEnt_p->nargs)
			get_arg_sizes(	local_msg, msg_p, opts
			              , myEnt_p->param_bytes, params_sizes, myEnt_p->nargs );
		else {
			if (opts&filter_newline_) {
				for (ii=0; msg_p[ii]!='\0' && ii<traceControl_p->siz_msg; ++ii) {
					local_msg[ii] = (msg_p[ii]!='\n')?msg_p[ii]:';';
				}
				if (ii<traceControl_p->siz_msg) local_msg[ii]='\0';
			}
			else
				strncpy( local_msg, msg_p, traceControl_p->siz_msg );
		}

		/* determine if args need to be copied */
		if        (	 ((myEnt_p->param_bytes==4) && (sizeof(long)==4))
		           ||((myEnt_p->param_bytes==8) && (sizeof(long)==8)) )
		{	seconds	 = myEnt_p->time.tv_sec;
			useconds = myEnt_p->time.tv_usec;
			param_va_ptr = (void*)params_p;
		}
		else if (  ((myEnt_p->param_bytes==4) && (sizeof(long)==8)) )
		{	void *xx = &(myEnt_p->time);
			unsigned *ptr=(unsigned*)xx;
			seconds	 = *ptr++;
			useconds = *ptr;
			ent_param_ptr = (uint8_t*)params_p;
			lcl_param_ptr = local_params;
			for (ii=0; ii<myEnt_p->nargs && params_sizes[ii].push!=0; ++ii)
			{
				if      (params_sizes[ii].push == 4)
				{	*(long*)lcl_param_ptr = (long)*(int*)ent_param_ptr;
					lcl_param_ptr += sizeof(long);
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
		{	void *xx=&myEnt_p->time;
			long long *ptr=(long long*)xx;
			seconds	 = (unsigned)*ptr++;
			useconds = (unsigned)*ptr;
			ent_param_ptr = (uint8_t*)params_p;
			lcl_param_ptr = local_params;
			for (ii=0; ii<myEnt_p->nargs && params_sizes[ii].push!=0; ++ii)
			{
				if      (params_sizes[ii].size == 4)
				{	*(unsigned*)lcl_param_ptr = (unsigned)*(unsigned long long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long);
				}
				else /* (params_sizes[ii].size == 8) */
				{	*(unsigned long long*)lcl_param_ptr = *(unsigned long long*)ent_param_ptr;
					lcl_param_ptr += sizeof(long long);
				}
				ent_param_ptr += params_sizes[ii].push;
			}
			param_va_ptr = (void*)local_params;
		}
		for (sp=ospec; *sp; ++sp)
		{
			switch (*sp)
			{
			case 'N': printf("%*u ", minw(3,countDigits(max-1)), printed ); break;
			case 's': printf("%*u ", buf_slot_width, rdIdx%traceControl_p->num_entries ); break;
			case 'T': if(tfmt_len){ strftime(tbuf,sizeof(tbuf),tfmt,localtime(&seconds)); strcat(tbuf," ");
					  printf(tbuf, useconds); } break;
			case 't': printf("%10u ", (unsigned)myEnt_p->tsc); break;
			case 'i': printf("%*d ", TRACE_TID_WIDTH, myEnt_p->tid); break; /* darwin 16 tids are routinely 7 digit */
			case 'I': printf("%3u ", myEnt_p->TrcId); break;
			case 'n': printf("%*.*s ",longest_name,longest_name,traceNamLvls_p[myEnt_p->TrcId].name);break;
			case 'C': printf("%3u ", myEnt_p->cpu); break;
			case 'l': printf("%2d ", myEnt_p->lvl); break;
			case 'L': printf("%*s ", longest_lvlstr, _lvlstr[(myEnt_p->lvl)&LVLBITSMSK]); break;
			case 'B': printf("%u ", myEnt_p->param_bytes); break;
			case 'P': printf("%5d ", myEnt_p->pid); break; /* /proc/sys/kernel/pid_max has 32768 */
			case 'R':
				if (myEnt_p->get_idxCnt_retries) printf( "%u ", myEnt_p->get_idxCnt_retries );
				else							 printf( ". " );
				break;
			case 'm': print_just_converted_ofmt=1; break;
			case '#': printf("%4u ", myEnt_p->nargs); break;
			}
		}

		/*typedef unsigned long parm_array_t[1];*/
		/*va_start( ap, params_p[-1] );*/
		if (!print_just_converted_ofmt && myEnt_p->nargs)
		{	/* Ref. http://andrewl.dreamhosters.com/blog/variadic_functions_in_amd64_linux/index.html
			   Here, I need an initializer so I must have this inside braces { } */
			va_list ap_=TRACE_VA_LIST_INIT(param_va_ptr);
			if (opts&indent_) {
				int ii=(myEnt_p->lvl>63)?63:myEnt_p->lvl;
				printf("%s",&indent[63-ii]);
			}
			vprintf( local_msg, ap_ );
			printf("\n");
		}
		else /* print_just_converted_ofmt */
			printf("%s\n",local_msg);
		fflush(stdout);
	}
}	/*traceShow*/


void traceInfo()
{
	uint32_t       used;
	TRACE_ATOMIC_T wrCopy;
	TRACE_ATOMIC_T nameLockCopy;
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
	TRACE_ATOMIC_STORE( &nameLockCopy, TRACE_ATOMIC_LOAD(&traceControl_rwp->namelock) );
	TRACE_ATOMIC_STORE( &wrCopy,       TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt) );
	used = ((traceControl_rwp->full)
		?traceControl_p->num_entries
		:TRACE_ATOMIC_LOAD(&wrCopy) ); /* if not full this shouldn't be > traceControl_p->num_entries */
	printf("trace.h rev       = %s\n"
	       "revision          = %s\n"
	       "create time       = %s\n"
	       "trace_initialized = %d\n"
	       "mode              = 0x%-8x  %s%s\n"
	       "writeIdxCount     = 0x%08x  entries used: %u\n"
	       "full              = %d\n"
	       "nameLock          = %u\n"
	       "largestMultiple   = 0x%08x\n"
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
	       "default TRACE_SHOW=%s others: t(tsc) B(paramBytes) s(slot) m(convertedMsgfmt_only) D(inDent) I(TID) #(nargs) l(lvl int)\n"
	       , TRACE_REV
	       , traceControl_p->version_string
	       , outstr
	       , traceControl_p->trace_initialized
	       , traceControl_rwp->mode.mode, traceControl_rwp->mode.bits.S?"Slow:ON, ":"Slow:off", traceControl_rwp->mode.bits.M?" Mem:ON":" Mem:off"
	       , wrCopy, used
	       , traceControl_rwp->full
	       , nameLockCopy
	       , traceControl_p->largest_multiple
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
	int         opt_loops=-1;
	unsigned    opt_dly_ms=0;
	unsigned    opt_burst=1;
	int         opt_all=0;
	const char *opt_name=NULL;

	while ((opt=getopt(argc,argv,"?hn:N:f:x:HqVL:Fl:d:b:a")) != -1) {
		switch (opt) {
		/*   '?' is also what you get w/ "invalid option -- -"   */
		case '?': case 'h': printf(USAGE);exit(0);           break;
		case 'N': opt_name=optarg;                           break;
		case 'n': setenv("TRACE_NAME",optarg,1);             break;
		case 'f': setenv("TRACE_FILE",optarg,1);             break;
		case 'x': trace_thread_option=strtoul(optarg,NULL,0);break;
		case 'H': do_heading=0;                              break;
		case 'q': show_opts|=quiet_;                         break;
		case 'V': printf( TRACE_CNTL_REV ); exit(0);         break;
		case 'L': setlocale(LC_NUMERIC,optarg);              break;
        case 'F': show_opts|=forward_;                       break;
		case 'l': opt_loops=strtoul(optarg,NULL,0);          break;
		case 'd': opt_dly_ms=strtoul(optarg,NULL,0);         break;
		case 'b': opt_burst=strtoul(optarg,NULL,0);          break;
		case 'a': opt_all=1;                                 break;
		}
	}
	if (argc - optind < 1) {
		printf( "Need cmd\n" );
		printf( USAGE ); exit( 0 );
	}
	cmd = argv[optind++];

	if (opt_name && !(strncmp(cmd,"lvl",3)==0))
		setenv("TRACE_NAME",opt_name,1);

	if(getenv("LC_NUMERIC"))
		setlocale(LC_NUMERIC,getenv("LC_NUMERIC"));

	if		(strcmp(cmd,"test1") == 0)
	{	int loops=1;
		if (opt_loops > -1) loops=opt_loops;
		for (; loops; --loops)
		{	TRACE( 2, "Hello %d. \"c 2.5 5 5000000000 15\" should be repeated here: %c %.1f %hd %lld %d"
				  , loops, 'c',2.5,(short)5,(long long)5000000000LL,15 );
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
		setenv("TRACE_LVLS","0xff",0);/*does TRACE_CNTL("lvlsetS",0xffLL);TRACE_CNTL("modeS",1LL);*/
		/* NOTE/Recall - _LVLS does not "activate" like TRACE_{FILE,NAME,MSGMAX,NUMENTS,NAMTBLENTS} */

		/* _at_least_ set bit 0 (lvl=0) in the "M" mask and turn on the "M" mode
		   bit -- this is what is on by default when the file is created */
		/*                    Mem    Slow Trig */
		TRACE_CNTL( "lvlset", 0xfLL, 0LL, 0LL );
		TRACE_CNTL( "modeM", 1LL );

		TRACE( 2, "hello" );

		myIdx = traceControl_p->largest_multiple - 3;
		printf("myIdx=0x%08x\n", myIdx );
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,1);
			printf( "myIdx==>myIdx+1: 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,-1);
			printf( "myIdx==>myIdx-1: 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		printf("myIdx=0x%08x\n", myIdx );

		xx=5;
		myIdx = traceControl_p->largest_multiple - (3*xx);
		printf("myIdx=0x%08x\n", myIdx );
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,xx);
			printf( "myIdx==>myIdx+5: 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		for (ii=0; ii<6; ++ii)
		{	desired = IDXCNT_ADD(myIdx,-xx);  /* NOTE: this will not work if xx is of type uint32_t; it must be of int32_t */
			printf( "myIdx==>myIdx-5: 0x%08x 0x%08x\n",myIdx, desired );
			myIdx = desired;
		}
		printf("myIdx=0x%08x\n", myIdx );


		TRACE( 1, "hello %d\nthere\n", 1 );
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
		TRACE_CNTL( "trig", (uint64_t)-1, 5LL );
#	   endif
		for (ii=0; ii<20; ++ii)
			TRACE( 2, "ii=%u", ii );
	}
	else if (strcmp(cmd,"test-ro") == 0)
	{
		/* To test normal userspace, can use: TRACE_FILE= trace_cntl test-ro
		 */
		setenv("TRACE_FILE","/proc/trace/buffer",0);
		traceInit(NULL);
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

		TRACE_CNTL("mode",3LL);
		traceControl_rwp->mode.bits.M = 1;		   // NOTE: TRACE_CNTL("modeM",1LL) hardwired to NOT enable when not mapped!

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
	{	int start=-1, count=-1;
		const char *ospec=getenv("TRACE_SHOW");
		if (!ospec) ospec=DFLT_SHOW;
		if ((do_heading==0) && (ospec[0]=='H')) ++ospec;
		if ((argc-optind)>=1) count=strtoul(argv[optind],  NULL,0);
		if ((argc-optind)>=2) start=strtoul(argv[optind+1],NULL,0);
		traceShow(ospec,count,start,show_opts);
	}
	else if (strncmp(cmd,"info",4) == 0)
	{
		traceInit(NULL);
		traceInfo();
	}
	else if (strcmp(cmd,"unlock") == 0)
	{	traceInit(NULL);
		trace_unlock( &traceControl_rwp->namelock );
	}
	else if (strcmp(cmd,"tids") == 0)
	{	int longest_name=0, namLvlTblEnts_digits;
		traceInit(NULL);
		/* calc longest name */
		for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
			if ((int)strnlen(traceNamLvls_p[ii].name,sizeof(traceNamLvls_p[0].name)) > longest_name)
				longest_name = (int)strnlen(traceNamLvls_p[ii].name,sizeof(traceNamLvls_p[0].name));
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
		switch (argc - optind) {
		case 0: printf("\"trace\" cmd requires at least lvl and fmt arguments."); break;
		case 1: printf("\"trace\" cmd requires at least lvl and fmt arguments."); break;
		case 2: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1] );
			break;
		case 3: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0) );
			break;
		case 4: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0) );
			break;
		case 5: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0) );
			break;
		case 6: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0) );
			break;
		case 7: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0)
		              ,strtoull(argv[optind+6],NULL,0) );
			break;
		case 8: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0)
		              ,strtoull(argv[optind+6],NULL,0),strtoull(argv[optind+7],NULL,0) );
			break;
		case 9: TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0)
		              ,strtoull(argv[optind+6],NULL,0),strtoull(argv[optind+7],NULL,0)
		              ,strtoull(argv[optind+8],NULL,0) );
			break;
		case 10:TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0)
		              ,strtoull(argv[optind+6],NULL,0),strtoull(argv[optind+7],NULL,0)
		              ,strtoull(argv[optind+8],NULL,0),strtoull(argv[optind+9],NULL,0) );
			break;
		case 11:TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0)
		              ,strtoull(argv[optind+6],NULL,0),strtoull(argv[optind+7],NULL,0)
		              ,strtoull(argv[optind+8],NULL,0),strtoull(argv[optind+9],NULL,0)
		              ,strtoull(argv[optind+10],NULL,0) );
			break;
		case 12:TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0)
		              ,strtoull(argv[optind+6],NULL,0),strtoull(argv[optind+7],NULL,0)
		              ,strtoull(argv[optind+8],NULL,0),strtoull(argv[optind+9],NULL,0)
		              ,strtoull(argv[optind+10],NULL,0),strtoull(argv[optind+11],NULL,0) );
			break;
		case 13:TRACE( strtoull(argv[optind+0],NULL,0),argv[optind+1]
		              ,strtoull(argv[optind+2],NULL,0),strtoull(argv[optind+3],NULL,0)
		              ,strtoull(argv[optind+4],NULL,0),strtoull(argv[optind+5],NULL,0)
		              ,strtoull(argv[optind+6],NULL,0),strtoull(argv[optind+7],NULL,0)
		              ,strtoull(argv[optind+8],NULL,0),strtoull(argv[optind+9],NULL,0)
		              ,strtoull(argv[optind+10],NULL,0),strtoull(argv[optind+11],NULL,0)
		              ,strtoull(argv[optind+12],NULL,0) );
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
		case 3: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2] );
			break;
		case 4: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		               ,strtoull(argv[optind+3],NULL,0) );
			break;
		case 5: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		               ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0) );
			break;
		case 6: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0) );
			break;
		case 7: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0) );
			break;
		case 8: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0)
		              ,strtoull(argv[optind+7],NULL,0) );
			break;
		case 9: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0)
		              ,strtoull(argv[optind+7],NULL,0),strtoull(argv[optind+8],NULL,0) );
			break;
		case 10: TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0)
		              ,strtoull(argv[optind+7],NULL,0),strtoull(argv[optind+8],NULL,0)
		              ,strtoull(argv[optind+9],NULL,0) );
			break;
		case 11:TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0)
		              ,strtoull(argv[optind+7],NULL,0),strtoull(argv[optind+8],NULL,0)
		              ,strtoull(argv[optind+9],NULL,0),strtoull(argv[optind+10],NULL,0) );
			break;
		case 12:TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0)
		              ,strtoull(argv[optind+7],NULL,0),strtoull(argv[optind+8],NULL,0)
		              ,strtoull(argv[optind+9],NULL,0),strtoull(argv[optind+10],NULL,0)
		              ,strtoull(argv[optind+11],NULL,0) );
			break;
		case 13:TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0)
		              ,strtoull(argv[optind+7],NULL,0),strtoull(argv[optind+8],NULL,0)
		              ,strtoull(argv[optind+9],NULL,0),strtoull(argv[optind+10],NULL,0)
		              ,strtoull(argv[optind+11],NULL,0),strtoull(argv[optind+12],NULL,0) );
			break;
		case 14:TRACEN( argv[optind],strtoull(argv[optind+1],NULL,0),argv[optind+2]
		              ,strtoull(argv[optind+3],NULL,0),strtoull(argv[optind+4],NULL,0)
		              ,strtoull(argv[optind+5],NULL,0),strtoull(argv[optind+6],NULL,0)
		              ,strtoull(argv[optind+7],NULL,0),strtoull(argv[optind+8],NULL,0)
		              ,strtoull(argv[optind+9],NULL,0),strtoull(argv[optind+10],NULL,0)
		              ,strtoull(argv[optind+11],NULL,0),strtoull(argv[optind+12],NULL,0)
		              ,strtoull(argv[optind+13],NULL,0) );
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
		else ret=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0) );
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
		}
	}
	return (ret);
}	/* main */
