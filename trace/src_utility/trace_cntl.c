/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    rev="$Revision: 1.65 $$Date: 2014/03/10 15:15:09 $";
    */
/*
NOTE: This is a .c file instead of c++ mainly because C is friendlier when it
      comes to extended initializer lists.
gxx_standards.sh trace_cntl.c
for std in $standards;do\
  for ll in 0 1 2 3;do echo $std $ll:;TRACE_LVL=$ll ./Trace_test-$std;done;\
done
   
*/
#include <stdio.h>		/* printf */
#include <stdint.h>		/* uint64_t */
#include <libgen.h>		/* basename */
#include <unistd.h>		/* getopt */
#include <sys/time.h>           /* gettimeofday, struct timeval */
#include <pthread.h>		/* pthread_self */
#include <sys/syscall.h>	/* syscall */

#include "trace.h"


#define USAGE "\
%s [opts] <cmd> [command opt/args]\n\
commands:\n\
 info, tids, reset, show\n\
 mode <mode>\n\
 modeM <mode>\n\
 modeS <mode>\n\
 lvlmskM <msk>  mask for Memory buffer\n\
 lvlmskS <msk>  mask for stdout\n\
 lvlmskT <msk>  mask for trigger\n\
 trig <modeMsk> <lvlmskM> <postEntries>\n\
opts:\n\
 -f<file>\n\
 -n<name>\n\
