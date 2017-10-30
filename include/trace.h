/* This file (trace.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: trace.h,v $
 */
#ifndef TRACE_H_5216
#define TRACE_H_5216

#define TRACE_REV  "$Revision: 655 $$Date: 2017-10-28 23:48:00 -0500 (Sat, 28 Oct 2017) $"

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

# if   defined(__CYGWIN__)
#  include <windows.h>
static inline pid_t trace_gettid(void) { return GetCurrentThreadId(); }
static inline int   trace_getcpu(void) { return GetCurrentProcessorNumber(); }
# else
#  if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
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
#  if (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#   pragma GCC diagnostic pop
#  endif
# endif

# ifndef PATH_MAX
#  define PATH_MAX 1024  /* conservative */
# endif
# ifdef __cplusplus
#  include <sstream> /* std::ostringstream */
#  include <string>
# endif

# if   defined(__cplusplus)      &&      (__cplusplus >= 201103L)
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
# else
/* THIS IS A PROBLEM (older compiler on unknown arch) -- I SHOULD PROBABLY #error */
#  define TRACE_ATOMIC_T              uint32_t
#  define TRACE_ATOMIC_INIT           0
#  define TRACE_ATOMIC_LOAD(ptr)      *(ptr)
#  define TRACE_ATOMIC_STORE(ptr,val) *(ptr) = val
#  define TRACE_THREAD_LOCAL
#  define cmpxchg(ptr, old, new) \
	({ uint32_t old__ = *(ptr); if(old__==(old)) *ptr=new; old__;  })  /* THIS IS A PROBLEM -- NEED OS MUTEX HELP :( */
# endif

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


/* 88,7=192 bytes/ent   96,6=192   128,10=256*/
#define TRACE_DFLT_MAX_MSG_SZ      128
#define TRACE_DFLT_MAX_PARAMS       10
#define TRACE_DFLT_NAMTBL_ENTS     127  /* this is for creating new trace_buffer file --
										   it currently matches the "trace DISABLED" number that
										   fits into traceControl[1] (see below) */
#define TRACE_DFLT_NAM_SZ           40 /* 40 was the value with 8K pages which gave 127 NAMTBL_ENTS with "trace DISBALED".
										  Search for trace_created_init(...) call in "DISABLE" case.
										  Names can have this many characters (not including null terminator) */
#define TRACE_DFLT_NUM_ENTRIES   50000
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

#define LVLBITSMSK ((sizeof(uint64_t)*8)-1)
#define TLVLMSK(xx) (1LL<<((xx)&LVLBITSMSK))

/* helper for TRACEing strings in C - ONLY in C!*/ 
#if defined(__cplusplus)
/* don't want to instantiate an std::vector as it may cause alloc/delete */
# define TSBUFDECL(fmt) /*std::vector<char> tsbuf__(traceControl_p->siz_msg);*/ const char *fmt=NULL/*&(tsbuf__[0])*/
/*# define TSBUFSIZ__     tsbuf__.size()*/
#else
# define TSBUFDECL(fmt) char tsbuf__[traceControl_p->siz_msg+1]; const char *fmt=&(tsbuf__[0])
# define TSBUFSIZ__     sizeof(tsbuf__)
# define TSPRINTF( ... ) ( snprintf( &(tsbuf__[0]), TSBUFSIZ__, __VA_ARGS__ ), &(tsbuf__[0]) )
#endif

/* Note: anonymous variadic macros were introduced in C99/C++11 (maybe C++0x) */

# define TRACE TRACEC
# define TRACEC( lvl, ... ) do			\
    {   TRACE_INIT_CHECK									\
	{   TSBUFDECL(fmt__);											\
			struct timeval lclTime; lclTime.tv_sec = 0;					\
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[traceTID].M & TLVLMSK(lvl))) \
			{   fmt__=trace_charstar(__VA_ARGS__);	/* when printf fmt, this checks fmt */ \
				trace( &lclTime, traceTID, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED \
				      , fmt__ TRACE_ARGS_ARGS(__VA_ARGS__) );	\
			}															\
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[traceTID].S & TLVLMSK(lvl))) \
			{   if (lclTime.tv_sec == 0) { fmt__=trace_charstar(__VA_ARGS__); } \
				TRACE_LOG_FUNCTION( &lclTime, traceTID, lvl, TRACE_NARGS(__VA_ARGS__) \
				                   , fmt__ TRACE_ARGS_ARGS(__VA_ARGS__) ); \
			}															\
        }								\
    } while (0)
