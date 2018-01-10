/* This file (tracemf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracemf.hh,v $
 // rev="$Revision: 783 $$Date: 2018-01-09 22:10:43 -0600 (Tue, 09 Jan 2018) $";
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

#define SEV_EN(lvl) ((lvl==0)||((lvl==1)&&mf::isWarningEnabled())||((lvl==2)&&mf::isInfoEnabled())||((lvl>=3)&&mf::isDebugEnabled()))
#define SL_FRC(lvl) ((lvl<=2)||((lvl==3)&&DEBUG_FORCED))

#undef  TLOG_ERROR           // TRACE_STREAMER(lvl, nam_or_fmt,fmt_or_nam,s_enabled,force_s)
#define TLOG_ERROR(name)   TRACE_STREAMER( TLVL_WARNING, &(name)[0], 0, 1, 1 )
#undef  TLOG_WARNING
#define TLOG_WARNING(name) TRACE_STREAMER( TLVL_WARNING, &(name)[0], 0, mf::isWarningEnabled(), 1 )
#undef  TLOG_INFO
#define TLOG_INFO(name)    TRACE_STREAMER( TLVL_INFO,    &(name)[0], 0, mf::isInfoEnabled(), 1)
#undef  TLOG_DEBUG
#define TLOG_DEBUG(name)   TRACE_STREAMER( TLVL_DEBUG,   &(name)[0], 0, mf::isDebugEnabled(), DEBUG_FORCED)
#undef  TLOG_TRACE
#define TLOG_TRACE(name)   TRACE_STREAMER( TLVL_TRACE,   &(name)[0], 0, mf::isDebugEnabled(), 0)
#undef  TLOG_DBG
#define TLOG_DBG(...)      TRACE_STREAMER( tlog_LVL(__VA_ARGS__,need_at_least_one),  tlog_ARG2(__VA_ARGS__,0,need_at_least_one) \
										  ,tlog_ARG3(__VA_ARGS__,0,"",need_at_least_one) \
										  ,SEV_EN(tlog_LVL(__VA_ARGS__,need_at_least_one)), SL_FRC(tlog_LVL( __VA_ARGS__,need_at_least_one)) )
#undef  TLOG_ARB
#define TLOG_ARB(...)      TRACE_STREAMER( tlog_LVL(__VA_ARGS__,need_at_least_one), tlog_ARG2(__VA_ARGS__,0,need_at_least_one) \
										  , tlog_ARG3(__VA_ARGS__,0,"",need_at_least_one) \
										  , SEV_EN(tlog_LVL(__VA_ARGS__,need_at_least_one)), SL_FRC(tlog_LVL( __VA_ARGS__,need_at_least_one)) )
#undef  TLOG
#define TLOG(...)          TRACE_STREAMER( tlog_LVL( __VA_ARGS__,need_at_least_one),tlog_ARG2(__VA_ARGS__,0,need_at_least_one) \
										  ,tlog_ARG3(__VA_ARGS__,0,"",need_at_least_one) \
										  ,SEV_EN(tlog_LVL(__VA_ARGS__,need_at_least_one)), SL_FRC(tlog_LVL( __VA_ARGS__,need_at_least_one)) )

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
			vsnprintf( &(obuf[printed]), sizeof(obuf)-printed, msg, ap ); // man page say obuf will always be terminated
		else {/* don't do any parsing for format specifiers in the msg -- tshow will
				also know to do this on the memory side of things */
			strncpy( &(obuf[printed]), msg, sizeof(obuf)-1-printed ); // man page says obuf may not get terminated
			obuf[sizeof(obuf)-1]='\0';
		}
		outp = obuf;
	} else
		outp = msg;


	char namebuf[TRACE_DFLT_NAM_CHR_MAX+1];
	strcpy( namebuf, traceNamLvls_p[TID].name ); // could just give traceNamLvls_p[TID].name to Log*
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
