 // This file (trace_function.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 17, 2020. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

#include "TRACE/trace.h"
#define TLOGF(...) TLOG(__VA_ARGS__) << __FUNCTION__ << ": "
#define TLOGPF(...) TLOG(__VA_ARGS__) << __PRETTY_FUNCTION__ << ": "

int main()
{
	TLOGF(2) << "hello";
	TLOGPF(2) << "hello";
	return (0);
}   // main
