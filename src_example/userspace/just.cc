/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just.cc,v $
// rev="$Revision: 1056 $$Date: 2019-02-25 15:57:13 -0600 (Mon, 25 Feb 2019) $";
*/

#include <string>
#include "TRACE/trace.h"		/* TRACE */

void example_sub_( void );

int
//main(  int	argc
//     , char	*argv[] )
main()
{
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    TRACE( 0, (std::string("hi %d")+" there").c_str(), 2 );
    example_sub_();
    return (0);
}   /* main */
