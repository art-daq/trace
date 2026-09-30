// This file (ttest.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Sep 28, 2026. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.39 $$Date: 2026/08/27 18:49:57 $";

#include <stdio.h>  // printf
#include <TRACE/trace.h>

int main(void)
{
	if (TTEST(0)) {  // NOTE: the level is a DEBUG level: 0=TLVL_DEBUG, 1=TLVL_DEBUG+1, ...
		TLOG_DEBUG() << "TLVL_DEBUG enabled/active for either or both slow or fast/mem paths";
	} else {
		printf("TLVLDEBUG disabled for both slow and fast/mem paths (includes TRACE_FILE disabled)\n");
		printf("Try: TRACE_LVLS=0x1ff ttest\n");
	}
	return (0);
}  // main
