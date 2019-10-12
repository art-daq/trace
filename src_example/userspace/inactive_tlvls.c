/*  This file (inactive_tlvls.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Aug  9, 2017. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1.30 $$Date: 2016/03/01 14:27:27 $";
	*/

#include <stdio.h>				/* snprintf */
#include "TRACE/trace.h"				/* TRACE, TRACE_CNTL */

int main( /*int argc, char *argv[]*/ )
{
	unsigned uu, longest_name=0;
	char name[15];
	for (uu=0; uu<70; ++uu) {
		snprintf(name,sizeof(name),"name%u",uu);
		TRACE_CNTL("name",name);
		TRACE_CNTL("lvlset",4L<<(uu%62),0L,0L);
		TRACE( 2, "trace not enabled by default" );
	}

	/* print the internal namLvlTbl */
	for (uu=0; uu<traceControl_p->num_namLvlTblEnts; ++uu)
		if (strnlen(traceNamLvls_p[uu].name,sizeof(traceNamLvls_p[0].name)) > longest_name)
			longest_name = strnlen(traceNamLvls_p[uu].name,sizeof(traceNamLvls_p[0].name));
	for (uu=0; uu<traceControl_p->num_namLvlTblEnts; ++uu)
		if (traceNamLvls_p[uu].name[0] != '\0')
			printf("%4u %*.*s 0x%016llx 0x%016llx 0x%016llx\n"
			       , uu
			       , longest_name, longest_name, traceNamLvls_p[uu].name
			       , (unsigned long long)traceNamLvls_p[uu].M
			       , (unsigned long long)traceNamLvls_p[uu].S
			       , (unsigned long long)traceNamLvls_p[uu].T
			       );
	return (0);
}   /* main */