# define TRACEN( nam, lvl, ... ) do										\
    {   TRACE_INIT_CHECK												\
		{   TSBUFDECL(fmt__);											\
			static int tid_=-1; struct timeval lclTime;					\
			if(tid_==-1)tid_=name2TID(nam);	lclTime.tv_sec = 0;			\
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl))) \
			{   fmt__=trace_charstar(__VA_ARGS__);	/* when printf fmt, this checks fmt */ \
				trace( &lclTime, tid_, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED \
                      , fmt__ TRACE_ARGS_ARGS(__VA_ARGS__) );			\
			}															\
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[tid_].S & TLVLMSK(lvl))) \
			{   if (lclTime.tv_sec == 0) { fmt__=trace_charstar(__VA_ARGS__); } \
				TRACE_LOG_FUNCTION( &lclTime, tid_, lvl, TRACE_NARGS(__VA_ARGS__) \
								   , fmt__ TRACE_ARGS_ARGS(__VA_ARGS__) ); \
			}															\
        }								\
    } while (0)

#if defined(__cplusplus)
/* used only in c++0x c++11 environment */
/* Note: This supports using a mix of stream syntax and format args, i.e: "string is " << some_str << " and float is %f", some_float
   Note also how the macro evaluates the first part (the "FMT") only once
   no matter which destination ("M" and/or "S") is active.
   Note: "xx" in TRACE_ARGS_FMT(__VA_ARGS__,xx) is just a dummy arg to that macro.
*/
# define TRACE_( lvl, ... ) do								\
    {   TRACE_INIT_CHECK												\
		{   struct timeval lclTime; lclTime.tv_sec = 0;					\
			std::ostringstream ostr__;									\
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[traceTID].M & TLVLMSK(lvl))) \
			{   ostr__ << TRACE_ARGS_FMT(__VA_ARGS__,xx);				\
				trace( &lclTime,traceTID, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED \
				      , ostr__.str() TRACE_ARGS_ARGS(__VA_ARGS__) );	\
			}															\
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[traceTID].S & TLVLMSK(lvl))) \
			{   if (lclTime.tv_sec == 0) { ostr__ << TRACE_ARGS_FMT(__VA_ARGS__,xx); } \
				TRACE_LOG_FUNCTION( &lclTime, traceTID, lvl, TRACE_NARGS(__VA_ARGS__), ostr__.str().c_str() TRACE_ARGS_ARGS(__VA_ARGS__) ); \
			}															\
        }																\
    } while (0)
