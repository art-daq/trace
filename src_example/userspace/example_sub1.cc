// This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: example_sub1.cc,v $
// rev="$Revision: 1702 $$Date: 2025-01-28 12:48:14 -0600 (Tue, 28 Jan 2025) $";

#define TRACE_NAME strcpy(buffer, "example_sub1")
static char buffer[15];
#include "TRACE/trace.h"

void example_sub2(void);

void example_sub1(void)
{
	TRACE(2, "hello from example_sub1() before calling example_sub2()");
	example_sub2();
	TLOG(2) << "hello from example_sub1() after  calling example_sub2()";
}
