/* This file (Trace.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: trace.h,v $
 // rev="$Revision: 1.1 $$Date: 2014/01/30 17:03:34 $";
 */

#ifndef TRACE_H_5216
#define TRACE_H_5216

#include <stdio.h>		/* printf */
#include <stdarg.h>		/* va_list */
#include <stdlib.h>		/* getenv, strtoul */
#include <stdint.h>		/* uint64_t */
#include <sys/time.h>           /* timeval */
#include <string.h>		/* strncmp */
#include <fcntl.h>		/* open, O_RDWR */
#include <sys/mman.h>		/* mmap */
#include <unistd.h>		/* lseek */
#include <sys/syscall.h>	/* syscall */
#include <pthread.h>		/* pthread_self */
#if   defined(__cplusplus)      &&      (__cplusplus >= 201103L)
# include <atomic>		/* atomic<> */
# define TRACE_ATOMIC_T     std::atomic<uint64_t>
# define TRACE_THREAD_LOCAL thread_local 
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
# include <stdatomic.h>		/* atomic_compare_exchange_weak */
# define TRACE_ATOMIC_T     _Atomic(uint64_t)
# define TRACE_THREAD_LOCAL _Thread_local
#else
# define TRACE_ATOMIC_T     uint64_t
# define TRACE_THREAD_LOCAL 
#endif


#if defined(__GXX_WEAK__) || ( defined(__cplusplus) && (__cplusplus >= 199711L) ) || ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )

# define TRACE( lvl, ... ) do \
    {   if ((traceControl_p!=NULL) || (traceInit()==0))		\
        {   if (  (traceControl_p->mode.s.M && (traceNamLvls_p[traceTID].M & (1<<lvl))) \
                ||(traceControl_p->mode.s.S && (traceNamLvls_p[traceTID].S & (1<<lvl))) ) \
                trace( lvl, N_ARGS(__VA_ARGS__)-1 \
                      TRACE_XTRA_PASSED	\
                      , __VA_ARGS__ );				\
        } \
    } while (0)

# define TRACE_CNTL( ... ) traceCntl( __VA_ARGS__ )
# define N_ARGS(...) N_ARGS_HELPER1(__VA_ARGS__,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0) /* 0 here but not below */
# define N_ARGS_HELPER1(...) N_ARGS_HELPER2(__VA_ARGS__)
# define N_ARGS_HELPER2(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n, ...) n

#else    /* __GXX_WEAK__... */

# define TRACE( lvl, msgargs... ) do		\
    {   if ((traceControl_p!=NULL)  || (traceInit()==0))		\
        {   if (  (traceControl_p->mode.s.M && (traceNamLvls_p[traceTID].M & (1<<lvl))) \
                ||(traceControl_p->mode.s.S && (traceNamLvls_p[traceTID].S & (1<<lvl))) ) \
	        trace( lvl, N_ARGS(msgargs)-1				\
                      TRACE_XTRA_PASSED		\
                      , msgargs );				\
	    } \
    } while (0)

# define TRACE_CNTL( cmdargs... ) traceCntl( cmdargs )
# define N_ARGS(args...) N_ARGS_HELPER1(args,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
# define N_ARGS_HELPER1(args...) N_ARGS_HELPER2(args)
# define N_ARGS_HELPER2(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,n, x...) n

#endif   /* __GXX_WEAK__... */

#if   defined(__i386__)
# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
# define TRACE_VA_LIST_INIT (va_list)&params_p[0]
# define TRACE_ENT_FILLER   uint32_t x[2];
#elif defined(__x86_64__)
# define TRACE_XTRA_PASSED  ,0,1,2,1.0,2.1,3.2,4.3,5.4,6.5,7.6,8.7
# define TRACE_XTRA_UNUSED  ,long l0,long l1,long l2,double d0,double d1,double d2,double d3,double d4,double d5,double d6,double d7
# define TRACE_VA_LIST_INIT {{6*8,6*8+9*16,&params_p[0],&params_p[0]}}
# define TRACE_ENT_FILLER
#else
# define TRACE_XTRA_PASSED
# define TRACE_XTRA_UNUSED
# define TRACE_VA_LIST_INIT {&params_p[0]}
# define TRACE_ENT_FILLER
#endif

static struct traceControl_s
{
    uint32_t       num_params;
    uint32_t       siz_msg;
    uint32_t       siz_entry;
    uint32_t       num_entries;
    uint64_t       largest_multiple;
    uint32_t       num_namLvlTblEnts;/* these and above would be read only if */
    int32_t        trace_initialized;/* in kernel */
    uint32_t       page_align[1016]; /* allow map page readonly */

    TRACE_ATOMIC_T wrIdxCnt;
    uint64_t       cacheline1[3];