# define TRACEN_( nam, lvl, ... ) do									\
    {   TRACE_INIT_CHECK												\
		{   static int tid_=-1; struct timeval lclTime;					\
			if(tid_==-1)tid_=name2TID(nam);	lclTime.tv_sec = 0;			\
			std::ostringstream ostr__;									\
			if (traceControl_rwp->mode.bits.M && (traceNamLvls_p[tid_].M & TLVLMSK(lvl))) \
			{   ostr__ << TRACE_ARGS_FMT(__VA_ARGS__,xx);				\
				trace( &lclTime,tid_, lvl, TRACE_NARGS(__VA_ARGS__) TRACE_XTRA_PASSED \
				      , ostr__.str() TRACE_ARGS_ARGS(__VA_ARGS__) );	\
			}															\
			if (traceControl_rwp->mode.bits.S && (traceNamLvls_p[tid_].S & TLVLMSK(lvl))) \
			{   if (lclTime.tv_sec == 0) { ostr__ << TRACE_ARGS_FMT(__VA_ARGS__,xx); } \
				TRACE_LOG_FUNCTION( &lclTime, tid_, lvl, TRACE_NARGS(__VA_ARGS__), ostr__.str().c_str() TRACE_ARGS_ARGS(__VA_ARGS__) ); \
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

#ifdef __GNUC__
#define SUPPRESS_NOT_USED_WARN __attribute__ ((unused))
#else
#define SUPPRESS_NOT_USED_WARN
#endif

static const char *trace_charstar(const char *ss,...) __attribute__((format(printf,1,2)));
SUPPRESS_NOT_USED_WARN
static const char *trace_charstar(const char *ss,...)  { return ss; }
static void trace(struct timeval*,int,uint16_t,uint16_t TRACE_XTRA_UNUSED,const char *,...)__attribute__((format(printf,TRACE_PRINTF_FMT_ARG_NUM,TRACE_PRINTF_FMT_ARG_NUM+1)));
#ifdef __cplusplus
SUPPRESS_NOT_USED_WARN
static const char *trace_charstar(const std::string &ss,...) { return ss.c_str(); }
static void trace(struct timeval*,int,uint16_t,uint16_t TRACE_XTRA_UNUSED,const std::string&,...);
#endif


union trace_mode_u
{   struct
    {   uint32_t M:1; /* b0 high speed circular Memory */
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
      uint32_t      xtra[TRACE_CACHELINE/sizeof(int32_t)-6]; /* force some sort of alignment -- taking into account the 6 fields (above) since the last cache line alignment */
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

struct traceEntryHdr_s
{   struct timeval time;/*T*/
    TRACE_ENT_TV_FILLER	     /* because timeval is larger on x86_64 (16 bytes compared to 8 for i686) */
    uint64_t       tsc; /*t*/
    uint16_t       lvl; /*L*/
    uint16_t       nargs;
    pid_t          pid; /*P system info */
    pid_t          tid; /*i system info - "thread id" */
    int32_t        cpu; /*C -- kernel sched switch will indicate this info? */
    int32_t        TrcId; /*I Trace ID ==> idx into lvlTbl, namTbl */
    uint16_t       get_idxCnt_retries;/*R*/
    uint16_t       param_bytes;       /*B*/
};                                    /*M -- NO, ALWAY PRINTED LAST! formated Message */
                                      /*N  index */
struct traceNamLvls_s
{   uint64_t      M;
    uint64_t      S;
    uint64_t      T;
    char          name[TRACE_DFLT_NAM_SZ];
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
TRACE_DECL( char * tracePrintFmt, =NULL ); /* hardcoded default below that can only be overridden via env.var */
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
# if defined(__GXX_WEAK__) || ( defined(__cplusplus) && (__cplusplus >= 199711L) ) || ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )
/* c++98 c99 c++0x c11 c++11 */
#  define TRACE_LOG_FUNCTION(tvp,tid,lvl,nargs,...)          trace_user( tvp,tid,lvl,nargs,__VA_ARGS__ )
# else
/* c89 */
#  define TRACE_LOG_FUNCTION(tvp,tid,lvl,nargs,msgargs... )  trace_user( tvp,tid,lvl,nargs,msgargs )
# endif   /* __GXX_WEAK__... */
#endif /* TRACE_LOG_FUNCTION */

SUPPRESS_NOT_USED_WARN
static void vtrace_user( struct timeval *tvp, int TrcId, uint16_t lvl, uint16_t nargs, const char *msg, va_list ap )
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
    char   obuf[0x1000]; char tbuf[0x100]; int printed=0;
    char   *cp;
    struct tm	tm_s;
	int    quiet_warn=0;
    if (tracePrintFmt==NULL)
    {   /* no matter who writes, it should basically be the same thing */
	if((cp=getenv("TRACE_TIME_FMT"))!=NULL) tracePrintFmt=cp; /* single write here */
	else                                    tracePrintFmt=(char*)TRACE_DFLT_TIME_FMT; /* OR single write here */
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
    printed += snprintf( &(obuf[printed]), sizeof(obuf)-printed
	                    , &(" %2d %2d ")[printed==0?1:0] /* skip leading " " if nothing was printed (TRACE_TIME_FMT="") */
			, TrcId, lvl );
    if (nargs)
	printed += vsnprintf( &(obuf[printed])
			     , (printed<(int)sizeof(obuf))?sizeof(obuf)-printed:0
			     , msg, ap );
    else /* don't do any parsing for format specifiers in the msg -- tshow will
	    also know to do this on the memory side of things */
	printed += snprintf( &(obuf[printed])
			    , (printed<(int)sizeof(obuf))?sizeof(obuf)-printed:0
			    , "%s", msg );
    if (printed < (int)sizeof(obuf))
    {   /* there is room for the \n */
	/* buf first see if it is needed */
	if (obuf[printed-1] != '\n')
	{   obuf[printed++] = '\n'; /* overwriting \0 is OK as we will specify the amount to write */
	    /*printf("added \\n printed=%d\n",printed);*/
	}
	/*else printf("already there printed=%d\n",printed);*/
	quiet_warn += write( tracePrintFd, obuf, printed );
    }
    else
    {  /* obuf[sizeof(obuf)-1] has '\0'. see if we should change it to \n */
	if (obuf[sizeof(obuf)-2] == '\n')
	    quiet_warn += write( tracePrintFd, obuf, sizeof(obuf)-1 );
	else
	{   obuf[sizeof(obuf)-1] = '\n';
	    quiet_warn += write( tracePrintFd, obuf, sizeof(obuf) );
	    /*printf("changed \\0 to \\n printed=%d\n",);*/
	}
    }
#endif
}
SUPPRESS_NOT_USED_WARN
static void trace_user( struct timeval *tvp, int TrcId, uint16_t lvl, uint16_t nargs, const char *msg, ... )
{
	va_list ap;
	va_start( ap, msg );
	vtrace_user( tvp, TrcId, lvl, nargs, msg, ap );
	va_end( ap );
}
#ifdef __cplusplus
SUPPRESS_NOT_USED_WARN
static void trace_user( struct timeval *tvp, int TrcId, uint16_t lvl, uint16_t nargs, std::string& msg, ... )
{
    va_list ap;
	va_start( ap, msg );
	vtrace_user( tvp, TrcId, lvl, nargs, msg.c_str(), ap );
	va_end( ap );	
}   /* trace */
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
    TRACE_GETTIMEOFDAY( tvp );  /* hopefully NOT a system call */

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
#endif

SUPPRESS_NOT_USED_WARN
static void trace( struct timeval *tvp, int trcId, uint16_t lvl, uint16_t nargs
                  TRACE_XTRA_UNUSED		  
		  , const char *msg, ... )
{
    va_list ap;
	va_start( ap, msg );
	vtrace( tvp, trcId, lvl, nargs, msg, ap );
	va_end( ap );	
}   /* trace */

#ifdef __cplusplus
SUPPRESS_NOT_USED_WARN
static void trace( struct timeval *tvp, int trcId, uint16_t lvl, uint16_t nargs
                  TRACE_XTRA_UNUSED		  
				  , const std::string& msg, ... )
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


static void trace_lock( TRACE_ATOMIC_T *atomic_addr )
{
    uint32_t desired=1, expect=0, hung=0;
#if defined(__KERNEL__)
    while (cmpxchg(atomic_addr,expect,desired) != expect)
	if (++hung >100000000) { TRACE_PRINT("trace_lock: hung?\n"); break; }
#elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
    while (!atomic_compare_exchange_weak(atomic_addr, &expect, desired))
    {   expect=0;
	if (++hung >100000000) { TRACE_PRINT("trace_lock: hung?\n"); break; }
    }
    if(atomic_addr==&traceInitLck && traceInitLck_hung_max<hung) traceInitLck_hung_max=hung;
#else
    while (cmpxchg(atomic_addr,expect,desired) != expect)
	if (++hung >100000000) { TRACE_PRINT("trace_lock: hung?\n"); break; }
    if(atomic_addr==&traceInitLck && traceInitLck_hung_max<hung) traceInitLck_hung_max=hung;
#endif
}

static void trace_unlock( TRACE_ATOMIC_T *atomic_addr )
{
#if defined(__KERNEL__)
    TRACE_ATOMIC_STORE(atomic_addr,(uint32_t)0);
#elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
    atomic_store(atomic_addr,(uint32_t)0);
#else
    TRACE_ATOMIC_STORE(atomic_addr,(uint32_t)0);
#endif
}

static int32_t name2TID( const char *name )
{
    uint32_t ii;
	char     valid_name[TRACE_DFLT_NAM_SZ];
#if defined(__KERNEL__)
    if (traceEntries_p==NULL) return -1;
#endif
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
		if (strncmp(traceNamLvls_p[ii].name,name,TRACE_DFLT_NAM_SZ)==0) {
			return (ii);
		}
	/* NOTE: multiple threads which may want to create the same name might arrive at this
       point at the same time. So, each thread should check if another thread has just released
       the lock after creating the name.
	*/
    trace_lock( &traceControl_rwp->namelock );
	/* only allow "valid" names to be inserted -- above, we assumed the
	   name was valid, giving the caller the benefit of the doubt, for
	   efficiency sake, but here we will make sure the name is valid */
	for (ii=0; name[ii]!='\0' && ii<TRACE_DFLT_NAM_SZ; ++ii) {
		if (isgraph(name[ii]))
			valid_name[ii] = name[ii];
		else
			valid_name[ii]='_';
	}
	if (ii<TRACE_DFLT_NAM_SZ) valid_name[ii]='\0';
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii) {
		if (traceNamLvls_p[ii].name[0] == '\0') {
			strncpy(traceNamLvls_p[ii].name,valid_name,TRACE_DFLT_NAM_SZ);
			if(trace_lvlS)         /* See also traceInitNames */
				traceNamLvls_p[ii].S = trace_lvlS;
			trace_unlock( &traceControl_rwp->namelock );
			return (ii);
		}
		if (strncmp(traceNamLvls_p[ii].name,valid_name,TRACE_DFLT_NAM_SZ)==0) {
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
	/* This is a signficant env.var as this can be setting the
	   level mask for many TRACE NAMEs/IDs AND potentially many process
	   could do this or at least be TRACE-ing. In other words, this has
	   the potential of effecting another process's TRACE-ing. */
	if ((cp=getenv("TRACE_NAMLVLSET"))) {
		int                 sts, xx;
		char                name[TRACE_DFLT_NAM_SZ+1];
		unsigned long long  M,S,T=0;
		while (   ((sts=sscanf(cp,"%d %" MM_STR(TRACE_DFLT_NAM_SZ) "s %llx %llx %llx",&xx,name,&M,&S,&T))&&sts==5)
		       || ((sts=sscanf(cp,"%" MM_STR(TRACE_DFLT_NAM_SZ) "s %llx %llx %llx",       name,&M,&S,&T))&&sts==4)
		       || ((sts=sscanf(cp,"%" MM_STR(TRACE_DFLT_NAM_SZ) "s %llx %llx",            name,&M,&S   ))&&sts==3)) {
			int tid=name2TID(name);
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
}   /* trace_namLvlSet */
#endif

/* NOTE: because of how this is call from a macro on both 64 and 32 bit
   systems AND THE WAY IT IS CALLED FROM THE trace_cntl programg,
   all argument, EXCEPT for the "name" and "file" commands, should be
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
    if (strncmp(cmd,"file",4) == 0)/* THIS really only makes sense for non-thread local-file-for-module or for non-static implementation w/TLS for file-per-thread */
    {	traceFile = va_arg(ap,char*);/* this can still be overridden by env.var.; suggest testing w. TRACE_ARGSMAX=10*/
	traceInit(TRACE_NAME);		/* will not RE-init as traceControl_p!=NULL skips mmap_file */
	va_end(ap); return (0);
    }
	else if (strncmp(cmd,"namlvlset",9) == 0) {
		/* use this if program sets TRACE_NAMLVLSET env.var.  This can be used
		   to Init or called trace_namLvlSet() after an Init has occurred. */
		const char *name=(nargs==0)?TRACE_NAME:va_arg(ap,char*); /* name is optional */
		/*printf("nargs=%d name=%s\n",nargs,name);*/
		if (traceControl_p == NULL)
			traceInit( name );	/* with traceControl_p==NULL. trace_namLvlSet() will be called */
		else
			trace_namLvlSet();  /* recall trace_namLvlSet(). optional name, if given, is ignored */
		va_end(ap); return (0);
	}
#endif
    TRACE_INIT_CHECK {};     /* note: allows name2TID to be called in userspace */

    if (strncmp(cmd,"name",4) == 0)/* THIS really only makes sense for non-thread local-name-for-module or for non-static implementation w/TLS for name-per-thread */
    {	traceName = va_arg(ap,char*);/* this can still be overridden by env.var. IF traceInit(TRACE_NAME) is called; suggest testing w. TRACE_ARGSMAX=10*/
	traceTID = name2TID( traceName );/* doing it this way allows this to be called by kernel module */
    }
    else if (strncmp(cmd,"trig",4) == 0)    /* takes 2 args: lvlsMsk, postEntries */
    {
	uint64_t lvlsMsk=va_arg(ap,uint64_t);
	unsigned post_entries=va_arg(ap,uint64_t);
	if ((traceNamLvls_p[traceTID].M&lvlsMsk) != lvlsMsk) {
#  ifndef __KERNEL__
		fprintf(stderr, "Warning: \"trig\" setting (additional) bits (0x%llx) in traceTID=%d\n", (unsigned long long)lvlsMsk, traceTID );
#  endif
		traceNamLvls_p[traceTID].M |= lvlsMsk;
	}
#  ifndef __KERNEL__
	if (traceControl_rwp->trigActivePost)
		fprintf(stderr, "Warning: \"trig\" overwriting trigActivePost (previous=%d)\n", traceControl_rwp->trigActivePost );
#  endif
	traceNamLvls_p[traceTID].T       = lvlsMsk;
	traceControl_rwp->trigActivePost = post_entries?post_entries:1; /* must be at least 1 */
	traceControl_rwp->triggered      = 0;
	traceControl_rwp->trigIdxCnt     = 0;
    }
    else if (strncmp(cmd,"lvlmsk",6) == 0) /* TAKES 1 or 3 args: lvlX or lvlM,lvlS,lvlT */
    {   uint64_t lvl, lvlm, lvls, lvlt;
	unsigned ee;
	if ((cmd[6]=='g')||((cmd[6])&&(cmd[7]=='g')))
	{   ii=0;        ee=traceControl_p->num_namLvlTblEnts;
	}
	else
	{   ii=traceTID; ee=traceTID+1;
	}
	lvl=va_arg(ap,uint64_t); /* "FIRST" ARG SHOULD ALWAYS BE THERE */
	switch (cmd[6])
	{
	case 'M': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].M = lvl; break;
	case 'S': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].S = lvl; break;
	case 'T': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].T = lvl; break;
	default:
	    if (nargs != 3)
	    {   TRACE_PRINT("need 3 lvlmsks; %d given\n",nargs);va_end(ap); return (-1);
	    }
	    lvlm=lvl; /* "FIRST" arg from above */
	    lvls=va_arg(ap,uint64_t);
	    lvlt=va_arg(ap,uint64_t);
	    for ( ; ii<ee; ++ii)
	    {   traceNamLvls_p[ii].M = lvlm;
		traceNamLvls_p[ii].S = lvls;
		traceNamLvls_p[ii].T = lvlt;
	    }
	}
    }
    else if (strncmp(cmd,"lvlset",6) == 0) /* TAKES 1 or 3 args: lvlX or lvlM,lvlS,lvlT ((0 val ==> no-op) */
    {   uint64_t lvl, lvlm, lvls, lvlt;
	unsigned ee;
	if ((cmd[6]=='g')||((cmd[6])&&(cmd[7]=='g')))
	{   ii=0;        ee=traceControl_p->num_namLvlTblEnts;
	}
	else
	{   ii=traceTID; ee=traceTID+1;
	}
	lvl=va_arg(ap,uint64_t); /* "FIRST" ARG SHOULD ALWAYS BE THERE */
	switch (cmd[6])
	{
	case 'M': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].M |= lvl; break;
	case 'S': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].S |= lvl; break;
	case 'T': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].T |= lvl; break;
	default:
	    if (nargs != 3)
	    {   TRACE_PRINT("need 3 lvlmsks; %d given\n",nargs);va_end(ap); return (-1);
	    }
	    lvlm=lvl; /* "FIRST" arg from above */
	    lvls=va_arg(ap,uint64_t);
	    lvlt=va_arg(ap,uint64_t);
	    for ( ; ii<ee; ++ii)
	    {   traceNamLvls_p[ii].M |= lvlm;
		traceNamLvls_p[ii].S |= lvls;
		traceNamLvls_p[ii].T |= lvlt;
	    }
	}
    }
    else if (strncmp(cmd,"lvlclr",6) == 0) /* TAKES 1 or 3 args: lvlX or lvlM,lvlS,lvlT ((0 val ==> no-op) */
    {   uint64_t lvl, lvlm, lvls, lvlt;
	unsigned ee;
	if ((cmd[6]=='g')||((cmd[6])&&(cmd[7]=='g')))
	{   ii=0;        ee=traceControl_p->num_namLvlTblEnts;
	}
	else
	{   ii=traceTID; ee=traceTID+1;
	}
	lvl=va_arg(ap,uint64_t); /* "FIRST" ARG SHOULD ALWAYS BE THERE */
	switch (cmd[6])
	{
	case 'M': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].M &= ~lvl; break;
	case 'S': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].S &= ~lvl; break;
	case 'T': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].T &= ~lvl; break;
	default:
	    if (nargs != 3)
	    {   TRACE_PRINT("need 3 lvlmsks; %d given\n",nargs);va_end(ap); return (-1);
	    }
	    lvlm=lvl; /* "FIRST" arg from above */
	    lvls=va_arg(ap,uint64_t);
	    lvlt=va_arg(ap,uint64_t);
	    for ( ; ii<ee; ++ii)
	    {   traceNamLvls_p[ii].M &= ~lvlm;
		traceNamLvls_p[ii].S &= ~lvls;
		traceNamLvls_p[ii].T &= ~lvlt;
	    }
	}
    }
    else if (strncmp(cmd,"mode",4) == 0)
    {
	switch (cmd[4])
	{
	case '\0':
	    ret=traceControl_rwp->mode.mode;
	    if (nargs==1)
	    {   uint32_t mode=va_arg(ap,uint64_t);
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
	    if (nargs==1)
	    {   uint32_t mode=va_arg(ap,uint64_t);
		traceControl_rwp->mode.bits.M = mode;
	    }
	    break;
	case 'S':
	    ret=traceControl_rwp->mode.bits.S;
	    if (nargs==1)
	    {   uint32_t mode=va_arg(ap,uint64_t);
		traceControl_rwp->mode.bits.S = mode;
	    }
	    break;
	default:
	    ret=-1;
	}
    }
    else if (strcmp(cmd,"reset") == 0) 
    {
	traceControl_rwp->full
	    = traceControl_rwp->trigIdxCnt
	    = traceControl_rwp->trigActivePost
	    = 0;
	TRACE_ATOMIC_STORE( &traceControl_rwp->wrIdxCnt, (uint32_t)0 );
	traceControl_rwp->triggered = 0;
    }
    else
    {   ret = -1;
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
    trace_user( &tv,0,0,1,"trace_created_init: tC_p=%p",t_p );
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
    trace_user( &tv,0,0,"trace_created_init: tC_p=%p",t_p );
# endif
}   /* trace_created_init */

#endif

#ifndef __KERNEL__

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

    trace_lock( &traceInitLck );
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
		((cp=getenv("TRACE_NUMENTS"))   &&       (numents_    =strtoul(cp,NULL,0))&&(activate=1))||(numents_   =TRACE_DFLT_NUM_ENTRIES);
		((cp=getenv("TRACE_NAMTBLENTS"))&&       (namtblents_ =strtoul(cp,NULL,0))&&(activate=1))||(namtblents_=TRACE_DFLT_NAMTBL_ENTS);

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
			                   , msgmax_
			                   , argsmax_
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

    if ((cp=getenv("TRACE_LVLS"))     && (*cp))
    {   TRACE_CNTL( "lvlmskS", strtoull(cp,NULL,0) );  /* set for this traceTID */
	    trace_lvlS = strtoull(cp,NULL,0);              /* set for future new traceTIDs */
    }

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
		traceNamLvls_p[ii].S = 0x7; /* then defaults except for trace_lvlS */
		traceNamLvls_p[ii].T = 0;   /* in name2TID. */
    }								/* (0 for err, 1=warn, 2=info, 3=debug) */
# ifdef __KERNEL__
    strcpy( traceNamLvls_p[0].name,"KERNEL" );
	/* like userspace TRACE_LVLS env.var - See also name2TID */
	if(trace_lvlS)
		traceNamLvls_p[0].S = trace_lvlS;
# endif
    strcpy( traceNamLvls_p[tC_p->num_namLvlTblEnts-2].name,"TRACE" );
    strcpy( traceNamLvls_p[tC_p->num_namLvlTblEnts-1].name,"_TRACE_" );
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

#endif /* TRACE_H_5216 */
