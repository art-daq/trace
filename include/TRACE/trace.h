/* This file (trace.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: trace.h,v $
 */
#ifndef TRACE_H
#define TRACE_H

#define TRACE_REV "$Revision: 1613 $$Date: 2024-01-19 12:15:35 -0600 (Fri, 19 Jan 2024) $"

// The C++ streamer style macros...............................................
/*
      TLOG() << "hello";
      TLOG(TLVL_INFO) << "hello";
	  TLOG("name",lvl) << "a param is: " << std::hex << param;
 */

#ifdef __cplusplus

/* clang-format off */
//  This group takes 0, 1 or 2 optional args: Name, and/or FormatControl; in any order.
//  Name is a const char* or std::string&
//  FormatControl is an int:  0 - format if slow enabled
//                           >0 - streamer format even if just fast/mem (useful if "%" is in format)
//                           <0 - sprintf format
#	define TLOG_ERROR(...)   TRACE_STREAMER(TLVL_ERROR,  TLOG2(__VA_ARGS__), TSTREAMER_SL_FRC(TLVL_ERROR))
#	define TLOG_WARNING(...) TRACE_STREAMER(TLVL_WARNING,TLOG2(__VA_ARGS__), TSTREAMER_SL_FRC(TLVL_WARNING))
#	define TLOG_INFO(...)    TRACE_STREAMER(TLVL_INFO,   TLOG2(__VA_ARGS__), TSTREAMER_SL_FRC(TLVL_INFO))
#	define TLOG_TRACE(...)   TRACE_STREAMER(TLVL_TRACE,  TLOG2(__VA_ARGS__), TSTREAMER_SL_FRC(TLVL_TRACE))

//  This group takes 0, 1, 2, or 3 optional args: Level, and/or Name, and/or FormatControl
//  Name is same as above, but FormatControl is either false (format if slow) or true (format even if just fast/mem)
//  Level - an int or TLVL_* enum -- 0 to 55 for TLOG_DEBUG/TLOG_DBG
//                                   0 to 63 for TLOG/TLOG_ARB
//  TLOG_DEBUG and TLOG_DBG are duplicates as are TLOG and TLOG_ARB
#	define TLOG_DEBUG(...)   TRACE_STREAMER(0,   TLOG_DEBUG3(__VA_ARGS__),   TSTREAMER_SL_FRC(_trc_.lvl))
#	define TLOG_DBG(...)     TRACE_STREAMER(0,   TLOG_DEBUG3(__VA_ARGS__),   TSTREAMER_SL_FRC(_trc_.lvl))
#	define TLOG(...)         TRACE_STREAMER(0,   TLOG3(__VA_ARGS__),         TSTREAMER_SL_FRC(_trc_.lvl))
#	define TLOG_ARB(...)     TRACE_STREAMER(0,   TLOG3(__VA_ARGS__),         TSTREAMER_SL_FRC(_trc_.lvl))

#  if __cplusplus >= 201703L

/*  Log entering and leaving/returning from method/functions.
    This macro takes 0, 1, 2 or 3 optional args: Level (default is 42 for enter, 43 for exit),
    name (mainly useful for use in header files), and/or FormatControl.
    Note: the exit level is 1 greater than enter level (unless >=55).
    Use:
    TLOG_ENTEX();
 or TLOG_ENTEX() << __PRETTY__FUNCTION__;
 or TLOG_ENTEX() << __PRETTY__FUNCTION__ << " p1=" << p1;
 or etc.
	The use of TSTREAMER_T_ in this macro allows for 1) default lvl processing and 2) different ent/ex lvls.
    If a common return variable is returned (e.g. return retval;), the following could be done:
    ...
    TLOG_DEBUG(42) << "Enter - p1=" << p1;
    int retval; TRACE_EXIT { TLOG_DEBUG(43) << "Exit - retval=" << retval; };
    ...
	NOTE: with simple concatenation of command statements (end at ';'), the TLOG_DEBUG3 and TLOG3
	method must copy any name argument (which may come from a tempary std::string
	arg, created and destroy as a function call arg, at the end of a command statement (end at ';').
 */
#   ifndef TLOG_ENTEX_DBGLVL
#		define TLOG_ENTEX_DBGLVL 42
#	endif
#   define TLOG_ENTEX(...) \
	TSTREAMER_T_ TRACE_VARIABLE(_trc_)((tlvle_t)0, TRACE_GET_STATIC()); \
	TRACE_VARIABLE(_trc_).TLOG_DEBUG3(__VA_ARGS__); \
	TRACE_VARIABLE(_trc_).lvl = (tlvle_t)((int)TRACE_VARIABLE(_trc_).lvl-TLVL_DEBUG); \
	if (   TRACE_VARIABLE(_trc_).lvl==0								\
	       && (TRACE_VARIABLE(_trc_).TLOG3(__VA_ARGS__),TRACE_VARIABLE(_trc_).lvl)==TLVL_LOG ) /* use TLOG3 to detect no lvl entered */ \
		TRACE_VARIABLE(_trc_).lvl = (tlvle_t)TLOG_ENTEX_DBGLVL; \
	TRACE_EXIT { TLOG_DEBUG(TRACE_VARIABLE(_trc_).lvl+1,TRACE_VARIABLE(_trc_).tn,(bool)TRACE_VARIABLE(_trc_).flgs.fmtnow) << "Exit"; }; \
	TLOG_DEBUG(TRACE_VARIABLE(_trc_).lvl,TRACE_VARIABLE(_trc_).tn,(bool)TRACE_VARIABLE(_trc_).flgs.fmtnow) << "Enter "

#  endif // __cplusplus >= 201703L

#endif

// The C/C++ printf style macros...............................................
/*
   TRACE(TLVL_DEBUG, "this is an int: %d or 0x%08x", intvar, intvar );
   TRACEN("example",TLVL_DEBUG, "this is an int: %d or 0x%08x", intvar, intvar );
 */

#define TRACE(lvl, ...)                                                                                                               \
	do {																\
		struct { char tn[TRACE_TN_BUFSZ]; } _trc_;						\
		if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn))) { \
			trace_tv_t lclTime;                                                                                                   \
			uint8_t lvl_ = (uint8_t)(lvl);								\
			TRACE_SBUFDECL;                                                                                                                \
			lclTime.tv_sec = 0;                                                                                                       \
			if (traceControl_rwp->mode.bits.M && (traceLvls_p[traceTID].M & TLVLMSK(lvl_))) {  \
				/* Note: CANNOT add to "...NARGS..." (i.e. for long doubles issue) b/c nargs==0 in mem entry is signficant */ \
				trace(&lclTime, traceTID, lvl_, __LINE__, __func__/*NULL*/, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED, __VA_ARGS__); \
			}                                                                                                                         \
			if (traceControl_rwp->mode.bits.S && (traceLvls_p[traceTID].S & TLVLMSK(lvl_))) {  \
				TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime) {				\
					TRACE_LOG_FUNCTION(&lclTime, traceTID, lvl_, _insert, __FILE__, __LINE__, __func__, TRACE_NARGS(__VA_ARGS__), __VA_ARGS__); \
				}                                                                                                                     \
			}                                                                                                                         \
		}                                                                                                                             \
	} while (0)

#define TRACEN(nam, lvl, ...)											\
	do {																\
		struct { char tn[TRACE_TN_BUFSZ];	} _trc_;					\
		if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn))) { \
			static TRACE_THREAD_LOCAL int tid_ = -1;				\
			trace_tv_t lclTime;											\
			uint8_t lvl_ = (uint8_t)(lvl);								\
			TRACE_SBUFDECL;													\
			if (tid_ == -1) tid_ = trace_tlog_name_(&(nam)[0],__TRACE_FILE__,__FILE__,_trc_.tn,sizeof(_trc_.tn)); \
			lclTime.tv_sec = 0;											\
			if (traceControl_rwp->mode.bits.M && (traceLvls_p[tid_].M & TLVLMSK(lvl_))) { \
				/* Note: CANNOT add to "...NARGS..." (i.e. for long doubles issue) b/c nargs==0 in mem entry is signficant */ \
				trace(&lclTime, tid_, lvl_, __LINE__, __func__, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED, __VA_ARGS__);	\
			}                                                                                                                     \
			if (traceControl_rwp->mode.bits.S && (traceLvls_p[tid_].S & TLVLMSK(lvl_))) { \
				TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime) {				\
					TRACE_LOG_FUNCTION(&lclTime, tid_, lvl_, _insert, __FILE__, __LINE__, __func__, TRACE_NARGS(__VA_ARGS__), __VA_ARGS__); \
				}                                                                                                                 \
			}                                                                                                                     \
		}                                                                                                                         \
	} while (0)


#ifndef TRACE_LVL_ENUM_0_9
/* Note: these should match values in the bitN_to_mask script */
#	define TRACE_LVL_ENUM_0_9 TLVL_FATAL= 0, TLVL_EMERG= TLVL_FATAL, TLVL_ALERT, TLVL_CRIT, TLVL_ERROR, TLVL_WARNING, \
		TLVL_WARN= TLVL_WARNING, TLVL_NOTICE, TLVL_INFO, TLVL_LOG, TLVL_DEBUG, TLVL_DBG= TLVL_DEBUG, TLVL_DEBUG_1, TLVL_TRACE= TLVL_DEBUG_1
#endif
#ifndef TRACE_LVL_ENUM_10_63
/* Use to fill out the enum to the proper range (0-63) so C++ -Wconversion will warn when out-of-range */
/* At some point, these may be used to produce a string which is parsed to automatically become the LVLSTRS (below) */
/* There currently is a desire to have short enums (e.g. TLVL_D03), but TLVL_DBG+3 may do for the time being */
#	define TRACE_LVL_ENUM_10_63 TLVL_DEBUG_2, TLVL_DEBUG_3, TLVL_DEBUG_4, TLVL_DEBUG_5, TLVL_DEBUG_6, TLVL_DEBUG_7,	\
		TLVL_DEBUG_8, TLVL_DEBUG_9, TLVL_DEBUG_10, TLVL_DEBUG_11, TLVL_1DEBUG_2, TLVL_DEBUG_13, TLVL_DEBUG_14, TLVL_DEBUG_15, \
		TLVL_DEBUG_16, TLVL_DEBUG_17, TLVL_DEBUG_18, TLVL_DEBUG_19, TLVL_DEBUG_20, TLVL_DEBUG_21, TLVL_DEBUG_22, TLVL_DEBUG_23, \
		TLVL_DEBUG_24, TLVL_DEBUG_25, TLVL_DEBUG_26, TLVL_DEBUG_27, TLVL_DEBUG_28, TLVL_DEBUG_29, TLVL_DEBUG_30, TLVL_DEBUG_31, \
		TLVL_DEBUG_32, TLVL_DEBUG_33, TLVL_DEBUG_34, TLVL_DEBUG_35, TLVL_DEBUG_36, TLVL_DEBUG_37, TLVL_DEBUG_38, TLVL_DEBUG_39, \
		TLVL_DEBUG_40, TLVL_DEBUG_41, TLVL_DEBUG_42, TLVL_DEBUG_43, TLVL_DEBUG_44, TLVL_DEBUG_45, TLVL_DEBUG_46, TLVL_DEBUG_47, \
		TLVL_DEBUG_48, TLVL_DEBUG_49, TLVL_DEBUG_50, TLVL_DEBUG_51, TLVL_DEBUG_52, TLVL_DEBUG_53, TLVL_DEBUG_54, TLVL_DEBUG_55
#endif
enum tlvle_t { TRACE_LVL_ENUM_0_9, TRACE_LVL_ENUM_10_63 };

/* clang-format on */

// A Control macro.............................................................
/*
   TRACE_CNTL("modeM",0); // "freeze"
 */
// See traceCntl below for list of command strings and arguments.
#define TRACE_CNTL(...)           traceCntl(TRACE_NAME,__FILE__,TRACE_NARGS(__VA_ARGS__),__VA_ARGS__) /* Note: cannot be used when TRACE_NAME is macro which has _trc_.tn unless compatible structure is introduced in the surrounding code. */



//###########################  the detail below  ##############################

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

// clang-format off
#define TRACE_REVx $_$Revision: 1613 $_$Date: 2024-01-19 12:15:35 -0600 (Fri, 19 Jan 2024) $
// Who would ever have an identifier/token that begins with $_$???
#define $_$Revision  0?0
#define $_$Date      ,
#define TRACE_REV_FROM_REVb( revnum, ... ) revnum
#define TRACE_REV_FROM_REV(...) TRACE_REV_FROM_REVb( __VA_ARGS__ )
#define TRACE_REVNUM  TRACE_REV_FROM_REV(TRACE_REVx)
// clang-format on
// Example:
#if TRACE_REVNUM == 1320
//#warning "put whatever here"
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifndef __KERNEL__

#	include <ctype.h>                                                /* isspace, isgraph */
#	include <errno.h>                                                /* errno */
#	include <fcntl.h>                                                /* open, O_RDWR */
#	include <fnmatch.h>                                              /* fnmatch */
#	include <limits.h>                                               /* PATH_MAX */
#	include <stdarg.h>                                               /* va_list */
#	include <stdint.h>                                               /* uint64_t */
#	include <stdio.h>                                                /* printf, __GLIBC_PREREQ */
#	include <stdlib.h>                                               /* getenv, setenv, strtoul */
#	include <string.h>                                               /* strncmp */
#	include <sys/mman.h>                                             /* mmap */
#	include <sys/stat.h>                                             /* fstat */
#	include <sys/time.h>                                             /* timeval */
#	include <sys/uio.h>                                              /* struct iovec */
#	include <time.h>                                                 /* struct tm, localtime_r, strftime */
#	include <unistd.h>                                               /* lseek */
#	include <strings.h>                                              /* rindex */
#	define TMATCHCMP(pattern, str_) (fnmatch(pattern, str_, 0) == 0) /*MAKE MACRO RETURN TRUE IF MATCH*/
/*# define TMATCHCMP(needle,haystack)   strstr(haystack,needle)*/     /*MAKE MACRO RETURN TRUE IF MATCH*/

#	if defined(__CYGWIN__)
#		include <windows.h>
static inline pid_t trace_gettid(void) { return GetCurrentThreadId(); }
static inline int trace_getcpu(void) { return GetCurrentProcessorNumber(); }
#	else
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#			pragma GCC diagnostic push
#			ifndef __cplusplus
#				pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#			endif
#			pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#		endif
#		include <sys/syscall.h> /* syscall */
#		if defined(__sun__)
#			define TRACE_GETTID SYS_lwp_self
static inline int trace_getcpu(void) { return 0; }
#		elif defined(__APPLE__)
#			define TRACE_GETTID SYS_thread_selfid
static inline int trace_getcpu(void) { return 0; }
#		else /* assume __linux__ */
#			define TRACE_GETTID __NR_gettid
#			include <sched.h> /* sched_getcpu - does vsyscall getcpu */
static inline int trace_getcpu(void) { return sched_getcpu(); }
#		endif
static inline pid_t trace_gettid(void)
{
	return (pid_t)syscall(TRACE_GETTID);
}
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#			pragma GCC diagnostic pop
#		endif
#	endif

#	ifndef PATH_MAX
#		define PATH_MAX 1024 /* conservative */
#	endif
#	ifdef __cplusplus
#		include <string>
#		include <sstream>   /* std::ostringstream */
#		include <iostream>  // cerr
#		include <iomanip>
#	endif

/* this first check is for Darwin 15 */
#	if defined(__cplusplus) && (__cplusplus == 201103L) && defined(__apple_build_version__) && defined(__clang_major__) && (__clang_major__ == 7)
#		include <atomic> /* atomic<> */
#		include <memory> /* std::unique_ptr */
#		define TRACE_ATOMIC_T               std::atomic<uint32_t>
#		define TRACE_ATOMIC_INIT            ATOMIC_VAR_INIT(0)
#		define TRACE_ATOMIC_LOAD(ptr)       atomic_load(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#		define TRACE_THREAD_LOCAL
#	elif defined(__cplusplus) && (__cplusplus >= 201103L)
#		include <atomic> /* atomic<> */
#		include <memory> /* std::unique_ptr */
#		define TRACE_ATOMIC_T               std::atomic<uint32_t>
#		define TRACE_ATOMIC_INIT            ATOMIC_VAR_INIT(0)
#		define TRACE_ATOMIC_LOAD(ptr)       atomic_load(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#		define TRACE_THREAD_LOCAL           thread_local
#	elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && (defined(__clang__) || (defined __GNUC__ && defined __GNUC_MINOR__ && (10000 * __GNUC__ + 1000 * __GNUC_MINOR__) >= 49000))
#		define TRACE_C11_ATOMICS
#		include <stdatomic.h> /* atomic_compare_exchange_weak */
#		define TRACE_ATOMIC_T               /*volatile*/ _Atomic(uint32_t)
#		define TRACE_ATOMIC_INIT            ATOMIC_VAR_INIT(0)
#		define TRACE_ATOMIC_LOAD(ptr)       atomic_load(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#		define TRACE_THREAD_LOCAL           _Thread_local
#	elif defined(__x86_64__) || defined(__i686__) || defined(__i386__)
#		define TRACE_ATOMIC_T               uint32_t
#		define TRACE_ATOMIC_INIT            0
#		define TRACE_ATOMIC_LOAD(ptr)       *(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) *(ptr)= val
#		define TRACE_THREAD_LOCAL
static inline uint32_t cmpxchg(uint32_t *ptr, uint32_t old, uint32_t new_)
{
	uint32_t __ret;
	uint32_t __old= (old);
	uint32_t __new= (new_);
	volatile uint32_t *__ptr= (volatile uint32_t *)(ptr);
	__asm__ volatile("lock cmpxchgl %2,%1"
					 : "=a"(__ret), "+m"(*__ptr)
					 : "r"(__new), "0"(__old)
					 : "memory");
	return (__ret);
}
#	elif defined(__sparc__)
/* Sparc, as per wikipedia 2016.01.11, does not do compare-and-swap (CAS).
   I could move the DECL stuff up, define another spinlock so that sparc could work in
   the define/declare environment. In this case, the module static TRACE_NAME feature could not
   be used. */
struct trace_atomic {
	uint32_t lck;
	uint32_t val;
};
#		define TRACE_ATOMIC_T              struct trace_atomic  // clang-format off
#		define TRACE_ATOMIC_INIT           {0}  // clang-format on
#		define TRACE_ATOMIC_LOAD(ptr)      (ptr)->val
#		define TRACE_ATOMIC_STORE(ptr, vv) (ptr)->val= vv
#		define TRACE_THREAD_LOCAL
static inline uint32_t xchg_u32(__volatile__ uint32_t *m, uint32_t val)
{
	__asm__ __volatile__("swap [%2], %0"
						 : "=&r"(val)
						 : ""(val), "r"(m)
						 : "memory");
	return val;
}
static inline uint32_t cmpxchg(TRACE_ATOMIC_T *ptr, uint32_t exp, uint32_t new_)
{
	uint32_t old;
	while (xchg_u32(&ptr->lck, 1) != 0)
		; /* lock */
	old= ptr->val;
	if (old == exp) ptr->val= new_;
	ptr->lck= 0; /* unlock */
	return (old);
}
#	else /* userspace arch */
/* THIS IS A PROBLEM (older compiler on unknown arch) -- I SHOULD PROBABLY #error */
#		define TRACE_ATOMIC_T               uint32_t
#		define TRACE_ATOMIC_INIT            0
#		define TRACE_ATOMIC_LOAD(ptr)       *(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) *(ptr)= val
#		define TRACE_THREAD_LOCAL
#		define cmpxchg(ptr, old, new) \
			({ uint32_t old__ = *(ptr); if(old__==(old)) *ptr=new; old__; }) /* THIS IS A PROBLEM -- NEED OS MUTEX HELP :( */
#	endif        /* userspace arch */

#	define TRACE_GETTIMEOFDAY(tvp) gettimeofday(tvp, NULL)
/* Note: anonymous variadic macros were introduced in C99/C++11 (maybe C++0x) */
#	define TRACE_STRTOL(...)       strtol(__VA_ARGS__)
#	define TRACE_PRN(...)          printf(__VA_ARGS__)
#	define TRACE_VPRN(...)         vprintf(__VA_ARGS__)
#	define TRACE_EPRN(...)         fprintf(stderr, __VA_ARGS__)
#	define TRACE_INIT_CHECK(nn)    ((traceTID != -1) || (traceInit(nn, 0) == 0)) /* See note by traceTID decl/def below */

typedef struct timeval trace_tv_t;

#else /* __KERNEL__ */

#	include <linux/ktime.h>                                      /* do_gettimeofday */
/*# include <linux/printk.h>	         printk, vprintk */
#	include <linux/kernel.h>                                     /* printk, vprintk */
#	include <linux/mm.h>                                         /* kmalloc OR __get_free_pages */
#	include <linux/vmalloc.h>                                    /* __vmalloc, vfree */
#	include <linux/spinlock.h>                                   /* cmpxchg */
#	include <linux/sched.h>                                      /* current (struct task_struct *) */
#	include <linux/time.h>                                       /* struct timeval */
#	include <linux/ctype.h>                                      /* isgraph */
#	include <linux/version.h>                                    /* KERNEL_VERSION */
/*# define TMATCHCMP(pattern,str_)         (strcmp(pattern,str_)==0)*/ /*MAKE MACRO RETURN TRUE IF MATCH*/
#	define TMATCHCMP(needle, haystack)  strstr(haystack, needle) /*MAKE MACRO RETURN TRUE IF MATCH*/
#	define TRACE_ATOMIC_T               uint32_t
#	define TRACE_ATOMIC_INIT            0
#	define TRACE_ATOMIC_LOAD(ptr)       *(ptr)
#	define TRACE_ATOMIC_STORE(ptr, val) *(ptr)= val
#	define TRACE_THREAD_LOCAL
#	if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 1)
#		define TRACE_GETTIMEOFDAY(tvp) ({struct timespec64 ts; ktime_get_real_ts64(&ts); (tvp)->tv_sec=ts.tv_sec; (tvp)->tv_usec=ts.tv_nsec/1000; })
#	else
#		define TRACE_GETTIMEOFDAY(tvp) do_gettimeofday(tvp)
#	endif
#	define TRACE_STRTOL(...)    simple_strtoul(__VA_ARGS__)
#	define TRACE_PRN(...)       printk(__VA_ARGS__)
#	define TRACE_VPRN(...)      vprintk(__VA_ARGS__)
#	define TRACE_EPRN(...)      printk(KERN_ERR __VA_ARGS__)
#	define TRACE_INIT_CHECK(nn) ((traceTID != -1) || ((traceTID= trace_name2TID(nn)) != -1))
#	ifndef MODULE
int trace_3_init(void);
int trace_3_proc_add(int);
int trace_3_sched_switch_hook_add(void); /* for when compiled into kernel */
#	endif
static inline int trace_getcpu(void)
{
	return raw_smp_processor_id();
}

#	if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
typedef struct __kernel_old_timeval trace_tv_t;
#	else
typedef struct timeval trace_tv_t;
#	endif

#endif /* __KERNEL__ */


#define TRACE_MIN(a, b) (((a) < (b)) ? (a) : (b))

/* A trace (TLOG or TRACE) from a header before and trace from the compilation unit (__BASE_FILE__) is mess things up */
#if defined(__GNUC__) || defined(__clang__)
#	define __TRACE_FILE__ __BASE_FILE__
#else
#	define __TRACE_FILE__ __FILE__
#endif

#if (defined(__clang_major__) && (__clang_major__ >= 4)) || (defined __GNUC__ && defined __GNUC_MINOR__ && (10000 * __GNUC__ + 1000 * __GNUC_MINOR__) >= 49000)
#	define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#else
#	define ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

/* the width used when printing out Process/task or Thread IDs, etc. */
#ifdef __APPLE__
#	define TRACE_TID_WIDTH 7
#else
#	define TRACE_TID_WIDTH 7
#endif
#define TRACE_CPU_WIDTH     3
#define TRACE_LINENUM_WIDTH 4

/* these were originally just used in the mmap function */
#define TRACE_STR(...)  TRACE_STR2(__VA_ARGS__)
#define TRACE_STR2(...) #__VA_ARGS__

/* Maximum UDP Datagram Data Length */
#ifndef TRACE_STREAMER_MSGMAX            /* allow test program to try different values */
#	define TRACE_STREAMER_MSGMAX 0x2000 /* 0x3400 seems to work for artdaq, 0x3800 does not. */
	                                     /* 65507 is way too much for when TraceStreamer is static thread_local */
#endif
#ifndef TRACE_USER_MSGMAX            /* allow test program to try different values */
#	define TRACE_USER_MSGMAX 0x1800
#endif
/* 88,7=192 bytes/ent   96,6=192   128,10=256  192,10=320 */
#define TRACE_DFLT_MAX_MSG_SZ  192
#define TRACE_DFLT_MAX_PARAMS  10
#define TRACE_DFLT_NAMTBL_ENTS 1022 /* this is for creating new trace_buffer file -- it currently <= the */
                                    /* "trace DISABLED" number that fits into traceControl[1-2] (see below) */
#define TRACE_DFLT_NAM_CHR_MAX 63   /* Really the hardcoded max name len. Name buffers should be +1 (for null */
                                    /* terminator). See: env -i ${TRACE_BIN}/trace_cntl info | grep namLvlTbl_ents */
	/* with "trace DISBALED". Search for trace_created_init(...) call in "DISABLE" case. */
	/* Names can have this many characters (and always be null terminated - so names can be printed from nam tbl) */

#define TRACE_TN_BUFSZ         128  /* hardcoded size of buffer used for creating "trace name." I've arbitrarily */
	                                /* imposed that this should/must be a multiple of 8. If 2048 is used (uber */
	/* ridiculous number as I think 128 is ridiculous), a module build warning occurs: */
	/*                     warning: the frame size of 2080 bytes is larger than 2048 bytes [-Wframe-larger-than=] */

#define TRACE_DFLT_NUM_ENTRIES 500000
#define TRACE_DFLT_TIME_FMT    "%m-%d %H:%M:%S.%%06d" /* match default in trace_delta.pl */
#ifndef TRACE_DFLT_NAME
#	define TRACE_DFLT_NAME        "%f %H"
#endif
#define TRACE_DFLT_LVLS        ((1ULL << (TLVL_DEBUG + 0)) - 1) /* non-debug for slow path -- NOT ERS COMPAT (ERS has */
	                                                     /* DEBUG_0 enabled by default, but I think "debug is debug") */
#define TRACE_DFLT_LVLM        ((1ULL << (TLVL_DEBUG + 1)) - 1) /* first lvl of debug and below on for fast/mem path */

#if !defined(TRACE_NAME)
static const char *TRACE_NAME= NULL; /* basically a flag which will indicate whether or not #define TRACE_NAME is present */
#endif

