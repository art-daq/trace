/* This file (tracemf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracemf.hh,v $
 // rev="$Revision: 716 $$Date: 2017-12-12 11:40:21 -0600 (Tue, 12 Dec 2017) $";
 */
/** 
 * \file tracemf.h
 * Defines TRACE macros which send "slow" traces to MessageFacility
 *
 */
#ifndef TRACEMF_H
#define TRACEMF_H

#ifdef __cplusplus

// Use this define!  -- trace.h won't define it's own version of TRACE_LOG_FUNCTION
// The TRACE macro will then use the static vmftrace_user function defined in this file 
// for the "slow" tracing function (if appropriate mask bit is set :)
#include <stdint.h>				// uint16_t
#include <string>				// std::string
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs, const char *msg, ...);
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs, const std::string& msg, ...);
# undef TRACE_LOG_FUNCTION
# define TRACE_LOG_FUNCTION(tvp,tid,lvl,insert,nargs,...)  mftrace_user(tvp, tid, lvl,insert,__FILE__,__LINE__,nargs, __VA_ARGS__ )
# include "trace.h"		/* TRACE */

# include "messagefacility/MessageLogger/MessageLogger.h"	// LOG_DEBUG

# include <string>

static void vmftrace_user(struct timeval *, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	first. There are 2 main reasons that this is done:
	1) allows the use of write to a specific tracePrintFd;
	2) there will be one system call which is most efficient and less likely
	to have the output mangled in a multi-threaded environment.
	*/
	char   obuf[0x1800]; size_t printed = 0;
	
    if (printed < (sizeof(obuf)-1)) {
		size_t max=sizeof(obuf)-1 - printed;
		strncpy( &obuf[printed], insert, max );
		if (max < strlen(insert)) {
			obuf[printed+max] = '\0';
			printed += max;
		} else printed += strlen(insert);
	}
	if (nargs)
		printed += vsnprintf(&(obuf[printed])
							 , (printed < (int)sizeof(obuf)) ? sizeof(obuf) - printed : 0
							 , msg, ap);
	else /* don't do any parsing for format specifiers in the msg -- tshow will
		 also know to do this on the memory side of things */
		printed += snprintf(&(obuf[printed])
							, (printed < (int)sizeof(obuf)) ? sizeof(obuf) - printed : 0
							, "%s", msg);

	char namebuf[TRACE_DFLT_NAM_SZ];
	snprintf(&namebuf[0], TRACE_DFLT_NAM_SZ, "%s", traceNamLvls_p[TID].name);
	switch (lvl)
	{
	case TLVL_ERROR:  ::mf::LogError(namebuf) << obuf; break;
	case TLVL_WARNING:  ::mf::LogWarning(namebuf) << obuf; break;
	case TLVL_INFO:  ::mf::LogInfo(namebuf) << obuf; break;
	case TLVL_DEBUG:  ::mf::LogDebug{ namebuf, file, line } << obuf; break;
	default:          ::mf::LogDebug{ namebuf, file, line } << std::to_string(lvl) << ": " << obuf; break;
	}
}

SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vmftrace_user(tvp, TID, lvl, insert, file, line, nargs, msg, ap);
	va_end(ap);
}

static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs, const std::string& msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vmftrace_user(tvp, TID, lvl, insert, file, line, nargs, &msg[0], ap);
	va_end(ap);
}   /* trace */


inline TraceStreamer& operator<<(TraceStreamer& x, cet::exception y)
{
	if (x.enabled) { x.msg += y.what(); }
	return x;
}

#endif
#endif /* TRACEMF_H */
