// This file (no_std.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Jul  3, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: no_std.cc,v $
// rev="$Revision: 1225 $$Date: 2019-10-08 09:58:24 -0500 (Tue, 08 Oct 2019) $";

// comfirm there are no warning when TRACE_LOG_FUNCTION is, well, something we may not expect (e.g. blank)

#include <sys/time.h>			/* timeval */
void trace_ignore(timeval*, ...) {}
#define TRACE_LOG_FUNCTION trace_ignore /* no stdout or stderr (depending on TRACE_PRINT_FD */
#include "TRACE/trace.h"				/* TRACE */

int
//main(  int      argc
//     , char     *argv[] )
main()
{
	TRACE(0, "hello - hopefully no compile warnings %d %.1f %d", 1, 1.5, 2);
	return (0);
}  // main