#if !defined(__KERNEL__) || (defined(__KERNEL__) && defined(TRACE_IMPL))  // if userspace OR kernel init environment (i.e. not sub module)
#	if defined(TRACE_PRINT) // must be defined before static code is compiled; can be undef after. NOTE: not used in any macro(s)
static const char *TRACE_PRINT__= TRACE_PRINT; /* Msg Limit Insert will have separator */
#	else                                       /* NOTE: kernel header kernel/trace/trace.h uses enum trace_type { ... TRACE_PRINT, ... } */
static const char *TRACE_PRINT__= "%T %*n %*L %F: %M"; /* Msg Limit Insert will have separator */
#	endif
#endif

/* 64bit sparc (nova.fnal.gov) has 8K pages (ref. ~/src/sysconf.c). This
											 (double) is no big deal on systems with 4K pages; more important (effects
											 userspace mapping) when actual 8K pages.
											 Cygwin uses 64K pages. */
#define TRACE_PAGESIZE  0x10000
#define TRACE_CACHELINE 64

#ifdef __GNUC__
#	define SUPPRESS_NOT_USED_WARN __attribute__((__unused__))
#else
#	define SUPPRESS_NOT_USED_WARN
#endif

#define TLVLBITSMSK ((sizeof(uint64_t) * 8) - 1)
#define TLVLMSK(xx) (1ULL << ((xx)&TLVLBITSMSK))

/* For C, these should go at the beginning of a block b/c they define new local variables */
#if defined(TRACE_NO_LIMIT_SLOW) /* || defined(__KERNEL__) */
#	define TRACE_LIMIT_SLOW(lvl, ins, tvp) \
		char ins[1]= {'\0'};                \
		if (1)
#else
#	define TRACE_LIMIT_SLOW(lvl, ins, tvp)                                                  \
		char ins[32];                                                                        \
		static TRACE_THREAD_LOCAL limit_info_t _info= {/*TRACE_ATOMIC_INIT,*/ 0, lsFREE, 0}; \
		if (trace_limit_do_print(tvp, &_info, ins, sizeof(ins)))
#endif

/* helper for TRACEing strings in C - ONLY in C (and not in KERNEL, which could use C90 compiler) */
#if defined(__KERNEL__)
#	define TRACE_SBUFDECL /* some kernel compiles complain: ISO C90 forbids variable length array 'tsbuf__' */
#elif defined(__cplusplus)
/* don't want to instantiate an std::vector as it may cause alloc/delete */
#	define TRACE_SBUFDECL /*std::vector<char> tsbuf__(traceControl_p->siz_msg);*/ /*&(tsbuf__[0])*/
/*# define TRACE_SBUFSIZ__     tsbuf__.size()*/
#else
#	define TRACE_SBUFDECL                                                \
		char tsbuf__[traceControl_p->siz_msg + 1] SUPPRESS_NOT_USED_WARN; \
		tsbuf__[0]= '\0'
#	define TRACE_SBUFSIZ__ sizeof(tsbuf__)
/* The following is what is to be optionally used: i.e.: TRACE(2,TSPRINTF("string=%s store_int=%%d",string),store_int);
   Watch for the case where string has an imbedded "%" */
#	define TSPRINTF(...)   (tsbuf__[0] ? &(tsbuf__[0]) : (snprintf(&(tsbuf__[0]), TRACE_SBUFSIZ__, __VA_ARGS__), &(tsbuf__[0])))
#endif


#if defined(__cplusplus)

// NOTE: I use the gnu extension __PRETTY_FUNCTION__ as opposed to __func__ b/c in C++ a method/function can have different arguments
// clang-format off

#	if defined(TRACE_STD_STRING_FORMAT)

// NOTE: No delayed formatting - except for 6 (or 7): time, lvl, pid, tid, cpu, line (and tsc).
// IFF I can figure out a _ss = fmt::vformat( msg, ap ), then I can create a static void trace_do_fmt( x,y,z, __VA_ARGS__ )
#		define TRACEF(lvl, ...)													\
	do {																\
		struct { char tn[TRACE_TN_BUFSZ];	} _trc_;					\
		if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn))) { \
			static TRACE_THREAD_LOCAL limit_info_t _info = {/*TRACE_ATOMIC_INIT,*/ 0, lsFREE, 0}; \
			trace_tv_t lclTime;                                                                                                   \
			uint8_t lvl_ = (uint8_t)(lvl);								\
			char _ins[32];                                                                         \
			std::string _ss; \
			lclTime.tv_sec = 0;                                                                                                       \
			bool do_m = traceControl_rwp->mode.bits.M && (traceLvls_p[traceTID].M & TLVLMSK(lvl_)); \
			bool do_s = traceControl_rwp->mode.bits.S && (traceLvls_p[traceTID].S & TLVLMSK(lvl_)) && trace_limit_do_print(&lclTime, &_info, _ins, sizeof(_ins)); \
			if (do_m || do_s) {											\
				_ss = TRACE_STD_STRING_FORMAT( __VA_ARGS__ );						\
				if (do_m) trace(&lclTime, traceTID, lvl_, __LINE__, __PRETTY_FUNCTION__, 0 TRACE_XTRA_PASSED, _ss.c_str()); \
				if (do_s) TRACE_LOG_FUNCTION(&lclTime, traceTID, lvl_, _ins, __FILE__, __LINE__, __PRETTY_FUNCTION__, 0, _ss.c_str() ); \
			}														\
		}                                                                                                                             \
	} while (0)

#		define TRACEFN(nam, lvl, ...)                                                                                                     \
	do {																\
		struct { char tn[TRACE_TN_BUFSZ];	} _trc_;					\
		if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn))) { \
			static TRACE_THREAD_LOCAL int tid_ = -1;                                                                              \
			static TRACE_THREAD_LOCAL limit_info_t _info = {/*TRACE_ATOMIC_INIT,*/ 0, lsFREE, 0}; \
			trace_tv_t lclTime;                                                                                               \
			uint8_t lvl_ = (uint8_t)(lvl);								\
			char _ins[32];                                                                         \
			std::string _ss; \
			lclTime.tv_sec = 0;                                                                                                   \
			if (tid_ == -1) tid_ = trace_tlog_name_(&(nam)[0],__TRACE_FILE__,__FILE__,_trc_.tn,sizeof(_trc_.tn));			\
			bool do_m = traceControl_rwp->mode.bits.M && (traceLvls_p[tid_].M & TLVLMSK(lvl_)); \
			bool do_s = traceControl_rwp->mode.bits.S && (traceLvls_p[tid_].S & TLVLMSK(lvl_)) && trace_limit_do_print(&lclTime, &_info, _ins, sizeof(_ins)); \
			if (do_m || do_s) {											\
				_ss = TRACE_STD_STRING_FORMAT( __VA_ARGS__ );						\
				if (do_m) trace(&lclTime, tid_, lvl_, __LINE__, __PRETTY_FUNCTION__, 0 TRACE_XTRA_PASSED, _ss.c_str()); \
				if (do_s) TRACE_LOG_FUNCTION(&lclTime, tid_, lvl_, _ins, __FILE__, __LINE__, __PRETTY_FUNCTION__, 0, _ss.c_str() ); \
			}														\
		}                                                                                                                             \
	} while (0)

#	endif /* TRACE_STD_STRING_FORMAT */

/* Note: This supports using a mix of stream syntax and format args, i.e: "string is " << some_str << " and float is %f", some_float
   Note also how the macro evaluates the first part (the "FMT") only once
   no matter which destination ("M" and/or "S") is active.
   Note: "xx" in TRACE_ARGS_1ST(__VA_ARGS__,xx) is just a dummy arg to that macro.
   THIS IS DEPRECATED. It is nice to have for comparison tests.
*/
#	define TRACEN_(nam, lvl, ...)										\
	do {																\
		struct { char tn[TRACE_TN_BUFSZ];	} _trc_;					\
		if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn))) { \
			static TRACE_THREAD_LOCAL int tid_ = -1;				\
			static TRACE_THREAD_LOCAL limit_info_t _info = {/*TRACE_ATOMIC_INIT,*/ 0, lsFREE, 0}; \
			trace_tv_t lclTime;											\
			uint8_t lvl_ = (uint8_t)(lvl);								\
			char _ins[32];												\
			if (tid_ == -1) tid_ = trace_tlog_name_(&(nam)[0],__TRACE_FILE__,__FILE__,_trc_.tn,sizeof(_trc_.tn));			\
			lclTime.tv_sec = 0;											\
			bool do_m = traceControl_rwp->mode.bits.M && (traceLvls_p[tid_].M & TLVLMSK(lvl_));	\
			bool do_s = traceControl_rwp->mode.bits.S && (traceLvls_p[tid_].S & TLVLMSK(lvl_)) && trace_limit_do_print(&lclTime, &_info, _ins, sizeof(_ins)); \
			if (do_s || do_m) {											\
				std::ostringstream ostr__; /*instance creation is heavy weight*/ \
				ostr__ << TRACE_ARGS_1ST(__VA_ARGS__, xx);				\
				/* Note: CANNOT add to "...NARGS..." (i.e. for long doubles issue) b/c nargs==0 in mem entry is signficant */ \
				if (do_m) trace(&lclTime, tid_, lvl_, __LINE__, __PRETTY_FUNCTION__, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED, ostr__.str() TRACE_ARGS_ARGS(__VA_ARGS__)); \
				if (do_s) TRACE_LOG_FUNCTION(&lclTime, tid_, lvl_, _ins, __FILE__, __LINE__, __PRETTY_FUNCTION__, TRACE_NARGS(__VA_ARGS__), ostr__.str().c_str() TRACE_ARGS_ARGS(__VA_ARGS__)); \
			}															\
		}															\
	} while (0)

#endif /* defined(___cplusplus) */

/* TRACE_NARGS configured to support 0 - 35 args */
#define TRACE_NARGS(...)          TRACE_NARGS_HELP1(__VA_ARGS__,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0) /* 0 here but not below */
#define TRACE_NARGS_HELP1(...)    TRACE_NARGS_HELP2(__VA_ARGS__,unused) /* "unused" to avoid warning "requires at least one argument for the "..." in a variadic macro" */
#define TRACE_NARGS_HELP2(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n,...) n
#define TRACE_ARGS_1ST(first,...) first
// clang-format on
/* TRACE_ARGS_ARGS(...) ignores the 1st arg (the "format" arg) and returns the remaining "args", if any.
   Being able
   The trick is: the number of args in __VA_ARGS__ "shifts" the appropriate XX*(__VA_ARGS__) macro
   to the DO_THIS postition in the TRACE_DO_XX macro. Then only that appropriate XX*(__VA_ARGS__) macro is
   evalutated; the others are ignored. The only 2 choices are TRACE_XXX_X() or TRACE_XXX_0(); TRACE_XXX_X is for when there
   is between 1 and 35 args and TRACE_XXX_0 is for 0 args. */
#define TRACE_ARGS_ARGS(...)                                                                  \
	TRACE_DO_XX(__VA_ARGS__,                                                                  \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), \
				TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_X(__VA_ARGS__), TRACE_XXX_0(__VA_ARGS__), unused) /* "unused" to avoid warning "requires at least one argument for the "..." in a variadic macro" */
#define TRACE_DO_XX(fmt, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31, x32, x33, x34, x35, do_this, ...) do_this
#define TRACE_XXX_0(A)
#define TRACE_XXX_X(A, ...) , __VA_ARGS__

#if defined(__CYGWIN__) /* check this first as __x86_64__ will also be defined */

#	define TRACE_XTRA_PASSED
#	define TRACE_XTRA_UNUSED
#	define TRACE_PRINTF_FMT_ARG_NUM 7
#	define TRACE_VA_LIST_INIT(addr) (va_list) addr
#	define TRACE_ENT_TV_FILLER
static inline uint64_t rdtsc(void) { uint32_t eax, edx; __asm__ __volatile__("rdtsc\n\t": "=a" (eax), "=d" (edx)); return (uint64_t)eax | (uint64_t)edx << 32; } /*NOLINT*/
#	define TRACE_TSC32(low) low = rdtsc()

#elif defined(__i386__)

#	define TRACE_XTRA_PASSED
#	define TRACE_XTRA_UNUSED
#	define TRACE_PRINTF_FMT_ARG_NUM 7
#	define TRACE_VA_LIST_INIT(addr) (va_list) addr
#	define TRACE_ENT_TV_FILLER      uint32_t x[2];
static inline uint64_t rdtsc(void) { uint32_t eax, edx; __asm__ __volatile__("rdtsc\n\t": "=a" (eax), "=d" (edx)); return (uint64_t)eax | (uint64_t)edx << 32; } /*NOLINT*/
#	define TRACE_TSC32(low) low = rdtsc()

#elif defined(__x86_64__)

/*  NOTE: may have to pass extra unused arguments to get proper stack alignment for args, specifically long double */
#	define TRACE_XTRA_PASSED        , 0, .0, .0, .0, .0, .0, .0, .0, .0
#	define TRACE_XTRA_UNUSED        , long l1 __attribute__((__unused__)), double d0 __attribute__((__unused__)), double d1 __attribute__((__unused__)), double d2 __attribute__((__unused__)), double d3 __attribute__((__unused__)), double d4 __attribute__((__unused__)), double d5 __attribute__((__unused__)), double d6 __attribute__((__unused__)), double d7 __attribute__((__unused__))
#	define TRACE_PRINTF_FMT_ARG_NUM 16  // clang-format off
#	define TRACE_VA_LIST_INIT(addr) { { 6*8, 6*8 + 8*16, addr, addr } } // clang-format of
#	define TRACE_ENT_TV_FILLER
#	ifdef __KERNEL__
#	 define TRACE_TSC32(low) low = rdtsc()
#	else
//static inline uint64_t rdtsc(void) { uint32_t eax, edx; __asm__ __volatile__("rdtsc\n\t": "=a" (eax), "=d" (edx)); return (uint64_t)eax | (uint64_t)edx << 32; } /*NOLINT*/
#    include <x86intrin.h>
#	 define TRACE_TSC32(low) low = _rdtsc()
#	endif

#elif defined(__powerpc__) && !defined(__powerpc64__)

#	define TRACE_XTRA_PASSED , 0, .0, .0, .0, .0, .0, .0, .0, .0
#	define TRACE_XTRA_UNUSED , long l1 __attribute__((__unused__)), double d0 __attribute__((__unused__)), double d1 __attribute__((__unused__)), double d2 __attribute__((__unused__)), double d3 __attribute__((__unused__)), double d4 __attribute__((__unused__)), double d5 __attribute__((__unused__)), double d6 __attribute__((__unused__)), double d7 __attribute__((__unused__))
#	define TRACE_PRINTF_FMT_ARG_NUM 16 // clang-format off
#	define TRACE_VA_LIST_INIT(addr) { { 8, 8, 0, addr } }  // clang-format on
#	define TRACE_ENT_TV_FILLER      uint32_t x[2];
#	define TRACE_TSC32(low)

#elif defined(__aarch64__)

#       ifdef __KERNEL__  /* __aarch64__, by default, doesn't like floating point in the kernel */
#	 define TRACE_XTRA_PASSED , 0
#	 define TRACE_XTRA_UNUSED , long l1 __attribute__((__unused__))
#	 define TRACE_PRINTF_FMT_ARG_NUM 8 // clang-format off
#       else
#	 define TRACE_XTRA_PASSED , 0, .0, .0, .0, .0, .0, .0, .0, .0
#	 define TRACE_XTRA_UNUSED , long l1 __attribute__((__unused__)), double d0 __attribute__((__unused__)), double d1 __attribute__((__unused__)), double d2 __attribute__((__unused__)), double d3 __attribute__((__unused__)), double d4 __attribute__((__unused__)), double d5 __attribute__((__unused__)), double d6 __attribute__((__unused__)), double d7 __attribute__((__unused__))
#	 define TRACE_PRINTF_FMT_ARG_NUM 16 // clang-format off
#       endif 
#	define TRACE_VA_LIST_INIT(addr) { addr }  // clang-format on
#	define TRACE_ENT_TV_FILLER
#	define TRACE_TSC32(low)

#elif defined(__arm__)

#	define TRACE_VA_LIST_INIT(addr) { addr }  // clang-format on
#	if defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 4
		/* need to assure arguments pushed on stack start on an 8 byte aligned address */
#		define TRACE_XTRA_PASSED , 0
#		define TRACE_XTRA_UNUSED , long l1 __attribute__((__unused__))
#		define TRACE_PRINTF_FMT_ARG_NUM 8  // clang-format off
#		define TRACE_ENT_TV_FILLER uint32_t x[2];
#	else
#		define TRACE_XTRA_PASSED 
#		define TRACE_XTRA_UNUSED 
#		define TRACE_PRINTF_FMT_ARG_NUM 7  // clang-format off
#		define TRACE_ENT_TV_FILLER
#	endif
#	define TRACE_TSC32(low)

#else

#	define TRACE_XTRA_PASSED
#	define TRACE_XTRA_UNUSED
#	define TRACE_PRINTF_FMT_ARG_NUM 7  // clang-format off
#	define TRACE_VA_LIST_INIT(addr) { addr }  // clang-format on
#	if defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 4
#		define TRACE_ENT_TV_FILLER uint32_t x[2];
#	else
#		define TRACE_ENT_TV_FILLER
#	endif
#	define TRACE_TSC32(low)

#endif

static void trace(trace_tv_t * /*tvp*/, int /*trcId*/, uint8_t /*lvl*/, int32_t /*line*/, const char * /*function*/, uint8_t /*nargs*/ TRACE_XTRA_UNUSED, const char * /*msg*/, ...) __attribute__((format(printf, TRACE_PRINTF_FMT_ARG_NUM, TRACE_PRINTF_FMT_ARG_NUM + 1)));
#ifdef __cplusplus
static void trace(trace_tv_t *, int, uint8_t, int32_t, const char *, uint8_t TRACE_XTRA_UNUSED, const std::string &, ...);
#endif

union trace_mode_u {
	struct
	{
		unsigned M : 1; /* b0 high speed circular Memory */
		unsigned S : 1; /* b1 printf (formatted) to Screen/Stdout */
		unsigned reserved : 14;
		int func : 2; /* func ==> 1=force on, 0=TRACE_PRINT, -1=force off */
		unsigned fast_do_getcpu : 1;	/* some archs (e.g. __arm__), this is not (Feb, 2021) vDSO */
	} bits;
	struct
	{
		uint16_t mode;
		uint16_t cntl;
	} words;
};

typedef char trace_vers_t[sizeof(int64_t) * 16];

struct traceControl_rw {
	/* the goal is to have wrIdxCnt in it's own cache line */
	TRACE_ATOMIC_T wrIdxCnt; /* 32 bits */
	uint32_t cacheline1[TRACE_CACHELINE / sizeof(int32_t) - (sizeof(TRACE_ATOMIC_T) / sizeof(int32_t))];

	/* the goal is to have namelock in it's own cache line */
	TRACE_ATOMIC_T namelock; /* 32 bits */
	uint32_t cacheline2[TRACE_CACHELINE / sizeof(int32_t) - (sizeof(TRACE_ATOMIC_T) / sizeof(int32_t))];

	union trace_mode_u mode;
	uint32_t reserved0;  /* use to be trigOffMode */
	uint32_t trigIdxCnt; /* BASED ON "M" mode Counts */
	int32_t triggered;
	uint32_t trigActivePost;
	int32_t full;
	uint32_t limit_span_on_ms; /* 4 billion ms is 49 days */
	uint32_t limit_span_off_ms;
	uint32_t limit_cnt_limit;
	uint32_t longest_name;                                 /* helps with trace_user if printing names */
	uint32_t xtra[TRACE_CACHELINE / sizeof(int32_t) - 10]; /* force some sort of alignment -- taking into account - */
	/* - the 6 fields (above) since the last cache line alignment */
};
struct traceControl_s {
	trace_vers_t version_string;
	uint32_t version;                                                                                      /*  1 */
	uint32_t num_params;                                                                                   /*  2 */
	uint32_t siz_msg;                                                                                      /*  3 */
	uint32_t siz_entry;                                                                                    /*  4 */
	uint32_t num_entries;                                                                                  /*  5 */
	uint32_t largest_multiple;                                                                             /*  6 */
	uint32_t num_namLvlTblEnts;                                                                            /*  7 */
	volatile int32_t trace_initialized;                                                                    /*  8 these and above would be read only if */
	uint32_t memlen;                                                                                       /*  9 in kernel */
	uint32_t create_tv_sec;                                                                                /* 10 */
	uint32_t largest_zero_offset;                                                                          /* 11 */
	uint32_t nam_arr_sz;                                                                                   /* 12 */
	uint32_t page_align[TRACE_PAGESIZE / sizeof(int32_t) - (12 + sizeof(trace_vers_t) / sizeof(int32_t))];
	               /* allow mmap 1st page(s) (stuff above) readonly */

	/* "page" break */
	struct traceControl_rw rw;
};
/* clang-format off *//*          bytes  TRACE_SHOW cntl char */
struct traceEntryHdr_s          /*-----   -------        */
{
	trace_tv_t time;            /* 16        T */
	TRACE_ENT_TV_FILLER /* because timeval is larger on x86_64 (16 bytes compared to 8 for i686) */

	uint64_t tsc;               /*  8        t */
	pid_t pid;                  /*  4        P system info */
	pid_t tid;                  /*  4        i system info - "thread id" */

	int32_t cpu;                /*  4  %3u   C -- kernel sched switch will indicate this info? */
	uint32_t linenum;           /*  4  %5u   u */
	int32_t TrcId;              /*  4  %4u   I Trace ID ==> idx into lvlTbl, namTbl */
	uint8_t get_idxCnt_retries; /*  1  %1u   R */
	uint8_t nargs;              /*  1  %4u   # */
	uint8_t lvl;                /*  1  %2d   L or l */
	uint8_t param_bytes;        /*  1  %1u   B */
	/*char msg[0];                             See "msg_p = (char *)(myEnt_p + 1)" below. Note: warning: ISO C++ forbids zero-size array */
};                              /* ---       M -- NO, ALWAY PRINTED LAST! formated Message */
/* msg buf,then params buf               48   adding uint32_t line;char file[60] (another cache line) doesn't seem worth it */
/* see TRACE_entSiz(siz_msg,num_params) and idxCnt2entPtr(idxCnt) */ /* other - N  index */
/* clang-format on */

struct traceLvls_s {			/* formerly traceNamLvls_s */
	uint64_t M;
	uint64_t S;
	uint64_t T;
	/*char name[TRACE_DFLT_NAM_CHR_MAX + 1];*/
};

struct trace_vtrace_cntl_s {
	unsigned prepend_func : 1; /* is %F in TRACE_PRINT; but see union trace_mode_u .func above */
	char sep[16];              /* arbitrary limit on separator characters */
};

#ifndef TRACE_8_PRINT_FD /* all must be given (for macro; env var (TRACE_PRINT_FD) is different) */
#	define TRACE_8_PRINT_FD 1, 1, 1, 1, 1, 1, 1, 1
#endif
#ifndef TRACE_56_PRINT_FD /* all must be given (for macro; env var (TRACE_PRINT_FD) is different) */
#	define TRACE_56_PRINT_FD 1, 1, 1, 1, 1, 1, 1, 1,                         \
							  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
							  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
							  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
#endif

#ifndef TRACE_8_LVLSTRS
#	define TRACE_8_LVLSTRS "FATAL", "ALERT", "CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "LOG"
#endif
#ifndef TRACE_56_LVLSTRS
#	define TRACE_56_LVLSTRS "DEBUG", "DEBUG_1", "DEBUG_2", "DEBUG_3", "DEBUG_4", "DEBUG_5", "DEBUG_6", "DEBUG_7",           \
							 "DEBUG_8", "DEBUG_9", "DEBUG_10", "DEBUG_11", "DEBUG_12", "DEBUG_13", "DEBUG_14", "DEBUG_15",   \
							 "DEBUG_16", "DEBUG_17", "DEBUG_18", "DEBUG_19", "DEBUG_20", "DEBUG_21", "DEBUG_22", "DEBUG_23", \
							 "DEBUG_24", "DEBUG_25", "DEBUG_26", "DEBUG_27", "DEBUG_28", "DEBUG_29", "DEBUG_30", "DEBUG_31", \
							 "DEBUG_32", "DEBUG_33", "DEBUG_34", "DEBUG_35", "DEBUG_36", "DEBUG_37", "DEBUG_38", "DEBUG_39", \
							 "DEBUG_40", "DEBUG_41", "DEBUG_42", "DEBUG_43", "DEBUG_44", "DEBUG_45", "DEBUG_46", "DEBUG_47", \
							 "DEBUG_48", "DEBUG_49", "DEBUG_50", "DEBUG_51", "DEBUG_52", "DEBUG_53", "DEBUG_54", "DEBUG_55"
#endif
#ifndef TRACE_LVLWIDTH
#	define TRACE_LVLWIDTH 8
#endif

/*--------------------------------------------------------------------------*/
/* Enter the 5 use case "areas" -- see doc/5in1.txt                         */
/*  defining TRACE_DEFINE wins over DECLARE or nothing. STATIC wins over all */
/* It's OK if one module has DEFINE and all the reset have any of (nothing, DECLARE, STATIC) */
/* The only thing that is invalid is if more than one has DEFINE.           */
/* If any have DECLARE, then there must be one (and only one) with DEFINE.  */
#if defined(TRACE_STATIC) && !defined(__KERNEL__)
#	define TRACE_DECL(var_type, var_name, arrDim, initializer) static var_type var_name arrDim initializer
#elif defined(TRACE_DEFINE) && !defined(__KERNEL__)
#	define TRACE_DECL(var_type, var_name, arrDim, initializer) \
		extern var_type var_name arrDim;                        \
		var_type var_name arrDim initializer
#elif defined(TRACE_DEFINE) && defined(__KERNEL__)
#	define TRACE_DECL(var_type, var_name, arrDim, initializer) \
		var_type var_name arrDim initializer;                   \
		EXPORT_SYMBOL_GPL(var_name)
#elif defined(TRACE_DECLARE) || defined(__KERNEL__)
#	define TRACE_DECL(var_type, var_name, arrDim, initializer) extern var_type var_name arrDim
#else
#	define TRACE_DECL(var_type, var_name, arrDim, initializer) static var_type var_name arrDim initializer
#endif

/*#define TRACE_THREAD_LOCALX TRACE_THREAD_LOCAL    * use this for separate FILE per thread -- very rare; perhaps NUMA issue??? */
#define TRACE_THREAD_LOCALX

