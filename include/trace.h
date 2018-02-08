/* This file (trace.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: trace.h,v $
 */
#ifndef TRACE_H_5216
#define TRACE_H_5216

#define TRACE_REV  "$Revision: 808 $$Date: 2018-02-07 23:44:55 -0600 (Wed, 07 Feb 2018) $"

#ifndef __KERNEL__

# include <stdio.h>		/* printf */
# include <stdarg.h>		/* va_list */
# include <stdint.h>		/* uint64_t */
# include <sys/time.h>          /* timeval */
# include <time.h>		/* struct tm, localtime_r, strftime */
# include <string.h>		/* strncmp */
# include <fcntl.h>		/* open, O_RDWR */
# include <sys/mman.h>		/* mmap */
# include <unistd.h>		/* lseek */
# include <sys/stat.h>		/* fstat */
# include <errno.h>		/* errno */
# include <limits.h>		/* PATH_MAX */
# include <stdlib.h>		/* getenv, setenv, strtoul */
# include <ctype.h>			/* isspace, isgraph */
# include <sys/uio.h>		/* struct iovec */
# include <fnmatch.h>		/* fnmatch */
# define TMATCHCMP(pattern,str_) (fnmatch(pattern,str_,0)==0) /*MAKE MACRO RETURN TRUE IF MATCH*/
/*# define TMATCHCMP(needle,haystack)   strstr(haystack,needle)*/ /*MAKE MACRO RETURN TRUE IF MATCH*/

# if   defined(__CYGWIN__)
#  include <windows.h>
static inline pid_t trace_gettid(void) { return GetCurrentThreadId(); }
static inline int   trace_getcpu(void) { return GetCurrentProcessorNumber(); }
# else
#  if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L)) || (defined(__cplusplus)&&(__cplusplus>=201103L))
#   pragma GCC diagnostic push
#   ifndef __cplusplus
#    pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#   endif
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#  endif
#  include <sys/syscall.h>	/* syscall */
#  if   defined(__sun__)
#   define TRACE_GETTID SYS_lwp_self
static inline int trace_getcpu(void) { return 0; }
#  elif defined(__APPLE__)
#   define TRACE_GETTID SYS_thread_selfid
static inline int trace_getcpu(void) { return 0; }
#  else /* assume __linux__ */
#   define TRACE_GETTID __NR_gettid
#   include <sched.h>		/* sched_getcpu - does vsyscall getcpu */
static inline int trace_getcpu(void) { return sched_getcpu(); }
#  endif
static inline pid_t trace_gettid(void) { return syscall(TRACE_GETTID); }
#  if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L)) || (defined(__cplusplus)&&(__cplusplus>=201103L))
#   pragma GCC diagnostic pop
#  endif
# endif

# ifndef PATH_MAX
#  define PATH_MAX 1024  /* conservative */
# endif
# ifdef __cplusplus
#  include <string>
#  include <sstream> /* std::ostringstream */
#  include <iostream>			// cerr
#  include <iomanip>
# endif

/* this first check is for Darwin 15 */
# if   defined(__cplusplus)      &&      (__cplusplus == 201103L) && defined(__clang_major__) && (__clang_major__ == 7)
#  include <atomic>		/* atomic<> */
#  define TRACE_ATOMIC_T              std::atomic<uint32_t>
#  define TRACE_ATOMIC_INIT           ATOMIC_VAR_INIT(0)
#  define TRACE_ATOMIC_LOAD(ptr)      atomic_load(ptr)
#  define TRACE_ATOMIC_STORE(ptr,val) atomic_store(ptr,val)
#  define TRACE_THREAD_LOCAL
# elif defined(__cplusplus)      &&      (__cplusplus >= 201103L)
#  include <atomic>		/* atomic<> */
#  define TRACE_ATOMIC_T              std::atomic<uint32_t>
#  define TRACE_ATOMIC_INIT           ATOMIC_VAR_INIT(0)
#  define TRACE_ATOMIC_LOAD(ptr)      atomic_load(ptr)
#  define TRACE_ATOMIC_STORE(ptr,val) atomic_store(ptr,val)
#  define TRACE_THREAD_LOCAL          thread_local 
# elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  include <stdatomic.h>	      /* atomic_compare_exchange_weak */
#  define TRACE_ATOMIC_T              /*volatile*/ _Atomic(uint32_t)
#  define TRACE_ATOMIC_INIT           ATOMIC_VAR_INIT(0)
#  define TRACE_ATOMIC_LOAD(ptr)      atomic_load(ptr)
#  define TRACE_ATOMIC_STORE(ptr,val) atomic_store(ptr,val)
#  define TRACE_THREAD_LOCAL          _Thread_local
# elif defined(__x86_64__) || defined(__i686__) || defined(__i386__)
#  define TRACE_ATOMIC_T              uint32_t
#  define TRACE_ATOMIC_INIT           0
#  define TRACE_ATOMIC_LOAD(ptr)      *(ptr)
#  define TRACE_ATOMIC_STORE(ptr,val) *(ptr) = val
#  define TRACE_THREAD_LOCAL
static inline uint32_t cmpxchg( uint32_t *ptr, uint32_t old, uint32_t new_)
    { uint32_t __ret;
    uint32_t __old = (old);
    uint32_t __new = (new_);
    volatile uint32_t *__ptr = (volatile uint32_t *)(ptr);
    __asm__ volatile("lock cmpxchgl %2,%1"
		 : "=a" (__ret), "+m" (*__ptr)
		 : "r" (__new), "0" (__old)
		 : "memory");
    return (__ret);
    }
# elif defined(__sparc__)
/* Sparc, as per wikipedia 2016.01.11, does not do CAS.
   I could move move the DECL stuff up can define another spinlock so that sparc could work in
   the define/declare environment. In this case, the module static TRACE_NAME feature could not
   be used. */
struct my_atomic { uint32_t lck; uint32_t val; };
#  define TRACE_ATOMIC_T              struct my_atomic
#  define TRACE_ATOMIC_INIT           {0}
#  define TRACE_ATOMIC_LOAD(ptr)      (ptr)->val
#  define TRACE_ATOMIC_STORE(ptr,vv)  (ptr)->val = vv
#  define TRACE_THREAD_LOCAL
static inline uint32_t xchg_u32(__volatile__ uint32_t *m, uint32_t val)
{
        __asm__ __volatile__("swap [%2], %0"
                             : "=&r" (val)
                             : "" (val), "r" (m)
                             : "memory");
        return val;
}
static inline uint32_t cmpxchg( TRACE_ATOMIC_T *ptr, uint32_t exp, uint32_t new_)
{  uint32_t old;
   while (xchg_u32(&ptr->lck,1)!=0) ;  /* lock */
   old=ptr->val;
   if (old==exp) ptr->val = new_;
   ptr->lck=0;                         /* unlock */
   return (old);
}
# else   /* userspace arch */
/* THIS IS A PROBLEM (older compiler on unknown arch) -- I SHOULD PROBABLY #error */
#  define TRACE_ATOMIC_T              uint32_t
#  define TRACE_ATOMIC_INIT           0
#  define TRACE_ATOMIC_LOAD(ptr)      *(ptr)
#  define TRACE_ATOMIC_STORE(ptr,val) *(ptr) = val
#  define TRACE_THREAD_LOCAL
#  define cmpxchg(ptr, old, new) \
	({ uint32_t old__ = *(ptr); if(old__==(old)) *ptr=new; old__;  })  /* THIS IS A PROBLEM -- NEED OS MUTEX HELP :( */
# endif /* userspace arch */

# define TRACE_GETTIMEOFDAY( tvp )    gettimeofday( tvp, NULL )
# define TRACE_PRINT                  printf
# define TRACE_VPRINT                 vprintf
# define TRACE_INIT_CHECK             if((traceTID!=-1)||(traceInit(TRACE_NAME)==0)) /* See note by traceTID decl/def below */

#else  /* __KERNEL__ */

# include <linux/time.h>	      /* do_gettimeofday */
/*# include <linux/printk.h>	         printk, vprintk */
# include <linux/kernel.h>	      /* printk, vprintk */
# include <linux/mm.h>		      /* kmalloc OR __get_free_pages */
# include <linux/vmalloc.h>	      /* __vmalloc, vfree */
# include <linux/spinlock.h>	      /* cmpxchg */
# include <linux/sched.h>	      /* current (struct task_struct *) */
# include <linux/ctype.h>	      /* isgraph */
/*# define TMATCHCMP(pattern,str_)         (strcmp(pattern,str_)==0)*/ /*MAKE MACRO RETURN TRUE IF MATCH*/
# define TMATCHCMP(needle,haystack)         strstr(haystack,needle) /*MAKE MACRO RETURN TRUE IF MATCH*/
# define TRACE_ATOMIC_T               uint32_t
# define TRACE_ATOMIC_INIT            0
# define TRACE_ATOMIC_LOAD(ptr)       *(ptr)
# define TRACE_ATOMIC_STORE(ptr,val)  *(ptr) = val
# define TRACE_THREAD_LOCAL 
# define TRACE_GETTIMEOFDAY( tvp )    do_gettimeofday( tvp )
# define TRACE_PRINT                  printk
# define TRACE_VPRINT                 vprintk
/*static int trace_no_init_cnt=0;*/
# define TRACE_INIT_CHECK             if((traceTID!=-1)||((traceTID=name2TID(TRACE_NAME))!=-1))
# ifndef MODULE
int trace_3_init(void);
int trace_sched_switch_hook_add( void );  /* for when compiled into kernel */
# endif

#endif /* __KERNEL__ */


#define TRACE_STREAMER_MSGMAX 0x1800
/* 88,7=192 bytes/ent   96,6=192   128,10=256  192,10=320 */
#define TRACE_DFLT_MAX_MSG_SZ      192
#define TRACE_DFLT_MAX_PARAMS       10
#define TRACE_DFLT_NAMTBL_ENTS     127  /* this is for creating new trace_buffer file --
										   it currently matches the "trace DISABLED" number that
										   fits into traceControl[1] (see below) */
#define TRACE_DFLT_NAM_CHR_MAX      39 /* Really The hardcoded max name len.
										  Name buffers should be +1 (for null terminator) - 40 was the value with 
										  8K pages which gave 127 NAMTBL_ENTS with "trace DISBALED".
										  Search for trace_created_init(...) call in "DISABLE" case.
										  Names can have this many characters (and always be null terminated - 
										  so names can be printed from nam tbl) */
#define TRACE_DFLT_NUM_ENTRIES   100000
#define TRACE_DFLT_TIME_FMT     "%m-%d %H:%M:%S.%%06d"   /* match default in trace_delta.pl */
#ifdef __KERNEL__
# define TRACE_DFLT_NAME        "KERNEL"
#else
# define TRACE_DFLT_NAME        "TRACE"
#endif

#if !defined(TRACE_NAME) && !defined(__KERNEL__)
static const char *  TRACE_NAME=NULL;
#elif !defined(TRACE_NAME) && defined(__KERNEL__)
# define TRACE_NAME TRACE_DFLT_NAME /* kernel doesn't have env.var, so different init path*/
#endif

#ifndef  TRACE_PRINT_FD
# define TRACE_PRINT_FD           1
#endif

/* 64bit sparc (nova.fnal.gov) has 8K pages (ref. ~/src/sysconf.c). This
   (double) is no big deal on systems with 4K pages; more important (effects
   userspace mapping) when actual 8K pages.
   Cygwin uses 64K pages. */
#define TRACE_PAGESIZE         0x10000
#define TRACE_CACHELINE        64

#ifdef __GNUC__
#define SUPPRESS_NOT_USED_WARN __attribute__((__unused__))
#else
#define SUPPRESS_NOT_USED_WARN
#endif

#define LVLBITSMSK ((sizeof(uint64_t)*8)-1)
#define TLVLMSK(xx) (1LL<<((xx)&LVLBITSMSK))

/* For C, these should go at the beginning of a block b/c they define new local variables */
#if defined(TRACE_NO_LIMIT_SLOW) || defined(__KERNEL__)
# define TRACE_LIMIT_SLOW(lvl,ins,tvp)  char ins[1]={'\0'};if(1)
#else
# define TRACE_LIMIT_SLOW(lvl,ins,tvp)  char ins[32]; static TRACE_THREAD_LOCAL limit_info_t _info={/*TRACE_ATOMIC_INIT,*/0,lsFREE,0}; \
                                    if(limit_do_print(tvp,&_info,ins,sizeof(ins)))
#endif

/* helper for TRACEing strings in C - ONLY in C!*/ 
#if defined(__cplusplus)
/* don't want to instantiate an std::vector as it may cause alloc/delete */
# define TSBUFDECL     /*std::vector<char> tsbuf__(traceControl_p->siz_msg);*/ /*&(tsbuf__[0])*/
/*# define TSBUFSIZ__     tsbuf__.size()*/
#else
# define TSBUFDECL      char tsbuf__[traceControl_p->siz_msg+1] SUPPRESS_NOT_USED_WARN; tsbuf__[0]='\0'
# define TSBUFSIZ__     sizeof(tsbuf__)
/* The following is what is to be optionally used: i.e.: TRACE(2,TSPRINTF("string=%s store_int=%%d",string),store_int);
   Watch for the case where string has an imbedded "%" */
# define TPRINTF( ... ) ( tsbuf__[0]?&(tsbuf__[0]):(snprintf(&(tsbuf__[0]),TSBUFSIZ__,__VA_ARGS__),&(tsbuf__[0])) )
#endif

/* Note: anonymous variadic macros were introduced in C99/C++11 (maybe C++0x) */

