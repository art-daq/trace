/* This file (tracemf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracemf.hh,v $
 // rev="$Revision: 641 $$Date: 2017-10-05 10:04:10 -0500 (Thu, 05 Oct 2017) $";
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
// The TRACE macro will then use the TRACE_MF_LOGGER macro defined below for the "slow"
// tracing function (if appropriate mask bit is set :)
#undef TRACE_LOG_FUNCTION
# define TRACE_LOG_FUNCTION(tvp,tid,lvl,nargs,...)  mftrace_user(tvp, tid, lvl,__FILE__,__LINE__,nargs, __VA_ARGS__ )
#include "trace.h"		/* TRACE */

// "static int tid_" is thread safe in so far as multiple threads may init,
// but will init with same value.
#define TRACE_STREAMER(lvl, name, force_s) {   TRACE_INIT_CHECK						\
	{static int tid_=-1; if(tid_==-1)tid_=name2TID(std::string(name).c_str());  int lvl_ = lvl; \
	bool do_m = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl)); \
	bool do_s = traceControl_rwp->mode.bits.S && (force_s || (traceNamLvls_p[tid_].S & TLVLMSK(lvl))); \
	if(do_s || do_m)  { std::ostringstream o;o

#define TRACE_ENDL ""; \
	   struct timeval lclTime; lclTime.tv_sec = 0;			\
		if(do_m) trace( &lclTime,tid_, lvl_, 0 TRACE_XTRA_PASSED,o.str() ); \
		if(do_s) TRACE_LOG_FUNCTION( &lclTime, tid_, lvl_, 0,o.str() );	\
	}}}
#define TLOG_ENDL TRACE_ENDL


#define TLVL_ERROR        0
#define TLVL_WARNING      1
#define TLVL_INFO         2
#define TLVL_DEBUG        3
#define TLVL_TRACE        4
#define TLOG_ERROR(name) TRACE_STREAMER(TLVL_ERROR, name, 1)
#define TLOG_WARNING(name) TRACE_STREAMER(TLVL_WARNING, name, mf::isWarningEnabled())
#define TLOG_INFO(name) TRACE_STREAMER(TLVL_INFO, name, mf::isInfoEnabled())
#define TLOG_DEBUG(name) TRACE_STREAMER(TLVL_DEBUG, name, mf::isDebugEnabled())
#define TLOG_TRACE(name) TRACE_STREAMER(TLVL_TRACE, name,0)
#define TLOG_ARB(lvl,name) TRACE_STREAMER(lvl,name,0)

#include "messagefacility/MessageLogger/MessageLogger.h"	// LOG_DEBUG

#include <string>

SUPPRESS_NOT_USED_WARN
static void vmftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* file, int line, uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	first. There are 2 main reasons that this is done:
	1) allows the use of write to a specific tracePrintFd;
	2) there will be one system call which is most efficient and less likely
	to have the output mangled in a multi-threaded environment.
	*/
	char   obuf[0x1000]; char tbuf[0x100]; size_t printed = 0;
	char   *cp;

	if (lvl > 3) {
		struct tm	tm_s;
		if (tracePrintFmt == NULL)
		{   /* no matter who writes, it should basically be the same thing */
			if ((cp = getenv("TRACE_TIME_FMT")) != NULL) tracePrintFmt = cp; /* single write here */
			else                                    tracePrintFmt = (char*)TRACE_DFLT_TIME_FMT; /* OR single write here */
		}
		if (tvp->tv_sec == 0) TRACE_GETTIMEOFDAY(tvp);
# if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
# endif
		localtime_r((time_t *)&tvp->tv_sec, &tm_s);
# if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#  pragma GCC diagnostic pop
# endif
		strftime(tbuf, sizeof(tbuf), tracePrintFmt, &tm_s);
		printed = snprintf(obuf, sizeof(obuf), tbuf, (int)tvp->tv_usec); /* possibly (probably) add usecs */
		if (printed < sizeof(obuf) - 2 && obuf[printed - 1] != ' ') {
			obuf[printed] = ' ';
			obuf[++printed] = '\0';
		}
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
	default: ::mf::LogTrace{ namebuf, file, line } << std::to_string(lvl) << ": " << obuf; break;
	}
}

SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* file, int line, uint16_t nargs, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vmftrace_user(tvp, TID, lvl, file, line, nargs, msg, ap);
	va_end(ap);
}
SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* file, int line, uint16_t nargs, std::string msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vmftrace_user(tvp, TID, lvl, file, line, nargs, msg.c_str(), ap);
	va_end(ap);
}   /* trace */

#endif /* TRACEMF_H */
#endif