    union
    {   struct
	{   uint32_t M:1; /* b0 high speed circular Memory */
	    uint32_t S:1; /* b1 printf (formatted) to Screen/Stdout */
	}   s;
	uint32_t  mode;
    }             mode;
    union
    {   struct
	{   uint32_t M:1; /* b0 high speed circular Memory */
	    uint32_t S:1; /* b1 printf (formatted) to stdout */
	}   s;
	uint32_t  mode;
    }             trigOffMode;    /* can configurably cause Print to stop */
    uint64_t      trigIdxCount;   /* BASED ON "M" mode Counts */
    uint32_t	  trigActivePost;
    int32_t       full;
}   traceControl;
static struct traceControl_s *traceControl_p=NULL;

static struct traceEntryHdr_s
{   struct timeval time;
    TRACE_ENT_FILLER
    int32_t        lvl;
    pid_t          pid;   /* system info */
    pid_t          tid;   /* system info - "thread id" */
    uint32_t       TID;   /* Trace ID ==> idx into lvlTbl, namTbl */
    /* int32_t       cpu; -- kernel sched switch will indicat this info */
    uint32_t       get_idxCnt_retries;
    uint32_t       param_bytes;
    uint64_t       tsc;
}   *traceEntries_p;

static struct traceNamLvls_s
{   uint64_t      M;
    uint64_t      S;
    uint64_t      T;
    char          name[48];
}   traceNamLvls;
static struct traceNamLvls_s *traceNamLvls_p=&traceNamLvls;


static int                      traceTID=0;  /* idx into lvlTbl, namTbl */
static pid_t                    tracePid=0;
static TRACE_THREAD_LOCAL pid_t traceTid=0;  /* thread id */


/* forward declarations, important functions */
static int                      traceInit(void);
static uint64_t                 idxCnt_add( uint64_t idxCnt, int add );
static struct traceEntryHdr_s*  idxCnt2entPtr( uint64_t idxCnt );
static uint32_t                 entSiz( uint32_t siz_msg, uint32_t num_params );
#if 0
static uint64_t                 idxCnt_delta( uint64_t wr, uint64_t rd );
static void                     getPtrs(  struct traceControl_s  **cc
					, struct traceEntryHdr_s **ee
					, struct traceNamLvls_s     **ll
					, int siz_lvlTbl );
#endif


/* I've worked the mode<->level checking thing out before ...
   check work/tracePrj/TRACE3...
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void trace( int lvl, int nargs
                  TRACE_XTRA_UNUSED		  
		  , const char *msg, ... )
{   struct timeval tv;
    va_list ap;
    if (   traceControl_p->trigActivePost
	&&((traceControl_p->wrIdxCnt-traceControl_p->trigIdxCount)
	   >=traceControl_p->trigActivePost))
    {   traceControl_p->mode.mode = 0;
	return;
    }

    gettimeofday( &tv, NULL );  /* hopefully NOT a system call */
    if (traceTid == 0)  traceTid = syscall(SYS_gettid);

    if (traceControl_p->mode.s.M && (traceNamLvls_p[traceTID].M & (1<<lvl)))
    {   struct traceEntryHdr_s* myEnt_p;
	char                  * msg_p;
	unsigned long         * params_p;
	unsigned                argIdx;
	uint64_t                myIdxCnt=traceControl_p->wrIdxCnt;
	uint16_t                get_idxCnt_retries=0;

#      if (defined(__cplusplus)&&(__cplusplus>=201103L)) || (defined(__STDC_VERSION__)&&(__STDC_VERSION__>=201112L))
	uint64_t desired=idxCnt_add(myIdxCnt,1);
	while (!atomic_compare_exchange_weak(&traceControl_p->wrIdxCnt
					     , &myIdxCnt, desired))
	{   ++get_idxCnt_retries;
	    desired = idxCnt_add( myIdxCnt,1);
	}
#       else
	traceControl_p->wrIdxCnt = idxCnt_add(myIdxCnt,1);
#       endif

	myEnt_p  = idxCnt2entPtr( myIdxCnt );
	msg_p    = (char*)(myEnt_p+1);
	params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);

	myEnt_p->time = tv;
	myEnt_p->lvl  = lvl;
	myEnt_p->pid  = tracePid;
	myEnt_p->tid  = traceTid;
	myEnt_p->TID  = traceTID;
	/*myEnt_p->cpu  = -1;    maybe don't need this when sched hook show tid<-->cpu */
	myEnt_p->get_idxCnt_retries = get_idxCnt_retries;
	myEnt_p->param_bytes = sizeof(long);
	myEnt_p->tsc         = 0;

	strncpy(msg_p,msg,traceControl_p->siz_msg);
	/* emulate stack push - right to left (so that arg1 end up at a lower
	   address, arg2 ends up at the next higher address, etc. */
	if (nargs)
	{
	    if (sizeof(long)==4) nargs*=2; /* kludge to support potential doubles on i686 */
	    if (nargs > traceControl_p->num_params) nargs=traceControl_p->num_params;
	    va_start( ap, msg );
	    for (argIdx=0; argIdx<nargs; ++argIdx)
		params_p[argIdx]=va_arg(ap,unsigned long);
	    va_end( ap );
	}
    }

    if (traceControl_p->mode.s.S && (traceNamLvls_p[traceTID].S & (1<<lvl)))
    {   printf("%10ld%06ld %2d %5d %d ",tv.tv_sec,tv.tv_usec,lvl,traceTid,nargs);
	va_start( ap, msg );
	vprintf( msg, ap );
	printf("\n");
	va_end( ap );
    }
}   /* trace */
#pragma GCC diagnostic pop



