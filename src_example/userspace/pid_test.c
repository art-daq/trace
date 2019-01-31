/*  This file (pid_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Jan 30, 2019. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1.33 $$Date: 2019/01/13 02:34:40 $";
	*/
/* compile:
   cd src_example/userspace
   gcc -g -Wall -I$TRACE_INC -std=c11 -o $TRACE_BIN/pid_test pid_test.c -lpthread 2>&1 | head -22
 */
#include <stdio.h>		// printf
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>    // pthread_create,join
#include "trace.h"		// TRACE

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
	pid_t     mypid=getpid();
	pthread_t thread_id;

	TRACE( 1,"main - mypid: %ld", (long)mypid );
	pthread_create(&thread_id,NULL,thread_func,(void*)(long)mypid );
	do_fork( sub1, mypid );
	pthread_join(thread_id, NULL);
	return (0);
}   /* main */
