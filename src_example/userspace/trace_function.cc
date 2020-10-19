 // This file (trace_function.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 17, 2020. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1405 $$Date: 2020-10-18 16:55:04 -0500 (Sun, 18 Oct 2020) $";
/*
  This is just an alternate way of prepending the function to the message.
  Or an example of prepending something else to the message.
  Now can be used to compare auto prepend (run once with and then again without auto-prepend)
 */
#define USAGE  "\
Usage: %s [-l<loops>] [-m<mask>] [-r<repeat>]\n\
", basename(argv[0])

#include <libgen.h>		/* basename */
#include <unistd.h>		/* write, getopt */

#include "TRACE/trace.h"
#define TLOGF(...) TLOG(__VA_ARGS__) << __FUNCTION__ << ": "
#define TLOGPF(...) TLOG(__VA_ARGS__) << __PRETTY_FUNCTION__ << ": "

static uint64_t gettimeofday_us()
{	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (uint64_t)(tv.tv_sec*1000000+tv.tv_usec);
} // gettimeofday_us


namespace someNamespace {
	void subroutine(int loops, int mask);
} // someNamespace

void someNamespace::subroutine(int loops, int mask)
{
	uint64_t mark_us;
	uint32_t delta_us;

#	define END_FMT  "%d lps in %10u us, %5.3f us/TLOG, %7.3f Mtlogs/s",\
		loops, delta_us, (double)delta_us/loops, (double)loops/delta_us

	if (mask & 0x1) {
		mark_us = gettimeofday_us();
		for (int ii=0; ii<loops; ++ii)
			TLOGPF(TLVL_DEBUG) << "0x1 hello " << ii << " of " << loops;
		delta_us = (uint32_t)(gettimeofday_us() - mark_us);
		TRACE(TLVL_INFO,"0x1 TLOGPF: " END_FMT );
	}

	if (mask & 0x2) {
		mark_us = gettimeofday_us();
		for (int ii=0; ii<loops; ++ii)
			TLOGF(TLVL_DEBUG)  << "0x2 hello " << ii << " of " << loops;
		delta_us = (uint32_t)(gettimeofday_us() - mark_us);
		TRACE(TLVL_INFO,"0x2 TLOGF:  " END_FMT );
	}

	if (mask & 0x4) {
		mark_us = gettimeofday_us();
		for (int ii=0; ii<loops; ++ii)
			TLOG(TLVL_DEBUG)   << "0x4 hello " << ii << " of " << loops;
		delta_us = (uint32_t)(gettimeofday_us() - mark_us);
		TRACE(TLVL_INFO,"0x4 TLOG:   " END_FMT );
	}

	if (mask & 0x8) {
		mark_us = gettimeofday_us();
		for (int ii=0; ii<loops; ++ii)
			TRACE(TLVL_DEBUG, "0x8 hello %d of %d", ii, loops);
		delta_us = (uint32_t)(gettimeofday_us() - mark_us);
		TRACE(TLVL_INFO,"0x8 TRACE:  " END_FMT );
	}
} // subroutine

int main(int argc, char *argv[])
{
	int     opt_loops=1;
	int     opt_repeat=1;
	int     opt_mask=0xf;
	int     opt;            /* for how I use getopt */
    while ((opt=getopt(argc,argv,"?l:m:r:")) != -1){
		switch (opt){
		/* '?' is also what you get w/ "invalid option -- -" */
		case '?': printf( USAGE ); exit (0); break;
		case 'l': opt_loops = (int)strtoul( optarg, NULL, 0 ); break;
		case 'm': opt_mask  = (int)strtoul( optarg, NULL, 0 ); break;
		case 'r': opt_repeat= (int)strtoul( optarg, NULL, 0 ); break;
		}
	}
	TLOG(TLVL_INFO) << "first TLOG does traceInit";
	for (int rr=0; rr<opt_repeat; ++rr)
		someNamespace::subroutine(opt_loops, opt_mask);
	return (0);
}   // main