static void traceCntl( const char *cmd, ... )
{
    va_list ap;

    if (strncmp(cmd,"file",4) == 0)
    {   /* although it may be counter intuitive, this should override
	   env.var as it could be used to set a file-per-thread.
	   But... what if this negative impacts thread performace???
	*/
    }

    if (traceControl_p == NULL) traceInit();

    va_start( ap, cmd );

    if      (strncmp(cmd,"trig",4) == 0)    /* takes 3 args: modeMsks, lvlsMsk, postEntries */
    {
	uint32_t modeMsk=va_arg(ap,uint32_t);
	uint64_t lvlsMsk=va_arg(ap,uint64_t);
	unsigned post_entries=va_arg(ap,unsigned);
	if (   (  (traceControl_p->mode.s.M && (traceNamLvls_p[traceTID].M & lvlsMsk))
		||(traceControl_p->mode.s.S && (traceNamLvls_p[traceTID].S & lvlsMsk)) )
	    && !traceControl_p->trigActivePost)
	{   traceControl_p->trigIdxCount   = traceControl_p->wrIdxCnt;
	    traceControl_p->trigActivePost = post_entries?post_entries:1; /* must be at least 1 */
	    traceControl_p->trigOffMode.mode = modeMsk;
	}
    }
    else if (strcmp(cmd,"lvl") == 0)   /* CURRENTLY TAKE just 1 arg: lvl */
    {   
	uint64_t lvl=va_arg(ap,uint64_t);
	traceNamLvls_p[traceTID].M = lvl;
	traceNamLvls_p[traceTID].S = lvl;
	printf("set levels for TID=%d to 0x%llx\n", traceTID, (unsigned long long)lvl );
    }
    else if (strcmp(cmd,"mode") == 0)
    {   
	uint32_t mode=va_arg(ap,uint64_t);
	traceControl_p->mode.mode = mode;
    }
    else if (strncmp(cmd,"info",4) == 0) 
    {
	uint64_t wrSav=traceControl_p->wrIdxCnt;
	uint64_t used=((wrSav<=traceControl_p->num_entries)
		       ?wrSav
		       :traceControl_p->num_entries);
	printf("trace_initialized =%d\n"
	       "mode              =0x%x\n"
	       "writeIdxCount     =0x%16llx entries used: %llu\n"
               "largestMultiple   =0x%16llx\n"
	       "trigIdxCount      =0x%16llx\n"
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
	       , traceControl_p->trace_initialized
	       , traceControl_p->mode.mode
	       , (unsigned long long)wrSav, (unsigned long long)used
	       , (unsigned long long)traceControl_p->largest_multiple
	       , (unsigned long long)traceControl_p->trigIdxCount
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
	       );
    }
    else if (strcmp(cmd,"reset") == 0) 
    {
	if (traceControl_p == NULL) traceInit();
	traceControl_p->full
	    = traceControl_p->wrIdxCnt
	    = traceControl_p->trigIdxCount
	    = traceControl_p->trigActivePost
	    = 0;
    }
    else if (strcmp(cmd,"show") == 0) 
    {
	uint64_t rdIdx=traceControl_p->wrIdxCnt;
	unsigned printed=0;
	uint64_t max=(rdIdx<=traceControl_p->num_entries)
	    ?rdIdx:traceControl_p->num_entries;
	struct traceEntryHdr_s* myEnt_p;
	char                  * msg_p;
	unsigned long         * params_p;

	rdIdx = idxCnt_add( rdIdx, -1 );
	for (printed=0; printed<max; ++printed)
	{   myEnt_p = idxCnt2entPtr( rdIdx );
	    msg_p    = (char*)(myEnt_p+1);
	    params_p = (unsigned long*)(msg_p+traceControl_p->siz_msg);

	    printf("%6u %10ld%06ld %2d %5d "
		   , printed
		   , myEnt_p->time.tv_sec, myEnt_p->time.tv_usec
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

	    rdIdx = idxCnt_add( rdIdx, -1 );
	}
    }
    else
    {   fprintf( stderr, "TRACE: invalid control string %s\n", cmd );
    }

    va_end(ap);
}   /* traceCntl */

static int traceInit(void)
{   int   fd;
    const char *levl=getenv("TRACE_LVL");
    const char *mode=getenv("TRACE_MODE");
    const char *path=getenv("TRACE_FILE");
    /*const char *conf=getenv("TRACE_CONF"); need params,msg_sz,num_entries,num_namLvlTblEnts */
    int  num_namLvlTblEnts=200;
    int  num_params=10;
    int  siz_msg=128;
    int  num_entries=1000000;
    int  mmlen=sizeof(struct traceControl_s);
    int  siz_cntl_pages;
    char lvltmpbuf[80];

    mmlen += 4096;
    mmlen -= mmlen&4095;
    siz_cntl_pages = mmlen;
    mmlen += sizeof(struct traceNamLvls_s) * num_namLvlTblEnts
	+ entSiz(siz_msg,num_params) * num_entries
	+ 4096;/* extra space incase output printf somehow want more args from last entry */

    if (path == NULL) path="/tmp/trace_buffer";
    if (mode == NULL) mode="3";
    if (levl == NULL) {sprintf(lvltmpbuf,"0x%llx",(unsigned long long)-1);levl=lvltmpbuf;}

    /* need to special processing (lock file?) for create */
    if ((fd=open(path,O_RDWR|O_CREAT,0666)) == -1)
    {    fprintf( stderr,"open of %s returned %d\n", path, fd );
	 traceControl_p=&traceControl;
    }
    else
    {
	off_t off = lseek( fd, 0, SEEK_END );
	if (off == (off_t)-1) { perror("lseek"); exit(1); }
	if (off != mmlen)
	{   uint8_t one_byte='\0';
	    off = lseek( fd, mmlen-1, SEEK_SET );
	    if (off == (off_t)-1) { perror("lseek"); exit(1); }
	    write( fd, &one_byte, 1 );
	}
	traceControl_p = (struct traceControl_s *)mmap( NULL, mmlen
						       , PROT_READ|PROT_WRITE
						       , MAP_SHARED, fd, 0 );
	if (traceControl_p == (struct traceControl_s *)-1)
	{   perror( "mmap(NULL,mmlen,PROT_READ,MAP_PRIVATE,fd,0) error" );
	    printf( "mmlen=%d\n", mmlen );
	    traceControl_p=&traceControl;
	}

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
	    traceControl_p->largest_multiple = (uint64_t)-1 - ((uint64_t)-1 % num_entries);
	    traceControl_p->siz_entry        = entSiz( siz_msg, num_params );

	    /* MUST THINK ABOUT "lvlM" and/or "lvlP" ???
	       AND        "modeM" and/or "modeP" ???
	    */
	    traceCntl( "lvl",  strtoul(levl,NULL,0) );
	    traceCntl( "mode", strtoul(mode,NULL,0) );
	}
    }
    return (0);
}   /* traceInit */

