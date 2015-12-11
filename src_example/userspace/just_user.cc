/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just_user.cc,v $
// rev="$Revision$$Date$";
*/


#include <string>
#include <sstream>

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
	std::string stdstr("hello my name is");
	std::ostringstream ostr;
	ostr << "<< stdstr (just to confuse %d" << stdstr;

    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,3 );

	TRACE_( 1, stdstr << " ron" );
	TRACE_( 1, "ron is my name." );

	// now with args
	TRACE_( 1, stdstr << " ron %d", 1 );
	TRACE_( 1, "ron is my name." );
    TRACE_( 1, "hello - hopefully no compile warnings %d %.1f %d",1,1.6,2 );
    TRACE_( 1, "hello - hopefully no compile warnings "<<1<<" "<<1.6<<" "<<3 );
	TRACE_( 1, ostr, 1 ); // an address  (also the 1 param is extra
	TRACE_( 1, ostr.str(), 1 );
	TRACE_( 1, ostr.str()<<"xx", 1 );
	//TRACE_( 1, (ostr<<"xx").str(), 2 ); these shenanigans don't compile

    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,2 );
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,3 );

    return (0);
}   /* main */
