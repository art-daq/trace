/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just_user.cc,v $
// rev="$Revision$$Date$";
*/


#include <string>

# if defined(__GXX_WEAK__) || ( defined(__cplusplus) && (__cplusplus >= 199711L) ) || ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )
/* c++98 c99 c++0x c11 c++11 */
#  define TRACE_LOG_FUNCTION(tvp,tid,lvl,...)          printf( __VA_ARGS__ );printf("\n")
# else
/* c89 */
#  define TRACE_LOG_FUNCTION(tvp,tid,lvl,msgargs... )  printf( msgargs );    printf("\n")
# endif   /* __GXX_WEAK__... */
#include "trace.h"		/* TRACE */

int
//main(  int	argc
//     , char	*argv[] )
main ()
{
	std::string stdstr_("hello my name is");

    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,3 );

	TRACE_( 1, << stdstr_ << " ron" );
	TRACE_( 1, << "ron is my name." );
    return (0);
}   /* main */
