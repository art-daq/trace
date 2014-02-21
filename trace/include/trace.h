/* This file (Trace.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: trace.h,v $
 // rev="$Revision: 1.29 $$Date: 2014-02-21 18:49:17 $";
 */

#ifndef TRACE_H_5216
#define TRACE_H_5216

#ifndef __KERNEL__

# include <stdio.h>		/* printf */
# include <stdarg.h>		/* va_list */
# include <stdlib.h>		/* getenv, strtoul */
# include <stdint.h>		/* uint64_t */
# include <sys/time.h>           /* timeval */
# include <string.h>		/* strncmp */
# include <fcntl.h>		/* open, O_RDWR */
# include <sys/mman.h>		/* mmap */
# include <unistd.h>		/* lseek */
# include <sys/syscall.h>	/* syscall */
# include <pthread.h>		/* pthread_self */
# if   defined(__cplusplus)      &&      (__cplusplus >= 201103L)
#  include <atomic>		/* atomic<> */
#  define TRACE_ATOMIC_T     std::atomic<uint32_t>
#  define TRACE_THREAD_LOCAL thread_local 
# elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  include <stdatomic.h>		/* atomic_compare_exchange_weak */
#  define TRACE_ATOMIC_T          _Atomic(uint32_t)
#  define TRACE_THREAD_LOCAL      _Thread_local
# else
#  define TRACE_ATOMIC_T          uint32_t
#  define TRACE_THREAD_LOCAL
#  define cmpxchg(ptr, old, new) \
    ({ uint32_t __ret;							\
    uint32_t __old = (old);						\
    uint32_t __new = (new);						\
    volatile uint32_t *__ptr = (volatile uint32_t *)(ptr);		\
    asm volatile("lock cmpxchgl %2,%1"					\
		 : "=a" (__ret), "+m" (*__ptr)				\
		 : "r" (__new), "0" (__old)				\
		 : "memory");						\
    __ret;								\
    })
# endif
# define TRACE_GETTIMEOFDAY( tv ) gettimeofday( tv, NULL )
# define TRACE_DO_TID             if(traceTid==0)traceTid=syscall(SYS_gettid);
# define TRACE_PRINT              printf
# define TRACE_VPRINT             vprintf
# ifndef NO_TRACE
#  define TRACE_INIT_CHECK         if((traceControl_p!=NULL)||(traceInit()==0))
# else
#  define TRACE_INIT_CHECK         if(0)/*whole trace should be optimized out*/
# endif

#else  /* __KERNEL__ */

# include <linux/time.h>              /* do_gettimeofday */
/*# include <linux/printk.h>	       printk, vprintk */
# include <linux/kernel.h>	      /* printk, vprintk */
# include <linux/mm.h>		      /* kmalloc OR __get_free_pages */
# include <linux/vmalloc.h>	      /* __vmalloc, vfree */
# include <linux/spinlock.h>	      /* cmpxchg */
# define TRACE_ATOMIC_T           uint32_t
# define TRACE_THREAD_LOCAL 
# define TRACE_GETTIMEOFDAY( tv ) do_gettimeofday( tv )
# define TRACE_DO_TID
# define TRACE_PRINT              printk
# define TRACE_VPRINT             vprintk
# define TRACE_INIT_CHECK         if((1)||(traceInit()==0))/* should be optimized out */

#endif /* __KERNEL__ */


/* 88,7=192 bytes/ent   96,6=192   128,10=256*/
#define TRACE_DFLT_MAX_MSG_SZ      128
#define TRACE_DFLT_MAX_PARAMS       10
#define TRACE_DFLT_NAMTBL_ENTS      20
#define TRACE_DFLT_NUM_ENTRIES   20000
#define TRACE_DFLT_NAM_SZ            8
#ifndef  TRACE_NAME
# define TRACE_NAME "TRACE"
#endif


#if defined(__GXX_WEAK__) || ( defined(__cplusplus) && (__cplusplus >= 199711L) ) || ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )

# define TRACE( lvl, ... ) do \
    {   TRACE_INIT_CHECK		\
            if (  (traceControl_p->mode.bits.M && (traceNamLvls_p[traceTID].M & (1<<lvl))) \
                ||(traceControl_p->mode.bits.S && (traceNamLvls_p[traceTID].S & (1<<lvl))) ) \
                trace( lvl, TRACE_ARGS(__VA_ARGS__)-1 \
                      TRACE_XTRA_PASSED	\
                      , __VA_ARGS__ );				\
    } while (0)