# define TRACE( lvl, ... ) do {											\
		TRACE_INIT_CHECK {												\
			struct timeval lclTime; TSBUFDECL; lclTime.tv_sec = 0;		\
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[traceTID].M & TLVLMSK(lvl))) { \
				trace( &lclTime, traceTID, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED \
				      , __VA_ARGS__ );	\
			}															\
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[traceTID].S & TLVLMSK(lvl))) { \
				TRACE_LIMIT_SLOW(lvl,_insert,&lclTime) {				\
					TRACE_LOG_FUNCTION( &lclTime, traceTID, lvl,_insert, TRACE_NARGS(__VA_ARGS__) \
					                   , __VA_ARGS__ ); \
				}														\
			}															\
        }								\
    } while (0)
/* static int tid_ could be TRACE_THREAD_LOCAL */
# define TRACEN( nam, lvl, ... ) do {									\
		TRACE_INIT_CHECK {												\
			static TRACE_THREAD_LOCAL int tid_=-1; struct timeval lclTime;					\
			TSBUFDECL;													\
			if(tid_==-1)tid_=name2TID(&(nam)[0]);	lclTime.tv_sec = 0;			\
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl))) { \
				trace( &lclTime, tid_, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED \
                      , __VA_ARGS__ );			\
			}															\
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[tid_].S & TLVLMSK(lvl))) {	\
				TRACE_LIMIT_SLOW(lvl,_insert,&lclTime) {				\
					TRACE_LOG_FUNCTION( &lclTime, tid_, lvl, _insert, TRACE_NARGS(__VA_ARGS__) \
					                   , __VA_ARGS__ ); \
				}														\
			}															\
        }								\
    } while (0)

#if defined(__cplusplus)

/* Note: This supports using a mix of stream syntax and format args, i.e: "string is " << some_str << " and float is %f", some_float
   Note also how the macro evaluates the first part (the "FMT") only once
   no matter which destination ("M" and/or "S") is active.
   Note: "xx" in TRACE_ARGS_FMT(__VA_ARGS__,xx) is just a dummy arg to that macro.
   THIS IS DEPRECATED. It is nice to have for comparison tests.
*/
# define TRACEN_( nam, lvl, ... ) do {									\
		TRACE_INIT_CHECK {												\
			static TRACE_THREAD_LOCAL int tid_=-1; struct timeval lclTime;				\
			if(tid_==-1)tid_=name2TID(&(nam)[0]);	lclTime.tv_sec = 0;			\
			bool do_m = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl));\
			bool do_s = traceControl_rwp->mode.bits.S && (traceNamLvls_p[tid_].S & TLVLMSK(lvl));\
			if (do_s || do_m) { std::ostringstream ostr__; /*instance creation is heavy weight*/ \
				ostr__ << TRACE_ARGS_FMT(__VA_ARGS__,xx);					\
				if(do_m) trace( &lclTime,tid_, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED \
				               , ostr__.str() TRACE_ARGS_ARGS(__VA_ARGS__) ); \
				if(do_s){TRACE_LIMIT_SLOW(lvl,_insert,&lclTime) {		\
						TRACE_LOG_FUNCTION( &lclTime, tid_, lvl, "", TRACE_NARGS(__VA_ARGS__) \
						                   , ostr__.str().c_str() TRACE_ARGS_ARGS(__VA_ARGS__) ); \
					}													\
				}														\
			}															\
        }																\
    } while (0)

#endif /* defined(___cplusplus) */


/* TRACE_NARGS configured to support 0 - 35 args */
# define TRACE_NARGS(...) TRACE_NARGS_HELP1(__VA_ARGS__,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0) /* 0 here but not below */
# define TRACE_NARGS_HELP1(...) TRACE_NARGS_HELP2(__VA_ARGS__,unused) /* "unused" to avoid warning "requires at least one argument for the "..." in a variadic macro" */
# define TRACE_NARGS_HELP2(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n, ...) n
# define TRACE_CNTL( ... ) traceCntl( TRACE_NARGS(__VA_ARGS__), __VA_ARGS__ )
# define TRACE_ARGS_FMT( first, ... ) first
/* TRACE_ARGS_ARGS(...) ignores the 1st arg (the "format" arg) and returns the remaining "args", if any.
   Being able 
   The trick is: the number of args in __VA_ARGS__ "shifts" the appropriate XX*(__VA_ARGS__) macro
   to the DO_THIS postition in the DO_XX macro. Then only that appropriate XX*(__VA_ARGS__) macro is
   evalutated; the others are ignored. The only 2 choices are XXX_X() or XXX_0(); XXX_X is for when there
   is between 1 and 35 args and XXX_0 is for 0 args. */
# define TRACE_ARGS_ARGS(...) \
	DO_XX(__VA_ARGS__,\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__),	\
		  XXX_X(__VA_ARGS__), XXX_X(__VA_ARGS__), XXX_0(__VA_ARGS__), unused ) /* "unused" to avoid warning "requires at least one argument for the "..." in a variadic macro" */
# define DO_XX(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,do_this,...) do_this
# define XXX_0(A)
# define XXX_X(A,...) ,__VA_ARGS__


#if   defined(__CYGWIN__)  /* check this first as __x86_64__ will also be defined */

# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
# define TRACE_PRINTF_FMT_ARG_NUM 5
# define TRACE_VA_LIST_INIT(addr) (va_list)addr
# define TRACE_ENT_TV_FILLER
# define TRACE_TSC32( low )       __asm__ __volatile__ ("rdtsc;movl %%eax,%0":"=m"(low)::"eax","edx")

#elif defined(__i386__)

# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
# define TRACE_PRINTF_FMT_ARG_NUM 5
# define TRACE_VA_LIST_INIT(addr) (va_list)addr
# define TRACE_ENT_TV_FILLER      uint32_t x[2];
# define TRACE_TSC32( low )       __asm__ __volatile__ ("rdtsc;movl %%eax,%0":"=m"(low)::"eax","edx")

#elif defined(__x86_64__)

# define TRACE_XTRA_PASSED        ,0, .0,.0,.0,.0,.0,.0,.0,.0
# define TRACE_XTRA_UNUSED        ,long l0 __attribute__((__unused__))\
	,double d0 __attribute__((__unused__)),double d1 __attribute__((__unused__)) \
	,double d2 __attribute__((__unused__)),double d3 __attribute__((__unused__)) \
	,double d4 __attribute__((__unused__)),double d5 __attribute__((__unused__)) \
	,double d6 __attribute__((__unused__)),double d7 __attribute__((__unused__))
# define TRACE_PRINTF_FMT_ARG_NUM 14
# define TRACE_VA_LIST_INIT(addr) {{6*8,6*8+9*16,addr,addr}}
# define TRACE_ENT_TV_FILLER
# define TRACE_TSC32( low )       __asm__ __volatile__ ("rdtsc" : "=a" (low) : : "edx")

#else

# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
# define TRACE_PRINTF_FMT_ARG_NUM 5
# define TRACE_VA_LIST_INIT(addr) {addr}
# if defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 4
#  define TRACE_ENT_TV_FILLER      uint32_t x[2];
# else
#  define TRACE_ENT_TV_FILLER
# endif
# define TRACE_TSC32( low )       

#endif


static void trace(struct timeval*,int,uint16_t,uint16_t TRACE_XTRA_UNUSED,const char *,...)__attribute__((format(printf,TRACE_PRINTF_FMT_ARG_NUM,TRACE_PRINTF_FMT_ARG_NUM+1)));
#ifdef __cplusplus
static void trace(struct timeval*,int,uint16_t,uint16_t TRACE_XTRA_UNUSED,const std::string&,...);
#endif


union trace_mode_u {
	struct {
		uint32_t M:1; /* b0 high speed circular Memory */
		uint32_t S:1; /* b1 printf (formatted) to Screen/Stdout */
	}   bits;
    uint32_t  mode;
};

typedef char trace_vers_t[sizeof(int64_t)*16];

struct traceControl_rw {
	TRACE_ATOMIC_T wrIdxCnt;	/* 32 bits */
	uint32_t       cacheline1[TRACE_CACHELINE/sizeof(int32_t)-(sizeof(TRACE_ATOMIC_T)/sizeof(int32_t))];   /* the goal is to have wrIdxCnt in it's own cache line */

	TRACE_ATOMIC_T namelock;	/* 32 bits */
	uint32_t       cacheline2[TRACE_CACHELINE/sizeof(int32_t)-(sizeof(TRACE_ATOMIC_T)/sizeof(int32_t))];   /* the goal is to have wrIdxCnt in it's own cache line */

	union trace_mode_u mode;
	uint32_t      reserved0;    /* use to be trigOffMode */
	uint32_t      trigIdxCnt;   /* BASED ON "M" mode Counts */
	int32_t       triggered;
	uint32_t	  trigActivePost;
	int32_t       full;
	uint32_t      limit_span_on_ms; /* 4 billion ms is 49 days */
	uint32_t      limit_span_off_ms;
	uint32_t      limit_cnt_limit;
	uint32_t      longest_name;    /* helps with trace_user if printing names */
	uint32_t      xtra[TRACE_CACHELINE/sizeof(int32_t)-10]; /* force some sort of alignment -- taking into account the 6 fields (above) since the last cache line alignment */
};
struct traceControl_s {
    trace_vers_t   version_string;
    uint32_t	   version;             /*  1 */
    uint32_t       num_params;          /*  2 */
    uint32_t       siz_msg;             /*  3 */
    uint32_t       siz_entry;           /*  4 */
    uint32_t       num_entries;         /*  5 */
    uint32_t       largest_multiple;    /*  6 */
    uint32_t       num_namLvlTblEnts;   /*  7 */
  volatile int32_t trace_initialized;   /*  8 these and above would be read only if */
    uint32_t       memlen;              /*  9 in kernel */
    uint32_t       create_tv_sec;       /* 10 */
    uint32_t       largest_zero_offset; /* 11 */
    uint32_t       page_align[TRACE_PAGESIZE/sizeof(int32_t)-(11+sizeof(trace_vers_t)/sizeof(int32_t))]; /* allow mmap 1st page(s) (stuff above) readonly */

	/* "page" break */
	struct traceControl_rw rw;
};
/*                                      bytes  TRACE_SHOW cntl char */
struct traceEntryHdr_s                /*-----   -------        */
{   struct timeval time;              /* 16        T */
    TRACE_ENT_TV_FILLER	     /* because timeval is larger on x86_64 (16 bytes compared to 8 for i686) */
    uint64_t       tsc;               /*  8        t */
    uint16_t       lvl;               /*  2        L or l */
    uint16_t       nargs;             /*  2        # */
    pid_t          pid;               /*  4        P system info */
    pid_t          tid;               /*  4        i system info - "thread id" */
    int32_t        cpu;               /*  4        C -- kernel sched switch will indicate this info? */
    int32_t        TrcId;             /*  4        I Trace ID ==> idx into lvlTbl, namTbl */
    uint16_t       get_idxCnt_retries;/*  2        R */
    uint16_t       param_bytes;       /*  2        B */
};                                    /* ---       M -- NO, ALWAY PRINTED LAST! formated Message */
/* msg buf,then params buf               48   adding uint32_t line;char file[60] (another cache line) doesn't seem worth it */
/* see entSiz(siz_msg,num_params) and idxCnt2entPtr(idxCnt) */ /* other - N  index */

struct traceNamLvls_s
{   uint64_t      M;
    uint64_t      S;
    uint64_t      T;
    char          name[TRACE_DFLT_NAM_CHR_MAX+1];
};


/*--------------------------------------------------------------------------*/
/* Enter the 5 use case "areas" -- see doc/5in1.txt                         */
/*  defining TRACE_DEFINE wins over DECLARE or nothing. STATIC wins over all */
/* It's OK if one module has DEFINE and all the reset have any of (nothing, DECLARE, STATIC) */
/* The only thing that is invalid is if more than one has DEFINE.           */
/* If any have DECLARE, then there must be one (and only one) with DEFINE.  */
#if   defined(TRACE_STATIC) && !defined(__KERNEL__)
# define TRACE_DECL( var_type_and_name, initializer ) static var_type_and_name initializer
#elif defined(TRACE_DEFINE)
# define TRACE_DECL( var_type_and_name, initializer ) var_type_and_name initializer
#elif defined(TRACE_DECLARE) || defined(__KERNEL__)
# define TRACE_DECL( var_type_and_name, initializer ) extern var_type_and_name
#else
# define TRACE_DECL( var_type_and_name, initializer ) static var_type_and_name initializer
#endif

//#define TRACE_THREAD_LOCALX TRACE_THREAD_LOCAL    /* use this for separate FILE per thread -- very rare; perhaps NUMA issue??? */
#define TRACE_THREAD_LOCALX

TRACE_DECL( struct traceControl_s    traceControl[2], ); /* for when TRACE is disabled. NOTE: traceNamLvls_p should always point to traceControl_p+1 */
TRACE_DECL( TRACE_THREAD_LOCALX struct traceControl_s  *traceControl_p, =NULL );
TRACE_DECL( TRACE_THREAD_LOCALX struct traceControl_rw  *traceControl_rwp, =NULL );
TRACE_DECL( TRACE_THREAD_LOCALX struct traceNamLvls_s *traceNamLvls_p, =(struct traceNamLvls_s*)&traceControl[1] );
TRACE_DECL( TRACE_THREAD_LOCALX struct traceEntryHdr_s *traceEntries_p, );
static TRACE_THREAD_LOCAL int   traceTID=-1;  /* idx into lvlTbl, namTbl -- always
												static (never global) and possibly
thread_local -- this will cause the most traceInit calls (but not much work, hopefully),
which will ensure that a module (not thread) can be assigned it's own trace name/id. */

TRACE_DECL( unsigned long trace_lvlS,         =0 );
TRACE_DECL( unsigned long trace_lvlM,         =0 );

