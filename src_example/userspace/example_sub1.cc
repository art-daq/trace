 // This file (example_obj.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: example_sub1.cc,v $
<<<<<<< HEAD
 // rev="$Revision$$Date$";
=======
 // rev="$Revision: 1.1 $$Date: 2014/03/12 22:49:10 $";
>>>>>>> b06df97e1064bf6d8b0e7c0cfd42e2343a2521a5

#define TRACE_NAME "example_sub1"
#include "trace.h"

void example_sub2( void );

void example_sub1( void )
{   TRACE( 0, "hello from example_sub1() before calling example_sub2()" );
    example_sub2();
    TRACE( 0, "hello from example_sub1() after  calling example_sub2()" );
}
