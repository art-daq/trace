 // This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: example_sub3.cc,v $
 // rev="$Revision: 1.1 $$Date: 2015-04-24 17:05:47 $";

#include "trace.h"
#define TRACE_NAME "example_sub3"

void example_sub4( void );

void example_sub3( void )
{   TRACE( 0, "hello from example_sub3 before calling sub4" );
    example_sub4();
    TRACE( 0, "hello from example_sub3 after  calling sub4" );
}
