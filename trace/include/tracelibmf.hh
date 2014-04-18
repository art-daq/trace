/* This file (tracelibmf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracelibmf.hh,v $
 // rev="$Revision: 1.3 $$Date: 2014/04/18 19:29:37 $";
 */
#ifndef TRACELIBMF_H
#define TRACELIBMF_H

#include "messagefacility/MessageLogger/MessageLogger.h"	// LOG_DEBUG
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
	case 3: ::mf::LogInfo(    cat ) << obuf; break;	\
	case 4: ::mf::LogInfo(    cat ) << obuf; break;	\
	case 5: ::mf::LogInfo(    cat ) << obuf; break;	\
	case 6:     LOG_DEBUG(    cat ) << obuf; break;	\
	case 7:     LOG_DEBUG(    cat ) << obuf; break;	\
	case 8:     LOG_TRACE(    cat ) << obuf; break;	\
	case 9:     LOG_DEBUG(    cat ) << obuf; break;	\
	case 10:    LOG_DEBUG(    cat ) << obuf; break;	\
	default:    LOG_DEBUG(    cat ) << obuf; break;	\
	}						\
    } while (0)

#endif /* TRACELIBMF_H */