# define TRACE_ARGS(...) TRACE_ARGS_HELPER1(__VA_ARGS__,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0) /* 0 here but not below */
# define TRACE_ARGS_HELPER1(...) TRACE_ARGS_HELPER2(__VA_ARGS__)
# define TRACE_ARGS_HELPER2(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n, ...) n
# ifndef NO_TRACE
#  define TRACE_CNTL( ... ) traceCntl( TRACE_ARGS(__VA_ARGS__), __VA_ARGS__ )
# else
#  define TRACE_CNTL( ... ) (0)
# endif

#else    /* __GXX_WEAK__... */

# define TRACE( lvl, msgargs... ) do		\
    {   TRACE_INIT_CHECK		\
            if (  (traceControl_p->mode.bits.M && (traceNamLvls_p[traceTID].M & (1<<lvl))) \
                ||(traceControl_p->mode.bits.S && (traceNamLvls_p[traceTID].S & (1<<lvl))) ) \
	        trace( lvl, TRACE_ARGS(msgargs)-1				\
                      TRACE_XTRA_PASSED		\
                      , msgargs );				\
    } while (0)

# define TRACE_ARGS(args...) TRACE_ARGS_HELPER1(args,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
# define TRACE_ARGS_HELPER1(args...) TRACE_ARGS_HELPER2(args)
# define TRACE_ARGS_HELPER2(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n, x...) n
# ifndef NO_TRACE
#  define TRACE_CNTL( cmdargs... ) traceCntl( TRACE_ARGS(cmdargs), cmdargs )
# else
#  define TRACE_CNTL( cmdargs... ) (0)
# endif

#endif   /* __GXX_WEAK__... */


#if   defined(__i386__)
# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
  static void trace(unsigned,unsigned,const char *,...)__attribute__((format(printf,3,4)));
# define TRACE_VA_LIST_INIT     (va_list)&params_p[0]
# define TRACE_ENT_FILLER       uint32_t x[2];
# define TRACE_32_DOUBLE_KLUDGE nargs*=2;    /* kludge to support potential double in msg fmt */
# define TRACE_TSC32( low )     __asm__ __volatile__ ("rdtsc;movl %%eax,%0":"=m"(low)::"eax","edx")
#elif defined(__x86_64__)
# define TRACE_XTRA_PASSED      ,0,0,0, .0,.0,.0,.0,.0,.0,.0,.0
# define TRACE_XTRA_UNUSED      ,long l0,long l1,long l2,double d0,double d1,double d2,double d3,double d4,double d5,double d6,double d7
  static void trace(unsigned,unsigned TRACE_XTRA_UNUSED,const char *,...)__attribute__((format(printf,14,15)));
# define TRACE_VA_LIST_INIT     {{6*8,6*8+9*16,&params_p[0],&params_p[0]}}
# define TRACE_ENT_FILLER
# define TRACE_32_DOUBLE_KLUDGE
# define TRACE_TSC32( low )     __asm__ __volatile__ ("rdtsc" : "=a" (low) : : "edx")
#else
# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
  static void trace(unsigned,unsigned,const char *,...)__attribute__((format(printf,3,4)));
# define TRACE_VA_LIST_INIT     {&params_p[0]}
# define TRACE_ENT_FILLER
# define TRACE_32_DOUBLE_KLUDGE if(sizeof(long)==4)nargs*=2;
# define TRACE_TSC32( low )     low=0
#endif

struct traceControl_s
{
    uint32_t       num_params;
    uint32_t       siz_msg;
    uint32_t       siz_entry;
    uint32_t       num_entries;
    uint32_t       largest_multiple;
    uint32_t       num_namLvlTblEnts;/* these and above would be read only if */
    int32_t        trace_initialized;/* in kernel */
    uint32_t       memlen;
    uint32_t       page_align[1016]; /* allow mmap page readonly */

    TRACE_ATOMIC_T wrIdxCnt;	/* 32 bit */
    uint32_t       cacheline1[15];

