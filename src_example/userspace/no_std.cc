 // This file (no_std.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jul  3, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: no_std.cc,v $
 // rev="$Revision: 424 $$Date: 2015-10-19 13:08:36 -0500 (Mon, 19 Oct 2015) $";

#define TRACE_LOG_FUNCTION(tvp,tid,lvl,...)
#include "trace.h"              /* TRACE */

int
//main(  int      argc
//     , char     *argv[] )
main()
{
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    return (0);
}  // main