#if defined(__KERNEL__)
TRACE_DECL( int           trace_allow_printk, =0 );          /* module_param */
static TRACE_THREAD_LOCAL const char *traceName=TRACE_DFLT_NAME;
#else
TRACE_DECL( TRACE_THREAD_LOCAL const char *traceName, =TRACE_DFLT_NAME );
TRACE_DECL( TRACE_THREAD_LOCALX const char *traceFile, ="/tmp/trace_buffer_%u" );/*a local/efficient FS device is best; operation when path is on NFS device has not been studied*/
TRACE_DECL( TRACE_THREAD_LOCAL pid_t traceTid,     =0 );  /* thread id */
TRACE_DECL( pid_t                    tracePid,     =0 );
TRACE_DECL( int                      tracePrintFd, =1 );
TRACE_DECL( TRACE_ATOMIC_T traceInitLck, =TRACE_ATOMIC_INIT );
TRACE_DECL( uint32_t traceInitLck_hung_max, =0 );
TRACE_DECL( const char * tracePrintFmt, =NULL ); /* hardcoded default below that can only be overridden via env.var */
TRACE_DECL( const char * tracePrintEndL, =NULL ); /* hardcoded default below that can only be overridden via env.var */
TRACE_DECL( size_t       tracePrintEndLen, =-1 );
static char traceFile_static[PATH_MAX]={0};
static struct traceControl_s *traceControl_p_static=NULL;
#endif
/*--------------------------------------------------------------------------*/

/* forward declarations, important functions */
static struct traceEntryHdr_s*  idxCnt2entPtr( uint32_t idxCnt );
#if !defined(__KERNEL__) || defined(TRACE_IMPL)  /* K=0,IMPL=0; K=0,IMPL=1; K=1,IMPL=1 */
static int                      traceInit( const char *name );
static void                     traceInitNames( struct traceControl_s*, struct traceControl_rw* );
# ifdef __KERNEL__                               /*                         K=1,IMPL=1 */
static int                msgmax=TRACE_DFLT_MAX_MSG_SZ;      /* module_param */
static int                argsmax=TRACE_DFLT_MAX_PARAMS;     /* module_param */
static int                numents=TRACE_DFLT_NUM_ENTRIES;    /* module_param */
static int                namtblents=TRACE_DFLT_NAMTBL_ENTS; /* module_param */
static int                trace_buffer_numa_node=-1;	     /* module_param */
# endif
#else    /*                                         K=1,IMPL=0  */

#endif   /*  __KERNEL__             TRACE_IMPL  */

static int                      traceCntl( int nargs, const char *cmd, ... );
static int32_t                  name2TID( const char *name );
#define cntlPagesSiz()          ((uint32_t)sizeof(struct traceControl_s))
#define namtblSiz( ents )       (((uint32_t)sizeof(struct traceNamLvls_s)*ents+TRACE_CACHELINE-1)&~(TRACE_CACHELINE-1))
#define entSiz( siz_msg, num_params ) ( sizeof(struct traceEntryHdr_s)\
    + sizeof(uint64_t)*num_params /* NOTE: extra size for i686 (32bit processors) */\
    + siz_msg )
#define traceMemLen( siz_cntl_pages, num_namLvlTblEnts, siz_msg, num_params, num_entries ) \
    (( siz_cntl_pages							\
      + namtblSiz( num_namLvlTblEnts )					\
      + entSiz(siz_msg,num_params)*num_entries				\
      + TRACE_PAGESIZE-1 )						\
     & ~(TRACE_PAGESIZE-1) )

/* The "largest_multiple" method (using (ulong)-1) allows "easy" "add 1"
   I must do the substract (ie. add negative) by hand.
   Ref. ShmRW class (~/src/ShmRW?)
   Some standards don't seem to line "static inline"
   Use of the following seems to produce the same code as the optimized
   code which calls inline idxCnt_add (the c11/c++11 optimizer seem to do what
   this macro does).
   NOTE: when using macro version, "add" should be of type int32_t */
#if 1
#define IDXCNT_ADD( idxCnt, add )					\
    (((add)<0)										\
	?(((uint32_t)-(add)>(idxCnt))										\
	   ?(traceControl_p->largest_multiple-(-(add)-(idxCnt)))%traceControl_p->largest_multiple \
	   :((idxCnt)-(-(add)))%traceControl_p->largest_multiple			\
       )								\
	 :((idxCnt)+(add))%traceControl_p->largest_multiple	\
    )
#else
static uint32_t IDXCNT_ADD( uint32_t idxCnt, int32_t add )
{
	uint32_t retval;
	if (add < 0)
		if (-add > idxCnt)
			retval = (traceControl_p->largest_multiple-(-add-idxCnt))%traceControl_p->largest_multiple;
		else
			retval = (idxCnt-(-add))%traceControl_p->largest_multiple;
	else
		retval = (idxCnt+add)%traceControl_p->largest_multiple;
	return retval;
}
#endif
#define IDXCNT_DELTA( cur, prv )			\
    ((cur>=prv)						\
     ? cur-prv						\
     : cur-prv-traceControl_p->largest_zero_offset	\
    )

#ifndef TRACE_LOG_FUNCTION
#  define TRACE_LOG_FUNCTION(tvp,tid,lvl,insert,nargs,...)          trace_user( tvp,tid,lvl,insert,nargs TRACE_XTRA_PASSED,__VA_ARGS__ )
#elif defined(TRACE_LOG_FUN_PROTO)
/* prototype for TRACE_LOG_FUNCTION as compiled in Streamer class below */
TRACE_LOG_FUN_PROTO;
#endif /* TRACE_LOG_FUNCTION */


static uint32_t trace_lock( TRACE_ATOMIC_T *atomic_addr )
{
	uint32_t desired=1, expect=0, hung=0;
#if defined(__KERNEL__)
	while (cmpxchg(atomic_addr,expect,desired) != expect)
		if (++hung >100000000) break;
#elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
	while (!atomic_compare_exchange_weak(atomic_addr, &expect, desired)) {
		expect=0;
		if (++hung >100000000) break;
	}
	if(atomic_addr==&traceInitLck && traceInitLck_hung_max<hung) traceInitLck_hung_max=hung;
#else
	while (cmpxchg(atomic_addr,expect,desired) != expect)
		if (++hung >100000000) break;
	if(atomic_addr==&traceInitLck && traceInitLck_hung_max<hung) traceInitLck_hung_max=hung;
#endif
	return ((hung<=100000000)?1:0);
}   /* trace_lock */

static void trace_unlock( TRACE_ATOMIC_T *atomic_addr )
{
#if defined(__KERNEL__)
    TRACE_ATOMIC_STORE(atomic_addr,(uint32_t)0);
#elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
    atomic_store(atomic_addr,(uint32_t)0);
#else
    TRACE_ATOMIC_STORE(atomic_addr,(uint32_t)0);
#endif
}   /* trace_unlock */


typedef enum {
	lsFREE,
	lsLIMITED
} limit_state_t;
 
typedef struct {
    /* choice: whole struct TLS or normal static with member:  TRACE_ATOMIC_T lock;*/
	uint64_t      span_start_ms;
	limit_state_t state;
	uint32_t      cnt;
} limit_info_t;

/* if a "do print" (return true/1) and if insert is provided and sz
   is non-zero, it will be NULL terminated */
SUPPRESS_NOT_USED_WARN
static inline int limit_do_print( struct timeval *tvp, limit_info_t *info, char *insert, size_t sz )
{
	uint64_t delta_ms, tnow_ms;
	int      do_print;
	/*struct timeval tvx;
	  trace( &tvx, 125, 1, 1 TRACE_XTRA_PASSED, "limit_do_print _cnt_=%u", traceControl_rwp->limit_cnt_limit );*/

	if (traceControl_rwp->limit_cnt_limit == 0) {
		if (insert && sz) *insert='\0';
		return (1);
	}
	if (tvp->tv_sec == 0) TRACE_GETTIMEOFDAY( tvp );
	tnow_ms = tvp->tv_sec*1000 + tvp->tv_usec/1000;
	/* could lock  trace_lock( &(info->lock) );*/
	delta_ms = tnow_ms - info->span_start_ms;
	if (info->state == lsFREE) {
		if (++(info->cnt) >= traceControl_rwp->limit_cnt_limit) {
			if (insert) {
				strncpy( insert, "[RATE LIMIT] ", sz );
				/*fprintf( stderr, "[LIMIT (%u/%.1fs) REACHED]\n", traceControl_rwp->limit_cnt_limit, (float)traceControl_rwp->limit_span_on_ms/1000000);*/
			}
			info->state = lsLIMITED;
			info->span_start_ms = tnow_ms; /* start tsLIMITED timespan */
			info->cnt   = 0;
		} else if (delta_ms >= traceControl_rwp->limit_span_on_ms) { /* start new timespan */
			info->span_start_ms = tnow_ms;
			info->cnt   = 1;
			if (insert && sz) *insert='\0';
		} else if (insert && sz) { /* counting messages in this period */
			*insert='\0';
		}
		do_print = 1;
	} else { /* state must be tsLIMITED */
		if (delta_ms >= traceControl_rwp->limit_span_off_ms) { /* done limiting, start new timespace */
			if (insert)
				snprintf( insert, sz, "[RESUMING dropped: %u] ", info->cnt );
			info->state = lsFREE;
			info->span_start_ms = tnow_ms;
			info->cnt=0;
			do_print = 1;
		} else {
			++(info->cnt);
			do_print = 0;
		}
	}
	/* unlock  trace_unlock( &(info->lock) );*/
	return (do_print);
}   /* limit_do_print */

#ifndef TRACE_4_LVLSTRS
# define TRACE_4_LVLSTRS "err","wrn","nfo","dbg"
#endif
#ifndef TRACE_60_LVLSTRS
# define TRACE_60_LVLSTRS       "d04","d05","d06","d07","d08","d09",	\
		"d10","d11","d12","d13","d14","d15","d16","d17","d18","d19",	\
		"d20","d21","d22","d23","d24","d25","d26","d27","d28","d29",	\
		"d30","d31","d32","d33","d34","d35","d36","d37","d38","d39",	\
		"d40","d41","d42","d43","d44","d45","d46","d47","d48","d49",	\
		"d50","d51","d52","d53","d54","d55","d56","d57","d58","d59",	\
		"d60","d61","d62","d63"
#endif
static const char _lvlstr[64][4] = { TRACE_4_LVLSTRS, TRACE_60_LVLSTRS };
 
