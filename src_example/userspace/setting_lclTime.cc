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
	struct timeval programStart, programEnd;

	// The execution time of these lines of code (especially if just memory tracing) would be within  afew microseconds.
	gettimeofday( &programStart, NULL);  // this can now be used to get latencies elsewhere in the program.
	TRACE( 1, "TRACE programStart usecs=%ld", (lclTime=programStart, lclTime.tv_usec) );

#  if 1
	// need extra shenanigans (tvp) to avoid: warning: operation on '_tlog_.main(int, char**)::_T_::tv' may be undefined [-Wsequence-point]
	struct timeval *tvp;
	TLOG(1) << "TLOG  programStart usecs=" << (*(tvp=&_tlog_.tv)=programStart, programStart.tv_usec);
#  else
	TLOG(1) << "TLOG  programStart usecs=" << (_tlog_.tv=programStart, programStart.tv_usec);
#  endif

	// NOTE: you can only get the time for TRACEs _IFF_both_memory_and_slow_path_are_enabled. Can't get time from TLOG
	TRACE( 1, "TRACE programEnd usecs=%ld - if zero, both mem/slow are not enabled", (programEnd=lclTime, lclTime.tv_sec) );

	return (0);
}   // main
