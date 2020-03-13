 // This file (trace_lvl.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Sep 17, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";


// test TRACE(lvl++,...) and TLOG( 

// g++ -Wall -g -o trace_lvl{,.cc} -I$TRACE_INC && ./trace_lvl

#include <stdio.h>		// printf
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
#define lvl        2
#define nam_or_fmt 0
#define fmt_or_nam ""
#define s_enabled  1
#define force_s    0
static TRACE_THREAD_LOCAL TraceStreamer steamer;
 for (struct _T_ {uint8_t lvl__; int tid; tstreamer_flags flgs; char ins[32]; struct timeval tv;
	 _T_(uint8_t llv):lvl__(llv),tid(-1){tv.tv_sec=0;}} _xx((uint8_t)(lvl));
		 (_xx.tid == -1) && ((_xx.tid = TRACE_STATIC_TID_ENABLED(t_arg_nmft(nam_or_fmt, fmt_or_nam, &_xx.flgs), _xx.lvl__, s_enabled, force_s,
		                                                         &_xx.flgs, &_xx.tv, _xx.ins, sizeof(_xx.ins))) != -1);
		 steamer.str())
		steamer.init(_xx.tid, _xx.lvl__, _xx.flgs, __FILE__, __LINE__, &_xx.tv, _xx.ins, &TRACE_LOG_FUNCTION) << "hello";


	int lvlx=0x107;  // testing conversion between int and uint8_t
	TRACEN("test",lvlx,"test");
	TRACEN_("test",lvlx,"test"<<"ing");
	sub1(2);
	usleep( 1000000 );
	sub2(2);
	return (0);
}   // main
