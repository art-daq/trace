 // This file (example_sub_.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Aug 11, 2018. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.30 $$Date: 2016/03/01 14:27:27 $";

// A module that just has a single TRACE
// Simple to compile and look at assemble

#include "trace.h"

void example_sub_( void )
{
    TRACE( 0, "hello from example_sub_" ); // module with single simply TRACE
}
