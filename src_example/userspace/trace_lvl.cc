 // This file (trace_lvl.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Sep 17, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1372 $$Date: 2020-09-19 21:20:04 -0500 (Sat, 19 Sep 2020) $";


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
#define _lvl       2
#define nam_or_fmt 0
#define fmt_or_nam ""
#define s_enabled  1
#define force_s    0
	static TRACE_THREAD_LOCAL TraceStreamer steamer;
	for (struct _T_ {unsigned once; uint8_t lvl; int *tidp; limit_info_t *lim_infop; tstreamer_flags flgs; const char *nn; char ins[32]; struct timeval tv; void* stmr__;
		_T_(uint8_t llv,tinfo_t *infop):once(1),lvl(llv),tidp(&infop->tid),lim_infop(&infop->info),stmr__(&__tstreamer){tv.tv_sec=0;}
		~_T_(){if(stmr__ != (void*)&__tstreamer) delete (TraceStreamer*)stmr__;} } _tlog_((uint8_t)(_lvl),TRACE_GET_STATIC());
		 _tlog_.once-- && TRACE_INIT_CHECK(TRACE_NAME)
			 && (_tlog_.nn=t_arg_nmft(nam_or_fmt, fmt_or_nam, &_tlog_.flgs),((*_tlog_.tidp != -1) || ((*_tlog_.tidp=(_tlog_.nn[0]?trace_name2TID(_tlog_.nn):traceTID))!=-1)))
			 && trace_do_streamer(&_tlog_.tv,_tlog_.tidp,_tlog_.lvl,_tlog_.lim_infop,_tlog_.ins,sizeof(_tlog_.ins),&_tlog_.flgs,s_enabled,force_s);
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