tests:  (use %s show after test)\n\
%s\n\
", basename(argv[0]), basename(argv[0]), USAGE_TESTS
#define USAGE_TESTS "\
 test1 [loops]  a single TRACE [or more if loops != 0]\n\
 test           various\n\
 test-ro        test first page of mmap read-only (for kernel module)\n\
 test-compare   compare TRACE fmt+args vs. format+args converted (via sprintf)\n\
 test-threads   threading\n\
 TRACE <lvl> <fmt> [ulong]...   (just ulong args are supported\n\
"

#if __SIZEOF_LONG__ == 8
#  define LX "lx"
#  define LU "lu"
#else
#  define LX "llx"
#  define LU "llu"
#endif

#define NUMTHREADS 4
static int trace_thread_option=0;

void traceInfo( void ); /* forward decl */

void* thread_func(void *arg)
{
    long loops=(long)arg;
    char tmp[PATH_MAX];
    long tid;
    if      (trace_thread_option == 1)
    {   /* IF -std=c11 is NOT used, a seg fault usually occurs if default file does not exit */
	tid = (long)syscall(SYS_GETTID);
	snprintf( tmp, sizeof(tmp),"/tmp/trace_buffer_%ld",tid );
	TRACE_CNTL( "file", tmp );
    }
    else if (trace_thread_option == 2)
    {   tid = (long)syscall(SYS_GETTID);
	snprintf( tmp, sizeof(tmp), "T%ld", tid );
	printf( "setting name to %s\n",tmp );
	printf("%ld: traceControl_p=%p\n",tid,(void*)traceEntries_p);
	TRACE_CNTL( "name", tmp );
    }

    if (loops < NUMTHREADS)
    {
	sleep( loops+1 );
	printf( "      traceControl_p=%p      traceEntries_p=%p\n"
	       ,(void*)traceEntries_p, (void*)traceEntries_p );
	traceInfo();
    }
    else
	while(loops-- > 0)
	{   TRACE( 0, "loops=%ld", loops );
	    TRACE( 0, "loops=%ld", --loops );
	    TRACE( 0, "loops=%ld", --loops );
	    TRACE( 0, "loops=%ld", --loops );
	}
    pthread_exit(NULL);
}


uint64_t get_us_timeofday()
{   struct timeval tv;
    gettimeofday( &tv, NULL );
    return (uint64_t)tv.tv_sec*1000000+tv.tv_usec;
}


struct sizepush
{   unsigned size:16;
    unsigned push:16;
};

void get_arg_sizes( char *fmt, int num_params, int param_bytes, struct sizepush *sizes_out )
{   char    *in;
    char    *percent_sav;
    int      numArgs=0;
    int      maxArgs=10;
    int      modifier=0;
    int      skip=0;
    in = fmt;
    while ((in=strchr(in,'%')))
    {   percent_sav = in;       /* save in case we need to modify it (too many args) */
	++in;  			/* point to next char */
	/*printf("next char=%c\n",*in);*/
	if ((*in=='%')||(*in=='m')) { ++in; continue; }/* ingore %% which specified a % char */
	if (numArgs == maxArgs) { *percent_sav = '*'; continue; }  /* SAFETY - no args beyond max */
	if ((skip=strspn(in,"0123456789.-# +'I"))) in+=skip;
 chkChr:
	switch (*in++)
	{
	    /* Basic conversion specifiers */
	case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
	    switch (modifier)
	    {
	    case -2: sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4; break; /* char */
	    case -1: sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4;break;/* short */
	    case 0:  sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4;break;/* int */
	    case 1:  sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=param_bytes;break;/* long */
	    case 2:  sizes_out[numArgs].push=8;          sizes_out[numArgs].size=8;break;/* long long */
	    default: printf("error\n");
	    }
	    modifier=0;
	    break;
	case 'e': case 'E': case 'f': case 'F': case 'g': case 'G': case 'a': case 'A':
	    if (modifier)
	    {   sizes_out[numArgs].push=(param_bytes==4)?12:16;
		sizes_out[numArgs].size=(param_bytes==4)?12:16;
		modifier=0;
	    }/* long double */
	    else
	    {   sizes_out[numArgs].push=8;
		sizes_out[numArgs].size=8;
	    } /* double */
	    break;

	    /* length modifiers */
	case 'h':  --modifier; goto chkChr;
	case 'l':  ++modifier; goto chkChr;
	case 'L':  ++modifier; goto chkChr;

	case 's': case 'n': case 'p':       /* SAFETY -- CONVERT %s to %p */
	    *(in-1)='p';
	    break;
	case 'm':
	    continue;
	    break;
	case '*':   /* special -- "arg" in an arg */
	    sizes_out[numArgs].push=param_bytes;sizes_out[numArgs].size=4;
	    if (++numArgs == maxArgs) break;
	    goto chkChr;
	default:
	    printf("unknown\n");
	}
	++numArgs;
    }
    if (numArgs < maxArgs) sizes_out[numArgs].push=0;
}

void traceShow( const char *ospec, int do_heading, int start, unsigned count )
{
    uint32_t rdIdx;
    uint32_t max;
    unsigned printed=0;
    unsigned ii;
    struct traceEntryHdr_s* myEnt_p;
    char                  * msg_p;
    unsigned long         * params_p;
    uint8_t               * local_params;
    char                  * local_msg;
    void                  * param_va_ptr;
    struct sizepush       * params_sizes;
    uint8_t               * ent_param_ptr;
    uint8_t               * lcl_param_ptr;
    const char            * sp; /*spec ptr*/

    traceInit();
    if (start == -1)
    {   rdIdx=traceControl_p->wrIdxCnt;
	max=((rdIdx<=traceControl_p->num_entries)
	     ?rdIdx:traceControl_p->num_entries);
    }
    else
    {   rdIdx = (unsigned)start>=traceControl_p->num_entries
	    ? traceControl_p->num_entries-1
	    : start;
	max = count<=traceControl_p->num_entries
	    ? count
	    : traceControl_p->num_entries;
    }
    local_msg    =            (char*)malloc( traceControl_p->siz_msg );
    local_params =         (uint8_t*)malloc( traceControl_p->num_params*sizeof(uint64_t) );
    params_sizes = (struct sizepush*)malloc( traceControl_p->num_params*sizeof(struct sizepush) );

    if (do_heading)
    {   sp = ospec;
	for (sp=ospec; *sp; ++sp)
	{
	    switch (*sp)
	    {
	    case 'N': printf("   idx "); break;
	    case 'T': printf("          us_tod "); break;
	    case 't': printf("       tsc "); break;
	    case 'i': printf("  tid "); break;
	    case 'I': printf("TID "); break;
	    case 'L': printf("lv "); break;
	    case 'B': printf("B "); break;
	    case 'P': printf("  pid "); break;
	    case 'R': printf("r "); break;
	    }
	}
	printf("msg\n");
	sp = ospec;
	for (sp=ospec; *sp; ++sp)
	{
	    switch (*sp)
	    {
	    case 'N': printf("------ "); break;
	    case 'T': printf("---------------- "); break;
	    case 't': printf("---------- "); break;
	    case 'i': printf("----- "); break;
	    case 'I': printf("--- "); break;
	    case 'L': printf("-- "); break;
	    case 'B': printf("-- "); break;
	    case 'P': printf("------ "); break;
	    case 'R': printf("- "); break;
	    }
	}
	printf("-----------------------------\n");
    }
    for (printed=0; printed<max; ++printed)
    {   unsigned seconds, useconds;
	rdIdx = IDXCNT_ADD( rdIdx, -1 );
	myEnt_p = idxCnt2entPtr( rdIdx );
	msg_p    = (char*)(myEnt_p+1);
	params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);
	msg_p[traceControl_p->siz_msg - 1] = '\0';
	strcpy( local_msg, msg_p );
	get_arg_sizes(  local_msg, traceControl_p->num_params
		      , myEnt_p->param_bytes, params_sizes );

	if        (  ((myEnt_p->param_bytes==4) && (sizeof(long)==4))
		   ||((myEnt_p->param_bytes==8) && (sizeof(long)==8)) )
	{   seconds  = myEnt_p->time.tv_sec;
	    useconds = myEnt_p->time.tv_usec;
	    param_va_ptr = (void*)params_p;
	} else if (  ((myEnt_p->param_bytes==4) && (sizeof(long)==8)) )
	{   unsigned *ptr=(unsigned*)&myEnt_p->time;
	    seconds  = *ptr++;
	    useconds = *ptr;
	    ent_param_ptr = (uint8_t*)params_p;
	    lcl_param_ptr = local_params;
	    for (ii=0; ii<traceControl_p->num_params && params_sizes[ii].push!=0; ++ii)
	    {
		if      (params_sizes[ii].push == 4)
		{   *(long*)lcl_param_ptr = (long)*(int*)ent_param_ptr;
		    lcl_param_ptr += sizeof(long);
		}
		else /* (params_sizes[ii].push == 8) */
		{   *(long*)lcl_param_ptr =      *(long*)ent_param_ptr;
		    lcl_param_ptr += sizeof(long);
		}
		ent_param_ptr += params_sizes[ii].push;
	    }
	    param_va_ptr = (void*)local_params;
	} else /* (  ((myEnt_p->param_bytes==8) && (sizeof(long)==4)) ) */
	{   long long *ptr=(long long*)&myEnt_p->time;
	    seconds  = (unsigned)*ptr++;
	    useconds = (unsigned)*ptr;
	    ent_param_ptr = (uint8_t*)params_p;
	    lcl_param_ptr = local_params;
	    for (ii=0; ii<traceControl_p->num_params && params_sizes[ii].push!=0; ++ii)
	    {
		if      (params_sizes[ii].size == 4)
		{   *(unsigned*)lcl_param_ptr = (unsigned)*(unsigned long long*)ent_param_ptr;
		    lcl_param_ptr += sizeof(long);
		}
		else /* (params_sizes[ii].size == 8) */
		{   *(unsigned long long*)lcl_param_ptr = *(unsigned long long*)ent_param_ptr;
		    lcl_param_ptr += sizeof(long long);		    
		}
		ent_param_ptr += params_sizes[ii].push;
	    }
	    param_va_ptr = (void*)local_params;
	}
	sp = ospec;
	for (sp=ospec; *sp; ++sp)
	{
	    switch (*sp)
	    {
	    case 'N': printf("%6u ", printed ); break;
	    case 'T': printf("%10u%06u ", seconds, useconds); break;
	    case 't': printf("%10u ", (unsigned)myEnt_p->tsc); break;
	    case 'i': printf("%5d ", myEnt_p->tid); break;
	    case 'I': printf("%3u ", myEnt_p->TID); break;
	    case 'L': printf("%2d ", myEnt_p->lvl); break;
	    case 'B': printf("%u ", myEnt_p->param_bytes); break;
	    case 'P': printf("%6d ", myEnt_p->pid); break;
	    case 'R':
		if (myEnt_p->get_idxCnt_retries) printf( "%u ", myEnt_p->get_idxCnt_retries );
		else                             printf( ". " );
	    }
	}

	/*typedef unsigned long parm_array_t[1];*/
	/*va_start( ap, params_p[-1] );*/
	{   /* Ref. http://andrewl.dreamhosters.com/blog/variadic_functions_in_amd64_linux/index.html
	     */
	    va_list ap=TRACE_VA_LIST_INIT(param_va_ptr);
	    vprintf( local_msg, ap );
	}
	printf("\n");
    }
}   /*traceShow*/


