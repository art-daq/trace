/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    rev="$Revision: 1.45 $$Date: 2014-02-27 20:09:46 $";
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

#define NUMTHREADS 4

#define USAGE "\
%s <cmd> [command opt/args]\n\
commands:\n\
 info, tids, reset, show\n\
 mode <mode>\n\
 modeset <mode>\n\
 modeclr <mode>\n\
 lvlmskM <msk>    mask for Memory buffer\n\
 lvlmskS <msk>    mask for stdout\n\
 lvlmskM <msk>    mask for trigger\n\
 trig <modeMsk> <lvlmskM> <postEntries>\n\
tests:  (use %s show after test)\n\
 test1          a single TRACE\n\
 test           various\n\
 test-ro        test first page of mmap read-only (for kernel module)\n\
 test-compare   compare TRACE fmt+args vs. format+args converted (via sprintf)\n\
 test-threads   threading\n\
", basename(argv[0]), basename(argv[0])


uint64_t get_us_timeofday()
{   struct timeval tv;
    gettimeofday( &tv, NULL );
    return (uint64_t)tv.tv_sec*1000000+tv.tv_usec;
}

void* thread_func(void *arg)
{
    long loops=(long)arg;
    char path[PATH_MAX];
# if 1
    snprintf(path,PATH_MAX,"%s/.trace_buffer_%lu",getenv("HOME"),syscall(SYS_gettid) );
    TRACE_CNTL( "file", path );
# else
    snprintf(path,PATH_MAX,"T%ld",syscall(SYS_gettid) );
    TRACE_CNTL( "name", path );
# endif
    while(loops-- > 0)
    {   TRACE( 0, "loops=%ld", loops );
	TRACE( 0, "loops=%ld", --loops );
	TRACE( 0, "loops=%ld", --loops );
	TRACE( 0, "loops=%ld", --loops );
    }
    pthread_exit(NULL);
}

void traceShow()
{
    uint32_t rdIdx;
    uint32_t max;
    unsigned printed=0;
    struct traceEntryHdr_s* myEnt_p;
    char                  * msg_p;
    unsigned long         * params_p;

    traceInit();
    rdIdx=traceControl_p->wrIdxCnt;
    max=((rdIdx<=traceControl_p->num_entries)
	 ?rdIdx:traceControl_p->num_entries);


#   if defined(__i386__)
#   endif

    printf("   idx           us_tod        tsc TID lv   tid r msg\n");
    printf("------ ---------------- ---------- --- -- ----- - -------------------");
    for (printed=0; printed<max; ++printed)
    {   rdIdx = IDXCNT_ADD( rdIdx, -1 );
	myEnt_p = idxCnt2entPtr( rdIdx );
	msg_p    = (char*)(myEnt_p+1);
	params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);

	printf("%6u %10ld%06ld %10u %3u %2d %5d "
	       , printed, myEnt_p->time.tv_sec, myEnt_p->time.tv_usec
	       , (unsigned)myEnt_p->tsc
	       , myEnt_p->TID, myEnt_p->lvl, myEnt_p->tid );
	if (myEnt_p->get_idxCnt_retries) printf( "%u ", myEnt_p->get_idxCnt_retries );
	else                             printf( ". " );

	/* MUST change all %s possibilities to %p */
	/* MUST change %* beyond num_params to %% */
	msg_p[traceControl_p->siz_msg - 1] = '\0';

	/*typedef unsigned long parm_array_t[1];*/
	/*va_start( ap, params_p[-1] );*/
	{   /* Ref. http://andrewl.dreamhosters.com/blog/variadic_functions_in_amd64_linux/index.html
	     */
	    va_list ap=TRACE_VA_LIST_INIT;
	    vprintf( msg_p, ap );
	}

	    printf("\n");
    }
}   /*traceShow*/


