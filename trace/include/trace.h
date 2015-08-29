/* This file (trace.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: trace.h,v $
 */
#ifndef TRACE_H_5216
#define TRACE_H_5216

#define TRACE_REV  "$Revision: 1.119 $$Date: 2015-08-29 06:59:41 $"

#ifndef __KERNEL__

# include <stdio.h>		/* printf */
# include <stdarg.h>		/* va_list */
# include <stdint.h>		/* uint64_t */
# include <sys/time.h>          /* timeval */
# include <string.h>		/* strncmp */
# include <fcntl.h>		/* open, O_RDWR */
# include <sys/mman.h>		/* mmap */
# include <unistd.h>		/* lseek */
# include <sys/stat.h>		/* fstat */
# include <errno.h>		/* errno */
# include <limits.h>		/* PATH_MAX */
# include <stdlib.h>		/* getenv, setenv, strtoul */
# include <sys/syscall.h>	/* syscall */
# ifdef __linux__
#  include <sched.h>		/* sched_getcpu - does vsyscall getcpu */
# endif
# ifndef PATH_MAX
#  define PATH_MAX 1024  /* conservative */
# endif
# ifdef __sun__
#  define SYS_GETTID SYS_lwp_self
# else
#  define SYS_GETTID SYS_gettid
# endif
# if   defined(__cplusplus)      &&      (__cplusplus >= 201103L)
#  include <atomic>		/* atomic<> */
#  define TRACE_ATOMIC_T     std::atomic<uint32_t>
#  define TRACE_THREAD_LOCAL thread_local 
# elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  include <stdatomic.h>		/* atomic_compare_exchange_weak */
#  define TRACE_ATOMIC_T          /*volatile*/ _Atomic(uint32_t)
#  define TRACE_THREAD_LOCAL      _Thread_local
# elif defined(__x86_64__) || defined(__i686__) || defined(__i386__)
#  define TRACE_ATOMIC_T          uint32_t
#  define TRACE_THREAD_LOCAL
static inline uint32_t cmpxchg( uint32_t *ptr, uint32_t old, uint32_t new_) \
    { uint32_t __ret;							\
    uint32_t __old = (old);						\
    uint32_t __new = (new_);						\
    volatile uint32_t *__ptr = (volatile uint32_t *)(ptr);		\
    __asm__ volatile("lock cmpxchgl %2,%1"				\
		 : "=a" (__ret), "+m" (*__ptr)				\
		 : "r" (__new), "0" (__old)				\
		 : "memory");						\
    return (__ret);							\
    }
# else
#  define TRACE_ATOMIC_T          uint32_t
#  define TRACE_THREAD_LOCAL
#  define cmpxchg(ptr, old, new) \
  ({ uint32_t __old = (old); old=*ptr; *ptr=new; __old;  })
# endif
# define TRACE_GETTIMEOFDAY( tvp ) gettimeofday( tvp, NULL )
# define TRACE_PRINT              printf
# define TRACE_VPRINT             vprintf
/*# define TRACE_INIT_CHECK         if((traceControl_p!=NULL)||(traceInit(TRACE_NAME)==0))*/
# define TRACE_INIT_CHECK         if(((traceTid!=0)&&(traceControl_p!=0))||(traceInit(TRACE_NAME)==0))

#else  /* __KERNEL__ */

# include <linux/time.h>	   /* do_gettimeofday */
/*# include <linux/printk.h>	      printk, vprintk */
# include <linux/kernel.h>	   /* printk, vprintk */
# include <linux/mm.h>		   /* kmalloc OR __get_free_pages */
# include <linux/vmalloc.h>	   /* __vmalloc, vfree */
# include <linux/spinlock.h>	   /* cmpxchg */
# include <linux/sched.h>	   /* current (struct task_struct *) */
# define TRACE_ATOMIC_T            uint32_t
# define TRACE_THREAD_LOCAL 
# define TRACE_GETTIMEOFDAY( tvp ) do_gettimeofday( tvp )
# define TRACE_PRINT               printk
# define TRACE_VPRINT              vprintk
# define TRACE_INIT_CHECK          /* no check for kernel -- init when module loaded */

#endif /* __KERNEL__ */


/* 88,7=192 bytes/ent   96,6=192   128,10=256*/
#define TRACE_DFLT_MAX_MSG_SZ      128
#define TRACE_DFLT_MAX_PARAMS       10
#define TRACE_DFLT_NAMTBL_ENTS      20
#define TRACE_DFLT_NUM_ENTRIES   50000
#define TRACE_DFLT_NAM_SZ           16
#define TRACE_DFLT_NAME        "TRACE"
#ifndef  TRACE_NAME
# if !defined(TRACE_LIB) && !defined(TRACE_IMPL)
static const char *  TRACE_NAME=NULL;
# endif
#endif
#ifndef  TRACE_PRINT_FD
# define TRACE_PRINT_FD           1
#endif

#define TRACE_PAGESIZE         0x2000
#define TRACE_CACHELINE        64


#if defined(__GXX_WEAK__) || ( defined(__cplusplus) && (__cplusplus >= 199711L) ) || ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )

/* c++98 c99 c++0x c11 c++11 */

# define TRACE( lvl, ... ) do \
    {   unsigned __lvl=lvl;						\
	TRACE_INIT_CHECK						\
	{   struct timeval lclTime; lclTime.tv_sec = 0;			\
	    if (traceControl_p->mode.bits.M && (traceNamLvls_p[traceTID].M & (1<<__lvl))) \
            {   trace( &lclTime, lvl, TRACE_ARGS(__VA_ARGS__)-1 TRACE_XTRA_PASSED \
                      , __VA_ARGS__ );					\
	    }								\
	    if (traceControl_p->mode.bits.S && (traceNamLvls_p[traceTID].S & (1<<__lvl))) \
	    {   TRACE_LOG_FUNCTION( &lclTime, traceTID, lvl, __VA_ARGS__ ); \
	    }								\
        }								\
    } while (0)