    union
    {   struct
	{   uint32_t M:1; /* b0 high speed circular Memory */
	    uint32_t S:1; /* b1 printf (formatted) to Screen/Stdout */
	}   bits;
	uint32_t  mode;
    }             mode;
    union
    {   struct
	{   uint32_t M:1; /* b0 high speed circular Memory */
	    uint32_t S:1; /* b1 printf (formatted) to stdout */
	}   bits;
	uint32_t  mode;
    }             trigOffMode;    /* can configurably cause Print to stop */
    uint32_t      trigIdxCnt;   /* BASED ON "M" mode Counts */
    int32_t       triggered;
    uint32_t	  trigActivePost;
    int32_t       full;
    uint32_t      xtra[2];
};

struct traceEntryHdr_s
{   struct timeval time;
    TRACE_ENT_FILLER	/* because timeval is larger on x86_64 (16 bytes compared to 8 for i686) */
    int32_t        lvl;
    pid_t          pid;   /* system info */
    pid_t          tid;   /* system info - "thread id" */
    uint32_t       TID;   /* Trace ID ==> idx into lvlTbl, namTbl */
    /* int32_t       cpu; -- kernel sched switch will indicat this info */
    uint32_t       get_idxCnt_retries;
    uint32_t       param_bytes;
    uint64_t       tsc;
};

struct traceNamLvls_s
{   uint64_t      M;
    uint64_t      S;
    uint64_t      T;
    char          name[TRACE_DFLT_NAM_SZ];
};

#ifndef __KERNEL__
static struct traceNamLvls_s  traceNamLvls[3];
static struct traceNamLvls_s  *traceNamLvls_p=&traceNamLvls[0];
static struct traceControl_s  *traceControl_p=NULL;
static struct traceEntryHdr_s *traceEntries_p;
#else
extern struct traceNamLvls_s  *traceNamLvls_p;
extern struct traceControl_s  *traceControl_p;
extern struct traceEntryHdr_s *traceEntries_p;
#endif


static int                      traceTID=0;  /* idx into lvlTbl, namTbl */
static pid_t                    tracePid=0;
static TRACE_THREAD_LOCAL pid_t traceTid=0;  /* thread id */


/* forward declarations, important functions */
static struct traceEntryHdr_s*  idxCnt2entPtr( uint32_t idxCnt );
static uint32_t                 entSiz( uint32_t siz_msg, uint32_t num_params );
static int                      traceMemLen(  int siz_msg
					    , int num_params
					    , int num_namLvlTblEnts
					    , int num_entries
					    , int *namLvls_offset );
static void                     traceInitNames( void );
static uint32_t                 name2tid( const char *name );
#if 0
static uint64_t                 idxCnt_delta( uint32_t wr, uint32_t rd );
static void                     getPtrs(  struct traceControl_s  **cc
					, struct traceEntryHdr_s **ee
					, struct traceNamLvls_s     **ll
					, int siz_lvlTbl );
#endif

/* The "largest_multiple" method (using (ulong)-1) allows "easy" "add 1"
   I must do the substract (ie. add negative) by hand.
   Ref. ShmRW class (~/src/ShmRW?)
   Some standards don't seem to line "static inline"
   Use of the following seems to produce the same code as the optimized
   code which calls inline idxCnt_add (the c11/c++11 optimizer seem to do what
   this macro does). */
#define IDXCNT_ADD( idxCnt, add ) \
    ((add<0)							\
     ?(((uint32_t)-add>idxCnt)						\
       ?(traceControl_p->largest_multiple-(-add-idxCnt))%traceControl_p->largest_multiple \
       :(idxCnt-(-add))%traceControl_p->largest_multiple\
      )						\
     :(idxCnt+add)%traceControl_p->largest_multiple\
    )


/* I've worked the mode<->level checking thing out before ...
   check work/tracePrj/TRACE3...
 */

#if (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

