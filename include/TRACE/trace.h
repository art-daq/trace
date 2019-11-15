/* This file (trace.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: trace.h,v $
 */
#ifndef TRACE_H
#define TRACE_H

#define TRACE_REV "$Revision: 1240 $$Date: 2019-11-14 08:37:14 -0600 (Thu, 14 Nov 2019) $"

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
	return syscall(TRACE_GETTID);
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
#		define TRACE_ATOMIC_T std::atomic<uint32_t>
#		define TRACE_ATOMIC_INIT ATOMIC_VAR_INIT(0)
#		define TRACE_ATOMIC_LOAD(ptr) atomic_load(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#		define TRACE_THREAD_LOCAL
#	elif defined(__cplusplus) && (__cplusplus >= 201103L)
#		include <atomic> /* atomic<> */
#		include <memory> /* std::unique_ptr */
#		define TRACE_ATOMIC_T std::atomic<uint32_t>
#		define TRACE_ATOMIC_INIT ATOMIC_VAR_INIT(0)
#		define TRACE_ATOMIC_LOAD(ptr) atomic_load(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#		define TRACE_THREAD_LOCAL thread_local
#	elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && (defined(__clang__) || (defined __GNUC__ && defined __GNUC_MINOR__ && (10000 * __GNUC__ + 1000 * __GNUC_MINOR__) >= 49000))
#		define TRACE_C11_ATOMICS
#		include <stdatomic.h> /* atomic_compare_exchange_weak */
#		define TRACE_ATOMIC_T /*volatile*/ _Atomic(uint32_t)
#		define TRACE_ATOMIC_INIT ATOMIC_VAR_INIT(0)
#		define TRACE_ATOMIC_LOAD(ptr) atomic_load(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#		define TRACE_THREAD_LOCAL _Thread_local
#	elif defined(__x86_64__) || defined(__i686__) || defined(__i386__)
#		define TRACE_ATOMIC_T uint32_t
#		define TRACE_ATOMIC_INIT 0
#		define TRACE_ATOMIC_LOAD(ptr) *(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) *(ptr) = val
#		define TRACE_THREAD_LOCAL
static inline uint32_t cmpxchg(uint32_t *ptr, uint32_t old, uint32_t new_)
{
	uint32_t __ret;
	uint32_t __old = (old);
	uint32_t __new = (new_);
	volatile uint32_t *__ptr = (volatile uint32_t *)(ptr);
	__asm__ volatile("lock cmpxchgl %2,%1"
					 : "=a"(__ret), "+m"(*__ptr)
					 : "r"(__new), "0"(__old)
					 : "memory");
	return (__ret);
}
#	elif defined(__sparc__)
/* Sparc, as per wikipedia 2016.01.11, does not do CAS.
   I could move move the DECL stuff up can define another spinlock so that sparc could work in
   the define/declare environment. In this case, the module static TRACE_NAME feature could not
   be used. */
struct my_atomic
{
	uint32_t lck;
	uint32_t val;
};
#		define TRACE_ATOMIC_T struct my_atomic
#		define TRACE_ATOMIC_INIT \
			{                     \
				0                 \
			}
#		define TRACE_ATOMIC_LOAD(ptr) (ptr)->val
#		define TRACE_ATOMIC_STORE(ptr, vv) (ptr)->val = vv
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
	old = ptr->val;
	if (old == exp) ptr->val = new_;
	ptr->lck = 0; /* unlock */
	return (old);
}
#	else /* userspace arch */
/* THIS IS A PROBLEM (older compiler on unknown arch) -- I SHOULD PROBABLY #error */
#		define TRACE_ATOMIC_T uint32_t
#		define TRACE_ATOMIC_INIT 0
#		define TRACE_ATOMIC_LOAD(ptr) *(ptr)
#		define TRACE_ATOMIC_STORE(ptr, val) *(ptr) = val
#		define TRACE_THREAD_LOCAL
#		define cmpxchg(ptr, old, new) \
			({ uint32_t old__ = *(ptr); if(old__==(old)) *ptr=new; old__; }) /* THIS IS A PROBLEM -- NEED OS MUTEX HELP :( */
#	endif        /* userspace arch */

#	define TRACE_GETTIMEOFDAY(tvp) gettimeofday(tvp, NULL)
#	define TRACE_PRN printf
#	define TRACE_VPRN vprintf
#	define TRACE_INIT_CHECK(nn) if ((traceTID != -1) || (traceInit(nn, 0) == 0)) /* See note by traceTID decl/def below */

#else /* __KERNEL__ */

#	include <linux/ktime.h>                                      /* do_gettimeofday */
/*# include <linux/printk.h>	         printk, vprintk */
#	include <linux/kernel.h>                                    /* printk, vprintk */
#	include <linux/mm.h>                                        /* kmalloc OR __get_free_pages */
#	include <linux/vmalloc.h>                                   /* __vmalloc, vfree */
#	include <linux/spinlock.h>                                  /* cmpxchg */
#	include <linux/sched.h>                                     /* current (struct task_struct *) */
#	include <linux/ctype.h>                                     /* isgraph */
#	include <linux/version.h>                                   /* KERNEL_VERSION */
/*# define TMATCHCMP(pattern,str_)         (strcmp(pattern,str_)==0)*/ /*MAKE MACRO RETURN TRUE IF MATCH*/
#	define TMATCHCMP(needle, haystack) strstr(haystack, needle) /*MAKE MACRO RETURN TRUE IF MATCH*/
#	define TRACE_ATOMIC_T uint32_t
#	define TRACE_ATOMIC_INIT 0
#	define TRACE_ATOMIC_LOAD(ptr) *(ptr)
#	define TRACE_ATOMIC_STORE(ptr, val) *(ptr) = val
#	define TRACE_THREAD_LOCAL
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,1)
#     define TRACE_GETTIMEOFDAY(tvp) ({struct timespec64 ts; ktime_get_real_ts64(&ts); (tvp)->tv_sec=ts.tv_sec; (tvp)->tv_usec=ts.tv_nsec/1000;})
#   else
#	  define TRACE_GETTIMEOFDAY(tvp) do_gettimeofday(tvp)
#   endif
#	define TRACE_PRN printk
#	define TRACE_VPRN vprintk
/*static int trace_no_init_cnt=0;*/
#	define TRACE_INIT_CHECK(nn) if ((traceTID != -1) || ((traceTID = name2TID(nn)) != -1))
#	ifndef MODULE
int trace_3_init(void);
int trace_sched_switch_hook_add(void); /* for when compiled into kernel */
#	endif
static inline int trace_getcpu(void)
{
	return raw_smp_processor_id();
}
#endif /* __KERNEL__ */

/* the width used when printing out Process/task or Thread IDs, etc. */
#ifdef __APPLE__
#	define TRACE_TID_WIDTH 7
#else
#	define TRACE_TID_WIDTH 6
#endif
#define TRACE_CPU_WIDTH 3
#define TRACE_LINENUM_WIDTH 4

/* these were originally just used in the mmap function */
#define MM_STR(xx) MM_STR2(xx)
#define MM_STR2(xx) #xx

/* Maximum UDP Datagram Data Length */
#ifndef TRACE_STREAMER_MSGMAX            /* allow test program to try different values */
#	define TRACE_STREAMER_MSGMAX 0x2000 /* 0x3400 seems to work for artdaq, but 0x3800 does not. 65507 is way too much for when TraceStreamer is static thread_local */
#endif
#ifndef TRACE_USER_MSGMAX            /* allow test program to try different values */
#	define TRACE_USER_MSGMAX 0x1800 /* Note: currently user msg part will be 10's of bytes less than this depending upon TRACE_PRINT format */
#endif
/* 88,7=192 bytes/ent   96,6=192   128,10=256  192,10=320 */
#define TRACE_DFLT_MAX_MSG_SZ 192
#define TRACE_DFLT_MAX_PARAMS 10
#define TRACE_DFLT_NAMTBL_ENTS 1022 /* this is for creating new trace_buffer file --         \
									   it currently matches the "trace DISABLED" number that \
									   fits into traceControl[1] (see below) */
#define TRACE_DFLT_NAM_CHR_MAX 39   /* Really The hardcoded max name len.                                      \
									   Name buffers should be +1 (for null terminator) - 40 was the value with \
									   8K pages which gave 127 NAMTBL_ENTS with "trace DISBALED".              \
									   Search for trace_created_init(...) call in "DISABLE" case.              \
									   Names can have this many characters (and always be null terminated -    \
									   so names can be printed from nam tbl) */
#define TRACE_DFLT_NUM_ENTRIES 100000
#define TRACE_DFLT_TIME_FMT "%m-%d %H:%M:%S.%%06d" /* match default in trace_delta.pl */
#ifdef __KERNEL__
#	define TRACE_DFLT_NAME "KERNEL"
#else
#	define TRACE_DFLT_NAME "TRACE"
#endif

#if !defined(TRACE_NAME) && !defined(__KERNEL__)
static const char *TRACE_NAME = NULL;
#elif !defined(TRACE_NAME) && defined(__KERNEL__)
#	define TRACE_NAME TRACE_DFLT_NAME /* kernel doesn't have env.var, so different init path*/
#endif

#if !defined(TRACE_PRINT)
static const char *TRACE_PRINT = "%T %n %L %M"; /* Msg Limit Insert will have separator */
#endif

#ifndef TRACE_PRINT_FD
#	define TRACE_PRINT_FD 1,1
#endif

/* 64bit sparc (nova.fnal.gov) has 8K pages (ref. ~/src/sysconf.c). This
											 (double) is no big deal on systems with 4K pages; more important (effects
											 userspace mapping) when actual 8K pages.
											 Cygwin uses 64K pages. */
#define TRACE_PAGESIZE 0x10000
#define TRACE_CACHELINE 64

#ifdef __GNUC__
#	define SUPPRESS_NOT_USED_WARN __attribute__((__unused__))
#else
#	define SUPPRESS_NOT_USED_WARN
#endif

#define LVLBITSMSK ((sizeof(uint64_t) * 8) - 1)
#define TLVLMSK(xx) (1LL << ((xx)&LVLBITSMSK))

/* For C, these should go at the beginning of a block b/c they define new local variables */
#if defined(TRACE_NO_LIMIT_SLOW) /* || defined(__KERNEL__) */
#	define TRACE_LIMIT_SLOW(lvl, ins, tvp) \
		char ins[1] = {'\0'};               \
		if (1)
#else
#	define TRACE_LIMIT_SLOW(lvl, ins, tvp)                                                   \
		char ins[32];                                                                         \
		static TRACE_THREAD_LOCAL limit_info_t _info = {/*TRACE_ATOMIC_INIT,*/ 0, lsFREE, 0}; \
		if (limit_do_print(tvp, &_info, ins, sizeof(ins)))
#endif

/* helper for TRACEing strings in C - ONLY in C (and not in KERNEL, which could use C90 compiler) */
#if defined(__KERNEL__)
#	define TSBUFDECL /* some kernel compiles complain: ISO C90 forbids variable length array 'tsbuf__' */
#elif defined(__cplusplus)
/* don't want to instantiate an std::vector as it may cause alloc/delete */
#	define TSBUFDECL /*std::vector<char> tsbuf__(traceControl_p->siz_msg);*/ /*&(tsbuf__[0])*/
/*# define TSBUFSIZ__     tsbuf__.size()*/
#else
#	define TSBUFDECL                                                     \
		char tsbuf__[traceControl_p->siz_msg + 1] SUPPRESS_NOT_USED_WARN; \
		tsbuf__[0] = '\0'
#	define TSBUFSIZ__ sizeof(tsbuf__)
/* The following is what is to be optionally used: i.e.: TRACE(2,TSPRINTF("string=%s store_int=%%d",string),store_int);
   Watch for the case where string has an imbedded "%" */
#	define TSPRINTF(...) (tsbuf__[0] ? &(tsbuf__[0]) : (snprintf(&(tsbuf__[0]), TSBUFSIZ__, __VA_ARGS__), &(tsbuf__[0])))
#endif

/* Note: anonymous variadic macros were introduced in C99/C++11 (maybe C++0x) */

#define TRACE(lvl, ...)                                                                                                               \
	do                                                                                                                                \
	{                                                                                                                                 \
		TRACE_INIT_CHECK(TRACE_NAME)                                                                                                  \
		{                                                                                                                             \
			struct timeval lclTime;                                                                                                   \
			uint16_t lvl_ = lvl;                                                                                                      \
			TSBUFDECL;                                                                                                                \
			lclTime.tv_sec = 0;                                                                                                       \
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[traceTID].M & TLVLMSK(lvl_)))                                        \
			{                                                                                                                         \
				trace(&lclTime, traceTID, lvl_, __LINE__, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED, __VA_ARGS__);                   \
			}                                                                                                                         \
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[traceTID].S & TLVLMSK(lvl_)))                                        \
			{                                                                                                                         \
				TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime)                                                                             \
				{                                                                                                                     \
					TRACE_LOG_FUNCTION(&lclTime, traceTID, lvl_, _insert, __FILE__, __LINE__, TRACE_NARGS(__VA_ARGS__), __VA_ARGS__); \
				}                                                                                                                     \
			}                                                                                                                         \
		}                                                                                                                             \
	} while (0)
/* static int tid_ could be TRACE_THREAD_LOCAL */
#define TRACEN(nam, lvl, ...)                                                                                                     \
	do                                                                                                                            \
	{                                                                                                                             \
		TRACE_INIT_CHECK(TRACE_NAME)                                                                                              \
		{                                                                                                                         \
			static TRACE_THREAD_LOCAL int tid_ = -1;                                                                              \
			struct timeval lclTime;                                                                                               \
			uint16_t lvl_ = lvl;                                                                                                  \
			TSBUFDECL;                                                                                                            \
			if (tid_ == -1) tid_ = name2TID(&(nam)[0]);                                                                           \
			lclTime.tv_sec = 0;                                                                                                   \
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl_)))                                        \
			{                                                                                                                     \
				trace(&lclTime, tid_, lvl_, __LINE__, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED, __VA_ARGS__);                   \
			}                                                                                                                     \
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[tid_].S & TLVLMSK(lvl_)))                                        \
			{                                                                                                                     \
				TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime)                                                                         \
				{                                                                                                                 \
					TRACE_LOG_FUNCTION(&lclTime, tid_, lvl_, _insert, __FILE__, __LINE__, TRACE_NARGS(__VA_ARGS__), __VA_ARGS__); \
				}                                                                                                                 \
			}                                                                                                                     \
		}                                                                                                                         \
	} while (0)

#if defined(__cplusplus)

/* Note: This supports using a mix of stream syntax and format args, i.e: "string is " << some_str << " and float is %f", some_float
   Note also how the macro evaluates the first part (the "FMT") only once
   no matter which destination ("M" and/or "S") is active.
   Note: "xx" in TRACE_ARGS_FMT(__VA_ARGS__,xx) is just a dummy arg to that macro.
   THIS IS DEPRECATED. It is nice to have for comparison tests.
*/
#	define TRACEN_(nam, lvl, ...)                                                                                                                                              \
		do                                                                                                                                                                      \
		{                                                                                                                                                                       \
			TRACE_INIT_CHECK(TRACE_NAME)                                                                                                                                        \
			{                                                                                                                                                                   \
				static TRACE_THREAD_LOCAL int tid_ = -1;                                                                                                                        \
				struct timeval lclTime;                                                                                                                                         \
				uint16_t lvl_ = lvl;                                                                                                                                            \
				if (tid_ == -1) tid_ = name2TID(&(nam)[0]);                                                                                                                     \
				lclTime.tv_sec = 0;                                                                                                                                             \
				bool do_m = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl_));                                                                          \
				bool do_s = traceControl_rwp->mode.bits.S && (traceNamLvls_p[tid_].S & TLVLMSK(lvl_));                                                                          \
				if (do_s || do_m)                                                                                                                                               \
				{                                                                                                                                                               \
					std::ostringstream ostr__; /*instance creation is heavy weight*/                                                                                            \
					ostr__ << TRACE_ARGS_FMT(__VA_ARGS__, xx);                                                                                                                  \
					if (do_m) trace(&lclTime, tid_, lvl_, __LINE__, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED, ostr__.str() TRACE_ARGS_ARGS(__VA_ARGS__));                     \
					if (do_s)                                                                                                                                                   \
					{                                                                                                                                                           \
						TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime)                                                                                                               \
						{                                                                                                                                                       \
							TRACE_LOG_FUNCTION(&lclTime, tid_, lvl_, _insert, __FILE__, __LINE__, TRACE_NARGS(__VA_ARGS__), ostr__.str().c_str() TRACE_ARGS_ARGS(__VA_ARGS__)); \
						}                                                                                                                                                       \
					}                                                                                                                                                           \
				}                                                                                                                                                               \
			}                                                                                                                                                                   \
		} while (0)