# define TRACE_ARGS(...) TRACE_ARGS_HELPER1(__VA_ARGS__,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0) /* 0 here but not below */
# define TRACE_ARGS_HELPER1(...) TRACE_ARGS_HELPER2(__VA_ARGS__)
# define TRACE_ARGS_HELPER2(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n, ...) n
# define TRACE_CNTL( ... ) traceCntl( TRACE_ARGS(__VA_ARGS__) - 1, __VA_ARGS__ )

#else    /* __GXX_WEAK__... */

/* c89 */

# define TRACE( lvl, msgargs... ) do		\
    {   unsigned __lvl=lvl;						\
	TRACE_INIT_CHECK						\
	{   struct timeval lclTime; lclTime.tv_sec = 0;			\
	    if (traceControl_p->mode.bits.M && (traceNamLvls_p[traceTID].M & (1<<__lvl))) \
	    {   trace( &lclTime, lvl, TRACE_ARGS(0, msgargs)-2 TRACE_XTRA_PASSED \
                      , msgargs );					\
	    }								\
	    if (traceControl_p->mode.bits.S && (traceNamLvls_p[traceTID].S & (1<<__lvl))) \
	    {   TRACE_LOG_FUNCTION( &lclTime, traceTID, lvl, msgargs );	\
	    }								\
	}								\
    } while (0)

# define TRACE_ARGS(args...) TRACE_ARGS_HELPER1(args,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
# define TRACE_ARGS_HELPER1(args...) TRACE_ARGS_HELPER2(args)
# define TRACE_ARGS_HELPER2(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n, x...) n
# define TRACE_CNTL( cmdargs... ) traceCntl( TRACE_ARGS( 0, cmdargs ) - 2 , cmdargs )

#endif   /* __GXX_WEAK__... */


#if   defined(__i386__)
# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
# ifndef TRACE_LIB
   static void trace(struct timeval*,unsigned,unsigned,const char *,...)__attribute__((format(printf,4,5)));
# endif
# define TRACE_VA_LIST_INIT(addr) (va_list)addr
# define TRACE_ENT_FILLER         uint32_t x[2];
# define TRACE_32_DOUBLE_KLUDGE   nargs*=2;    /* kludge to support potential double in msg fmt */
# define TRACE_TSC32( low )       __asm__ __volatile__ ("rdtsc;movl %%eax,%0":"=m"(low)::"eax","edx")
#elif defined(__x86_64__)
# define TRACE_XTRA_PASSED        ,0,0, .0,.0,.0,.0,.0,.0,.0,.0
# define TRACE_XTRA_UNUSED        ,long l0 __attribute__((__unused__)),long l1 __attribute__((__unused__))\
	,double d0 __attribute__((__unused__)),double d1 __attribute__((__unused__)) \
	,double d2 __attribute__((__unused__)),double d3 __attribute__((__unused__)) \
	,double d4 __attribute__((__unused__)),double d5 __attribute__((__unused__)) \
	,double d6 __attribute__((__unused__)),double d7 __attribute__((__unused__))
# ifndef TRACE_LIB
   static void trace(struct timeval*,unsigned,unsigned TRACE_XTRA_UNUSED,const char *,...)__attribute__((format(printf,14,15)));
# endif
# define TRACE_VA_LIST_INIT(addr) {{6*8,6*8+9*16,addr,addr}}
# define TRACE_ENT_FILLER
# define TRACE_32_DOUBLE_KLUDGE
# define TRACE_TSC32( low )       __asm__ __volatile__ ("rdtsc" : "=a" (low) : : "edx")
#else
# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
# ifndef TRACE_LIB
   static void trace(struct timeval*,unsigned,unsigned,const char *,...)__attribute__((format(printf,4,5)));
# endif
# define TRACE_VA_LIST_INIT(addr) {addr}
# define TRACE_ENT_FILLER
# define TRACE_32_DOUBLE_KLUDGE   if(sizeof(long)==4)nargs*=2;
# define TRACE_TSC32( low )       low=0
#endif


union trace_mode_u
{   struct
    {   uint32_t M:1; /* b0 high speed circular Memory */
	uint32_t S:1; /* b1 printf (formatted) to Screen/Stdout */
    }   bits;
    uint32_t  mode;
};

struct traceControl_s
{
    char           version_string[sizeof(int32_t)*16];
    uint32_t	   version;
    uint32_t       num_params;
    uint32_t       siz_msg;
    uint32_t       siz_entry;
    uint32_t       num_entries;
    uint32_t       largest_multiple;
    uint32_t       num_namLvlTblEnts;
  volatile int32_t trace_initialized;/* these and above would be read only if */
    uint32_t       memlen;           /* in kernel */
    uint32_t       create_tv_sec;
    uint32_t       largest_zero_offset;
    uint32_t       page_align[TRACE_PAGESIZE/sizeof(int32_t)-27]; /* allow mmap 1st page(s) (stuff above) readonly */

    TRACE_ATOMIC_T wrIdxCnt;	/* 32 bit */
    uint32_t       cacheline1[TRACE_CACHELINE/sizeof(int32_t)-1];   /* the goal is to have wrIdxCnt in it's own cache line */

