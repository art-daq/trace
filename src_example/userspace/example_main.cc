 // This file (example_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: example_main.cc,v $
 // rev="$Revision: 1056 $$Date: 2019-02-25 15:57:13 -0600 (Mon, 25 Feb 2019) $";

#include <pthread.h>		/* pthread_self */
#include <sys/time.h>		/* gettimeofday */
#include <libgen.h>		/* basename (Darwin) */

#define TRACE_DEFINE
#include "TRACE/trace.h"

#define USAGE "\
  usage: %s\n\
", basename(argv[0])

void example_sub1( void );

#define NUMTHREADS 3
static int trace_thread_option=0;

void* thread_func(void *arg)
{
    long loops=(long)arg;
    char tmp[PATH_MAX];
    long tid;
    timeval mark;
    if      (trace_thread_option == 1)
    {   /* IF -std=c11 is NOT used, a seg fault usually occurs if default file does not exit */
		tid = (long)trace_gettid();
	snprintf( tmp, sizeof(tmp),"/tmp/trace_buffer_%ld",tid );
	TRACE_CNTL( "file", tmp );
    }
    else if (trace_thread_option == 2)
    {   tid = (long)trace_gettid();
	snprintf( tmp, sizeof(tmp), "T%ld", tid );
	printf( "setting name to %s\n",tmp );
	TRACE_CNTL( "name", tmp );
    }

    gettimeofday( &mark, NULL );
    while(loops-- > 0)
    {   TRACE( 0, "loops=%ld", loops );
	example_sub1();
    }
    TRACE( 1, "mark: %ld%06ld", mark.tv_sec, (long)mark.tv_usec );

    pthread_exit(NULL);
}


int
main(  int	argc
     , char	*argv[] )
{
extern  char        * optarg;        /* for getopt */
extern  int           optind;         /* for getopt */
        int           opt;            /* for how I use getopt */
	unsigned      ii;
	pthread_t   * threads;
	unsigned      num_threads=NUMTHREADS;
	unsigned long loops=4;

    while ((opt=getopt(argc,argv,"?hn:f:x:")) != -1)
    {   switch (opt)
        { /* '?' is also what you get w/ "invalid option -- -" */
        case '?': case 'h': printf(USAGE);exit(0);           break;
	case 'n': setenv("TRACE_NAME",optarg,1);             break;
	case 'f': setenv("TRACE_FILE",optarg,1);             break;
	case 'x': trace_thread_option=strtoul(optarg,NULL,0);break;
        }
    }

    if ((argc-optind)>=1)
    {   loops=strtoul(argv[1],NULL,0);
	printf("loops set to %lu\n", loops );
    }
    if ((argc-optind)==2)
    {   num_threads=strtoul(argv[2],NULL,0);
	printf("num_threads set to %u\n", num_threads );
    }
    threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
    TRACE( 0, "example_main calling example" );
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

    TRACE( 0, "back to example_main" );
    free( threads );
    return (0);
}   // main
