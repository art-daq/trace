 // This file (setting_lvlTime.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // May 14, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

// When this program is executed, both "traces" (TRACE and TLOG) with display the same time.
// and ltrace shows gettimeofday is called just once.

#include <stdio.h>		// printf
#include <TRACE/trace.h>		// TRACE, TLOG
#include <sys/time.h>			// gettimeofday, struct timeval

int main(  int argc, char *argv[] )
{
	struct timeval programStart, *tvp;

	// The execution time of these lines of code (especially if just memory tracing) would be within  afew microseconds.
	gettimeofday( &programStart, NULL);  // this can now be used to get latencies elsewhere in the program.
	TRACE( 1, "TRACE programStart usecs=%ld", (lclTime=programStart, lclTime.tv_usec) );
    // need extra shenanigans (tvp) as tv is actually an int array which can't be access directly w/o breaking strict-aliasing rules
	TLOG(1) << "TLOG  programStart usecs=" << (*(tvp=(timeval*)tv)=programStart, programStart.tv_usec);

	// NOTE: you can only get the time for TRACEs if both memory and slow path are enabled. Can't get time from TLOG

	return (0);
}   // main
