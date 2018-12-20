/*  This file (inactive_tlvls.c) was created by Ron Rechenmacher <ron@fnal.gov> on
        Aug  9, 2017. "TERMS AND CONDITIONS" governing this file are in the README
        or COPYING file. If you do not have such a file, one can be obtained by
        contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
        $RCSfile: .emacs.gnu,v $
        rev="$Revision: 1.30 $$Date: 2016/03/01 14:27:27 $";
        */

#include <stdio.h> /* snprintf */
#include "trace.h" /* TRACE, TRACE_CNTL */

int main(int argc, char *argv[]) {
  int ii, longest_name = 0;
  char name[15];
  for (ii = 0; ii < 70; ++ii) {
    snprintf(name, sizeof(name), "name%d", ii);
    TRACE_CNTL("name", name);
    TRACE_CNTL("lvlset", 4L << (ii % 62), 0L, 0L);
    TRACE(2, "trace not enabled by default");
  }

  /* print the internal namLvlTbl */
  for (ii = 0; ii < traceControl_p->num_namLvlTblEnts; ++ii)
    if (strnlen(traceNamLvls_p[ii].name, sizeof(traceNamLvls_p[0].name)) > longest_name)
      longest_name = strnlen(traceNamLvls_p[ii].name, sizeof(traceNamLvls_p[0].name));
  for (ii = 0; ii < traceControl_p->num_namLvlTblEnts; ++ii)
    if (traceNamLvls_p[ii].name[0] != '\0')
      printf("%4d %*.*s 0x%016llx 0x%016llx 0x%016llx\n", ii, longest_name, longest_name, traceNamLvls_p[ii].name,
             (unsigned long long)traceNamLvls_p[ii].M, (unsigned long long)traceNamLvls_p[ii].S,
             (unsigned long long)traceNamLvls_p[ii].T);
  return (0);
} /* main */