    TRACE_ATOMIC_T spinlock;	/* 32 bit */
    uint32_t       cacheline2[TRACE_CACHELINE/sizeof(int32_t)-1];   /* the goal is to have wrIdxCnt in it's own cache line */

    union trace_mode_u mode;
    uint32_t      reserved0;    /* use to be trigOffMode */
    uint32_t      trigIdxCnt;   /* BASED ON "M" mode Counts */
    int32_t       triggered;
    uint32_t	  trigActivePost;
    int32_t       full;
    uint32_t      xtra[TRACE_CACHELINE/sizeof(int32_t)-6]; /* force some sort of alignment -- here 10+6(fields above) = 16 */
};

struct traceEntryHdr_s
{   struct timeval time;/*T*/
    TRACE_ENT_FILLER	     /* because timeval is larger on x86_64 (16 bytes compared to 8 for i686) */
    uint64_t       tsc;               /*t*/
    int32_t        lvl; /*L*/
    pid_t          pid; /*P system info */
    pid_t          tid; /*i system info - "thread id" */
    int32_t        cpu; /*C -- kernel sched switch will indicate this info? */
    uint32_t       TID; /*I Trace ID ==> idx into lvlTbl, namTbl */
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
/* enter the 5 use case "areas" -- see doc/5in1.txt                         */


#ifndef TRACE_DECL
#define TRACE_DECL( scope, type_name, initializer ) scope type_name initializer
#endif

#if defined(__KERNEL__)
extern struct traceNamLvls_s  *traceNamLvls_p;
extern struct traceEntryHdr_s *traceEntries_p;
extern struct traceControl_s  *traceControl_p;
extern int                     trace_allow_printk;                 /* module_param */
static const char             *traceName="KERNEL";
#else
# define TRACE_DISABLE_NAM_SZ  2   /* used also in tracelib.c */
TRACE_DECL( static, struct traceNamLvls_s  traceNamLvls[TRACE_DISABLE_NAM_SZ], ); /* IMPORTANT - 1) this size, 2) traceInit setting of num_namLvlTblEnts, 3) traceInitNames and 4) "tids" MUST agree */
TRACE_DECL( static, TRACE_THREAD_LOCAL struct traceNamLvls_s *traceNamLvls_p, =&traceNamLvls[0] );
TRACE_DECL( static, TRACE_THREAD_LOCAL struct traceEntryHdr_s *traceEntries_p, );
TRACE_DECL( static, TRACE_THREAD_LOCAL struct traceControl_s  *traceControl_p, =NULL );
TRACE_DECL( static, TRACE_THREAD_LOCAL const char *traceFile, ="/tmp/trace_buffer_%s" );/*a local/efficient FS device is best; operation when path is on NFS device has not been studied*/
TRACE_DECL( static, TRACE_THREAD_LOCAL const char *traceName, =TRACE_DFLT_NAME );
TRACE_DECL( static, int                      tracePrintFd, =1 );
TRACE_DECL( static, pid_t                    tracePid,     =0 );
TRACE_DECL( static, TRACE_THREAD_LOCAL pid_t traceTid,     =0 );  /* thread id */
#endif


TRACE_DECL( static, TRACE_THREAD_LOCAL int   traceTID, =0 );  /* idx into lvlTbl, namTbl */


#ifndef TRACE_LIB

/* forward declarations, important functions */
static struct traceEntryHdr_s*  idxCnt2entPtr( uint32_t idxCnt );
# if !defined(__KERNEL__) || defined(TRACE_IMPL)  /* K=0,IMPL=0; K=0,IMPL=1; K=1,IMPL=1 */
static int                      traceInit( const char *name );
static void                     traceInitNames( void );
#  ifdef __KERNEL__                               /*                         K=1,IMPL=1 */
static int                msgmax=TRACE_DFLT_MAX_MSG_SZ;      /* module_param */
static int                argsmax=TRACE_DFLT_MAX_PARAMS;     /* module_param */
static int                numents=TRACE_DFLT_NUM_ENTRIES;    /* module_param */
static int                namtblents=TRACE_DFLT_NAMTBL_ENTS; /* module_param */
       int                trace_allow_printk=0;              /* module_param */
#  endif
# else    /*                                         K=1,IMPL=0  */

# endif   /*  __KERNEL__             TRACE_IMPL  */
# ifndef __KERNEL__
static struct traceControl_s    traceControl;
# endif

static int                      traceCntl( int nargs, const char *cmd, ... );
static uint32_t                 name2tid( const char *name );
#define cntlPagesSiz()          ((uint32_t)sizeof(struct traceControl_s))
#define namtblSiz( ents )       (((uint32_t)sizeof(struct traceNamLvls_s)*ents+TRACE_CACHELINE)&~(TRACE_CACHELINE-1))
#define entSiz( siz_msg, num_params ) ( sizeof(struct traceEntryHdr_s)\
    + sizeof(uint64_t)*num_params /* NOTE: extra size for i686 (32bit processors) */\
    + siz_msg )
#define traceMemLen( siz_cntl_pages, num_namLvlTblEnts, siz_msg, num_params, num_entries ) \
    (( siz_cntl_pages							\
      + namtblSiz( num_namLvlTblEnts )					\
      + entSiz(siz_msg,num_params)*num_entries				\
      + TRACE_PAGESIZE )						\
     & ~(TRACE_PAGESIZE-1) )

/* The "largest_multiple" method (using (ulong)-1) allows "easy" "add 1"
   I must do the substract (ie. add negative) by hand.
   Ref. ShmRW class (~/src/ShmRW?)
   Some standards don't seem to line "static inline"
   Use of the following seems to produce the same code as the optimized
   code which calls inline idxCnt_add (the c11/c++11 optimizer seem to do what
   this macro does). */
