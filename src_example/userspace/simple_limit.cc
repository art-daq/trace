 // This file (simple_limit.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 31, 2020. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1304 $$Date: 2020-04-13 01:26:17 -0500 (Mon, 13 Apr 2020) $";
/*
$ TRACE_LIMIT_MS=4,50,500 simple_limit
03-31 10:12:30.965233   TRACE nfo The following burst of messages may be limited depending on the value of TRACE_LIMIT_MS
03-31 10:12:30.965314   TRACE nfo TLOG 0may be limited.
03-31 10:12:30.965319   TRACE nfo TLOG 1may be limited.
03-31 10:12:30.965322   TRACE nfo TRACE 1 may be limited.
03-31 10:12:30.965325   TRACE nfo TLOG 2may be limited.
03-31 10:12:30.965328   TRACE nfo [RATE LIMIT] TLOG 3may be limited.
03-31 10:12:30.965330   TRACE nfo TRACE 3 may be limited.
03-31 10:12:30.965333   TRACE nfo TRACE 5 may be limited.
03-31 10:12:30.965347   TRACE nfo [RATE LIMIT] TRACE 7 may be limited.
03-31 10:12:31.965456   TRACE nfo sleep(1)=0
03-31 10:12:32.965587   TRACE nfo [RESUMING dropped: 5] TLOG 9may be limited.
03-31 10:12:32.965592   TRACE nfo [RESUMING dropped: 0] TRACE 9 may be limited.
 */
#include "TRACE/trace.h"

int main()
{
	TLOG(TLVL_INFO) << "The following burst of messages may be limited depending on the value of TRACE_LIMIT_MS";
	for (unsigned uu=0; uu<10; ++uu) {
		if(uu==9)
			TRACE(TLVL_INFO,"sleep(1)=%d",sleep(1));
		TLOG(TLVL_INFO) << "TLOG "<<uu<< "may be limited.";
		if (uu&1)
			TRACE(TLVL_INFO,"TRACE %u may be limited.",uu);
	}
	return (0);
}   // main
