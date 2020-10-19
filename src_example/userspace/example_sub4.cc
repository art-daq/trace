 // This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: example_sub4.cc,v $
 // rev="$Revision: 1385 $$Date: 2020-09-25 13:58:09 -0500 (Fri, 25 Sep 2020) $";

#include "TRACE/trace.h"

void example_sub4( void )
{   TRACE( TLVL_INFO, "hello from example_sub4()" );
#define TRACE_NAME "example_sub4"
    TRACE( TLVL_INFO, "hello again from example_sub4()" );
}