int main(  int	argc
	 , char	*argv[] )
{
	const char *cmd;
extern  char       *optarg;        /* for getopt */
extern  int        optind;         /* for getopt */
        int        opt;            /* for how I use getopt */

    while ((opt=getopt(argc,argv,"?hn:f:")) != -1)
    {   switch (opt)
        { /* '?' is also what you get w/ "invalid option -- -" */
        case '?': case 'h':  printf( USAGE );  exit( 0 ); break;
	case 'n': setenv("TRACE_NAME",optarg,1); break;
	case 'f': setenv("TRACE_FILE",optarg,1); break;
        }
    }
    if (argc - optind != 1)
    {   printf( "Need cmd\n" );
        printf( USAGE ); exit( 0 );
    }
    cmd = argv[optind];


    if      (strcmp(cmd,"test1") == 0)
    {
	TRACE( 0, "hello" );
    }
    else if (strcmp(cmd,"test") == 0)
    {   unsigned ii;
	float    ff[10];
	pid_t	 tid;
	uint64_t desired, myIdx;

#      if   defined(__cplusplus)      &&      (__cplusplus >= 201103L)
	tid = (pid_t)syscall( SYS_gettid );
#      elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
	tid = (pid_t)syscall( SYS_gettid );
#      else
	tid = (pid_t)syscall( SYS_gettid );
#      endif
	if (tid == -1) perror("syscall");
	printf("tid=%ld\n", (long int)syscall(SYS_gettid) );

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
	printf("myIdx=0x%016lx\n", myIdx );
	for (ii=0; ii<6; ++ii)
	{   desired = IDXCNT_ADD(myIdx,1);
	    printf( "myIdx==>myIdx+1: 0x%016lx 0x%016lx\n",myIdx, desired );
	    myIdx = desired;
	}
	for (ii=0; ii<6; ++ii)
	{   desired = IDXCNT_ADD(myIdx,-1);
	    printf( "myIdx==>myIdx-1: 0x%016lx 0x%016lx\n",myIdx, desired );
	    myIdx = desired;
	}
	printf("myIdx=0x%016lx\n", myIdx );
	TRACE( 1, "hello %d", 1 );
	TRACE( 2, "hello %d %d", 1, 2 );
	TRACE( 3, "hello %d %d %d", 1,2,3 );
	TRACE( 3, "hello %d %d %d %d %d %d %d %d %d %d %d"
	      , 1,2,3,4,5,6,7,8,9,10, 11 );   /* extra param does not get saved in buffer */
	TRACE( 3, "hello %f %f %f %f %f %f %f %f %f %f"
	      , 1.0,2.0,3.0,4.0, ff[5],6.0,7.0,8.0,9.0,10.0 );
	TRACE( 4, "hello %d %d %f  %d %d %f  %d %d"
	      ,           1, 2,3.3,4, 5, 6.6, 7, 8 );

#       ifndef TEST_UNUSED_FUNCTION
	TRACE_CNTL( "trig", 3, (uint64_t)-1, 5 );
#       endif
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
# if defined(TEST_WRITE_PAST_END)
	*(((uint8_t*)traceControl_p)+traceControl_p->memlen) = 6;
# endif
    }
    else if (strcmp(cmd,"test-compare") == 0)
    {   unsigned ii;
	char     buffer[200];
	uint64_t mark;
	unsigned compares=1000; /* some big number */
	if (argc == 3) compares=strtoul(argv[2],NULL,0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start no snprintf in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "any msg" );
	} TRACE_CNTL("mode",2);TRACE(0,"end   no snprintf in mode 1 delta=%lu", get_us_timeofday()-mark );


	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start snprintf 1 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   snprintf( buffer, sizeof(buffer), "this is one small param: %u", 12345678 );
	    TRACE( 0, buffer );
	} TRACE_CNTL("mode",2);TRACE(0,"end   snprintf 1 arg in mode 1 delta=%lu", get_us_timeofday()-mark );

	if (--compares == 0) return (0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start snprintf 2 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   snprintf( buffer, sizeof(buffer), "this is 2 params: %u %u", 12345678, ii );
	    TRACE( 0, buffer );
	} TRACE_CNTL("mode",2);TRACE(0,"end   snprintf 2 arg in mode 1 delta=%lu", get_us_timeofday()-mark );

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
	} TRACE_CNTL("mode",2);TRACE(0,"end   snprintf 8 arg in mode 1 delta=%lu", get_us_timeofday()-mark );

	if (--compares == 0) return (0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start TRACE w/8 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "this is 8 params: %u %u %u %u %u %u %u %f"
		     , 12345678, ii, ii*2, ii+6
		     , 12345679, ii, ii-7, (float)ii*1.5
		     );
	} TRACE_CNTL("mode",2);TRACE(0,"end   TRACE w/8 arg in mode 1 delta=%lu", get_us_timeofday()-mark );

	if (--compares == 0) return (0);

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start (repeat) no snprintf in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "any msg" );
	} TRACE_CNTL("mode",2);TRACE(0,"end   (repeat) no snprintf in mode 1 delta=%lu", get_us_timeofday()-mark );
    }
#   ifdef DO_THREADS   /* requires linking with -lpthreads */
    else if (strcmp(cmd,"test-threads") == 0)
    {   unsigned ii;
	pthread_t threads[NUMTHREADS];
	long loops=10000;
	if (argc == 3) loops=strtoul(argv[2],NULL,0);
	loops -= loops%4;	/* assuming thread does 4 TRACEs per loop */
	TRACE( 0, "before pthread_create - loops=%lu (must be multiple of 4)", loops );
	for (ii=0; ii<NUMTHREADS; ii++)
	{   pthread_create(&threads[ii],NULL,thread_func,(void*)loops);
	}
	for (ii=0; ii<NUMTHREADS; ii++)
	{   pthread_join(threads[ii], NULL);
	}
	TRACE( 0, "after pthread_join" );
	sleep( 10 ); /* in lieu of at_exit close files */
    }
#   endif
    else if (strcmp(cmd,"show") == 0) 
    {
	traceShow();
    }
    else
    {   int sts=0;
	switch (argc)
	{
	case 2: sts=TRACE_CNTL( cmd ); break;
	case 3: sts=TRACE_CNTL( cmd, strtoull(argv[2],NULL,0) ); break;
	case 4: sts=TRACE_CNTL( cmd, strtoull(argv[2],NULL,0), strtoull(argv[3],NULL,0) ); break;
	case 5: sts=TRACE_CNTL( cmd, strtoull(argv[2],NULL,0), strtoull(argv[3],NULL,0)
			       , strtoull(argv[4],NULL,0) ); break;
	}
	if (sts < 0)
	{   printf("invalid command: %s\n", cmd );
	    printf( USAGE );
	}
    }
    return (0);
}   /* main */
