 // This file (no_std.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jul  3, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: no_std.cc,v $
<<<<<<< HEAD
 // rev="$Revision$$Date$";
=======
 // rev="$Revision: 1.1 $$Date: 2014/07/04 02:36:08 $";
>>>>>>> b06df97e1064bf6d8b0e7c0cfd42e2343a2521a5

#define TRACE_LOG_FUNCTION(tvp,tid,lvl,...)
#include "trace.h"              /* TRACE */

int
<<<<<<< HEAD
//main(  int      argc
//     , char     *argv[] )
main()
=======
main(  int      argc
     , char     *argv[] )
>>>>>>> b06df97e1064bf6d8b0e7c0cfd42e2343a2521a5
{
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    return (0);
}  // main