static void trace( unsigned lvl, unsigned nargs
                  TRACE_XTRA_UNUSED		  
		  , const char *msg, ... )
{   struct timeval tv;
    va_list ap;
    int     trig_reset_S=0;

    tv.tv_sec = 0;		/* indicate that we need to get the time */
    TRACE_DO_TID                /* only appliable for user space */

    if (traceControl_p->mode.bits.M && (traceNamLvls_p[traceTID].M & (1<<lvl)))
    {   struct traceEntryHdr_s* myEnt_p;
	char                  * msg_p;
	unsigned long         * params_p;
	unsigned                argIdx;
	uint16_t                get_idxCnt_retries=0;
	uint32_t                myIdxCnt=traceControl_p->wrIdxCnt;

#      if defined(__KERNEL__)
	uint64_t desired=IDXCNT_ADD(myIdxCnt,1);
	while (cmpxchg(&traceControl_p->wrIdxCnt,myIdxCnt,desired)!=myIdxCnt)
	{   ++get_idxCnt_retries;
	    myIdxCnt=traceControl_p->wrIdxCnt;
	    desired = IDXCNT_ADD( myIdxCnt,1);
	}
#      elif (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
	uint64_t desired=IDXCNT_ADD(myIdxCnt,1);
	while (!atomic_compare_exchange_weak(&traceControl_p->wrIdxCnt
					     , &myIdxCnt, desired))
	{   ++get_idxCnt_retries;
	    desired = IDXCNT_ADD( myIdxCnt,1);
	}
#       else
	uint64_t desired=IDXCNT_ADD(myIdxCnt,1);
	while (cmpxchg(&traceControl_p->wrIdxCnt,myIdxCnt,desired)!=myIdxCnt)
	{   ++get_idxCnt_retries;
	    myIdxCnt=traceControl_p->wrIdxCnt;
	    desired = IDXCNT_ADD( myIdxCnt,1);
	}
#       endif

	TRACE_GETTIMEOFDAY( &tv );  /* hopefully NOT a system call */

	myEnt_p  = idxCnt2entPtr( myIdxCnt );
	msg_p    = (char*)(myEnt_p+1);
	params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);

	TRACE_TSC32( myEnt_p->tsc );
	myEnt_p->time = tv;
	myEnt_p->lvl  = lvl;
	myEnt_p->pid  = tracePid;
	myEnt_p->tid  = traceTid;
	myEnt_p->TID  = traceTID;
	/*myEnt_p->cpu  = -1;    maybe don't need this when sched hook show tid<-->cpu */
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
		if ((myIdxCnt-traceControl_p->trigIdxCnt)
		    >=traceControl_p->trigActivePost )
		{   /* I think there should be an indication in the M buffer */
		    traceControl_p->mode.bits.M = 0;
		    if (traceControl_p->trigOffMode.bits.S) trig_reset_S = 1;
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
    }

    if (traceControl_p->mode.bits.S && (traceNamLvls_p[traceTID].S & (1<<lvl)))
    {
	if (tv.tv_sec == 0) TRACE_GETTIMEOFDAY( &tv );
	TRACE_PRINT("%10ld%06ld %2d %5d %d ",tv.tv_sec,tv.tv_usec,lvl,traceTid,nargs);
	va_start( ap, msg );
	TRACE_VPRINT( msg, ap );
	TRACE_PRINT("\n");
	va_end( ap );
    }
    if (trig_reset_S) traceControl_p->mode.bits.S = 0;
}   /* trace */

#if (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
# pragma GCC diagnostic pop
#endif

#ifndef __KERNEL__

static int                    traceInit( void );
static struct traceControl_s  traceControl;

