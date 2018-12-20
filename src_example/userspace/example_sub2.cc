// This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: example_sub2.cc,v $
// rev="$Revision: 808 $$Date: 2018-02-07 23:44:55 -0600 (Wed, 07 Feb 2018) $";

#define TRACE_DECLARE
#include "trace.h"

void example_sub3(void);

void example_sub2(void) {
  TRACE(0, "hello from example_sub2 before calling sub3");
  example_sub3();
  TLOG(2) << "hello from example_sub2 after  calling sub3";
}
