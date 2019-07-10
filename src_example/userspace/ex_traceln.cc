 // This file (ex_traceln.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jun  7, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

#include <stdio.h>		// printf
#include "ex_traceln.h"
#include "TRACE/traceln.h"
//#define TRACE_NAME &std::string(__FILE__).substr(std::string(__FILE__).rfind('/')+1)[0]
//#define __FIL__ "/x/srcs/this/that/then"
#define __FIL__ "ex_traceln.cc"
//#define TRACE_NAME (strstr(&__FIL__[0], "/srcs/") ? strstr(&__FIL__[0], "/srcs/") + 6 : __FIL__)
#define TRACE_NAME &std::string(__FIL__).substr(std::string(__FIL__).rfind('/',std::string(__FIL__).rfind('/')-1)+1)[0]

#define Q(X) #X
#define QUOTE(X) Q(X)
#define VAL(X) QUOTE(X) << " = " << X


int
main(  int	argc
     , char	*argv[] )
{
	int delayBetweenDataRequests = 1000; // some example variable
#   define FFF "/x/srcs/this/that/then"
	TLOG(1) << "start " << std::string(FFF).substr(std::string(FFF).rfind('/')+1);
	TLOG(1) << "start " << std::string(FFF).substr(std::string(FFF).rfind('/',std::string(FFF).rfind('/')-1)+1);
	ExTraceLn().trc();
	TLOG(1) << VAL(delayBetweenDataRequests);
	return (0);
}   // main