SUPPRESS_NOT_USED_WARN
static void vtrace_user( struct timeval *tvp, int TrcId, uint16_t lvl, const char *insert, uint16_t nargs, const char *msg, va_list ap )
{
#ifdef __KERNEL__
	if (!trace_allow_printk) return;
	TRACE_VPRINT( msg, ap );
	if(msg[strlen(msg)-1]!='\n')TRACE_PRINT("\n");
#else
    /* I format output in a local output buffer (with specific/limited size)
       first. There are 2 main reasons that this is done:
       1) allows the use of write to a specific tracePrintFd;
       2) there will be one system call which is most efficient and less likely
	   to have the output mangled in a multi-threaded environment.
    */
	char   obuf[TRACE_STREAMER_MSGMAX]; char tbuf[0x100]; size_t printed=0;
	char   *cp;
	struct tm	tm_s;
	int    quiet_warn=0;
	if (tracePrintFmt==NULL) {
		/* no matter who writes, it should basically be the same thing */
		if((cp=getenv("TRACE_TIME_FMT"))!=NULL) tracePrintFmt=cp; /* single write here */
		else                                    tracePrintFmt=TRACE_DFLT_TIME_FMT; /* OR single write here */
	}
	if (tracePrintEndLen == (size_t)-1) {
		if((cp=getenv("TRACE_ENDL"))!=NULL && *cp != '\0') {
			tracePrintEndL=cp;
			tracePrintEndLen=strlen(cp);
			while (tracePrintEndLen && cp[tracePrintEndLen-1] == '\n') /* strip off ending newlines and adjust endlen */
				cp[--tracePrintEndLen] = '\0';
		}
		else
			tracePrintEndLen = 0;
	}
	if (tvp->tv_sec == 0) TRACE_GETTIMEOFDAY( tvp );
# if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
# endif
	localtime_r( (time_t *)&tvp->tv_sec, &tm_s );
# if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#  pragma GCC diagnostic pop
# endif
	strftime( tbuf, sizeof(tbuf), tracePrintFmt, &tm_s );
	printed = snprintf( obuf, sizeof(obuf), tbuf, (int)tvp->tv_usec ); /* possibly (probably) add usecs */

# if 1
	printed += snprintf( &(obuf[printed]), sizeof(obuf)-printed
	                    , &(" %*s %s %s")[printed==0?1:0] /* skip leading " " if nothing was printed (TRACE_TIME_FMT="") */
	                    , traceControl_rwp->longest_name,traceNamLvls_p[TrcId].name
	                    , _lvlstr[lvl&LVLBITSMSK]?_lvlstr[lvl&LVLBITSMSK]:"", insert );

	if (nargs)
		printed += vsnprintf( &(obuf[printed])
		                     , (printed<(int)sizeof(obuf))?sizeof(obuf)-printed:0
		                     , msg, ap );
	else /* don't do any parsing for format specifiers in the msg -- tshow will
			also know to do this on the memory side of things */
		printed += snprintf( &(obuf[printed])
		                    , (printed<(int)sizeof(obuf))?sizeof(obuf)-printed:0
		                    , "%s", msg );

	if (tracePrintEndLen) {
		if (obuf[printed-1] == '\n') /* roll back any trailing nl at this point */
			--printed;
		if (printed >= (sizeof(obuf)-1-tracePrintEndLen)) { /* sizeof(obuf)-1 to leave room for potential nl */
			strcpy( &obuf[sizeof(obuf)-1-tracePrintEndLen], tracePrintEndL );
			printed = sizeof(obuf)-1;
		} else {
			strcpy( &obuf[printed], tracePrintEndL );
			printed += tracePrintEndLen;
		}
	}

	/* why not use writev??? B/c when writing to stdout, only each individual
	   vector (not the whole array of vectors) is atomic/thread safe */
	if (printed < (int)sizeof(obuf)) {
		/* there is room for the \n */
		/* buf first see if it is needed */
		if (obuf[printed-1] != '\n')
		{   obuf[printed++] = '\n'; /* overwriting \0 is OK as we will specify the amount to write */
			/*printf("added \\n printed=%d\n",printed);*/
		}
		/*else printf("already there printed=%d\n",printed);*/
		quiet_warn += write( tracePrintFd, obuf, printed );
	} else {
		/* obuf[sizeof(obuf)-1] has '\0'. see if we should change it to \n */
		if (obuf[sizeof(obuf)-2] == '\n')
			quiet_warn += write( tracePrintFd, obuf, sizeof(obuf)-1 );
		else {
			obuf[sizeof(obuf)-1] = '\n';
			quiet_warn += write( tracePrintFd, obuf, sizeof(obuf) );
			/*printf("changed \\0 to \\n printed=%d\n",);*/
		}
	}
# else
	/* NOTE: even though man pages seem to indicate the single writev should be atomic:
	   ...
       The data transfers performed by readv() and writev()  are  atomic:  the
       data  written  by  writev()  is  written  as a single block that is not
       intermingled with output  from  writes  in  other  processes  (but  see
       pipe(7) for an exception); analogously, readv() is guaranteed to read a
       contiguous block of data from the file, regardless of  read  operations
       performed  in  other  threads  or  processes that have file descriptors
       referring to the same open file description (see open(2)).
	   ...

	   The above man page appears to be wrong, at least for SL7 -
	   Linux mu2edaq01.fnal.gov 3.10.0-514.10.2.el7.x86_64 #1 SMP Thu Mar 2 11:21:24 CST 2017 x86_64 x86_64 x86_64 GNU/Linux

/home/ron/work/tracePrj/trace
mu2edaq01 :^) tcntl test-threads 0 -l5 & tcntl test-threads 0 -l5 & tcntl test-threads 0 -l5
[3] 28144
[4] 28145
11-13 10:31:23.604651    TRACE wrn 11-13 10:31:23.604651    TRACE wrn before pthread_create - main_tid=28146 loops=5, threads=0, dly_ms=0 traceControl_p=0x613740before pthread_create - main_tid=28147 loops=5, threads=0, dly_ms=0 traceControl_p=0x613740

11-13 10:31:23.604736    TRACE inf 11-13 10:31:23.604736    TRACE inf tidx=0 loop=1 of 5 tid=28146 I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message manglingtidx=0 loop=1 of 5 tid=28147 I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message mangling

11-13 10:31:23.604746    TRACE inf 11-13 10:31:23.604747    TRACE inf tidx=0 loop=1 of 5 tid=28146 this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9tidx=0 loop=1 of 5 tid=28147 this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9

11-13 10:31:23.604762     TRACE inf 11-13 10:31:23.604762     TRACE inf tidx=0 loop=2 of 5 tid=28146 I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message manglingtidx=0 loop=2 of 5 tid=28147 I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message mangling

11-13 10:31:23.604772     TRACE inf 11-13 10:31:23.604773     TRACE inf tidx=0 loop=2 of 5 tid=28146 this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9tidx=0 loop=2 of 5 tid=28147 this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9

11-13 10:31:23.604780     TRACE inf 11-13 10:31:23.604781     TRACE inf tidx=0 loop=3 of 5 tid=28146 I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message manglingtidx=0 loop=3 of 5 tid=28147 I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message mangling
...
11-13 10:31:23.604895     TRACE inf tidx=0 loop=4 of 5 tid=28148 this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9
11-13 10:31:23.605072     TRACE inf tidx=0 loop=5 of 5 tid=28148 I need to test longer messages. They need to be about 256 characters - longer than the circular memory message buffer size. This will check for message mangling
11-13 10:31:23.605077     TRACE inf tidx=0 loop=5 of 5 tid=28148 this is the second long message - second0 second1 second2 second3 second4 second5 second6 second7 second8 second9
11-13 10:31:23.605082     TRACE wrn after pthread_join - main_tid=28148 traceControl_p=0x613740
--2017-11-13_10:31:23--
	*/
	{
		struct iovec  iov[6]; int iovcnt=0;
		printed += snprintf( &(obuf[printed]), sizeof(obuf)-printed
		                    , &(" %*s %s ")[printed==0?1:0] /* skip leading " " if nothing was printed (TRACE_TIME_FMT="") */
		                    , traceControl_rwp->longest_name,traceNamLvls_p[TrcId].name
		                    , _lvlstr[lvl&LVLBITSMSK]?_lvlstr[lvl&LVLBITSMSK]:"" );
		iov[iovcnt  ].iov_base = obuf;
		iov[iovcnt++].iov_len  = printed;
		if (insert && insert[0]) {
			iov[iovcnt  ].iov_base = (void*)insert;
			iov[iovcnt++].iov_len  = strlen(insert);
		}
		if (nargs) {
			int sts;
			iov[iovcnt  ].iov_base = &(obuf[printed]);
			sts = vsnprintf( &(obuf[printed])
			                , (printed<(int)sizeof(obuf))?sizeof(obuf)-printed:0
			                , msg, ap );
			if (obuf[printed+sts-1] == '\n')
				iov[iovcnt++].iov_len  = sts-1;
			else
				iov[iovcnt++].iov_len  = sts;
		} else {
			int len=strlen(msg);
			iov[iovcnt  ].iov_base = (void*)msg;
			iov[iovcnt++].iov_len  = msg[len-1]=='\n'?len-1:len;
		}
		if (tracePrintEndLen) {
			iov[iovcnt  ].iov_base = (void*)tracePrintEndL;
			iov[iovcnt++].iov_len  = tracePrintEndLen;
		}
		iov[iovcnt  ].iov_base = (void*)"\n";
		iov[iovcnt++].iov_len  = 1;
		quiet_warn += writev( tracePrintFd, iov, iovcnt );
	}
# endif
#endif
}   /* vtrace_user */
SUPPRESS_NOT_USED_WARN
static void trace_user( struct timeval *tvp, int TrcId, uint16_t lvl, const char *insert, uint16_t nargs TRACE_XTRA_UNUSED, const char *msg, ... )
{
	va_list ap;
	va_start( ap, msg );
	vtrace_user( tvp, TrcId, lvl, insert, nargs, msg, ap );
	va_end( ap );
}   /* trace_user - const char* */
#ifdef __cplusplus
# if (__cplusplus>=201103L)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wvarargs"
# endif
SUPPRESS_NOT_USED_WARN
static void trace_user( struct timeval *tvp, int TrcId, uint16_t lvl, const char *insert, uint16_t nargs TRACE_XTRA_UNUSED, const std::string& msg, ... )
{
    va_list ap;
	va_start( ap, msg );
	vtrace_user( tvp, TrcId, lvl, insert, nargs, msg.c_str(), ap );
	va_end( ap );	
}   /* trace_user - std::string& */
# if (__cplusplus>=201103L)
#  pragma GCC diagnostic pop
# endif
#endif


static void vtrace( struct timeval *tvp, int trcId, uint16_t lvl, uint16_t nargs
		  , const char *msg, va_list ap )
{
    struct traceEntryHdr_s* myEnt_p;
    char                  * msg_p;
    uint64_t              * params_p; /* some archs (ie. i386,32 bit arm) pass have 32 bit and 64 bit args; use biggest */
    unsigned                argIdx;
    uint16_t                get_idxCnt_retries=0;
    uint32_t                myIdxCnt=TRACE_ATOMIC_LOAD( &traceControl_rwp->wrIdxCnt );

#if defined(__KERNEL__)
    uint32_t desired=IDXCNT_ADD(myIdxCnt,1);
    while (cmpxchg(&traceControl_rwp->wrIdxCnt,myIdxCnt,desired)!=myIdxCnt)
    {   ++get_idxCnt_retries;
	myIdxCnt=TRACE_ATOMIC_LOAD( &traceControl_rwp->wrIdxCnt );
	desired = IDXCNT_ADD( myIdxCnt,1);
    }
#elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
	pid_t    chkpid;
    uint32_t desired=IDXCNT_ADD(myIdxCnt,1);
    while (!atomic_compare_exchange_weak(&traceControl_rwp->wrIdxCnt
					 , &myIdxCnt, desired))
    {   ++get_idxCnt_retries;
	desired = IDXCNT_ADD( myIdxCnt,1);
    }
#else
	pid_t    chkpid;
    uint32_t desired=IDXCNT_ADD(myIdxCnt,1);
    while (cmpxchg(&traceControl_rwp->wrIdxCnt,myIdxCnt,desired)!=myIdxCnt)
    {   ++get_idxCnt_retries;
	myIdxCnt=TRACE_ATOMIC_LOAD( &traceControl_rwp->wrIdxCnt );
	desired = IDXCNT_ADD( myIdxCnt,1);
    }
#endif

    if (myIdxCnt == traceControl_p->num_entries)
	traceControl_rwp->full = 1; /* now we'll know if wrIdxCnt has rolled over */

	/* There are some places in the kernel where the gettimeofday routine
	   cannot be called (i.e. kernel/notifier.c routines). For these routines,
	   add 64 for the level (i.e. 22+64) */
#ifdef __KERNEL__
    if(lvl>=64){tvp->tv_sec=1;tvp->tv_usec=0;}
	else
#endif
    if (tvp->tv_sec == 0) TRACE_GETTIMEOFDAY( tvp );  /* hopefully NOT a system call */

    myEnt_p  = idxCnt2entPtr( myIdxCnt );
    msg_p    = (char*)(myEnt_p+1);
    params_p = (uint64_t*)(msg_p+traceControl_p->siz_msg);

    myEnt_p->time = *tvp;
    TRACE_TSC32( myEnt_p->tsc );
    myEnt_p->lvl  = lvl;
    myEnt_p->nargs = nargs;
#if defined(__KERNEL__)
    myEnt_p->pid  = current->tgid;
    myEnt_p->tid  = current->pid;
    myEnt_p->cpu  = raw_smp_processor_id();
#else
	if (tracePid != (chkpid=getpid())) /* "pid/tid not changed after 2nd fork" issue */
		tracePid = traceTid = chkpid;
    myEnt_p->pid  = tracePid;
    myEnt_p->tid  = traceTid;
    myEnt_p->cpu  = trace_getcpu(); /* this costs alot :( */
#endif
    myEnt_p->TrcId  = trcId;
    myEnt_p->get_idxCnt_retries = get_idxCnt_retries;
    myEnt_p->param_bytes = sizeof(long);

    strncpy(msg_p,msg,traceControl_p->siz_msg);
    /* emulate stack push - right to left (so that arg1 end up at a lower
       address, arg2 ends up at the next higher address, etc. */
    if (nargs) {
	    if (nargs > traceControl_p->num_params) nargs=traceControl_p->num_params;
	for (argIdx=0; argIdx<nargs; ++argIdx)
	    params_p[argIdx]=va_arg(ap,uint64_t); /* this will usually copy 2x and 32bit archs, but they might be all %f or %g args */
    }
	if (traceControl_rwp->trigActivePost) { /* armed, armed/trigger */
		if (traceControl_rwp->triggered) { /* triggered */
			if (IDXCNT_DELTA(myIdxCnt,traceControl_rwp->trigIdxCnt)
			    >=traceControl_rwp->trigActivePost ) {
				/* I think there should be an indication in the M buffer */
				TRACE_CNTL( "modeM", (uint64_t)0 );   /* calling traceCntl here eliminates the "defined but not used" warning for modules which do not use TRACE_CNTL */
				traceControl_rwp->trigActivePost = 0;
				/* triggered and trigIdxCnt should be cleared when
				   "armed" (when trigActivePost is set) */
			}
			/* else just waiting... */
		} else if (traceNamLvls_p[trcId].T & TLVLMSK(lvl)) {
			traceControl_rwp->triggered = 1;
			traceControl_rwp->trigIdxCnt = myIdxCnt;
		}
    }
}   /* vtrace */

#if (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"   /* b/c of TRACE_XTRA_UNUSED */
# ifdef __cplusplus
#  pragma GCC diagnostic ignored "-Wvarargs"
# endif
#endif

SUPPRESS_NOT_USED_WARN
static void trace( struct timeval *tvp, int trcId, uint16_t lvl, uint16_t nargs
                  TRACE_XTRA_UNUSED, const char *msg, ... )
{
    va_list ap;
	va_start( ap, msg );
	vtrace( tvp, trcId, lvl, nargs, msg, ap );
	va_end( ap );	
}   /* trace */

#ifdef __cplusplus
SUPPRESS_NOT_USED_WARN
static void trace( struct timeval *tvp, int trcId, uint16_t lvl, uint16_t nargs
                  TRACE_XTRA_UNUSED, const std::string& msg, ... )
{
    va_list ap;
	va_start( ap, msg );
	vtrace( tvp, trcId, lvl, nargs, msg.c_str(), ap );
	va_end( ap );	
}   /* trace */
#endif

#if (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
# pragma GCC diagnostic pop
#endif

/* Search for name anf insert if not found and not full
 */