TRACE_DECL(struct traceControl_s, traceControl, [3], ); /* for when TRACE is disabled. NOTE: traceLvls_p should always point to traceControl_p+1 */
TRACE_DECL(TRACE_THREAD_LOCALX struct traceControl_s *, traceControl_p, , = NULL);
TRACE_DECL(TRACE_THREAD_LOCALX struct traceControl_rw *, traceControl_rwp, , = NULL);
TRACE_DECL(TRACE_THREAD_LOCALX struct traceLvls_s *, traceLvls_p, , = (struct traceLvls_s *)&traceControl[1]);
TRACE_DECL(TRACE_THREAD_LOCALX char                  *, traceNams_p, , = NULL);
TRACE_DECL(TRACE_THREAD_LOCALX struct traceEntryHdr_s *, traceEntries_p, , );
static TRACE_THREAD_LOCAL int traceTID= -1; /* idx into lvlTbl, namTbl -- always
												static (never global) and possibly
thread_local -- this will cause the most traceInit calls (but not much work, hopefully),
which will ensure that a module (not thread) can be assigned it's own trace name/id. */

TRACE_DECL(uint64_t, trace_lvlS, , = 0);
TRACE_DECL(uint64_t, trace_lvlM, , = 0);
TRACE_DECL(const char *, tracePrint_cntl, , = NULL); /* hardcoded default below that can only be overridden via env.var */
// clang-format off
#define TRACE_VTRACE_CNTL_INIT {0,{':',' ',0}}				/* this is needed to hide the comma in first pass preprocessing to _not_ get wrong number of macro arguments error */
TRACE_DECL(struct trace_vtrace_cntl_s,trace_vtrace_cntl,, = TRACE_VTRACE_CNTL_INIT); /* b0 is if "function" is in tracePrint_cntl OR should there be a "global flag" in
												   traceControl_rw??? behaving like TRACE_LIMIT_MS??? -- __func__ is prepended to msg */
#define TRACE_LVLSTRS_INIT {TRACE_8_LVLSTRS, TRACE_56_LVLSTRS} /* This is needed to hide the comma in first pass preprocessing to _not_ get wrong number of macro arguments error */
typedef char (trace_lvlstrs_t)[64][16];						   /* For implementing N aliases */
enum { trace_lvlstrs_aliases=6 };
TRACE_DECL(trace_lvlstrs_t, trace_lvlstrs,[trace_lvlstrs_aliases], = {TRACE_LVLSTRS_INIT});
TRACE_DECL(unsigned, trace_lvlwidth,, = TRACE_LVLWIDTH);
#ifndef TRACE_LVLCOLORS_INIT
#	define TRACE_LVLCOLORS_INIT {{"[31m","[0m"},{"[31m","[0m"},{"[31m","[0m"},{"[31m","[0m"},\
		{"[93m","[0m"},{"[93m","[0m"},{"[32m","[0m"},{"[32m","[0m"}} /* this is needed to hide the comma in first pass preprocessing to _not_ get wrong number of macro arguments error */
#endif
// clang-format on
/* [2] for 1)([0]) On, and 2)([1]) off; [24] to support \033[38:2:<r>:<g>:<b>m  eg \033[38:2:255:165:100m */
TRACE_DECL(char, trace_lvlcolors, [64][2][24], = TRACE_LVLCOLORS_INIT);

#if defined(__KERNEL__)
TRACE_DECL(int, trace_allow_printk, , = 0);                /* module_param */
TRACE_DECL(char, trace_print, [200], = {0});               /* module_param */
static TRACE_THREAD_LOCAL const char *traceName= "KERNEL"; /* can always be module static in the kernel */
#else
TRACE_DECL(TRACE_THREAD_LOCAL const char *, traceName, , = "TRACE");                 /* */
TRACE_DECL(TRACE_THREAD_LOCALX const char *, traceFile, , = "/tmp/trace_buffer_%u"); /*a local/efficient FS device is best; operation when path is on NFS device has not been studied*/
TRACE_DECL(TRACE_THREAD_LOCAL pid_t, traceTid, , = 0);                               /* thread id */
TRACE_DECL(pid_t, tracePid, , = 0);
TRACE_DECL(int, trace_no_pid_check, , = 0);
#	define TRACE_PRINT_FD_INIT TRACE_8_PRINT_FD, TRACE_56_PRINT_FD
TRACE_DECL(int, tracePrintFd, [64], = {TRACE_PRINT_FD_INIT});
TRACE_DECL(TRACE_ATOMIC_T, traceInitLck, , = TRACE_ATOMIC_INIT);
TRACE_DECL(uint32_t, traceInitLck_hung_max, , = 0);
TRACE_DECL(const char *, traceTimeFmt, , = NULL); /* hardcoded default below that can only be overridden via env.var */
static char traceFile_static[PATH_MAX]= {0};
static struct traceControl_s *traceControl_p_static= NULL;
#endif
/*--------------------------------------------------------------------------*/

/* forward declarations, important functions */
static struct traceEntryHdr_s *idxCnt2entPtr(uint32_t idxCnt);
static char *idx2namsPtr(int32_t TrcId);
#if !defined(__KERNEL__) || defined(TRACE_IMPL) /* K=0,IMPL=0; K=0,IMPL=1; K=1,IMPL=1 */
static int traceInit(const char *_name, int allow_ro);
static void traceInitNames(struct traceControl_s * /*tC_p*/, struct traceControl_rw * /*tC_rwp*/);
#	ifdef __KERNEL__                           /*                         K=1,IMPL=1 */
static int msgmax= TRACE_DFLT_MAX_MSG_SZ;       /* module_param */
static int argsmax= TRACE_DFLT_MAX_PARAMS;      /* module_param */
static int numents= TRACE_DFLT_NUM_ENTRIES;     /* module_param */
static int namtblents= TRACE_DFLT_NAMTBL_ENTS;  /* module_param */
static int namemax= TRACE_DFLT_NAM_CHR_MAX + 1; /* module_param */
static int trace_buffer_numa_node= -1;          /* module_param */
#	endif
#else /*                                         K=1,IMPL=0  */

#endif /*  __KERNEL__             TRACE_IMPL  */

static int64_t traceCntl(const char *_name, const char *_file, int nargs, const char *cmd, ...);
static uint32_t trace_name2TID(const char *nn);
#define TRACE_TID2NAME(idx)               idx2namsPtr(idx)
#define TRACE_cntlPagesSiz()              ((uint32_t)sizeof(struct traceControl_s))
#define TRACE_namtblSiz(ents, nam_sz)     (((uint32_t)(sizeof(struct traceLvls_s) + ((nam_sz + 7) & (unsigned)~7)) * (ents) + (unsigned)(TRACE_CACHELINE - 1)) & ~((unsigned)(TRACE_CACHELINE - 1)))
#define TRACE_entSiz(siz_msg, num_params) (uint32_t)(sizeof(struct traceEntryHdr_s) + sizeof(uint64_t) * (unsigned)(num_params) /* NOTE: extra size for i686 (32bit processors) */ \
													 + (unsigned)(siz_msg))
#define traceMemLen(siz_cntl_pages, num_namLvlTblEnts, nam_sz, siz_msg, num_params, num_entries) \
	(((siz_cntl_pages) + TRACE_namtblSiz(num_namLvlTblEnts, nam_sz) + TRACE_entSiz(siz_msg, num_params) * (num_entries) + (unsigned)(TRACE_PAGESIZE - 1)) & ~((unsigned)(TRACE_PAGESIZE - 1)))

/* The "largest_multiple" method (using (ulong)-1) allows "easy" "add 1"
   I must do the substract (ie. add negative) by hand.
   Ref. ShmRW class (~/src/ShmRW?)
   Some standards don't seem to inline "static inline"
   Use of the following seems to produce the same code as the optimized
   code which calls inline idxCnt_add (the c11/c++11 optimizer seem to do what
   this macro does).
   NOTE: when using macro version, "add" should be of type int32_t */
#if 1
#	define TRACE_IDXCNT_ADD(idxCnt, add)                                                                                     \
		(((add) < 0)                                                                                                          \
			 ? (((uint32_t) - (add) > (idxCnt))                                                                               \
					? (traceControl_p->largest_multiple - ((uint32_t) - (add) - (idxCnt))) % traceControl_p->largest_multiple \
					: ((idxCnt) - ((uint32_t) - (add))) % traceControl_p->largest_multiple)                                   \
			 : ((idxCnt) + (uint32_t)(add)) % traceControl_p->largest_multiple)
#else
static uint32_t TRACE_IDXCNT_ADD(uint32_t idxCnt, int32_t add)
{
	uint32_t retval;
	if (add < 0)
		if (-add > idxCnt)
			retval= (traceControl_p->largest_multiple - (-add - idxCnt)) % traceControl_p->largest_multiple;
		else
			retval= (idxCnt - (-add)) % traceControl_p->largest_multiple;
	else
		retval= (idxCnt + add) % traceControl_p->largest_multiple;
	return retval;
}
#endif
#define TRACE_IDXCNT_DELTA(cur, prv) \
	(((cur) >= (prv))                \
		 ? (cur) - (prv)             \
		 : (cur) - (prv)-traceControl_p->largest_zero_offset)

typedef void (*trace_log_function_type)(trace_tv_t *, int, uint8_t, const char *, const char *, int, const char *, uint16_t, const char *, ...);
#ifndef TRACE_LOG_FUNCTION
#	define TRACE_LOG_FUNCTION trace_user
#elif defined(TRACE_LOG_FUN_PROTO)
/* prototype for TRACE_LOG_FUNCTION as compiled in Streamer class below */
TRACE_LOG_FUN_PROTO;
#endif /* TRACE_LOG_FUNCTION */

/* Return non-NULL if found, else NULL
   This handles the case of %%<flag> which would be false positive if just
   searching for %<flag>.
 */
static inline const char *trace_strflg(const char *ospec, char flag)
{
	for (; *ospec; ++ospec) {
		if (*ospec == '%') {
			++ospec;
			while (*ospec <= '9' && *ospec >= '0') ++ospec; // Efficient - when *ospec is e.g. 'F', 1st test fails
			if (*ospec == '\0')
				break;
			if (*ospec == flag)
				return (ospec);
		}
	}
	return (NULL);
}

static uint32_t trace_lock(TRACE_ATOMIC_T *atomic_addr)
{
	uint32_t desired= 1, expect= 0, hung= 0;
#if defined(__KERNEL__)
	while (cmpxchg(atomic_addr, expect, desired) != expect)
		if (++hung > 100000000) break;
#elif (defined(__cplusplus) && (__cplusplus >= 201103L)) || defined(TRACE_C11_ATOMICS)
	while (!atomic_compare_exchange_weak(atomic_addr, &expect, desired)) {
		expect= 0;
		if (++hung > 100000000) {
			break;
		}
	}
	if (atomic_addr == &traceInitLck && traceInitLck_hung_max < hung) {
		traceInitLck_hung_max= hung;
	}
#else
	while (cmpxchg(atomic_addr, expect, desired) != expect)
		if (++hung > 100000000) break;
	if (atomic_addr == &traceInitLck && traceInitLck_hung_max < hung) traceInitLck_hung_max= hung;
#endif
	return ((hung <= 100000000) ? 1 : 0);
} /* trace_lock */

static void trace_unlock(TRACE_ATOMIC_T *atomic_addr)
{
#if defined(__KERNEL__)
	TRACE_ATOMIC_STORE(atomic_addr, (uint32_t)0);
#elif (defined(__cplusplus) && (__cplusplus >= 201103L)) || defined(TRACE_C11_ATOMICS)
	atomic_store(atomic_addr, (uint32_t)0);
#else
	TRACE_ATOMIC_STORE(atomic_addr, (uint32_t)0);
#endif
} /* trace_unlock */

typedef enum {
	lsFREE,
	lsLIMITED
} limit_state_t;

typedef struct
{
	/* choice: whole struct TLS or normal static with member:  TRACE_ATOMIC_T lock;*/
	uint64_t span_start_ms;
	limit_state_t state;
	uint32_t cnt;
} limit_info_t;

// n_additional_components -> if negative, then return whole path
SUPPRESS_NOT_USED_WARN
static const char *trace_path_components(const char *in_cp, int n_additional_components)
{
	const char *tmp_cp= in_cp + strlen(in_cp);
	if (n_additional_components < 0)
		return (in_cp);
	while (tmp_cp != in_cp) {
		if (*--tmp_cp == '/' && --n_additional_components == -1) {
			++tmp_cp;
			break;
		}
	}
	return (tmp_cp);
} /* trace_path_components */

SUPPRESS_NOT_USED_WARN
static const char *trace_name_path( const char* spec, const char*file, const char*hdrf, char*buf, size_t bufsz)
{
	char stop, special='%';
	char *obuf=buf;
	int  spec_off=1;/*, no_ext=0;;*/
	const char *ccp, *bn, *extp;
	size_t cpylen=0;
	int additional_path=0;
	if (strchr(spec,' ')) stop=' ';
	else                  stop='\0';
	--bufsz;					/* so I don't have to keep doing 'bufsz-1' */
	while(*spec != stop) {
		if(*spec != special) {
			*obuf++ = *spec;          /* NOT TERMINATED!!! */
			if (--bufsz == 0) break;  /* "goto out" */
			++spec;
		} else {
			switch (spec[spec_off]) {
			case '-': /*no_ext=1;*/ ++spec_off; continue;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (spec_off == 1) additional_path = spec[spec_off]&0xf;
				++spec_off;
				continue;
			case 'F':
				/*if (no_ext) goto no_ext;*/
				ccp=trace_path_components(file,additional_path);
				cpylen=TRACE_MIN(strlen(ccp),bufsz);
#	if __GNUC__ >= 8
#		pragma GCC diagnostic push
					// With -O3, I get warnings. I do not get warning if -O2 or -O0
#		pragma GCC diagnostic ignored "-Wstringop-truncation"
#	endif
				strncpy(obuf,ccp,cpylen); obuf+=cpylen;bufsz-=cpylen;
				break;
			case 'f':			/* without extension */
				/*no_ext:*/
				ccp=trace_path_components(file,additional_path);
				extp=strrchr(trace_path_components(ccp,0),'.');
				if (extp) cpylen=TRACE_MIN((size_t)(extp-ccp),bufsz);
				else      cpylen=TRACE_MIN(strlen(ccp),bufsz);
				strncpy(obuf,ccp,cpylen); obuf+=cpylen;bufsz-=cpylen;
				break;
			case 'H':
				ccp=trace_path_components(hdrf,additional_path);
				cpylen=TRACE_MIN(strlen(ccp),bufsz);
				strncpy(obuf,ccp,cpylen); obuf+=cpylen;bufsz-=cpylen;
				break;
			case 'h':			/* without extension */
				ccp=trace_path_components(hdrf,additional_path);
				extp=strrchr((bn=trace_path_components(ccp,0)),'.');
				if (extp) cpylen=TRACE_MIN((size_t)(extp-ccp),bufsz);
				else      cpylen=TRACE_MIN(strlen(ccp),bufsz);
				strncpy(obuf,ccp,cpylen); obuf+=cpylen;bufsz-=cpylen;
				break;
/*#	define TRACE_DO__PROGNAME*/
#	ifdef TRACE_DO__PROGNAME
			case 'p':
				{
					extern char *__progname;
					cpylen=TRACE_MIN(strlen(__progname),bufsz);
					strncpy(obuf,__progname,cpylen); obuf+=cpylen;bufsz-=cpylen;
				}
				break;
#	endif
#	if __GNUC__ >= 8
#		pragma GCC diagnostic pop
#	endif
			case '!':
				special='\0';
				break;
			case '%':
				*obuf++ = spec[spec_off];
				if (--bufsz == 0) goto out;
				break;
			case '\0': --spec_off; /* Fall Through */
			default:
				for (int uu=0;uu<=spec_off;++uu) {
					*obuf++ = spec[uu];
					if (--bufsz == 0) goto out;
				}					
			}
			//TRACE(TLVL_LOG,"%c cpylen=%ld",spec[spec_off], cpylen);
			spec+=spec_off+1; spec_off=1; /*no_ext=0;*/
			if (bufsz==0) break;
		}
	}
 out:
	*obuf = '\0';				/* make sure terminated */
	//TRACE(TLVL_LOG,buf);
	return (buf);
} /* trace_name_path */

#define TRACE_SNPRINTED(rr, ss)  ((((size_t)(rr) + 1) < (ss)) ? (size_t)(rr) : ((ss) ? (ss)-1 : 0)) /* TRICKY - rr is strlen and ss is sizeof. When ss is 0 or 1, it's strlen should be 0 */

/*  There are two recognized patterns:
    1) %%
	2) %[-][0-9]f
    1) produces and single '%'
    2) causes the "file" parameter to be process -- the "-" will remove the extension;
       the 0-9 will include that many additional path components (besides the base).
       If 0-9 is not given, the whole path will be included.
    If a pattern besides the 2 indicated occurs, it will be transferred to the output.
 */
SUPPRESS_NOT_USED_WARN
static const char *trace_name(const char *name, const char *file, char *buf, size_t bufsz)
{
	const char *ret;
	const char*spec;

#if !defined(__KERNEL__) && defined(TRACE_DEBUG_INIT)
	fprintf(stderr,"file=%s\n",file);
#endif
	// ASSUME ALWAYS main/base file -- Just call xxx -- it will determine if "name" has " "  ????
	if (name && *name) spec = name;
	else {
#ifndef __KERNEL__
		spec= getenv("TRACE_NAME");
		if (!(name && *name))
#endif
			spec= TRACE_DFLT_NAME;
	}
	ret = trace_name_path( spec, file, file, buf, bufsz );
	return ret;
} /* trace_name */

/*  Note: this function was originally developed for use in TLOG calls, but now
	is also used in the TRACEN* macros.
 */
SUPPRESS_NOT_USED_WARN
static int trace_tlog_name_(const char* given, const char *base_file, const char *FILEp, char *buf, size_t buflen)
{
	int ret;
	const char *spec;

	if (given && *given) {
		ret = (int)trace_name2TID(given);  /* THE MOST Efficient */
	} else {
		if (base_file==NULL) base_file="";
		if (FILEp    ==NULL) FILEp="";
		if (strcmp(base_file,FILEp) == 0)
			ret = traceTID;			/* in main/base file -- name already determined THIS IS AN IMPORTANT OPTIMISATION */
		else {						/* -- TRACE_NAME or env(TRACE_NAME) are not "dynamic" (at least not in BASE_FILE)!!! */
			const char *xx;
			// in included (header) file (NOT main/base file)
			// IF no ' ' then I could EITHER 1) pass DFLT_HNAME spec
			//                            OR 2) pass DFLT_NAME w/ base_file=FILEp (the only way to get "base..file" is by having ' ' in TRACE_NAME or getenv("TRACE_NAME")
			if (TRACE_NAME && *TRACE_NAME) {
				// need to check for ' '
				spec=TRACE_NAME;
			} else {
				spec= getenv("TRACE_NAME"); /* THIS IS/COULD BE costly :( -- THIS WILL HAPPENS (once) FOR EVERY TLOG/TRACEN in a header/included file!!! */
				if (spec && *spec) {
					// need to check for ' '
				} else spec= TRACE_DFLT_NAME; // no need to check for ' '   OR spec=TRACE_DFLT_NAME and base_file=FILEp
			}
			if ((xx=strchr(spec,' '))) {
				spec=xx+1;
				ret = (int)trace_name2TID(trace_name_path(spec,base_file,FILEp,buf,buflen));
			} else
				ret = (int)trace_name2TID(trace_name_path(spec,FILEp,FILEp,buf,buflen));
		}
	}
	return ret;
} /* trace_tlog_name_ */

/* if a "do print" (return true/1) and if insert is provided and sz
   is non-zero, it will be NULL terminated */
SUPPRESS_NOT_USED_WARN
static inline int trace_limit_do_print(trace_tv_t *tvp, limit_info_t *info, char *insert, size_t sz)
{
	uint64_t delta_ms, tnow_ms;
	int do_print;
	/*trace_tv_t tvx;
	  trace( &tvx, 125, 1, __LINE__, __func__, 1 TRACE_XTRA_PASSED, "trace_limit_do_print _cnt_=%u", traceControl_rwp->limit_cnt_limit );*/

	if (traceControl_rwp->limit_cnt_limit == 0) {
		if (insert && sz) {
			*insert= '\0';
		}
		return (1);
	}
	if (tvp->tv_sec == 0) {
		TRACE_GETTIMEOFDAY(tvp);
	}
	tnow_ms= (uint64_t)(tvp->tv_sec * 1000 + tvp->tv_usec / 1000);
	/* could lock  trace_lock( &(info->lock) );*/
	delta_ms= tnow_ms - info->span_start_ms;
	if (info->state == lsFREE) {
		if (delta_ms >= traceControl_rwp->limit_span_on_ms) { /* start new timespan */
			info->span_start_ms= tnow_ms;
			info->cnt= 1;
			if (insert && sz) {
				*insert= '\0';
			}
		} else if (++(info->cnt) >= traceControl_rwp->limit_cnt_limit) {
			if (insert) {
				strncpy(insert, "[RATE LIMIT]", sz);
				/*fprintf( stderr, "[LIMIT (%u/%.1fs) REACHED]\n", traceControl_rwp->limit_cnt_limit, (float)traceControl_rwp->limit_span_on_ms/1000000);*/
			}
			info->state= lsLIMITED;
			info->span_start_ms= tnow_ms; /* start tsLIMITED timespan */
			info->cnt= 0;
		} else if (insert && sz) { /* counting messages in this period */
			*insert= '\0';
		}
		do_print= 1;
	} else {                                                   /* state must be tsLIMITED */
		if (delta_ms >= traceControl_rwp->limit_span_off_ms) { /* done limiting, start new timespace */
			if (insert) {
				snprintf(insert, sz, "[RESUMING dropped: %u]", info->cnt);
			}
			info->state= lsFREE;
			info->span_start_ms= tnow_ms;
			info->cnt= 0;
			do_print= 1;
		} else {
			++(info->cnt);
			do_print= 0;
		}
	}
	/* unlock  trace_unlock( &(info->lock) );*/
	return (do_print);
} /* trace_limit_do_print */

/*
  Inspired by ERS/src/Context.cxx (formatting mine):
...
    void print_function( std::ostream & out, const char * function, int verbosity )
    {
        if ( verbosity ) { out << function; return; }
        const char * end = strchr( function, '(' );
        if ( end ) {
            const char * beg = end;
            while ( beg > function ) {
                if ( *(beg-1) == ' ' )
                    break;
                --beg;
            }
            out.write( beg, end - beg );
            out << "(...)";
        } else
            out << function;
    }
...

Below replaces a non-empty arg list with "(...)" and strips return type
i.e. "int func_0(T) [with T = int]" becomes "func_0(...)"
Ret val is a strlen val - does not include null ('\0') which is always
written (similar to snprintf). So, ret val will always be at least 1 less
than input "sz". strip_ns 
 */
static size_t trace_func_to_short_func(const char *in, char *out, size_t sz, int strip_ns)
{
	const char *cptr;
	int len;
	size_t ret;
	if ((cptr= strchr(in, '('))) {
		if (*(cptr + 1) == ')') {
			while (*--cptr != ' ' && (*cptr!=':' || strip_ns==0) && cptr != in)
				;
			if (*cptr == ' ' || (*cptr==':' && strip_ns==1)) ++cptr; /* if at space between return type and func; some (template) funcs do not) */
			ret = (size_t)snprintf(out, sz, "%s", cptr);
		} else {
			const char *endptr= cptr;
			while (*--cptr != ' ' && (*cptr!=':' || strip_ns==0) && cptr != in)
				;
			if (*cptr == ' ' || (*cptr==':' && strip_ns==1)) ++cptr; /* if at space between return type and func; some (template) funcs do not) */
			len= (int)(endptr - cptr);
			ret = (size_t)snprintf(out, sz, "%.*s(...)", len, cptr);
		}
	} else
		ret = (size_t)snprintf(out, sz, "%s", in);
	return TRACE_MIN(ret,sz-1);
} /* trace_func_to_short_func */

typedef char (trace_width_ca_t)[9];
static int trace_build_L_fmt(char *fmtbuf, char *altbuf, char *l3buf, char **lvl_cp, int *vwidth,
                             char *flags_ca, int *width_ia, trace_width_ca_t *width_ca)
{
	int retval;
	char *lvlcp = *lvl_cp;
	if (strchr(flags_ca,'#')) {
		int ii;
		for (ii=0; lvlcp[ii]; ++ii) altbuf[ii] = lvlcp[ii]^0x20;
		altbuf[ii]='\0';
		*lvl_cp=lvlcp=altbuf;
	}
	if (strchr(flags_ca,'*')) {
		strcpy(fmtbuf,"%*s");
		*vwidth = (int)trace_lvlwidth;
		retval = 1;
	} else {
		strcpy(fmtbuf, "%");
		if (strchr(flags_ca,'.')) {
			fmtbuf[1]= '.';
			fmtbuf[2]= '\0';
		}
		if (width_ca[0][0]) {
			strcat(&fmtbuf[1], width_ca[0]);
			/* fatal->ftl, alert->alr, crit->crt, error->err,warning->wrn, notice->ntc,
			   info->nfo, log->log, debug->dbg, debug_1-> d01 */
			if (width_ia[0] == 3) {
				size_t zz=strlen(lvlcp);
				if        (lvlcp[zz-1]>='0' && lvlcp[zz-1]<='9') {
					l3buf[0]=lvlcp[0];
					l3buf[2]=lvlcp[zz-1];
					if (lvlcp[zz-2]>='0' && lvlcp[zz-2]<='9') l3buf[1]=lvlcp[zz-2];
					else                                l3buf[1]='0';
				} else if ((lvlcp[0]|0x20)=='i') {
					l3buf[0]=lvlcp[1];l3buf[1]=lvlcp[2];l3buf[2]=lvlcp[3];/*assume info*/
				} else if (zz == 3){
					l3buf[0]=lvlcp[0];l3buf[1]=lvlcp[1];l3buf[2]=lvlcp[2];/*assume log*/
				} else {
					int ii,jj=1;
					l3buf[0]=lvlcp[0];
					/* now 2 more non-vowels */
					for (ii=0; ii<2; ++ii) {
						while ((strchr("aeiou",lvlcp[jj]|0x20))) ++jj;
						l3buf[ii+1]=lvlcp[jj++];
					}
				}
				l3buf[3]='\0';
				*lvl_cp = l3buf;
			}
		}
		strcat(&fmtbuf[1], "s");
		retval=0;
	}
	return retval;
} /* trace_build_L_fmt */