static int traceCntl( int nargs, const char *cmd, ... )
{
    va_list ap;

    if (strncmp(cmd,"file",4) == 0)
    {   /* although it may be counter intuitive, this should override
	   env.var as it could be used to set a file-per-thread.
	   But... what if this negative impacts thread performace???
	*/
    }
    else if (strncmp(cmd,"name",4) == 0)
    {   /* although it may be counter intuitive, this should override
	   env.var as it could be used to set a file-per-thread.
	   But... what if this negative impacts thread performace???
	*/
	/* HOW DOES THE NAME TO TID WORK???? */
    }

    if (traceControl_p == NULL) traceInit();

    va_start( ap, cmd );

    if      (strncmp(cmd,"trig",4) == 0)    /* takes 3 args: modeMsks, lvlsMsk, postEntries */
    {
	uint32_t modeMsk=va_arg(ap,uint64_t);
	uint64_t lvlsMsk=va_arg(ap,uint64_t);
	unsigned post_entries=va_arg(ap,unsigned);
	if (   (traceControl_p->mode.bits.M && (traceNamLvls_p[traceTID].M&lvlsMsk))
	    && !traceControl_p->trigActivePost )
	{   traceNamLvls_p[traceTID].T       = lvlsMsk;
	    traceControl_p->trigActivePost   = post_entries?post_entries:1; /* must be at least 1 */
	    traceControl_p->triggered        = 0;
	    traceControl_p->trigIdxCnt       = 0;
	    traceControl_p->trigOffMode.mode = modeMsk;
	}
    }
    else if (strcmp(cmd,"lvlmskM") == 0)   /* CURRENTLY TAKE just 1 arg: lvl */
    {   
	uint64_t lvl=va_arg(ap,uint64_t);
	traceNamLvls_p[traceTID].M = lvl;
	/*printf("set level for TID=%d to 0x%llx\n", traceTID, (unsigned long long)lvl );*/
    }
    else if (strcmp(cmd,"lvlmskS") == 0)   /* CURRENTLY TAKE just 1 arg: lvl */
    {   
	uint64_t lvl=va_arg(ap,uint64_t);
	if (  (strcmp(traceNamLvls_p[traceTID].name,"KERNEL")==0)
	    &&(lvl&0xff000000) )
	{   fprintf(stderr, "not allowed to set some level bits which could "
		    "cause KERNEL issues\n");
	} else
	{   traceNamLvls_p[traceTID].S = lvl;
	    /*printf("set level for TID=%d to 0x%llx\n", traceTID, (unsigned long long)lvl );*/
	}
    }
    else if (strcmp(cmd,"lvlmskT") == 0)   /* CURRENTLY TAKE just 1 arg: lvl */
    {   
	uint64_t lvl=va_arg(ap,uint64_t);
	traceNamLvls_p[traceTID].T = lvl;
	/*printf("set level for TID=%d to 0x%llx\n", traceTID, (unsigned long long)lvl );*/
    }
    else if (strcmp(cmd,"mode") == 0)
    {   
	uint32_t mode=va_arg(ap,uint64_t);
	traceControl_p->mode.mode = mode;
    }
    else if (strcmp(cmd,"modeset") == 0)
    {   
	uint32_t mode=va_arg(ap,uint64_t);
	traceControl_p->mode.mode |= mode;
    }
    else if (strcmp(cmd,"modeclr") == 0)
    {   
	uint32_t mode=va_arg(ap,uint64_t);
	traceControl_p->mode.mode &= ~mode;
    }
    else if (strncmp(cmd,"info",4) == 0) 
    {
	uint32_t wrSav=traceControl_p->wrIdxCnt;
	uint32_t used=((wrSav<=traceControl_p->num_entries)
		       ?wrSav
		       :traceControl_p->num_entries);
	printf("trace_initialized =%d\n"
	       "mode              =0x%x\n"
	       "writeIdxCount     =0x%08x entries used: %u\n"
               "largestMultiple   =0x%08x\n"
	       "trigIdxCnt        =0x%08x\n"
	       "triggered         =%d\n"
	       "trigActivePost    =%u\n"
	       "traceLevel        =0x%*llx 0x%*llx\n"
	       "num_entries       =%u\n"
	       "max_msg_sz        =%u  includes system inforced terminator\n"
	       "max_params        =%u\n"
	       "entry_size        =%u\n"
	       "namLvlTbl_ents    =%u\n"
	       "wrIdxCnt offset   =%p\n"
	       "namLvls offset    =0x%lx\n"
	       "buffer_offset     =0x%lx\n"
	       "memlen            =%u\n"
	       , traceControl_p->trace_initialized
	       , traceControl_p->mode.mode
	       , wrSav, used
	       , traceControl_p->largest_multiple
	       , traceControl_p->trigIdxCnt
	       , traceControl_p->triggered
	       , traceControl_p->trigActivePost
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceNamLvls_p[traceTID].M
	       , (int)sizeof(uint64_t)*2, (unsigned long long)traceNamLvls_p[traceTID].S
	       , traceControl_p->num_entries
	       , traceControl_p->siz_msg
	       , traceControl_p->num_params
	       , traceControl_p->siz_entry
	       , traceControl_p->num_namLvlTblEnts
	       , (void*)&((struct traceControl_s*)0)->wrIdxCnt
	       , (unsigned long)traceNamLvls_p - (unsigned long)traceControl_p
	       , (unsigned long)traceEntries_p - (unsigned long)traceControl_p
	       , traceControl_p->memlen
	       );
    }
    else if (strcmp(cmd,"reset") == 0) 
    {
	if (traceControl_p == NULL) traceInit();
	traceControl_p->full
	    = traceControl_p->wrIdxCnt
	    = traceControl_p->trigIdxCnt
	    = traceControl_p->trigActivePost
	    = 0;
	traceControl_p->triggered = 0;
    }
    else if (strcmp(cmd,"tids") == 0) 
    {   unsigned ii;
	for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
	{
	    if (traceNamLvls_p[ii].name[0] != '\0')
	    {   printf( "%3d %*s 0x%16lx 0x%16lx 0x%16lx\n", ii
		       , (int)sizeof(traceNamLvls_p->name)
		       , traceNamLvls_p[ii].name
		       , traceNamLvls_p[ii].M
		       , traceNamLvls_p[ii].S
		       , traceNamLvls_p[ii].T
		       );
	    }
	}
    }
    else if (strcmp(cmd,"show") == 0) 
    {
	uint32_t rdIdx=traceControl_p->wrIdxCnt;
	uint32_t max=(rdIdx<=traceControl_p->num_entries)
	    ?rdIdx:traceControl_p->num_entries;
	unsigned printed=0;
	struct traceEntryHdr_s* myEnt_p;
	char                  * msg_p;
	unsigned long         * params_p;

	rdIdx = IDXCNT_ADD( rdIdx, -1 );
	for (printed=0; printed<max; ++printed)
	{   myEnt_p = idxCnt2entPtr( rdIdx );
	    msg_p    = (char*)(myEnt_p+1);
	    params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);

	    printf("%6u %10ld%06ld %10u %2d %5d "
		   , printed
		   , myEnt_p->time.tv_sec, myEnt_p->time.tv_usec
		   , (unsigned)myEnt_p->tsc
		   , myEnt_p->lvl, myEnt_p->tid );
	    if (myEnt_p->get_idxCnt_retries) printf( "%u ", myEnt_p->get_idxCnt_retries );
	    else                             printf( ". " );

	    /* MUST change all %s possibilities to %p */
	    /* MUST change %* beyond num_params to %% */
	    msg_p[traceControl_p->siz_msg - 1] = '\0';

	    /*typedef unsigned long parm_array_t[1];*/
	    /*va_start( ap, params_p[-1] );*/
	    {   /* Ref. http://andrewl.dreamhosters.com/blog/variadic_functions_in_amd64_linux/index.html
		 */
		va_list ap=TRACE_VA_LIST_INIT;
		vprintf( msg_p, ap );
	    }

	    printf("\n");

	    rdIdx = IDXCNT_ADD( rdIdx, -1 );
	}
    }
    else
    {   fprintf( stderr, "TRACE: invalid control string %s nargs=%d\n", cmd, nargs );
	return (-1);
    }

    va_end(ap);
    return (0);
}   /* traceCntl */