static int32_t name2TID( const char *nn )
{
    uint32_t ii, len;
	const char *name=(nn&&nn[0])?nn:traceName;
	char     valid_name[TRACE_DFLT_NAM_CHR_MAX+1];
#if defined(__KERNEL__)
    if (traceEntries_p==NULL) return -1;
#endif
	//fprintf(stderr,"n2t=%p %s\n",name,name);
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
		if (strncmp(traceNamLvls_p[ii].name,name,TRACE_DFLT_NAM_CHR_MAX)==0) {
			return (ii);
		}
	/* only allow "valid" names to be inserted -- above, we assumed the
	   name was valid, giving the caller the benefit of the doubt, for
	   efficiency sake, but here we will make sure the name is valid */
	for (ii=0; name[ii]!='\0' && ii<TRACE_DFLT_NAM_CHR_MAX; ++ii) {
		if (isgraph(name[ii]))
			valid_name[ii] = name[ii];
		else
			valid_name[ii]='_';
	}
	valid_name[ii]='\0'; len=ii;
	/* NOTE: multiple threads which may want to create the same name might arrive at this
       point at the same time. Checking for the name again, with the lock, make this OK.
	*/
    if (!trace_lock(&traceControl_rwp->namelock)) TRACE_PRINT("trace_lock: namelock hung?\n");
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii) {
		if (traceNamLvls_p[ii].name[0] == '\0') {
			strncpy(traceNamLvls_p[ii].name,valid_name,TRACE_DFLT_NAM_CHR_MAX);
			if(trace_lvlS)         /* See also traceInitNames */
				traceNamLvls_p[ii].S = trace_lvlS;
			if(trace_lvlM)         /* See also traceInitNames */
				traceNamLvls_p[ii].M = trace_lvlM;
			if (traceControl_rwp->longest_name < len)
				traceControl_rwp->longest_name = len;
			trace_unlock( &traceControl_rwp->namelock );
			return (ii);
		}
		if (strncmp(traceNamLvls_p[ii].name,valid_name,TRACE_DFLT_NAM_CHR_MAX)==0) {
			trace_unlock( &traceControl_rwp->namelock );
			return (ii);
		}
	}
    trace_unlock( &traceControl_rwp->namelock );
    return (traceControl_p->num_namLvlTblEnts-1);  /* the case for when the table is full */
}   /* name2TID */


/* these were originally just used in the mmap function */
#define MM_STR(xx) MM_STR2(xx)
#define MM_STR2(xx) #xx

#ifndef __KERNEL__
static void trace_namLvlSet( void )
{
    const char *cp;
	int         sts;
	/* This is a signficant env.var as this can be setting the
	   level mask for many TRACE NAMEs/IDs AND potentially many process
	   could do this or at least be TRACE-ing. In other words, this has
	   the potential of effecting another process's TRACE-ing. */
	if ((cp=getenv("TRACE_NAMLVLSET"))) {
		int                 ign; /* will ignore trace id (index) if present */
		char                name[TRACE_DFLT_NAM_CHR_MAX+1];
		unsigned long long  M,S,T=0;
		while (   ((sts=sscanf(cp,"%d %" MM_STR(TRACE_DFLT_NAM_CHR_MAX) "s %llx %llx %llx",&ign,name,&M,&S,&T))&&sts>=4)
		       || ((sts=sscanf(cp,   "%" MM_STR(TRACE_DFLT_NAM_CHR_MAX) "s %llx %llx %llx",     name,&M,&S,&T))&&sts>=3)) {
			int tid=name2TID(name);
			/*fprintf(stderr,"name=%s sts=%d\n",name,sts );*/
			traceNamLvls_p[tid].M = M;
			traceNamLvls_p[tid].S = S;
			traceNamLvls_p[tid].T = T;
			cp = strchr( cp, '\n' );
			if (cp == NULL)
				break;
			++cp;
			T=0;
		}
		if (cp != NULL && *cp != '\0')
			fprintf( stderr, "Warning: TRACE_NAMLVLSET in env., but processing did not complete\n" );
		if ((cp=getenv("TRACE_MODE")))
			traceControl_rwp->mode.mode = strtoul( cp,NULL,0 );
	}
	if ((cp=getenv("TRACE_LIMIT_MS"))) {
		unsigned cnt; unsigned long long  on_ms, off_ms;
		sts=sscanf( cp, "%u,%llu,%llu", &cnt, &on_ms, &off_ms );
		switch (sts) {
		case 2: off_ms = on_ms;
		case 3: traceControl_rwp->limit_cnt_limit = cnt;
			traceControl_rwp->limit_span_on_ms = on_ms;
			traceControl_rwp->limit_span_off_ms = off_ms;
			break;
		default:
			fprintf( stderr, "Warning: problem parsing TRACE_LIMIT_MS - should be: <cnt>,<on_ms>[,off_ms]\n" );
			traceControl_rwp->limit_cnt_limit = 0;
		}
	}
}   /* trace_namLvlSet */
#endif

static inline void trace_msk_op( uint64_t *v1, int op, uint64_t v2 )
{
	switch (op) {
	case 0: *v1  =  v2;  break;
	case 1: *v1 |=  v2;  break;
	case 2: *v1 &= ~v2;  break;
	}
}

/* NOTE: because of how this is call from a macro on both 64 and 32 bit
   systems AND THE WAY IT IS CALLED FROM THE trace_cntl programg,
   all argument, EXCEPT for the "name", "file", and "lvl*" commands, should be
   64bits -- they can either be cast (uint64_t) or "LL" constants.
   See the trace_cntl.c tests for examples.
 */
static int traceCntl( int nargs, const char *cmd, ... )
{
	int      ret=0;
	va_list  ap;
	unsigned ii;
#if 0 && !defined(__KERNEL__)
	va_start( ap, cmd );
	for (ii=0; ii<nargs; ++ii) /* nargs is number of args AFTER cmd */
		printf("arg%u=0x%llx\n",ii+1,va_arg(ap,unsigned long long));
	va_end( ap );
#endif

	va_start( ap, cmd );

	/* although it may be counter intuitive, this should override
	   env.var as it could be used to set a file-per-thread.
	   NO!!!  -- I think env will over ride, this will just change
	   the default for name/file.
	   NOTE: CAN'T HAVE FILE-PER-THREAD unless traceControl_p,traceEntries_p,traceNamLvls_p are THREAD_LOCAL
	   CAN'T HAVE NAME-PER-THREAD unless traceTID       is THREAD_LOCAL
    */
#ifndef __KERNEL__
	if (strcmp(cmd,"file") == 0) {/* THIS really only makes sense for non-thread local-file-for-module or for non-static implementation w/TLS for file-per-thread */
		traceFile = va_arg(ap,char*);/* this can still be overridden by env.var.; suggest testing w. TRACE_ARGSMAX=10*/
		traceInit(TRACE_NAME);		/* will not RE-init as traceControl_p!=NULL skips mmap_file */
		va_end(ap); return (0);
    } else if (strcmp(cmd,"namlvlset") == 0) {
		/* use this if program sets TRACE_NAMLVLSET env.var.  This can be used
		   to Init or called trace_namLvlSet() after an Init has occurred. */
		const char *name=(nargs==0)?TRACE_NAME:va_arg(ap,char*); /* name is optional */
		/*printf("nargs=%d name=%s\n",nargs,name);*/
		if (traceControl_p == NULL)
			traceInit( name );	/* with traceControl_p==NULL. trace_namLvlSet() will be called */
		else
			trace_namLvlSet();  /* recall trace_namLvlSet(). optional name, if given, is ignored */
		va_end(ap); return (0);
	} else if (strcmp(cmd,"mapped") == 0) {
		TRACE_INIT_CHECK {};
		ret = (traceControl_p!=&traceControl[0]); /* compatible with define TRACE_CNTL(...) (0) */
		va_end(ap); return (ret);
	}
#endif
	TRACE_INIT_CHECK {};     /* note: allows name2TID to be called in userspace */

	if (strcmp(cmd,"name") == 0) {/* THIS really only makes sense for non-thread local-name-for-module or for non-static implementation w/TLS for name-per-thread */
		char *tnam= va_arg(ap,char*);/* this can still be overridden by env.var. IF traceInit(TRACE_NAME) is called; suggest testing w. TRACE_ARGSMAX=10*/
		traceName = tnam?tnam:TRACE_DFLT_NAME;/* doing it this way allows this to be called by kernel module */
		traceTID = name2TID( traceName );
	}
	else if (strncmp(cmd,"mode",4) == 0) { /* this returns the (prv/cur) mode requested */
		switch (cmd[4]) {
		case '\0':
			ret=traceControl_rwp->mode.mode;
			if (nargs==1) {
				uint32_t mode=va_arg(ap,uint64_t);
				union trace_mode_u tmp;
				tmp.mode = mode;
#ifndef __KERNEL__
				if (traceControl_p == &(traceControl[0])) tmp.bits.M=0;
#endif
				traceControl_rwp->mode = tmp;
			}
			break;
		case 'M':
			ret=traceControl_rwp->mode.bits.M;
#ifndef __KERNEL__
			if (traceControl_p == &(traceControl[0])) break;
#endif
			if (nargs==1) {
				uint32_t mode=va_arg(ap,uint64_t);
				traceControl_rwp->mode.bits.M = mode;
			}
			break;
		case 'S':
			ret=traceControl_rwp->mode.bits.S;
			if (nargs==1) {
				uint32_t mode=va_arg(ap,uint64_t);
				traceControl_rwp->mode.bits.S = mode;
			}
			break;
		default:
			ret=-1;
		}
	} else if (  (strncmp(cmd,"lvlmskn",7)==0)
	           ||(strncmp(cmd,"lvlsetn",7)==0)
	           ||(strncmp(cmd,"lvlclrn",7)==0) ) { /* TAKES 0, 1 or 3 args: lvlX or lvlM,lvlS,lvlT */
		uint64_t lvl, lvlm, lvls, lvlt;
		unsigned ee;
		int op, slen=strlen(&cmd[7]);
		char *name_spec;
		if (slen>1 || (slen==1&&!strpbrk(&cmd[6],"MST"))) {
			TRACE_PRINT("only M,S,or T allowed after lvl...\n");va_end(ap); return (-1);
		}
		
		if      (strncmp(&cmd[3],"msk",3) == 0) op=0;
		else if (strncmp(&cmd[3],"set",3) == 0) op=1;
		else                                    op=2;
		name_spec = va_arg(ap,char*);
		/* find first match */
		ee = traceControl_p->num_namLvlTblEnts;
		for (ii=0; ii<ee; ++ii) {
			if (traceNamLvls_p[ii].name[0]
			    &&TMATCHCMP(name_spec,traceNamLvls_p[ii].name) )
				break;
		}
		if (ii==ee)
			return (0);
		lvl = va_arg(ap,uint64_t);
		switch (cmd[7]) {
		case 'M':
			ret = traceNamLvls_p[ii].M;
			for ( ; ii<ee; ++ii)
				if(traceNamLvls_p[ii].name[0]&&TMATCHCMP(name_spec,traceNamLvls_p[ii].name))
					trace_msk_op( &traceNamLvls_p[ii].M,op,lvl );
			break;
		case 'S':
			ret = traceNamLvls_p[ii].S;
			for ( ; ii<ee; ++ii)
				if(traceNamLvls_p[ii].name[0]&&TMATCHCMP(name_spec,traceNamLvls_p[ii].name))
					trace_msk_op( &traceNamLvls_p[ii].S,op,lvl );
			break;
		case 'T':
			ret = traceNamLvls_p[ii].T;
			for ( ; ii<ee; ++ii)
				if(traceNamLvls_p[ii].name[0]&&TMATCHCMP(name_spec,traceNamLvls_p[ii].name))
					trace_msk_op( &traceNamLvls_p[ii].T,op,lvl );
			break;
		default:
			if (nargs != 4) { /* "name" plus 3 lvls */
				TRACE_PRINT("need 3 lvlmsks; %d given\n",nargs-1);va_end(ap); return (-1);
			}
			lvlm=lvl; /* arg from above */
			lvls=va_arg(ap,uint64_t);
			lvlt=va_arg(ap,uint64_t);
			for ( ; ii<ee; ++ii)
				if(traceNamLvls_p[ii].name[0]&&TMATCHCMP(name_spec,traceNamLvls_p[ii].name)) {
					trace_msk_op( &traceNamLvls_p[ii].M,op,lvlm );
					trace_msk_op( &traceNamLvls_p[ii].S,op,lvls );
					trace_msk_op( &traceNamLvls_p[ii].T,op,lvlt );
				}
		}
	} else if (  (strncmp(cmd,"lvlmsk",6)==0)
	           ||(strncmp(cmd,"lvlset",6)==0)
	           ||(strncmp(cmd,"lvlclr",6)==0) ) { /* TAKES 0, 1 or 3 args: lvlX or lvlM,lvlS,lvlT */
		uint64_t lvl, lvlm, lvls, lvlt;
		unsigned ee, doNew=1, op;
		if      (strncmp(&cmd[3],"msk",3) == 0) op=0;
		else if (strncmp(&cmd[3],"set",3) == 0) op=1;
		else                                    op=2;
		if ((cmd[6]=='g')||((cmd[6])&&(cmd[7]=='g'))) {
			ii=0;        ee=traceControl_p->num_namLvlTblEnts;
		} else if ((cmd[6]=='G')||((cmd[6])&&(cmd[7]=='G'))) {
			ii=0;        ee=traceControl_p->num_namLvlTblEnts;
			doNew=0;  /* Capital G short ciruits the "set for future/new trace ids */
		} else {
			ii=traceTID; ee=traceTID+1;
			switch (cmd[6]) {
			case 'M': ret = traceNamLvls_p[ii].M; break;
			case 'S': ret = traceNamLvls_p[ii].S; break;
			case 'T': ret = traceNamLvls_p[ii].T; break;
			}
		}
		lvl=va_arg(ap,uint64_t); /* "FIRST" ARG SHOULD ALWAYS BE THERE */
		switch (cmd[6]) {
		case 'M': for ( ; ii<ee; ++ii) if(doNew||traceNamLvls_p[ii].name[0])
										   trace_msk_op( &traceNamLvls_p[ii].M,op,lvl );
			break;
		case 'S': for ( ; ii<ee; ++ii) if(doNew||traceNamLvls_p[ii].name[0])
										   trace_msk_op( &traceNamLvls_p[ii].S,op,lvl );
			break;
		case 'T': for ( ; ii<ee; ++ii) if(doNew||traceNamLvls_p[ii].name[0])
										   trace_msk_op( &traceNamLvls_p[ii].T,op,lvl );
			break;
		default:
			if (nargs != 3) {
				TRACE_PRINT("need 3 lvlmsks; %d given\n",nargs);va_end(ap); return (-1);
			}
			lvlm=lvl; /* "FIRST" arg from above */
			lvls=va_arg(ap,uint64_t);
			lvlt=va_arg(ap,uint64_t);
			for ( ; ii<ee; ++ii) {
				if(!doNew&&!traceNamLvls_p[ii].name[0]) continue;
				trace_msk_op( &traceNamLvls_p[ii].M,op,lvlm );
				trace_msk_op( &traceNamLvls_p[ii].S,op,lvls );
				trace_msk_op( &traceNamLvls_p[ii].T,op,lvlt );
			}
		}
	} else if (strcmp(cmd,"trig") == 0) { /* takes 2 args: lvlsMsk, postEntries - optional 3rd arg will suppress warnings */
		uint64_t lvlsMsk=va_arg(ap,uint64_t);
		unsigned post_entries=va_arg(ap,uint64_t);
		if ((traceNamLvls_p[traceTID].M&lvlsMsk) != lvlsMsk) {
#  ifndef __KERNEL__
			if(nargs==2)
				fprintf(stderr, "Warning: \"trig\" setting (additional) bits (0x%llx) in traceTID=%d\n", (unsigned long long)lvlsMsk, traceTID );
#  endif
			traceNamLvls_p[traceTID].M |= lvlsMsk;
		}
#  ifndef __KERNEL__
		if (traceControl_rwp->trigActivePost && nargs==2)
			fprintf(stderr, "Warning: \"trig\" overwriting trigActivePost (previous=%d)\n", traceControl_rwp->trigActivePost );
#  endif
		traceNamLvls_p[traceTID].T       = lvlsMsk;
		traceControl_rwp->trigActivePost = post_entries?post_entries:1; /* must be at least 1 */
		traceControl_rwp->triggered      = 0;
		traceControl_rwp->trigIdxCnt     = 0;
	} else if (strcmp(cmd,"reset") == 0) {
		traceControl_rwp->full
			= traceControl_rwp->trigIdxCnt
			= traceControl_rwp->trigActivePost
			= 0;
		TRACE_ATOMIC_STORE( &traceControl_rwp->wrIdxCnt, (uint32_t)0 );
		traceControl_rwp->triggered = 0;
	} else if (strcmp(cmd,"limit_ms") == 0) { /* 2 or 3 args: limit_cnt, span_on_ms, [span_off_ms] */
		if (nargs == 0) {
			ret = traceControl_rwp->limit_cnt_limit;
		} else if (nargs>=2 && nargs<=3) {
			ret = traceControl_rwp->limit_cnt_limit;
			traceControl_rwp->limit_cnt_limit  = va_arg(ap,uint64_t);
			traceControl_rwp->limit_span_on_ms = va_arg(ap,uint64_t);
			if (nargs == 3)
				traceControl_rwp->limit_span_off_ms = va_arg(ap,uint64_t);
			else
				traceControl_rwp->limit_span_off_ms = traceControl_rwp->limit_span_on_ms;
		} else {
			TRACE_PRINT("limit needs 0 or 2 or 3 args (cnt,span_of[,span_off]) %d given\n",nargs);va_end(ap); return (-1);
		}
	} else {
		ret = -1;
	}
	va_end(ap);
#ifdef __KERNEL__
	if (ret==-1) printk(  KERN_ERR "TRACE: invalid control string %s nargs=%d\n", cmd, nargs );
#else
	if (ret==-1) fprintf( stderr, "TRACE: invalid control string %s nargs=%d\n", cmd, nargs );
#endif
	return (ret);
}   /* traceCntl */


