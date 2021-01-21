 // This file (TLOG.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 20, 2021. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";
/*
    # try:
    treset; TLOG; PAGER= tshow | tdelta -ct 1
 */
#include "TRACE/trace.h"

int main(/*int argc, char *argv[]*/)
{
	TLOG()                      << "hello - TLOG() - no args - lvl defaults to new (new-ish) \"LOG\" level";
	TLOG(TLVL_INFO)             << "hello - TLOG(TLVL_INFO) - lvl arg";
	TLOG("TEST_INFO",TLVL_INFO) << "hello - TLOG(\"TEST_INFO\",TLVL_INFO) - lvl and name args, name first";

	// RECALL -- if no non-string args, no problem
	TLOG(TLVL_LOG,1)            << "hi - TLOG(TLVL_LOG,1) - always a msg (w/ non-str arg) w/ % as in %MSG " << 1;
	unsigned long old_modeS=TRACE_CNTL("modeS",0);
	TLOG(TLVL_LOG,1)            << "hi - TLOG(TLVL_LOG,1) - always a msg (w/ non-str arg) w/ % as in %MSG " << 1;
	TLOG(TLVL_LOG)              << "hi - TLOG(TLVL_LOG)   - dly fmt if only fast - a msg w/ % as in %MSG and a non-str arg: " << 1;
	TRACE_CNTL("modeS",old_modeS);
	TLOG(TLVL_LOG)              << "hi - TLOG(TLVL_LOG)   - dly fmt if only fast - a msg w/ % as in %MSG and a non-str arg: " << 1;
	return (0);
}   // main
