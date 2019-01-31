/* This file (tracemf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracemf.hh,v $
 // rev="$Revision: 710 $$Date: 2017-11-21 15:12:10 -0600 (Tue, 21 Nov 2017) $";
 */
 /**
  * \file tracemf.h
  * Defines TRACE macros which send "slow" traces to MessageFacility
  *
  */
#ifndef TRACEMF2_H
#define TRACEMF2_H

#ifdef __cplusplus
  // Use this define!  -- trace.h won't define it's own version of TRACE_LOG_FUNCTION
  // The TRACE macro will then use the static vmftrace_user function defined below
  // for the "slow" tracing function (if appropriate mask bit is set :)
#undef TRACE_LOG_FUNCTION
# define TRACE_LOG_FUNCTION(tvp,tid,lvl,insert,file,line,nargs,...)  mftrace_user(tvp, tid, lvl,insert,file,line,nargs, __VA_ARGS__ )
#include "trace.h"		/* TRACE */

// "static int tid_" is thread safe in so far as multiple threads may init,
// but will init with same value.
#define TRACE_STREAMER(lvl, name, force_s) {   TRACE_INIT_CHECK						\
	{static TRACE_THREAD_LOCAL int tid_ =-1;if (tid_ == -1)tid_ = name2TID(std::string(name).c_str()); \
	static TRACE_THREAD_LOCAL TraceStreamer s; s.init(tid_, lvl, force_s, __FILE__, __LINE__)

#define TRACE_ENDL ""; s.str();}}
#define TLOG_ENDL TRACE_ENDL

#ifdef __OPTIMIZE__
# define DEBUG_FORCED 0
#else
# define DEBUG_FORCED 1
#endif

#define TLVL_ERROR        0
#define TLVL_WARNING      1
#define TLVL_INFO         2
#define TLVL_DEBUG        3
#define TLVL_TRACE        4
#define TLOG_ERROR(name) TRACE_STREAMER(TLVL_ERROR, name, 1)
#define TLOG_WARNING(name) TRACE_STREAMER(TLVL_WARNING, name, mf::isWarningEnabled())
#define TLOG_INFO(name) TRACE_STREAMER(TLVL_INFO, name, mf::isInfoEnabled())
#define TLOG_DEBUG(name) TRACE_STREAMER(TLVL_DEBUG, name, mf::isDebugEnabled() && DEBUG_FORCED)
#define TLOG_TRACE(name) TRACE_STREAMER(TLVL_TRACE, name,0)
#define TLOG_DBG(lvl,name) TRACE_STREAMER(lvl,name,0)
#define TLOG_ARB(lvl,name) TRACE_STREAMER(lvl,name,0)
#define TRACE_STREAMER_ARGSMAX 35
#define TRACE_STREAMER_MSGMAX 300
#define TRACE_STREAMER_DEBUG 0
#define TRACE_STREAMER_TEMPLATE 1
#define TRACE_STREAMER_EXPAND(args) args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9] \
                                   ,args[10],args[11],args[12],args[13],args[14],args[15],args[16],args[17],args[18],args[19] \
								   ,args[20],args[21],args[22],args[23],args[24],args[25],args[26],args[27],args[28],args[29] \
                                   ,args[30],args[31],args[32],args[33],args[34]

#include "messagefacility/MessageLogger/MessageLogger.h"	// LOG_DEBUG

#include <string>
#include <array>
#include <iostream>
#include <iomanip>


SUPPRESS_NOT_USED_WARN
static void vmftrace_user(struct timeval *, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	first. There are 2 main reasons that this is done:
	1) allows the use of write to a specific tracePrintFd;
	2) there will be one system call which is most efficient and less likely
	to have the output mangled in a multi-threaded environment.
	*/
	char   obuf[0x1800]; size_t printed = 0;

	if (printed < (sizeof(obuf) - 1))
	{
		size_t max = sizeof(obuf) - 1 - printed;
		strncpy(&obuf[printed], insert, max);
		if (max < strlen(insert))
		{
			obuf[printed + max] = '\0';
			printed += max;
		}
		else printed += strlen(insert);
	}
	// could/should check for \n at end of msg, but this implementation is not that important
	if (nargs)
		vsnprintf( &(obuf[printed])
		          , (printed < (int)sizeof(obuf)) ? sizeof(obuf) - printed : 0
		          , msg, ap );
	else { /* don't do any parsing for format specifiers in the msg -- tshow will
		 also know to do this on the memory side of things because nargs is
		 stored in memory trace buffer. */
		strncpy( &(obuf[printed])
		        , (printed < (int)sizeof(obuf)) ? sizeof(obuf) - 1 - printed : 0
		        , msg );
		obuf[sizeof(obuf)-1] = '\0';
	}

	char namebuf[TRACE_DFLT_NAM_CHR_MAX+1];
	strcpy( namebuf, traceNamLvls_p[TID].name ); // could just give traceNamLvls_p[TID].name to Log*
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
SUPPRESS_NOT_USED_WARN
static void mftrace_user(struct timeval *tvp, int TID, uint16_t lvl, const char* insert, const char* file, int line, uint16_t nargs, const std::string& msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vmftrace_user(tvp, TID, lvl, insert, file, line, nargs, &msg[0], ap);
	va_end(ap);
}   /* trace */



