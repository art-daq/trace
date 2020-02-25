/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just.cc,v $
// rev="$Revision: 1056 $$Date: 2019-02-25 15:57:13 -0600 (Mon, 25 Feb 2019) $";
*/
/*
/home/ron/work/tracePrj/trace
ron@mu2edaq01 :^) just_recursive
02-06 12:17:58.697050   TRACE err Completing greeting from sub
02-06 12:17:58.697114   TRACE err Completing greeting there lvl=0 from sub
02-06 12:17:58.697117   TRACE err Completing greeting there lvl=1 from sub
02-06 12:17:58.697119   TRACE err Completing greeting there lvl=2 from sub
02-06 12:17:58.697121   TRACE err Completing greeting there lvl=3 from sub
02-06 12:17:58.697122   TRACE err Completing greeting there lvl=4 from sub
02-06 12:17:58.697124   TRACE err Completing greeting there lvl=5 one
02-06 12:17:58.697128   TRACE err Completing greeting from sub
02-06 12:17:58.697130   TRACE err Completing greeting there lvl=0 from sub
02-06 12:17:58.697132   TRACE err Completing greeting there lvl=1 from sub
02-06 12:17:58.697134   TRACE err Completing greeting there lvl=2 two
02-06 12:17:58.697137   TRACE err hi there there
02-06 12:17:58.697139   TRACE err Completing greeting main
--2020-02-06_12:17:58--
*/
#include <string>
#include "TRACE/trace.h"		/* TRACE */

std::string example_sub_(const char *msg, int lvl) {
    if (lvl--)
		TLOG(TLVL_ERROR) << "Completing greeting" << example_sub_("from sub", lvl) << " lvl=" << lvl << " " << msg;
	else
		TLOG(TLVL_ERROR) << "Completing greeting " << msg;
	return " there";
}

int
//main(  int	argc
//     , char	*argv[] )
main()
{
	TLOG(TLVL_ERROR) << "hi" << example_sub_("one",6) << example_sub_("two",3);
    example_sub_("main", 0);
    return (0);
}   /* main */