#endif /* defined(___cplusplus) */

/* TRACE_NARGS configured to support 0 - 35 args */
#define TRACE_NARGS(...) TRACE_NARGS_HELP1(__VA_ARGS__, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0) /* 0 here but not below */
#define TRACE_NARGS_HELP1(...) TRACE_NARGS_HELP2(__VA_ARGS__, unused)                                                                                                                         /* "unused" to avoid warning "requires at least one argument for the "..." in a variadic macro" */
#define TRACE_NARGS_HELP2(fmt, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31, x32, x33, x34, x35, n, ...) n
#define TRACE_CNTL(...) traceCntl(TRACE_NAME, TRACE_NARGS(__VA_ARGS__), __VA_ARGS__)
#define TRACE_ARGS_FMT(first, ...) first
/* TRACE_ARGS_ARGS(...) ignores the 1st arg (the "format" arg) and returns the remaining "args", if any.
   Being able
   The trick is: the number of args in __VA_ARGS__ "shifts" the appropriate XX*(__VA_ARGS__) macro
   to the DO_THIS postition in the DO_XX macro. Then only that appropriate XX*(__VA_ARGS__) macro is
   evalutated; the others are ignored. The only 2 choices are XXX_X() or XXX_0(); XXX_X is for when there
   is between 1 and 35 args and XXX_0 is for 0 args. */
#define TRACE_ARGS_ARGS(...)                                          \
	DO_XX(__VA_ARGS__,                                                \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), \
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_0(__VA_ARGS__), unused) /* "unused" to avoid warning "requires at least one argument for the "..." in a variadic macro" */
#define DO_XX(fmt, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31, x32, x33, x34, x35, do_this, ...) do_this
#define XXX_0(A)
#define XXX_X(A, ...) , __VA_ARGS__

#if defined(__CYGWIN__) /* check this first as __x86_64__ will also be defined */

#	define TRACE_XTRA_PASSED
#	define TRACE_XTRA_UNUSED
#	define TRACE_PRINTF_FMT_ARG_NUM 6
#	define TRACE_VA_LIST_INIT(addr) (va_list) addr
#	define TRACE_ENT_TV_FILLER
#	define TRACE_TSC32(low) __asm__ __volatile__("rdtsc;movl %%eax,%0" \
												  : "=m"(low)::"eax", "edx")

#elif defined(__i386__)

#	define TRACE_XTRA_PASSED
#	define TRACE_XTRA_UNUSED
#	define TRACE_PRINTF_FMT_ARG_NUM 6
#	define TRACE_VA_LIST_INIT(addr) (va_list) addr
#	define TRACE_ENT_TV_FILLER uint32_t x[2];
#	define TRACE_TSC32(low) __asm__ __volatile__("rdtsc;movl %%eax,%0" \
												  : "=m"(low)::"eax", "edx")

#elif defined(__x86_64__)

#	define TRACE_XTRA_PASSED , .0, .0, .0, .0, .0, .0, .0, .0
#	define TRACE_XTRA_UNUSED , double d0 __attribute__((__unused__)), double d1 __attribute__((__unused__)), double d2 __attribute__((__unused__)), double d3 __attribute__((__unused__)), double d4 __attribute__((__unused__)), double d5 __attribute__((__unused__)), double d6 __attribute__((__unused__)), double d7 __attribute__((__unused__))
#	define TRACE_PRINTF_FMT_ARG_NUM 14
#	define TRACE_VA_LIST_INIT(addr)              \
		{                                         \
			{                                     \
				6 * 8, 6 * 8 + 9 * 16, addr, addr \
			}                                     \
		}
#	define TRACE_ENT_TV_FILLER
#	define TRACE_TSC32(low) __asm__ __volatile__("rdtsc"     \
												  : "=a"(low) \
												  :           \
												  : "edx") /*NOLINT*/

#elif defined(__powerpc__) && !defined(__powerpc64__)

#	define TRACE_XTRA_PASSED , 0, 0, .0, .0, .0, .0, .0, .0, .0, .0
#	define TRACE_XTRA_UNUSED , long l1 __attribute__((__unused__)), long l2 __attribute__((__unused__)), double d0 __attribute__((__unused__)), double d1 __attribute__((__unused__)), double d2 __attribute__((__unused__)), double d3 __attribute__((__unused__)), double d4 __attribute__((__unused__)), double d5 __attribute__((__unused__)), double d6 __attribute__((__unused__)), double d7 __attribute__((__unused__))
#	define TRACE_PRINTF_FMT_ARG_NUM 16
#	define TRACE_VA_LIST_INIT(addr) \
		{                            \
			{                        \
				8, 8, 0, addr        \
			}                        \
		}
#	define TRACE_ENT_TV_FILLER uint32_t x[2];
#	define TRACE_TSC32(low)

#else

#	define TRACE_XTRA_PASSED
#	define TRACE_XTRA_UNUSED
#	define TRACE_PRINTF_FMT_ARG_NUM 6
#	define TRACE_VA_LIST_INIT(addr) \
		{                            \
			addr                     \
		}
#	if defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 4
#		define TRACE_ENT_TV_FILLER uint32_t x[2];
#	else
#		define TRACE_ENT_TV_FILLER
#	endif
#	define TRACE_TSC32(low)

#endif

static void trace(struct timeval * /*tvp*/, int /*trcId*/, uint16_t /*lvl*/, int32_t /*line*/, uint16_t TRACE_XTRA_UNUSED, const char * /*msg*/, ...) __attribute__((format(printf, TRACE_PRINTF_FMT_ARG_NUM, TRACE_PRINTF_FMT_ARG_NUM + 1)));
#ifdef __cplusplus
static void trace(struct timeval *, int, uint16_t, int32_t, uint16_t TRACE_XTRA_UNUSED, const std::string &, ...);
#endif

union trace_mode_u
{
	struct
	{
		uint32_t M : 1; /* b0 high speed circular Memory */
		uint32_t S : 1; /* b1 printf (formatted) to Screen/Stdout */
	} bits;
	uint32_t mode;
};

typedef char trace_vers_t[sizeof(int64_t) * 16];

struct traceControl_rw
{
	TRACE_ATOMIC_T wrIdxCnt;                                                                             /* 32 bits */
	uint32_t cacheline1[TRACE_CACHELINE / sizeof(int32_t) - (sizeof(TRACE_ATOMIC_T) / sizeof(int32_t))]; /* the goal is to have wrIdxCnt in it's own cache line */

	TRACE_ATOMIC_T namelock;                                                                             /* 32 bits */
	uint32_t cacheline2[TRACE_CACHELINE / sizeof(int32_t) - (sizeof(TRACE_ATOMIC_T) / sizeof(int32_t))]; /* the goal is to have wrIdxCnt in it's own cache line */

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
	uint32_t xtra[TRACE_CACHELINE / sizeof(int32_t) - 10]; /* force some sort of alignment -- taking into account the 6 fields (above) since the last cache line alignment */
};
struct traceControl_s
{
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
	uint32_t page_align[TRACE_PAGESIZE / sizeof(int32_t) - (11 + sizeof(trace_vers_t) / sizeof(int32_t))]; /* allow mmap 1st page(s) (stuff above) readonly */

	/* "page" break */
	struct traceControl_rw rw;
};
/*                                      bytes  TRACE_SHOW cntl char */
struct traceEntryHdr_s /*-----   -------        */
{
	struct timeval time; /* 16        T */
	TRACE_ENT_TV_FILLER  /* because timeval is larger on x86_64 (16 bytes compared to 8 for i686) */

		uint64_t tsc; /*  8        t */
	pid_t pid;        /*  4        P system info */
	pid_t tid;        /*  4        i system info - "thread id" */

	int32_t cpu;                /*  4  %3u   C -- kernel sched switch will indicate this info? */
	uint32_t linenum;           /*  4  %5u   u */
	int32_t TrcId;              /*  4  %4u   I Trace ID ==> idx into lvlTbl, namTbl */
	uint8_t get_idxCnt_retries; /*  1  %1u   R */
	uint8_t nargs;              /*  1  %4u   # */
	uint8_t lvl;                /*  1  %2d   L or l */
	uint8_t param_bytes;        /*  1  %1u   B */
};                              /* ---       M -- NO, ALWAY PRINTED LAST! formated Message */
/* msg buf,then params buf               48   adding uint32_t line;char file[60] (another cache line) doesn't seem worth it */
/* see entSiz(siz_msg,num_params) and idxCnt2entPtr(idxCnt) */ /* other - N  index */

struct traceNamLvls_s
{
	uint64_t M;
	uint64_t S;
	uint64_t T;
	char name[TRACE_DFLT_NAM_CHR_MAX + 1];
};

/*--------------------------------------------------------------------------*/
/* Enter the 5 use case "areas" -- see doc/5in1.txt                         */
/*  defining TRACE_DEFINE wins over DECLARE or nothing. STATIC wins over all */
/* It's OK if one module has DEFINE and all the reset have any of (nothing, DECLARE, STATIC) */
/* The only thing that is invalid is if more than one has DEFINE.           */
/* If any have DECLARE, then there must be one (and only one) with DEFINE.  */
#if defined(TRACE_STATIC) && !defined(__KERNEL__)
#	define TRACE_DECL(var_type_and_name, initializer) static var_type_and_name initializer
#elif defined(TRACE_DEFINE)
#	define TRACE_DECL(var_type_and_name, initializer) var_type_and_name initializer
#elif defined(TRACE_DECLARE) || defined(__KERNEL__)
#	define TRACE_DECL(var_type_and_name, initializer) extern var_type_and_name
#else
#	define TRACE_DECL(var_type_and_name, initializer) static var_type_and_name initializer
#endif

/*#define TRACE_THREAD_LOCALX TRACE_THREAD_LOCAL    * use this for separate FILE per thread -- very rare; perhaps NUMA issue??? */
#define TRACE_THREAD_LOCALX

TRACE_DECL(struct traceControl_s traceControl[2], ); /* for when TRACE is disabled. NOTE: traceNamLvls_p should always point to traceControl_p+1 */
TRACE_DECL(TRACE_THREAD_LOCALX struct traceControl_s *traceControl_p, = NULL);
TRACE_DECL(TRACE_THREAD_LOCALX struct traceControl_rw *traceControl_rwp, = NULL);
TRACE_DECL(TRACE_THREAD_LOCALX struct traceNamLvls_s *traceNamLvls_p, = (struct traceNamLvls_s *)&traceControl[1]);
TRACE_DECL(TRACE_THREAD_LOCALX struct traceEntryHdr_s *traceEntries_p, );
static TRACE_THREAD_LOCAL int traceTID = -1; /* idx into lvlTbl, namTbl -- always
												static (never global) and possibly
thread_local -- this will cause the most traceInit calls (but not much work, hopefully),
which will ensure that a module (not thread) can be assigned it's own trace name/id. */

TRACE_DECL(uint64_t trace_lvlS, = 0);
TRACE_DECL(uint64_t trace_lvlM, = 0);
TRACE_DECL(const char *tracePrint_cntl, = NULL); /* hardcoded default below that can only be overridden via env.var */

#if defined(__KERNEL__)
TRACE_DECL(int trace_allow_printk, = 0);  /* module_param */
TRACE_DECL(char trace_print[200], = {0}); /* module_param */
static TRACE_THREAD_LOCAL const char *traceName = TRACE_DFLT_NAME;
#else
TRACE_DECL(TRACE_THREAD_LOCAL const char *traceName, = TRACE_DFLT_NAME);
TRACE_DECL(TRACE_THREAD_LOCALX const char *traceFile, = "/tmp/trace_buffer_%u"); /*a local/efficient FS device is best; operation when path is on NFS device has not been studied*/
TRACE_DECL(TRACE_THREAD_LOCAL pid_t traceTid, = 0);                              /* thread id */
TRACE_DECL(pid_t tracePid, = 0);
TRACE_DECL(int tracePrintFd[2], = {TRACE_PRINT_FD});
TRACE_DECL(TRACE_ATOMIC_T traceInitLck, = TRACE_ATOMIC_INIT);
TRACE_DECL(uint32_t traceInitLck_hung_max, = 0);
TRACE_DECL(const char *traceTimeFmt, = NULL); /* hardcoded default below that can only be overridden via env.var */
static char traceFile_static[PATH_MAX] = {0};
static struct traceControl_s *traceControl_p_static = NULL;
#endif
/*--------------------------------------------------------------------------*/

/* forward declarations, important functions */
static struct traceEntryHdr_s *idxCnt2entPtr(uint32_t idxCnt);
#if !defined(__KERNEL__) || defined(TRACE_IMPL) /* K=0,IMPL=0; K=0,IMPL=1; K=1,IMPL=1 */
static int traceInit(const char *_name, int allow_ro);
static void traceInitNames(struct traceControl_s * /*tC_p*/, struct traceControl_rw * /*tC_rwp*/);
#	ifdef __KERNEL__                           /*                         K=1,IMPL=1 */
static int msgmax = TRACE_DFLT_MAX_MSG_SZ;      /* module_param */
static int argsmax = TRACE_DFLT_MAX_PARAMS;     /* module_param */
static int numents = TRACE_DFLT_NUM_ENTRIES;    /* module_param */
static int namtblents = TRACE_DFLT_NAMTBL_ENTS; /* module_param */
static int trace_buffer_numa_node = -1;         /* module_param */
#	endif
#else /*                                         K=1,IMPL=0  */

#endif /*  __KERNEL__             TRACE_IMPL  */

static long traceCntl(const char *_name, int nargs, const char *cmd, ...);
static int32_t name2TID(const char *nn);
#define cntlPagesSiz() ((uint32_t)sizeof(struct traceControl_s))
#define namtblSiz(ents) (((uint32_t)sizeof(struct traceNamLvls_s) * (ents) + TRACE_CACHELINE - 1) & ~(TRACE_CACHELINE - 1))
#define entSiz(siz_msg, num_params) (sizeof(struct traceEntryHdr_s) + sizeof(uint64_t) * (num_params) /* NOTE: extra size for i686 (32bit processors) */ \
									 + (siz_msg))
#define traceMemLen(siz_cntl_pages, num_namLvlTblEnts, siz_msg, num_params, num_entries) \
	(((siz_cntl_pages) + namtblSiz(num_namLvlTblEnts) + entSiz(siz_msg, num_params) * (num_entries) + TRACE_PAGESIZE - 1) & ~(TRACE_PAGESIZE - 1))

/* The "largest_multiple" method (using (ulong)-1) allows "easy" "add 1"
   I must do the substract (ie. add negative) by hand.
   Ref. ShmRW class (~/src/ShmRW?)
   Some standards don't seem to line "static inline"
   Use of the following seems to produce the same code as the optimized
   code which calls inline idxCnt_add (the c11/c++11 optimizer seem to do what
   this macro does).
   NOTE: when using macro version, "add" should be of type int32_t */
#if 1
#	define IDXCNT_ADD(idxCnt, add)                                                                               \
		(((add) < 0)                                                                                              \
			 ? (((uint32_t) - (add) > (idxCnt))                                                                   \
					? (traceControl_p->largest_multiple - (-(add) - (idxCnt))) % traceControl_p->largest_multiple \
					: ((idxCnt) - (-(add))) % traceControl_p->largest_multiple)                                   \
			 : ((idxCnt) + (add)) % traceControl_p->largest_multiple)