static int traceInit(void)
{   int   fd;
    const char *levl=getenv("TRACE_LVL");
    const char *mode=getenv("TRACE_MODE");
    const char *path=getenv("TRACE_FILE");
    const char *name=getenv("TRACE_NAME");
    /*const char *conf=getenv("TRACE_CONF"); need params,msg_sz,num_entries,num_namLvlTblEnts */
    int      num_namLvlTblEnts=TRACE_DFLT_NAMTBL_ENTS;
    int      num_params       =TRACE_DFLT_MAX_PARAMS;
    int      siz_msg          =TRACE_DFLT_MAX_MSG_SZ;
    int      num_entries      =TRACE_DFLT_NUM_ENTRIES;
    int      mmlen;
    int      siz_cntl_pages;
    char     lvltmpbuf[80];

    mmlen = traceMemLen( siz_msg, num_params, num_namLvlTblEnts, num_entries
			, &siz_cntl_pages );

    if (path == NULL) path="/tmp/trace_buffer";
    if (mode == NULL) mode="3";
    if (levl == NULL) {sprintf(lvltmpbuf,"0x%llx",(unsigned long long)-1);levl=lvltmpbuf;}
    if (name == NULL) name=TRACE_NAME;

    /* need to special processing (lock file?) for create */
    if ((fd=open(path,O_RDWR|O_CREAT,0666)) == -1)
    {    fprintf( stderr,"open of %s returned %d\n", path, fd );
	 traceControl_p=&traceControl;
    }
    else
    {
	uint8_t *rw_p;
	off_t    off = lseek( fd, 0, SEEK_END );
	if (off == (off_t)-1) { perror("lseek"); exit(1); }
	if (off != mmlen)
	{   uint8_t one_byte='\0';
	    off = lseek( fd, mmlen-1, SEEK_SET );
	    if (off == (off_t)-1) { perror("lseek"); exit(1); }
	    write( fd, &one_byte, 1 );
	}

# if 0  /* currently can't get 1st page of kernel memory read-only with single mmap call :( */
	traceControl_p = (struct traceControl_s *)mmap( NULL, mmlen
						       , PROT_READ|PROT_WRITE
						       , MAP_SHARED, fd, 0 );
	if (traceControl_p == (struct traceControl_s *)-1)
	{   perror( "mmap(NULL,0x1000,PROT_READ,MAP_PRIVATE,fd,0) error" );
	    printf( "mmlen=%d\n", mmlen );
	    traceControl_p=&traceControl;
	}
	printf("initialize=%d namLvlTblEnts=%u\n"
	       , traceControl_p->trace_initialized
	       , traceControl_p->num_namLvlTblEnts );
# else
	rw_p = (uint8_t*)mmap( NULL, mmlen-0x1000
			      , PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x1000 );
	if (rw_p == (void *)-1)
	{   perror( "mmap(NULL,mmlen-0x1000,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0) error" );
	    printf( "mmlen=%d\n", mmlen );
	    traceControl_p=&traceControl;
	}

	off = (off_t)&((struct traceControl_s *)0)->wrIdxCnt;
	traceControl_p = (struct traceControl_s *)mmap( rw_p-off, 0x1000
						       , PROT_READ|PROT_WRITE
						       , MAP_SHARED, fd, 0 );
	if (traceControl_p == (struct traceControl_s *)-1)
	{   perror( "mmap(NULL,0x1000,PROT_READ,MAP_PRIVATE,fd,0) error" );
	    printf( "mmlen=%d\n", mmlen );
	    traceControl_p=&traceControl;
	}

	if (rw_p != ((uint8_t*)traceControl_p)+0x1000)
	    printf( "traceControl_p=%p rw_p=%p\n",(void*)traceControl_p,rw_p );
# endif

	tracePid = getpid();

	traceNamLvls_p = (struct traceNamLvls_s *)			\
	    ((unsigned long)traceControl_p+siz_cntl_pages);

	traceEntries_p = (struct traceEntryHdr_s *)	\
	    ((unsigned long)traceNamLvls_p
	     +sizeof(struct traceNamLvls_s)*num_namLvlTblEnts);

	if (traceControl_p->trace_initialized == 0)
	{   traceControl_p->trace_initialized = 1;
	    traceControl_p->num_namLvlTblEnts = num_namLvlTblEnts;
	    traceControl_p->num_params       = num_params;
	    traceControl_p->siz_msg          = siz_msg;
	    traceControl_p->num_entries      = num_entries;
	    traceControl_p->largest_multiple = (uint32_t)-1 - ((uint32_t)-1 % num_entries);
	    traceControl_p->siz_entry        = entSiz( siz_msg, num_params );

	    traceInitNames();

	    /* MUST THINK ABOUT "lvlM" and/or "lvlP" ???
	       AND        "modeM" and/or "modeP" ???
	    */
	    traceCntl( 2, "lvlmskM",  strtoul(levl,NULL,0) );
	    traceCntl( 2, "mode", strtoul(mode,NULL,0) );
	}
	traceTID = name2tid( name );
    }
    return (0);
}   /* traceInit - userspace */

