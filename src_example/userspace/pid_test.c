/*  This file (pid_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Jan 30, 2019. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1398 $$Date: 2020-10-01 22:44:25 -0500 (Thu, 01 Oct 2020) $";
	*/
/*
 This is a test program. It checks that the trace Process ID and Thread ID
 saved in the memory buffer are correct. This is done by tracing the PID
 and TID as parameters in the trace message and then checking to make sure
 they match the tid and pid printed in the trace show output.
# compile (if not already done):
   cd src_example/userspace # or appropriate
   # you may need to set TRACE_{INC,BIN} 
   gcc -g -Wall -I$TRACE_INC -std=c11 -o $TRACE_BIN/pid_test pid_test.c -lpthread 2>&1 | head -22
# run:
   treset;strace -f -e getpid,gettid pid_test
# check:
   tshow | awk '/mypid/{tpid=$3;mypid=gensub(".*mypid: ([0-9]*).*","\\1",1);
 if(tpid!=mypid){print"No good at ",$1;nogood=1;exit}}
END{if(!nogood)print"ALL OK"}'

*/
#define USAGE "\
  usage: %s [options]\n\
example: %s\n\
options:\n\
", basename(argv[0]), basename(argv[0])

#include <stdio.h>              // printf
#include <stdlib.h>             // exit
#include <pthread.h>		// pthread_self,pthread_create,join
#include <unistd.h>		// getopt
#include <getopt.h>
#include <sys/types.h>          // pid_t
#include <sys/wait.h>           // waitpid
#include <libgen.h>             // basename
#include "TRACE/trace.h"		// TRACE

typedef void (*fp_t)(pid_t mypid);

void do_fork( fp_t fp, pid_t mypid )
{
	pid_t cpid, w;
	int wstatus;

	cpid = fork();
	if (cpid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (cpid == 0) {            /* Code executed by child */
		pid_t cpid=getpid();
		TRACE( 1, "Child mypid: %ld begin", (long)cpid );
		if (fp)
			fp(cpid);
		TRACE( 1, "Child mypid: %ld exiting", (long)cpid );
		_exit( 0 );

	} else {                    /* Code executed by parent */
		do {
			w = waitpid(cpid, &wstatus, WUNTRACED);
			if (w == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}

			if (WIFEXITED(wstatus)) {
				TRACE( 1, "mypid: %ld cpid: %ld exited, status=%d", (long)mypid, (long)cpid, WEXITSTATUS(wstatus));
			} else if (WIFSIGNALED(wstatus)) {
				TRACE( 1, "mypid: %ld cpid: %ld killed by signal %d", (long)mypid, (long)cpid, WTERMSIG(wstatus));
			} else if (WIFSTOPPED(wstatus)) {
				TRACE( 1, "mypid: %ld cpid: %ld stopped by signal %d", (long)mypid, (long)cpid, WSTOPSIG(wstatus));
			} else {
				TRACE( 1, "mypid: %ld cpid: %ld unknown wstatus(0x%x) - continued???", (long)mypid, (long)cpid, wstatus );
			}
		} while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
		TRACE( 1, "mypid: %ld cpid: %ld has exited", (long)mypid, (long)cpid );
	}
}

void sub1( pid_t ppid )
{
	do_fork( NULL, ppid );
}

void* thread_func(void *arg)
{
	pid_t mypid=(pid_t)(long)arg;
	TRACE( 1, "thread_func: mypid: %ld begin", (long)mypid );
	do_fork( sub1, mypid );
	pthread_exit(NULL);
}

int main(  int	argc
         , char	*argv[] )
{
extern  char        * optarg;        // for getopt
        int           opt;           // for how I use getopt
		pid_t         mypid=getpid();
		pthread_t     thread_id;
		unsigned long opt_loops=1;
		unsigned      ii;

    while ((opt=getopt(argc,argv,"?hl:")) != -1)
    {   switch (opt)
        { // '?' is also what you get w/ "invalid option -- -"
        case '?': case 'h': printf(USAGE);exit(0);    break;
		case 'l': opt_loops=strtoul(optarg,NULL,0);       break;
        }
    }
	TRACE( 1,"main - mypid: %ld", (long)mypid );
	for (ii=0; ii<opt_loops; ++ii) {
		pthread_create(&thread_id,NULL,thread_func,(void*)(long)mypid );
		do_fork( sub1, mypid );
		pthread_join(thread_id, NULL);
	}
	return (0);
}   /* main */
