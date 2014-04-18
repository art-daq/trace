/* This file (tracelibmf.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: tracelibmf.hh,v $
 // rev="$Revision: 1.1 $$Date: 2014-04-18 16:00:41 $";
 */
#ifndef TRACELIBMF_H
#define TRACELIBMF_H

#include "messagefacility/MessageLogger/MessageLogger.h"	// LOG_DEBUG
# define TRACE_LOG_FUNCTION(tvp,tid,lvl,...)  TRACE_MF_LOGGER( lvl,__VA_ARGS__ )
#include "tracelib.h"		/* TRACE */

#include <string>

/* TRACE_MF_LOGGER is a macro because Log* output the file and line number.
*/
#define TRACE_MF_LOGGER( lvl, ... ) do \
    {					       \
	char	        obuf[8192];\
	snprintf( obuf, sizeof(obuf), __VA_ARGS__ );\
	std::string category( __FILE__ "Lvl" # lvl );	\
	int offset=category.rfind('/');                 \
        category.replace(category.rfind('.'),1,"_");    \
        char const *cat=category.c_str()+offset+1;      \
	switch (lvl)					\
	{						\
	case 0:    ::mf::LogError(   cat ) << obuf; break;	\
	case 1:    ::mf::LogWarning( cat ) << obuf; break;	\
	case 2:    ::mf::LogWarning( cat ) << obuf; break;	\
	case 3:    ::mf::LogInfo(    cat ) << obuf; break;	\
	case 4:    ::mf::LogInfo(    cat ) << obuf; break;	\
	case 5:    ::mf::LogInfo(    cat ) << obuf; break;	\
	case 6:    LOG_DEBUG(   cat ) << obuf; break;	\
	case 7:    LOG_DEBUG(   cat ) << obuf; break;	\
	case 8:    LOG_TRACE(   cat ) << obuf; break;	\
	case 9:    LOG_DEBUG(   cat ) << obuf; break;	\
	case 10:   LOG_DEBUG(   cat ) << obuf; break;	\
	default:   LOG_DEBUG(   cat ) << obuf; break;	\
	}						\
    } while (0)

#endif /* TRACELIBMF_H */
