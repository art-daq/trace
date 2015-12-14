/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just.cc,v $
<<<<<<< HEAD
// rev="$Revision$$Date$";
=======
// rev="$Revision: 1.5 $$Date: 2014/04/28 16:31:10 $";
>>>>>>> b06df97e1064bf6d8b0e7c0cfd42e2343a2521a5
*/

#include <string>
#include "trace.h"		/* TRACE */

int
<<<<<<< HEAD
//main(  int	argc
//     , char	*argv[] )
main()
=======
main(  int	argc
     , char	*argv[] )
>>>>>>> b06df97e1064bf6d8b0e7c0cfd42e2343a2521a5
{
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    TRACE( 0, (std::string("hi %d")+" there").c_str(), 2 );
    return (0);
}   /* main */