#else /* __KERNEL__ */

static int traceInit(void)
{
    int  memlen;
    int  num_namLvlTblEnts=TRACE_DFLT_NAMTBL_ENTS;
    int  num_params=TRACE_DFLT_MAX_PARAMS;
    int  siz_msg=TRACE_DFLT_MAX_MSG_SZ;
    int  num_entries=TRACE_DFLT_NUM_ENTRIES;
    int  siz_cntl_pages;

    memlen = traceMemLen( siz_msg, num_params, num_namLvlTblEnts, num_entries
			 , &siz_cntl_pages );

    printk(  KERN_INFO "init_trace_3 called -- attempt to allocate %d bytes\n"
	   , memlen );

    traceControl_p = (struct traceControl_s *)vmalloc( memlen );
#if 0
    traceControl_p = (struct traceControl_s *)__vmalloc( memlen,GFP_KERNEL,PAGE_KERNEL );
    traceControl_p = (struct traceControl_s *)__vmalloc( memlen,GFP_KERNEL|GFP_DMA32,PAGE_KERNEL_IO );
    traceControl_p = (struct traceControl_s *)__get_free_pages( GFP_KERNEL, get_order(memlen) );
    traceControl_p = (struct traceControl_s *)kmalloc( memlen, GFP_KERNEL );
#endif
    if (!traceControl_p) return -ENOMEM;
    printk("init_trace_3 alloc(%d)=%p\n",memlen,traceControl_p);

    traceControl_p->num_params        = num_params;
    traceControl_p->siz_msg           = siz_msg;
    traceControl_p->siz_entry         = entSiz(  traceControl_p->siz_msg
					       , traceControl_p->num_params );
    traceControl_p->num_entries       = num_entries;
    traceControl_p->largest_multiple  = (uint32_t)-1 - ((uint32_t)-1 % num_entries);
    traceControl_p->num_namLvlTblEnts = num_namLvlTblEnts;
    traceControl_p->trace_initialized = 1;
    traceControl_p->memlen            = memlen;

    traceControl_p->wrIdxCnt         = 0;
    traceControl_p->trigActivePost   = 0;
    traceControl_p->mode.mode        = 0;

    traceNamLvls_p = (struct traceNamLvls_s *)			\
	((unsigned long)traceControl_p+siz_cntl_pages);

    traceEntries_p = (struct traceEntryHdr_s *)	\
	((unsigned long)traceNamLvls_p
	 +sizeof(struct traceNamLvls_s)*num_namLvlTblEnts);

    traceInitNames();
    traceTID=name2tid( "KERNEL" );

    return (0);
}   /* traceInit - kernel */


