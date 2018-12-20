/* This file (tracemf3.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // rev="$Revision: 552 $$Date: 2017-01-27 12:32:23 -0600 (Fri, 27 Jan 2017) $";
 */
#ifndef TRACEMF3_H
#define TRACEMF3_H

#include "messagefacility/MessageLogger/MessageLogger.h"  // LOG_DEBUG
// Use this define!  -- trace.h won't define it's own version of TRACE_LOG_FUNCTION
// The TRACE macro will then use the TRACE_MF_LOGGER macro defined below for the "slow"
// tracing function (if appropriate mask bit is set :)
#include "trace.h" /* TRACE */

#include <string>

/* TRACE_MF_LOGGER is a macro because LOG_* output the file and line number.
   Example (__FILE__=some/path/V1495Driver_generator.cc): TRACE( 3, "V1495Driver::resume_" );
   becomes:
Fri Apr 18 11:55:38 -0500 2014: %MSG-i V1495Driver_generator:  BoardReader-dsfr6-5440 MF-online
Fri Apr 18 11:55:38 -0500 2014: V1495Driver::resume_
Fri Apr 18 11:55:38 -0500 2014: %MSG
*/
#define TRACE_MF_LOGGER(lvl, ...)               \
  do {                                          \
    char obuf[8192];                            \
    snprintf(obuf, sizeof(obuf), __VA_ARGS__);  \
    std::string category(__FILE__);             \
    int off = category.rfind('/') + 1;          \
    int len = category.rfind('.') - off;        \
    std::string cat(category.substr(off, len)); \
    switch (lvl) {                              \
      case 0:                                   \
        ::mf::LogError(cat) << obuf;            \
        break;                                  \
      case 1:                                   \
        ::mf::LogWarning(cat) << obuf;          \
        break;                                  \
      case 2:                                   \
        ::mf::LogWarning(cat) << obuf;          \
        break;                                  \
      case 3:                                   \
        ::mf::LogWarning(cat) << obuf;          \
        break;                                  \
      case 4:                                   \
        ::mf::LogInfo(cat) << obuf;             \
        break;                                  \
      case 5:                                   \
        ::mf::LogInfo(cat) << obuf;             \
        break;                                  \
      case 6:                                   \
        ::mf::LogInfo(cat) << obuf;             \
        break;                                  \
      case 7:                                   \
        ::mf::LogInfo(cat) << obuf;             \
        break;                                  \
      case 8:                                   \
        LOG_TRACE(cat) << obuf;                 \
        break;                                  \
      case 9:                                   \
        LOG_DEBUG(cat) << obuf;                 \
        break;                                  \
      case 10:                                  \
        LOG_DEBUG(cat) << obuf;                 \
        break;                                  \
      default:                                  \
        LOG_DEBUG(cat) << obuf;                 \
        break;                                  \
    }                                           \
  } while (0)

/* Now define a trace macro that uses the "trigger mask" as a "3rd function" mask
 */
#undef TRACE
#define TRACE(lvl, ...)                                                                                             \
  do {                                                                                                              \
    TRACE_INIT_CHECK {                                                                                              \
      struct timeval lclTime;                                                                                       \
      lclTime.tv_sec = 0;                                                                                           \
      /* 1st "function" is memory */                                                                                \
      if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[traceTID].M & TLVLMSK(lvl))) {                           \
        trace(&lclTime, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED, __VA_ARGS__);                              \
      }                                                                                                             \
      /* 2nd "function" is console */                                                                               \
      if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[traceTID].S & TLVLMSK(lvl))) {                           \
        if (lclTime.tv_sec == 0) gettimeofday(&lclTime, NULL);                                                      \
        TRACE_LOG_FUNCTION(&lclTime, traceTID, lvl, "", __FILE__, __LINE__, TRACE_NARGS(__VA_ARGS__), __VA_ARGS__); \
      }                                                                                                             \
      /* 3rnd "function" is network */                                                                              \
      if (traceControl_rwp->mode.mode & (1 << 2) && (traceNamLvls_p[traceTID].T & TLVLMSK(lvl))) {                  \
        TRACE_MF_LOGGER(lvl, __VA_ARGS__);                                                                          \
      }                                                                                                             \
    }                                                                                                               \
  } while (0)

#endif /* TRACEMF3_H */
