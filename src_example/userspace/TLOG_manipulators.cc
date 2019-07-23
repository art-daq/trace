 // This file (TLOG_manipulators.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jul 18, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: TLOG_manipulators.cc,v $
 // rev="$Revision: 1.1 $$Date: 2019/07/18 15:28:45 $";

#include <stdio.h>		// printf
//#define TRACE_STREAMER_DEBUG 1
#include "TRACE/trace.h"

int
main(  int	argc
     , char	*argv[] )
{
	for (unsigned ii=0; ii<2; ++ii)
		TLOG(TLVL_INFO) << "ii+10=" << ii+10 << " "
						<< std::fixed << std::setprecision(1) << 123.456;
	for (unsigned ii=0; ii<2; ++ii) {
		TLOG(TLVL_INFO) << "ii+10=" << ii+10 << std::hex << " " << ii+10 << " " << 789.012 << " " << std::setprecision(1) << 123.456;
		TLOG(TLVL_INFO) << "ii+10=" << ii+10 << std::hex << " " << ii+10 << " "
						<< 789.012 << " " << std::setprecision(2) << 123.456;
	}
	return (0);
}   // main