#endif /* __KERNEL__ */


#if 0
static uint64_t idxCnt_delta( uint32_t wr, uint32_t rd )
{   return ((wr-rd)%traceControl_p->largest_multiple);
}
#endif

static struct traceEntryHdr_s* idxCnt2entPtr( uint32_t idxCnt )
{   uint32_t idx=idxCnt%traceControl_p->num_entries;
    off_t off=idx*traceControl_p->siz_entry;
    return (struct traceEntryHdr_s *)((unsigned long)traceEntries_p+off);
}

static uint32_t entSiz( uint32_t siz_msg, uint32_t num_params )
{
    return (  sizeof(struct traceEntryHdr_s)
	    + sizeof(uint64_t)*num_params /* NOTE: extra size for i686 (32bit processors) */
	    + siz_msg );
}

static int traceMemLen(  int siz_msg, int num_params, int num_namLvlTblEnts
		       , int num_entries, int *namLvls_offset )
{
    int len = sizeof(struct traceControl_s);
    len += 4096;
    len -= len&4095;
    *namLvls_offset = len;
    len += sizeof(struct traceNamLvls_s)*num_namLvlTblEnts
	+ (  sizeof(struct traceEntryHdr_s)
	   + sizeof(unsigned long)*num_params
	   + siz_msg)*num_entries;
    len += 4096;
    len -= len%4096;  /* page align as mapping always likes page align */
    len += 4096; /* make 100% sure there is a bit more for last entry read doubles */
    return (len);
}

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

static uint32_t name2tid( const char *name )
{
    uint32_t ii;
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
	if (strncmp(traceNamLvls_p[ii].name,name,TRACE_DFLT_NAM_SZ)==0) return (ii);
    for (ii=0; ii<traceControl_p->num_namLvlTblEnts; ++ii)
	if (traceNamLvls_p[ii].name[0] == '\0')
	{   strncpy(traceNamLvls_p[ii].name,name,TRACE_DFLT_NAM_SZ);
	    return (ii);
	}
    return (traceControl_p->num_namLvlTblEnts-1);
}
#endif /* TRACE_H_5216 */