#if !defined(__KERNEL__) || defined(TRACE_IMPL)

static void trace_created_init(  struct traceControl_s *t_p
                               , struct traceControl_rw *t_rwp
                               , uint32_t msgmax,  uint32_t argsmax
                               , uint32_t numents, uint32_t namtblents
                               , int      memlen,  unsigned modeM )
{
    struct timeval tv;
    TRACE_GETTIMEOFDAY( &tv );
# ifdef TRACE_DEBUG_INIT
    trace_user( &tv,0,0,1 TRACE_XTRA_PASSED,"trace_created_init: tC_p=%p",t_p );
# endif
    strncpy( t_p->version_string, TRACE_REV, sizeof(t_p->version_string) );
    t_p->version_string[sizeof(t_p->version_string)-1] = '\0';
    t_p->create_tv_sec       = (uint32_t)tv.tv_sec;
    t_p->num_params          = argsmax;
    t_p->siz_msg             = msgmax;
    t_p->siz_entry           = entSiz( msgmax, argsmax );
    t_p->num_entries         = numents;
    t_p->largest_multiple    = (uint32_t)-1 - ((uint32_t)-1 % numents);
    t_p->largest_zero_offset = ((uint32_t)-1 % numents) +1;
    t_p->num_namLvlTblEnts   = namtblents;
    t_p->memlen              = memlen;

    TRACE_ATOMIC_STORE( &t_rwp->namelock, (uint32_t)0 );
    
    /*TRACE_CNTL( "reset" );  Can't call traceCntl during Init b/c it does an INIT_CHECK and will call Init */
    TRACE_ATOMIC_STORE( &t_rwp->wrIdxCnt, (uint32_t)0 );
    t_rwp->full = t_rwp->trigIdxCnt = t_rwp->trigActivePost = t_rwp->triggered = 0;
	t_rwp->limit_span_on_ms = t_rwp->limit_span_off_ms = t_rwp->limit_cnt_limit = 0;

    t_rwp->mode.mode           = 0;
    t_rwp->mode.bits.M         = modeM;
    t_rwp->mode.bits.S         = 1;

    traceInitNames( t_p, t_rwp ); /* THIS SETS GLOBAL traceNamLvls_p REFERENCED NEXT. */

    /* this depends on the actual value of the num_namLvlTblEnts which
       may be different from the "calculated" value WHEN the buffer has
       previously been configured */
    traceEntries_p = (struct traceEntryHdr_s *)				\
	((unsigned long)traceNamLvls_p+namtblSiz(t_p->num_namLvlTblEnts));

    t_p->trace_initialized = 1;
# ifdef TRACE_DEBUG_INIT
    tv.tv_sec=0;
    trace_user( &tv,0,0,1 TRACE_XTRA_PASSED,"trace_created_init: tC_p=%p",t_p );
# endif
}   /* trace_created_init */

#endif

#ifndef __KERNEL__

/* This is currently (as of Nov, 2017) used to build a file name from
   TRACE_FILE. Currently not applicable for __KERNEL__ (module or in-source)
   which always creates the virtual file at /proc/trace/buffer.
 */
static void tsnprintf( char *obuf, size_t bsz, const char *input )
{
	size_t       outoff, ii;
	const char  *inp=input;
	char         loguid[15];
	char        *cp_uname=NULL, *cp_uid=NULL;

	for (outoff=0; outoff<bsz && *inp!='\0'; ++inp) {
		if (*inp == '%') {
			++inp;
			switch (*inp) {
			case '%':
				obuf[outoff++] = *inp;
				break;
			case 'u':
				if (    cp_uname==NULL
				    && (cp_uname=getenv("LOGNAME"))==NULL
				    && (cp_uname=getenv("USERNAME"))==NULL
				    && (cp_uname=getenv("USER"))==NULL )
					cp_uname=(char*)"";
				for (ii=0; outoff<bsz && cp_uname[ii]!='\0'; ++ii)
					obuf[outoff++] = cp_uname[ii];
				break;
			case 'U':
				if (cp_uid==NULL) {
					sprintf( loguid, "%u", getuid() );	
					cp_uid = loguid;
				}
				for (ii=0; outoff<bsz && cp_uid[ii]!='\0'; ++ii)
					obuf[outoff++] = cp_uid[ii];
				break;
			case '\0':
				obuf[outoff++] = '%';
				--inp; /* let for loop exit test see this */
				break;
			default:
				/* just put them both out if there is space in obuf */
				obuf[outoff++] = '%';
				if (outoff<bsz)
					obuf[outoff++] = *inp;
			}
		} else
			obuf[outoff++] = *inp;
	}
	if (outoff>=bsz)
		obuf[bsz-1] = '\0';
	else
		obuf[outoff] = '\0';
}   /* tsnprintf */


/* RETURN "created" status */
static int trace_mmap_file( const char *_file
                           , int       *memlen   /* in/out -- in for when file created, out when not */
                           , struct traceControl_s **tC_p
                           , struct traceControl_rw **tC_rwp /* out */
                           , uint32_t msgmax, uint32_t argsmax, uint32_t numents, uint32_t namtblents  /* all in for when file created */
                           )
{
    int                    fd;
    struct traceControl_s  *controlFirstPage_p=NULL;
	struct traceControl_rw *rw_rwp;
    off_t                  off;
    char                   path[PATH_MAX];
    int			   created=0;
    int			   stat_try=0;
	int            quiet_warn=0;

	tsnprintf( path, PATH_MAX, _file );
    if ((fd=open(path,O_RDWR|O_CREAT|O_EXCL,0666)) != -1)
    {   /* successfully created new file - must init */
		uint8_t one_byte='\0';
		off = lseek( fd, (*memlen)-1, SEEK_SET );
		if (off == (off_t)-1) { perror("lseek"); *tC_p=&(traceControl[0]);return (0); }
		quiet_warn += write( fd, &one_byte, 1 );
		created = 1;
    }
    else
    {   /* There's an existing file... map 1st page ro */
		struct stat           statbuf;
		int                   try_=3000;
		/* must verify that it already exists */
		fd=open(path,O_RDWR);
		if (fd == -1)
		{   fprintf( stderr,"TRACE: open(%s)=%d errno=%d pid=%d\n", path, fd, errno, tracePid );
			*tC_p=&(traceControl[0]);
			*tC_rwp=&(traceControl[0].rw);
			return (0);
		}
		/*printf( "trace_mmap_file - fd=%d\n",fd );*/ /*interesting in multithreaded env.*/
		if (fstat(fd,&statbuf) == -1)
		{   perror("fstat");
			close( fd );
			*tC_p=&(traceControl[0]);
			*tC_rwp=&(traceControl[0].rw);
			return (0);
		}
		while (statbuf.st_size < (off_t)sizeof(struct traceControl_s))
		{   fprintf(stderr,"stat again\n");
			if (   ((stat_try++ >= 30)         && (fprintf(stderr,"too many stat tries\n"),1))
			    || ((fstat(fd,&statbuf) == -1) && (perror("fstat"),1)) )
			{   close( fd );
				*tC_p=&(traceControl[0]);
				*tC_rwp=&(traceControl[0].rw);
				return (0);
			}
		}

		controlFirstPage_p = (struct traceControl_s *)mmap( NULL, TRACE_PAGESIZE
		                                                   , PROT_READ
		                                                   , MAP_SHARED, fd, 0 );
		if (controlFirstPage_p == (struct traceControl_s *)-1)
		{   perror( "mmap(NULL,TRACE_PAGESIZE,PROT_READ,MAP_SHARED,fd,0) error" );
			*tC_p=&(traceControl[0]);
			*tC_rwp=&(traceControl[0].rw);
			return (0);
		}
		while (try_--)
			if (controlFirstPage_p->trace_initialized != 1) {
				if(try_==0) {
					printf("Trace file not initialzed; consider (re)moving it.\n");
					close( fd );
					*tC_p=&(traceControl[0]);
					*tC_rwp=&(traceControl[0].rw);
					return (0);
				}
				if(try_==1) sleep(1); /* just sleep a crazy amount the last time */
			}
			else break;
		/*sleep(1);*/
		*memlen = controlFirstPage_p->memlen;
    }


    /* ??? OLD:I MUST allocate/grab a contiguous vm address space! [in testing threads (where address space
       is shared (obviously)), thread creation allocates vm space which can occur between
       these two calls]
	   ???
	   NEW: mmap from rw portion on */
    rw_rwp = (struct traceControl_rw*)mmap( NULL,(*memlen)-TRACE_PAGESIZE
	                                       ,PROT_READ|PROT_WRITE,MAP_SHARED
	                                       ,fd,TRACE_PAGESIZE );
    if (rw_rwp == (void *)-1)
    {   perror("Error:mmap(NULL,(*memlen)-TRACE_PAGESIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,TRACE_PAGESIZE)");
		printf( "(*memlen)=%d errno=%d\n", (*memlen)-TRACE_PAGESIZE, errno );
		close( fd );
		*tC_p=&(traceControl[0]);
		*tC_rwp=&(traceControl[0].rw);
		return (0);
    }

    if (created) {
		/* need controlFirstPage_p RW temporarily */
		controlFirstPage_p = (struct traceControl_s *)mmap( NULL, TRACE_PAGESIZE
		                                                   , PROT_READ|PROT_WRITE
		                                                   , MAP_SHARED, fd, 0 );
		if (controlFirstPage_p == (struct traceControl_s *)-1) {
			perror( "mmap(NULL,sizeof(struct traceControl_s),PROT_READ,MAP_SHARED,fd,0) error" );
			munmap( rw_rwp, (*memlen )-TRACE_PAGESIZE);
			close( fd );
			*tC_p=&(traceControl[0]);
			*tC_rwp=&(traceControl[0].rw);
			return (0);
		}
		trace_created_init( controlFirstPage_p, rw_rwp, msgmax, argsmax, numents, namtblents, *memlen, 1 );
		/* Now make first page RO */
		munmap( controlFirstPage_p, TRACE_PAGESIZE );
#      define MM_FLAGS MAP_SHARED/*|MAP_FIXED*/
		controlFirstPage_p = (struct traceControl_s *)mmap( NULL, TRACE_PAGESIZE
		                                                   , PROT_READ
		                                                   , MM_FLAGS, fd, 0 );
		if (controlFirstPage_p == (struct traceControl_s *)-1) {
			perror( "Error:mmap(NULL,TRACE_PAGESIZE,PROT_READ," MM_STR(MM_FLAGS) ",fd,0)");
			printf( "(*memlen)=%d errno=%d\n", (*memlen), errno );
			munmap( rw_rwp, (*memlen)-TRACE_PAGESIZE );
			close( fd );
			*tC_p=&(traceControl[0]);
			*tC_rwp=&(traceControl[0].rw);
			return (0);
		}
	}

	traceNamLvls_p = (struct traceNamLvls_s *)(rw_rwp+1);
	traceEntries_p = (struct traceEntryHdr_s *)							\
		((unsigned long)traceNamLvls_p+namtblSiz(controlFirstPage_p->num_namLvlTblEnts));

	*tC_rwp = rw_rwp;
    *tC_p = controlFirstPage_p;

    /* The POSIX mmap man page says:
       The mmap() function shall add an extra reference to the file
       associated with the file descriptor fildes which is not removed by a
       subsequent  close() on that file descriptor.  This reference shall
       be removed when there are no more mappings to the file.
    */
    close( fd );
    return (created);
}   /* trace_mmap_file */

