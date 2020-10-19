/*  This file (inactive_tlvls.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Aug  9, 2017. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1398 $$Date: 2020-10-01 22:44:25 -0500 (Thu, 01 Oct 2020) $";
	*/
/*
# This cycles through all the debug levels for the fast/mem path. It creates
# a new name each time. The TRACEs after TRACE_CNTL("name"...) use that name.
export TRACE_FILE=/tmp/trace_buffer_${USER}_types;\
rm -f $TRACE_FILE;\
inactive_tlvls|less;\
tshow|tdelta -ct 1 -d 1
 */
#include <stdio.h>				/* snprintf */
#include "TRACE/trace.h"				/* TRACE, TRACE_CNTL */

int main( /*int argc, char *argv[]*/ )
{
	unsigned uu;
	unsigned longest_name=0;
	char name[15];
	for (uu=0; uu<70; ++uu) {
		uint8_t lvl=(uint8_t)(TLVL_DEBUG+(uu%(64-TLVL_DEBUG)));
		uint64_t msk=1ULL<<lvl;
		snprintf(name,sizeof(name),"name%u",uu);
		TRACE_CNTL("name",name);
		TRACE_CNTL("lvlset",msk,0ULL,0ULL); /* set just the fast path mask */
		TRACE( lvl, "trace not enabled by default (%llx)",(unsigned long long)msk );
	}

	/* print the internal namLvlTbl */
	for (uu=0; uu<traceControl_p->num_namLvlTblEnts; ++uu)
		if ((unsigned)strnlen(TRACE_TID2NAME((int32_t)uu),traceControl_p->nam_arr_sz) > longest_name)
			longest_name = (unsigned)strnlen(TRACE_TID2NAME((int32_t)uu),traceControl_p->nam_arr_sz);
	for (uu=0; uu<traceControl_p->num_namLvlTblEnts; ++uu)
		if (TRACE_TID2NAME((int32_t)uu)[0] != '\0')
			printf("%4u %*.*s 0x%016llx 0x%016llx 0x%016llx\n"
			       , uu
			       , longest_name, longest_name, TRACE_TID2NAME((int32_t)uu)
			       , (unsigned long long)traceLvls_p[uu].M
			       , (unsigned long long)traceLvls_p[uu].S
			       , (unsigned long long)traceLvls_p[uu].T
			       );
	return (0);
}   /* main */
