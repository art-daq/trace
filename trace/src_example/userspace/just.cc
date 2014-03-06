/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just.cc,v $
// rev="$Revision: 1.3 $$Date: 2014-03-06 15:45:11 $";
*/

#include "trace.h"		/* TRACE */

int
main(  int	argc
     , char	*argv[] )
{
    TRACE( 0, "hello - hopefully no warnings %d %.1f %d",1,1.5,2 );
    return (0);
}   /* main */