void traceInfo()
{
	uint32_t wrCopy, used;
	uint32_t spinlockCopy;
	spinlockCopy = traceControl_p->spinlock;
	wrCopy = traceControl_p->wrIdxCnt;
	used = ((wrCopy<=traceControl_p->num_entries)
		?wrCopy
		:traceControl_p->num_entries);
	printf("trace.h rev       = %s\n"
	       "revision          = %s\n"
	       "trace_initialized =%d\n"
	       "mode              =0x%x\n"
	       "writeIdxCount     =0x%08x entries used: %u\n"
	       "lock              =%u\n"
               "largestMultiple   =0x%08x\n"
	       "trigIdxCnt        =0x%08x\n"
	       "triggered         =%d\n"
	       "trigActivePost    =%u\n"
	       "traceLevel        =0x%0*" LX " 0x%0*" LX "\n"
	       "num_entries       =%u\n"
	       "max_msg_sz        =%u  includes system inforced terminator\n"
	       "max_params        =%u\n"
	       "entry_size        =%u\n"
	       "namLvlTbl_ents    =%u\n"
	       "namLvlTbl_name_sz =%u\n"
	       "wrIdxCnt offset   =%p\n"
	       "namLvls offset    =0x%lx\n"
	       "buffer_offset     =0x%lx\n"
	       "memlen            =%u\n"
	       , TRACE_REV
	       , traceControl_p->version_string
	       , traceControl_p->trace_initialized
	       , traceControl_p->mode.mode
	       , wrCopy, used
	       , spinlockCopy
	       , traceControl_p->largest_multiple
	       , traceControl_p->trigIdxCnt
	       , traceControl_p->triggered
	       , traceControl_p->trigActivePost
	       , (int)sizeof(uint64_t)*2, traceNamLvls_p[traceTID].M
	       , (int)sizeof(uint64_t)*2, traceNamLvls_p[traceTID].S
	       , traceControl_p->num_entries
	       , traceControl_p->siz_msg
	       , traceControl_p->num_params
	       , traceControl_p->siz_entry
	       , traceControl_p->num_namLvlTblEnts
	       , (int)sizeof(traceNamLvls_p[0].name)
	       , (void*)&((struct traceControl_s*)0)->wrIdxCnt
	       , (unsigned long)traceNamLvls_p - (unsigned long)traceControl_p
	       , (unsigned long)traceEntries_p - (unsigned long)traceControl_p
	       , traceControl_p->memlen
	       );
}   /* traceInfo */