#define IDXCNT_ADD( idxCnt, add )					\
    ((add<0)								\
     ?(((uint32_t)-add>idxCnt)						\
       ?(traceControl_p->largest_multiple-(-add-idxCnt))%traceControl_p->largest_multiple \
       :(idxCnt-(-add))%traceControl_p->largest_multiple		\
       )								\
     :(idxCnt+add)%traceControl_p->largest_multiple			\
    )
#define IDXCNT_DELTA( cur, prv )			\
    ((cur>=prv)						\
     ? cur-prv						\
     : cur-prv-traceControl_p->largest_zero_offset	\
    )


# ifndef TRACE_LOG_FUNCTION
#  if defined(__GXX_WEAK__) || ( defined(__cplusplus) && (__cplusplus >= 199711L) ) || ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )
/* c++98 c99 c++0x c11 c++11 */
#   define TRACE_LOG_FUNCTION(tvp,tid,lvl,...)          trace_user( tvp,tid,lvl,__VA_ARGS__ )
#  else
/* c89 */
#   define TRACE_LOG_FUNCTION(tvp,tid,lvl,msgargs... )  trace_user( tvp,tid,lvl,msgargs )
#  endif   /* __GXX_WEAK__... */

static void trace_user( struct timeval *tvp, int TID, unsigned lvl, const char *msg, ... )
{   va_list ap;
#  ifdef __KERNEL__
    if (!trace_allow_printk) return;
    va_start( ap, msg );
    TRACE_VPRINT( msg, ap );
    va_end( ap );
    TRACE_PRINT("\n");
#  else
    char   obuf[0x1000]; int printed=0;
    if (tvp->tv_sec == 0) TRACE_GETTIMEOFDAY( tvp );
    printed += snprintf( &(obuf[printed])
			, (printed<(int)sizeof(obuf))?sizeof(obuf)-printed:0
			, "%10ld%06ld %2d %2d ",tvp->tv_sec,(long)tvp->tv_usec
			, TID,lvl );
    va_start( ap, msg );
    printed += vsnprintf( &(obuf[printed])
			 , (printed<(int)sizeof(obuf))?sizeof(obuf)-printed:0
			 , msg, ap );
    va_end( ap );
    if (printed < (int)sizeof(obuf))
    {   /* there is room for the \n */
	/* buf first see if it is needed */
	if (obuf[printed-1] != '\n')
	{   obuf[printed++] = '\n'; /* overwriting \0 is OK as we will specify the amount to write */
	    /*printf("added \\n printed=%d\n",printed);*/
	}
	/*else printf("already there printed=%d\n",printed);*/
	write( tracePrintFd, obuf, printed );
    }
    else
    {  /* obuf[sizeof(obuf)-1] has '\0'. see if we should change it to \n */
	if (obuf[sizeof(obuf)-2] == '\n')
	    write( tracePrintFd, obuf, sizeof(obuf)-1 );
	else
	{   obuf[sizeof(obuf)-1] = '\n';
	    write( tracePrintFd, obuf, sizeof(obuf) );
	    /*printf("changed \\0 to \\n printed=%d\n",);*/
	}
    }
#  endif
}
# endif /* TRACE_LOG_FUNCTION */


# if (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"   /* b/c of TRACE_XTRA_UNUSED */
# endif

static void trace( struct timeval *tvp, unsigned lvl, unsigned nargs
                  TRACE_XTRA_UNUSED		  
		  , const char *msg, ... )
{
    va_list ap;

    struct traceEntryHdr_s* myEnt_p;
    char                  * msg_p;
    unsigned long         * params_p;
    unsigned                argIdx;
    uint16_t                get_idxCnt_retries=0;
    uint32_t                myIdxCnt=traceControl_p->wrIdxCnt;

# if defined(__KERNEL__)
    uint32_t desired=IDXCNT_ADD(myIdxCnt,1);
    while (cmpxchg(&traceControl_p->wrIdxCnt,myIdxCnt,desired)!=myIdxCnt)
    {   ++get_idxCnt_retries;
	myIdxCnt=traceControl_p->wrIdxCnt;
	desired = IDXCNT_ADD( myIdxCnt,1);
    }
# elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
    uint32_t desired=IDXCNT_ADD(myIdxCnt,1);
    while (!atomic_compare_exchange_weak(&traceControl_p->wrIdxCnt
					 , &myIdxCnt, desired))
    {   ++get_idxCnt_retries;
	desired = IDXCNT_ADD( myIdxCnt,1);
    }
# else
    uint32_t desired=IDXCNT_ADD(myIdxCnt,1);
    while (cmpxchg(&traceControl_p->wrIdxCnt,myIdxCnt,desired)!=myIdxCnt)
    {   ++get_idxCnt_retries;
	myIdxCnt=traceControl_p->wrIdxCnt;
	desired = IDXCNT_ADD( myIdxCnt,1);
    }
# endif

    if (myIdxCnt == traceControl_p->num_entries)
	traceControl_p->full = 1; /* now we'll know if wrIdxCnt has rolled over */

    TRACE_GETTIMEOFDAY( tvp );  /* hopefully NOT a system call */

    myEnt_p  = idxCnt2entPtr( myIdxCnt );
    msg_p    = (char*)(myEnt_p+1);
    params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);

    myEnt_p->time = *tvp;
    TRACE_TSC32( myEnt_p->tsc );
    myEnt_p->lvl  = lvl;
# if defined(__KERNEL__)
    myEnt_p->pid  = current->tgid;
    myEnt_p->tid  = current->pid;
    myEnt_p->cpu  = raw_smp_processor_id();