#endif	/* not __KERNEL__*/


#if !defined(__KERNEL__) || defined(TRACE_IMPL)

static int traceInit( const char *_name )
{
    int         memlen;
    uint32_t    msgmax_, argsmax_, numents_, namtblents_;
# ifndef __KERNEL__
    int         activate=0;
    const char *_file;
    const char *cp;

    if (!trace_lock( &traceInitLck ))
		TRACE_PRINT("trace_lock: InitLck hung?\n");
#  ifdef TRACE_DEBUG_INIT
    printf("traceInit(debug:A): tC_p=%p static=%p _name=%p Tid=%d TrcId=%d\n",traceControl_p,traceControl_p_static,_name,traceTid,traceTID);
#  endif
    if (traceControl_p == NULL) {
		/* This stuff should happen once (per PROCESS) */

		/* test for activation. (See below for _name override/default) */
		if (_name != NULL) {
			/* name is specified in module, which "wins" over env, but does not "activate" */
			if (getenv("TRACE_NAME")) activate=1;
		}
		else if ((_name=getenv("TRACE_NAME")) && (*_name!='\0')) {
			/* name not specified in module, nor is a non-"" specified in env */
			activate = 1;
		}

		if (!((_file=getenv("TRACE_FILE"))&&(*_file!='\0')&&(activate=1))) _file=traceFile;
		if ((cp=getenv("TRACE_ARGSMAX"))  &&(*cp)&&(activate=1)) argsmax_=strtoul(cp,NULL,0);else argsmax_   =TRACE_DFLT_MAX_PARAMS;
		/* use _MSGMAX= so exe won't override and _MSGMAX won't activate; use _MSGMAX=0 to activate with default MAX_MSG */
		((cp=getenv("TRACE_MSGMAX"))    &&(*cp)&&(activate=1)&&(msgmax_     =strtoul(cp,NULL,0)))||(msgmax_    =TRACE_DFLT_MAX_MSG_SZ);
		((cp=getenv("TRACE_NUMENTS"))   &&       (numents_    = strtoul(cp,NULL,0))&&(activate=1))||(numents_   =TRACE_DFLT_NUM_ENTRIES);
		((cp=getenv("TRACE_NAMTBLENTS"))&&       (namtblents_ = strtoul(cp,NULL,0))&&(activate=1))||(namtblents_=TRACE_DFLT_NAMTBL_ENTS);
	    ((cp=getenv("TRACE_LVLM"))      &&       (trace_lvlM  =strtoull(cp,NULL,0))&&(activate=1)); /* activate if non-zero */

		/* TRACE_LVLS and TRACE_PRINT_FD can be used when active or inactive */
		if ((cp=getenv("TRACE_PRINT_FD")) && (*cp)) tracePrintFd=strtoul(cp,NULL,0);

		if (!activate) {
			traceControl_rwp=&(traceControl[0].rw);
			traceControl_p=&(traceControl[0]);
		} else {
			if (namtblents_ == 1) namtblents_ = 2; /* If it has been specified in the env. it should be at least 2 */
			memlen = traceMemLen( cntlPagesSiz(), namtblents_, msgmax_, argsmax_, numents_ );
			if ((traceControl_p_static!=NULL) && (strcmp(traceFile_static,_file)==0))
				traceControl_p=traceControl_p_static;
			else
				trace_mmap_file( _file, &memlen, &traceControl_p, &traceControl_rwp, msgmax_, argsmax_, numents_, namtblents_ );
		}

		/* trace_mmap_file may have failed */
		if (traceControl_p == &(traceControl[0]))
		{
#       define DISABLED_ENTS 1
			trace_created_init( traceControl_p, traceControl_rwp
			                   , msgmax_, argsmax_
			                   , DISABLED_ENTS /*numents_*/
			                   , ((sizeof(traceControl)-sizeof(traceControl[0])
							       -DISABLED_ENTS*entSiz(msgmax_,argsmax_))
							      /sizeof(struct traceNamLvls_s)) /*namtblents_*/
			                   , sizeof(traceControl) /*memlen*/
			                   , 0 /*modeM*/ );
		}
		else 
		{
			if (traceControl_p_static == NULL)
			{
				strcpy( traceFile_static, _file );
				traceControl_p_static = traceControl_p;
			}
		}

		trace_namLvlSet();
    }

    if (_name == NULL) if (!((_name=getenv("TRACE_NAME"))&&(*_name!='\0'))) _name=traceName;

    traceTID = name2TID( _name );
    /* Now that the critical variables
       (traceControl_p, traceNamLvls_p, traceEntries_p) and even traceTID are
       set, it's OK to indicate that the initialization is complete */
    if(traceTid==0) /* traceInit may be called w/ or w/o checking traceTid */
    {   tracePid = getpid();  /* do/re-do -- it may be forked process */
#  if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#  endif
		traceTid=trace_gettid();
#  if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#   pragma GCC diagnostic pop
#  endif
    }
#  ifdef TRACE_DEBUG_INIT
    printf("traceInit(debug:Z): tC_p=%p static=%p _name=%p Tid=%d TrcId=%d\n",traceControl_p,traceControl_p_static,_name,traceTid,traceTID);
#  endif
    trace_unlock( &traceInitLck );

    if ((cp=getenv("TRACE_LVLS"))     && (*cp)) {
		TRACE_CNTL( "lvlmskSg", strtoull(cp,NULL,0) );  /* set for all current and future (until cmd line tonSg or toffSg) */
	    trace_lvlS = strtoull(cp,NULL,0);              /* set for future new traceTIDs (from this process) regardless of cmd line tonSg or toffSg - if non-zero! */
    }
	if (trace_lvlM)
		TRACE_CNTL( "lvlmskMg", trace_lvlM ); /* all current and future (until cmdline tonMg/toffMg) (and new from this process regardless of cmd line tonSg or toffSg) */

# else  /* ifndef __KERNEL__ */

    msgmax_     =msgmax;	 /* module_param */
    argsmax_    =argsmax;	 /* module_param */
    numents_    =numents;	 /* module_param */
    namtblents_ =namtblents; /* module_param */
    printk("numents_=%d msgmax_=%d argsmax_=%d namtblents_=%d\n"
	   ,numents_,   msgmax_,   argsmax_,   namtblents_ );
    memlen = traceMemLen( cntlPagesSiz(), namtblents_, msgmax_, argsmax_, numents_ );
    traceControl_p = (struct traceControl_s *)vmalloc_node( memlen, trace_buffer_numa_node );
	traceControl_rwp = (struct traceControl_rw *)((unsigned long)traceControl_p+TRACE_PAGESIZE);
    trace_created_init( traceControl_p, traceControl_rwp, msgmax_, argsmax_, numents_, namtblents_, memlen, 1 );
    if (_name == NULL) _name=traceName;
    traceTID = name2TID( _name );
# endif

    return (0);
}   /* traceInit */


static void traceInitNames( struct traceControl_s *tC_p, struct traceControl_rw *tC_rwp )
{
    unsigned ii;
    traceNamLvls_p = (struct traceNamLvls_s *)(tC_rwp+1);
    for (ii=0; ii<tC_p->num_namLvlTblEnts; ++ii)
    {   traceNamLvls_p[ii].name[0] = '\0';
		traceNamLvls_p[ii].M = 0xf; /* As Name/TIDs can't go away, these are */
		traceNamLvls_p[ii].S = 0x7; /* then defaults except for trace_lvlS/trace_lvlM */
		traceNamLvls_p[ii].T = 0;   /* in name2TID. */
    }								/* (0 for err, 1=warn, 2=info, 3=debug) */
# ifdef __KERNEL__
    strcpy( traceNamLvls_p[0].name,"KERNEL" );
	/* like userspace TRACE_LVLS env.var - See also name2TID */
	if(trace_lvlS)
		traceNamLvls_p[0].S = trace_lvlS;
	if(trace_lvlM)
		traceNamLvls_p[0].M = trace_lvlM;
# endif
    strcpy( traceNamLvls_p[tC_p->num_namLvlTblEnts-2].name,"TRACE" );
    strcpy( traceNamLvls_p[tC_p->num_namLvlTblEnts-1].name,"_TRACE_" );
	tC_rwp->longest_name = 7;
}   /* traceInitNames */

#endif /* !defined(__KERNEL__) || defined(TRACE_IMPL) */


static struct traceEntryHdr_s* idxCnt2entPtr( uint32_t idxCnt )
{   uint32_t idx;
    off_t    off;
    uint32_t num_entries=traceControl_p->num_entries;
    idx = idxCnt % num_entries;
    off = idx * traceControl_p->siz_entry;
    return (struct traceEntryHdr_s *)((char*)traceEntries_p+off);
}   /* idxCnt2entPtr */


#ifdef __cplusplus

/* The Streamer instance will be temporary. No class statics can be used --
   The goal is statics for each TRACE/TLOG and a class static won't do.
   A lambda expression can by used to hold/create statics for each TLOG.
   "static int tid_" is thread safe in so far as multiple threads may init,
   but will init with same value.
*/

# if   (__cplusplus >= 201103L)

#  define TRACE_STATIC_TID_ENABLED(name,lvl,s_enbld,s_frc,mp,sp,tvp,ins,ins_sz) \
	[](const char* nn,int lvl_,int s_enabled_,int s_frc_,int*do_m_,int*do_s_,timeval*tvp_,char*ins_,size_t sz){ \
		TRACE_INIT_CHECK {				\
			static TRACE_THREAD_LOCAL int tid_=-1;if(tid_==-1){tid_=name2TID(nn[0]?nn:TRACE_NAME);} \
			static TRACE_THREAD_LOCAL limit_info_t _info;				\
			*do_m_ = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl_)); \
			*do_s_ = (   s_enabled_										\
			          && ( s_frc_ ||(traceControl_rwp->mode.bits.S&&(traceNamLvls_p[tid_].S&TLVLMSK(lvl_))) ) \
			          && limit_do_print(tvp_,&_info,ins_,sz) );			\
			return (*do_m_||*do_s_)?tid_:-1;								\
		} else							\
		  return -1;						\
	}(name,lvl,s_enbld,s_frc,mp,sp,tvp,ins,ins_sz)

# else 

// Note: the s_enbld and s_frc args are used directly in the macro definition
#  define TRACE_STATIC_TID_ENABLED(name,lvl,s_enbld,s_frc,mp,sp,tvp,ins,ins_sz) \
	({const char* nn=name;int lvl_=lvl,*do_m_=mp,*do_s_=sp;char*ins_=ins;size_t sz=ins_sz; \
		int tid__;														\
		TRACE_INIT_CHECK {												\
			static TRACE_THREAD_LOCAL int tid_=-1;if(tid_==-1){tid_=name2TID(nn[0]?nn:TRACE_NAME);} \
			static TRACE_THREAD_LOCAL limit_info_t _info;				\
			*do_m_ = traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl_)); \
			*do_s_ = (   s_enbld										\
			          && ( s_frc ||(traceControl_rwp->mode.bits.S&&(traceNamLvls_p[tid_].S&TLVLMSK(lvl_))) ) \
			          && limit_do_print(tvp,&_info,ins_,sz) );			\
			tid__ = (*do_m_||*do_s_)?tid_:-1;							\
		} else															\
			  tid__ = -1;												\
		tid__;															\
	})

# endif

// Use C++ "for" statement to create single statement scope for key (static) variable that
// are initialized and then, if enabled, passed to the Streamer class temporary instances.
// s_enabled = is_slowpath_enabled; fmtnow = force formating (i.e. if Memory only)
#define TRACE_STREAMER(lvl, nam_or_fmt,fmt_or_nam,s_enabled,force_s)			\
	for( int tid=-1,do__m=0,do__s=0,fmtnow,ins[32/sizeof(int)], tv[sizeof(timeval)/sizeof(int)]={0}; \
		 (tid==-1) && ((tid=TRACE_STATIC_TID_ENABLED(t_arg_nmft(nam_or_fmt,fmt_or_nam,&fmtnow),lvl,s_enabled,force_s \
		                                             ,&do__m,&do__s,(timeval*)&tv,(char*)ins,sizeof(ins)))!=-1); \
	    ) TraceStreamer{}.init( tid, lvl, do__m, do__s, fmtnow, (timeval*)&tv, (char*)ins )

#define TRACE_ENDL ""
#define TLOG_ENDL TRACE_ENDL

// This will help devleper who use too many TLOG_INFO/TLOG_DEBUG
// to use more TLOG{,_TRACE,_DBG,_ARB} (use more levels)
#ifdef __OPTIMIZE__
# define DEBUG_FORCED 0
#else
# define DEBUG_FORCED 1
#endif