int main(  int	argc
	 , char	*argv[] )
{
	const char *cmd;
extern  char       *optarg;        /* for getopt */
extern  int        optind;         /* for getopt */
        int        opt;            /* for how I use getopt */
	int	   do_heading=1;
	unsigned   ii=0;

    while ((opt=getopt(argc,argv,"?hn:f:x:H")) != -1)
    {   switch (opt)
        { /* '?' is also what you get w/ "invalid option -- -" */
        case '?': case 'h': printf(USAGE);exit(0);           break;
	case 'n': setenv("TRACE_NAME",optarg,1);             break;
	case 'f': setenv("TRACE_FILE",optarg,1);             break;
	case 'x': trace_thread_option=strtoul(optarg,NULL,0);break;
	case 'H': do_heading=0;                              break;
        }
    }
    if (argc - optind < 1)
    {   printf( "Need cmd\n" );
        printf( USAGE ); exit( 0 );
    }
    cmd = argv[optind++];


    if      (strcmp(cmd,"test1") == 0)
    {   unsigned loops=0;
	if (argc - optind == 1) loops=strtoul(argv[optind],NULL,0);
	do { TRACE( 0, "Hello. \"1 2.5 5 10 15\" should be repeated here: %d %.1f %d %d %d",1,2.5,5,10,15 ); } while (loops--);
    }
    else if (strcmp(cmd,"test") == 0)
    {   float    ff[10];
	pid_t	 tid;
	uint32_t desired, myIdx;

#      if   defined(__cplusplus)      &&      (__cplusplus >= 201103L)
	tid = (pid_t)syscall( SYS_GETTID );
#      elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
	tid = (pid_t)syscall( SYS_GETTID );
#      elif !defined(__sparc__)
	tid = (pid_t)syscall( SYS_GETTID );
#      endif
	if (tid == -1) perror("syscall");
	printf("tid=%ld\n", (long int)syscall(SYS_GETTID) );

	printf("sizeof: int:%u long:%u pid_t:%u pthread_t:%u timeval:%u "
	       "double:%u traceControl_s:%u traceEntryHdr_s:%u\n"
	       , (int)sizeof(int), (int)sizeof(long), (int)sizeof(pid_t)
	       , (int)sizeof(pthread_t), (int)sizeof(struct timeval)
	       , (int)sizeof(double), (int)sizeof(struct traceControl_s)
	       , (int)sizeof(struct traceEntryHdr_s));
	printf("offset: trigOffMode    =%p\n"
	       "        trigIdxCount   =%p\n"
	       "        trigActivePost =%p\n"
	       "        full           =%p\n"
	       "        lvl            =%p\n"
	       "        pid            =%p\n"
	       "        TID            =%p\n"
	       "        get_retries    =%p\n"
	       "        param_bytes    =%p\n"
	       "        tsc            =%p\n"
	       , (void*)&((struct traceControl_s*)0)->trigOffMode
	       , (void*)&((struct traceControl_s*)0)->trigIdxCnt
	       , (void*)&((struct traceControl_s*)0)->trigActivePost
	       , (void*)&((struct traceControl_s*)0)->full
	       , (void*)&((struct traceEntryHdr_s*)0)->lvl
	       , (void*)&((struct traceEntryHdr_s*)0)->pid
	       , (void*)&((struct traceEntryHdr_s*)0)->TID
	       , (void*)&((struct traceEntryHdr_s*)0)->get_idxCnt_retries
	       , (void*)&((struct traceEntryHdr_s*)0)->param_bytes
	       , (void*)&((struct traceEntryHdr_s*)0)->tsc
	       );

	for (ii=0; ii<sizeof(ff)/sizeof(ff[0]); ++ii)  ff[ii]=2.5*ii;
	TRACE_CNTL( "lvlmskS", 0xfL ); TRACE_CNTL( "lvlmskM", 0xfL );
	TRACE( 0, "hello" );
	myIdx = traceControl_p->largest_multiple - 3;
	printf("myIdx=0x%08x\n", myIdx );
	for (ii=0; ii<6; ++ii)
	{   desired = IDXCNT_ADD(myIdx,1);
	    printf( "myIdx==>myIdx+1: 0x%08x 0x%08x\n",myIdx, desired );
	    myIdx = desired;
	}
	for (ii=0; ii<6; ++ii)
	{   desired = IDXCNT_ADD(myIdx,-1);
	    printf( "myIdx==>myIdx-1: 0x%08x 0x%08x\n",myIdx, desired );
	    myIdx = desired;
	}
	printf("myIdx=0x%08x\n", myIdx );
	TRACE( 1, "hello %d", 1 );
	TRACE( 2, "hello %d %d", 1, 2 );
	TRACE( 3, "hello %d %d %d", 1,2,3 );
	TRACE( 3, "hello %d %d %d %d %d %d %d %d %d %d %d"
	      , 1,2,3,4,5,6,7,8,9,10, 11 );   /* extra param does not get saved in buffer */
	TRACE( 3, "hello %f %f %f %f %f %f %f %f %f %f"
	      , 1.0,2.0,3.0,4.0, ff[5],6.0,7.0,8.0,9.0,10.0 );
	TRACE( 4, "hello %d %d %f  %d %d %f  %d %d"
	      ,           1, 2,3.3,4, 5, 6.6, 7, 8 );

#      ifndef TEST_UNUSED_FUNCTION
	TRACE_CNTL( "trig", 3, (uint64_t)-1, 5 );
#      endif
	for (ii=0; ii<20; ++ii)
	    TRACE( 0, "ii=%u", ii );
    }
    else if (strcmp(cmd,"test-ro") == 0)
    {
	setenv("TRACE_FILE","/proc/trace/buffer",1);
	traceInit();
	printf("try write to (presumably kernel memory) write-protected 1st page...\n");
	traceControl_p->trace_initialized = 2;
	printf("write succeeded.\n");
#      if defined(TEST_WRITE_PAST_END)
	*(((uint8_t*)traceControl_p)+traceControl_p->memlen) = 6;
#      endif
    }
    else if (strcmp(cmd,"test-compare") == 0)
    {   char     buffer[200];
	uint64_t mark;
	unsigned compares=1000; /* some big number */
	if (argc == 3) compares=strtoul(argv[optind+1],NULL,0);

#      define END_FMT "end   in mode 1 delta=%" LU
	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start no snprintf in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "any msg" );
	} TRACE_CNTL("mode",2);TRACE(0,END_FMT,get_us_timeofday()-mark );


	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start snprintf 1 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   snprintf( buffer, sizeof(buffer), "this is one small param: %u", 12345678 );
	    TRACE( 0, buffer );
	} TRACE_CNTL("mode",2);TRACE(0,END_FMT,get_us_timeofday()-mark );

	if (--compares == 0) return (0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start snprintf 2 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   snprintf( buffer, sizeof(buffer), "this is 2 params: %u %u", 12345678, ii );
	    TRACE( 0, buffer );
	} TRACE_CNTL("mode",2);TRACE(0,END_FMT,get_us_timeofday()-mark );

	if (--compares == 0) return (0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start snprintf 8 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   snprintf( buffer, sizeof(buffer)
		     , "this is 8 params: %u %u %u %u %u %u %u %f"
		     , 12345678, ii, ii*2, ii+6
		     , 12345679, ii, ii-7, (float)ii*1.5
		     );
	    TRACE( 0, buffer );
	} TRACE_CNTL("mode",2);TRACE(0,END_FMT,get_us_timeofday()-mark );

	if (--compares == 0) return (0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start TRACE w/8 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "this is 8 params: %u %u %u %u %u %u %u %f"
		     , 12345678, ii, ii*2, ii+6
		     , 12345679, ii, ii-7, (float)ii*1.5
		     );
	} TRACE_CNTL("mode",2);TRACE(0,END_FMT,get_us_timeofday()-mark );

	if (--compares == 0) return (0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start (repeat) no snprintf in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "any msg" );
	} TRACE_CNTL("mode",2);TRACE(0,END_FMT,get_us_timeofday()-mark );
    }