# else
    myEnt_p->pid  = tracePid;
    myEnt_p->tid  = traceTid;
#  if defined(__linux__) && !defined(NO_SCHED_GETCPU)
    myEnt_p->cpu  = sched_getcpu();
#  endif
# endif
    myEnt_p->TID  = traceTID;
    myEnt_p->get_idxCnt_retries = get_idxCnt_retries;
    myEnt_p->param_bytes = sizeof(long);

    strncpy(msg_p,msg,traceControl_p->siz_msg);
    /* emulate stack push - right to left (so that arg1 end up at a lower
       address, arg2 ends up at the next higher address, etc. */
    if (nargs)
    {   TRACE_32_DOUBLE_KLUDGE
	    if (nargs > traceControl_p->num_params) nargs=traceControl_p->num_params;
	va_start( ap, msg );
	for (argIdx=0; argIdx<nargs; ++argIdx)
	    params_p[argIdx]=va_arg(ap,unsigned long);
	va_end( ap );
    }
    if (traceControl_p->trigActivePost) /* armed, armed/trigger */
    {
	if (traceControl_p->triggered) /* triggered */
	{
	    if (IDXCNT_DELTA(myIdxCnt,traceControl_p->trigIdxCnt)
		>=traceControl_p->trigActivePost )
	    {   /* I think there should be an indication in the M buffer */
		TRACE_CNTL( "modeM", (uint64_t)0 );   /* calling traceCntl here eliminates the "defined but not used" warning for modules which do not use TRACE_CNTL */
		traceControl_p->trigActivePost = 0;
		/* triggered and trigIdxCnt should be cleared when
		   "armed" (when trigActivePost is set) */
	    }
	    /* else just waiting... */
	}
	else if (traceNamLvls_p[traceTID].T & (1<<lvl))
	{   traceControl_p->triggered = 1;
	    traceControl_p->trigIdxCnt = myIdxCnt;
	}
    }
}   /* trace */

# if (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
#  pragma GCC diagnostic pop
# endif


static void trace_lock( void )
{
    uint32_t desired=1, expect=0, hung=0;
# if defined(__KERNEL__)
    while (cmpxchg(&traceControl_p->spinlock,expect,desired) != expect)
	if (++hung >100000000) { TRACE_PRINT("trace_lock: hung?\n"); break; }
# elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
    while (!atomic_compare_exchange_weak(&traceControl_p->spinlock, &expect, desired))
    {   expect=0;
	if (++hung >100000000) { TRACE_PRINT("trace_lock: hung?\n"); break; }
    }
# else
    while (cmpxchg(&traceControl_p->spinlock,expect,desired) != expect)
	if (++hung >100000000) { TRACE_PRINT("trace_lock: hung?\n"); break; }
# endif
}

static void trace_unlock( void )
{   traceControl_p->spinlock=0;
}

static uint32_t name2tid( const char *name )
{
    uint32_t ii;
    trace_lock();
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
	if (strncmp(traceNamLvls_p[ii].name,name,TRACE_DFLT_NAM_SZ)==0)
	{   trace_unlock();
	    return (ii);
	}
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
	if (traceNamLvls_p[ii].name[0] == '\0')
	{   strncpy(traceNamLvls_p[ii].name,name,TRACE_DFLT_NAM_SZ);
	    traceNamLvls_p[ii].M = 0x1;
	    trace_unlock();
	    return (ii);
	}
    trace_unlock();
    return (traceControl_p->num_namLvlTblEnts-1);
}


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
# if 0 && !defined(__KERNEL__)
    va_start( ap, cmd );
    for (ii=0; ii<nargs; ++ii) /* nargs is number of args AFTER cmd */
	printf("arg%u=0x%llx\n",ii+1,va_arg(ap,unsigned long long));
    va_end( ap );
# endif

    va_start( ap, cmd );

    /* although it may be counter intuitive, this should override
       env.var as it could be used to set a file-per-thread.
       NO!!!  -- I think env will over ride, this will just change
       the default for name/file.
       NOTE: CAN'T HAVE FILE-PER-THREAD unless traceControl_p,traceEntries_p,traceNamLvls_p are THREAD_LOCAL
             CAN'T HAVE NAME-PER-THREAD unless traceTID       is THREAD_LOCAL
    */
# ifndef __KERNEL__
    if (strncmp(cmd,"file",4) == 0)/* THIS really only makes sense for non-thread local-file-for-module or for tracelib.h (non-static implementation) w/TLS to file-per-thread */
    {	traceFile = va_arg(ap,char*);/* this can still be overridden by env.var.; suggest testing w. TRACE_ARGSMAX=10*/
	traceInit(TRACE_NAME);		/* force (re)init */
	va_end(ap); return (0);
    }
    /*if (traceControl_p == NULL) traceInit(TRACE_NAME);*/
    TRACE_INIT_CHECK {};     /* note: allows name2tid to be called in userspace */
