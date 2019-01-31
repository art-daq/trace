 // This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: example_sub4.cc,v $
 // rev="$Revision: 416 $$Date: 2015-10-13 11:48:10 -0500 (Tue, 13 Oct 2015) $";

#include "trace.h"

void example_sub4( void )
{   TRACE( 0, "hello from example_sub4()" );
#define TRACE_NAME "example_sub4"
    TRACE( 0, "hello again from example_sub4()" );
}
