/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    rev="$Revision: 1.27 $$Date: 2014/01/31 16:48:00 $";
    */
/*
gxx_standards.sh Trace_test.c
for std in $standards;do\
  for ll in 0 1 2 3;do echo $std $ll:;TRACE_LVL=$ll ./Trace_test-$std;done;\
done
   
*/
#include <stdio.h>		/* printf */
#include <libgen.h>		/* basename */
#include <sys/time.h>           /* gettimeofday, struct timeval */
#include <pthread.h>		/* pthread_self */
#include <sys/syscall.h>	/* syscall */
#ifndef   VA_PARSE_ARGS
# include "Trace_mmap4.h"
#else
# include "Trace_mmap2.h"
#endif

#define NUMTHREADS 4

#define USAGE "\
%s <cmd> [command opt/args]\n\
commands: info, reset, show, mode, lvl\n\
   tests: test, test-compare, test-threads, \n\
", basename(argv[0])


uint64_t
get_us_timeofday()
{   struct timeval tv;
    gettimeofday( &tv, NULL );
    return (uint64_t)tv.tv_sec*1000000+tv.tv_usec;
}

void* thread_func(void *arg)
{
    long loops=(long)arg;

    while(loops-- > 0)
    {   TRACE( 0, "loops=%lu", loops );
	TRACE( 0, "loops=%lu", --loops );
	TRACE( 0, "loops=%lu", --loops );
	TRACE( 0, "loops=%lu", --loops );
    }
    pthread_exit(NULL);
}


int
main(  int	argc
     , char	*argv[] )
{

#  if 0
    int         opt;
    while (1)
    {   int option_index = 0;
        static struct option long_options[] =
        {   /* name,   has_arg, int* flag, val_for_flag */
            {0,            0,      0,          0}
        };
        opt = getopt_long (argc, argv, "?hn:s:p",
                        long_options, &option_index);
        if (opt == -1) break;
        switch (opt)
        {
        case '?': case 'h':  printf( USAGE );  exit( 0 ); break;
        default:  printf ("?? getopt returned character code 0%o ??\n", opt);
        }
    }
    if (argc - optind != 1)
    {   printf( "Need cmd\n" );
        printf( USAGE ); exit( 0 );
    }
#  endif


    if (argc <= 1) { printf(USAGE); exit(0); }


    if      (strncmp(argv[1],"info",4) == 0) 
    {
	TRACE_CNTL( argv[1] );
    }
    else if (strcmp(argv[1],"reset") == 0) 
    {
	TRACE_CNTL( argv[1] );
    }
    else if (strcmp(argv[1],"lvl") == 0) 
    {
	if (argc <= 2) { printf(USAGE); exit(0); }
	TRACE_CNTL( argv[1], strtoul(argv[2],NULL,0) );
    }
    else if (strcmp(argv[1],"mode") == 0) 
    {
	if (argc <= 2) { printf(USAGE); exit(0); }
	TRACE_CNTL( argv[1], strtoul(argv[2],NULL,0) );
    }
    else if (strcmp(argv[1],"show") == 0) 
    {
	TRACE_CNTL( argv[1] );
    }
    else if (strcmp(argv[1],"test") == 0)
    {   unsigned ii;
	float    ff[10];
	pid_t	 tid;

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
	       , (void*)&((struct traceControl_s*)0)->trigIdxCount
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
	TRACE( 0, "hello" );
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
    else if (strcmp(argv[1],"test-compare") == 0)
    {   unsigned ii;
	char     buffer[200];
	uint64_t mark;

	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start no snprintf in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "any msg" );
	}
	TRACE_CNTL("mode",2);TRACE(0,"end   no snprintf in mode 1 delta=%lu", get_us_timeofday()-mark );


	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start snprintf 1 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   snprintf( buffer, sizeof(buffer)
		     , "this is one small param: %u", 12345678 );
	    TRACE( 0, buffer );
	}
	TRACE_CNTL("mode",2);TRACE(0,"end   snprintf 1 arg in mode 1 delta=%lu", get_us_timeofday()-mark );


	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start snprintf 2 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   snprintf( buffer, sizeof(buffer)
		     , "this is 2 params: %u %u", 12345678, ii );
	    TRACE( 0, buffer );
	}
	TRACE_CNTL("mode",2);TRACE(0,"end   snprintf 2 arg in mode 1 delta=%lu", get_us_timeofday()-mark );


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
	}
	TRACE_CNTL("mode",2);TRACE(0,"end   snprintf 8 arg in mode 1 delta=%lu", get_us_timeofday()-mark );


	TRACE_CNTL("reset");
	TRACE_CNTL("mode",2);TRACE(0,"start TRACE w/8 arg in mode 1");TRACE_CNTL("mode",1);
	mark = get_us_timeofday();
	for (ii=0; ii<1000000; ++ii)
	{   TRACE( 0, "this is 8 params: %u %u %u %u %u %u %u %f"
		     , 12345678, ii, ii*2, ii+6
		     , 12345679, ii, ii-7, (float)ii*1.5
		     );
	}
	TRACE_CNTL("mode",2);TRACE(0,"end   TRACE w/8 arg in mode 1 delta=%lu", get_us_timeofday()-mark );
    }
#   ifdef DO_THREADS
    else if (strcmp(argv[1],"test-threads") == 0)
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
    }
#   endif
    else
    {   printf("invalid command: %s\n", argv[1] );
	printf( USAGE );
    }
    return (0);
}   /* main */