#   ifdef DO_THREADS   /* requires linking with -lpthreads */
    else if (strcmp(cmd,"test-threads") == 0)
    {   pthread_t *threads;
	unsigned   num_threads=NUMTHREADS;
	long loops=10000;
	if ((argc-optind)>=1)
	{   loops=strtoul(argv[optind],NULL,0);
	    printf("loops set to %ld\n", loops );
	}
	loops -= loops%4;	/* assuming thread does 4 TRACEs per loop */
	if ((argc-optind)==2)
	{   num_threads=strtoul(argv[optind+1],NULL,0);
	    printf("num_threads set to %d\n", num_threads );
	}
	threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));

	TRACE( 0, "before pthread_create - loops=%lu (must be multiple of 4)", loops );
	printf("test-threads - before create loop - traceControl_p=%p\n",(void*)traceControl_p);
	for (ii=0; ii<num_threads; ii++)
	{
	    if (loops == num_threads)
		pthread_create(&(threads[ii]),NULL,thread_func,(void*)(unsigned long)ii);
	    else
		pthread_create(&(threads[ii]),NULL,thread_func,(void*)loops);
	}
	for (ii=0; ii<num_threads; ii++)
	{   pthread_join(threads[ii], NULL);
	}
	printf("test-threads - after create loop - traceControl_p=%p\n",(void*)traceControl_p);
	TRACE( 0, "after pthread_join" );
	free( threads );
    }
