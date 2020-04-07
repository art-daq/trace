 // This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: example_sub3.cc,v $
 // rev="$Revision: 1294 $$Date: 2020-04-03 00:01:01 -0500 (Fri, 03 Apr 2020) $";

#include "TRACE/trace.h"
#include <libgen.h>				// basename
#define TRACE_NAME basename((char*)__FILE__)

void example_sub4( void );

void example_sub3( void )
{   int mode=(int)TRACE_CNTL("mode");
	TRACE( 2, "hello from example_sub3 mode=%d before calling sub4",mode );
    example_sub4();
    TLOG(2,TRACE_NAME) << "hello from example_sub3 after  calling sub4";
}