/* The "largest_multiple" method (using (ulong)-1) allows "easy" "add 1"
// I must do the substract (ie. add negative) by hand.
// Ref. ShmRW class (~/src/ShmRW?)
*/
static uint64_t idxCnt_add( uint64_t idxCnt, int add )
{   uint64_t tt;
    if (add < 0)
    {   add = -add;
        if ((uint32_t)add > idxCnt)
        {   add -= idxCnt;
            tt=traceControl_p->largest_multiple - add;
        }
        else
            tt=idxCnt-add;
    }
    else
        tt=idxCnt+add;
    return (tt%traceControl_p->largest_multiple);
}

#if 0
static uint64_t idxCnt_delta( uint64_t wr, uint64_t rd )
{   return ((wr-rd)%traceControl_p->largest_multiple);
}
#endif

static struct traceEntryHdr_s* idxCnt2entPtr( uint64_t idxCnt )
{   uint64_t idx=idxCnt%traceControl_p->num_entries;
    off_t off=idx*traceControl_p->siz_entry;
    return (struct traceEntryHdr_s *)((unsigned long)traceEntries_p+off);
}

static uint32_t entSiz( uint32_t siz_msg, uint32_t num_params )
{
    return (  sizeof(struct traceEntryHdr_s)
	    + sizeof(uint64_t)*num_params /* NOTE: extra size for i686 (32bit processors) */
	    + siz_msg );
}

#endif /* TRACE_H_5216 */
