/* This file (tracelibmf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracelibmf.hh,v $
 // rev="$Revision: 430 $$Date: 2015-12-08 11:34:52 -0600 (Tue, 08 Dec 2015) $";
 */
#ifndef TRACELIBMF_H
#define TRACELIBMF_H

#include "messagefacility/MessageLogger/MessageLogger.h"	// LOG_DEBUG
// Use this define!  -- trace.h won't define it's own version of TRACE_LOG_FUNCTION
// The TRACE macro will then use the TRACE_MF_LOGGER macro defined below for the "slow"
// tracing function (if appropriate mask bit is set :)
# define TRACE_LOG_FUNCTION(tvp,tid,lvl,...)  TRACE_MF_LOGGER( lvl,__VA_ARGS__ )
#include "tracelib.h"		/* TRACE */

#include <string>

/* TRACE_MF_LOGGER is a macro because LOG_* output the file and line number.
   Example (__FILE__=some/path/V1495Driver_generator.cc): TRACE( 3, "V1495Driver::resume_" );
   becomes:
Fri Apr 18 11:55:38 -0500 2014: %MSG-i V1495Driver_generator:  BoardReader-dsfr6-5440 MF-online 
Fri Apr 18 11:55:38 -0500 2014: V1495Driver::resume_
Fri Apr 18 11:55:38 -0500 2014: %MSG
*/
#define TRACE_MF_LOGGER( lvl, ... ) do			\
    {							\
	char	        obuf[8192];			\
	snprintf( obuf, sizeof(obuf), __VA_ARGS__ );	\
	std::string category( __FILE__ );		\
	int off=category.rfind('/')+1;			\
	int len=category.rfind('.')-off;		\
	std::string cat( category.substr(off,len) );	\
	switch (lvl)					\
	{						\
	case 0: ::mf::LogError(   cat ) << obuf; break;	\
	case 1: ::mf::LogWarning( cat ) << obuf; break;	\
	case 2: ::mf::LogWarning( cat ) << obuf; break;	\
	case 3: ::mf::LogWarning( cat ) << obuf; break;	\
	case 4: ::mf::LogInfo(    cat ) << obuf; break;	\
	case 5: ::mf::LogInfo(    cat ) << obuf; break;	\
	case 6: ::mf::LogInfo(    cat ) << obuf; break;	\
	case 7: ::mf::LogInfo(    cat ) << obuf; break;	\
	case 8:     LOG_TRACE(    cat ) << obuf; break;	\
	case 9:     LOG_DEBUG(    cat ) << obuf; break;	\
	case 10:    LOG_DEBUG(    cat ) << obuf; break;	\
	default:    LOG_DEBUG(    cat ) << obuf; break;	\
	}						\
    } while (0)

#endif /* TRACELIBMF_H */
