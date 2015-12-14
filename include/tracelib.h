/* This file (tracelib.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 11, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracelib.h,v $
 // rev="$Revision$$Date$";
 */
#ifndef TRACELIB_H
#define TRACELIB_H

#ifdef cplusplus
extern "C" {
#endif

#define TRACE_DECL( scope, type_name, initializer ) extern type_name
#include "trace.h"
#undef  TRACE_DECL

#ifdef cplusplus
}
#endif

#endif /* TRACELIB_H */
