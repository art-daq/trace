// This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: example_sub3.cc,v $
// rev="$Revision: 808 $$Date: 2018-02-07 23:44:55 -0600 (Wed, 07 Feb 2018) $";

#include "trace.h"
#define TRACE_NAME strcpy(buffer, "example_sub3")
static char buffer[15];

void example_sub4(void);

void example_sub3(void) {
  int mode = TRACE_CNTL("mode");
  TRACE(2, "hello from example_sub3 mode=%d before calling sub4", mode);
  example_sub4();
  TLOG(2, TRACE_NAME) << "hello from example_sub3 after  calling sub4";
}
