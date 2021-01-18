/* This file (traceln.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: traceln.hh,v $
 // rev="$Revision: 1056 $$Date: 2019-02-25 15:57:13 -0600 (Mon, 25 Feb 2019) $";
 */
 /**
  * \file traceln.h
  * Defines TRACE macros which send "slow" traces to cout with line numbers
  *
  */
#ifndef TRACELN_H
#define TRACELN_H

#ifdef __cplusplus

  // Use this define!  -- trace.h won't define it's own version of TRACE_LOG_FUNCTION
  // The TRACE macro will then use the static vlntrace_user function defined in this file
  // for the "slow" tracing function (if appropriate mask bit is set :)
#include <stdint.h>				// uint16_t
#include <string>				// std::string

#define TRACE_LOG_FUN_PROTO \
  static void lntrace_user(struct timeval *, int, uint8_t, const char*, const char*, int, const char*, uint16_t nargs, const char *msg, ...); \
  static void lntrace_user(struct timeval *, int, uint8_t, const char*, const char*, int, const char*, uint16_t nargs, const std::string& msg, ...)
#define TRACE_LOG_FUNCTION lntrace_user
#include "TRACE/trace.h"		/* TRACE */


#if defined(__has_feature)
#  if __has_feature(thread_sanitizer)
__attribute__((no_sanitize("thread")))
#  endif
#endif
static void vlntrace_user(struct timeval *tvp, int TID __attribute__((__unused__)), uint8_t lvl, const char* insert
                          , const char* file, int line, const char *function __attribute__((__unused__)), uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	   first. There are 2 main reasons that this is done:
	   1) allows the use of write to a specific tracePrintFd;
	   2) there will be one system call which is most efficient and less likely
	   to have the output mangled in a multi-threaded environment.
	*/
	char obuf[TRACE_USER_MSGMAX];
	char tbuf[0x100];
	size_t printed = 0;
	char *cp;
	struct tm tm_s;
	ssize_t quiet_warn = 0;
	if (traceTimeFmt == NULL)
	{
		/* no matter who writes, it should basically be the same thing */
		if ((cp = getenv("TRACE_TIME_FMT")) != NULL)
		{
			traceTimeFmt = cp; /* single write here */
		}
		else
		{
			traceTimeFmt = TRACE_DFLT_TIME_FMT; /* OR single write here */
		}
	}
	if (tvp->tv_sec == 0)
	{
		TRACE_GETTIMEOFDAY(tvp);
	}
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#	endif
	localtime_r((time_t *)&tvp->tv_sec, &tm_s);
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic pop
#	endif
	strftime(tbuf, sizeof(tbuf), traceTimeFmt, &tm_s);
	printed = snprintf(obuf, sizeof(obuf), tbuf, (int)tvp->tv_usec);                                    /* possibly (probably) add usecs */


#define __SHORTFILE__ \
       (strstr(&file[0], "/srcs/") ? strstr(&file[0], "/srcs/") + 6 : file)

	printed += snprintf(&(obuf[printed]), sizeof(obuf) - printed, &(" %s %s:%d %s")[printed == 0 ? 1 : 0] /* skip leading " " if nothing was printed (TRACE_TIME_FMT="") */
	                    , trace_lvlstrs[0][lvl & TLVLBITSMSK]
	                    , __SHORTFILE__, line
	                    , insert);

	if (nargs)
	{
		printed += vsnprintf(&(obuf[printed]), (printed < (int)sizeof(obuf)) ? sizeof(obuf) - printed : 0, msg, ap);
	}
	else
	{ /* don't do any parsing for format specifiers in the msg -- tshow will
		 also know to do this on the memory side of things */
		printed += snprintf(&(obuf[printed]), (printed < (int)sizeof(obuf)) ? sizeof(obuf) - printed : 0, "%s", msg);
	}

	/* why not use writev??? B/c when writing to stdout, only each individual
	   vector (not the whole array of vectors) is atomic/thread safe */
	if (printed < (int)sizeof(obuf))
	{
		/* there is room for the \n */
		/* buf first see if it is needed */
		if (obuf[printed - 1] != '\n')
		{
			obuf[printed++] = '\n'; /* overwriting \0 is OK as we will specify the amount to write */
									/*printf("added \\n printed=%d\n",printed);*/
		}
		/*else printf("already there printed=%d\n",printed);*/
		quiet_warn += write(tracePrintFd[lvl & TLVLBITSMSK], obuf, printed);
	}
	else
	{
		/* obuf[sizeof(obuf)-1] has '\0'. see if we should change it to \n */
		if (obuf[sizeof(obuf) - 2] == '\n')
		{
			quiet_warn += write(tracePrintFd[lvl & TLVLBITSMSK], obuf, sizeof(obuf) - 1);
		}
		else
		{
			obuf[sizeof(obuf) - 1] = '\n';
			quiet_warn += write(tracePrintFd[lvl & TLVLBITSMSK], obuf, sizeof(obuf));
			/*printf("changed \\0 to \\n printed=%d\n",);*/
		}
	}
	if (quiet_warn == -1)
	{
		perror("writeTracePrintFd");
	}
}

SUPPRESS_NOT_USED_WARN
static void lntrace_user(struct timeval *tvp, int TID, uint8_t lvl, const char* insert, const char* file, int line, const char* function, uint16_t nargs, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vlntrace_user(tvp, TID, lvl, insert, file, line, function, nargs, msg, ap);
	va_end(ap);
}

#if ( __GNUC__ >= 6 ) || ( __cplusplus >= 201103L )
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wvarargs"
#endif
SUPPRESS_NOT_USED_WARN
static void lntrace_user(struct timeval *tvp, int TID, uint8_t lvl, const char* insert, const char* file, int line, const char* function, uint16_t nargs, const std::string& msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vlntrace_user(tvp, TID, lvl, insert, file, line, function, nargs, &msg[0], ap);
	va_end(ap);
}   /* trace */
#if ( __GNUC__ >= 6 ) || ( __cplusplus >= 201103L )
#pragma GCC diagnostic pop
#endif

#endif /* __cplusplus */
#endif /* TRACELN_H */
