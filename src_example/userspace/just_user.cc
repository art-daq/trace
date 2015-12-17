/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just_user.cc,v $
// rev="$Revision: 449 $$Date: 2015-12-15 13:30:38 -0600 (Tue, 15 Dec 2015) $";
*/


#include <stdarg.h>		/* va_list */
#include <string>
#include <sstream>
#include <ios>
#include <iostream>

#if defined(__GXX_WEAK__) || ( defined(__cplusplus) && (__cplusplus >= 199711L) ) || ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )
/* c++98 c99 c++0x c11 c++11 */
# define TRACE_LOG_FUNCTION(tvp,tid,lvl,...)          my_log( __VA_ARGS__ )
#else
/* c89 */
# define TRACE_LOG_FUNCTION(tvp,tid,lvl,msgargs... )  my_log( msgargs )
#endif   /* __GXX_WEAK__... */

#include "trace.h"		/* TRACE */


#define SUPPRESS_NOT_USED_WARN __attribute__ ((unused))

SUPPRESS_NOT_USED_WARN
void my_log(const char   *msg,...)
{ va_list ap;
  va_start(ap,msg);
  vprintf(msg,ap);printf("\n");
  va_end(ap);
}
SUPPRESS_NOT_USED_WARN
void my_log(std::string  msg,...)
{ va_list ap;
  va_start(ap,msg);
  vprintf(msg.c_str(),ap);printf("\n");
  va_end(ap);
}


int
//main(  int	argc
//     , char	*argv[] )
main ()
{
	std::string stdstr("hello my name is");
	std::ostringstream ostr;
	char const *filename="some/path";
	std::string fname("some/path2");

	std::cout << "hello\n";
	//TRACE(1,std::cout << "hello"); // THIS prints hello to screen and an address -- not useful

    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,3 );

	TRACE_( 1, stdstr << " ron" );
	TRACE( 1, "ron is my name." );

	// now with args
	//TRACE_( 1, stdstr << " ron %d", 1 );
	TRACE( 1, "ron is my name." );
    TRACE_( 1, "hello - nice for strings: file="<<filename<<" %d %.1f %d",1,1.6,2 );
    TRACE( 1, "hello - nice for strings: file="+fname+" %d %.1f %d",1,1.6,2 );
    TRACE_( 1, "hello - hopefully no compile warnings "<<1<<" "<<1.6<<" "<<std::hex<<15 );

	std::cout<<"\n";

	ostr << "<< stdstr (just to confuse %d" << stdstr;
	std::cout << ostr << "\n";

	std::cout<<"\n";

	//TRACE_( 1, ostr, 1 ); // an address  (also the 1 param is extra)
	TRACE( 1, ostr.str(), 1 );

	TRACE_( 1, "print a string: %s %s %s %s <this is after the string>"+stdstr,0LL,1LL,2LL,3LL );
	TRACE_( 1, "print a string: %s %s %s %s <this is after the string>"+stdstr );
	TRACE_( 1, "print a string: %s %s %s %s <this is after the string>"+stdstr );
	TRACE( 1, "print a string: %s %s %s %s <this is after the string>"+stdstr );

	std::cout<<"\n";

	//TRACE( 1, ostr <<"I would like to see what is it, not it's address", 1 ); // THIS prints to the screen and prints an address -- not useful
	//TRACE_( 1, (ostr<<"xx").str(), 2 ); these shenanigans don't compile

    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,2 );
    TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,3 );

	TRACE( 1,
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		   , 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 );
	TRACE( 1,
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		   , 41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75 );
    return (0);
}   /* main */