SUPPRESS_NOT_USED_WARN
static void vtrace_user(trace_tv_t *tvp, int TrcId, uint8_t lvl, const char *insert, const char *file, int line, const char *function, uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	   first. There are 2 main reasons that this is done:
	   1) allows the use of write to a specific tracePrintFd;
	   2) there will be one system call which is most efficient and less likely
	   to have the output mangled in a multi-threaded environment.
	*/
	char tbuf[0x100];
	int width_state, width_ia[3]= {0}; /* allow up to 3 widths w[.w[.w]] */
	trace_width_ca_t width_ca[3];               /* need to save the string for leading "0" (e.g. "-04"*/
	char flags_ca[4];                  /*  */
	size_t flags_sz;
	const char *default_unknown_sav;
	char *endptr;
	size_t printed= 0; /* does not include '\0' */
	const char *print_cntl;
	//size_t print_cntl_len;
	size_t size;
	int retval= 0, msg_printed= 0;
	uint32_t name_width;
#ifndef __KERNEL__
	char obuf[TRACE_USER_MSGMAX];
	char *cp;
	struct tm tm_s;
	ssize_t quiet_warn= 0;
	int useconds;
#else
	char obuf[256]; /* kernel has restricted stack size */

	if (!trace_allow_printk) return;
#endif

	obuf[0]= '\0';
	print_cntl= tracePrint_cntl;
	//print_cntl_len= strlen(print_cntl);             /* used to make sure end of TRACE_PRINT can be printed even with too large msg */
													/* NOTE: could count '%' and times 2 and substract that from print_cntl_len */
#define TRACE_ROOM_FOR_NL        (sizeof(obuf) - 1)
#define TRACE_OBUF_PRINT_CNTL_CHECK   ((TRACE_ROOM_FOR_NL > print_cntl_len)?TRACE_ROOM_FOR_NL - print_cntl_len:0)
#define TRACE_PRINTSIZE(printed) (printed < (sizeof(obuf))) ? (sizeof(obuf)) - printed : 0 // FOR use as size param in snprintf which will always includes "the terminating null byte ('\0')"
	for (; *print_cntl; ++print_cntl) {
		if (*print_cntl != '%') {
			if (printed < (TRACE_ROOM_FOR_NL)) {
				/* -2 to leave room for final \n\0 */
				obuf[printed++]= *print_cntl;
				obuf[printed]= '\0';
			}
			continue;
		}

		/* PROCESS a "%" format specification */
		default_unknown_sav= print_cntl;
		if (*++print_cntl == '\0') { /* inc past % */
			/* trailing/ending '%' in spec is probably a mistake??? */
			if (printed < (TRACE_ROOM_FOR_NL)) {
				/* -2 to leave room for final \n\0 */
				obuf[printed++]= '%';
				obuf[printed]= '\0';
			}
			break;
		}

		// see if there is a width/flags specification
		flags_sz= strspn(print_cntl, "#*."); /* originally:"#-+ '*."      %#n.mf#/src#  */
		if (flags_sz) {
			snprintf(flags_ca, TRACE_MIN(flags_sz + 1, sizeof(flags_ca)), "%s", print_cntl);  // snprintf always terminates
			print_cntl+= flags_sz;                                                            // use just flags_sz here to ignore wacky +++++++
		} else flags_ca[0]='\0';
		width_ca[1][0] = width_ca[2][0] = '\0';
		for (width_state= 0; width_state < 3; ++width_state) {
			width_ia[width_state]= (int)TRACE_STRTOL(print_cntl, &endptr, 10); /* this will accept [0-9] with optional leading "-" */
			if (endptr == print_cntl || endptr > (print_cntl + 4)) { /* check if no num or num too big (allow "-099") */
				width_ca[width_state][0]='\0';
				break;
			}
			snprintf(width_ca[width_state], sizeof(width_ca[0]), "%.*s", (int)(endptr - print_cntl), print_cntl);
			print_cntl= endptr;
			if (*print_cntl == '.')
				++print_cntl;
		}

		switch (*print_cntl) {
		case '%':
			if (printed < (TRACE_ROOM_FOR_NL)) {
				/* -1 to leave room for final \n */
				obuf[printed++]= *print_cntl;
				obuf[printed]= '\0';
			}
			continue; /* avoid any further adjustment to "printed" variable */
		case 'S':     /* "severity" -- just first charater of level string - e.g. %%MSG-%S */
			if (printed < (TRACE_ROOM_FOR_NL) && trace_lvlstrs[0][lvl & TLVLBITSMSK][0] != '\0') {
				char alternate=0;
				if (flags_sz && flags_ca[0] == '#')
					alternate = 0x20;
				/* -2 to leave room for final \n */
				obuf[printed++]= trace_lvlstrs[0][lvl & TLVLBITSMSK][0] ^ alternate;
				obuf[printed]= '\0';
			}
			continue; /* avoid any further adjustment to "printed" variable */

			/* ALL THE ITEMS BELOW COULD GET TRUNKCATED IF PRINTED AFTER LARGE MSG */

		case 'a': /* nargs */
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%u", nargs);
			break;
		case 'C': /* CPU i.e. core */
			if (strchr(flags_ca,'*'))
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%" TRACE_STR(TRACE_CPU_WIDTH) "d", trace_getcpu());
			else {
				strcpy(tbuf, "%");
				if (width_state >= 1)
					strcpy(&tbuf[1], width_ca[0]);
				strcat(&tbuf[1], "d");
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), tbuf, trace_getcpu());
			}
			break;
		case 'e': /* TrcName:linenum */
			snprintf(tbuf, sizeof(tbuf), "%s:%d", TRACE_TID2NAME(TrcId), line);
			name_width = traceControl_rwp->longest_name;
			if (name_width > traceControl_p->nam_arr_sz) name_width = traceControl_p->nam_arr_sz;
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%*s", name_width + 1 + TRACE_LINENUM_WIDTH, tbuf); /* +1 for ':' */
			break;
		case 'F': {                                                   /* function */
			char snfmt[0x10];
			const char *funp=function;
			/* The "width" is used different than in a normal string spec (min[.max]):
			   "min" is not min, it's "verbosity" -
                  0 = minial (strip return type, namespace, and args)
                  1 = medium (strip return type and args; allow namespace)
                 >1 = no stripping
			   If "min" is 0 or 1 and max is used, max will be used as both the min and max.
               Note: min > max becomes min == max.
			   In the case of "%.20.30F" the ".30" is ignore (and the snfmt becomes "%20.20s".
			 */
			/* first determine how to manipulate function */
			if (   strchr(flags_ca,'.') /* no min field */
			    || (   !strchr(flags_ca,'.')
				    && (!width_state || (width_state && (width_ia[0]==0||width_ia[0]==1))))){
				int strip_ns=(   strchr(flags_ca,'.') /* no "min" field present - default to "verbosity" = 0 */
				              || (   !strchr(flags_ca,'.')
							      && (!width_state || (width_state && (width_ia[0]==0)))));
				trace_func_to_short_func(function, tbuf, sizeof(tbuf), strip_ns);
				funp=tbuf;
			}
			/* now deal with field widths */
			strcpy(snfmt, "%");
			if (strchr(flags_ca,'.')) {
				if (width_state) { /* not having a "max width" would be invalid */
					/* use the max as the min also to set a firm field width */
					strncat(&snfmt[1], width_ca[0], sizeof(snfmt)-2);
					strncat(&snfmt[1],".",sizeof(snfmt)-2);
					strncat(&snfmt[1], width_ca[0], sizeof(snfmt)-2);
				}
			} else if (width_state >= 2) {
				if (width_ia[0] <= 1)    strncat(&snfmt[1], width_ca[1], sizeof(snfmt)-2);
				else                     strncat(&snfmt[1], width_ca[0], sizeof(snfmt)-2);
				strncat(&snfmt[1], ".", sizeof(snfmt)-2);
				strncat(&snfmt[1], width_ca[1], sizeof(snfmt)-2);
			} else if (width_state == 1) strncat(&snfmt[1], width_ca[0], sizeof(snfmt)-2);
			
			strncat(                          &snfmt[1], "s", sizeof(snfmt)-2);
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), snfmt, funp);
		}
			break;
		case 'f':                                                          /* filename */
			if (flags_sz && flags_ca[0] == '#' && print_cntl[1] == '#') {  // NOTE: print_cntl[1]=='#' would be the 2nd '#'
				/* expecting something like: %#f#/src/# or %#f#/src# which would be like /src* which would match /src/ or /srcs/, among others */
				const char *ccp;
				// grab the substring spec
				unsigned long slen= (unsigned long)(strchr(&print_cntl[2], '#') - &print_cntl[2]);
				snprintf(tbuf, TRACE_MIN(slen + 1, sizeof(tbuf)), "%s", &print_cntl[2]);
				print_cntl+= (slen + 2);                     // add the value that is associated with the substring and the 2 enclosing '#' characters
				slen= TRACE_MIN(slen + 1, sizeof(tbuf)) - 1; /* length in the tbuf */
				if ((ccp= strstr(file, tbuf))) {
					ccp+= slen - 1;                    /* go to the last char in the substring */
					while (*ccp && *ccp != '/') ++ccp; /* if it's not a '/', then find the next '/' */
					if (*ccp == '/') ++ccp;            /* now, inc past it */
				} else
					ccp= file;
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s", ccp);
			} else {
				if (!width_state)
					width_ia[0]= -1;
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s", trace_path_components(file, width_ia[0]));
			}
			break;
		case 'I': /* TrcId */
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%d", TrcId);
			break;
		case 'i': /* thread id */
#if defined(__KERNEL__)
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%" TRACE_STR(TRACE_TID_WIDTH) "d", current->pid);
#else
			if (strchr(flags_ca,'*'))
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%" TRACE_STR(TRACE_TID_WIDTH) "d", traceTid); /* thread */
			else {
				strcpy(tbuf, "%");
				if (width_state >= 1)
					strcpy(&tbuf[1], width_ca[0]);
				strcat(&tbuf[1], "d");
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), tbuf, traceTid); /* thread */
			}
#endif
			break;
		case 'L': {				/* level string */
			char altbuf[0x100], l3buf[4], *lvlcp=trace_lvlstrs[0][lvl & TLVLBITSMSK];
			int  vwidth;
			if (trace_build_L_fmt( tbuf, altbuf, l3buf, &lvlcp, &vwidth, flags_ca, width_ia, width_ca ))
				retval = snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), tbuf, vwidth, lvlcp);
			else
				retval = snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), tbuf, lvlcp);
		} break;
		case 'l': /* lvl int */
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%2u", lvl);
			break;
		case 'M': /* msg with possible insert from throttle */
		case 'm': /* msg */
			if (*print_cntl == 'M' && insert[0]) {
				/* space separator only printed if insert is non-empty */
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s ", insert);
				printed+= TRACE_SNPRINTED(retval, TRACE_PRINTSIZE(printed));
			}
			if (nargs) {
				retval= msg_printed= vsnprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), msg, ap);
			} else {
				/* don't do any parsing for format specifiers in the msg -- tshow will
				   also know to do this on the memory side of things */
				retval= msg_printed= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s", msg);
			}
			break;
		case 'N': /* trace name - not padded */
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s", TRACE_TID2NAME(TrcId));
			break;
		case 'n': /* trace name - padded */
			name_width = traceControl_rwp->longest_name;
			if (name_width > traceControl_p->nam_arr_sz) name_width = traceControl_p->nam_arr_sz;
			if (strchr(flags_ca,'*')) {
				/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . always pads (and truncate if too large) */
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%*.*s", name_width, name_width, TRACE_TID2NAME(TrcId));
			} else {
				/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . don't pad, and just truncate if too large */
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%.*s", name_width, TRACE_TID2NAME(TrcId));
			}
			break;
		case 'O': /* cOlor On */
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s", trace_lvlcolors[lvl & TLVLBITSMSK][0]);
			break;
		case 'o': /* cOlor off */
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s", trace_lvlcolors[lvl & TLVLBITSMSK][1]);
			break;
		case 'P': /* process id */
#if defined(__KERNEL__)
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%" TRACE_STR(TRACE_TID_WIDTH) "d", current->tgid);
#else
			if (strchr(flags_ca,'*'))
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%" TRACE_STR(TRACE_TID_WIDTH) "d", tracePid);
			else {
				strcpy(tbuf, "%");
				if (width_state >= 1)
					strcpy(&tbuf[1], width_ca[0]);
				strcat(&tbuf[1], "d");
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), tbuf, tracePid);
			}
#endif
			break;
#	ifdef TRACE_DO__PROGNAME
		case 'p': {
			extern char *__progname;
			retval= snprintf(&obuf[printed], TRACE_PRINTSIZE(printed),"%s",__progname);
		}	break;
#	endif
		/*case 'S': ABOVE */
		case 'T': /* Time */
			if (tvp->tv_sec == 0)
				TRACE_GETTIMEOFDAY(tvp);
#ifdef __KERNEL__
			retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%ld.%06ld", tvp->tv_sec, tvp->tv_usec);
#else
			if (traceTimeFmt == NULL) {
				/* no matter who writes, it should basically be the same thing */
				if ((cp= getenv("TRACE_TIME_FMT")) != NULL)
					traceTimeFmt= cp; /* single write here */
				else
					traceTimeFmt= TRACE_DFLT_TIME_FMT; /* OR single write here */
			}
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#		pragma GCC diagnostic ignored "-Wformat-nonliteral"
#	endif
			localtime_r((time_t *)&tvp->tv_sec, &tm_s);
			if (strftime(tbuf, sizeof(tbuf), traceTimeFmt, &tm_s) == 0)
				tbuf[0]= '\0';
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic pop
#	endif
			useconds= (int)tvp->tv_usec;
			if ((cp= strstr(tbuf, "%0")) && *(cp + 2) >= '1' && *(cp + 2) <= '5' && *(cp + 3) == 'd') {
				// NOTE: if *(cp+2)==6, don't do anything.
				// Example: fmt originally had "%%04d" which got changed by strftime to "%04d";
				// grab the character '4' and adjust useconds accordingly.
				int div;
				switch (*(cp + 2)) {
				case '1': div= 100000; break;
				case '2': div= 10000;  break;
				case '3': div= 1000;   break;
				case '4': div= 100;    break;
				case '5': div= 10;     break;
				default:  div= 1;      break;
				}
				useconds= (int)((double)useconds / div + 0.5); /* div, round and cast back to int -- FIXME: theoretically possible to have negative useconds */
			}
			size = TRACE_PRINTSIZE(printed);
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wformat-nonliteral"
#	endif
			retval= snprintf(&obuf[printed], size, tbuf, useconds); /* possibly (probably) add usecs */
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic pop
#	endif
#endif /* __KERNEL__ */
			break;
		case 't': /* msg limit insert - may be null (as fmt does not have " " after) */
			if (insert[0])
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s ", insert);
			else
				retval=0;
			break;
		case 'u': /* lineNumber */
			if (strchr(flags_ca,'*'))
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%" TRACE_STR(TRACE_LINENUM_WIDTH) "d", line);
			else {
				strcpy(tbuf, "%");
				if (width_state >= 1)
					strcpy(&tbuf[1], width_ca[0]);
				strcat(&tbuf[1], "d");
				retval= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), tbuf, line);
			}
			break;
		default:
			size = TRACE_PRINTSIZE(printed);
			retval= snprintf(&(obuf[printed]), size, "%.*s", (int)(print_cntl - default_unknown_sav + 1), default_unknown_sav);
		}
		printed+= TRACE_SNPRINTED(retval, TRACE_PRINTSIZE(printed));
	} /* for (; *print_cntl; ++print_cntl) */

	/* a restriction on the TRACE_PRINT spec is that it must print the message */
	if (msg_printed == 0) {
		if (!strchr(" \t:-|", obuf[printed - 1])) {
			/* if the specification does not end in a "separator" */
			obuf[printed++]= ' ';
			obuf[printed]= '\0';
		}
		if (nargs) {
			retval= msg_printed= vsnprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), msg, ap);
		} else {
			/* don't do any parsing for format specifiers in the msg -- tshow will
			   also know to do this on the memory side of things */
			retval= msg_printed= snprintf(&(obuf[printed]), TRACE_PRINTSIZE(printed), "%s", msg);
		}
		printed+= TRACE_SNPRINTED(retval, TRACE_PRINTSIZE(printed));
	}

	if (printed < TRACE_ROOM_FOR_NL) { /* SHOULD/COULD BE AN ASSERT */
		/* there is room for the \n */
		/* buf first see if it is needed */
		if (obuf[printed - 1] != '\n' && obuf[printed - 1] != '\r') {
			obuf[printed++]= '\n'; /* overwriting \0 is OK as we will specify the amount to write */
								   /*printf("added \\n printed=%d\n",printed);*/
			obuf[printed]= '\0';
		}
		/*else printf("already there printed=%d\n",printed);*/
#if defined(__KERNEL__)
		printk(obuf);
#else
		quiet_warn= write(tracePrintFd[lvl & TLVLBITSMSK], obuf, printed);
		if (quiet_warn == -1)
			perror("writeTracePrintFd");
#endif
	} else {
		/* obuf[sizeof(obuf)-1] has '\0'. */
		if (obuf[sizeof(obuf) - 2] != '\n' && obuf[sizeof(obuf) - 2] != '\r') {
			obuf[sizeof(obuf) - 2]= '\n';
		}
#if defined(__KERNEL__)
		printk(obuf);
#else
		quiet_warn= write(tracePrintFd[lvl & TLVLBITSMSK], obuf, sizeof(obuf) - 1);
		if (quiet_warn == -1)
			perror("writeTracePrintFd");
#endif
	}
	/*TRACE_PRN("sizeof(obuf)=%zu retval=%d msg_printed=%d printed=%zd\n",sizeof(obuf), retval, msg_printed, printed);*/
} /* vtrace_user */

SUPPRESS_NOT_USED_WARN
static void trace_user(trace_tv_t *tvp, int TrcId, uint8_t lvl, const char *insert, const char *file, int line, const char *function, uint16_t nargs, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vtrace_user(tvp, TrcId, lvl, insert, file, line, function, nargs, msg, ap);
	va_end(ap);
} /* trace_user - const char* */
#ifdef __cplusplus
#	if (__cplusplus >= 201103L)
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wvarargs"
#	endif
SUPPRESS_NOT_USED_WARN
static void trace_user(trace_tv_t *tvp, int TrcId, uint8_t lvl, const char *insert, const char *file, int line, const char *function, uint16_t nargs, const std::string &msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vtrace_user(tvp, TrcId, lvl, insert, file, line, function, nargs, msg.c_str(), ap);
	va_end(ap);
} /* trace_user - std::string& */
#	if (__cplusplus >= 201103L)
#		pragma GCC diagnostic pop
#	endif
#endif

/* set defaults first (these wont ever be used in KERNEL) */
#define TRACE_REGISTER_ATFORK
#define TRACE_CHK_PID \
	pid_t chkpid;     \
	if (tracePid != (chkpid= getpid())) { tracePid= traceTid= chkpid; }

/* now test is defaults need to be changed. GLIBC is relevant for user space   */
#if defined(__GLIBC_PREREQ)
/* if undefined __GLIBC_PREREQ is used in a #if (i.e. previous line), some preprocessor will give:
    error: missing binary operator before token "("
 */
#		ifdef __cplusplus
extern "C" int __register_atfork(void (*)(void), void (*)(void), void (*)(void));
#			ifdef __THROW
# 				define ___THROW __THROW
#			else
#				define ___THROW
#			endif
extern size_t confstr(int,char*,size_t) ___THROW;
#		else
extern int __register_atfork(void (*)(void), void (*)(void), void (*)(void));
extern size_t confstr(int,char*,size_t);
#		endif
static void trace_pid_atfork(void)
{
	/*traceTid=tracePid=getpid();TRACEN("TRACE",61,"trace_pid_atfork " __BASE_FILE__ );*/
	const char *rp;
	char somebuf[120];
	somebuf[0]= '\0';
	traceTid= tracePid= getpid();
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#			pragma GCC diagnostic push
#			pragma GCC diagnostic ignored "-Wformat-security"
#			if !defined(__clang__) && defined(__GNUC__) && __GNUC__ >= 8
#				pragma GCC diagnostic ignored "-Wformat-truncation"
#			endif
#		endif
	TRACEN("TRACE", 61,
		   (somebuf[0]
				? &(somebuf[0])
				: (snprintf(&(somebuf[0]), sizeof(somebuf), "trace_pid_atfork %s",
							(rp= rindex(__BASE_FILE__, '/')) != NULL
								? rp + 1
								: __BASE_FILE__),
				   &(somebuf[0]))));
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#			pragma GCC diagnostic pop
#		endif
}
#	if __GLIBC_PREREQ(2, 27)
#		undef TRACE_REGISTER_ATFORK
#		define TRACE_REGISTER_ATFORK __register_atfork(NULL, NULL, trace_pid_atfork)
#		undef TRACE_CHK_PID
#		define TRACE_CHK_PID
#	endif
#endif

ATTRIBUTE_NO_SANITIZE_ADDRESS
static void vtrace(trace_tv_t *tvp, int trcId, uint8_t lvl, int32_t line, const char *function, uint8_t nargs, const char *msg, va_list ap)
{
	struct traceEntryHdr_s *myEnt_p;
	char *msg_p;
	uint64_t *params_p; /* some archs (ie. i386,32 bit arm) pass have 32 bit and 64 bit args; use biggest */
	unsigned argIdx;
	uint8_t get_idxCnt_retries= 0;
	uint32_t myIdxCnt;
	uint32_t desired;
	size_t sz, len;
	char *out;
#ifndef __KERNEL__
	//TRACE_CHK_PID;
	if (!trace_no_pid_check){     // starting at glibc 2.27, I do not have to check b/c atfork
		pid_t chkpid;
		if (tracePid != (chkpid= getpid())) { tracePid= traceTid= chkpid; }
	}
	/*printf("overflow_arg_area=%p\n",ap->overflow_arg_area); NOTE: THIS MUST INDICATE 16 byte alignment in order for long double to work */
#endif
	/* I learned ... ---v---v---v---v---v---v---v---v---v---v---
	   I originally thought I would get this entry idx/slot first (because
	   there might be a retry which would take time) and then get the time
	   (which I thought would _always_ be quick). Well, it turns out that
       getting time can take some time, perhaps similar to getting the idx?

       big_ex.sh on laptop w/ default -t50 -l50  and "time before idx":
          tshow | tdelta | grep '   -[1-9]' | wc -l ===> 424
	      tshow | tdelta -stats | tail -5
                      min     -5332
                      max     13154
                      tot  19553408
                      ave 1.7840616
                      cnt  10960052
          tshow | grep '[^ ] [1-9] [^ ]' | wc -l    ===> 26739 # seems doing time 1st synchronizes threads to cause more idx retries
          tshow | tdelta -stats | grep -C1 '   -[1-9]' | less  # shows - negative times are associated with idx retry

       big_ex.sh on laptop w/ default -t50 -l50  and "idx before time":
          tshow | tdelta | grep '   -[1-9]' | wc -l ===> 248
	      tshow | tdelta -stats | tail -5
                      min     -6164
                      max      8302
                      tot  19424406
                      ave 1.7722914
                      cnt  10960052
          tshow | grep '[^ ] [1-9] [^ ]' | wc -l    ===> 16783
          tshow | tdelta -stats | grep -C1 '   -[1-9]' | less  # does not indicate nearby retries --
                 -- most like time just took a lot of time to calculate/return (or maybe an
				 interrupt/context switch??) -- unknown cause.

# just focused on "negatives"
big_ex.sh ./big_ex.d;\
TRACE_SHOW=HxtnLR tshow | tdelta -d 0 | grep '\-[1-9]' | wc -l;\
TRACE_SHOW=HxTnLR tshow | tdelta -d 0 | grep '\-[1-9]' | wc -l

tod,idx,tsc:             ( tod before )
tsc:    646     739    478
tod:    482     371    376

idx,tsc,tod:             ( tod after -- significantly more "negatives" )
tsc:   1205     840
tod: 132348  133161

	 */
	/* ---^---^---^---^---^---^---^---^---^---^---^---^---^---^---^---^--- */

#define TRACE_TSC_EXPERIMENT 0 /* is TSC "consistent" across all core? (only negative is rollover) */
							   /* experiment shows that if time were always retrieved exactly with idx, it
	   would always increment, but this garuntee would cause TRACE to take near 10x longer, realizing
	   that regardless of locking, there is always the possibility that a task will be interrupted
	   between getting the idx and getting the tsc.
	   big_ex w/o lock: tshow|tdelta  -stats|tail|grep ave: 1.8037151; w/ lock: 15.132927 */
#if TRACE_TSC_EXPERIMENT == 1
	if (!trace_lock(&traceControl_rwp->namelock)) {
		TRACE_PRN("trace_lock: namelock hung?\n");
	}
#endif
	myIdxCnt= TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
	desired= TRACE_IDXCNT_ADD(myIdxCnt, 1);
#if defined(__KERNEL__)
	while (cmpxchg(&traceControl_rwp->wrIdxCnt, myIdxCnt, desired) != myIdxCnt) {
		++get_idxCnt_retries;
		myIdxCnt= TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
		desired= TRACE_IDXCNT_ADD(myIdxCnt, 1);
	}
#elif (defined(__cplusplus) && (__cplusplus >= 201103L)) || defined(TRACE_C11_ATOMICS)
	while (!atomic_compare_exchange_weak(&traceControl_rwp->wrIdxCnt, &myIdxCnt, desired)) {
		++get_idxCnt_retries;
		desired= TRACE_IDXCNT_ADD(myIdxCnt, 1);
	}
#else
	while (cmpxchg(&traceControl_rwp->wrIdxCnt, myIdxCnt, desired) != myIdxCnt) {
		++get_idxCnt_retries;
		myIdxCnt= TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
		desired= TRACE_IDXCNT_ADD(myIdxCnt, 1);
	}
#endif

	/* Now, "desired" is the count (full check below) and myIdxCnt is the index */
	myEnt_p= idxCnt2entPtr(myIdxCnt);

#ifdef __KERNEL__
	/* There are some places in the kernel where the gettimeofday routine
	   cannot be called (i.e. kernel/notifier.c routines). For these routines,
	   add 64 for the level (i.e. 22+64) */
	if (lvl >= 64) {
		tvp->tv_sec= 1;
		tvp->tv_usec= 0;
	} else
#endif
		if (tvp->tv_sec == 0) {
		TRACE_GETTIMEOFDAY(tvp); /* hopefully NOT a system call */
	}

	/*myEnt_p->time = *tvp;   move to end - reasonable time is indication of complete */
	TRACE_TSC32(myEnt_p->tsc);

	if (desired == traceControl_p->num_entries) {
		traceControl_rwp->full= 1; /* now we'll know if wrIdxCnt has rolled over */
	}

#if TRACE_TSC_EXPERIMENT == 1
	trace_unlock(&traceControl_rwp->namelock);
#endif
#undef TRACE_TSC_EXPERIMENT

#if defined(__KERNEL__)
	myEnt_p->pid= current->tgid;
	myEnt_p->tid= current->pid;
#else
	myEnt_p->pid= tracePid;
	myEnt_p->tid= traceTid;
#endif
#ifdef __arm__
	if (traceControl_rwp->mode.bits.fast_do_getcpu)
		myEnt_p->cpu= trace_getcpu(); /* for userspace, this costs alot :(*/
	else
		myEnt_p->cpu= -1;
#else
	myEnt_p->cpu= trace_getcpu(); /* for userspace, this costs alot :(*/
#endif

	myEnt_p->linenum= (uint32_t)line;
	myEnt_p->TrcId= trcId;
	myEnt_p->lvl= lvl;
	myEnt_p->nargs= nargs;
	myEnt_p->get_idxCnt_retries= get_idxCnt_retries;
	myEnt_p->param_bytes= sizeof(long);

	msg_p= (char *)(myEnt_p + 1);
	sz=traceControl_p->siz_msg; /* ASSUME min sz=1; sz would/could/does include null, len does not */
	out=msg_p;
#	if 1
	/* func ==> 1=force on, 0=TRACE_PRINT, -1=force off */
	if ((trace_vtrace_cntl.prepend_func && traceControl_rwp->mode.bits.func != -1) || traceControl_rwp->mode.bits.func == 1) {
		const char * start;
        const char * end = strchr( function, '(' );
        if ( end ) {
            start = end;
            while ( start > function ) {
                if ( *(start-1) == ':' || *(start-1) == ' ' ) /*stop at space between return type OR beginning of namespace "::" */
                    break;
                --start;
            }
			len = (size_t)(end - start);
        } else {
            start = function;
			len = strlen(function);
			end = start + len;
		}
		if (len>sz) {
			len=sz;
			end=start+len;
		}
		while (start != end) /*  function name will be too short to man strcpy worth it */
			*out++ = *start++;

		sz -= len; /* sz could become 0 */
		/* now copy any separator characters */
		for (len=0; (len+1)<sz && trace_vtrace_cntl.sep[len]; ++len)
			*out++ = trace_vtrace_cntl.sep[len];

		sz -= len;
		//for (len=0; (len+1)<sz && msg[len]; ++len) *out++ = msg[len];
		//*out='\0';
	} //else
	//for (len=0; (len+1)<sz && msg[len]; ++len) *out++ = msg[len];
	//*out='\0';
	// strcpy and strncpy must do optimized copy (i.e multiple bytes in loops).
	if (strlen(msg) < sz)
		strcpy(out,msg);
	else 
#	endif
	{
		strncpy(out, msg, sz);
		out[sz-1] = '\0';
	}

	params_p= (uint64_t *)(msg_p + traceControl_p->siz_msg);
	/* emulate stack push - right to left (so that arg1 end up at a lower
	   address, arg2 ends up at the next higher address, etc. */
	if (nargs) /* in one view of the world: # of 64 bit args - WHICH is _not_ - */
	{          /* - the number of actual args! 
				  long double on i686 is 96, on x64 is 128 bits */
		nargs=(uint8_t)(nargs+2);
					/* ++nargs; * GCC 4.8.5 gives conversion warning with nargs+=2.
							 This will/would be 2 long doubles on x64 and 4 on i686.
					   NOTE: the C++ streamer interface adds for each long double
					   (has nargs increase passed in; see below),
					   so this would be redundant.
					   This is the compromise - ugh! FIXME??? - I CANNOT do this
					   addition in the TRACE*(...) macros b/c nargs==0 in entry has meaning!
					   The bottom line is the C++ streamer can/does know exactly
					   how many long doubles while the C TRACE* macros do not!
					*/
		if (nargs > traceControl_p->num_params)
			nargs= (uint8_t)traceControl_p->num_params;
		for (argIdx= 0; argIdx < nargs; ++argIdx) {
			params_p[argIdx]= va_arg(ap, uint64_t); /* this will usually copy 2x and 32bit archs, but they might be all %f or %g args */
		}
	}

	myEnt_p->time= *tvp; /* reasonable time (>= prev ent) is indication of complete */

	if (traceControl_rwp->trigActivePost) { /* armed, armed/trigger */
		if (traceControl_rwp->triggered) {  /* triggered */
			if (TRACE_IDXCNT_DELTA(desired, traceControl_rwp->trigIdxCnt) >= traceControl_rwp->trigActivePost) {
				/* I think there should be an indication in the M buffer */
				TRACE_CNTL("modeM", 0); /* calling traceCntl here eliminates the "defined but not used" warning for modules which do not use TRACE_CNTL */
				traceControl_rwp->trigActivePost= 0;
				/* triggered and trigIdxCnt should be cleared when
				   "armed" (when trigActivePost is set) */
			}
			/* else just waiting... */
		} else if (traceLvls_p[trcId].T & TLVLMSK(lvl)) {
			traceControl_rwp->triggered= 1;
			traceControl_rwp->trigIdxCnt= myIdxCnt;
		}
	}
} /* vtrace */

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-parameter" /* b/c of TRACE_XTRA_UNUSED */
#	ifdef __cplusplus
#		pragma GCC diagnostic ignored "-Wvarargs"
#	endif
#endif