# endif

    if (strncmp(cmd,"name",4) == 0)/* THIS really only makes sense for non-thread local-name-for-module or for tracelib.h (non-static implementation) w/TLS to name-per-thread */
    {	traceName = va_arg(ap,char*);/* this can still be overridden by env.var. IF traceInit(TRACE_NAME) is called; suggest testing w. TRACE_ARGSMAX=10*/
	traceTID = name2tid( traceName );/* doing it this way allows this to be called by kernel module */
    }
    else if (strncmp(cmd,"trig",4) == 0)    /* takes 2 args: lvlsMsk, postEntries */
    {
	uint64_t lvlsMsk=va_arg(ap,uint64_t);
	unsigned post_entries=va_arg(ap,uint64_t);
	if (   (traceNamLvls_p[traceTID].M&lvlsMsk)
	    && !traceControl_p->trigActivePost )
	{   traceNamLvls_p[traceTID].T       = lvlsMsk;
	    traceControl_p->trigActivePost   = post_entries?post_entries:1; /* must be at least 1 */
	    traceControl_p->triggered        = 0;
	    traceControl_p->trigIdxCnt       = 0;
	}
    }
    else if (strncmp(cmd,"lvlmsk",6) == 0)   /* CURRENTLY TAKE just 1 arg: lvl */
    {   unsigned ee;
	uint64_t lvl=va_arg(ap,uint64_t);
	if ((cmd[6])&&(cmd[7]=='g')){ ii=0;        ee=traceControl_p->num_namLvlTblEnts; }
	else                        { ii=traceTID; ee=traceTID+1; }
	switch (cmd[6])
	{
	case 'M': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].M = lvl; break;
	case 'S': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].S = lvl; break;
	case 'T': for ( ; ii<ee; ++ii) traceNamLvls_p[ii].T = lvl; break;
	default: ret=-1;
	}
    }
    else if (strncmp(cmd,"lvlset",6) == 0)   /* takes 3 args: M S T ((0 val ==> no-op) */
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
    else if (strncmp(cmd,"lvlclr",6) == 0)   /* takes 3 args: M S T ((0 val ==> no-op) */
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
	    ret=traceControl_p->mode.mode;
	    if (nargs==1)
	    {   uint32_t mode=va_arg(ap,uint64_t);
		union trace_mode_u tmp;
		tmp.mode = mode;
# ifndef __KERNEL__
	        if (traceControl_p == &traceControl) tmp.bits.M=0;
# endif
		traceControl_p->mode = tmp;
	    }
	    break;
	case 'M':
	    ret=traceControl_p->mode.bits.M;
# ifndef __KERNEL__
	    if (traceControl_p == &traceControl) break;
# endif
	    if (nargs==1)
	    {   uint32_t mode=va_arg(ap,uint64_t);
		traceControl_p->mode.bits.M = mode;
	    }
	    break;
	case 'S':
	    ret=traceControl_p->mode.bits.S;
	    if (nargs==1)
	    {   uint32_t mode=va_arg(ap,uint64_t);
		traceControl_p->mode.bits.S = mode;
	    }
	    break;
	default:
	    ret=-1;
	}
    }
    else if (strcmp(cmd,"reset") == 0) 
    {
	traceControl_p->full
	    = traceControl_p->wrIdxCnt
	    = traceControl_p->trigIdxCnt
	    = traceControl_p->trigActivePost
	    = 0;
	traceControl_p->triggered = 0;
    }
    else
    {   ret = -1;
    }
    va_end(ap);
# ifdef __KERNEL__
    if (ret==-1) printk(  KERN_ERR "TRACE: invalid control string %s nargs=%d\n", cmd, nargs );
# else
    if (ret==-1) fprintf( stderr, "TRACE: invalid control string %s nargs=%d\n", cmd, nargs );
# endif
    return (ret);
}   /* traceCntl */



# ifndef __KERNEL__


