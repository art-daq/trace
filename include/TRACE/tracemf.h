/* This file (tracemf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracemf.hh,v $
 // rev="$Revision: 1440 $$Date: 2020-10-30 01:15:37 -0500 (Fri, 30 Oct 2020) $";
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
  static void mftrace_user(struct timeval *, int, uint8_t, const char*, const char*, int, const char*, uint16_t nargs, const char *msg, ...); \
  static void mftrace_user(struct timeval *, int, uint8_t, const char*, const char*, int, const char*, uint16_t nargs, const std::string& msg, ...)
#undef TRACE_LOG_FUNCTION
#define TRACE_LOG_FUNCTION mftrace_user
#include "TRACE/trace.h"		/* TRACE */

#define MFBOOL_WARNING __mwe
#define MFBOOL_INFO __mie
#define MFBOOL_DEBUG __mde

#define SEV_EN(lvl) (  ((lvl<static_cast<int>(TLVL_WARNING))||lvl==static_cast<int>(TLVL_NOTICE)) \
					 ||((lvl==static_cast<int>(TLVL_WARNING))&&MFBOOL_WARNING) \
					 ||((lvl==static_cast<int>(TLVL_INFO))&&MFBOOL_INFO) \
					    ||((lvl>=static_cast<int>(TLVL_DEBUG))&&MFBOOL_DEBUG))
// SLow FoRCe
#define SL_FRC(lvl) ((lvl<static_cast<int>(TLVL_DEBUG))||((lvl==static_cast<int>(TLVL_DEBUG))&&DEBUG_FORCED))

#undef  TLOG_ERROR           // TRACE_STREAMER(lvl, nam_or_fmt,fmt_or_nam,s_enabled,force_s)
#define TLOG_ERROR(name)   TRACE_STREAMER( TLVL_ERROR, &(name)[0], 0, 1, 1 )
#undef  TLOG_WARNING
#define TLOG_WARNING(name) TRACE_STREAMER( TLVL_WARNING, &(name)[0], 0, MFBOOL_WARNING, 1 )
#undef  TLOG_INFO
#define TLOG_INFO(name)    TRACE_STREAMER( TLVL_INFO,    &(name)[0], 0, MFBOOL_INFO, 1)
#undef  TLOG_DEBUG
#define TLOG_DEBUG(name)   TRACE_STREAMER( TLVL_DEBUG,   &(name)[0], 0, MFBOOL_DEBUG, DEBUG_FORCED)
#undef  TLOG_TRACE
#define TLOG_TRACE(name)   TRACE_STREAMER( TLVL_TRACE,   &(name)[0], 0, MFBOOL_DEBUG, 0)
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
#include "cetlib_except/exception.h" // cet::exception

SUPPRESS_NOT_USED_WARN
static bool __mwe = mf::isWarningEnabled(), __mie = mf::isInfoEnabled(), __mde = true;//mf::isDebugEnabled(); always false in v2_02_01

#if defined(__has_feature)
#  if __has_feature(thread_sanitizer)
__attribute__((no_sanitize("thread")))
#  endif
#endif
static void vmftrace_user(struct timeval *, int TID, uint8_t lvl, const char* insert
	, const char* file, int line, uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	first. There are 2 main reasons that this is done:
	1) allows the use of write to a specific tracePrintFd;
	2) there will be one system call which is most efficient and less likely
	to have the output mangled in a multi-threaded environment.
	*/
	size_t printed = 0;
	int    retval;
	const char *outp;
		char   obuf[TRACE_USER_MSGMAX];

	if ((insert && (printed = strlen(insert))) || nargs)
	{
		/* check insert 1st to make sure printed is set */
		// assume insert is smaller than obuf
		if (printed) {
			retval = snprintf(obuf,sizeof(obuf),"%s ",insert );
			printed = TRACE_SNPRINTED(retval,sizeof(obuf));
		}
		if (nargs) {
			retval = vsnprintf(&(obuf[printed]), sizeof(obuf) - printed, msg, ap); // man page say obuf will always be terminated
			printed += TRACE_SNPRINTED(retval,sizeof(obuf)-printed);
		} else {
			/* don't do any parsing for format specifiers in the msg -- tshow will
			   also know to do this on the memory side of things */
			retval = snprintf( &(obuf[printed]), sizeof(obuf)-printed, "%s", msg );
			printed += TRACE_SNPRINTED(retval,sizeof(obuf)-printed);
		}
		if (obuf[printed-1] == '\n')
			obuf[printed-1] = '\0';  // DONE w/ printed (don't need to decrement
		outp = obuf;
	} else {
		if (msg[strlen(msg)-1] == '\n') { // need to copy to remove the trailing nl
			retval = snprintf( obuf, sizeof(obuf), "%s", msg );
			printed = TRACE_SNPRINTED(retval,sizeof(obuf));
			if (obuf[printed-1] == '\n')
				obuf[printed-1] = '\0';  // DONE w/ printed (don't need to decrement
			outp = obuf;
		} else
			outp = msg;
	}