SUPPRESS_NOT_USED_WARN
static void trace(trace_tv_t *tvp, int trcId, uint8_t lvl, int32_t line, const char *function, uint8_t nargs TRACE_XTRA_UNUSED, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg); /*printf("ap->overflow_arg_area=%p\n", ap->overflow_arg_area );*/
	vtrace(tvp, trcId, lvl, line, function, nargs, msg, ap);
	va_end(ap);
} /* trace */

#ifdef __cplusplus
SUPPRESS_NOT_USED_WARN
static void trace(trace_tv_t *tvp, int trcId, uint8_t lvl, int32_t line, const char *function, uint8_t nargs TRACE_XTRA_UNUSED, const std::string &msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vtrace(tvp, trcId, lvl, line, function, nargs, msg.c_str(), ap);
	va_end(ap);
} /* trace */
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#	pragma GCC diagnostic pop
#endif

/* https://stackoverflow.com/questions/7666509/hash-function-for-string
   http://www.cse.yorku.ca/~oz/hash.html */
typedef uint32_t trace_hash_t;
static trace_hash_t trace_name_hash(const char *str)
{
	trace_hash_t hash= 5381;
	int c;

	while ((c= *str++))
		hash= ((hash << 5) + hash) + (trace_hash_t)c; /* hash * 33 + c */

	return hash;
}

/* Search for name and insert if not found and not full
   Returns value between 0 and traceControl_p->num_namLvlTblEnts - 1, inclusive
 */
static uint32_t trace_name2TID(const char *nn)
{
	uint32_t start, ii, len;
	const char *name= (nn && nn[0]) ? nn : traceName; /* attempt argument (to trace_name2TID) safety checking */
	char valid_name[TRACE_TN_BUFSZ];
	int rehash= 0;
	size_t zz = strlen(name);
#if defined(__KERNEL__)
	if (traceEntries_p == NULL) return -1;
#elif defined(TRACE_DEBUG_INIT)
	fprintf(stderr,"n2t=%p name=%s\n",name,name);
#endif
	/** Since names are mostly associated with the filename which can be a
		path, the right side (end) has the most (significant) info. So, if
		name length are greater than max, chop off from the left (beginning). 
	 */
	if (zz > (traceControl_p->nam_arr_sz-1))
		name += zz - (traceControl_p->nam_arr_sz-1);

	/* First, starting at "start" search for the name. We can stop
	   searching when either the string or an empty string is found.
	   If the string is found, return. If an empty is found, go on
	   to insert. */
	if (strcmp(name, "_TRACE_") == 0) /* if hashing, "_TRACE_" is special, so need to check */
		return (traceControl_p->num_namLvlTblEnts - 1);
	ii= start= /*0;*/ trace_name_hash(name) % traceControl_p->num_namLvlTblEnts;
	do {
		char *namep= idx2namsPtr((int32_t)ii);
		if (*namep == '\0')
			break;
		if (strncmp(namep, name, traceControl_p->nam_arr_sz - 1) == 0)
			return (ii); /* found name -- it already exists! */
		ii= (ii + 1) % traceControl_p->num_namLvlTblEnts;
	} while (ii != start);

	/* only allow "valid" names to be inserted -- above, we assumed the
	   name was valid, giving the caller the benefit of the doubt, for
	   efficiency sake, but here we will make sure the name is valid */
	for (ii= 0; name[ii] != '\0' && ii < (traceControl_p->nam_arr_sz - 1); ++ii) {
		if (isgraph(name[ii])) /* checks for any printable character except space. */
			valid_name[ii]= name[ii];
		else {
			valid_name[ii]= '_';
			rehash= 1;
		}
	}
	valid_name[ii]= '\0';
	len= ii;
	if (rehash)
		start= /*0;*/ trace_name_hash(valid_name) % traceControl_p->num_namLvlTblEnts;

	/* NOTE: multiple threads which may want to create the same name might arrive at this
	   point at the same time. Checking for the name again (see strncmp
	   near end of loop below), with the lock, makes this OK. */
	if (!trace_lock(&traceControl_rwp->namelock))
		TRACE_PRN("trace_lock: namelock hung?\n");

	ii= start;
	do {
		char *namep= idx2namsPtr((int32_t)ii);
		if (namep[0] == '\0') {
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wpragmas"
#	pragma GCC diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
			strncpy(TRACE_TID2NAME((int32_t)ii), valid_name, (traceControl_p->nam_arr_sz - 1));
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#	pragma GCC diagnostic pop
#endif
			if (trace_lvlS)
				traceLvls_p[ii].S= trace_lvlS; /* See also traceInitNames */
			if (trace_lvlM)
				traceLvls_p[ii].M= trace_lvlM; /* See also traceInitNames */
			if (traceControl_rwp->longest_name < len)
				traceControl_rwp->longest_name= len;
			trace_unlock(&traceControl_rwp->namelock);
			return (ii); /* filled empty slot with new name and inited lvls */
		}
		if (strncmp(namep, valid_name, (traceControl_p->nam_arr_sz - 1)) == 0) {
			trace_unlock(&traceControl_rwp->namelock);
			return (ii); /* (non-empty) Existing slot/name matched -- i.e. (valid) name already exists */
		}
		ii= (ii + 1) % traceControl_p->num_namLvlTblEnts;
	} while (ii != start);
	trace_unlock(&traceControl_rwp->namelock);
	return (traceControl_p->num_namLvlTblEnts - 1); /* the case for when the table is full */
} /* trace_name2TID */

#ifndef __KERNEL__
static void trace_namLvlSet(void)
{
	const char *cp;
	int sts;
	/* This is a signficant env.var as this can be setting the
	   level mask for many TRACE NAMEs/IDs AND potentially many process
	   could do this or at least be TRACE-ing. In other words, this has
	   the potential of effecting another process's TRACE-ing. */
	if ((cp= getenv("TRACE_NAMLVLSET"))) {
		int ign; /* will ignore trace id (index) if present */
		char name[PATH_MAX], line[PATH_MAX];
		const char *cpnl;
		size_t len;
		unsigned long long M, S, T= 0;
		/* NOTE: Currently, (it could change; see trace_name2TID)
		   TRACE_NAME's can contain ',' (i.e "THIS,THAT" is a valid),
		   but since it's unusual, after checking for " " separated,
		   I'll  check for "," separated (because it seems reasonable) */
		if ((cpnl=strchr(cp,'\n'))) len=(size_t)(cpnl-cp);
		else                        len=strlen(cp);
	    len=TRACE_MIN(sizeof(line)-1,len); /* need room, possibly, for terminator */
		strncpy(line,cp,len); line[len]='\0';
		while (((sts= sscanf(line, "%d %s %llx %llx %llx", &ign, name, &M, &S, &T)) && sts >= 4)  //NOLINT
			   || ((sts= sscanf(line, "%s %llx %llx %llx", name, &M, &S, &T)) && sts >= 3)        //NOLINT
			   || ((sts= sscanf(line, "%[^,],%llx,%llx,%llx", name, &M, &S, &T)) && sts >= 3)        //NOLINT
			   || ((sts= sscanf(line, "%s %llx", name, &S)) && sts == 2)                          //NOLINT
			   || ((sts= sscanf(line, "%s[^,],%llx", name, &S)) && sts == 2))                         //NOLINT
																								// The last case is for when TRACE will remain "inactive" (not tracing to mem and not using lvls from trace file) -- just concerned about slow path
		{
			int32_t tid= (int32_t)trace_name2TID(name);
			/*fprintf(stderr,"name=%s tid=%d\n",name,tid );*/
			traceLvls_p[tid].S= S;
			if (sts > 2)  // note: if sts==2, M will be set to S as previous sscanf will have set M; this check filters this effect out.
			{
				traceLvls_p[tid].M= M;
				traceLvls_p[tid].T= T;
			}
			if (cpnl == NULL)
				break;
			cp = cpnl+1;
			if ((cpnl=strchr(cp,'\n'))) len=(size_t)(cpnl-cp);
			else                        len=strlen(cp);
			len=TRACE_MIN(sizeof(line)-1,len); /* need room, possibly, for terminator */
			strncpy(line,cp,len); line[len]='\0';
			T= 0;
		}
		if (cpnl != NULL && *cpnl != '\0') {
			fprintf(stderr, "Warning: TRACE_NAMLVLSET in env., but processing did not complete\n");
		}
	}
	if ((cp= getenv("TRACE_MODE"))) {
		traceControl_rwp->mode.words.mode= (uint16_t)strtoul(cp, NULL, 0);
	}
	if ((cp= getenv("TRACE_LIMIT_MS"))) {
		unsigned cnt;
		uint32_t on_ms, off_ms;
		sts= sscanf(cp, "%u,%u,%u", &cnt, &on_ms, &off_ms);  //NOLINT
		switch (sts) {
		case 0: /* As a way to temp unset TRACE_LIMIT_MS, allow: TRACE_LIMIT_MS= tinfo */
			break;
#	if defined(__STDC_VERSION__) && (__GNUC__ >= 7)
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
#	endif
		case 2:
			off_ms= on_ms;
			//fall through after setting default off_ms to on_ms
#	if defined(__cplusplus) && (__GNUC__ >= 7)  //(__cplusplus >= 201703L) warning happen even with c++11
#		if __has_cpp_attribute(fallthrough)
			[[fallthrough]];
#		else
			[[gnu:fallthrough]];
#		endif
#	endif
		case 3:
#	if defined(__STDC_VERSION__) && (__GNUC__ >= 7)
#		pragma GCC diagnostic pop
#	endif
			traceControl_rwp->limit_cnt_limit= cnt;
			traceControl_rwp->limit_span_on_ms= on_ms;
			traceControl_rwp->limit_span_off_ms= off_ms;
			break;
		default:
			fprintf(stderr, "Warning: problem parsing TRACE_LIMIT_MS - should be: <cnt>,<on_ms>[,off_ms]\n");
			traceControl_rwp->limit_cnt_limit= 0;
		}
	}
} /* trace_namLvlSet */
#endif

static inline void trace_msk_op(uint64_t *v1, int op, uint64_t v2)
{
	switch (op) {
	case 0: *v1= v2; break;
	case 1: *v1|= v2; break;
	case 2: *v1&= ~v2; break;
	}
}

/* 
passing _name: because this method will have to make sure trace is initialize
and to avoid compiling in a (default) name into (hard-coding) the function in
this header file, _name is passed. _name will then take on the value used when
compiling calling of traceCntl via the TRACE_CNTL macro.
NOTE: on 64 bit arch, ints (4 bytes) and shorts (2 bytes) are pushed as 8!!!
      on 32 bit arch, shorts (2 bytes) are pushed as 4.

cmd		   		 arg	      	  notes        returns
------           ---------		----------    ---------
file			 char*				1             0
namlvlset		 optional char*		1             0
mapped			 n/a                             bool
init             n/a                              0
name			 char*				1             0
mode[ MS]		 unsigned				1            old mode (uint16_t)
getcpu           unsigned                         0
lvlmskn[ MST]	 ULL				2            -1(error), 0 (success)
lvlsetn[ MST]	 ULL				2            -1(error), 0 (success)
lvlclrn[ MST]	 ULL				2            -1(error), 0 (success)
lvlmsk[ MST][gG] ULL				2             0
lvlset[ MST][gG] ULL				2             0
lvlclr[ MST][gG] ULL				2             0
trig			 UL [ULL]			1,2           0
reset			 n/a                              0
limit_ms		 UL UL [UL]			1            limit_cnt_limit (uint32_t)
<unknown>                                        -1

Note 0: "int" is 4 bytes on both.
Note 1: 4 bytes on 32, 8 bytes on 64
Note 2: 8 bytes (LL or ULL) on both 32 and 64 bit machines

find . -name \*.[ch] -o -name \*.cc | xargs grep -n 'TRACE_CNTL('
for cc in file namlvl mapped name mode lvl trig reset limit_ms; do
  echo === $cc ===
  find . -name \*.[ch] -o -name \*.cc | xargs grep -n "TRACE_CNTL( *\"$cc"
done
 */
static int64_t traceCntl(const char *_name, const char *_file, int nargs, const char *cmd, ...)
{ /* . . . . . . . . . . . . . . . . nargs is args after cmd */
	int64_t ret= 0;
	va_list ap;
	unsigned uu;
	struct {
		char tn[TRACE_TN_BUFSZ];
	} _trc_;

	va_start(ap, cmd);

	/* although it may be counter intuitive, this should override
	   env.var as it could be used to set a file-per-thread.
	   NO!!!  -- I think env will over ride, this will just change
	   the default for name/file.
	   NOTE: CAN'T HAVE FILE-PER-THREAD unless traceControl_p,traceEntries_p,traceLvls_p,traceNams_p are THREAD_LOCAL
	   CAN'T HAVE NAME-PER-THREAD unless traceTID       is THREAD_LOCAL
	*/
#ifndef __KERNEL__
	if (strcmp(cmd, "file") == 0)                                           /* if TRACE_CNTL "name" and "file" are both used, "file" must be first (i.e. "name" first will _set_ file which won't be changed by "file") */
	{                                                                       /* THIS really only makes sense for non-thread local-file-for-module or for non-static implementation w/TLS for file-per-thread */
		traceFile= va_arg(ap, char *);                                      /* this can still be overridden by env.var.; suggest testing w. TRACE_ARGSMAX=10*/
		traceInit(trace_name(_name, _file, _trc_.tn, sizeof(_trc_.tn)), 0); /* will not RE-init as traceControl_p!=NULL skips mmap_file */
		va_end(ap);
		return (0);
	}
	if (strcmp(cmd, "namlvlset") == 0) {
		/* use this if program sets TRACE_NAMLVLSET env.var.  This can be used
		   to Init or called trace_namLvlSet() after an Init has occurred. */
		const char *name= (nargs == 0) ? _name : va_arg(ap, char *); /* name is optional */
		/*printf("nargs=%d name=%s\n",nargs,name);*/
		if (traceControl_p == NULL) {
			traceInit(trace_name(name, _file, _trc_.tn, sizeof(_trc_.tn)), 0); /* with traceControl_p==NULL. trace_namLvlSet() will be called */
		} else {
			trace_namLvlSet(); /* recall trace_namLvlSet(). optional name, if given, is ignored */
		}
		va_end(ap);
		return (0);
	}
	if (strcmp(cmd, "mapped") == 0) {
		if TRACE_INIT_CHECK (trace_name(_name, _file, _trc_.tn, sizeof(_trc_.tn))) {};
		ret= (traceControl_p != &traceControl[0]); /* compatible with define TRACE_CNTL(...) (0) */
		va_end(ap);
		return (ret);
	}
#endif
	if TRACE_INIT_CHECK (trace_name(_name, _file, _trc_.tn, sizeof(_trc_.tn))) {}; /* note: allows trace_name2TID to be called in userspace */

	if (strcmp(cmd, "init") == 0)
		return (0);                    /* just done above */
	else if (strcmp(cmd, "name") == 0) /* if TRACE_CNTL "name" and "file" are both used, "file" must be first (i.e. "name" first will _set_ file which won't be changed by "file") */
	{                                  /* THIS really only makes sense for non-thread local-name-for-module or for non-static implementation w/TLS for name-per-thread */
		char *tnam;                    /* this can still be overridden by env.var. IF traceInit(TRACE_NAME,0) is called; suggest testing w. TRACE_ARGSMAX=10*/
		if (nargs < 1 || nargs > 1) {
			TRACE_EPRN("traceCntl \"name\" command must one argument: <name>\n");
			return (-1);
		}
		tnam= va_arg(ap, char *);
		/* set traceName, which we don't want to be "" */
		traceName= (tnam && *tnam) ? tnam : trace_name(_name, _file, _trc_.tn, sizeof(_trc_.tn)); /* doing it this way allows this to be called by kernel module */
		traceTID= (int)trace_name2TID(traceName);                                                 /* this requires traceInit */
	} else if (strncmp(cmd, "cntl", 4) == 0) {
		traceControl_rwp->mode.words.cntl= (uint16_t)va_arg(ap, long);
		;                                      // long is 4 bytes on 32, 8 on 64
	} else if (strncmp(cmd, "mode", 4) == 0) { /* this returns the (prv/cur) mode requested */
		switch (cmd[4]) {
		case '\0':
			ret= traceControl_rwp->mode.words.mode;
			if (nargs == 1) {
				uint16_t mode= (uint16_t)va_arg(ap, unsigned);  // 4 bytes on both 32 and 64, but pushed as 64 on 64; va_arg knows this.
				union trace_mode_u tmp= traceControl_rwp->mode;
				tmp.words.mode= mode;
#ifndef __KERNEL__
				if (traceControl_p == &(traceControl[0])) {
					tmp.bits.M= 0;
				}
#endif
				traceControl_rwp->mode= tmp;
			}
			break;
		case 'M':
			ret= traceControl_rwp->mode.bits.M;
#ifndef __KERNEL__
			if (traceControl_p == &(traceControl[0])) {
				break;
			}
#endif
			if (nargs == 1) {
				unsigned mode= va_arg(ap, unsigned);  // 4 bytes on both 32 and 64
				traceControl_rwp->mode.bits.M= (mode != 0);
			}
			break;
		case 'S':
			ret= traceControl_rwp->mode.bits.S;
			if (nargs == 1) {
				unsigned mode= va_arg(ap, unsigned);  // 4 bytes on both 32 and 64
				traceControl_rwp->mode.bits.S= (mode != 0);
			}
			break;
		default:
			ret= -1;
		}
	} else if (strcmp(cmd, "getcpu") == 0) { /* ??? */
		ret= traceControl_rwp->mode.bits.fast_do_getcpu;
		if (nargs == 1) {
			unsigned getcpu= va_arg(ap, unsigned);  // 4 bytes on both 32 and 64
			traceControl_rwp->mode.bits.fast_do_getcpu= (getcpu != 0);
		}
	} else if ((strncmp(cmd, "lvlmskn", 7) == 0) || (strncmp(cmd, "lvlsetn", 7) == 0) || (strncmp(cmd, "lvlclrn", 7) == 0)) { /* TAKES 2 or 4 args: name,lvlX or name,lvlM,lvlS,lvlT */
		uint64_t lvl, lvlm, lvls, lvlt;
		unsigned ee;
		int op;
		size_t slen= strlen(&cmd[7]);
		char *name_spec;
		if (slen > 1 || (slen == 1 && !strpbrk(&cmd[6], "MST"))) {
			TRACE_PRN("only M,S,or T allowed after lvl...n\n");
			va_end(ap);
			return (-1);
		}

		if (strncmp(&cmd[3], "msk", 3) == 0) {
			op= 0;
		} else if (strncmp(&cmd[3], "set", 3) == 0) {
			op= 1;
		} else  // must be clr
		{
			op= 2;
		}
		name_spec= va_arg(ap, char *);
		/* find first match */
		ee= traceControl_p->num_namLvlTblEnts;
		for (uu= 0; uu < ee; ++uu) {
			if (TRACE_TID2NAME((int32_t)uu)[0] && TMATCHCMP(name_spec, TRACE_TID2NAME((int32_t)uu))) {
				break;
			}
		}
		if (uu == ee) {
			va_end(ap);
			return (0);
		}
		lvl= va_arg(ap, uint64_t);
		switch (cmd[7]) {
		case 'M':
			ret= (long)traceLvls_p[uu].M; /* FIXME - mask is uint64_t (on 32/64 systems), ret val is signed and 32 bits on 32 bit systems */
			for (; uu < ee; ++uu) {
				if (TRACE_TID2NAME((int32_t)uu)[0] && TMATCHCMP(name_spec, TRACE_TID2NAME((int32_t)uu))) {
					trace_msk_op(&traceLvls_p[uu].M, op, lvl);
				}
			}
			break;
		case 'S':
			ret= (long)traceLvls_p[uu].S; /* FIXME - mask is uint64_t (on 32/64 systems), ret val is signed and 32 bits on 32 bit systems */
			for (; uu < ee; ++uu) {
				if (TRACE_TID2NAME((int32_t)uu)[0] && TMATCHCMP(name_spec, TRACE_TID2NAME((int32_t)uu))) {
					trace_msk_op(&traceLvls_p[uu].S, op, lvl);
				}
			}
			break;
		case 'T':
			ret= (long)traceLvls_p[uu].T; /* FIXME - mask is uint64_t (on 32/64 systems), ret val is signed and 32 bits on 32 bit systems */
			for (; uu < ee; ++uu) {
				if (TRACE_TID2NAME((int32_t)uu)[0] && TMATCHCMP(name_spec, TRACE_TID2NAME((int32_t)uu))) {
					trace_msk_op(&traceLvls_p[uu].T, op, lvl);
				}
			}
			break;
		default:
			if (nargs != 4) { /* "name" plus 3 lvls */
				TRACE_PRN("need 3 lvlmsks; %d given\n", nargs - 1);
				va_end(ap);
				return (-1);
			}
			lvlm= lvl; /* arg from above */
			lvls= va_arg(ap, uint64_t);
			lvlt= va_arg(ap, uint64_t);
			for (; uu < ee; ++uu) {
				if (TRACE_TID2NAME((int32_t)uu)[0] && TMATCHCMP(name_spec, TRACE_TID2NAME((int32_t)uu))) {
					trace_msk_op(&traceLvls_p[uu].M, op, lvlm);
					trace_msk_op(&traceLvls_p[uu].S, op, lvls);
					trace_msk_op(&traceLvls_p[uu].T, op, lvlt);
				}
			}
		}
	} else if ((strncmp(cmd, "lvlmsk", 6) == 0) || (strncmp(cmd, "lvlset", 6) == 0) || (strncmp(cmd, "lvlclr", 6) == 0)) { /* TAKES 1 or 3 args: lvlX or lvlM,lvlS,lvlT */
		uint64_t lvl, lvlm, lvls, lvlt;
		unsigned ee, doNew= 1;
		int op;
		if (strncmp(&cmd[3], "msk", 3) == 0) {
			op= 0;
		} else if (strncmp(&cmd[3], "set", 3) == 0) {
			op= 1;
		} else {
			op= 2;
		}
		if ((cmd[6] == 'g') || ((cmd[6]) && (cmd[7] == 'g'))) {
			uu= 0;
			ee= traceControl_p->num_namLvlTblEnts;
		} else if ((cmd[6] == 'G') || ((cmd[6]) && (cmd[7] == 'G'))) {
			uu= 0;
			ee= traceControl_p->num_namLvlTblEnts;
			doNew= 0; /* Capital G short ciruits the "set for future/new trace ids */
		} else {
			uu= (unsigned)traceTID;
			ee= (unsigned)traceTID + 1;
		}
		lvl= va_arg(ap, uint64_t); /* "FIRST" ARG SHOULD ALWAYS BE THERE */
		switch (cmd[6]) {
		case 'M':
			for (; uu < ee; ++uu) {
				if (doNew || TRACE_TID2NAME((int32_t)uu)[0]) {
					trace_msk_op(&traceLvls_p[uu].M, op, lvl);
				}
			}
			break;
		case 'S':
			for (; uu < ee; ++uu) {
				if (doNew || TRACE_TID2NAME((int32_t)uu)[0]) {
					trace_msk_op(&traceLvls_p[uu].S, op, lvl);
				}
			}
			break;
		case 'T':
			for (; uu < ee; ++uu) {
				if (doNew || TRACE_TID2NAME((int32_t)uu)[0]) {
					trace_msk_op(&traceLvls_p[uu].T, op, lvl);
				}
			}
			break;
		default:
			if (nargs != 3) {
				TRACE_PRN("need 3 lvlmsks; %d given\n", nargs);
				va_end(ap);
				return (-1);
			}
			lvlm= lvl; /* "FIRST" arg from above */
			lvls= va_arg(ap, uint64_t);
			lvlt= va_arg(ap, uint64_t);
			for (; uu < ee; ++uu) {
				if (!doNew && !TRACE_TID2NAME((int32_t)uu)[0]) {
					continue;
				}
				trace_msk_op(&traceLvls_p[uu].M, op, lvlm);
				trace_msk_op(&traceLvls_p[uu].S, op, lvls);
				trace_msk_op(&traceLvls_p[uu].T, op, lvlt);
			}
		}
	} else if (strcmp(cmd, "trig") == 0) { /* takes 1 or 2 args: postEntries [lvlmsk] - optional 3rd arg will suppress warnings */
		uint64_t lvlsMsk= 0;
		unsigned long post_entries= 0;
		if (nargs == 1) {
			post_entries= va_arg(ap, unsigned long);  // 4 bytes on 32, 8 on 64
			lvlsMsk= traceLvls_p[traceTID].M;
		} else if (nargs >= 2) {
			post_entries= va_arg(ap, unsigned long);  // 4 bytes on 32, 8 on 64
			lvlsMsk= va_arg(ap, uint64_t);
		}
		if ((traceLvls_p[traceTID].M & lvlsMsk) != lvlsMsk) {
			if (nargs == 2) {
				TRACE_EPRN("Warning: \"trig\" setting (additional) bits (0x%llx) in traceTID=%d\n", (unsigned long long)lvlsMsk, traceTID);
			}
			traceLvls_p[traceTID].M|= lvlsMsk;
		}
		if (traceControl_rwp->trigActivePost && nargs == 2) {
			TRACE_EPRN("Warning: \"trig\" overwriting trigActivePost (previous=%d)\n", traceControl_rwp->trigActivePost);
		}
		traceLvls_p[traceTID].T= lvlsMsk;
		traceControl_rwp->trigActivePost= post_entries ? (uint32_t)post_entries : 1; /* must be at least 1 */
		traceControl_rwp->triggered= 0;
		traceControl_rwp->trigIdxCnt= 0;
	} else if (strcmp(cmd, "reset") == 0) {
		traceControl_rwp->trigIdxCnt= traceControl_rwp->trigActivePost= 0;
		traceControl_rwp->full= 0;
		TRACE_ATOMIC_STORE(&traceControl_rwp->wrIdxCnt, (uint32_t)0);
		traceControl_rwp->triggered= 0;
	} else if (strcmp(cmd, "limit_ms") == 0) { /* 0, 2 or 3 args: limit_cnt, span_on_ms, [span_off_ms] */
		if (nargs == 0) {
			ret= traceControl_rwp->limit_cnt_limit;
		} else if (nargs >= 2 && nargs <= 3) {
			ret= traceControl_rwp->limit_cnt_limit;
			traceControl_rwp->limit_cnt_limit= (uint32_t)va_arg(ap, long);   // 4 bytes on 32, 8 on 64
			traceControl_rwp->limit_span_on_ms= (uint32_t)va_arg(ap, long);  // 4 bytes on 32, 8 on 64
			if (nargs == 3) {
				traceControl_rwp->limit_span_off_ms= (uint32_t)va_arg(ap, long);  // 4 bytes on 32, 8 on 64
			} else {
				traceControl_rwp->limit_span_off_ms= traceControl_rwp->limit_span_on_ms;
			}
		} else {
			TRACE_PRN("limit needs 0 or 2 or 3 args (cnt,span_of[,span_off]) %d given\n", nargs);
			va_end(ap);
			return (-1);
		}
#ifndef __KERNEL__
	} else if (strcmp(cmd, "printfd") == 0) {
		if (nargs) {
			char buf[512]; /* C++ would help here; oh well */
			size_t  oo=0;
			size_t buflen=sizeof(buf);
			if (nargs > 64) nargs=64;
			for (uu=0; uu<(unsigned)nargs; ++uu) {
				int fd=va_arg(ap, int);
				tracePrintFd[uu] = fd;
				/* Could build mirror value for TRACE_PRINT_FD ... */
				if (oo < sizeof(buf)) {
					int xx=snprintf(&buf[oo],buflen-oo,",%d", fd);
					oo += TRACE_SNPRINTED(xx,buflen-oo);
				}
			}
			/* ... and put it in environment. */
			setenv("TRACE_PRINT_FD",&buf[1],1);
			for (    ; uu<64;    ++uu) tracePrintFd[uu]= tracePrintFd[uu - 1];
		}
#endif
	} else {
		TRACE_EPRN("TRACE: invalid control string %s nargs=%d\n", cmd, nargs);
		ret= -1;
	}
	va_end(ap);
	return (ret);
} /* traceCntl */