/* RETURN "created" status */
static int trace_mmap_file( const char *_file
			   , int       *memlen   /* in/out -- in for when file created, out when not */
			   , struct traceControl_s **t_p )
{
    int                    fd;
    uint8_t               *rw_p;
    off_t                  off;
    char                   path[PATH_MAX];
    char                  *logname=getenv("LOGNAME"); /* login/user name */
    int			   created=0;
    int			   stat_try=0;

    snprintf( path, PATH_MAX, _file, logname?logname:"");/* in case, for some strange reason, LOGNAME does not exist */
    if ((fd=open(path,O_RDWR|O_CREAT|O_EXCL,0666)) != -1)
    {   /* successfully created new file - must init */
	uint8_t one_byte='\0';
	off = lseek( fd, (*memlen)-1, SEEK_SET );
	if (off == (off_t)-1) { perror("lseek"); *t_p=&traceControl;return (0); }
	write( fd, &one_byte, 1 );
	created = 1;
    }
    else
    {   struct stat           statbuf;
	struct traceControl_s *tmp_traceControl_p;
	/* must verify that it already exists */
	fd=open(path,O_RDWR);
	if (fd == -1)
	{   fprintf( stderr,"TRACE: open(%s)=%d errno=%d pid=%d\n", path, fd, errno, tracePid );
	    *t_p=&traceControl;
	    return (0);
	}
	/*printf( "trace_mmap_file - fd=%d\n",fd );*/ /*interesting in multithreaded env.*/
	if (fstat(fd,&statbuf) == -1)
	{   perror("fstat");
	    close( fd );
	    *t_p=&traceControl;
	    return (0);
	}
	while (statbuf.st_size < (off_t)sizeof(struct traceControl_s))
	{   fprintf(stderr,"stat again\n");
	    if (   ((stat_try++ >= 30)         && (fprintf(stderr,"too many stat tries\n"),1))
		|| ((fstat(fd,&statbuf) == -1) && (perror("fstat"),1)) )
	    {   close( fd );
		*t_p=&traceControl;
		return (0);
	    }
	}

	tmp_traceControl_p = (struct traceControl_s *)mmap( NULL, sizeof(struct traceControl_s)
					 , PROT_READ
					 , MAP_SHARED, fd, 0 );
	if (tmp_traceControl_p == (struct traceControl_s *)-1)
	{   perror( "mmap(NULL,sizeof(struct traceControl_s),PROT_READ,MAP_SHARED,fd,0) error" );
	    *t_p=&traceControl;
	    return (0);
	}
	if (  (tmp_traceControl_p->trace_initialized != 1)
	    /*||(tmp_traceControl_p->siz_entry         != 256)
	    ||(tmp_traceControl_p->num_entries       != 50000)
	    ||(tmp_traceControl_p->num_namLvlTblEnts != 20)
	    ||(tmp_traceControl_p->memlen            != (unsigned)*memlen)
	    ||(tmp_traceControl_p->siz_msg           != 128)*/ )
	{   printf("Trace file not initialzed; consider (re)moving it.\n");
	    close( fd );
	    *t_p=&traceControl;
	    return (0);
	}
	/*sleep(1);*/
	*memlen = tmp_traceControl_p->memlen;
	munmap( tmp_traceControl_p, sizeof(struct traceControl_s) ); /* throw this mapping out */
    }


    /* I MUST allocate/grab a contiguous vm address space! [in testing threads (where address space
       is shared (obviously)), thread creation allocates vm space which can occur between
       these two calls] */
    rw_p = (uint8_t*)mmap( NULL,*memlen,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0 );
    if (rw_p == (void *)-1)
    {   perror("Error:mmap(NULL,*memlen,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0)");
	printf( "(*memlen)=%d\n", (*memlen) );
	close( fd );
	*t_p=&traceControl;
	return (0);
    }

    /* If I didn't create it, go back and make the first "page" read-only */
    if (created == 0)
    {
	*t_p = (struct traceControl_s *)mmap( rw_p, TRACE_PAGESIZE, PROT_READ
					     , MAP_SHARED|MAP_FIXED, fd, 0 );
	if (*t_p == (struct traceControl_s *)-1)
	{   perror( "Error: mmap(rw_p,TRACE_PAGESIZE,PROT_READ,MAP_SHARED|MAP_FIXED,fd,0)");
	    printf( "(*memlen)=%d\n", (*memlen) );
	    munmap( rw_p, *memlen );
	    close( fd );
	    *t_p=&traceControl;
	    return (0);
	}
	if (*t_p != (struct traceControl_s *)rw_p)
	{   printf( "traceInit mmap read-only returned invalid address\n" );
	    munmap( rw_p, *memlen );
	    close( fd );
	    *t_p=&traceControl;
	    return (0);
	}
    }
    else
	*t_p = (struct traceControl_s *)rw_p;

    /* The POSIX mmap man page says:
       The mmap() function shall add an extra reference to the file
       associated with the file descriptor fildes which is not removed by a
       subsequent  close() on that file descriptor.  This reference shall
       be removed when there are no more mappings to the file.
    */
    close( fd );
    return (created);
}   /* trace_mmap_file */

# endif	/* not __KERNEL__*/


# if !defined(__KERNEL__) || defined(TRACE_IMPL)