static inline const char* t_arg_nmft  ( const char* nm,       int fmtnow, int *fmtret ) { *fmtret=fmtnow; return nm;       }
static inline const char* t_arg_nmft  ( const std::string&nm, int fmtnow, int *fmtret ) { *fmtret=fmtnow; return nm.c_str();  }
static inline const char* t_arg_nmft  ( int fmtnow, const char* nm,       int *fmtret ) { *fmtret=fmtnow; return nm?nm:""; } // could be addr 0 (null)
static inline const char* t_arg_nmft  ( int fmtnow, const std::string&nm, int *fmtret ) { *fmtret=fmtnow; return nm.c_str(); }
static inline const char* t_arg_nmft  ( int fmtnow, int nm __attribute__((__unused__)), int *fmtret ) { *fmtret=fmtnow; return ""; }

# define tlog_LVL( a1,...)        a1
# define tlog_ARG2( a1,a2,...)    a2
# define tlog_ARG3( a1,a2,a3,...) a3
#define TLVL_ERROR        0
#define TLVL_WARNING      1
#define TLVL_INFO         2
#define TLVL_DEBUG        3
#define TLVL_TRACE        4
//                                 args are: lvl,         name, fmtnow, s_enabled, s_force, 
#define TLOG_ERROR(name)   TRACE_STREAMER( TLVL_ERROR,  &(name)[0], 0,1,0 )
#define TLOG_WARNING(name) TRACE_STREAMER( TLVL_WARNING,&(name)[0], 0,1,0 )
#define TLOG_INFO(name)    TRACE_STREAMER( TLVL_INFO,   &(name)[0], 0,1,0 )
#define TLOG_DEBUG(name)   TRACE_STREAMER( TLVL_DEBUG,  &(name)[0], 0,1,0 )
#define TLOG_TRACE(name)   TRACE_STREAMER( TLVL_TRACE,  &(name)[0], 0,1,0 )
// For the following, the 1st arg must be lvl. 
#define TLOG_DBG(...)      TRACE_STREAMER( tlog_LVL(__VA_ARGS__,need_at_least_one), tlog_ARG2(__VA_ARGS__,0,need_at_least_one) \
										  ,tlog_ARG3(__VA_ARGS__,0,"",need_at_least_one), 1,0 )
#define TLOG_ARB(...)      TRACE_STREAMER( tlog_LVL(__VA_ARGS__,need_at_least_one), tlog_ARG2(__VA_ARGS__,0,need_at_least_one) \
										  ,tlog_ARG3(__VA_ARGS__,0,"",need_at_least_one), 1,0 )
#define TLOG(...)          TRACE_STREAMER( tlog_LVL( __VA_ARGS__,need_at_least_one),tlog_ARG2(__VA_ARGS__,0,need_at_least_one) \
										  ,tlog_ARG3(__VA_ARGS__,0,"",need_at_least_one), 1,0 )
#define TRACE_STREAMER_ARGSMAX 35
#define TRACE_STREAMER_TEMPLATE 1
#define TRACE_STREAMER_EXPAND(args) args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9] \
                                   ,args[10],args[11],args[12],args[13],args[14],args[15],args[16],args[17],args[18],args[19] \
								   ,args[20],args[21],args[22],args[23],args[24],args[25],args[26],args[27],args[28],args[29] \
                                   ,args[30],args[31],args[32],args[33],args[34]

#define TraceMin(a,b) (((a)<(b))?(a):(b))

namespace {  // unnamed namespace (i.e. static (for each compliation unit only))

struct TraceStreamer : std::ios {
	union arg {
		int i;
		double d;
		long unsigned int u;
		long int l;
		void* p;
	};
	char   msg[TRACE_STREAMER_MSGMAX];
	size_t msg_sz;
	arg    args[TRACE_STREAMER_ARGSMAX];
	size_t argCount;
	int tid_;
	int lvl_;
	bool do_s, do_m, do_f;
	char widthStr[16];
	char precisionStr[16];
	char fmtbuf[32];			// buffer for fmt (e.g. "%022.12llx")
	struct timeval *lclTime_p;
	const char *ins_;
public:

	explicit TraceStreamer() : msg_sz(0),argCount(0)
	{
#      ifdef TRACE_STREAMER_DEBUG
		std::cout << "TraceStreamer CONSTRUCTOR\n";
#      endif
        std::ios::init(0);
	}

	inline ~TraceStreamer() {
		str();
	}

	// use this method to return a reference (to the temporary, in its intended use)
	inline TraceStreamer& init(int tid, int lvl, bool dom, bool dos, int force_f, timeval *tvp, const char *ins)
	{	widthStr[0] = precisionStr[0] = msg[0] = '\0'; msg_sz=0;
		argCount = 0;
		tid_ = tid;
		lvl_ = lvl;
		do_m = dom;
		do_s = dos;
		do_f = (force_f==-1) ? 0 : (dos || force_f);
		ins_ = ins;
		lclTime_p = tvp;
		return *this;
	}

	inline void str()
	{
#      ifdef TRACE_STREAMER_DEBUG
		std::cout << "Message is " << msg << std::endl;
#      endif
		while(msg_sz && msg[msg_sz-1]=='\n') {
			msg[msg_sz-1]='\0';
			--msg_sz;
		}
#     if (defined(__cplusplus)&&(__cplusplus>=201103L))
#      pragma GCC diagnostic push
#      pragma GCC diagnostic ignored "-Wformat-security"
#     endif
		if (do_f) {
			if (do_m) trace(             lclTime_p, tid_, lvl_,       0 TRACE_XTRA_PASSED, msg );
			if (do_s) TRACE_LOG_FUNCTION(lclTime_p, tid_, lvl_, ins_, 0, msg );
		} else {
			if (do_m) trace( lclTime_p, tid_, lvl_, argCount TRACE_XTRA_PASSED, msg, TRACE_STREAMER_EXPAND(args));
			if (do_s) TRACE_LOG_FUNCTION(lclTime_p, tid_, lvl_, ins_, argCount, msg, TRACE_STREAMER_EXPAND(args));
		}
#     if (defined(__cplusplus)&&(__cplusplus>=201103L))
#      pragma GCC diagnostic pop
#     endif
	}

	inline void msg_append( const char *src )
	{	size_t add = TraceMin(strlen(src),sizeof(msg)-1-msg_sz);
		strncpy( &msg[msg_sz],src,add);
		msg_sz+=add;
		msg[msg_sz]='\0';
	}

	// assum fmtbuf is big enough
	inline char *format(bool isFloat, bool isUnsigned, const char *length, std::ios::fmtflags flags)
	{	//See, for example: http://www.cplusplus.com/reference/cstdio/printf/
		size_t oo=0;
		fmtbuf[oo++] = '%';

		// Flags
		if (flags & left)                   fmtbuf[oo++] = '-';
		if (flags & showpos)                fmtbuf[oo++] = '+';
		if (flags & (showpoint | showbase)) fmtbuf[oo++] = '#';  // INCLUSIVE OR

#     define APPEND( ss ) strcpy( &fmtbuf[oo], ss ); oo+=strlen(ss)
		// Width
		APPEND( widthStr );

		if (isFloat) {
			// Precision
			if(precisionStr[0]) { APPEND( precisionStr ); }
			APPEND( length );

			if ((flags & (fixed | scientific)) == (fixed | scientific)) /*AND*/ { APPEND( flags & uppercase ? "A" : "a"); }
			else if (flags & fixed)      { APPEND( flags & uppercase ? "F" : "f"); }
			else if (flags & scientific) { APPEND( flags & uppercase ? "E" : "e"); }
			else                         { APPEND( flags & uppercase ? "G" : "g"); }
		} else {
			APPEND( length );
			if (isUnsigned) {
				if (flags & hex)      { APPEND( flags & uppercase ? "X" : "x"); }
				else if (flags & oct) { APPEND( "o"); }
				else                  { APPEND( "u"); }
			} else                    { APPEND( "d"); }
		}
		return fmtbuf;
	}

	inline TraceStreamer& width(int y)
	{
        if (y != std::ios_base::width()) {
			std::ios_base::width(y);
			snprintf( widthStr, sizeof(widthStr), "%d", y );
#          ifdef TRACE_STREAMER_DEBUG
			std::cout << "TraceStreamer widthStr is now " << widthStr << std::endl;
#          endif
		}
		return *this;
	}

	inline TraceStreamer& precision(int y)
	{
		if (y != std::ios_base::precision()) {
			std::ios_base::precision(y);
			if(y) snprintf( precisionStr, sizeof(precisionStr), ".%d", y );
			else  precisionStr[0]='\0';
#          ifdef TRACE_STREAMER_DEBUG
			std::cout << "TraceStreamer precisionStr is now " << precisionStr << std::endl;
#          endif
		}
		return *this;
	}
# ifndef __clang__
	inline TraceStreamer& operator<<(std::_Setprecision r)
	{	precision(r._M_n);
		return *this;
	}
	inline TraceStreamer& operator<<(std::_Setw r)
	{	width(r._M_n);
		return *this;
	}
# endif

	// necessary for std::hex, std::dec
	typedef std::ios_base& (*manipulator)(std::ios_base&);
	inline TraceStreamer& operator<<(manipulator r)
	{	r(*this);
		return *this;
	}

	inline TraceStreamer& operator<<(void* const& r) // Tricky C++...to pass pointer by reference, have to have the const AFTER the type
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, "%p", r );
		else if (argCount<TRACE_STREAMER_ARGSMAX) { msg_append( "%p");  args[argCount++].p = r; }
		return *this;
	}
# ifndef __clang__
	inline TraceStreamer& operator<<(const int& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(false,false,"", _M_flags), r );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(false,false,"", _M_flags)); args[argCount++].i = r; }
		return *this;
	}
	inline TraceStreamer& operator<<(const long int& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(false,false,"l",_M_flags), r );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(false,false,"l",_M_flags)); args[argCount++].l = r; }
		return *this;
	}
	inline TraceStreamer& operator<<(const unsigned int& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(false,true, "", _M_flags), r );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(false,true, "", _M_flags)); args[argCount++].u = r; }
		return *this;
	}
	inline TraceStreamer& operator<<(const long unsigned int& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(false,true, "l",_M_flags), r );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(false,true, "l",_M_flags)); args[argCount++].u = r; }
		return *this;
	}
	inline TraceStreamer& operator<<(const double& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(true, false,"", _M_flags), r );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(true, false,"", _M_flags)); args[argCount++].d = r; }
		return *this;
	}
	inline TraceStreamer& operator<<(const float& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(true, false,"", _M_flags), r );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(true, false,"", _M_flags)); args[argCount++].d = r; }
		return *this;
	}
	inline TraceStreamer& operator<<(const bool& r)
	{	if (_M_flags & boolalpha)        msg_append( r ? "true" : "false");
		else if (do_f)                   msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, "%d", r );
		else if (argCount<TRACE_STREAMER_ARGSMAX) { msg_append( "%d");    args[argCount++].i = r; }
		return *this;
	}
# endif
	inline TraceStreamer& operator<<(const std::string& r) { msg_append( r.c_str()); return *this; }
	inline TraceStreamer& operator<<(char const* r)        { msg_append( r);         return *this; }
	inline TraceStreamer& operator<<(char* r)              { msg_append( r);         return *this; }
# if !defined(__clang__) && (__cplusplus >= 201103L)
	inline TraceStreamer& operator<<(std::atomic<unsigned long> const& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(false,true, "l",_M_flags), r.load() );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(false,true, "l",_M_flags)); args[argCount++].u = r.load(); }
		return *this;
	}
	inline TraceStreamer& operator<<(std::atomic<short int> const& r)
	{	if(do_f) msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, format(false,false,"h",_M_flags), r.load() );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append(format(false,false,"h",_M_flags)); args[argCount++].i = r.load(); }
		return *this;
	}
	inline TraceStreamer& operator<<(std::atomic<bool> const& r)
	{	if (_M_flags & boolalpha)          msg_append( (r.load() ? "true" : "false"));
		else if (do_f)                     msg_sz += snprintf( &msg[msg_sz], sizeof(msg)-1-msg_sz, "%d", r.load() );
		else if (argCount < TRACE_STREAMER_ARGSMAX) { msg_append( "%d");    args[argCount++].i = r.load(); }
		return *this;
	}
# endif
	// compiler asked for this -- can't think of why or when it will be used, but do the reasonable thing (append format and append args)
	inline TraceStreamer& operator<<(const TraceStreamer& r)
	{	for (size_t ii = argCount; ii < (argCount + (  r.argCount < TRACE_STREAMER_ARGSMAX
													 ? argCount + r.argCount
													 : TRACE_STREAMER_ARGSMAX) ); ++ii) {
			args[ii] = r.args[ii - argCount];
		}
		argCount = argCount + r.argCount < TRACE_STREAMER_ARGSMAX ? argCount + r.argCount : TRACE_STREAMER_ARGSMAX;
		msg_append( r.msg);
		return *this;
	}

	////https://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator
	/// https://stackoverflow.com/questions/2212776/overload-handling-of-stdendl
	typedef std::ostream& (*ostream_manipulator)(std::ostream&);

	inline TraceStreamer& operator<<(ostream_manipulator r)
	{	if(r == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::endl)
			msg_append( "\n");
		return *this;
	}

#  if TRACE_STREAMER_TEMPLATE
    // This is heavy weight (instantiation of stringstream); hopefully will not be done too often or not at all
	template<typename T>
	inline TraceStreamer& operator<<(const T& r)
	{
#      if DEBUG_FORCED
		std::cerr << "WARNING: " << __PRETTY_FUNCTION__ << " TEMPLATE CALLED: Consider implementing a function with this signature!" << std::endl;
#      endif
		std::ostringstream s; s << r; msg_append( s.str().c_str() );
		return *this;
	}
#  endif
};								// struct Streamer

} // unnamed namespace

#endif /* __cplusplus */

#endif /* TRACE_H_5216 */