#else
static uint32_t IDXCNT_ADD(uint32_t idxCnt, int32_t add)
{
	uint32_t retval;
	if (add < 0)
		if (-add > idxCnt)
			retval = (traceControl_p->largest_multiple - (-add - idxCnt)) % traceControl_p->largest_multiple;
		else
			retval = (idxCnt - (-add)) % traceControl_p->largest_multiple;
	else
		retval = (idxCnt + add) % traceControl_p->largest_multiple;
	return retval;
}
#endif
#define IDXCNT_DELTA(cur, prv) \
	(((cur) >= (prv))          \
		 ? (cur) - (prv)       \
		 : (cur) - (prv)-traceControl_p->largest_zero_offset)

typedef void (*trace_log_function_type)(struct timeval *, int, uint16_t, const char *, const char *, int, uint16_t, const char *, ...);
#ifndef TRACE_LOG_FUNCTION
#	define TRACE_LOG_FUNCTION trace_user
#elif defined(TRACE_LOG_FUN_PROTO)
/* prototype for TRACE_LOG_FUNCTION as compiled in Streamer class below */
TRACE_LOG_FUN_PROTO;
#endif /* TRACE_LOG_FUNCTION */

static uint32_t trace_lock(TRACE_ATOMIC_T *atomic_addr)
{
	uint32_t desired = 1, expect = 0, hung = 0;
#if defined(__KERNEL__)
	while (cmpxchg(atomic_addr, expect, desired) != expect)
		if (++hung > 100000000) break;
#elif (defined(__cplusplus) && (__cplusplus >= 201103L)) || defined(TRACE_C11_ATOMICS)
	while (!atomic_compare_exchange_weak(atomic_addr, &expect, desired))
	{
		expect = 0;
		if (++hung > 100000000)
		{
			break;
		}
	}
	if (atomic_addr == &traceInitLck && traceInitLck_hung_max < hung)
	{
		traceInitLck_hung_max = hung;
	}
#else
	while (cmpxchg(atomic_addr, expect, desired) != expect)
		if (++hung > 100000000) break;
	if (atomic_addr == &traceInitLck && traceInitLck_hung_max < hung) traceInitLck_hung_max = hung;
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

typedef enum
{
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

/* if a "do print" (return true/1) and if insert is provided and sz
   is non-zero, it will be NULL terminated */
SUPPRESS_NOT_USED_WARN
static inline int limit_do_print(struct timeval *tvp, limit_info_t *info, char *insert, size_t sz)
{
	uint64_t delta_ms, tnow_ms;
	int do_print;
	/*struct timeval tvx;
	  trace( &tvx, 125, 1, __LINE__, 1 TRACE_XTRA_PASSED, "limit_do_print _cnt_=%u", traceControl_rwp->limit_cnt_limit );*/

	if (traceControl_rwp->limit_cnt_limit == 0)
	{
		if (insert && sz)
		{
			*insert = '\0';
		}
		return (1);
	}
	if (tvp->tv_sec == 0)
	{
		TRACE_GETTIMEOFDAY(tvp);
	}
	tnow_ms = tvp->tv_sec * 1000 + tvp->tv_usec / 1000;
	/* could lock  trace_lock( &(info->lock) );*/
	delta_ms = tnow_ms - info->span_start_ms;
	if (info->state == lsFREE)
	{
		if (++(info->cnt) >= traceControl_rwp->limit_cnt_limit)
		{
			if (insert)
			{
				strncpy(insert, "[RATE LIMIT]", sz);
				/*fprintf( stderr, "[LIMIT (%u/%.1fs) REACHED]\n", traceControl_rwp->limit_cnt_limit, (float)traceControl_rwp->limit_span_on_ms/1000000);*/
			}
			info->state = lsLIMITED;
			info->span_start_ms = tnow_ms; /* start tsLIMITED timespan */
			info->cnt = 0;
		}
		else if (delta_ms >= traceControl_rwp->limit_span_on_ms)
		{ /* start new timespan */
			info->span_start_ms = tnow_ms;
			info->cnt = 1;
			if (insert && sz)
			{
				*insert = '\0';
			}
		}
		else if (insert && sz)
		{ /* counting messages in this period */
			*insert = '\0';
		}
		do_print = 1;
	}
	else
	{ /* state must be tsLIMITED */
		if (delta_ms >= traceControl_rwp->limit_span_off_ms)
		{ /* done limiting, start new timespace */
			if (insert)
			{
				snprintf(insert, sz, "[RESUMING dropped: %u]", info->cnt);
			}
			info->state = lsFREE;
			info->span_start_ms = tnow_ms;
			info->cnt = 0;
			do_print = 1;
		}
		else
		{
			++(info->cnt);
			do_print = 0;
		}
	}
	/* unlock  trace_unlock( &(info->lock) );*/
	return (do_print);
} /* limit_do_print */

#ifndef TRACE_ADJUST_FILE
SUPPRESS_NOT_USED_WARN
static const char *trace_path_components(const char *in_cp, int n_components)
{
	const char *tmp_cp = in_cp + strlen(in_cp);
	if (n_components <= 0)
		return (in_cp);
	for (; tmp_cp != in_cp; --tmp_cp)
	{
		if (*tmp_cp == '/' && --n_components == 0)
			break;
	}
	if (*tmp_cp == '/')
		++tmp_cp;
	return (tmp_cp);
}
#	define TRACE_ADJUST_FILE(FFF) trace_path_components(FFF, 2)
#endif

#ifndef TRACE_4_LVLSTRS
#	define TRACE_4_LVLSTRS "err", "wrn", "nfo", "dbg"
#endif
#ifndef TRACE_60_LVLSTRS
#	define TRACE_60_LVLSTRS "d04", "d05", "d06", "d07", "d08", "d09",                             \
							 "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19", \
							 "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", \
							 "d30", "d31", "d32", "d33", "d34", "d35", "d36", "d37", "d38", "d39", \
							 "d40", "d41", "d42", "d43", "d44", "d45", "d46", "d47", "d48", "d49", \
							 "d50", "d51", "d52", "d53", "d54", "d55", "d56", "d57", "d58", "d59", \
							 "d60", "d61", "d62", "d63"
#endif
static const char _lvlstr[64][4] = {TRACE_4_LVLSTRS, TRACE_60_LVLSTRS};

SUPPRESS_NOT_USED_WARN
static void vtrace_user(struct timeval *tvp, int TrcId, uint16_t lvl, const char *insert, const char *file, int line, uint16_t nargs, const char *msg, va_list ap)
{
	/* I format output in a local output buffer (with specific/limited size)
	   first. There are 2 main reasons that this is done:
	   1) allows the use of write to a specific tracePrintFd;
	   2) there will be one system call which is most efficient and less likely
	   to have the output mangled in a multi-threaded environment.
	*/
#ifdef __KERNEL__
	char obuf[256]; /* kernel has restricted stack size */
#else
	char obuf[TRACE_USER_MSGMAX];
	char *cp;
	struct tm tm_s;
	int quiet_warn = 0;
	int useconds;
#endif
	char tbuf[0x100];
	size_t printed = 0; /* does not include '\0' */
	const char *print_cntl;
	size_t print_cntl_len;
	int retval = 0, msg_printed = 0;

#ifdef __KERNEL__
	if (!trace_allow_printk) return;
#endif

	if (tracePrint_cntl == NULL)
	{
#ifdef __KERNEL__
		if (trace_print[0] != '\0')
		{
			tracePrint_cntl = trace_print;
		}
#else
		if ((cp = getenv("TRACE_PRINT")) != NULL)
		{
			tracePrint_cntl = cp;
			if (strlen(tracePrint_cntl) > 200)
			{ /* cannot see how this could be OK/desirable */
				fprintf(stderr, "Invalid TRACE_PRINT environment variable value.\n");
				tracePrint_cntl = TRACE_PRINT; /* assume this (potentially user supplied) is more valid */
			}
		}
#endif
		else
			tracePrint_cntl = TRACE_PRINT;
	}

	obuf[0] = '\0';
	print_cntl = tracePrint_cntl;
	if (*print_cntl == '\0')
		print_cntl = "%m";
	print_cntl_len = strlen(print_cntl); /* used to make sure end of TRACE_PRINT can be printed even with too large msg */
										 /* NOTE: could count '%' and times 2 and substract that from print_cntl_len */
#define ROOM_FOR_NL (sizeof(obuf) - 2)   /* actually room for \n and \0 */
#define PRINTSIZE(printed) (printed < (ROOM_FOR_NL - print_cntl_len)) ? ROOM_FOR_NL - print_cntl_len - printed : 0
#define SNPRINTED(rr, ss) ((((size_t)(rr) + 1) < (ss)) ? (size_t)(rr) : ((ss) ? (ss)-1 : 0)) /* TRICKY - rr is strlen and ss is sizeof. When ss is 0 or 1, it's strlen should be 0 */
	for (; *print_cntl; ++print_cntl)
	{
		if (*print_cntl != '%')
		{
			if (printed < (ROOM_FOR_NL))
			{ /* -2 to leave room for final \n\0 */
				obuf[printed++] = *print_cntl;
				obuf[printed] = '\0';
			}
			continue;
		}
		if (*++print_cntl == '\0')
		{ /* inc past % */
			/* trailing/ending '%' in spec is probably a mistake??? */
			if (printed < (ROOM_FOR_NL))
			{ /* -2 to leave room for final \n\0 */
				obuf[printed++] = '%';
				obuf[printed] = '\0';
			}
			break;
		}
		switch (*print_cntl)
		{
			case '%':
			case ' ':
			case ':':
			case ';':
			case '\t':
			case '\n':
			case '|':
			case '[':
			case ']':
			case ',':
			case '-':
			case '_': /* separators */
				if (printed < (ROOM_FOR_NL))
				{ /* -1 to leave room for final \n */
					obuf[printed++] = *print_cntl;
					obuf[printed] = '\0';
				}
				continue; /* avoid any further adjustment to "printed" variable */
			case 's':     /* "severity" -- just first charater of level string */
				if (printed < (ROOM_FOR_NL) && _lvlstr[lvl & LVLBITSMSK])
				{ /* -2 to leave room for final \n */
					obuf[printed++] = _lvlstr[lvl & LVLBITSMSK][0];
					obuf[printed] = '\0';
				}
				continue; /* avoid any further adjustment to "printed" variable */

				/* ALL THE ITEMS BELOW COULD GET TRUNKCATED IF PRINTED AFTER LARGE MSG */

			case 'a': /* nargs */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%u", nargs);
				break;
			case 'C': /* CPU i.e. core */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%" MM_STR(TRACE_CPU_WIDTH) "d", trace_getcpu());
				break;
			case 'e': /* TrcName:linenum */
				snprintf(tbuf, sizeof(tbuf), "%s:%d", traceNamLvls_p[TrcId].name, line);
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%*s", traceControl_rwp->longest_name + 1 + TRACE_LINENUM_WIDTH, tbuf); /* +1 for ':' */
				break;
			case 'f': /* filename */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%s", TRACE_ADJUST_FILE(file));
				break;
			case 'I': /* TrcId */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%d", TrcId);
				break;
			case 'i': /* thread id */
#if defined(__KERNEL__)
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%" MM_STR(TRACE_TID_WIDTH) "d", current->pid);
#else
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%" MM_STR(TRACE_TID_WIDTH) "d", traceTid); /* thread */
#endif
				break;
			case 'L': /* level string */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%s", _lvlstr[lvl & LVLBITSMSK] ? _lvlstr[lvl & LVLBITSMSK] : "");
				break;
			case 'l': /* lvl int */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%2u", lvl);
				break;
#if defined(__STDC_VERSION__) && (__GNUC__ >= 7)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
#endif
			case 'M': /* msg limit insert - fall through to msg*/
				if (insert[0])
				{
					/* space separator only printed if insert is non-empty */
					retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%s ", insert);
					printed += SNPRINTED(retval, PRINTSIZE(printed));
				}
				/*break; FALL through */
#if defined(__cplusplus) && (__GNUC__ >= 7)  //(__cplusplus >= 201703L) warning happen even with c++11
#	if __has_cpp_attribute(fallthrough)
				[[fallthrough]];
#	else
				[[gnu:fallthrough]];
#	endif
#endif
			case 'm': /* msg */
#if defined(__STDC_VERSION__) && (__GNUC__ >= 7)
#	pragma GCC diagnostic pop
#endif
				if (nargs)
				{
					retval = msg_printed = vsnprintf(&(obuf[printed]), PRINTSIZE(printed), msg, ap);
				}
				else
				{
					/* don't do any parsing for format specifiers in the msg -- tshow will
				   also know to do this on the memory side of things */
					retval = msg_printed = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%s", msg);
				}
				break;
			case 'N': /* trace name - not padded */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%s", traceNamLvls_p[TrcId].name);
				break;
			case 'n': /* trace name - padded */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%*s", traceControl_rwp->longest_name, traceNamLvls_p[TrcId].name);
				break;
			case 'P': /* process id */
#if defined(__KERNEL__)
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%" MM_STR(TRACE_TID_WIDTH) "d", current->tgid);
#else
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%" MM_STR(TRACE_TID_WIDTH) "d", tracePid);
#endif
				break;
			/*case 's': ABOVE */
			case 'T': /* Time */
				if (tvp->tv_sec == 0)
					TRACE_GETTIMEOFDAY(tvp);
#ifdef __KERNEL__
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%ld.%06ld", tvp->tv_sec, tvp->tv_usec);
#else
				if (traceTimeFmt == NULL)
				{
					/* no matter who writes, it should basically be the same thing */
					if ((cp = getenv("TRACE_TIME_FMT")) != NULL)
						traceTimeFmt = cp; /* single write here */
					else
						traceTimeFmt = TRACE_DFLT_TIME_FMT; /* OR single write here */
				}
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#		pragma GCC diagnostic ignored "-Wformat-nonliteral"
#	endif
				localtime_r((time_t *)&tvp->tv_sec, &tm_s);
				if (strftime(tbuf, sizeof(tbuf), traceTimeFmt, &tm_s) == 0)
					tbuf[0] = '\0';
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic pop
#	endif
				useconds = (int)tvp->tv_usec;
				if ((cp = strstr(tbuf, "%0")) && *(cp + 2) >= '1' && *(cp + 2) <= '5' && *(cp + 3) == 'd')
				{
					// NOTE: if *(cp+2)==6, don't do anything.
					// Example: fmt originally had "%%04d" which got changed by strftime to "%04d";
					// grab the character '4' and adjust useconds accordingly.
					int div;
					switch (*(cp + 2))
					{
						case '1':
							div = 100000;
							break;
						case '2':
							div = 10000;
							break;
						case '3':
							div = 1000;
							break;
						case '4':
							div = 100;
							break;
						case '5':
							div = 10;
							break;
					}
					useconds = (unsigned)((double)useconds / div + 0.5);  // div, round and cast back to unsigned
				}
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wformat-nonliteral"
#	endif
				retval = snprintf(&obuf[printed], PRINTSIZE(printed), tbuf, useconds); /* possibly (probably) add usecs */
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#		pragma GCC diagnostic pop
#	endif
#endif /* __KERNEL__ */
				break;
			case 't': /* msg limit insert - may be null */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%s", insert);
				break;
			case 'u': /* lineNumber */
				retval = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%" MM_STR(TRACE_LINENUM_WIDTH) "d", line);
				break;
		}
		printed += SNPRINTED(retval, PRINTSIZE(printed));
	}

	/* a restriction on the TRACE_PRINT spec is that it must print the message */
	if (msg_printed == 0)
	{
		if (!strchr(" \t:-|", obuf[printed - 1]))
		{ /* if the specification does not end in a "separator" */
			obuf[printed++] = ' ';
			obuf[printed] = '\0';
		}
		if (nargs)
		{
			retval = msg_printed = vsnprintf(&(obuf[printed]), PRINTSIZE(printed), msg, ap);
		}
		else
		{
			/* don't do any parsing for format specifiers in the msg -- tshow will
			   also know to do this on the memory side of things */
			retval = msg_printed = snprintf(&(obuf[printed]), PRINTSIZE(printed), "%s", msg);
		}
		printed += SNPRINTED(retval, PRINTSIZE(printed));
	}

	if (printed < ROOM_FOR_NL)
	{ /* SHOULD/COULD BE AN ASSERT */
		/* there is room for the \n */
		/* buf first see if it is needed */
		if (obuf[printed - 1] != '\n' && obuf[printed - 1] != '\r')
		{
			obuf[printed++] = '\n'; /* overwriting \0 is OK as we will specify the amount to write */
									/*printf("added \\n printed=%d\n",printed);*/
			obuf[printed] = '\0';
		}
		/*else printf("already there printed=%d\n",printed);*/
#if defined(__KERNEL__)
		printk(obuf);
#else
		quiet_warn = write(lvl?tracePrintFd[0]:tracePrintFd[1], obuf, printed);
		if (quiet_warn == -1)
			perror("writeTracePrintFd");
#endif
	}
	else
	{
		/* obuf[sizeof(obuf)-1] has '\0'. */
		if (obuf[sizeof(obuf) - 2] != '\n' && obuf[sizeof(obuf) - 2] != '\r')
		{
			obuf[sizeof(obuf) - 2] = '\n';
		}
#if defined(__KERNEL__)
		printk(obuf);
#else
		quiet_warn = write(lvl?tracePrintFd[0]:tracePrintFd[1], obuf, sizeof(obuf) - 1);
		if (quiet_warn == -1)
			perror("writeTracePrintFd");
#endif
	}
	/*TRACE_PRN("sizeof(obuf)=%zu retval=%d msg_printed=%d printed=%zd\n",sizeof(obuf), retval, msg_printed, printed);*/
} /* vtrace_user */
SUPPRESS_NOT_USED_WARN
static void trace_user(struct timeval *tvp, int TrcId, uint16_t lvl, const char *insert, const char *file, int line, uint16_t nargs, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vtrace_user(tvp, TrcId, lvl, insert, file, line, nargs, msg, ap);
	va_end(ap);
} /* trace_user - const char* */
#ifdef __cplusplus
#	if (__cplusplus >= 201103L)
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wvarargs"
#	endif
SUPPRESS_NOT_USED_WARN
static void trace_user(struct timeval *tvp, int TrcId, uint16_t lvl, const char *insert, const char *file, int line, uint16_t nargs, const std::string &msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vtrace_user(tvp, TrcId, lvl, insert, file, line, nargs, msg.c_str(), ap);
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
	if (tracePid != (chkpid = getpid())) { tracePid = traceTid = chkpid; }

/* now test is defaults need to be changed. GLIBC is relevant for user space   */
#if defined(__GLIBC_PREREQ)
/* if undefined __GLIBC_PREREQ is used in a #if (i.e. previous line), some preprocessor will give:
    error: missing binary operator before token "("
 */
#	if __GLIBC_PREREQ(2, 27)
#		ifdef __cplusplus
extern "C" int __register_atfork(void (*)(void), void (*)(void), void (*)(void));
#		else
extern int __register_atfork(void (*)(void), void (*)(void), void (*)(void));
#		endif
static void trace_pid_atfork(void)
{
	/*traceTid=tracePid=getpid();TRACEN("TRACE",61,"trace_pid_atfork " __BASE_FILE__ );*/
	const char *rp;
	char somebuf[120];
	somebuf[0] = '\0';
	traceTid = tracePid = getpid();
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wformat-security"
#	endif
	TRACEN("TRACE", 61,
		   (somebuf[0]
				? &(somebuf[0])
				: (snprintf(&(somebuf[0]), sizeof(somebuf), "trace_pid_atfork %s",
							(rp = rindex(__BASE_FILE__, '/')) != NULL
								? rp + 1
								: __BASE_FILE__),
				   &(somebuf[0]))));
#	if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#		pragma GCC diagnostic pop
#	endif
}
#		undef TRACE_REGISTER_ATFORK
#		define TRACE_REGISTER_ATFORK __register_atfork(NULL, NULL, trace_pid_atfork)
#		undef TRACE_CHK_PID
#		define TRACE_CHK_PID
#	endif
#endif

static void vtrace(struct timeval *tvp, int trcId, uint16_t lvl, int32_t line, uint16_t nargs, const char *msg, va_list ap)
{
	struct traceEntryHdr_s *myEnt_p;
	char *msg_p;
	uint64_t *params_p; /* some archs (ie. i386,32 bit arm) pass have 32 bit and 64 bit args; use biggest */
	unsigned argIdx;
	uint16_t get_idxCnt_retries = 0;
	uint32_t myIdxCnt;
	uint32_t desired;
#ifndef __KERNEL__
	TRACE_CHK_PID;
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

#ifdef __KERNEL__
	/* There are some places in the kernel where the gettimeofday routine
	   cannot be called (i.e. kernel/notifier.c routines). For these routines,
	   add 64 for the level (i.e. 22+64) */
	if (lvl >= 64)
	{
		tvp->tv_sec = 1;
		tvp->tv_usec = 0;
	}
	else
#endif
		if (tvp->tv_sec == 0)
	{
		TRACE_GETTIMEOFDAY(tvp); /* hopefully NOT a system call */
	}

#define TRACE_TSC_EXPERIMENT 0 /* is TSC "consistent" across all core? (only negative is rollover) */
							   /* experiment shows that if time were always retrieved exactly with idx, it
	   would always increment, but this garuntee would cause TRACE to take near 10x longer, realizing
	   that regardless of locking, there is always the possibility that a task will be interrupted
	   between getting the idx and getting the tsc.
	   big_ex w/o lock: tshow|tdelta  -stats|tail|grep ave: 1.8037151; w/ lock: 15.132927 */
#if TRACE_TSC_EXPERIMENT == 1
	if (!trace_lock(&traceControl_rwp->namelock))
	{
		TRACE_PRN("trace_lock: namelock hung?\n");
	}
#endif
	myIdxCnt = TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
	desired = IDXCNT_ADD(myIdxCnt, 1);
#if defined(__KERNEL__)
	while (cmpxchg(&traceControl_rwp->wrIdxCnt, myIdxCnt, desired) != myIdxCnt)
	{
		++get_idxCnt_retries;
		myIdxCnt = TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
		desired = IDXCNT_ADD(myIdxCnt, 1);
	}
#elif (defined(__cplusplus) && (__cplusplus >= 201103L)) || defined(TRACE_C11_ATOMICS)
	while (!atomic_compare_exchange_weak(&traceControl_rwp->wrIdxCnt, &myIdxCnt, desired))
	{
		++get_idxCnt_retries;
		desired = IDXCNT_ADD(myIdxCnt, 1);
	}
#else
	while (cmpxchg(&traceControl_rwp->wrIdxCnt, myIdxCnt, desired) != myIdxCnt)
	{
		++get_idxCnt_retries;
		myIdxCnt = TRACE_ATOMIC_LOAD(&traceControl_rwp->wrIdxCnt);
		desired = IDXCNT_ADD(myIdxCnt, 1);
	}
#endif

	/* Now, "desired" is the count (and myIdxCnt is the index) */
	if (desired == traceControl_p->num_entries)
	{
		traceControl_rwp->full = 1; /* now we'll know if wrIdxCnt has rolled over */
	}

	myEnt_p = idxCnt2entPtr(myIdxCnt);

	/*myEnt_p->time = *tvp;   move to end - reasonable time is indication of complete */
	TRACE_TSC32(myEnt_p->tsc);

#if TRACE_TSC_EXPERIMENT == 1
	trace_unlock(&traceControl_rwp->namelock);
#endif
#undef TRACE_TSC_EXPERIMENT

#if defined(__KERNEL__)
	myEnt_p->pid = current->tgid;
	myEnt_p->tid = current->pid;
#else
	myEnt_p->pid = tracePid;
	myEnt_p->tid = traceTid;
#endif
	myEnt_p->cpu = trace_getcpu(); /* for userspace, this costs alot :(*/
	myEnt_p->linenum = line;
	myEnt_p->TrcId = trcId;
	myEnt_p->lvl = lvl;
	myEnt_p->nargs = nargs;
	myEnt_p->get_idxCnt_retries = get_idxCnt_retries;
	myEnt_p->param_bytes = sizeof(long);

	msg_p = (char *)(myEnt_p + 1);
	params_p = (uint64_t *)(msg_p + traceControl_p->siz_msg);

	strncpy(msg_p, msg, traceControl_p->siz_msg);
	/* emulate stack push - right to left (so that arg1 end up at a lower
	   address, arg2 ends up at the next higher address, etc. */
	if (nargs)
	{
		if (nargs > traceControl_p->num_params)
		{
			nargs = traceControl_p->num_params;
		}
		else if (nargs < traceControl_p->num_params)
			++nargs;  // one xtra for one long double on x86_64
		for (argIdx = 0; argIdx < nargs; ++argIdx)
		{
			params_p[argIdx] = va_arg(ap, uint64_t); /* this will usually copy 2x and 32bit archs, but they might be all %f or %g args */
		}
	}

	myEnt_p->time = *tvp; /* reasonable time (>= prev ent) is indication of complete */

	if (traceControl_rwp->trigActivePost)
	{ /* armed, armed/trigger */
		if (traceControl_rwp->triggered)
		{ /* triggered */
			if (IDXCNT_DELTA(desired, traceControl_rwp->trigIdxCnt) >= traceControl_rwp->trigActivePost)
			{
				/* I think there should be an indication in the M buffer */
				TRACE_CNTL("modeM", 0); /* calling traceCntl here eliminates the "defined but not used" warning for modules which do not use TRACE_CNTL */
				traceControl_rwp->trigActivePost = 0;
				/* triggered and trigIdxCnt should be cleared when
				   "armed" (when trigActivePost is set) */
			}
			/* else just waiting... */
		}
		else if (traceNamLvls_p[trcId].T & TLVLMSK(lvl))
		{
			traceControl_rwp->triggered = 1;
			traceControl_rwp->trigIdxCnt = myIdxCnt;
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
static void trace(struct timeval *tvp, int trcId, uint16_t lvl, int32_t line, uint16_t nargs TRACE_XTRA_UNUSED, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vtrace(tvp, trcId, lvl, line, nargs, msg, ap);
	va_end(ap);
} /* trace */

#ifdef __cplusplus
SUPPRESS_NOT_USED_WARN
static void trace(struct timeval *tvp, int trcId, uint16_t lvl, int32_t line, uint16_t nargs TRACE_XTRA_UNUSED, const std::string &msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vtrace(tvp, trcId, lvl, line, nargs, msg.c_str(), ap);
	va_end(ap);
} /* trace */
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#	pragma GCC diagnostic pop
#endif

/* Search for name anf insert if not found and not full
 */
static int32_t name2TID(const char *nn)
{
	uint32_t ii, len;
	const char *name = (nn && nn[0]) ? nn : traceName;
	char valid_name[TRACE_DFLT_NAM_CHR_MAX + 1];
#if defined(__KERNEL__)
	if (traceEntries_p == NULL) return -1;
#endif
	/*fprintf(stderr,"n2t=%p %s\n",name,name);*/
	for (ii = 0; ii < traceControl_p->num_namLvlTblEnts; ++ii)
	{
		if (strncmp(traceNamLvls_p[ii].name, name, TRACE_DFLT_NAM_CHR_MAX) == 0)
		{
			return (ii);
		}
	}
	/* only allow "valid" names to be inserted -- above, we assumed the
	   name was valid, giving the caller the benefit of the doubt, for
	   efficiency sake, but here we will make sure the name is valid */
	for (ii = 0; name[ii] != '\0' && ii < TRACE_DFLT_NAM_CHR_MAX; ++ii)
	{
		if (isgraph(name[ii]))
		{
			valid_name[ii] = name[ii];
		}
		else
		{
			valid_name[ii] = '_';
		}
	}
	valid_name[ii] = '\0';
	len = ii;
	/* NOTE: multiple threads which may want to create the same name might arrive at this
	   point at the same time. Checking for the name again, with the lock, make this OK.
	*/
	if (!trace_lock(&traceControl_rwp->namelock))
	{
		TRACE_PRN("trace_lock: namelock hung?\n");
	}
	for (ii = 0; ii < traceControl_p->num_namLvlTblEnts; ++ii)
	{
		if (traceNamLvls_p[ii].name[0] == '\0')
		{
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wpragmas"
#	pragma GCC diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
			strncpy(traceNamLvls_p[ii].name, valid_name, TRACE_DFLT_NAM_CHR_MAX);
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#	pragma GCC diagnostic pop
#endif
			if (trace_lvlS)
			{ /* See also traceInitNames */
				traceNamLvls_p[ii].S = trace_lvlS;
			}
			if (trace_lvlM)
			{ /* See also traceInitNames */
				traceNamLvls_p[ii].M = trace_lvlM;
			}
			if (traceControl_rwp->longest_name < len)
			{
				traceControl_rwp->longest_name = len;
			}
			trace_unlock(&traceControl_rwp->namelock);
			return (ii);
		}
		if (strncmp(traceNamLvls_p[ii].name, valid_name, TRACE_DFLT_NAM_CHR_MAX) == 0)
		{
			trace_unlock(&traceControl_rwp->namelock);
			return (ii);
		}
	}
	trace_unlock(&traceControl_rwp->namelock);
	return (traceControl_p->num_namLvlTblEnts - 1); /* the case for when the table is full */
} /* name2TID */

#ifndef __KERNEL__
static void trace_namLvlSet(void)
{
	const char *cp;
	int sts;
	/* This is a signficant env.var as this can be setting the
	   level mask for many TRACE NAMEs/IDs AND potentially many process
	   could do this or at least be TRACE-ing. In other words, this has
	   the potential of effecting another process's TRACE-ing. */
	if ((cp = getenv("TRACE_NAMLVLSET")))
	{
		int ign; /* will ignore trace id (index) if present */
		char name[TRACE_DFLT_NAM_CHR_MAX + 1];
		unsigned long long M, S, T = 0;
		while (((sts = sscanf(cp, "%d %" MM_STR(TRACE_DFLT_NAM_CHR_MAX) "s %llx %llx %llx", &ign, name, &M, &S, &T)) && sts >= 4)  //NOLINT
			   || ((sts = sscanf(cp, "%" MM_STR(TRACE_DFLT_NAM_CHR_MAX) "s %llx %llx %llx", name, &M, &S, &T)) && sts >= 3))       //NOLINT
		{
			int tid = name2TID(name);
			/*fprintf(stderr,"name=%s sts=%d\n",name,sts );*/
			traceNamLvls_p[tid].M = M;
			traceNamLvls_p[tid].S = S;
			traceNamLvls_p[tid].T = T;
			cp = strchr(cp, '\n');
			if (cp == NULL)
			{
				break;
			}
			++cp;
			T = 0;
		}
		if (cp != NULL && *cp != '\0')
		{
			fprintf(stderr, "Warning: TRACE_NAMLVLSET in env., but processing did not complete\n");
		}
	}
	if ((cp = getenv("TRACE_MODE")))
	{
		traceControl_rwp->mode.mode = strtoul(cp, NULL, 0);
	}
	if ((cp = getenv("TRACE_LIMIT_MS")))
	{
		unsigned cnt;
		unsigned long long on_ms, off_ms;
		sts = sscanf(cp, "%u,%llu,%llu", &cnt, &on_ms, &off_ms);  //NOLINT
		switch (sts)
		{
			case 0: /* As a way to temp unset TRACE_LIMIT_MS, allow: TRACE_LIMIT_MS= tinfo */
				break;
#if defined(__STDC_VERSION__) && (__GNUC__ >= 7)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
#endif
			case 2:
				off_ms = on_ms;
				//fall through after setting default off_ms to on_ms
#	if defined(__cplusplus) && (__GNUC__ >= 7)  //(__cplusplus >= 201703L) warning happen even with c++11
#		if __has_cpp_attribute(fallthrough)
				[[fallthrough]];
#		else
				[[gnu:fallthrough]];
#		endif
#	endif
			case 3:
#if defined(__STDC_VERSION__) && (__GNUC__ >= 7)
#	pragma GCC diagnostic pop
#endif
				traceControl_rwp->limit_cnt_limit = cnt;
				traceControl_rwp->limit_span_on_ms = on_ms;
				traceControl_rwp->limit_span_off_ms = off_ms;
				break;
			default:
				fprintf(stderr, "Warning: problem parsing TRACE_LIMIT_MS - should be: <cnt>,<on_ms>[,off_ms]\n");
				traceControl_rwp->limit_cnt_limit = 0;
		}
	}
} /* trace_namLvlSet */
#endif

static inline void trace_msk_op(uint64_t *v1, int op, uint64_t v2)
{
	switch (op)
	{
		case 0:
			*v1 = v2;
			break;
		case 1:
			*v1 |= v2;
			break;
		case 2:
			*v1 &= ~v2;
			break;
	}
}

/* 
passing _name: because this method will have to make sure trace is initialize
and to avoid compiling in a (default) name into (hard-coding) the function in
this header file, _name is passed. _name will then take on the value used when
compiling calling of traceCntl via the TRACE_CNTL macro.

cmd		   		arg	      	  	  notes
------          ---------		--------------
file			char*				1
namlvlset		optional char*		1
mapped			n/a
name			char*				1
mode			UL					1
lvlmskn[MST]	ULL					2
lvlsetn[MST]	ULL					2
lvlclrn[MST]	ULL					2
lvlmsk[MST][gG]	ULL					2
lvlset[MST][gG]	ULL					2
lvlclr[MST][gG]	ULL					2
trig			UL [ULL]			1,2
reset			n/a
limit_ms		UL UL [UL]			1

Note 1: 4 bytes on 32, 8 bytes on 64
Note 2: 8 bytes (LL or ULL) on both 32 and 64 bit machines

find . -name \*.[ch] -o -name \*.cc | xargs grep -n 'TRACE_CNTL('
for cc in file namlvl mapped name mode lvl trig reset limit_ms; do
  echo === $cc ===
  find . -name \*.[ch] -o -name \*.cc | xargs grep -n "TRACE_CNTL( *\"$cc"
done
 */
static long traceCntl(const char *_name, int nargs, const char *cmd, ...)
{
	long ret = 0;
	va_list ap;
	unsigned ii;

	va_start(ap, cmd);

	/* although it may be counter intuitive, this should override
	   env.var as it could be used to set a file-per-thread.
	   NO!!!  -- I think env will over ride, this will just change
	   the default for name/file.
	   NOTE: CAN'T HAVE FILE-PER-THREAD unless traceControl_p,traceEntries_p,traceNamLvls_p are THREAD_LOCAL
	   CAN'T HAVE NAME-PER-THREAD unless traceTID       is THREAD_LOCAL
	*/
#ifndef __KERNEL__
	if (strcmp(cmd, "file") == 0)       /* if TRACE_CNTL "name" and "file" are both used, "file" must be first (i.e. "name" first will _set_ file which won't be changed by "file") */
	{                                   /* THIS really only makes sense for non-thread local-file-for-module or for non-static implementation w/TLS for file-per-thread */
		traceFile = va_arg(ap, char *); /* this can still be overridden by env.var.; suggest testing w. TRACE_ARGSMAX=10*/
		traceInit(_name, 0);            /* will not RE-init as traceControl_p!=NULL skips mmap_file */
		va_end(ap);
		return (0);
	}
	if (strcmp(cmd, "namlvlset") == 0)
	{
		/* use this if program sets TRACE_NAMLVLSET env.var.  This can be used
		   to Init or called trace_namLvlSet() after an Init has occurred. */
		const char *name = (nargs == 0) ? _name : va_arg(ap, char *); /* name is optional */
		/*printf("nargs=%d name=%s\n",nargs,name);*/
		if (traceControl_p == NULL)
		{
			traceInit(name, 0); /* with traceControl_p==NULL. trace_namLvlSet() will be called */
		}
		else
		{
			trace_namLvlSet(); /* recall trace_namLvlSet(). optional name, if given, is ignored */
		}
		va_end(ap);
		return (0);
	}
	if (strcmp(cmd, "mapped") == 0)
	{
		TRACE_INIT_CHECK(_name){};
		ret = (traceControl_p != &traceControl[0]); /* compatible with define TRACE_CNTL(...) (0) */
		va_end(ap);
		return (ret);
	}
#endif
	TRACE_INIT_CHECK(_name){}; /* note: allows name2TID to be called in userspace */

	if (strcmp(cmd, "name") == 0) /* if TRACE_CNTL "name" and "file" are both used, "file" must be first (i.e. "name" first will _set_ file which won't be changed by "file") */
	{                             /* THIS really only makes sense for non-thread local-name-for-module or for non-static implementation w/TLS for name-per-thread */
		char *tnam;               /* this can still be overridden by env.var. IF traceInit(TRACE_NAME,0) is called; suggest testing w. TRACE_ARGSMAX=10*/
		tnam = va_arg(ap, char *);
		traceName = tnam ? tnam : TRACE_DFLT_NAME; /* doing it this way allows this to be called by kernel module */
		traceTID = name2TID(traceName);
	}
	else if (strncmp(cmd, "mode", 4) == 0)
	{ /* this returns the (prv/cur) mode requested */
		switch (cmd[4])
		{
			case '\0':
				ret = traceControl_rwp->mode.mode;
				if (nargs == 1)
				{
					uint32_t mode = va_arg(ap, long);  // 4 bytes on 32, 8 on 64
					union trace_mode_u tmp;
					tmp.mode = mode;
#ifndef __KERNEL__
					if (traceControl_p == &(traceControl[0]))
					{
						tmp.bits.M = 0;
					}
#endif
					traceControl_rwp->mode = tmp;
				}
				break;
			case 'M':
				ret = traceControl_rwp->mode.bits.M;
#ifndef __KERNEL__
				if (traceControl_p == &(traceControl[0]))
				{
					break;
				}
#endif
				if (nargs == 1)
				{
					uint32_t mode = va_arg(ap, long);  // 4 bytes on 32, 8 on 64
					traceControl_rwp->mode.bits.M = mode;
				}
				break;
			case 'S':
				ret = traceControl_rwp->mode.bits.S;
				if (nargs == 1)
				{
					uint32_t mode = va_arg(ap, long);  // 4 bytes on 32, 8 on 64
					traceControl_rwp->mode.bits.S = mode;
				}
				break;
			default:
				ret = -1;
		}
	}
	else if ((strncmp(cmd, "lvlmskn", 7) == 0) || (strncmp(cmd, "lvlsetn", 7) == 0) || (strncmp(cmd, "lvlclrn", 7) == 0))
	{ /* TAKES 2 or 4 args: name,lvlX or name,lvlM,lvlS,lvlT */
		uint64_t lvl, lvlm, lvls, lvlt;
		unsigned ee;
		int op, slen = strlen(&cmd[7]);
		char *name_spec;
		if (slen > 1 || (slen == 1 && !strpbrk(&cmd[6], "MST")))
		{
			TRACE_PRN("only M,S,or T allowed after lvl...n\n");
			va_end(ap);
			return (-1);
		}

		if (strncmp(&cmd[3], "msk", 3) == 0)
		{
			op = 0;
		}
		else if (strncmp(&cmd[3], "set", 3) == 0)
		{
			op = 1;
		}
		else  // must be clr
		{
			op = 2;
		}
		name_spec = va_arg(ap, char *);
		/* find first match */
		ee = traceControl_p->num_namLvlTblEnts;
		for (ii = 0; ii < ee; ++ii)
		{
			if (traceNamLvls_p[ii].name[0] && TMATCHCMP(name_spec, traceNamLvls_p[ii].name))
			{
				break;
			}
		}
		if (ii == ee)
		{
			va_end(ap);
			return (0);
		}
		lvl = va_arg(ap, uint64_t);
		switch (cmd[7])
		{
			case 'M':
				ret = traceNamLvls_p[ii].M;
				for (; ii < ee; ++ii)
				{
					if (traceNamLvls_p[ii].name[0] && TMATCHCMP(name_spec, traceNamLvls_p[ii].name))
					{
						trace_msk_op(&traceNamLvls_p[ii].M, op, lvl);
					}
				}
				break;
			case 'S':
				ret = traceNamLvls_p[ii].S;
				for (; ii < ee; ++ii)
				{
					if (traceNamLvls_p[ii].name[0] && TMATCHCMP(name_spec, traceNamLvls_p[ii].name))
					{
						trace_msk_op(&traceNamLvls_p[ii].S, op, lvl);
					}
				}
				break;
			case 'T':
				ret = traceNamLvls_p[ii].T;
				for (; ii < ee; ++ii)
				{
					if (traceNamLvls_p[ii].name[0] && TMATCHCMP(name_spec, traceNamLvls_p[ii].name))
					{
						trace_msk_op(&traceNamLvls_p[ii].T, op, lvl);
					}
				}
				break;
			default:
				if (nargs != 4)
				{ /* "name" plus 3 lvls */
					TRACE_PRN("need 3 lvlmsks; %d given\n", nargs - 1);
					va_end(ap);
					return (-1);
				}
				lvlm = lvl; /* arg from above */
				lvls = va_arg(ap, uint64_t);
				lvlt = va_arg(ap, uint64_t);
				for (; ii < ee; ++ii)
				{
					if (traceNamLvls_p[ii].name[0] && TMATCHCMP(name_spec, traceNamLvls_p[ii].name))
					{
						trace_msk_op(&traceNamLvls_p[ii].M, op, lvlm);
						trace_msk_op(&traceNamLvls_p[ii].S, op, lvls);
						trace_msk_op(&traceNamLvls_p[ii].T, op, lvlt);
					}
				}
		}
	}
	else if ((strncmp(cmd, "lvlmsk", 6) == 0) || (strncmp(cmd, "lvlset", 6) == 0) || (strncmp(cmd, "lvlclr", 6) == 0))
	{ /* TAKES 1 or 3 args: lvlX or lvlM,lvlS,lvlT */
		uint64_t lvl, lvlm, lvls, lvlt;
		unsigned ee, doNew = 1, op;
		if (strncmp(&cmd[3], "msk", 3) == 0)
		{
			op = 0;
		}
		else if (strncmp(&cmd[3], "set", 3) == 0)
		{
			op = 1;
		}
		else
		{
			op = 2;
		}
		if ((cmd[6] == 'g') || ((cmd[6]) && (cmd[7] == 'g')))
		{
			ii = 0;
			ee = traceControl_p->num_namLvlTblEnts;
		}
		else if ((cmd[6] == 'G') || ((cmd[6]) && (cmd[7] == 'G')))
		{
			ii = 0;
			ee = traceControl_p->num_namLvlTblEnts;
			doNew = 0; /* Capital G short ciruits the "set for future/new trace ids */
		}
		else
		{
			ii = traceTID;
			ee = traceTID + 1;
			switch (cmd[6])
			{
				case 'M':
					ret = traceNamLvls_p[ii].M;
					break;
				case 'S':
					ret = traceNamLvls_p[ii].S;
					break;
				case 'T':
					ret = traceNamLvls_p[ii].T;
					break;
			}
		}
		lvl = va_arg(ap, uint64_t); /* "FIRST" ARG SHOULD ALWAYS BE THERE */
		switch (cmd[6])
		{
			case 'M':
				for (; ii < ee; ++ii)
				{
					if (doNew || traceNamLvls_p[ii].name[0])
					{
						trace_msk_op(&traceNamLvls_p[ii].M, op, lvl);
					}
				}
				break;
			case 'S':
				for (; ii < ee; ++ii)
				{
					if (doNew || traceNamLvls_p[ii].name[0])
					{
						trace_msk_op(&traceNamLvls_p[ii].S, op, lvl);
					}
				}
				break;
			case 'T':
				for (; ii < ee; ++ii)
				{
					if (doNew || traceNamLvls_p[ii].name[0])
					{
						trace_msk_op(&traceNamLvls_p[ii].T, op, lvl);
					}
				}
				break;
			default:
				if (nargs != 3)
				{
					TRACE_PRN("need 3 lvlmsks; %d given\n", nargs);
					va_end(ap);
					return (-1);
				}
				lvlm = lvl; /* "FIRST" arg from above */
				lvls = va_arg(ap, uint64_t);
				lvlt = va_arg(ap, uint64_t);
				for (; ii < ee; ++ii)
				{
					if (!doNew && !traceNamLvls_p[ii].name[0])
					{
						continue;
					}
					trace_msk_op(&traceNamLvls_p[ii].M, op, lvlm);
					trace_msk_op(&traceNamLvls_p[ii].S, op, lvls);
					trace_msk_op(&traceNamLvls_p[ii].T, op, lvlt);
				}
		}
	}
	else if (strcmp(cmd, "trig") == 0)
	{ /* takes 1 or 2 args: postEntries [lvlmsk] - optional 3rd arg will suppress warnings */
		uint64_t lvlsMsk = 0;
		unsigned post_entries = 0;
		if (nargs == 1)
		{
			post_entries = va_arg(ap, unsigned long);  // 4 bytes on 32, 8 on 64
			lvlsMsk = traceNamLvls_p[traceTID].M;
		}
		else if (nargs >= 2)
		{
			post_entries = va_arg(ap, unsigned long);  // 4 bytes on 32, 8 on 64
			lvlsMsk = va_arg(ap, uint64_t);
		}
		if ((traceNamLvls_p[traceTID].M & lvlsMsk) != lvlsMsk)
		{
#ifndef __KERNEL__
			if (nargs == 2)
			{
				fprintf(stderr, "Warning: \"trig\" setting (additional) bits (0x%llx) in traceTID=%d\n", (unsigned long long)lvlsMsk, traceTID);
			}
#endif
			traceNamLvls_p[traceTID].M |= lvlsMsk;
		}
#ifndef __KERNEL__
		if (traceControl_rwp->trigActivePost && nargs == 2)
		{
			fprintf(stderr, "Warning: \"trig\" overwriting trigActivePost (previous=%d)\n", traceControl_rwp->trigActivePost);
		}
#endif
		traceNamLvls_p[traceTID].T = lvlsMsk;
		traceControl_rwp->trigActivePost = post_entries ? post_entries : 1; /* must be at least 1 */
		traceControl_rwp->triggered = 0;
		traceControl_rwp->trigIdxCnt = 0;
	}
	else if (strcmp(cmd, "reset") == 0)
	{
		traceControl_rwp->full = traceControl_rwp->trigIdxCnt = traceControl_rwp->trigActivePost = 0;
		TRACE_ATOMIC_STORE(&traceControl_rwp->wrIdxCnt, (uint32_t)0);
		traceControl_rwp->triggered = 0;
	}
	else if (strcmp(cmd, "limit_ms") == 0)
	{ /* 0, 2 or 3 args: limit_cnt, span_on_ms, [span_off_ms] */
		if (nargs == 0)
		{
			ret = traceControl_rwp->limit_cnt_limit;
		}
		else if (nargs >= 2 && nargs <= 3)
		{
			ret = traceControl_rwp->limit_cnt_limit;
			traceControl_rwp->limit_cnt_limit = va_arg(ap, long);   // 4 bytes on 32, 8 on 64
			traceControl_rwp->limit_span_on_ms = va_arg(ap, long);  // 4 bytes on 32, 8 on 64
			if (nargs == 3)
			{
				traceControl_rwp->limit_span_off_ms = va_arg(ap, long);  // 4 bytes on 32, 8 on 64
			}
			else
			{
				traceControl_rwp->limit_span_off_ms = traceControl_rwp->limit_span_on_ms;
			}
		}
		else
		{
			TRACE_PRN("limit needs 0 or 2 or 3 args (cnt,span_of[,span_off]) %d given\n", nargs);
			va_end(ap);
			return (-1);
		}
	}
	else
	{
		ret = -1;
	}
	va_end(ap);
#ifdef __KERNEL__
	if (ret == -1) printk(KERN_ERR "TRACE: invalid control string %s nargs=%d\n", cmd, nargs);
#else
	if (ret == -1)
	{
		fprintf(stderr, "TRACE: invalid control string %s nargs=%d\n", cmd, nargs);
	}
#endif
	return (ret);
} /* traceCntl */

#if !defined(__KERNEL__) || defined(TRACE_IMPL)

static void trace_created_init(struct traceControl_s *t_p, struct traceControl_rw *t_rwp, uint32_t msgmax, uint32_t argsmax, uint32_t numents, uint32_t namtblents, int memlen, unsigned modeM)
{
	struct timeval tv;
	TRACE_GETTIMEOFDAY(&tv);
#	ifdef TRACE_DEBUG_INIT
	trace_user(&tv, 0, 0, "", 1, "trace_created_init: tC_p=%p", t_p);
#	endif
	strncpy(t_p->version_string, TRACE_REV, sizeof(t_p->version_string));
	t_p->version_string[sizeof(t_p->version_string) - 1] = '\0';
	t_p->create_tv_sec = (uint32_t)tv.tv_sec;
	t_p->num_params = argsmax;
	t_p->siz_msg = msgmax;
	t_p->siz_entry = entSiz(msgmax, argsmax);
	t_p->num_entries = numents;
	t_p->largest_multiple = (uint32_t)-1 - ((uint32_t)-1 % numents);
	t_p->largest_zero_offset = ((uint32_t)-1 % numents) + 1; /* used in DELTA. largest_multiple+largest_zero_offset=0 (w/ rollover) */
	t_p->num_namLvlTblEnts = namtblents;
	t_p->memlen = memlen;

	TRACE_ATOMIC_STORE(&t_rwp->namelock, (uint32_t)0);

	/*TRACE_CNTL( "reset" );  Can't call traceCntl during Init b/c it does an INIT_CHECK and will call Init */
	TRACE_ATOMIC_STORE(&t_rwp->wrIdxCnt, (uint32_t)0);
	t_rwp->full = t_rwp->trigIdxCnt = t_rwp->trigActivePost = t_rwp->triggered = 0;
	t_rwp->limit_span_on_ms = t_rwp->limit_span_off_ms = t_rwp->limit_cnt_limit = 0;

	t_rwp->mode.mode = 0;
	t_rwp->mode.bits.M = modeM;
	t_rwp->mode.bits.S = 1;

	traceInitNames(t_p, t_rwp); /* THIS SETS GLOBAL traceNamLvls_p REFERENCED NEXT. */

	/* this depends on the actual value of the num_namLvlTblEnts which
	   may be different from the "calculated" value WHEN the buffer has
	   previously been configured */
	traceEntries_p = (struct traceEntryHdr_s *)((unsigned long)traceNamLvls_p + namtblSiz(t_p->num_namLvlTblEnts));

	t_p->trace_initialized = 1;
#	ifdef TRACE_DEBUG_INIT
	tv.tv_sec = 0;
	trace_user(&tv, 0, 0, "", 1, "trace_created_init: tC_p=%p", t_p);
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
	const char *inp = input;
	char loguid[15];
	char *cp_uname = NULL, *cp_uid = NULL;

	for (outoff = 0; outoff < bsz && *inp != '\0'; ++inp)
	{
		if (*inp == '%')
		{
			++inp;
			switch (*inp)
			{
				case '%':
					obuf[outoff++] = *inp;
					break;
				case 'u':
					/*  ......................now stop at first non-NULL */
					if (cp_uname == NULL && ((cp_uname = getenv("USER")) == NULL && (cp_uname = getenv("LOGNAME")) == NULL && (cp_uname = getenv("USERNAME")) == NULL))
					{
						cp_uname = (char *)"";
					}
					for (ii = 0; outoff < bsz && cp_uname[ii] != '\0'; ++ii)
					{
						obuf[outoff++] = cp_uname[ii];
					}
					break;
				case 'U':
					if (cp_uid == NULL)
					{
						sprintf(loguid, "%u", getuid());
						cp_uid = loguid;
					}
					for (ii = 0; outoff < bsz && cp_uid[ii] != '\0'; ++ii)
					{
						obuf[outoff++] = cp_uid[ii];
					}
					break;
				case '\0':
					obuf[outoff++] = '%';
					--inp; /* let for loop exit test see this */
					break;
				default:
					/* just put them both out if there is space in obuf */
					obuf[outoff++] = '%';
					if (outoff < bsz)
					{
						obuf[outoff++] = *inp;
					}
			}
		}
		else
		{
			obuf[outoff++] = *inp;
		}
	}
	if (outoff >= bsz)
	{
		obuf[bsz - 1] = '\0';
	}
	else
	{
		obuf[outoff] = '\0';
	}
	return (obuf);
} /* tsnprintf */

/* RETURN "created" status */
static int trace_mmap_file(const char *_file, int *memlen /* in/out -- in for when file created, out when not */
						   ,
						   struct traceControl_s **tC_p, struct traceControl_rw **tC_rwp /* out */
						   ,
						   uint32_t msgmax, uint32_t argsmax, uint32_t numents, uint32_t namtblents /* all in for when file created */
						   ,
						   int allow_ro)
{
	int fd;
	struct traceControl_s *controlFirstPage_p = NULL;
	struct traceControl_rw *rw_rwp;
	off_t off;
	char path[PATH_MAX];
	int created = 0;
	int stat_try = 0;
	int quiet_warn = 0;
	int prot_flags = PROT_READ | PROT_WRITE;

	(void)tsnprintf(path, PATH_MAX, _file);                        /* resolves any %u, etc, in _file */
	if ((fd = open(path, O_RDWR | O_CREAT | O_EXCL, 0666)) != -1)  //NOLINT
	{                                                              /* successfully created new file - must init */
		uint8_t one_byte = '\0';
		off = lseek(fd, (*memlen) - 1, SEEK_SET);
		if (off == (off_t)-1)
		{
			perror("lseek");
			*tC_p = &(traceControl[0]);
			*tC_rwp = &(traceControl[0].rw);
			unlink(path);
			return (0);
		}
		quiet_warn += write(fd, &one_byte, 1);
		if (quiet_warn < 0)
		{
			perror("writeOneByte");
		}
		created = 1;
	}
	else
	{ /* There's an existing file... map 1st page ro */
		struct stat statbuf;
		int try_ = 3000;
		/* must verify that it already exists */
		fd = open(path, O_RDWR);  //NOLINT
		if (fd == -1)
		{
			if (allow_ro)
			{
				/* try read-only for traceShow */
				fd = open(path, O_RDONLY);  //NOLINT
			}
			if (fd == -1)
			{
				fprintf(stderr, "TRACE: open(%s)=%d errno=%d pid=%d\n", path, fd, errno, tracePid);
				*tC_p = &(traceControl[0]);
				*tC_rwp = &(traceControl[0].rw);
				return (0);
			}
			prot_flags = PROT_READ;
		}
		/*printf( "trace_mmap_file - fd=%d\n",fd );*/ /*interesting in multithreaded env.*/
		if (fstat(fd, &statbuf) == -1)
		{
			perror("fstat");
			close(fd);
			*tC_p = &(traceControl[0]);
			*tC_rwp = &(traceControl[0].rw);
			return (0);
		}
		while (statbuf.st_size < (off_t)sizeof(struct traceControl_s))
		{
			fprintf(stderr, "stat again\n");
			if (((stat_try++ >= 30) && (fprintf(stderr, "too many stat tries\n"), 1)) || ((fstat(fd, &statbuf) == -1) && (perror("fstat"), 1)))
			{
				close(fd);
				*tC_p = &(traceControl[0]);
				*tC_rwp = &(traceControl[0].rw);
				return (0);
			}
		}

		controlFirstPage_p = (struct traceControl_s *)mmap(NULL, TRACE_PAGESIZE, PROT_READ, MAP_SHARED, fd, 0);
		if (controlFirstPage_p == (struct traceControl_s *)-1)
		{
			perror("mmap(NULL,TRACE_PAGESIZE,PROT_READ,MAP_SHARED,fd,0) error");
			*tC_p = &(traceControl[0]);
			*tC_rwp = &(traceControl[0].rw);
			return (0);
		}
		while (try_--)
		{
			if (controlFirstPage_p->trace_initialized != 1)
			{
				if (try_ == 0)
				{
					printf("Trace file not initialzed; consider (re)moving it.\n");
					close(fd);
					*tC_p = &(traceControl[0]);
					*tC_rwp = &(traceControl[0].rw);
					return (0);
				}
				if (try_ == 1)
				{
					sleep(1); /* just sleep a crazy amount the last time */
				}
			}
			else
			{
				break;
			}
		}
		/*sleep(1);*/
		*memlen = controlFirstPage_p->memlen;
	}

	/* ??? OLD:I MUST allocate/grab a contiguous vm address space! [in testing threads (where address space
	   is shared (obviously)), thread creation allocates vm space which can occur between
	   these two calls]
	   ???
	   NEW: mmap from rw portion on */
	rw_rwp = (struct traceControl_rw *)mmap(NULL, (*memlen) - TRACE_PAGESIZE, prot_flags, MAP_SHARED, fd, TRACE_PAGESIZE);
	if (rw_rwp == (void *)-1)
	{
		perror("Error:mmap(NULL,(*memlen)-TRACE_PAGESIZE,PROT_READ[|PROT_WRITE],MAP_SHARED,fd,TRACE_PAGESIZE)");
		printf("(*memlen)=%d errno=%d\n", (*memlen) - TRACE_PAGESIZE, errno);
		close(fd);
		*tC_p = &(traceControl[0]);
		*tC_rwp = &(traceControl[0].rw);
		return (0);
	}

	if (created)
	{
		/* need controlFirstPage_p RW temporarily */
		controlFirstPage_p = (struct traceControl_s *)mmap(NULL, TRACE_PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (controlFirstPage_p == (struct traceControl_s *)-1)
		{
			perror("mmap(NULL,sizeof(struct traceControl_s),PROT_READ,MAP_SHARED,fd,0) error");
			munmap(rw_rwp, (*memlen) - TRACE_PAGESIZE);
			close(fd);
			*tC_p = &(traceControl[0]);
			*tC_rwp = &(traceControl[0].rw);
			return (0);
		}
		trace_created_init(controlFirstPage_p, rw_rwp, msgmax, argsmax, numents, namtblents, *memlen, 1);
		/* Now make first page RO */
		munmap(controlFirstPage_p, TRACE_PAGESIZE);
#	define MM_FLAGS MAP_SHARED /*|MAP_FIXED*/
		controlFirstPage_p = (struct traceControl_s *)mmap(NULL, TRACE_PAGESIZE, PROT_READ, MM_FLAGS, fd, 0);
		if (controlFirstPage_p == (struct traceControl_s *)-1)
		{
			perror("Error:mmap(NULL,TRACE_PAGESIZE,PROT_READ," MM_STR(MM_FLAGS) ",fd,0)");
			printf("(*memlen)=%d errno=%d\n", (*memlen), errno);
			munmap(rw_rwp, (*memlen) - TRACE_PAGESIZE);
			close(fd);
			*tC_p = &(traceControl[0]);
			*tC_rwp = &(traceControl[0].rw);
			return (0);
		}
	}

	traceNamLvls_p = (struct traceNamLvls_s *)(rw_rwp + 1);
	traceEntries_p = (struct traceEntryHdr_s *)((unsigned long)traceNamLvls_p + namtblSiz(controlFirstPage_p->num_namLvlTblEnts));

	*tC_rwp = rw_rwp;
	*tC_p = controlFirstPage_p;

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
	uint32_t msgmax_, argsmax_, numents_, namtblents_;
#	ifndef __KERNEL__
	int activate = 0;
	const char *_file;
	const char *cp;

	if (!trace_lock(&traceInitLck))
	{
		TRACE_PRN("trace_lock: InitLck hung?\n");
	}
#		ifdef TRACE_DEBUG_INIT
	printf("traceInit(debug:A): tC_p=%p static=%p _name=%p Tid=%d TrcId=%d\n", traceControl_p, traceControl_p_static, _name, traceTid, traceTID);
#		endif
	if (traceControl_p == NULL)
	{
		/* This stuff should happen once (per TRACE_DEFINE compilation module) */
		TRACE_REGISTER_ATFORK;

		/* test for activation. (See below for _name override/default) */
		if (_name != NULL)
		{
			/* name is specified in module, which "wins" over env, but does not "activate" */
			const char *scratch_name;
			if ((scratch_name = getenv("TRACE_NAME")) && (*scratch_name != '\0'))
			{
				activate = 1;
			}
		}
		else if ((_name = getenv("TRACE_NAME")) && (*_name != '\0'))
		{
			/* name not specified in module, nor is a non-"" specified in env */
			activate = 1;
		}

		if (!((_file = getenv("TRACE_FILE")) && (*_file != '\0') && (activate = 1)))
		{
			_file = traceFile;
		}
		if ((cp = getenv("TRACE_ARGSMAX")) && (*cp) && (activate = 1)) { argsmax_ = strtoul(cp, NULL, 0); }
		else
		{
			argsmax_ = TRACE_DFLT_MAX_PARAMS;
		}
		/* use _MSGMAX= so exe won't override and _MSGMAX won't activate; use _MSGMAX=0 to activate with default MAX_MSG */
		((cp = getenv("TRACE_MSGMAX")) && (*cp) && (activate = 1) && (msgmax_ = strtoul(cp, NULL, 0))) || (msgmax_ = TRACE_DFLT_MAX_MSG_SZ);
		((cp = getenv("TRACE_NUMENTS")) && (numents_ = strtoul(cp, NULL, 0)) && (activate = 1)) || (numents_ = TRACE_DFLT_NUM_ENTRIES);
		((cp = getenv("TRACE_NAMTBLENTS")) && (namtblents_ = strtoul(cp, NULL, 0)) && (activate = 1)) || (namtblents_ = TRACE_DFLT_NAMTBL_ENTS);
		((cp = getenv("TRACE_LVLM")) && (trace_lvlM = strtoull(cp, NULL, 0)) && (activate = 1)); /* activate if non-zero */

		/* TRACE_LVLS and TRACE_PRINT_FD can be used when active or inactive */
		if ((cp = getenv("TRACE_PRINT_FD")) && (*cp))
		{
			int sts = sscanf(cp,"%d,%d",&tracePrintFd[0],&tracePrintFd[1]);
			if (sts == 1)
				tracePrintFd[1] = tracePrintFd[0];
		}

		if (!activate)
		{
			traceControl_rwp = &(traceControl[0].rw);
			traceControl_p = &(traceControl[0]);
		}
		else
		{
			if (namtblents_ == 1)
			{
				namtblents_ = 2; /* If it has been specified in the env. it should be at least 2 */
			}
			memlen = traceMemLen(cntlPagesSiz(), namtblents_, msgmax_, argsmax_, numents_);
			if ((traceControl_p_static != NULL) && (strcmp(traceFile_static, _file) == 0))
			{
				traceControl_p = traceControl_p_static;
			}
			else
			{
				trace_mmap_file(_file, &memlen, &traceControl_p, &traceControl_rwp, msgmax_, argsmax_, numents_, namtblents_, allow_ro);
			}
		}

		/* trace_mmap_file may have failed */
		if (traceControl_p == &(traceControl[0]))
		{
#		define DISABLED_ENTS 1
			trace_created_init(traceControl_p, traceControl_rwp, msgmax_, argsmax_, DISABLED_ENTS /*numents_*/
							   ,
							   ((sizeof(traceControl) - sizeof(traceControl[0]) - DISABLED_ENTS * entSiz(msgmax_, argsmax_)) / sizeof(struct traceNamLvls_s)) /*namtblents_*/
							   ,
							   sizeof(traceControl) /*memlen*/
							   ,
							   0 /*modeM*/);
		}
		else
		{
			if (traceControl_p_static == NULL)
			{
				strcpy(traceFile_static, _file);  //NOLINT
				traceControl_p_static = traceControl_p;
			}
		}

		trace_namLvlSet(); /* more env vars checked */
	}

	if (_name == NULL)
	{
		if (!((_name = getenv("TRACE_NAME")) && (*_name != '\0')))
		{
			_name = traceName;
		}
	}

	traceTID = name2TID(_name);
	/* Now that the critical variables
	   (traceControl_p, traceNamLvls_p, traceEntries_p) and even traceTID are
	   set, it's OK to indicate that the initialization is complete */
	if (traceTid == 0) /* traceInit may be called w/ or w/o checking traceTid */
	{
		tracePid = getpid(); /* do/re-do -- it may be forked process */
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#			pragma GCC diagnostic push
#			pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#		endif
		traceTid = trace_gettid();
#		if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#			pragma GCC diagnostic pop
#		endif
	}
#		ifdef TRACE_DEBUG_INIT
	printf("traceInit(debug:Z): tC_p=%p static=%p _name=%p Tid=%d TrcId=%d\n", traceControl_p, traceControl_p_static, _name, traceTid, traceTID);
#		endif
	trace_unlock(&traceInitLck);

	if ((cp = getenv("TRACE_LVLS")) && (*cp))
	{
		trace_lvlS = strtoull(cp, NULL, 0); /* set for future new traceTIDs (from this process) regardless of cmd line tonSg or toffSg - if non-zero! */
		TRACE_CNTL("lvlmskSg", trace_lvlS); /* set for all current and future (until cmd line tonSg or toffSg) */
	}
	if (trace_lvlM)
	{
		TRACE_CNTL("lvlmskMg", trace_lvlM); /* all current and future (until cmdline tonMg/toffMg) (and new from this process regardless of cmd line tonSg or toffSg) */
	}

#	else /* ifndef __KERNEL__ */

	msgmax_ = msgmax;         /* module_param */
	argsmax_ = argsmax;       /* module_param */
	numents_ = numents;       /* module_param */
	namtblents_ = namtblents; /* module_param */
	printk("numents_=%d msgmax_=%d argsmax_=%d namtblents_=%d\n", numents_, msgmax_, argsmax_, namtblents_);
	memlen = traceMemLen(cntlPagesSiz(), namtblents_, msgmax_, argsmax_, numents_);
	traceControl_p = (struct traceControl_s *)vmalloc_node(memlen, trace_buffer_numa_node);
	traceControl_rwp = (struct traceControl_rw *)((unsigned long)traceControl_p + TRACE_PAGESIZE);
	trace_created_init(traceControl_p, traceControl_rwp, msgmax_, argsmax_, numents_, namtblents_, memlen, 1);
	if (_name == NULL) _name = traceName;
	traceTID = name2TID(_name);
#	endif

	return (0);
} /* traceInit */

static void traceInitNames(struct traceControl_s *tC_p, struct traceControl_rw *tC_rwp)
{
	unsigned ii;
	traceNamLvls_p = (struct traceNamLvls_s *)(tC_rwp + 1);
	for (ii = 0; ii < tC_p->num_namLvlTblEnts; ++ii)
	{
		traceNamLvls_p[ii].name[0] = '\0';
		traceNamLvls_p[ii].M = 0xf; /* As Name/TIDs can't go away, these are */
		traceNamLvls_p[ii].S = 0x7; /* then defaults except for trace_lvlS/trace_lvlM */
		traceNamLvls_p[ii].T = 0;   /* in name2TID. */
	}                               /* (0 for err, 1=warn, 2=info, 3=debug) */
#	ifdef __KERNEL__
	strcpy(traceNamLvls_p[0].name, "KERNEL");
	/* like userspace TRACE_LVLS env.var - See also name2TID */
	if (trace_lvlS)
		traceNamLvls_p[0].S = trace_lvlS;
	if (trace_lvlM)
		traceNamLvls_p[0].M = trace_lvlM;
#	endif
	strcpy(traceNamLvls_p[tC_p->num_namLvlTblEnts - 2].name, "TRACE");    //NOLINT
	strcpy(traceNamLvls_p[tC_p->num_namLvlTblEnts - 1].name, "_TRACE_");  //NOLINT
	tC_rwp->longest_name = 7;
} /* traceInitNames */

#endif /* !defined(__KERNEL__) || defined(TRACE_IMPL) */

static struct traceEntryHdr_s *idxCnt2entPtr(uint32_t idxCnt)
{
	uint32_t idx;
	off_t off;
	uint32_t num_entries = traceControl_p->num_entries;
	idx = idxCnt % num_entries;
	off = idx * traceControl_p->siz_entry;
	return (struct traceEntryHdr_s *)((char *)traceEntries_p + off);
} /* idxCnt2entPtr */

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
  -1 = don't format in streamer, even if slow enabled -- pass args to mem and slow output function  */
struct tstreamer_flags { unsigned do_m:1; unsigned do_s:1; int fmtnow:2; tstreamer_flags():do_m(0),do_s(0),fmtnow(0){}};

#	if (__cplusplus >= 201103L)

#		define TRACE_STATIC_TID_ENABLED(name, lvl, s_enbld, s_frc, flgsp, tvp, ins, ins_sz)                                         \
			[&](const char *nn, int lvl_, int s_enabled_, int s_frc_, tstreamer_flags *flgsp_, timeval *tvp_, char *ins_, size_t sz) { \
				TRACE_INIT_CHECK(TRACE_NAME)                                                                                          \
				{                                                                                                                     \
					static TRACE_THREAD_LOCAL int tid_ = -1;                                                                          \
					if (tid_ == -1) { tid_ = nn[0] ? name2TID(nn) : traceTID /*traceTID from TRACE_INIT_CHECK*/; }                    \
					flgsp_->do_m = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl_));                               \
					flgsp_->do_s = (s_enabled_ && traceControl_rwp->mode.bits.S && (s_frc_ || (traceNamLvls_p[tid_].S & TLVLMSK(lvl_))));   \
					if (flgsp_->do_s)                                                                                                       \
					{ /* try to avoid TLS lookup of _info - compiler probably does this anyway */                                     \
						static TRACE_THREAD_LOCAL limit_info_t _info;                                                                 \
						flgsp_->do_s = limit_do_print(tvp_, &_info, ins_, sz);                                                              \
					}                                                                                                                 \
					return (flgsp_->do_m || flgsp_->do_s) ? tid_ : -1;                                                                            \
				}                                                                                                                     \
				else return -1;                                                                                                       \
			}(name, lvl, s_enbld, s_frc, flgsp, tvp, ins, ins_sz)

#	else /* (__cplusplus >= 201103L) */

// Note: the s_enbld, s_frc and tvp args are used directly in the macro definition
#		define TRACE_STATIC_TID_ENABLED(name, lvl, s_enbld, s_frc, flgsp, tvp, ins, ins_sz)                                                                            \
			({                                                                                                                                                           \
				const char *nn = name;                                                                                                                                   \
				int lvl_ = lvl; \
				tstreamer_flags *flgsp_ = flgsp;						\
				char *ins_ = ins;                                                                                                                                        \
				size_t sz = ins_sz;                                                                                                                                      \
				int tid__;                                                                                                                                               \
				TRACE_INIT_CHECK(TRACE_NAME)                                                                                                                             \
				{                                                                                                                                                        \
					static TRACE_THREAD_LOCAL int tid_ = -1;                                                                                                             \
					if (tid_ == -1) { tid_ = nn[0] ? name2TID(nn) : traceTID /*traceTID from TRACE_INIT_CHECK*/; }                                                       \
					static TRACE_THREAD_LOCAL limit_info_t _info;                                                                                                        \
					flgsp_->do_m = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl_));                                                                  \
					flgsp_->do_s = (s_enbld && traceControl_rwp->mode.bits.S && (s_frc || (traceNamLvls_p[tid_].S & TLVLMSK(lvl_))) && limit_do_print(tvp, &_info, ins_, sz)); \
					tid__ = (flgsp_->do_m || flgsp_->do_s) ? tid_ : -1;                                                                                                              \
				}                                                                                                                                                        \
				else tid__ = -1;                                                                                                                                         \
				tid__;                                                                                                                                                   \
			})

#	endif /* else (__cplusplus >= 201103L) */

// Use C++ "for" statement to create single statement scope for key (static) variable that
// are initialized and then, if enabled, passed to the Streamer class temporary instances.
// arg1   - lvl;
// arg2,3 - nam_or_fmt = name or fmtnow (note: fmtnow can be used to force formatting env if Memory only);
// arg4   - s_enabled = is_slowpath_enabled (b/c one version of message facility had a global "is_enabled");
// arg5   - force_s = force_slow - override tlvlmskS&lvl==0
#	define TRACE_USE_STATIC_STREAMER 1
#	if TRACE_USE_STATIC_STREAMER == 1
#		define TRACE_STREAMER(lvl, nam_or_fmt, fmt_or_nam, s_enabled, force_s)                                                                                      \
			for (struct _T_ {int lvl__, tid; tstreamer_flags flgs; char ins[32]; struct timeval tv; \
			                     _T_(int llv):lvl__(llv),tid(-1){tv.tv_sec=0;} } _tlog_(lvl);	\
				 (_tlog_.tid == -1) && ((_tlog_.tid = TRACE_STATIC_TID_ENABLED(t_arg_nmft(nam_or_fmt, fmt_or_nam, &_tlog_.flgs), _tlog_.lvl__, s_enabled, force_s, \
																			   &_tlog_.flgs, &_tlog_.tv, _tlog_.ins, sizeof(_tlog_.ins))) != -1);    \
				 __streamer.str())                                                                                                                                   \
			__streamer.init(_tlog_.tid, _tlog_.lvl__, _tlog_.flgs, __FILE__, __LINE__, &_tlog_.tv, _tlog_.ins, &TRACE_LOG_FUNCTION)
#	else
#		define TRACE_STREAMER(lvl, nam_or_fmt, fmt_or_nam, s_enabled, force_s)                                                                                      \
			for (struct _T_ {int lvl__, tid; tstreamer_flags flgs; char ins[32]; struct timeval tv; \
			                     _T_(int llv):lvl__(llv),tid(-1){tv.tv_sec=0;} } _tlog_(lvl);	\
				 (_tlog_.tid == -1) && ((_tlog_.tid = TRACE_STATIC_TID_ENABLED(t_arg_nmft(nam_or_fmt, fmt_or_nam, &_tlog_.flgs), _tlog_.lvl__, s_enabled, force_s, \
																			   &_tlog_.flgs, &_tlog_.tv, _tlog_.ins, sizeof(_tlog_.ins))) != -1);)   \
			TraceStreamer().init(_tlog_.tid, _tlog_.lvl__, _tlog_.flgs, __FILE__, __LINE__, &_tlog_.tv, _tlog_.ins, &TRACE_LOG_FUNCTION)
#	endif

#	define TRACE_ENDL ""
#	define TLOG_ENDL TRACE_ENDL

// This will help devleper who use too many TLOG_INFO/TLOG_DEBUG
// to use more TLOG{,_TRACE,_DBG,_ARB} (use more levels)
#	ifdef __OPTIMIZE__
#		define DEBUG_FORCED 0
#	else
#		define DEBUG_FORCED 1
#	endif

static inline const char *t_arg_nmft(const char *nm, int fmtnow, tstreamer_flags *fmtret)
{
	fmtret->fmtnow = fmtnow;
	return nm;
}
static inline const char *t_arg_nmft(const std::string &nm, int fmtnow, tstreamer_flags *fmtret)
{
	fmtret->fmtnow = fmtnow;
	return nm.c_str();
}
static inline const char *t_arg_nmft(int fmtnow, const char *nm, tstreamer_flags *fmtret)
{
	fmtret->fmtnow = fmtnow;
	return nm ? nm : "";
}  // could be addr 0 (null)
static inline const char *t_arg_nmft(int fmtnow, const std::string &nm, tstreamer_flags *fmtret)
{
	fmtret->fmtnow = fmtnow;
	return nm.c_str();
}
static inline const char *t_arg_nmft(int fmtnow, int nm __attribute__((__unused__)), tstreamer_flags *fmtret)
{
	fmtret->fmtnow = fmtnow;
	return "";
}

#	define tlog_LVL(a1, ...) a1
#	define tlog_ARG2(a1, a2, ...) a2
#	define tlog_ARG3(a1, a2, a3, ...) a3
#	define TLVL_ERROR 0
#	define TLVL_WARNING 1
#	define TLVL_INFO 2
#	define TLVL_DEBUG 3
#	define TLVL_TRACE 4
//                                 args are: lvl,         name, fmtnow, s_enabled, s_force,
#	define TLOG_ERROR(name) TRACE_STREAMER(TLVL_ERROR, &(name)[0], 0, 1, 0)
#	define TLOG_WARNING(name) TRACE_STREAMER(TLVL_WARNING, &(name)[0], 0, 1, 0)
#	define TLOG_INFO(name) TRACE_STREAMER(TLVL_INFO, &(name)[0], 0, 1, 0)
#	define TLOG_DEBUG(name) TRACE_STREAMER(TLVL_DEBUG, &(name)[0], 0, 1, 0)
#	define TLOG_TRACE(name) TRACE_STREAMER(TLVL_TRACE, &(name)[0], 0, 1, 0)
// For the following, the 1st arg must be lvl.
#	define TLOG_DBG(...) TRACE_STREAMER(tlog_LVL(__VA_ARGS__, need_at_least_one), tlog_ARG2(__VA_ARGS__, 0, need_at_least_one), tlog_ARG3(__VA_ARGS__, 0, "", need_at_least_one), 1, 0)
#	define TLOG_ARB(...) TRACE_STREAMER(tlog_LVL(__VA_ARGS__, need_at_least_one), tlog_ARG2(__VA_ARGS__, 0, need_at_least_one), tlog_ARG3(__VA_ARGS__, 0, "", need_at_least_one), 1, 0)
#	define TLOG(...) TRACE_STREAMER(tlog_LVL(__VA_ARGS__, need_at_least_one), tlog_ARG2(__VA_ARGS__, 0, need_at_least_one), tlog_ARG3(__VA_ARGS__, 0, "", need_at_least_one), 1, 0)
#	define TRACE_STREAMER_ARGSMAX 35
#	define TRACE_STREAMER_TEMPLATE 1
#	define TRACE_STREAMER_EXPAND(args) args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22], args[23], args[24], args[25], args[26], args[27], args[28], args[29], args[30], args[31], args[32], args[33], args[34]

#	define TraceMin(a, b) (((a) < (b)) ? (a) : (b))

#	ifdef TRACE_STREAMER_DEBUG
#		define T_STREAM_DBG std::cout
#	else
#		define T_STREAM_DBG \
			if (0) std::cout
#	endif

//typedef unsigned long long trace_ptr_t;
//typedef void* trace_ptr_t;
typedef void *trace_ptr_t;

namespace {  // unnamed namespace (i.e. static (for each compliation unit only))

struct TraceStreamer : std::ios
{
	typedef unsigned long long arg;  // room for 64 bit args (i.e double on 32 or 64bit machines)
	char msg[TRACE_STREAMER_MSGMAX];
	size_t msg_sz;
	arg args[TRACE_STREAMER_ARGSMAX];
	size_t argCount;
	void *param_va_ptr;
	int tid_;
	int lvl_;
	bool do_s, do_m, do_f;
	char widthStr[16];
	char precisionStr[16];
	char fmtbuf[32];  // buffer for fmt (e.g. "%022.12llx")
	struct timeval *lclTime_p;
	const char *ins_;
	const char *file_;
	int line_;
	trace_log_function_type user_fun_ptr_;

		public : explicit TraceStreamer()
		: msg_sz(0), argCount(0), param_va_ptr(args)
	{
		T_STREAM_DBG << "TraceStreamer CONSTRUCTOR\n";
		std::ios::init(0);
	}

	inline ~TraceStreamer()
	{
#	if TRACE_USE_STATIC_STREAMER != 1
		str();
#	endif
	}

	// use this method to return a reference (to the temporary, in its intended use)
	inline TraceStreamer &init(int tid, int lvl, tstreamer_flags flgs, const char *file, int line, timeval *tvp, const char *ins, trace_log_function_type user_fun_ptr)
	{
		widthStr[0] = precisionStr[0] = msg[0] = '\0';
		msg_sz = 0;
		argCount = 0;
		param_va_ptr = args;
		tid_ = tid;
		lvl_ = lvl;
		do_m = flgs.do_m; // m=memory, aka "fast", but not to be confused with "format"
		do_s = flgs.do_s;
		do_f = (flgs.fmtnow == -1) ? 0 : (flgs.do_s || flgs.fmtnow); // here "f" is "format", not "fast"
		ins_ = ins;
		file_ = file;
		line_ = line;
		lclTime_p = tvp;
		user_fun_ptr_ = user_fun_ptr;
#	if TRACE_USE_STATIC_STREAMER == 1
		std::dec(*this);
		std::noshowbase(*this);
#	endif
		return *this;
	}

#	ifdef __clang__
#		define _M_flags flags()
#	endif

	inline void str()
	{
		T_STREAM_DBG << "Message is " << msg << std::endl;
		while (msg_sz && msg[msg_sz - 1] == '\n')
		{
			msg[msg_sz - 1] = '\0';
			--msg_sz;
		}
#	if (defined(__cplusplus) && (__cplusplus >= 201103L))
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wformat-security"
#	endif
		if (do_f)
		{
			if (do_m) trace(lclTime_p, tid_, lvl_, line_, 0 TRACE_XTRA_PASSED, msg);
			if (do_s) { (*user_fun_ptr_)(lclTime_p, tid_, lvl_, ins_, file_, line_, 0, msg); } /* can be null */
		}
		else
		{
			if (do_m)
#	if (defined(__cplusplus) && (__cplusplus >= 201103L))
			{
				va_list ap = TRACE_VA_LIST_INIT((void *)args);  // warning: extended initializer lists only available with [since] -std=c++11 ...
				vtrace(lclTime_p, tid_, lvl_, line_, argCount, msg, ap);
			}
#	else
				trace(lclTime_p, tid_, lvl_, line_, argCount TRACE_XTRA_PASSED, msg, TRACE_STREAMER_EXPAND(args));
#	endif
			if (do_s)
			{
				(*user_fun_ptr_)(lclTime_p, tid_, lvl_, ins_, file_, line_, argCount, msg, TRACE_STREAMER_EXPAND(args));
			} /* can be null */
		}
#	if (defined(__cplusplus) && (__cplusplus >= 201103L))
#		pragma GCC diagnostic pop
#	endif

		// Silence Clang static analyzer "dangling references"
		ins_ = 0;
		lclTime_p = 0;
	}

	inline void msg_append(const char *src, size_t len = 0)
	{
		if (!len)
			len = strlen(src);
		size_t add = TraceMin(len, sizeof(msg) - 1 - msg_sz);
		//strncpy(&msg[msg_sz], src, add);
		memcpy(&msg[msg_sz], src, add);
		msg_sz += add;
		msg[msg_sz] = '\0';
	}

	// Return a format string (e.g "%d") - assume class fmtbuf char[] is big enough.
	inline char *format(bool isFloat, bool isUnsigned, const char *length, std::ios::fmtflags flags, size_t *len = NULL)
	{  //See, for example: http://www.cplusplus.com/reference/cstdio/printf/
		size_t oo = 0;
		fmtbuf[oo++] = '%';

		// Flags
		if (flags & left) fmtbuf[oo++] = '-';
		if (flags & showpos) fmtbuf[oo++] = '+';
		if (flags & (showpoint | showbase)) fmtbuf[oo++] = '#';  // INCLUSIVE OR

#	define APPEND(ss)                   \
		do                               \
		{                                \
			if (ss && ss[0])             \
			{                            \
				strcpy(&fmtbuf[oo], ss); \
				oo += strlen(ss);        \
			}                            \
		} while (0)
		// Width
		APPEND(widthStr);

		if (isFloat)
		{
			// Precision
			APPEND(precisionStr);
			APPEND(length);

			if ((flags & (fixed | scientific)) == (fixed | scientific)) /*AND*/ { fmtbuf[oo++] = flags & uppercase ? 'A' : 'a'; }
			else if (flags & fixed)
			{
				fmtbuf[oo++] = flags & uppercase ? 'F' : 'f';
			}
			else if (flags & scientific)
			{
				fmtbuf[oo++] = flags & uppercase ? 'E' : 'e';
			}
			else
			{
				fmtbuf[oo++] = flags & uppercase ? 'G' : 'g';
			}
		}
		else
		{
			APPEND(length);
			// this is more of the expected behavior - not necessarily what the standard describes
			if (flags & hex) { fmtbuf[oo++] = flags & uppercase ? 'X' : 'x'; }
			else if (flags & oct)
			{
				fmtbuf[oo++] = 'o';
			}
			else if (isUnsigned)
			{
				fmtbuf[oo++] = 'u';
			}
			else
			{
				fmtbuf[oo++] = 'd';
			}
		}
		fmtbuf[oo] = '\0';
		if (len) *len = oo;
		return fmtbuf;
	}

	inline TraceStreamer &width(int y)
	{
		if (y != std::ios_base::width())
		{
			std::ios_base::width(y);
		}
		snprintf(widthStr, sizeof(widthStr), "%d", y);
		T_STREAM_DBG << "TraceStreamer widthStr is now " << widthStr << std::endl;
		return *this;
	}

	inline TraceStreamer &precision(int y)
	{
		if (y != std::ios_base::precision())
		{
			std::ios_base::precision(y);
		}
		if (y)
			snprintf(precisionStr, sizeof(precisionStr), ".%d", y);
		else
			precisionStr[0] = '\0';
		T_STREAM_DBG << "TraceStreamer precisionStr is now " << precisionStr << std::endl;
		return *this;
	}
#	if !defined(__clang__) || (defined(__clang__) && __clang_major__ == 3 && __clang_minor__ == 4)
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
	//setprecision
	inline TraceStreamer &operator<<(std::__1::__iom_t5 r)
	{
		std::ostringstream ss;
		ss << r;
		precision(ss.precision());
		return *this;
	}
	//setwidth
	inline TraceStreamer &operator<<(std::__1::__iom_t6 r)
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

	template<typename T>
	inline TraceStreamer &operator<<(const T *const &r)
	{
		T ** const vp = (T ** const)param_va_ptr;
		if (do_f || (vp+1) > (T ** const)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, "%p", static_cast<const void *>(r));
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf T1 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			msg_append("%p", 2);
			++argCount;
			*vp = (T * const)r;
			param_va_ptr = vp+1;
			T_STREAM_DBG << "streamer check T1 (const T*const &r) msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}

	template<typename T>
	inline TraceStreamer &operator<<(T *const &r)  // Tricky C++...to pass pointer by reference, have to have the const AFTER the type
	{
		T ** const vp = (T ** const)param_va_ptr;
		if (do_f || (vp+1) > (T ** const)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, "%p", static_cast<void *>(r));
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf T2 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			msg_append("%p", 2);
			++argCount;
			*vp = (T * const)r;
			param_va_ptr = vp+1;
			T_STREAM_DBG << "streamer check T2 (T *const &r) msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}

	inline TraceStreamer &operator<<(const char &r)
	{
		long *vp = (long *)param_va_ptr;  // note: char gets pushed onto stack as sizeof(long)
		if (do_f || (vp+1) > (long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, false, NULL, _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 1 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 1 (const char &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const unsigned char &r)
	{
		unsigned long *vp = (unsigned long *)param_va_ptr;  // Note: char gets pushed as sizeof(long)
		if (do_f || (vp+1) > (unsigned long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, true, NULL, _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 2 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, true, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 2 (const unsigned char &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const int &r)
	{
		long *vp = (long *)param_va_ptr;  // int goes to long
		if (do_f || (vp+1) > (long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, false, NULL, _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 3 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 3 (const int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const short int &r)
	{
		long *vp = (long *)param_va_ptr;  // Note: shorts get pushed onto stack as sizeof(long)
		if (do_f || (vp+1) > (long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, false, "h", _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 4 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, false, "h", _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 4 (const short int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const long int &r)
	{
		long int *vp = (long int *)param_va_ptr;
		if (do_f || (vp+1) > (long int *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, false, "l", _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 5 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, false, "l", _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 5 (const long int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const short unsigned int &r)
	{
		unsigned long *vp = (unsigned long *)param_va_ptr;  // NOTE: shorts get pushed onto stack as sizeof(long)
		if (do_f || (vp+1) > (unsigned long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, true, "h", _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 6 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, true, "h", _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 6 (const short unsigned int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const unsigned int &r)
	{
		unsigned long *vp = (unsigned long *)param_va_ptr;  // int goes to long
		if (do_f || (vp+1) > (unsigned long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, true, NULL, _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 7 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, true, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 7 (const unsigned int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const long unsigned int &r)
	{
		long unsigned int *vp = (long unsigned int *)param_va_ptr;
		if (do_f || (vp+1) > (long unsigned int *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, true, "l", _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 8 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, true, "l", _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 8 (const long unsiged int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const long long unsigned int &r)
	{
		long long unsigned int *vp = (long long unsigned int *)param_va_ptr;
		if (do_f || (vp+1) > (long long unsigned int *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, true, "ll", _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 9 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, true, "ll", _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 9 (const long long unsiged int &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const double &r)
	{
		unsigned long nvp = (unsigned long)param_va_ptr;
		double *vp = (double*)nvp;
		if (do_f || (vp+1) > (double *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(true, false, NULL, _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 10 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(true, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 10 (const double &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const long double &r)
	{
		unsigned long nvp = (unsigned long)param_va_ptr;
		long double *vp;
		if (sizeof(long double)==16 && (nvp&0xf))
			vp = (long double *)((nvp+15)&~0xf); // alignment requirement
		else
			vp = (long double *)nvp;
		if (do_f || (vp+1) > (long double *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(true, false, "L", _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 10a rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(true, false, "L", _M_flags, &f_l), f_l);
			argCount += (sizeof(long double)+sizeof(long)/2) / sizeof(long);
			if (sizeof(long double)==16 && (nvp & 0xf))
			    ++argCount; // speudo extra arg satisfies alignment requirement
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 10a (const long double &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const float &r)
	{
		double *vp = (double *)param_va_ptr;  // note: floats get pushed onto stack as double
		if (do_f || (vp+1) > (double *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(true, false, NULL, _M_flags), r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 11 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(true, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 11 (const float &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(const bool &r)
	{
		long *vp = (long *)param_va_ptr;  // note: bool is pushed as long
		if (_M_flags & boolalpha)
			msg_append(r ? "true" : "false", r ? 4 : 5);
		else if (do_f || (vp+1) > (long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, "%d", r);
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 12 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			msg_append("%d", 2);
			++argCount;
			*vp++ = r;
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 12 (const bool &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
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
		long *vp = (long *)param_va_ptr;  // note: int goes to long
		if (do_f || (vp+1) > (long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, false, NULL, _M_flags), r.load());
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 13 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, false, NULL, _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r.load();
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 13 (const std::atomic<int> &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(std::atomic<unsigned long> const &r)
	{
		unsigned long *vp = (unsigned long *)param_va_ptr;
		if (do_f || (vp+1) > (unsigned long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, true, "l", _M_flags), r.load());
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 14 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			msg_append(format(false, true, "l", _M_flags));
			++argCount;
			*vp++ = r.load();
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 14 (std::atomic<unsigned long> const &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(std::atomic<short int> const &r)
	{
		long *vp = (long *)param_va_ptr;  // note: shorts get pushed as long
		if (do_f || (vp+1) > (long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, format(false, false, "h", _M_flags), r.load());
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 15 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			size_t f_l = 0;
			msg_append(format(false, false, "h", _M_flags, &f_l), f_l);
			++argCount;
			*vp++ = r.load();
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 15 (std::atomic<short int> const &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}
	inline TraceStreamer &operator<<(std::atomic<bool> const &r)
	{
		long *vp = (long *)param_va_ptr;  // note: bool goes to long
		if (_M_flags & boolalpha)
		{
			bool rr = r.load();
			msg_append(rr ? "true" : "false", rr ? 4 : 5);
		}
		else if (do_f || (vp+1) > (long *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, "%d", r.load());
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 16 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			msg_append("%d", 2);
			++argCount;
			*vp++ = r.load();
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 16 (std::atomic<bool> const &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";
		}
		return *this;
	}

	template<typename T>
	inline TraceStreamer &operator<<(std::unique_ptr<T> const &r)
	{
		trace_ptr_t *vp = (trace_ptr_t *)param_va_ptr;  // address is unsigned long
		if (do_f || (vp+1) > (trace_ptr_t *)&args[traceControl_p->num_params])
		{
			size_t ss = sizeof(msg) - 1 - msg_sz;
			int rr = snprintf(&msg[msg_sz], ss, "%p", static_cast<void *>(r.get()));
			msg_sz += SNPRINTED(rr, ss);
			T_STREAM_DBG << "streamer snprintf 17 rr=" << rr << " ss=" << ss << "\n";
		}
		else if (argCount < TRACE_STREAMER_ARGSMAX)
		{
			msg_append("%p", 2);
			++argCount;
			*vp++ = (trace_ptr_t)(void *)(r.get());
			param_va_ptr = vp;
			T_STREAM_DBG << "streamer check 17 (std::unique_ptr<T> const &r) " << r.get()
						 << " sizeof(r.get())=" << sizeof(r.get())
						 << " (unsigned long)r.get()=0x" << std::hex << (trace_ptr_t)(void *)(r.get())
						 << " sizeof(trace_ptr_t)=" << sizeof(trace_ptr_t)
						 << " msg_sz=" << std::dec << msg_sz << "\n";  // ALERT: without ".get()" - error: no match for 'operator<<' (operand types are 'std::basic_ostream<char>' and 'const std::unique_ptr<std::__cxx11::basic_string<char> >')
		}
		return *this;
	}
#	endif /* (__cplusplus >= 201103L) */

	// compiler asked for this -- can't think of why or when it will be used, but do the reasonable thing (append format and append args)
	inline TraceStreamer &operator<<(const TraceStreamer &r)
	{
		for (size_t ii = argCount; ii < (argCount + (r.argCount < TRACE_STREAMER_ARGSMAX
														 ? argCount + r.argCount
														 : TRACE_STREAMER_ARGSMAX));
			 ++ii)
		{
			args[ii] = r.args[ii - argCount];
		}
		argCount = argCount + r.argCount < TRACE_STREAMER_ARGSMAX ? argCount + r.argCount : TRACE_STREAMER_ARGSMAX;
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
inline TraceStreamer &TraceStreamer::operator<<(void *const &r)  // Tricky C++...to pass pointer by reference, have to have the const AFTER the type
{
	trace_ptr_t *vp = (trace_ptr_t *)param_va_ptr;  // note: addresses are unsigned long
	if (do_f || (vp+1) > (trace_ptr_t *)&args[traceControl_p->num_params])
	{
		size_t ss = sizeof(msg) - 1 - msg_sz;
		int rr = snprintf(&msg[msg_sz], ss, "%p", r);
		msg_sz += SNPRINTED(rr, ss);
		T_STREAM_DBG << "streamer snprintf 18 rr=" << rr << " ss=" << ss << "\n";
	}
	else if (argCount < TRACE_STREAMER_ARGSMAX)
	{
		msg_append("%p", 2);
		++argCount;
		*vp++ = (trace_ptr_t)r;
		param_va_ptr = vp;
		T_STREAM_DBG << "streamer check 18 (void *const &r) " << r << " msg_sz=" << std::dec << msg_sz << "\n";  // ALERT: without ".get()" - error: no match for 'operator<<' (operand types are 'std::basic_ostream<char>' and 'const std::unique_ptr<std::__cxx11::basic_string<char> >')
	}
	return *this;
}

#	if TRACE_USE_STATIC_STREAMER == 1
static TRACE_THREAD_LOCAL TraceStreamer __streamer;
#	endif
}  // unnamed namespace

#endif /* __cplusplus */

#endif /* TRACE_H */