// Define MESSAGEFACILITY_HEX_VERSION in top-level CMakeLists.txt (see artdaq's CMakeLists.txt!)
#ifdef MESSAGEFACILITY_HEX_VERSION
# if MESSAGEFACILITY_HEX_VERSION >= 0x20201
#  ifdef ARTDAQ_DAQDATA_GLOBALS_HH
	mf::SetIteration(GetMFIteration());
	mf::SetModuleName(GetMFModuleName());
#  endif
# else
#  ifdef ARTDAQ_DAQDATA_GLOBALS_HH
	mf::SetContextIteration(GetMFIteration());
	mf::SetContextSinglet(GetMFModuleName());
#  endif
# endif
#endif

	switch (lvl)
	{
#  ifdef TRACEMF_USE_VERBATIM
		// NOTE: need this "Verbatim" set of methods to get full filename
	case TLVL_ERROR:   ::mf::LogProblem(TRACE_TID2NAME(TID), file, line) << outp; break;
	case TLVL_WARNING: ::mf::LogPrint(TRACE_TID2NAME(TID), file, line) << outp; break;
	case TLVL_INFO:    ::mf::LogVerbatim{ TRACE_TID2NAME(TID), file, line } << outp; break;
	case TLVL_DEBUG:   ::mf::LogTrace{ TRACE_TID2NAME(TID), file, line } << outp; break;
	default:           ::mf::LogTrace{ TRACE_TID2NAME(TID), file, line } << std::to_string(lvl) << ": " << outp; break;
#  else
	case TLVL_ERROR:   ::mf::LogError(TRACE_TID2NAME(TID), file, line) << outp; break;
	case TLVL_WARNING: ::mf::LogWarning(TRACE_TID2NAME(TID), file, line) << outp; break;
	case TLVL_INFO:    ::mf::LogInfo{ TRACE_TID2NAME(TID), file, line } << outp; break;
	case TLVL_DEBUG:   ::mf::LogDebug{ TRACE_TID2NAME(TID), file, line } << outp; break;
	default:           ::mf::LogDebug{ TRACE_TID2NAME(TID), file, line } << std::to_string(lvl) << ": " << outp; break;
#  endif
	}
}

SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint8_t lvl, const char* insert, const char* file, int line, const char* function, uint16_t nargs, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	if (mf::isMessageProcessingSetUp())
	{
		vmftrace_user(tvp, TID, lvl, insert, file, line, nargs, msg, ap);
	}
	else
	{
		vtrace_user(tvp, TID, lvl, insert, file, line, function, nargs, msg, ap); // vtrace_user does not use file, line
	}
	va_end(ap);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvarargs"
SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint8_t lvl, const char* insert, const char* file, int line, const char* function, uint16_t nargs, const std::string& msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	if (mf::isMessageProcessingSetUp())
	{
		vmftrace_user(tvp, TID, lvl, insert, file, line, nargs, &msg[0], ap);
	}
	else
	{
		vtrace_user(tvp, TID, lvl, insert, file, line, function, nargs, &msg[0], ap); // vtrace_user does not use file, line
	}
	va_end(ap);
}   /* trace */
#pragma GCC diagnostic pop



inline TraceStreamer& operator<<(TraceStreamer& x, cet::exception r)
{
	if (x.do_s || x.do_m)
		x.msg_append(r.what());
	return x;
}

#endif /* __cplusplus */
#endif /* TRACEMF_H */
