 // This file (ex_traceln.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jun  7, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1604 $$Date: 2023-10-14 22:51:04 -0500 (Sat, 14 Oct 2023) $";

#include <stdio.h>		// printf
#include "TRACE/traceln.h"
#define HDRNAME &std::string(__FILE__).substr(std::string(__FILE__).rfind('/')+1)[0]

class ExTraceLn
{
public:
	void trc(void) {
		TLOG()          << "LOG()   from header";
		TLOG(2,HDRNAME) << "TLOG(2) from header";
	};
};
