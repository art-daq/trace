/* This file (tracemf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracemf.hh,v $
 // rev="$Revision: 772 $$Date: 2017-12-27 17:28:55 -0600 (Wed, 27 Dec 2017) $";
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
#define TRACE_LOG_FUN_PROTO \
  static void mftrace_user(struct timeval *, int, uint16_t, const char*, const char*, int, uint16_t nargs TRACE_XTRA_UNUSED, const char *msg, ...);\
  static void mftrace_user(struct timeval *, int, uint16_t, const char*, const char*, int, uint16_t nargs TRACE_XTRA_UNUSED, const std::string& msg, ...)
#undef TRACE_LOG_FUNCTION
#define TRACE_LOG_FUNCTION(tvp,tid,lvl,insert,nargs,...)				\
	mftrace_user(tvp, tid, lvl,insert,__FILE__,__LINE__,nargs TRACE_XTRA_PASSED, __VA_ARGS__ )
#include "trace.h"		/* TRACE */
#undef  TLOG_WARNING
#define TLOG_WARNING(name) TRACE_STREAMER(TLVL_WARNING, &(name)[0], mf::isWarningEnabled(), 0)
#undef  TLOG_INFO
#define TLOG_INFO(name)    TRACE_STREAMER(TLVL_INFO,    &(name)[0], mf::isInfoEnabled(), 0)
#undef  TLOG_DEBUG
#define TLOG_DEBUG(name)   TRACE_STREAMER(TLVL_DEBUG,   &(name)[0], mf::isDebugEnabled() && DEBUG_FORCED, 0)

#include "messagefacility/MessageLogger/MessageLogger.h"	// LOG_DEBUG

#include <string>

static void vmftrace_user(struct timeval *, int TID, uint16_t lvl, const char* insert
                          , const char* file, int line
                          , uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	first. There are 2 main reasons that this is done:
	1) allows the use of write to a specific tracePrintFd;
	2) there will be one system call which is most efficient and less likely
	to have the output mangled in a multi-threaded environment.
	*/
	char   obuf[0x1800]; size_t printed=0;
	const char *outp;

	if ((insert && (printed=strlen(insert))) || nargs) { /* check insert 1st to make sure printed is set */
		// assume insert is smaller than obuf
		if (printed)
			strcpy( obuf, insert );
		if (nargs)
			vsnprintf( &(obuf[printed]), sizeof(obuf)-1-printed, msg, ap );
		else /* don't do any parsing for format specifiers in the msg -- tshow will
				also know to do this on the memory side of things */
			strncpy( &(obuf[printed]), msg, sizeof(obuf)-1-printed );
		obuf[sizeof(obuf)-1]='\0';
		outp = obuf;
	} else
		outp = msg;


	char namebuf[TRACE_DFLT_NAM_SZ];
	snprintf(&namebuf[0], TRACE_DFLT_NAM_SZ, "%s", traceNamLvls_p[TID].name);
	switch (lvl)
	{
	case TLVL_ERROR:   ::mf::LogError(namebuf)   << outp; break;
	case TLVL_WARNING: ::mf::LogWarning(namebuf) << outp; break;
	case TLVL_INFO:    ::mf::LogInfo{namebuf}    << outp; break;
	case TLVL_DEBUG:   ::mf::LogDebug{ namebuf, file, line } << outp; break;
	default:           ::mf::LogDebug{ namebuf, file, line } << std::to_string(lvl) << ": " << outp; break;
	}
}

SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs TRACE_XTRA_UNUSED, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vmftrace_user(tvp, TID, lvl, insert, file, line, nargs, msg, ap);
	va_end(ap);
}

SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs TRACE_XTRA_UNUSED, const std::string& msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vmftrace_user(tvp, TID, lvl, insert, file, line, nargs, &msg[0], ap);
	va_end(ap);
}   /* trace */


inline TraceStreamer& operator<<(TraceStreamer& x, cet::exception r)
{
	x.msg_append( r.what() );
	return x;
}

#endif
#endif /* TRACEMF_H */
