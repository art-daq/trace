 // This file (TLOG_ENTEX.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar  2, 2024. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.35 $$Date: 2023/10/14 17:08:08 $";

#include "TRACE/trace.h"

#if __cplusplus < 201703L
# define TLOG_ENTEX(...) std::cout << ""
# define TRACE_EXIT if(0)
#endif

int sub1(int x)
{
	int retval=x+1;
	TLOG_DEBUG() << "x="<<x;
	TRACE_EXIT { TLOG_DEBUG(1)<<"returning retval="<<retval; };
	return retval;
}

int main( /*int argc, char *argv[]*/ )
{
#if __cplusplus < 201703L
	TLOG() << "Must compile with C++17 or higher"; exit(1);
#endif
	TRACE_CNTL("lvlsetS",(1<<TLVL_DEBUG)|(1<<(TLVL_DEBUG+1)));
	int retval=0;
	TLOG_ENTEX(0);
	TLOG_DEBUG() << "sub1(1)="<<sub1(1);
	return retval;
}   // main
