 // This file (TLOG_lower_levels.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 20, 2024. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.35 $$Date: 2023/10/14 17:08:08 $";

#include "TRACE/trace.h"

int main( /*int argc, char *argv[]*/ )
{
	TLOG_INFO()    << "INFO    is numeric level " << TLVL_INFO;
	TLOG_NOTICE()  << "NOTICE  is numeric level " << TLVL_NOTICE;
	TLOG_WARNING() << "WARNING is numeric level " << TLVL_WARNING;
	TLOG_ERROR()   << "ERROR   is numeric level " << TLVL_ERROR;
	TLOG_CRIT()    << "CRIT    is numeric level " << TLVL_CRIT;
	TLOG_ALERT()   << "ALERT   is numeric level " << TLVL_ALERT;
	TLOG_FATAL()   << "FATAL   is numeric level " << TLVL_FATAL;
	return (0);
}   // main