#if !defined(__KERNEL__) || defined(TRACE_IMPL)

static void trace_created_init(struct traceControl_s *t_p, struct traceControl_rw *t_rwp, uint32_t msgmax, uint32_t argsmax,
							   uint32_t numents, uint32_t namtblents, uint32_t nam_arr_sz, int memlen, unsigned modeM)
{
	trace_tv_t tv;
	TRACE_GETTIMEOFDAY(&tv);
#	ifdef TRACE_DEBUG_INIT
	trace_user(&tv, traceControl_p->num_namLvlTblEnts-1, 0, "", __FILE__, __LINE__, __func__, 1, "trace_created_init: tC_p=%p", t_p);
#	endif
	strncpy(t_p->version_string, TRACE_REV, sizeof(t_p->version_string));
	t_p->version_string[sizeof(t_p->version_string) - 1]= '\0';
	t_p->create_tv_sec= (uint32_t)tv.tv_sec;
	t_p->num_params= argsmax;
	t_p->siz_msg= msgmax;
	t_p->siz_entry= TRACE_entSiz(msgmax, argsmax);
	t_p->num_entries= numents;
	t_p->largest_multiple= (uint32_t)-1 - ((uint32_t)-1 % numents);
	t_p->largest_zero_offset= ((uint32_t)-1 % numents) + 1; /* used in DELTA. largest_multiple+largest_zero_offset=0 (w/ rollover) */
	t_p->num_namLvlTblEnts= namtblents;
	t_p->nam_arr_sz= ((nam_arr_sz) + 7) & (unsigned)~7;
	t_p->memlen= (uint32_t)memlen;

	TRACE_ATOMIC_STORE(&t_rwp->namelock, (uint32_t)0);

	/*TRACE_CNTL( "reset" );  Can't call traceCntl during Init b/c it does an INIT_CHECK and will call Init */
	TRACE_ATOMIC_STORE(&t_rwp->wrIdxCnt, (uint32_t)0);
	t_rwp->trigIdxCnt= t_rwp->trigActivePost= 0;
	t_rwp->full= t_rwp->triggered= 0;
	t_rwp->limit_span_on_ms= t_rwp->limit_span_off_ms= t_rwp->limit_cnt_limit= 0;

	t_rwp->mode.words.cntl= 0;
	t_rwp->mode.words.mode= 0;
	t_rwp->mode.bits.M= (modeM != 0);
	t_rwp->mode.bits.S= 1;
	t_rwp->mode.bits.fast_do_getcpu = 0; /* only checked if __arm__; default to false as syscall is heavy weight. Turn on via tcntl OR use kernel module. */

	traceLvls_p= (struct traceLvls_s *)(t_rwp + 1);
	traceNams_p= (char*)(&traceLvls_p[t_p->num_namLvlTblEnts]);
	traceControl_p= t_p;
	/* this depends on the actual value of the num_namLvlTblEnts which
	   may be different from the "calculated" value WHEN the buffer has
	   previously been configured */
	traceEntries_p= (struct traceEntryHdr_s *)((unsigned long)traceLvls_p + TRACE_namtblSiz(t_p->num_namLvlTblEnts, t_p->nam_arr_sz));

	traceInitNames(t_p, t_rwp);

	t_p->trace_initialized= 1;
#	ifdef TRACE_DEBUG_INIT
	tv.tv_sec= 0;
	trace_user(&tv, traceControl_p->num_namLvlTblEnts-1, 0, "", __FILE__, __LINE__, __func__, 1, "trace_created_init: tC_p=%p", t_p);
#	endif
} /* trace_created_init */

#endif

#ifndef __KERNEL__

/* This is currently (as of Nov, 2017) used to build a file name from
   TRACE_FILE. Currently not applicable for __KERNEL__ (module or in-source)
   which always creates the virtual file at /proc/trace/buffer.
 */
static char *tsnprintf(char *obuf, size_t bsz, const char *input)
{
	size_t outoff, ii;
	const char *inp= input;
	char loguid[32];
	char hstnam[256]; /* man gethostname - SUSv2 guarantees that "Host names
                             are limited to 255 bytes".  POSIX.1-2001 guarantees
                             that "Host names (not including the terminating
                             null byte) are limited to HOST_NAME_MAX bytes".
                             On  Linux, HOST_NAME_MAX is defined with the value
                             64, which has been the limit since Linux 1.0 */
	char *cp_uname= NULL, *cp_uid= NULL, *cp_hostname= NULL;

	for (outoff= 0; outoff < bsz && *inp != '\0'; ++inp) {
		if (*inp == '%') {
			++inp;
			switch (*inp) {
			case '%':
				obuf[outoff++]= *inp;
				break;
			case 'u':
				/*  ......................now stop at first non-NULL */
				if (cp_uname == NULL && ((cp_uname= getenv("USER")) == NULL && (cp_uname= getenv("LOGNAME")) == NULL && (cp_uname= getenv("USERNAME")) == NULL)) {
					cp_uname= (char *)"";
				}
				for (ii= 0; outoff < bsz && cp_uname[ii] != '\0'; ++ii) {
					obuf[outoff++]= cp_uname[ii];
				}
				break;
			case 'U':
				if (cp_uid == NULL) {
					sprintf(loguid, "%u", getuid());
					cp_uid= loguid;
				}
				for (ii= 0; outoff < bsz && cp_uid[ii] != '\0'; ++ii) {
					obuf[outoff++]= cp_uid[ii];
				}
				break;
			case 'h':
				if (cp_hostname == NULL) {
					cp_hostname= getenv("HOSTNAME");
					if (cp_hostname == NULL || *cp_hostname == '\0') { /* HOSTNAME='' is invalid */
						/* try gethostname */
						char *period;
						(void)gethostname(hstnam,sizeof(hstnam)); /* not EFAULT, EINVAL, or ENAMETOOLONG */
						if ((period=strchr(hstnam,'.'))) *period= '\0';  /* alway return "short" hostname */
						cp_hostname= hstnam;
					}
				}
				for (ii= 0; outoff < bsz && cp_hostname[ii] != '\0'; ++ii) {
					obuf[outoff++]= cp_hostname[ii];
				}
				break;
			case '\0':
				obuf[outoff++]= '%';
				--inp; /* let for loop exit test see this */
				break;
			default:
				/* just put them both out if there is space in obuf */
				obuf[outoff++]= '%';
				if (outoff < bsz) {
					obuf[outoff++]= *inp;
				}
			}
		} else {
			obuf[outoff++]= *inp;
		}
	}
	if (outoff >= bsz) {
		obuf[bsz - 1]= '\0';
	} else {
		obuf[outoff]= '\0';
	}
	return (obuf);
} /* tsnprintf */