static int traceInit( const char *_name )
{
    int         memlen;
    uint32_t    msgmax_, argsmax_, numents_, namtblents_;
    int		I_created=0;
#  ifndef __KERNEL__
    int         activate=0;
    const char *_file;
    const char *cp;

    if(traceTid==0) /* traceInit may be called w/ or w/o checking traceTid */
    {   traceTid=syscall(SYS_GETTID);
	tracePid = getpid();  /* do/re-do -- it may be forked process */
#   ifdef TRACE_DEBUG_INIT
	if (traceControl_p == NULL) printf("init both (tid=%d)\n",traceTid );
	else                        printf("init just traceTid (tid=%d)\n",traceTid);
#   endif
    }
#   ifdef TRACE_DEBUG_INIT
    else if (traceControl_p == NULL) printf("init just traceControl_p (tid=%d)\n",traceTid);
#   endif

    if (traceControl_p == NULL)
    {
	/*const char *conf=getenv("TRACE_CONF"); need params,msg_sz,num_entries,num_namLvlTblEnts */
	if (_name != NULL){if(getenv("TRACE_NAME")) activate=1;}
	else if (!((_name=getenv("TRACE_NAME"))&&(*_name!='\0')&&(activate=1))) _name=traceName;
	if (!((_file=getenv("TRACE_FILE"))&&(*_file!='\0')&&(activate=1))) _file=traceFile;
	if ((cp=getenv("TRACE_ARGSMAX"))  &&(*cp)&&(activate=1)) argsmax_=strtoul(cp,NULL,0);else argsmax_   =TRACE_DFLT_MAX_PARAMS;
	/* use _MSGMAX= so exe won't override and _MSGMAX won't activate; use _MSGMAX=0 to activate with default MAX_MSG */
	((cp=getenv("TRACE_MSGMAX"))    &&(*cp)&&(activate=1)&&(msgmax_     =strtoul(cp,NULL,0)))||(msgmax_    =TRACE_DFLT_MAX_MSG_SZ);
	((cp=getenv("TRACE_NUMENTS"))   &&       (numents_    =strtoul(cp,NULL,0))&&(activate=1))||(numents_   =TRACE_DFLT_NUM_ENTRIES);
	((cp=getenv("TRACE_NAMTBLENTS"))&&       (namtblents_ =strtoul(cp,NULL,0))&&(activate=1))||(namtblents_=TRACE_DFLT_NAMTBL_ENTS);

	/* TRACE_LVLS and TRACE_PRINT_FD can be used when active or inactive */
	if ((cp=getenv("TRACE_PRINT_FD")) && (*cp)) tracePrintFd=strtoul(cp,NULL,0);

	if (!activate)
	{   traceControl_p=&traceControl;
	}
	else
	{
	    if (namtblents_ == 1) namtblents_ = 2; /* If it has been specified in the env. it should be at least 2 */
	    memlen = traceMemLen( cntlPagesSiz(), namtblents_, msgmax_, argsmax_, numents_ );
	    I_created = trace_mmap_file( _file, &memlen, &traceControl_p );
	}
	if (traceControl_p == &traceControl)
	{
	    traceControl_p->num_namLvlTblEnts = sizeof(traceNamLvls)/sizeof(traceNamLvls[0]);
	    traceInitNames();

	    /* now, to allow test in trace_cntl.c to proceed */
	    traceControl_p->num_entries      = 1;
	    traceControl_p->largest_multiple = (uint32_t)-1 - ((uint32_t)-1 % 1);

	    if ((cp=getenv("TRACE_LVLS"))     && (*cp))
	    {   /* Calling traceCntl here causes general circular dependency
		   (b/c traceCntl calls traceInit) (but never infinite
		   loop). But mainly, calling traceCntl here redemiats the
		   "warning: 'traceCntl' defined but not used" that would happen
		   in the user space use case (currently no traceInit/traceCntl
		   for modules) */
		TRACE_CNTL( "lvlmskS", strtoull(cp,NULL,0) );
		TRACE_CNTL( "modeS", 1LL );
	    }
	    return (0);
	}
#  else  /* ifndef __KERNEL__ */
    {
	msgmax_     =msgmax;	 /* module_param */
	argsmax_    =argsmax;	 /* module_param */
	numents_    =numents;	 /* module_param */
	namtblents_ =namtblents; /* module_param */
	printk("numents_=%d msgmax_=%d argsmax_=%d namtblents_=%d\n"
	       ,numents_,   msgmax_,   argsmax_,   namtblents_ );
	memlen = traceMemLen( cntlPagesSiz(), namtblents_, msgmax_, argsmax_, numents_ );
	traceControl_p = (struct traceControl_s *)vmalloc( memlen );
	I_created = 1;  /* KERNEL always creates  (no verification against existing needed) */
#  endif

	/* this is needed in order to initNames */
	/* ADD TO ABOVE, FOR WHEN (I_created==0), VERIFICATION THAT cntlPagesSiz() == size from file */
	traceNamLvls_p = (struct traceNamLvls_s *)			\
	    ((unsigned long)traceControl_p+cntlPagesSiz());

	if (I_created)
	{   struct timeval tv;
	    TRACE_GETTIMEOFDAY( &tv );
	    strncpy( traceControl_p->version_string, TRACE_REV, sizeof(traceControl_p->version_string) );
	    traceControl_p->version_string[sizeof(traceControl_p->version_string)-1] = '\0';
	    traceControl_p->create_tv_sec       = (uint32_t)tv.tv_sec;
	    traceControl_p->num_params          = argsmax_;
	    traceControl_p->siz_msg             = msgmax_;
	    traceControl_p->siz_entry           = entSiz( msgmax_, argsmax_ );
	    traceControl_p->num_entries         = numents_;
	    traceControl_p->largest_multiple    = (uint32_t)-1 - ((uint32_t)-1 % numents_);
	    traceControl_p->largest_zero_offset = ((uint32_t)-1 % numents_) +1;
	    traceControl_p->num_namLvlTblEnts   = namtblents_;
	    traceControl_p->memlen              = memlen;

	    traceControl_p->spinlock            = 0;
	    TRACE_CNTL( "reset" );
	    traceControl_p->mode.mode           = 0;
	    traceControl_p->mode.bits.M         = 1;

	    traceInitNames();

	    traceControl_p->trace_initialized = 1;
	}

	/* this depends on the actual value of the num_namLvlTblEnts which
	   may be different from the "calculated" value WHEN the buffer has
	   previously been configured */
	traceEntries_p = (struct traceEntryHdr_s *)	\
	    ((unsigned long)traceNamLvls_p+namtblSiz(traceControl_p->num_namLvlTblEnts));
    }   /* if KERNEL - end "{"; else end "if (traceControl_p==NULL)" */
    if (_name == NULL) _name=traceName; /* for when naming a thread */
    traceTID = name2tid( _name );
    /*printf("traceTID=%d\n",traceTID);*/
#  ifndef __KERNEL__
    if ((cp=getenv("TRACE_LVLS"))     && (*cp))
    {   TRACE_CNTL( "lvlmskS", strtoull(cp,NULL,0) );
	TRACE_CNTL( "modeS", 1LL );
    }
#  endif
    return (0);
}   /* traceInit */


static void traceInitNames( void )
{
    unsigned ii;
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
    {   traceNamLvls_p[ii].name[0] = '\0';
	traceNamLvls_p[ii].S = traceNamLvls_p[ii].T = 0;
	traceNamLvls_p[ii].M = 0x1;
    }
#  ifdef __KERNEL__
    strcpy( traceNamLvls_p[0].name,"KERNEL" );
#  endif
    strcpy( traceNamLvls_p[traceControl_p->num_namLvlTblEnts-2].name,"TRACE" );
    strcpy( traceNamLvls_p[traceControl_p->num_namLvlTblEnts-1].name,"_TRACE_" );
}

# endif /* !defined(__KERNEL__) || defined(TRACE_IMPL) */


static struct traceEntryHdr_s* idxCnt2entPtr( uint32_t idxCnt )
{   uint32_t idx;
    off_t    off;
    uint32_t num_entries=traceControl_p->num_entries;
    idx = idxCnt % num_entries;
    off = idx * traceControl_p->siz_entry;
    return (struct traceEntryHdr_s *)((char*)traceEntries_p+off);
}

#endif /* TRACE_LIB */

#endif /* TRACE_H_5216 */
