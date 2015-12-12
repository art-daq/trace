/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just_user.cc,v $
// rev="$Revision$$Date$";
*/


#include <string>
#include <sstream>
#include <ios>

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
	char const *filename="some/path";
	std::string fname("some/path2");
	ostr << "<< stdstr (just to confuse %d" << stdstr;

    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,3 );

	TRACE_( 1, stdstr << " ron" );
	TRACE_( 1, "ron is my name." );

	// now with args
	TRACE_( 1, stdstr << " ron %d", 1 );
	TRACE_( 1, "ron is my name." );
    TRACE_( 1, "hello - nice for strings: file="<<filename<<" %d %.1f %d",1,1.6,2 );
    TRACE_( 1, "hello - nice for strings: file="+fname+" %d %.1f %d",1,1.6,2 );
    TRACE_( 1, "hello - hopefully no compile warnings "<<1<<" "<<1.6<<" "<<std::hex<<15 );
	//TRACE_( 1, ostr, 1 ); // an address  (also the 1 param is extra)
	TRACE_( 1, ostr.str(), 1 );
	TRACE_( 1, ostr.str()<<"xx", 1 );
	//TRACE_( 1, (ostr<<"xx").str(), 2 ); these shenanigans don't compile

    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,2 );
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,3 );

	TRACE_( 1,
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		   , 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 );
	TRACE_( 1,
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		   , 41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75 );
    return (0);
}   /* main */