/* RETURN "created" status */
static int trace_mmap_file(const char *_file, int *memlen /* in/out -- in for when file created, out when not */
						   ,
						   struct traceControl_s **tC_p, struct traceControl_rw **tC_rwp /* out */
						   ,
						   uint32_t msgmax, uint32_t argsmax, uint32_t numents, uint32_t namtblents, uint32_t namemax /* all in for when file created */
						   ,
						   int allow_ro)
{
	int fd;
	struct traceControl_s *controlFirstPage_p= NULL;
	struct traceControl_rw *rw_rwp;
	off_t off;
	char path[PATH_MAX];
	int created= 0;
	int stat_try= 0;
	ssize_t quiet_warn= 0;
	int prot_flags= PROT_READ | PROT_WRITE;

	(void)tsnprintf(path, PATH_MAX, _file);                       /* resolves any %u, etc, in _file */
	if ((fd= open(path, O_RDWR | O_CREAT | O_EXCL, 0666)) != -1)  //NOLINT
	{                                                             /* successfully created new file - must init */
		uint8_t one_byte= '\0';
		off= lseek(fd, (*memlen) - 1, SEEK_SET);
		if (off == (off_t)-1) {
			perror("lseek");
			*tC_p= &(traceControl[0]);
			*tC_rwp= &(traceControl[0].rw);
			unlink(path);
			return (0);
		}
		quiet_warn+= write(fd, &one_byte, 1);
		if (quiet_warn < 0) {
			perror("writeOneByte");
		}
		created= 1;
	} else { /* There's an existing file... map 1st page ro */
		struct stat statbuf;
		int try_= 3000;
		/* must verify that it already exists */
		fd= open(path, O_RDWR);  //NOLINT
		if (fd == -1) {
			if (allow_ro) {
				/* try read-only for traceShow */
				fd= open(path, O_RDONLY);  //NOLINT
			}
			if (fd == -1) {
				fprintf(stderr, "TRACE: open(%s)=%d errno=%d pid=%d\n", path, fd, errno, tracePid);
				*tC_p= &(traceControl[0]);
				*tC_rwp= &(traceControl[0].rw);
				return (0);
			}
			prot_flags= PROT_READ;
		}
		/*printf( "trace_mmap_file - fd=%d\n",fd );*/ /*interesting in multithreaded env.*/
		if (fstat(fd, &statbuf) == -1) {
			perror("fstat");
			close(fd);
			*tC_p= &(traceControl[0]);
			*tC_rwp= &(traceControl[0].rw);
			return (0);
		}
		while (statbuf.st_size < (off_t)sizeof(struct traceControl_s)) {
			/* fprintf(stderr, "stat again\n"); This can/will happen in multiprocess env -- so not an error */
			if (((stat_try++ >= 30) && (fprintf(stderr, "too many stat tries\n"), 1)) || ((fstat(fd, &statbuf) == -1) && (perror("fstat"), 1))) {
				close(fd);
				*tC_p= &(traceControl[0]);
				*tC_rwp= &(traceControl[0].rw);
				return (0);
			}
		}

		controlFirstPage_p= (struct traceControl_s *)mmap(NULL, TRACE_PAGESIZE, PROT_READ, MAP_SHARED, fd, 0);
		if (controlFirstPage_p == (struct traceControl_s *)-1) {
			perror("mmap(NULL,TRACE_PAGESIZE,PROT_READ,MAP_SHARED,fd,0) error");
			*tC_p= &(traceControl[0]);
			*tC_rwp= &(traceControl[0].rw);
			return (0);
		}
		while (try_--) {
			if (controlFirstPage_p->trace_initialized != 1) {
				if (try_ == 0) {
					printf("Trace file not initialzed; consider (re)moving it.\n");
					close(fd);
					*tC_p= &(traceControl[0]);
					*tC_rwp= &(traceControl[0].rw);
					return (0);
				}
				if (try_ == 1) {
					sleep(1); /* just sleep a crazy amount the last time */
				}
			} else {
				break;
			}
		}
		/*sleep(1);*/
		*memlen= (int)controlFirstPage_p->memlen;
	}

	/* ??? OLD:I MUST allocate/grab a contiguous vm address space! [in testing threads (where address space
	   is shared (obviously)), thread creation allocates vm space which can occur between
	   these two calls]
	   ???
	   NEW: mmap from rw portion on */
	rw_rwp= (struct traceControl_rw *)mmap(NULL, (size_t)((*memlen) - TRACE_PAGESIZE), prot_flags, MAP_SHARED, fd, TRACE_PAGESIZE);
	if (rw_rwp == (void *)-1) {
		perror("Error:mmap(NULL,(*memlen)-TRACE_PAGESIZE,PROT_READ[|PROT_WRITE],MAP_SHARED,fd,TRACE_PAGESIZE)");
		printf("(*memlen)=%d errno=%d\n", (*memlen) - TRACE_PAGESIZE, errno);
		close(fd);
		*tC_p= &(traceControl[0]);
		*tC_rwp= &(traceControl[0].rw);
		return (0);
	}

	if (created) {
		/* need controlFirstPage_p RW temporarily */
		controlFirstPage_p= (struct traceControl_s *)mmap(NULL, TRACE_PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (controlFirstPage_p == (struct traceControl_s *)-1) {
			perror("mmap(NULL,sizeof(struct traceControl_s),PROT_READ,MAP_SHARED,fd,0) error");
			munmap(rw_rwp, (size_t)((*memlen) - TRACE_PAGESIZE));
			close(fd);
			*tC_p= &(traceControl[0]);
			*tC_rwp= &(traceControl[0].rw);
			return (0);
		}
		/* In this "created" case, tC_rwp will point to traceControl_rwp which needs to be set 
		   for trace_created_init to call traceInitNames which calls trace_name2TID */
		*tC_rwp= rw_rwp; /* as per above comment, we have to assume we will succeed with the remap */
		trace_created_init(controlFirstPage_p, rw_rwp, msgmax, argsmax, numents, namtblents, namemax, *memlen, 1);
		/* Now make first page RO */
		munmap(controlFirstPage_p, TRACE_PAGESIZE);
#	define TRACE_MM_FLAGS MAP_SHARED /*|MAP_FIXED*/
		controlFirstPage_p= (struct traceControl_s *)mmap(NULL, TRACE_PAGESIZE, PROT_READ, TRACE_MM_FLAGS, fd, 0);
		if (controlFirstPage_p == (struct traceControl_s *)-1) {
			perror("Error:mmap(NULL,TRACE_PAGESIZE,PROT_READ," TRACE_STR(TRACE_MM_FLAGS) ",fd,0)");
			printf("(*memlen)=%d errno=%d\n", (*memlen), errno);
			munmap(rw_rwp, (size_t)((*memlen) - TRACE_PAGESIZE));
			close(fd);
			*tC_p= &(traceControl[0]);
			*tC_rwp= &(traceControl[0].rw);
			return (0);
		}
	}

	traceLvls_p= (struct traceLvls_s *)(rw_rwp + 1);
	traceNams_p= (char*)(&traceLvls_p[controlFirstPage_p->num_namLvlTblEnts]);
	traceEntries_p= (struct traceEntryHdr_s *)((unsigned long)traceLvls_p + TRACE_namtblSiz(controlFirstPage_p->num_namLvlTblEnts, controlFirstPage_p->nam_arr_sz));

	*tC_rwp= rw_rwp;
	*tC_p= controlFirstPage_p;

	/* The POSIX mmap man page says:
	   The mmap() function shall add an extra reference to the file
	   associated with the file descriptor fildes which is not removed by a
	   subsequent  close() on that file descriptor.  This reference shall
	   be removed when there are no more mappings to the file.
	*/
	close(fd);
	return (created);
} /* trace_mmap_file */

#endif /* not __KERNEL__*/

#if !defined(__KERNEL__) || defined(TRACE_IMPL)

static int traceInit(const char *_name, int allow_ro)
{
	int memlen;
	uint32_t msgmax_, argsmax_, numents_, namtblents_, nammax_;
	const char *cp;
#	ifndef __KERNEL__
	uint64_t trace_lvl_off, lvlM_lcl=0;
	char *lvlM_endptr;
	int sts;
	int activate= 0;
	const char *_file;
	int traceControl_p_was_NULL= 0;

	if (!trace_lock(&traceInitLck)) {
		TRACE_PRN("trace_lock: InitLck hung?\n");
	}
#		if !defined(__KERNEL__) && defined(TRACE_DEBUG_INIT)
	fprintf(stderr,"traceInit(debug:A): tC_p=%p static=%p _name=%p Tid=%d TrcId=%d\n", (void*)traceControl_p, (void*)traceControl_p_static, _name, traceTid, traceTID);
#		endif
	if (traceControl_p == NULL) {
#		if defined(__GLIBC_PREREQ)   // use this to indicate if we are in a GNU env
		char buf[64], *minor;
		size_t sz=confstr(_CS_GNU_LIBC_VERSION,buf,sizeof(buf)-1);
		if (sz > 6 && sz<=(sizeof(buf)-1) && (minor=strchr(buf,'.'))){
			// have string like "glibc 2.31"
			int32_t trace_libc_version = (atoi(&buf[6])<<16) + atoi(minor+1);
			//printf("trace_libc_version=0x%x\n",trace_libc_version);
			if (trace_libc_version >= ((2<<16) + 27)){     // starting at glibc 2.27
				trace_no_pid_check=1;
				__register_atfork(NULL, NULL, trace_pid_atfork);
			}
		}
#		else
		/* This stuff should happen once (per TRACE_DEFINE compilation module) */
		TRACE_REGISTER_ATFORK;
#		endif
		traceControl_p_was_NULL= 1;

		/* test for activation. (See below for _name override/default) */
		if (_name != NULL) {
			/* name is specified in module, which "wins" over env, but does not "activate" */
			const char *scratch_name;
			if ((scratch_name= getenv("TRACE_NAME")) && (*scratch_name != '\0')) {
				activate= 1;
			}
		} else if ((_name= getenv("TRACE_NAME")) && (*_name != '\0')) {
			/* name not specified in module. But TRACE_NAME is set in the env and it is non-"" */
			activate= 1;
		}

		if (!((_file= getenv("TRACE_FILE")) && (*_file != '\0') && (activate= 1))) {
			_file= traceFile;
		}
		if ((cp= getenv("TRACE_ARGSMAX")) && (*cp) && (activate= 1)) {
			argsmax_= (uint32_t)strtoul(cp, NULL, 0);
		} else {
			argsmax_= TRACE_DFLT_MAX_PARAMS;
		}
		/* use _MSGMAX='' so exe won't override and _MSGMAX won't activate; use _MSGMAX=0 to activate with default MAX_MSG */
		((cp= getenv("TRACE_MSGMAX")) && (*cp) && (activate= 1) && (msgmax_= (uint32_t)strtoul(cp, NULL, 0))) || (msgmax_= TRACE_DFLT_MAX_MSG_SZ);
		((cp= getenv("TRACE_NUMENTS")) && (numents_= (uint32_t)strtoul(cp, NULL, 0)) && (activate= 1)) || (numents_= TRACE_DFLT_NUM_ENTRIES);
		((cp= getenv("TRACE_NAMTBLENTS")) && (namtblents_= (uint32_t)strtoul(cp, NULL, 0)) && (activate= 1)) || (namtblents_= TRACE_DFLT_NAMTBL_ENTS);
		((cp= getenv("TRACE_NAMEMAX")) && (nammax_= (uint32_t)strtoul(cp, NULL, 0)) && (activate= 1)) || (nammax_= TRACE_DFLT_NAM_CHR_MAX + 1);
		((cp= getenv("TRACE_LVLM")) && (lvlM_lcl= strtoull(cp, &lvlM_endptr, 0)) && (activate= 1)); /* activate if non-zero */

		/* TRACE_LVLSTRS, TRACE_LVLS and TRACE_PRINT_FD can be used when active or inactive.
		   See also processing in bitN_to_mask script. */
		if ((cp= getenv("TRACE_LVLSTRS")) && (*cp)) {
			unsigned lvlidx= 0, dbgidx;
			unsigned tmp_lvlwidth= 0;  // in case the lvlwidth actually decreases
			/* parse, for example: fatal,alert,crit,error,warn,notice,info,log,debug,dbg01 NOTE: no escape sequences */
			size_t ll= strcspn(cp, ",");
			while (*cp && lvlidx < 64) {
				if (ll) {
					strncpy(trace_lvlstrs[0][lvlidx], cp, TRACE_MIN(ll, sizeof(trace_lvlstrs[0][0]) - 1));
					trace_lvlstrs[0][lvlidx][TRACE_MIN(ll, sizeof(trace_lvlstrs[0][0]) - 1)]= '\0';
					cp+= ll;
				}
				if ((ll= strlen(trace_lvlstrs[0][lvlidx++])) > tmp_lvlwidth) tmp_lvlwidth= (unsigned)ll;
				if (*cp == ',') ++cp;
				ll= strcspn(cp, ",");
			}
			/* look at last/previous */
            if ( !lvlidx ) ++lvlidx;
			cp= strpbrk(trace_lvlstrs[0][lvlidx - 1], "0123456789");
			if (cp && sscanf(cp, "%u", &dbgidx)) {
				char tmp[16];
				long ii= (cp - trace_lvlstrs[0][lvlidx - 1]); /* length of the non-numeric part (i.e. "template") */
				strncpy(tmp, trace_lvlstrs[0][lvlidx - 1], (size_t)ii);       /* reset to the beginning of the "template" */
				for (; lvlidx < 64; ++lvlidx) {
					snprintf(trace_lvlstrs[0][lvlidx], sizeof(trace_lvlstrs[0][0]), "%.*s%02u", (int)ii, tmp, ++dbgidx);
					//trace_lvlstrs[0][lvlidx][sizeof(trace_lvlstrs[0][0])-1] = '\0'; SHOULD NOT be needed
					if ((ll= strlen(trace_lvlstrs[0][lvlidx])) > tmp_lvlwidth) tmp_lvlwidth= (unsigned)ll;
				}
			} else
				for (; lvlidx < 64; ++lvlidx) {
					if ((ll= strlen(trace_lvlstrs[0][lvlidx])) > tmp_lvlwidth) tmp_lvlwidth= (unsigned)ll;
				}
			trace_lvlwidth= tmp_lvlwidth;  // should be the correct answer
		}
		if ((cp= getenv("TRACE_LVLCOLORS")) && (*cp)) {
			unsigned lvlidx= 0, onoff= 0;
			/* parse, for example: fatal,error,warn,info,log,debug,dbg01 NOTE: no escape sequences */
			size_t ll= strcspn(cp, ",");
			while (*cp && lvlidx < 64) {
				if (ll) {
					strncpy(trace_lvlcolors[lvlidx][onoff], cp, TRACE_MIN(ll, sizeof(trace_lvlcolors[0][0])));
					trace_lvlcolors[lvlidx][onoff][TRACE_MIN(ll, sizeof(trace_lvlcolors[0][0]) - 1)]= '\0';
					cp+= ll;
				}
				if (*cp == ',') ++cp;
				ll= strcspn(cp, ",");

				if ((onoff= !onoff) == 0) ++lvlidx;
			}
		}
		if ((cp= getenv("TRACE_PRINT_FD")) && (*cp)) {
			sts= sscanf(cp,
						"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
						"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
						"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
						"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
						&tracePrintFd[0], &tracePrintFd[1], &tracePrintFd[2], &tracePrintFd[3],
						&tracePrintFd[4], &tracePrintFd[5], &tracePrintFd[6], &tracePrintFd[7],
						&tracePrintFd[8], &tracePrintFd[9], &tracePrintFd[10], &tracePrintFd[11],
						&tracePrintFd[12], &tracePrintFd[13], &tracePrintFd[14], &tracePrintFd[15],
						&tracePrintFd[16], &tracePrintFd[17], &tracePrintFd[18], &tracePrintFd[19],
						&tracePrintFd[20], &tracePrintFd[21], &tracePrintFd[22], &tracePrintFd[23],
						&tracePrintFd[24], &tracePrintFd[25], &tracePrintFd[26], &tracePrintFd[27],
						&tracePrintFd[28], &tracePrintFd[29], &tracePrintFd[30], &tracePrintFd[31],
						&tracePrintFd[32], &tracePrintFd[33], &tracePrintFd[34], &tracePrintFd[35],
						&tracePrintFd[36], &tracePrintFd[37], &tracePrintFd[38], &tracePrintFd[39],
						&tracePrintFd[40], &tracePrintFd[41], &tracePrintFd[42], &tracePrintFd[43],
						&tracePrintFd[44], &tracePrintFd[45], &tracePrintFd[46], &tracePrintFd[47],
						&tracePrintFd[48], &tracePrintFd[49], &tracePrintFd[50], &tracePrintFd[51],
						&tracePrintFd[52], &tracePrintFd[53], &tracePrintFd[54], &tracePrintFd[55],
						&tracePrintFd[56], &tracePrintFd[57], &tracePrintFd[58], &tracePrintFd[59],
						&tracePrintFd[60], &tracePrintFd[61], &tracePrintFd[62], &tracePrintFd[63]);
			if ((sts >= 1) && (sts < 64)) {
				int ii;
				for (ii= sts; ii < 64; ++ii)
					tracePrintFd[ii]= tracePrintFd[sts - 1];
			}
		}
		if ((cp= getenv("TRACE_PRINT")) != NULL) {
			tracePrint_cntl= cp;
			if (strlen(tracePrint_cntl) > 200) { /* cannot see how this could be OK/desirable */
				fprintf(stderr, "Invalid TRACE_PRINT environment variable value.\n");
				tracePrint_cntl= TRACE_PRINT__; /* assume this (potentially user supplied) is more valid */
			}
			if (*tracePrint_cntl == '\0') /* incase someone does export TRACE_PRINT= */
				tracePrint_cntl= "%m";
		} else
			tracePrint_cntl= TRACE_PRINT__;
		if ((cp= trace_strflg(tracePrint_cntl, 'F'))) {
			unsigned uu= 0;
			trace_vtrace_cntl.prepend_func= 1;
			while (*++cp && *cp != '%' && uu < (sizeof(trace_vtrace_cntl.sep) - 1))
				trace_vtrace_cntl.sep[uu++]= *cp;
			trace_vtrace_cntl.sep[uu]= '\0';
		}

		/* I want nammax_ to be a multiple of 8 bytes */
		if (nammax_ < 8) nammax_= 8;
		nammax_= (nammax_ + 7) & (unsigned)~7;
		if (nammax_ > TRACE_TN_BUFSZ) nammax_= TRACE_TN_BUFSZ & (unsigned)~7;

		if (!activate) {
			traceControl_rwp= &(traceControl[0].rw);
			traceControl_p= &(traceControl[0]);
		} else {
			if (namtblents_ == 1) {
				namtblents_= 2; /* If it has been specified in the env. it should be at least 2 */
			}
			memlen= (int)traceMemLen(TRACE_cntlPagesSiz(), namtblents_, nammax_, msgmax_, argsmax_, numents_);
			if ((traceControl_p_static != NULL) && (strcmp(traceFile_static, _file) == 0)) {
				traceControl_p= traceControl_p_static;
			} else {
				trace_mmap_file(_file, &memlen, &traceControl_p, &traceControl_rwp, msgmax_, argsmax_, numents_, namtblents_, nammax_, allow_ro);
			}
		}

		/* trace_mmap_file may have failed */
		if (traceControl_p == &(traceControl[0])) {
#		define TRACE_DISABLED_ENTS 1
			trace_created_init(traceControl_p, traceControl_rwp, msgmax_, argsmax_,
							   TRACE_DISABLED_ENTS /*numents_*/,
			                   (uint32_t)((sizeof(traceControl) - sizeof(traceControl[0]) /* size for everything - sizeof traceControl = size for namtblents and entries */
							               - TRACE_DISABLED_ENTS * TRACE_entSiz(msgmax_, argsmax_)) /* - size for entries = (leaves) size for namtblents */
							              / (sizeof(struct traceLvls_s) + nammax_)), /* / sizeof individual namtblent = num_namtblents_ */
							   nammax_,
							   sizeof(traceControl) /*memlen*/, 0 /*modeM*/);
		} else {
			if (traceControl_p_static == NULL) {
				strcpy(traceFile_static, _file);  //NOLINT
				traceControl_p_static= traceControl_p;
			}
		}
	} // (traceControl_p == NULL) -- once per process

	if (_name == NULL) {
		if (!((_name= getenv("TRACE_NAME")) && (*_name != '\0'))) {
			_name= traceName;
		}
	}

	traceTID= (int)trace_name2TID(_name);
	/* Now that the critical variables
	   (traceControl_p, traceLvls_p, traceNams_p, traceEntries_p) and even traceTID are
	   set, it's OK to indicate that the initialization is complete */
	if (traceTid == 0) /* traceInit may be called w/ or w/o checking traceTid */
	{
		tracePid= getpid(); /* do/re-do -- it may be forked process */
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#			pragma GCC diagnostic push
#			pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#		endif
		traceTid= trace_gettid();
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#			pragma GCC diagnostic pop
#		endif
	}
#		ifdef TRACE_DEBUG_INIT
	printf("traceInit(debug:Z): tC_p=%p static=%p _name=%p Tid=%d TrcId=%d\n", (void*)traceControl_p, (void*)traceControl_p_static, _name, traceTid, traceTID);
#		endif
	trace_unlock(&traceInitLck);

	if (traceControl_p_was_NULL)
	{	/* This stuff gets done once per process */
		if ((cp= getenv("TRACE_LVLS")) && (*cp)) /* TRACE_TLVLM above as it "activates" */
		{                                        /* Note "g" in lvl*Mg -- TRACE_NAME not needed */
			char *endptr;               /* 2 formats: 1) TRACE_LVLS=<mask>, 2) TRACE_LVLS=<set>,<clr>  clr first, then set */
			trace_lvlS= strtoull(cp, &endptr, 0); /* set for future new traceTIDs (from this process) regardless of cmd line tonSg or toffSg - if non-zero! */
			if (endptr != cp && *endptr == ',') {
				trace_lvl_off= strtoull(endptr + 1, NULL, 0);
				TRACE_CNTL("lvlclrSg", trace_lvl_off);
				TRACE_CNTL("lvlsetSg", trace_lvlS);
				trace_lvlS = 0; /* Do not "msk" with this "set" value in trace_name2TID */
			} else
				TRACE_CNTL("lvlmskSg", trace_lvlS);
		}
		if (lvlM_lcl) /* env "activate" is above -- Note "g" in lvl*Mg -- TRACE_NAME not needed */
		{               /* all current and future (until cmdline tonMg/toffMg) (and new from this process regardless of cmd line tonSg or toffSg) */
			if (*lvlM_endptr == ',') { /* NOTE: lvlM_endptr is only set (above) when traceControl_p_was_NULL */
				trace_lvl_off= strtoull(lvlM_endptr + 1, NULL, 0);
				TRACE_CNTL("lvlclrMg", trace_lvl_off);
				TRACE_CNTL("lvlsetMg", lvlM_lcl);
			} else {
				TRACE_CNTL("lvlmskMg", lvlM_lcl);
				trace_lvlM = lvlM_lcl; /* save this "msk" value*/
			}
		}
		trace_namLvlSet(); /* more env vars checked - I want this to be done once,
							  but after TRACE_LVLS and/or TRACE_LVLM processing.
						      This is b/c this processing can be more specific than */
	}

#	else /* ifndef __KERNEL__ */

	msgmax_= msgmax;         /* module_param */
	argsmax_= argsmax;       /* module_param */
	numents_= numents;       /* module_param */
	namtblents_= namtblents; /* module_param */
	nammax_= namemax;        /* module_param */
	if (nammax_ < 8) nammax_= 8;
	nammax_= (nammax_ + 7) & (unsigned)~7;
	if (nammax_ > TRACE_TN_BUFSZ) nammax_= TRACE_TN_BUFSZ & (unsigned)~7;
	//printk("numents_=%d msgmax_=%d argsmax_=%d namtblents_=%d\n", numents_, msgmax_, argsmax_, namtblents_); // CAN ONLY BE DONE IF module or after console_init
	memlen= traceMemLen(TRACE_cntlPagesSiz(), namtblents_, nammax_, msgmax_, argsmax_, numents_);
	traceControl_p= (struct traceControl_s *)vmalloc_node(memlen, trace_buffer_numa_node);
	traceControl_rwp= (struct traceControl_rw *)((unsigned long)traceControl_p + TRACE_PAGESIZE);
	trace_created_init(traceControl_p, traceControl_rwp, msgmax_, argsmax_, numents_, namtblents_, nammax_, memlen, 1);
	if (_name == NULL) _name= traceName;
	traceTID= trace_name2TID(_name);
	if (trace_print[0] != '\0') {
		tracePrint_cntl= trace_print;
	} else
		tracePrint_cntl= TRACE_PRINT__;
	if (*tracePrint_cntl == '\0') /* incase someone does trace_print="" */
		tracePrint_cntl= "%m";
	if ((cp= trace_strflg(tracePrint_cntl, 'F'))) {
		unsigned uu= 0;
		trace_vtrace_cntl.prepend_func= 1;
		while (*++cp && *cp != '%' && uu < (sizeof(trace_vtrace_cntl.sep) - 1))
			trace_vtrace_cntl.sep[uu++]= *cp;
		trace_vtrace_cntl.sep[uu]= '\0';
	}
#	endif

	return (0);
} /* traceInit */

/* traceInitNames requires:
   traceControl_p, (for TRACE_TID2NAME and idx2namLvlsPtr need nam_arr_sz (i.e. traceControl_p->nam_arr_sz))
   traceNams_p, and
   traceEntries_p  (for trace_name2TID)
 */
static void traceInitNames(struct traceControl_s *tC_p, struct traceControl_rw *tC_rwp)
{
	unsigned ii;
	int32_t TrcId;
	for (ii= 0; ii < tC_p->num_namLvlTblEnts; ++ii) {
		TRACE_TID2NAME((int32_t)ii)[0]= '\0';
		traceLvls_p[ii].M= TRACE_DFLT_LVLM; /* As Name/TIDs can't go away, these are */
		traceLvls_p[ii].S= TRACE_DFLT_LVLS; /* then defaults except for trace_lvlS/trace_lvlM */
		traceLvls_p[ii].T= 0;               /* in trace_name2TID. */
	}                                                    /* (0 for err, 1=warn, 2=info, 3=debug) */
	// if hashing the name, special considerations need to me made -- see trace_name2TID(nn)
	//strcpy(TRACE_TID2NAME((int32_t)tC_p->num_namLvlTblEnts - 2), "TRACE");    //NOLINT
	TrcId= (int32_t)tC_p->num_namLvlTblEnts - 1;
	strcpy(TRACE_TID2NAME(TrcId), "_TRACE_");  //NOLINT - do first, before any hashing
	tC_rwp->longest_name= 7;
#	ifdef __KERNEL__
	TrcId= (int32_t)trace_name2TID("KERNEL");
	/* like userspace TRACE_LVLS env.var - See also trace_name2TID */
	if (trace_lvlS)
		traceLvls_p[TrcId].S= trace_lvlS;
	if (trace_lvlM)
		traceLvls_p[TrcId].M= trace_lvlM;
#	endif
	trace_name2TID("TRACE");	/* make sure TRACE is in the table as some apps may expect this */
} /* traceInitNames */

#endif /* !defined(__KERNEL__) || defined(TRACE_IMPL) */

static inline struct traceEntryHdr_s *idxCnt2entPtr(uint32_t idxCnt)
{
	uint32_t idx;
	off_t off;
	uint32_t num_entries= traceControl_p->num_entries;
	idx= idxCnt % num_entries;
	off= (off_t)(idx * traceControl_p->siz_entry);
	return (struct traceEntryHdr_s *)((char *)traceEntries_p + off);
} /* idxCnt2entPtr */

static inline char *idx2namsPtr(int32_t idx) /* formerly idx2namLvlsPtr(int32_t idx) */
{
	size_t off;
	off= (size_t)traceControl_p->nam_arr_sz * (size_t)idx;
	return (traceNams_p + off);
}

#ifdef __cplusplus

/* The Streamer instance will be temporary. No class statics can be used --
   The goal is statics for each TRACE/TLOG and a class static won't do.
   A lambda expression can by used to hold/create statics for each TLOG.
   "static int tid_" is thread safe in so far as multiple threads may init,
   but will init with same value.
*/

/* fmtnow can be:
   1 = do msg formating in streamer for both slow and/or mem
   0 = format in streamer for slow and mem if slow path enabled
  -1 = don't format in streamer, even if slow enabled -- pass args to mem and slow output function. Only available via TLOG2(...)
*/
struct tstreamer_flags {
	unsigned do_m : 1;
	unsigned do_s : 1;
	int fmtnow : 2;
	tstreamer_flags()
		: do_m(0), do_s(0), fmtnow(0) {}
};
typedef struct
{
	int tid;
	limit_info_t info;
} tinfo_t;

#	if (__cplusplus >= 201103L)
#		define TRACE_GET_STATIC() [&]() {static TRACE_THREAD_LOCAL tinfo_t info={-1,{0,lsFREE,0}}; return &info; }()
#	else
#		define TRACE_GET_STATIC() ({static TRACE_THREAD_LOCAL tinfo_t info={-1,{0,lsFREE,0}};        &info; })
#	endif

// Use C++ "for" statement to create single statement scope for key (static) variable that
// are initialized and then, if enabled, passed to the Streamer class temporary instances.
// arg1   - lvl;
// arg2   - the TSTREAMER_T_ method used to set the "lvl,nam,fmt" or "name,fmt" members/flag
//          note: fmtnow can be used to force formatting env if Memory only;
// arg3   - force_s = force_slow - override tlvlmskS&lvl==0
// NOTE: I use the gnu extension __PRETTY_FUNCTION__ as opposed to __func__ b/c in C++ a method/function can have different arguments
// NOTE: any temporary (created within the for statement) std::string seems will
//       only be destroyed at the end of the comma (',') separated statement. This
//       allows saving the address in .nn and using .nn later. Because this
//       is not the case for TLOG_ENTEX, so the string needs to be copied. This
//       means, for TRACE_STREAMER, the string is copied superfluously.
#	ifndef TRACE_USE_STATIC_STREAMER
#		define TRACE_USE_STATIC_STREAMER 1
#	endif
#   define _PRAGMA(xx) /*_Pragma(xx)*/
#	if TRACE_USE_STATIC_STREAMER == 1
//                   args are: lvl, lvl/name/fmtnow_method, s_force
#		define TRACE_STREAMER(_lvl, lvnafm_nafm_method, force_s)	\
			for (TSTREAMER_T_ _trc_((tlvle_t)(_lvl), TRACE_GET_STATIC()); \
				 _trc_.once && TRACE_INIT_CHECK( trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn)) ) \
					 && (_trc_.lvnafm_nafm_method, ((*_trc_.tidp != -1) || ((*_trc_.tidp= trace_tlog_name_(_trc_.nn,__TRACE_FILE__,__FILE__,_trc_.tn,sizeof(_trc_.tn))) != -1))) \
					 && trace_do_streamer(&_trc_); \
				 _trc_.once=0, ((TraceStreamer *)_trc_.stmr__)->str())	\
				_PRAGMA("GCC diagnostic ignored \"-Wunused-value\"") \
					*((TraceStreamer *)(_trc_.stmr__= (void *)&((TraceStreamer *)_trc_.stmr__)->init(*_trc_.tidp, (uint8_t)(_trc_.lvl), _trc_.flgs, __FILE__, __LINE__, __PRETTY_FUNCTION__, &_trc_.tv, _trc_.ins, &TRACE_LOG_FUNCTION)))
#	else
#		define TRACE_STREAMER(_lvl, lvnafm_nafm_method, force_s)                                                                                                                                                                                                                                                                                                                                          \
	for (TSTREAMER_T_ _trc_((tlvle_t)(_lvl), TRACE_GET_STATIC());		\
				 _trc_.once && TRACE_INIT_CHECK( trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn)) ) \
				     && (_trc_.lvnafm_nafm_method, ((*_trc_.tidp != -1) || ((*_trc_.tidp= trace_tlog_name_(_trc_.nn,__TRACE_FILE__,__FILE__,_trc_.tn,sizeof(_trc_.tn))) != -1))) \
					 && trace_do_streamer(&_trc_); \
				 _trc_.once=0)                                                                                                                                                                                                                                                                                                                                                                                            \
				_PRAGMA("GCC diagnostic ignored \"-Wunused-value\"") \
			TraceStreamer().init(*_trc_.tidp, (uint8_t)(_trc_.lvl), _trc_.flgs, __FILE__, __LINE__, __PRETTY_FUNCTION__, &_trc_.tv, _trc_.ins, &TRACE_LOG_FUNCTION)
#	endif

#	define TRACE_ENDL ""
#	define TLOG_ENDL  TRACE_ENDL

// This will help devleper who use too many TLOG_INFO/TLOG_DEBUG to use more
// TLOG{,_TRACE,_DBG,_ARB} (use more levels). DEBUG_FORCED is also used in tracemf.h
#	ifdef __OPTIMIZE__
#		define DEBUG_FORCED 0
#	else
#		define DEBUG_FORCED 1
#	endif


#	define TRACE_STREAMER_ARGSMAX      35
#	define TRACE_STREAMER_TEMPLATE     1
#	define TRACE_STREAMER_EXPAND(args) args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22], args[23], args[24], args[25], args[26], args[27], args[28], args[29], args[30], args[31], args[32], args[33], args[34]

#	ifdef TRACE_STREAMER_DEBUG
#		define T_STREAM_DBG std::cout
#	else
#		define T_STREAM_DBG if (0) std::cout
#	endif

//typedef unsigned long long trace_ptr_t;
//typedef void* trace_ptr_t;
typedef void *trace_ptr_t;

namespace {  // unnamed namespace (i.e. static (for each compliation unit only))

struct TraceStreamer : std::ios {
	typedef unsigned long long arg;                                 // room for 64 bit args (i.e double on 32 or 64bit machines)
	arg args[TRACE_STREAMER_ARGSMAX] __attribute__((aligned(16)));  // needed for delayed long double formatting
	char msg[TRACE_STREAMER_MSGMAX];
	size_t msg_sz;
	size_t argCount;
	void *param_va_ptr;
	int tid_;
	uint8_t lvl_;
	bool do_s, do_m, do_f;  // do slow, do memory, do format (ie. no delayed formatting)
	char widthStr[16];
	char precisionStr[16];
	char fmtbuf[32];  // buffer for fmt (e.g. "%022.12llx")
	trace_tv_t *lclTime_p;
	const char *ins_;
	const char *file_;
	int line_;
	const char *function_;
	trace_log_function_type user_fun_ptr_;
#	if TRACE_USE_STATIC_STREAMER == 1
	bool inUse_;
#	endif

public:
	explicit TraceStreamer()
		: msg_sz(0), argCount(0), param_va_ptr(args)
	{
		T_STREAM_DBG << "TraceStreamer CONSTRUCTOR for this=" << this << " at line "<< __LINE__ << "\n";
		std::ios::init(0);
	}

	inline ~TraceStreamer()
	{
		T_STREAM_DBG << "TraceStreamer DESTRUCTOR for this=" << this << " at line "<< __LINE__ << "\n";
#	if TRACE_USE_STATIC_STREAMER != 1
		str();
#	endif
	}

	// use this method to return a reference (to the temporary, in its intended use)
	inline TraceStreamer &init(int tid, uint8_t lvl, tstreamer_flags flgs, const char *file, int line, const char *function,
							   timeval *tvp, const char *ins, trace_log_function_type user_fun_ptr)
	{
#	if TRACE_USE_STATIC_STREAMER == 1
		if (!inUse_) {
			inUse_= true;
#	endif
			widthStr[0]= precisionStr[0]= msg[0]= '\0';
			msg_sz= 0;
			argCount= 0;
			param_va_ptr= args;
			tid_= tid;
			lvl_= lvl;
			do_m= flgs.do_m;  // m=memory, aka "fast", but not to be confused with "format"
			do_s= flgs.do_s;
			do_f= (flgs.fmtnow == -1) ? 0 : (flgs.do_s || flgs.fmtnow);  // here "f" is "format", not "fast"
			ins_= ins;
			file_= file;
			line_= line;
			function_= function;
			lclTime_p= tvp;
			user_fun_ptr_= user_fun_ptr;
#	if TRACE_USE_STATIC_STREAMER == 1
			std::dec(*this);
			std::noshowbase(*this);
#	endif
			return *this;
#	if TRACE_USE_STATIC_STREAMER == 1
		} else {
			T_STREAM_DBG << "TraceStreamer.init(...):" << __LINE__ << " returning new TraceStreamer()\n";
			return (new TraceStreamer())->init(tid, lvl, flgs, file, line, function, tvp, ins, user_fun_ptr);
		}
#	endif
	} // init

#	ifdef __clang__
#		define _M_flags flags()
#	endif

	inline void str()
	{
		T_STREAM_DBG << "TraceStreamer.str(), msg=\"" << msg << "\"\n";
		while (msg_sz && msg[msg_sz - 1] == '\n') {
			msg[msg_sz - 1]= '\0';
			--msg_sz;
		}
#	if (__cplusplus >= 201103L)
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wformat-security"
#	endif
		if (do_f) {  // already formatted -- no "arguments" (for delayed formatting)
			if (do_m) trace(lclTime_p, tid_, lvl_, line_, function_, 0 TRACE_XTRA_PASSED, (const char *)msg);
			if (do_s) { (*user_fun_ptr_)(lclTime_p, tid_, lvl_, ins_, file_, line_, function_, 0, msg); } /* can be null */
		} else {
			if (do_m)
#	if (__cplusplus >= 201103L)
			{
#		if defined(__arm__) || defined(__aarch64__) /* address an alleged compiler bug (dealing with initializer) with the gnu arm compiler circa Feb, 2021 */
				va_list ap={}; // clear
				unsigned long *ulp = (unsigned long*)&ap;
				*ulp = (unsigned long)args;
#		else
				va_list ap= TRACE_VA_LIST_INIT((void *)args);  // warning: extended initializer lists only available with [since] -std=c++11 ...
#		endif
				vtrace(lclTime_p, tid_, lvl_, line_, function_, (uint8_t)argCount, msg, ap);
			}
#	else
				trace(lclTime_p, tid_, lvl_, line_, function_, (uint8_t)argCount TRACE_XTRA_PASSED, msg, TRACE_STREAMER_EXPAND(args));
#	endif
			if (do_s) {
				(*user_fun_ptr_)(lclTime_p, tid_, lvl_, ins_, file_, line_, function_, (uint8_t)argCount, msg, TRACE_STREAMER_EXPAND(args));
			} /* can be null */
		}
#	if (__cplusplus >= 201103L)
#		pragma GCC diagnostic pop
#	endif

		// Silence Clang static analyzer "dangling references"
		ins_= 0;
		lclTime_p= 0;
#	if TRACE_USE_STATIC_STREAMER == 1
		inUse_= false;
#	endif
	}  // str

	inline void msg_append(const char *src, size_t len= 0)
	{
		if (!len)
			len= strlen(src);
		size_t add= TRACE_MIN(len, sizeof(msg) - 1 - msg_sz);
		memcpy(&msg[msg_sz], src, add);
		msg_sz+= add;
		msg[msg_sz]= '\0';
	}

