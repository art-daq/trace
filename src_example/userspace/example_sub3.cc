// This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: example_sub3.cc,v $
// rev="$Revision: 1784 $$Date: 2026-10-09 13:51:46 -0500 (Fri, 09 Oct 2026) $";

#include "TRACE/trace.h"
#include <libgen.h>                            // basename
#define TRACE_NAME basename((char *)__FILE__)  // NOLINT

void example_sub4(void);

void example_sub3(void)
{
	int mode= (int)TRACE_CNTL("mode");  // NOLINT
	TRACE(2, "hello from example_sub3 mode=%d before calling sub4", mode);
	example_sub4();
	TLOG(2, TRACE_NAME) << "hello from example_sub3 after  calling sub4";
}
