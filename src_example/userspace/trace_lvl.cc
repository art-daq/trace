 // This file (trace_lvl.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Sep 17, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1459 $$Date: 2021-01-02 09:23:56 -0600 (Sat, 02 Jan 2021) $";


// test TRACE(lvl++,...) and TLOG( 

// g++ -Wall -g -o trace_lvl{,.cc} -I$TRACE_INC && ./trace_lvl

#include <stdio.h>		// printf
#define TRACE_USE_STATIC_STREAMER 1 // the use of TraceStreamer in the file assumes this and precludes non-static
#include "TRACE/trace.h"

void sub1(int lvl)
{
	TRACE( lvl++&1?lvl:2, "trace lvl=%d", lvl-1 );
    if (lvl > 110)
		return;
	sub1(lvl);
}

void sub2(int lvl)
{
	TLOG(lvl++&1?lvl:2) << "log lvl=" << lvl-1;
    if (lvl > 110)
		return;
	sub2(lvl);
}

int
main(/*  int	argc
		 , char	*argv[]*/ )
{

	// The following is just shoved in here as a place to help develop the TRACE_STREAMER macro
#define _lvl       TLVL_INFO
#define nam_or_fmt 0
#define fmt_or_nam ""
	static TRACE_THREAD_LOCAL TraceStreamer steamer;
	for (TSTREAMER_T_ _tlog_((tlvle_t)(_lvl), TRACE_GET_STATIC());
		 _tlog_.once-- && TRACE_INIT_CHECK(TRACE_NAME)
			 && (_tlog_.TLOG2(nam_or_fmt,fmt_or_nam), ((*_tlog_.tidp != -1) || ((*_tlog_.tidp=(_tlog_.nn[0]?trace_name2TID(_tlog_.nn):traceTID))!=-1)))
			 && trace_do_streamer(&_tlog_);
		 steamer.str())
		steamer.init(*_tlog_.tidp, _tlog_.lvl, _tlog_.flgs, __FILE__, __LINE__, __FUNCTION__, &_tlog_.tv, _tlog_.ins, &TRACE_LOG_FUNCTION) << "hello";


	int lvlx=0x107;  // testing conversion between int and uint8_t
	TRACEN("test",lvlx,"test");
	TRACEN_("test",lvlx,"test"<<"ing");
	sub1(2);
	usleep( 1000000 );
	sub2(2);
	return (0);
}   // main