	// Return a format string (e.g "%d") - assume class fmtbuf char[] is big enough.
	inline char *format(bool isFloat, bool isUnsigned, const char *length, std::ios::fmtflags flags, size_t *len= NULL)
	{  //See, for example: http://www.cplusplus.com/reference/cstdio/printf/
		size_t oo= 0;
		fmtbuf[oo++]= '%';

		// Flags
		if (flags & left) fmtbuf[oo++]= '-';
		if (flags & showpos) fmtbuf[oo++]= '+';
		if (flags & (showpoint | showbase)) fmtbuf[oo++]= '#';  // INCLUSIVE OR

#	define TSTREAMER_APPEND(ss)         \
		do {                             \
			if (ss && ss[0]) {           \
				strcpy(&fmtbuf[oo], ss); \
				oo+= strlen(ss);         \
			}                            \
		} while (0)
		// Width
		TSTREAMER_APPEND(widthStr);

		if (isFloat) {
			// Precision
			TSTREAMER_APPEND(precisionStr);
			TSTREAMER_APPEND(length);

			if ((flags & (fixed | scientific)) == (fixed | scientific)) /*AND*/ {
				fmtbuf[oo++]= flags & uppercase ? 'A' : 'a';
			} else if (flags & fixed) {
				fmtbuf[oo++]= flags & uppercase ? 'F' : 'f';
			} else if (flags & scientific) {
				fmtbuf[oo++]= flags & uppercase ? 'E' : 'e';
			} else {
				fmtbuf[oo++]= flags & uppercase ? 'G' : 'g';
			}
		} else {
			TSTREAMER_APPEND(length);
			// this is more of the expected behavior - not necessarily what the standard describes
			if (flags & hex) {
				fmtbuf[oo++]= flags & uppercase ? 'X' : 'x';
			} else if (flags & oct) {
				fmtbuf[oo++]= 'o';
			} else if (isUnsigned) {
				fmtbuf[oo++]= 'u';
			} else {
				fmtbuf[oo++]= 'd';
			}
		}
		fmtbuf[oo]= '\0';
		if (len) *len= oo;
		return fmtbuf;
	}

	inline TraceStreamer &width(int y)
	{
		if (y != std::ios_base::width()) {
			std::ios_base::width(y);
		}
		snprintf(widthStr, sizeof(widthStr), "%d", y);
		T_STREAM_DBG << "TraceStreamer widthStr is now " << widthStr << std::endl;
		return *this;
	}

	inline TraceStreamer &precision(int y)
	{
		if (y != std::ios_base::precision()) {
			std::ios_base::precision(y);
		}
		if (y)
			snprintf(precisionStr, sizeof(precisionStr), ".%d", y);
		else
			precisionStr[0]= '\0';
		T_STREAM_DBG << "TraceStreamer precisionStr is now " << precisionStr << std::endl;
		return *this;
	}
#	if !defined(__clang__) || (defined(__clang__) && __clang_major__ == 3 && __clang_minor__ == 4) \
	|| (__clang_major__ >= 10 && __clang_major__ <= 11)
	inline TraceStreamer &operator<<(std::_Setprecision r)
	{
		precision(r._M_n);
		return *this;
	}
	inline TraceStreamer &operator<<(std::_Setw r)
	{
		width(r._M_n);
		return *this;
	}
#	else
#	ifndef _LIBCPP_ABI_NAMESPACE
#		define _LIBCPP_ABI_NAMESPACE __1
#	endif
	//setprecision
	inline TraceStreamer &operator<<(std::_LIBCPP_ABI_NAMESPACE::__iom_t5 r)
	{
		std::ostringstream ss;
		ss << r;
		precision(ss.precision());
		return *this;
	}
	//setwidth
	inline TraceStreamer &operator<<(std::_LIBCPP_ABI_NAMESPACE::__iom_t6 r)
	{
		std::ostringstream ss;
		ss << r;
		width(ss.width());
		return *this;
	}
#	endif

	// necessary for std::hex, std::dec
	typedef std::ios_base &(*manipulator)(std::ios_base &);
	inline TraceStreamer &operator<<(manipulator r)
	{
		r(*this);
		return *this;
	}

	// ------------------------------------------------------------------------

	template<typename T>
	inline void delay_format(const T *const &r)
	{
		T **const vp= (T * *const) param_va_ptr;
		if (do_f || (vp + 1) > (T * *const) & args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, "%p", static_cast<const void *>(r));
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer delay_format line " << __LINE__ << ", snprintf 1T rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			msg_append("%p", 2);
			++argCount;
			*vp= (T *const)r;
			param_va_ptr= vp + 1;
			T_STREAM_DBG << "streamer check 1T (const T*const &r) msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	template<typename T>
	inline TraceStreamer &operator<<(const T *const &r)
	{
		delay_format(r);
		return *this;
	}

	template<typename T>
	inline void delay_format(T *const &r)
	{
		T **const vp= (T * *const) param_va_ptr;
		if (do_f || (vp + 1) > (T * *const) & args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, "%p", static_cast<void *>(r));
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 2T rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			msg_append("%p", 2);
			++argCount;
			*vp= (T *const)r;
			param_va_ptr= vp + 1;
			T_STREAM_DBG << "streamer check 2T (T *const &r) msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	template<typename T>
	inline TraceStreamer &operator<<(T *const &r)  // Tricky C++...to pass pointer by reference, have to have the const AFTER the type
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const char &r)
	{
		long *vp= (long *)param_va_ptr;  // note: char gets pushed onto stack as sizeof(long)
		if (do_f || (vp + 1) > (long *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr=0;
			if (r != '\0') {							 // BEST TO JUST SKIP IF NULL
				rr= snprintf(&msg[msg_sz], ss, "%c", r); // print "null" if null???
				msg_sz+= TRACE_SNPRINTED(rr, ss);
			}
			T_STREAM_DBG << "streamer snprintf 3 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			msg_append("%c", 2);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 3 (const char &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const char &r)
	{	// std::iostream just outputs the character
		if (r != '\0') msg_append(&r,1);
		else           delay_format(r);
		return *this;
	}

	inline void delay_format(const unsigned char &r)
	{
		unsigned long *vp= (unsigned long *)param_va_ptr;  // Note: char gets pushed as sizeof(long)
		if (do_f || (vp + 1) > (unsigned long *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr;
			if (r != '\0') {							  // BEST TO JUST SKIP IF NULL
				rr = snprintf(&msg[msg_sz], ss, "%c", r); // print "null" if null???
				msg_sz+= TRACE_SNPRINTED(rr, ss);
			}
			T_STREAM_DBG << "streamer snprintf 4 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			msg_append("%c", 2);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 4 (const unsigned char &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const unsigned char &r)
	{	// std::iostream just outputs the character
		if (r != '\0') msg_append((const char*)&r,1);
		else           delay_format(r);
		return *this;
	}

	inline void delay_format(const int &r)
	{
		long *vp= (long *)param_va_ptr;  // int goes to long
		if (do_f || (vp + 1) > (long *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, false, NULL, _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer delay_format line " << __LINE__ << ", snprintf 5 rr=" << rr << " ss=" << ss << " msg=\"" << msg << "\"\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 5 (const int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const short int &r)
	{
		long *vp= (long *)param_va_ptr;  // Note: shorts get pushed onto stack as sizeof(long)
		if (do_f || (vp + 1) > (long *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, false, "h", _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 6 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, false, "h", _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 6 (const short int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const short int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const long int &r)
	{
		long int *vp= (long int *)param_va_ptr;
		if (do_f || (vp + 1) > (long int *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, false, "l", _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 7 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, false, "l", _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 7 (const long int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const long int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const short unsigned int &r)
	{
		unsigned long *vp= (unsigned long *)param_va_ptr;  // NOTE: shorts get pushed onto stack as sizeof(long)
		if (do_f || (vp + 1) > (unsigned long *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, true, "h", _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 8 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, true, "h", _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 8 (const short unsigned int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const short unsigned int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const unsigned int &r)
	{
		unsigned long *vp= (unsigned long *)param_va_ptr;  // int goes to long
		if (do_f || (vp + 1) > (unsigned long *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, true, NULL, _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 9 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, true, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 9 (const unsigned int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const unsigned int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const long unsigned int &r)
	{
		long unsigned int *vp= (long unsigned int *)param_va_ptr;
		if (do_f || (vp + 1) > (long unsigned int *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, true, "l", _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 10 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, true, "l", _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 10 (const long unsiged int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const long unsigned int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const long long unsigned int &r)
	{
		unsigned long nvp= (unsigned long)param_va_ptr;
		long long unsigned int *vp= (long long unsigned int *)nvp;
#	if defined(__arm__)
		if (nvp & 7)
			vp= (long long unsigned int*)((nvp + 7) & ~7); // alignment requirement
#	endif
		if (do_f || (vp + 1) > (long long unsigned int *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, true, "ll", _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 11 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, true, "ll", _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 11 (const long long unsiged int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const long long unsigned int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const long long int &r)
	{
		unsigned long nvp= (unsigned long)param_va_ptr;
		long long int *vp= (long long int *)nvp;
#	if defined(__arm__)
		if (nvp & 7)
			vp= (long long int*)((nvp + 7) & ~7); // alignment requirement
#	endif
		if (do_f || (vp + 1) > (long long int *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(false, true, "ll", _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 11 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(false, true, "ll", _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 11 (const long long unsiged int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const long long int &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const double &r)
	{
		unsigned long nvp= (unsigned long)param_va_ptr;
		double *vp= (double *)nvp;
#	if defined(__arm__)
		if (nvp & 7)
			vp= (double*)((nvp + 7) & ~7); // alignment requirement
#	endif
		if (do_f || (vp + 1) > (double *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(true, false, NULL, _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 12 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(true, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 12 (const double &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const double &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const long double &r)
	{
		unsigned long nvp= (unsigned long)param_va_ptr;
		long double *vp;
		if (sizeof(long double) == 16 && (nvp & 0xf))
			vp= (long double *)((nvp + 15) & ~(unsigned long)0xf);  // alignment requirement
		else
			vp= (long double *)nvp;
		if (do_f || (vp + 1) > (long double *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(true, false, "L", _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 13 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(true, false, "L", _M_flags, &f_l), f_l);
			argCount+= (sizeof(long double) + sizeof(long) / 2) / sizeof(long);
			if (sizeof(long double) == 16 && (nvp & 0xf))
				++argCount;  // speudo extra arg satisfies alignment requirement
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 13 (const long double &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const long double &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const float &r)
	{
		unsigned long nvp= (unsigned long)param_va_ptr;
		double *vp= (double *)nvp;  // note: floats get pushed onto stack as double
#	if defined(__arm__)
		if (nvp & 7)
			vp= (double*)((nvp + 7) & ~7); // alignment requirement
#	endif
		if (do_f || (vp + 1) > (double *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, format(true, false, NULL, _M_flags), r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 14 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			size_t f_l= 0;
			msg_append(format(true, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 14 (const float &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const float &r)
	{
		delay_format(r);
		return *this;
	}

	inline void delay_format(const bool &r)
	{
		long *vp= (long *)param_va_ptr;  // note: bool is pushed as long
		if (_M_flags & boolalpha)
			msg_append(r ? "true" : "false", r ? 4 : 5);
		else if (do_f || (vp + 1) > (long *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, "%d", r);
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 15 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			msg_append("%d", 2);
			++argCount;
			*vp++= r;
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 15 (const bool &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
	}
	inline TraceStreamer &operator<<(const bool &r)
	{
		delay_format(r);
		return *this;
	}

	inline TraceStreamer &operator<<(const std::string &r)
	{
		msg_append(r.c_str(), r.size());
		return *this;
	}
	inline TraceStreamer &operator<<(char *r)
	{
		msg_append(r);
		return *this;
	}

#	if (__cplusplus >= 201103L)
	inline TraceStreamer &operator<<(const std::atomic<int> &r)
	{
		delay_format(r.load());
		return *this;
	}

	inline TraceStreamer &operator<<(std::atomic<unsigned long> const &r)
	{
		delay_format(r.load());
		return *this;
	}

	inline TraceStreamer &operator<<(std::atomic<short int> const &r)
	{
		delay_format(r.load());
		return *this;
	}

	inline TraceStreamer &operator<<(std::atomic<bool> const &r)
	{
		delay_format(r.load());
		return *this;
	}

	template<typename T>
	inline void delay_format(std::unique_ptr<T> const &r)
	{
		trace_ptr_t *vp= (trace_ptr_t *)param_va_ptr;  // address is unsigned long
		if (do_f || (vp + 1) > (trace_ptr_t *)&args[traceControl_p->num_params]) {
			size_t ss= sizeof(msg) - 1 - msg_sz;
			int rr= snprintf(&msg[msg_sz], ss, "%p", static_cast<void *>(r.get()));
			msg_sz+= TRACE_SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 20 rr=" << rr << " ss=" << ss << "\n";
		} else if (argCount < TRACE_STREAMER_ARGSMAX) {
			msg_append("%p", 2);
			++argCount;
			*vp++= (trace_ptr_t)(void *)(r.get());
			param_va_ptr= vp;
			T_STREAM_DBG << "streamer check 20 (std::unique_ptr<T> const &r) " << r.get()
						 << " sizeof(r.get())=" << sizeof(r.get())
						 << " (unsigned long)r.get()=0x" << std::hex << (trace_ptr_t)(void *)(r.get())
						 << " sizeof(trace_ptr_t)=" << sizeof(trace_ptr_t)
						 << " msg_sz=" << std::dec << msg_sz << "\n";  // ALERT: without ".get()" - error: no match for 'operator<<' (operand types are 'std::basic_ostream<char>' and 'const std::unique_ptr<std::__cxx11::basic_string<char> >')
		}
	}
	template<typename T>
	inline TraceStreamer &operator<<(std::unique_ptr<T> const &r)
	{
		delay_format(r);
		return *this;
	}
#	endif /* (__cplusplus >= 201103L) */

	// compiler asked for this -- can't think of why or when it will be used, but do the reasonable thing (append format and append args)
	inline TraceStreamer &operator<<(const TraceStreamer &r)
	{
		for (size_t ii= argCount; ii < (argCount + (r.argCount < TRACE_STREAMER_ARGSMAX
														? argCount + r.argCount
														: TRACE_STREAMER_ARGSMAX));
			 ++ii) {
			args[ii]= r.args[ii - argCount];
		}
		argCount= argCount + r.argCount < TRACE_STREAMER_ARGSMAX ? argCount + r.argCount : TRACE_STREAMER_ARGSMAX;
		msg_append(r.msg);
		return *this;
	}

	////https://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator
	/// https://stackoverflow.com/questions/2212776/overload-handling-of-stdendl
	typedef std::ostream &(*ostream_manipulator)(std::ostream &);

	inline TraceStreamer &operator<<(ostream_manipulator r)
	{
		if (r == (std::basic_ostream<char> & (*)(std::basic_ostream<char> &)) & std::endl)
			msg_append("\n");
		return *this;
	}

	inline TraceStreamer &operator<<(char const *r)
	{
		msg_append(r);
		return *this;
	}

	inline TraceStreamer &operator<<(const tlvle_t& r)
	{
		delay_format(r);
		return *this;
	}

#	if TRACE_STREAMER_TEMPLATE
	// This is heavy weight (instantiation of stringstream); hopefully will not be done too often or not at all
	template<typename T>
	inline TraceStreamer &operator<<(const T &r)
	{
#		if DEBUG_FORCED
		std::cerr << "WARNING: " << __PRETTY_FUNCTION__ << " TEMPLATE CALLED: Consider implementing a function with this signature!" << std::endl;
#		endif
		std::ostringstream s;
		s << r;
		msg_append(s.str().c_str());
		return *this;
	}
#	endif
};  // struct Streamer

template<>
inline void TraceStreamer::delay_format(void *const &r)
{
	trace_ptr_t *vp= (trace_ptr_t *)param_va_ptr;  // note: addresses are unsigned long
	if (do_f || (vp + 1) > (trace_ptr_t *)&args[traceControl_p->num_params]) {
		size_t ss= sizeof(msg) - 1 - msg_sz;
		int rr= snprintf(&msg[msg_sz], ss, "%p", r);
		msg_sz+= TRACE_SNPRINTED(rr, ss);
		T_STREAM_DBG << "streamer snprintf 21 rr=" << rr << " ss=" << ss << "\n";
	} else if (argCount < TRACE_STREAMER_ARGSMAX) {
		msg_append("%p", 2);
		++argCount;
		*vp++= (trace_ptr_t)r;
		param_va_ptr= vp;
		T_STREAM_DBG << "streamer check 21 (void *const &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";  // ALERT: without ".get()" - error: no match for 'operator<<' (operand types are 'std::basic_ostream<char>' and 'const std::unique_ptr<std::__cxx11::basic_string<char> >')
	}
}
template<>
inline TraceStreamer &TraceStreamer::operator<<(void *const &r)  // Tricky C++...to pass pointer by reference, have to have the const AFTER the type
{
	delay_format(r);
	return *this;
}

#	if TRACE_USE_STATIC_STREAMER == 1
static TRACE_THREAD_LOCAL TraceStreamer __tstreamer;
#	endif


struct TSTREAMER_T_ {
	unsigned once;
	tlvle_t lvl;
	int* tidp;			 // initialized to address of tid in static tinfo_t
	limit_info_t* lim_infop;  // initialized to address of info in static tinfo_t
	tstreamer_flags flgs;
	const char* nn;           // the name passed in the TLOG*(...) arg list
	char tn[TRACE_TN_BUFSZ];  // for converting the __FILE__ to a trace name - just used in TRACE_INIT_CHECK(trace_name(...)) call.
	char ins[32];
	trace_tv_t tv;
	void* stmr__;
	inline TSTREAMER_T_(tlvle_t llv, tinfo_t* infop)
		: once(1), lvl(llv), tidp(&infop->tid), lim_infop(&infop->info)
#	if TRACE_USE_STATIC_STREAMER == 1
		, stmr__(&__tstreamer)
#	endif
	{
		T_STREAM_DBG << "TSTREAMER_T_ CONSTRUCTOR for this=" << this << " at line "<< __LINE__ << "\n";
		tv.tv_sec= 0;
	}
	inline ~TSTREAMER_T_() // implementation below (or not)
	{
		T_STREAM_DBG << "TSTREAMER_T_ DESTRUCTOR for this=" << this << " at line "<< __LINE__ << "\n";
#	if TRACE_USE_STATIC_STREAMER == 1
		if (stmr__ != (void*)&__tstreamer) delete (TraceStreamer*)stmr__;
#	endif
	}

	// FOR TLOG(...)
	inline void TLOG3(int _lvl= TLVL_LOG, bool fmt= false, const char* nam= "") //  1
	{
		//if (_lvl < 0) _lvl= 0;
		lvl= (tlvle_t)_lvl;
		if (!fmt) flgs.fmtnow= 0;
		else      flgs.fmtnow= 1;
		nn= nam;
		size_t sz = strlen(nam);
		if (sz<sizeof(tn)) strcpy(tn,nam);
		else { strncpy(tn,nam,sizeof(tn)-1);tn[sizeof(tn)-1]='\0';}
	}
	inline void TLOG3(int _lvl, bool fmt,           const std::string& nam)           { TLOG3(_lvl, fmt, &nam[0]); }       //  2
	inline void TLOG3(bool fmt, int _lvl=TLVL_LOG,  const char*        nam="")        { TLOG3(_lvl, fmt, &nam[0]); }       //  3
	inline void TLOG3(bool fmt, int _lvl,           const std::string& nam)           { TLOG3(_lvl, fmt, &nam[0]); }       //  4
	inline void TLOG3(int _lvl, const char*        nam,           bool fmt=false)     { TLOG3(_lvl, fmt, &nam[0]); }       //  5
	inline void TLOG3(int _lvl, const std::string& nam,           bool fmt=false)     { TLOG3(_lvl, fmt, &nam[0]); }       //  6
	inline void TLOG3(bool fmt, const char*        nam,           int _lvl=TLVL_LOG)  { TLOG3(_lvl, fmt, &nam[0]); }       //  7
	inline void TLOG3(bool fmt, const std::string& nam,           int _lvl=TLVL_LOG)  { TLOG3(_lvl, fmt, &nam[0]); }       //  8
	inline void TLOG3(const char*        nam, int _lvl= TLVL_LOG, bool fmt=false)     { TLOG3(_lvl, fmt, &nam[0]); }       //  9
	inline void TLOG3(const std::string& nam, int _lvl= TLVL_LOG, bool fmt=false)     { TLOG3(_lvl, fmt, &nam[0]); }       // 10
	inline void TLOG3(const char*        nam, bool fmt,           int _lvl= TLVL_LOG) { TLOG3(_lvl, fmt, &nam[0]); }       // 11
	inline void TLOG3(const std::string& nam, bool fmt,           int _lvl= TLVL_LOG) { TLOG3(_lvl, fmt, &nam[0]); }       // 12
	inline void TLOG3(int _lvl, const char*        nam,           int fmt)            { TLOG3(_lvl, (bool)fmt, &nam[0]); } // 13
	inline void TLOG3(int _lvl, const std::string& nam,           int fmt)            { TLOG3(_lvl, (bool)fmt, &nam[0]); } // 14
	inline void TLOG3(const char*        nam, int _lvl,           int fmt)            { TLOG3(_lvl, (bool)fmt, &nam[0]); } // 15
	inline void TLOG3(const std::string& nam, int _lvl,           int fmt)            { TLOG3(_lvl, (bool)fmt, &nam[0]); } // 16
	inline void TLOG3(int _lvl,               int  fmt,   const char* nam="")         { TLOG3(_lvl, (bool)fmt, &nam[0]); } // 17

	// FOR TLOG_DEBUG(...)
	inline void TLOG_DEBUG3(int _lvl= 0, bool fmt= false, const char* nam= "")
	{
		if (_lvl < 0) _lvl= 0;
		else if (_lvl > (63 - TLVL_DEBUG))
			_lvl= (63 - TLVL_DEBUG);
		lvl= (tlvle_t)(TLVL_DEBUG + _lvl);

		if (!fmt) flgs.fmtnow= 0;
		else      flgs.fmtnow= 1;
		nn= nam;
		size_t sz = strlen(nam);
		if (sz<sizeof(tn)) strcpy(tn,nam);
		else { strncpy(tn,nam,sizeof(tn)-1);tn[sizeof(tn)-1]='\0';}
	}
	inline void TLOG_DEBUG3(int _lvl, bool fmt,           const std::string& nam)       { TLOG_DEBUG3(_lvl, fmt,       &nam[0]); }
	inline void TLOG_DEBUG3(int _lvl, int  fmt,           const char*        nam="")    { TLOG_DEBUG3(_lvl, (bool)fmt, &nam[0]); }
	inline void TLOG_DEBUG3(bool fmt, int _lvl=0,         const char*        nam="")    { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(int _lvl, const char*        nam,           bool fmt=false) { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(int _lvl, const std::string& nam,           bool fmt=false) { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(int _lvl, const char*        nam,           int  fmt)       { TLOG_DEBUG3(_lvl, (bool)fmt, &nam[0]); }
	inline void TLOG_DEBUG3(int _lvl, const std::string& nam,           int  fmt)       { TLOG_DEBUG3(_lvl, (bool)fmt, &nam[0]); }
	inline void TLOG_DEBUG3(bool fmt, const char*        nam,           int _lvl=0)     { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(bool fmt, const std::string& nam,           int _lvl=0)     { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(const char*        nam, int _lvl= 0, bool fmt=false)        { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(const std::string& nam, int _lvl= 0, bool fmt=false)        { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(const char*        nam, bool fmt,           int _lvl= 0)    { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(const std::string& nam, bool fmt,           int _lvl= 0)    { TLOG_DEBUG3(_lvl, fmt, &nam[0]); }
	inline void TLOG_DEBUG3(const char*        nam, int _lvl,           int fmt)        { TLOG_DEBUG3(_lvl, (bool)fmt, &nam[0]); }
	inline void TLOG_DEBUG3(const std::string& nam, int _lvl,           int fmt)        { TLOG_DEBUG3(_lvl, (bool)fmt, &nam[0]); }

	// FOR TLOG_ERROR, TLOG_WARNING, TLOG_INFO and possibly TLOG when only TLVL_LOG
	inline void TLOG2(int fmt= 0, const char* nam= "")
	{
		if      (fmt==0) flgs.fmtnow=  0;
		else if (fmt >0) flgs.fmtnow=  1;
		else             flgs.fmtnow= -1;
		nn= nam;
		size_t sz = strlen(nam);
		if (sz<sizeof(tn)) strcpy(tn,nam);
		else { strncpy(tn,nam,sizeof(tn)-1);tn[sizeof(tn)-1]='\0';}
	}
	inline void TLOG2(int fmt, const std::string& nam) { TLOG2(fmt, &nam[0]); }
	inline void TLOG2(const char* nam, int fmt= 0) { TLOG2(fmt, &nam[0]); }
	inline void TLOG2(const std::string& nam, int fmt= 0) { TLOG2(fmt, &nam[0]); }
};

}  // unnamed namespace

// SLow FoRCe
#	ifndef TSTREAMER_SL_FRC
#		define TSTREAMER_SL_FRC(lvl)     0
#	endif

static inline bool trace_do_streamer(TSTREAMER_T_ *ts_p)
{
	ts_p->flgs.do_m=     (traceLvls_p[*ts_p->tidp].M & TLVLMSK(ts_p->lvl)) && traceControl_rwp->mode.bits.M;
	ts_p->flgs.do_s= ( (((traceLvls_p[*ts_p->tidp].S & TLVLMSK(ts_p->lvl)) && traceControl_rwp->mode.bits.S) || TSTREAMER_SL_FRC(ts_p->lvl))
	                  && trace_limit_do_print(&ts_p->tv, ts_p->lim_infop, ts_p->ins, sizeof(ts_p->ins)) );
	return (ts_p->flgs.do_m || ts_p->flgs.do_s);
}


#	if __cplusplus >= 201703L

#		define TRACE_EXIT \
	auto TRACE_VARIABLE(TRACE_EXIT_STATE) \
	= ::detail::TraceGuardOnExit() + [&]() noexcept

#define TRACENATE_IMPL(s1, s2)   s1##s2
#define TRACENATE(s1, s2)        TRACENATE_IMPL(s1, s2)

#ifdef __COUNTER__XX  // cannot use counter as it can only be used once in macro where as __LINE__ is can be used multiple
                      // times; it will be the starting line of multiline macro.
#       define TRACE_VARIABLE(pre) TRACENATE(pre,__COUNTER__)
#else
#       define TRACE_VARIABLE(pre) TRACENATE(pre,__LINE__)
#endif

namespace detail {

	template <class Fun>
	class TraceGuard {
		Fun f_;
		bool active_;
	public:
		TraceGuard(Fun f) : f_(std::move(f)), active_(true) {}
		~TraceGuard() { if (active_) f_(); }
		void dismiss() { active_ = false; }
		TraceGuard() = delete;
		TraceGuard( const TraceGuard&) = delete;
		TraceGuard& operator=(const TraceGuard&) = delete;
		TraceGuard(TraceGuard&& rhs) : f_(std::move(rhs.f_)), active_(rhs.active_) { rhs.dismiss(); }
	};

	enum class TraceGuardOnExit {};

	template <class Fun>
	TraceGuard<Fun> scopeGuard(Fun f) {
		return TraceGuard<Fun>(std::move(f));
	}
        
	template<typename Fun>
	TraceGuard<Fun> operator+(TraceGuardOnExit, Fun&& fn) {    // to allow the saving of the lambda
		return TraceGuard<Fun>(std::forward<Fun>(fn));
	}  

} // namespace detail

#	endif

#endif /* __cplusplus */

#endif /* TRACE_H */
