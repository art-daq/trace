/*  This file (tracelib.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Mar 11, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: tracelib.c,v $
    rev="$Revision$$Date$";
    */

#define TRACE_LIB
#include "tracelib.h"

struct traceNamLvls_s  traceNamLvls[TRACE_DISABLE_NAM_SZ];
TRACE_THREAD_LOCAL struct traceNamLvls_s  *traceNamLvls_p=&traceNamLvls[0];
TRACE_THREAD_LOCAL struct traceEntryHdr_s *traceEntries_p;
TRACE_THREAD_LOCAL struct traceControl_s  *traceControl_p=NULL;
TRACE_THREAD_LOCAL const char *traceFile="/tmp/trace_buffer_%s";/*a local/efficient FS device is best; operation when path is on NFS device has not been studied*/
TRACE_THREAD_LOCAL const char *traceName="TRACE";
int                      tracePrintFd=1;
pid_t                    tracePid=0;
TRACE_THREAD_LOCAL int   traceTID=0;
TRACE_THREAD_LOCAL pid_t traceTid=0;