struct TraceStreamer : std::ios
{
	union arg
	{
		int i;
		double d;
		long unsigned int u;
		long int l;
		void* p;
	};

	std::string msg;
	arg  args[TRACE_STREAMER_ARGSMAX];
	size_t argCount;
	int tid_;
	int lvl_;
	bool do_s;
	bool do_m;
	bool enabled;
	std::string widthStr;
	std::string precisionStr;
	const char *file_;
	int         line_;

public:
	explicit TraceStreamer() : argCount(0)
	{
		msg.reserve(TRACE_STREAMER_MSGMAX);
#if TRACE_STREAMER_DEBUG
		std::cout << "TraceStreamer CONSTRUCTOR" << std::endl;
#endif
	}

	inline TraceStreamer& init(int tid, int lvl, bool force_s,const char *file, int line)
	{
		TRACE_INIT_CHECK
		{
			msg = "";
		argCount = 0;
		tid_ = tid;
		lvl_ = lvl;
		file_ = file;
        line_ = line;
		do_m = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl));
		do_s = traceControl_rwp->mode.bits.S && ((force_s) || (traceNamLvls_p[tid_].S & TLVLMSK(lvl)));
		enabled = do_s || do_m;
		}
		return *this;
	}

	inline void str()
	{
#if TRACE_STREAMER_DEBUG
		std::cout << "Message is " << msg << std::endl;
#endif
		//call_trace(args, argCount);
		struct timeval lclTime; lclTime.tv_sec = 0;
		if (do_m) trace(&lclTime, tid_, lvl_, argCount TRACE_XTRA_PASSED, msg, TRACE_STREAMER_EXPAND(args));
		if (do_s)
		{
			TRACE_LIMIT_SLOW(_insert, &lclTime) TRACE_LOG_FUNCTION(&lclTime, tid_, lvl_, _insert, file_, line_, argCount, msg, TRACE_STREAMER_EXPAND(args));
		}
}

	inline void format(bool isFloat, bool isUnsigned, std::string length, std::ios::fmtflags flags)
	{
		//See, for example: http://www.cplusplus.com/reference/cstdio/printf/
		msg += "%";

		// Flags
		if (flags & left) { msg += "-"; }
		if (flags & showpos) { msg += "+"; }
		if (flags & (showpoint | showbase)) { msg += "#"; } // INCLUSIVE OR

		// Width
		msg += widthStr;

		if (isFloat)
		{
			// Precision
			msg += (precisionStr.size() ? "." + precisionStr : "") + length;

			if ((flags & (fixed | scientific)) == (fixed | scientific)) /*AND*/ { msg += flags & uppercase ? "A" : "a"; }
			else if (flags & fixed) { msg += flags & uppercase ? "F" : "f"; }
			else if (flags & scientific) { msg += flags & uppercase ? "E" : "e"; }
			else { msg += flags & uppercase ? "G" : "g"; }
		}
		else
		{
			msg += length;
			if (isUnsigned)
			{
				if (flags & hex) { msg += flags & uppercase ? "X" : "x"; }
				else if (flags & oct) { msg += "o"; }
				else { msg += "u"; }
			}
			else
			{
				msg += "d";
			}
		}
	}

	inline TraceStreamer& precision(int y)
	{
		if (y != _M_precision)
		{
			_M_precision = y;
			precisionStr = std::to_string(y);
#if TRACE_STREAMER_DEBUG
			std::cout << "TraceStreamer precisionStr is now " << precisionStr << std::endl;
#endif
		}
		return *this;
	}

	inline TraceStreamer& width(int y)
	{
		if (y != _M_width)
		{
			_M_width = y;
			widthStr = std::to_string(y);
#if TRACE_STREAMER_DEBUG
			std::cout << "TraceStreamer widthStr is now " << widthStr << std::endl;
#endif
		}		return *this;
	}

	inline TraceStreamer& operator<<(void* const& p) // Tricky C++...to pass pointer by reference, have to have the const AFTER the type
	{
		if (enabled && argCount < TRACE_STREAMER_ARGSMAX)
		{
			msg += "%p";
			args[argCount].p = p;
			argCount++;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const bool& b)
	{
		if (enabled)
		{
			if (_M_flags & boolalpha)
			{
				msg += (b ? "true" : "false");
			}
			else if (argCount < TRACE_STREAMER_ARGSMAX)
			{
				msg += "%d";
				args[argCount].i = b;
				argCount++;
			}
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const int& r)
	{
		if (enabled && argCount < TRACE_STREAMER_ARGSMAX)
		{
			format(false, false, "", _M_flags);
			args[argCount].i = r;
			argCount++;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const long int& r)
	{
		if (enabled && argCount < TRACE_STREAMER_ARGSMAX)
		{
			format(false, false, "l", _M_flags);
			args[argCount].l = r;
			argCount++;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const unsigned int& r)
	{
		if (enabled && argCount < TRACE_STREAMER_ARGSMAX)
		{
			format(false, true, "", _M_flags);
			args[argCount].u = r;
			argCount++;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const long unsigned int& r)
	{
		if (enabled && argCount < TRACE_STREAMER_ARGSMAX)
		{
			format(false, true, "l", _M_flags);
			args[argCount].u = r;
			argCount++;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const double& r)
	{
		if (enabled && argCount < TRACE_STREAMER_ARGSMAX)
		{
			format(true, false, "", _M_flags);
			args[argCount].d = r;
			argCount++;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const std::string& s)
	{
		if (enabled)
		{
			msg += s;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(char const* s)
	{
		if (enabled)
		{
			msg += s;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(char* s)
	{
		if (enabled)
		{
			msg += s;
		}
		return *this;
	}

	inline TraceStreamer& operator<<(const TraceStreamer& r)
	{
		if (enabled)
		{
			for (size_t ii = argCount; ii < (argCount + r.argCount < TRACE_STREAMER_ARGSMAX ? argCount + r.argCount : TRACE_STREAMER_ARGSMAX); ++ii)
			{
				args[ii] = r.args[ii - argCount];
			}
			argCount = argCount + r.argCount < TRACE_STREAMER_ARGSMAX ? argCount + r.argCount : TRACE_STREAMER_ARGSMAX;

			msg += r.msg;
		}
		return *this;
	}

	typedef std::ios_base& (*manipulator)(std::ios_base&);
	inline TraceStreamer& operator<<(manipulator r)
	{
		r(*this);
		return *this;
	}

	////https://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator
	/// https://stackoverflow.com/questions/2212776/overload-handling-of-stdendl
	typedef std::ostream& (*ostream_manipulator)(std::ostream&);
	inline TraceStreamer& operator<<(ostream_manipulator f)
	{
		if (f == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::endl)
		{
			msg += "\n";
		}
		return *this;
	}

	inline TraceStreamer& operator<<(std::_Setprecision __f)
	{
		precision(__f._M_n);
		return *this;
	}

	inline TraceStreamer& operator<<(std::_Setw __f)
	{
		width(__f._M_n);
		return *this;
	}

#if TRACE_STREAMER_TEMPLATE
	template<typename T>
	inline TraceStreamer& operator<<(const T& t)
	{
#if DEBUG_FORCED
		std::cerr << "WARNING: " << __PRETTY_FUNCTION__ << " TEMPLATE CALLED: Consider implementing a function with this signature!" << std::endl;
#endif
		if (enabled)
		{
			std::stringstream s;
			s << t;
			msg += s.str();
		}
		return *this;
	}
#endif
};

inline TraceStreamer& operator<<(TraceStreamer& x, cet::exception y)
{
	if (x.enabled) { x.msg += y.what(); }
	return x;
}
inline TraceStreamer& operator<<(TraceStreamer& x, std::atomic<unsigned long> const& a)
{
	if (x.enabled)
	{
		x.format(false, true, "l", x.flags());
		x.args[x.argCount].u = a.load();
		x.argCount++;
	}
	return x;
}
inline TraceStreamer& operator<<(TraceStreamer& x, std::atomic<short int> const& a)
{
	if (x.enabled)
	{
		x.format(false, false, "h", x.flags());
		x.args[x.argCount].i = a.load();
		x.argCount++;
	}
	return x;
}
inline TraceStreamer& operator<<(TraceStreamer& x, std::atomic<bool> const& a)
{
	return x << a.load();
}

#endif /* TRACEMF2_H */
#endif