#   endif
    else if (strcmp(cmd,"show") == 0) 
    {   int start=-1;
	const char *ospec=getenv("TRACE_SHOW");
	if (!ospec) ospec="NTtiILR";
	if ((argc-optind)==2)
	{   start=strtoul(argv[optind],NULL,0);
	    ii   =strtoul(argv[optind+1],NULL,0);
	}
	traceShow(ospec,do_heading,start,ii);
    }
    else if (strncmp(cmd,"info",4) == 0) 
    {
	traceInit();
	traceInfo();
    }
    else if (strcmp(cmd,"unlock") == 0) 
    {   traceInit();
	trace_unlock();
    }
    else if (strcmp(cmd,"tids") == 0) 
    {   traceInit();
#       define UNDERLINE "----------------------------------------------"
	printf("%*s %*s %*s %*s %*s\n"
	       , 3, "TID"
	       , (int)sizeof(traceNamLvls_p[0].name), "NAME"
	       , 18, "maskM"
	       , 18, "maskS"
	       , 18, "maskT"
	       );
	printf("%.*s %.*s %.*s %.*s %.*s\n"
	       , 3, UNDERLINE
	       , (int)sizeof(traceNamLvls_p[0].name), UNDERLINE
	       , 18, UNDERLINE
	       , 18, UNDERLINE
	       , 18, UNDERLINE
	       );
	for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
	{
	    if (traceNamLvls_p[ii].name[0] != '\0')
	    {   printf("%3d %*s 0x%016" LX " 0x%016" LX " 0x%016" LX "\n"
		       , ii, (int)sizeof(traceNamLvls_p->name)
		       , traceNamLvls_p[ii].name
		       , traceNamLvls_p[ii].M
		       , traceNamLvls_p[ii].S
		       , traceNamLvls_p[ii].T
		       );
	    }
	}
    }
    else if (strcmp(cmd,"TRACE") == 0)
    {   
	switch (argc - optind)
	{
	case 0: printf("\"trace\" cmd rquires at least lvl and fmt arguments."); break;
	case 1: printf("\"trace\" cmd rquires at least lvl and fmt arguments."); break;
	case 2: TRACE( strtoull(argv[optind+0],NULL,0)
		      ,         argv[optind+1] );
	    break;
	case 3: TRACE( strtoull(argv[optind+0],NULL,0)
		      ,         argv[optind+1]
		      ,strtoull(argv[optind+2],NULL,0) );
	    break;
	case 4: TRACE( strtoull(argv[optind+0],NULL,0)
		      ,         argv[optind+1]
		      ,strtoull(argv[optind+2],NULL,0)
		      ,strtoull(argv[optind+3],NULL,0) );
	    break;
	case 5: TRACE( strtoull(argv[optind+0],NULL,0)
		      ,         argv[optind+1]
		      ,strtoull(argv[optind+2],NULL,0)
		      ,strtoull(argv[optind+3],NULL,0)
		      ,strtoull(argv[optind+4],NULL,0) );
	    break;
	case 6: TRACE( strtoull(argv[optind+0],NULL,0)
		      ,         argv[optind+1]
		      ,strtoull(argv[optind+2],NULL,0)
		      ,strtoull(argv[optind+3],NULL,0)
		      ,strtoull(argv[optind+4],NULL,0)
		      ,strtoull(argv[optind+5],NULL,0) );
	    break;
	}
    }
    else
    {   int sts=0;
	/*printf("argc - optind = %d\n", argc - optind );*/
	switch (argc - optind)
	{
	case 0: sts=TRACE_CNTL( cmd ); break;
	case 1: sts=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0) ); break;
	case 2: sts=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0)
			       , strtoull(argv[optind+1],NULL,0) ); break;
	case 3: sts=TRACE_CNTL( cmd, strtoull(argv[optind],NULL,0)
			       , strtoull(argv[optind+1],NULL,0)
			       , strtoull(argv[optind+2],NULL,0) ); break;
	}
	if (sts < 0)
	{   printf("invalid command: %s\n", cmd );
	    printf( USAGE );
	}
    }
    return (0);
}   /* main */
