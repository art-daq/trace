#! /bin/sh
 # This file (big_ex.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Mar 10, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: big_ex.sh,v $
 # rev='$Revision: 1471 $$Date: 2021-01-19 23:48:57 -0600 (Tue, 19 Jan 2021) $'
set -u
opt_depth=30
opt_std=c++11
do_define=1
do_declare=1
do_mapcheck=2
do_shared=1
do_trace_active=1
#check_opts='-l5000 -t7 -x3'
opt_tlogs_per=150   # default for subs besides last
opt_name=jones
def_threads=`expr $(nproc) \* 91 / 100`   # 
def_loops=50
def_process_forks=1
def_stack=0x4000
def_min_delta=-100
USAGE="\
   usage: `basename $0` <dir>
examples: `basename $0` ./big_ex.d
          `basename $0` ./big_ex.d -O3
          `basename $0` ./big_ex.d --std=  # use compiler default c++ std
          `basename $0` ./big_ex.d -DTRACE_STATIC -d200 --mapcheck=2 --check-opts='-l1000 -t75 -x1' --check-numents=16000000
          `basename $0` ./big_ex.d -d200 --mapcheck=2 --check-opts='-l1000 -t75 -x3' --check-numents=16000000
          `basename $0` ./big_ex.d -DNO_TRACE -d200 --check-opts='-l1000 -t75 -x3' --check-numents=16000000
          `basename $0` ./big_ex.d --no-define --no-declare -d25 -std=
          `basename $0` ./big_ex.d -DTRACE_STATIC --check-opts='-l50 -t2048 -x5' # os tid can recycle with - 
                                                    #- higher -t val, approx 2600 uniq tids have been seen.
          `basename $0` ./big_ex.d -DTRACE_STATIC --check-opts='-l50 -t512 -x1' --depth=200 --check-numents=6000000 # -
                                                    #- with --depth=200 a -t much above 512 can lead to -
                                                    #- \"Resource temporarily unavailable\" from pthread_create
          TRACE_LIMIT_MS=4,1,4000 `basename $0` ./big_ex.d
If directory does not exist, it will be created.
Files in the dir will be overwritten (unless... see --rerun below).
-v               more verbose
-d, --depth=     defalut is $opt_depth, but a better test might be 500. MIN 10
-DTRACE_DECLARE  add -DTRACE_DECLARE to compile line
-DTRACE_STATIC   add -DTRACE_STATIC to compile line
--std=<c++std>,-std=<c++std)   both single and double - work default=$opt_std
--asan           Use AddressSanitizer (mutually exclusive with tsan)
--tsan           Use ThreadSanitizer (mutually exclusive with asan)
--ubsan          Use UndefinedBehaviorSanitizer
--no-define      remvoe #define TRACE_DEFINE from main
--no-declare     remove #define TRACE_DECLARE from subs (could also do -DTRACE_STATIC)
--no-shared      do not make subs into .so files
--mapcheck[=num] default=$do_mapcheck, the number of check LOOPS
-t<threads>      default $def_threads
-l<loops>        default $def_loops
-p<forks>        default $def_process_forks
--tlogs-per=[num] default $opt_tlogs_per
--check-opts=<opts> default=\"-t$def_threads -l$def_loops\"  See below. Note: thread and loops options added
--check-numents=<ents>  default=calculated: ( depth * tlogs_per + 2 ) * threads * loops
--stack=[num]    stack used in all but last subroutine - default $def_stack
-O<x>            compile optimization level. default: no -O
--rerun          Only recompile main program (don't remake and compile all other files)
--inactive       no memory/fast path, just stdout/slow

Options passed to the program to be checked:
-n<TRACE_NAME>
-f<TRACE_FILE>
-l<loops>
-t<num_threads>   # in addition to main program
-x<mask>  extra options - b0=count maps; b1=use sub-threads; b2=random delay in sub1 after 1st TRACE

NOTE: the program can display the number of trace_buffer mappings which
can get very large when a large combination of TRACE_STATIC modules and
threads are used.
"
VUSAGE=''
set +u # avoid issues with $@ and bash 4.3.27 on SunOS
op1chr='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set -- "-$rest" "$@"'
op1arg='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set --  "$rest" "$@"'
reqarg="$op1arg;"'test -z "${1+1}" &&echo opt -$op requires arg. &&echo "$USAGE" &&exit'
args= do_help= opt_v=0
compile_opts=
while [ -n "${1-}" ];do
    if expr "x${1-}" : 'x-' >/dev/null;then
        op=`expr "x$1" : 'x-\(.*\)'`; shift   # done with $1
        leq=`expr "x$op" : 'x[^=]*\(=\)'` lev=`expr "x$op" : 'x[^=]*=\(.*\)'`
        test "x$leq" != x &&eval "set -- \"\$lev\" \"\$@\""&&op=`expr "x$op" : 'x\([^=]*\)'`
        case "$op" in
        \?*|h*)     eval $op1chr; do_help=1;;
        v*)         eval $op1chr; opt_v=`expr $opt_v + 1`;;
        x*)         eval $op1chr; test $opt_v -ge 1 && set -xv || set -x;;
        d*|-depth)  eval $reqarg; opt_depth=$1;                        shift;;
        D*)         eval $reqarg; compile_opts="$compile_opts -D$1";   shift;;
        O*)         eval $reqarg; compile_opts="$compile_opts -O$1";   shift;;
        -asan)      compile_opts="$compile_opts -fsanitize=address";   shift;;
        -tsan)      compile_opts="$compile_opts -fsanitize=thread";    shift;;
        -ubsan)      compile_opts="$compile_opts -fsanitize=undefined";shift;;
        -std)       eval $reqarg; opt_std=$1;                          shift;;
        std)                      opt_std=$1;                          shift;;
        t*)         eval $reqarg; opt_threads="$1";                    shift;;
        l*)         eval $reqarg; opt_loops="$1";                      shift;;
        p*)         eval $reqarg; opt_forks="$1";                      shift;;
        -stack)     eval $reqarg; opt_stack="$1";                      shift;;
        -tlogs-per) eval $reqarg; opt_tlogs_per="$1";                  shift;;
        -check-opts)eval $reqarg; check_opts=$1;                       shift;;
        -no-define) do_define= ;;
        -no-declare)do_declare= ;;
        -no-shared) do_shared= ;;
        -rerun)     opt_rerun=1;;
        -mapcheck)  test -n "$leq"&&do_mapcheck=$1&&shift||do_mapcheck=1;;
        -check-numents)eval $reqarg; check_numents=$1;                 shift;;
        -extra-ents)eval $reqarg; opt_extra_ents=$1;                   shift;;
        -inactive)  do_trace_active=;;
        *)          echo "Unknown option -$op"; do_help=1;;
        esac
    else
        aa=`echo "$1" | sed -e "s/'/'\"'\"'/g"`; args="$args '$aa'"; shift
    fi
done
eval "set -- $args \"\$@\""; unset args aa
set -u
test $# -lt 1 && echo 'Missing required "directory" argument.' && do_help=1
test $# -gt 1 && echo 'Extra arguments.' && do_help=1
help() { echo "$USAGE";test $opt_v -ge 1 && echo "$VUSAGE" || true for pipeline; }
test $opt_depth -lt 10 && echo WARNING: depth option should be min 10
test -n "${do_help-}" && help && exit


test $opt_depth -ge 10 || { echo "depth option cannot less than 10"; exit 1; }
hash trace_cntl || { echo "trace_cntl program must be in PATH"; exit 1; }

test -z "${opt_stack-}" && opt_stack=$def_stack
test -z "${opt_threads-}" && opt_threads=$def_threads
test -z "${opt_loops-}"   && opt_loops=$def_loops
test -z "${opt_forks-}"   && opt_forks=$def_process_forks
test -z "${check_opts-}" \
 && check_opts="-l$opt_loops -t$opt_threads -p$opt_forks" \
 || check_opts="-l$opt_loops -t$opt_threads -p$opt_forks $check_opts"

vprintf() { num=$1; shift; test $opt_v -ge $num && printf "`date`: ""$@"; }

odir=$1
test -d $odir || mkdir -p $odir

cd $odir
# Clean up
if [ -z "${opt_rerun-}" ];then
  rm -f sub*.cc sub*.o sub*.so
  rm -f big_ex_main.cc big_ex_main
fi

# set TRACE_FILE now as it is used in constructing source files
export TRACE_FILE
test -z "${TRACE_FILE-}" && TRACE_FILE=/tmp/trace_buffer_`whoami`  # make sure

# - - - - - - - - - - - Make all the sub modules (except for the last 1) - - -

# trace versions less than v3_13_12 need to have
#      ln -s . $TRACE_INC/TRACE
#      ln -s trace_delta.pl $TRACE_BIN/trace_delta
# manually added.
trace_revnum=`awk '/Revision:/{print$4;exit}' $TRACE_INC/TRACE/trace.h`
if   [ $trace_revnum -le  719 ];then
    opt_def_trace_revnum="-DTRACE_REVNUM=$trace_revnum -DTLOG(lvl)=TLOG_ARB(lvl,\"somename\")"
elif [ $trace_revnum -le 1429 ];then
    opt_def_trace_revnum="-DTRACE_REVNUM=$trace_revnum"
else
    opt_def_trace_revnum=
fi
struct_args='struct args { pid_t tid; unsigned loop; int xtra_options; useconds_t dly_us; int thread_idx; unsigned char *tosp; }'

echo opt_depth=$opt_depth opt_tlogs_per=$opt_tlogs_per check_opts=$check_opts
flags=$-
nn=1
while [ -z "${opt_rerun-}" -a $nn -lt $opt_depth ];do
    next=`expr $nn + 1`
    test $nn -eq 1 && POTENTIAL_DELAY='if(aa->xtra_options&4){aa->dly_us=random()%5000;usleep(aa->dly_us);}'\
                   || POTENTIAL_DELAY=
    expr $nn % 10 >/dev/null && do_TRACE_NAME= || do_TRACE_NAME="\
#define TRACE_NAME \"sub$nn\""
    cat >sub$nn.cc <<EOF
#include <sys/types.h>          // pid_t
${do_declare:+#ifndef TRACE_DECLARE}
${do_declare:+# define TRACE_DECLARE // trace variables are defined somewhere else}
${do_declare:+#endif}
$do_TRACE_NAME
#ifndef NO_TRACE
# include "TRACE/trace.h"
#else
# define TRACE(...)
# define TLOG(...) if(0)std::cout /* allow compiler to compile-n-optimize-out */
#endif
#include <stdlib.h> /* random (optionally) */
#include <unistd.h> /* usleep (optionally) */

$struct_args;

void sub$next( struct args *aa );
void sub$nn( struct args *aa )
{   unsigned buffer[$opt_stack], total=0;
    unsigned char tos;
    TLOG(2) << "sub$nn tid="<<aa->tid<<" loop="<<aa->loop
            << " calling sub$next tC_p="<<(void*)traceControl_p<<" "
            <<traceInitLck_hung_max<<"=tIL_hung_max"
#   if TRACE_REVNUM <= 762
    << TLOG_ENDL
#   endif
    ;
    $POTENTIAL_DELAY
EOF
    set +x
    xx=$opt_tlogs_per
    while xx=`expr $xx - 1`;do
        cat >>sub$nn.cc <<EOF
    TLOG(2) << "sub$nn additional $xx"
#   if TRACE_REVNUM <= 762
    << TLOG_ENDL
#   endif
    ;
EOF
    done
    cat >>sub$nn.cc <<EOF
    for(unsigned ii=0; ii<(sizeof(buffer)/sizeof(buffer[0])); ++ii) buffer[ii]=1;
    sub$next(aa);
    for(unsigned ii=0; ii<(sizeof(buffer)/sizeof(buffer[0])); ++ii) total+=buffer[ii];
    TRACE( 3, "sub$nn tid=%d after (return from) call to sub$next tot=%u %ld=stack",aa->tid, total, (long)(aa->tosp-&tos) );
}
EOF
    nn=`expr $nn + 1`
done
set -$flags

# - - - - - - - - - - - Make the last sub module - - - - - - - -

if [ -z "${opt_rerun-}" ];then

last=$nn
expr $nn % 10 >/dev/null && do_TRACE_NAME= || do_TRACE_NAME="\
#define TRACE_NAME \"sub$nn\""
cat >sub$last.cc <<EOF
#include <sys/types.h>          // pid_t
${do_declare:+#ifndef TRACE_DECLARE}
${do_declare:+# define TRACE_DECLARE // trace variables are defined somewhere else}
${do_declare:+#endif}
$do_TRACE_NAME
#ifndef NO_TRACE
# include "TRACE/trace.h"
#else
# define TRACE(...)
# define TLOG(...) if(0)std::cout /* allow compiler to compile-n-optimize-out */
#endif

$struct_args;

void* simple_thread_func(void *arg)
{
    struct args aa=*(struct args *)arg;
    TLOG(2) << "hello from simple_thread idx " << aa.thread_idx << " tC_p="<<(void*)traceControl_p
#   if TRACE_REVNUM <= 762
    << TLOG_ENDL
#   endif
    ;
    pthread_exit(NULL);
}

void sub$last( struct args *aa )
{   unsigned char tos;
    pthread_t     thread;
    TRACE( 2, "sub$last tid=%d loop=%.4f=dly ret %u=tIL_hung_max %ld=stack"
          , aa->tid,aa->loop+aa->dly_us/10000.0,traceInitLck_hung_max, (long)(aa->tosp-&tos));
    int sts=pthread_create(&thread,NULL,simple_thread_func,(void*)aa);
    if(sts!=0){perror("pthread_create-simple");exit(1);}
    pthread_join(thread, NULL);
}
EOF


# - - - - - - - - - - - Make the main module - - - - - -

cat >big_ex_main.cc <<EOF
#include <stdio.h>              // printf
#include <stdlib.h>             // exit
#include <pthread.h>		// pthread_self
#include <unistd.h>		// getopt
#include <sys/types.h>          // pid_t
#include <sys/wait.h>           // waitpid
#ifndef __CYGWIN__
#include <sys/syscall.h>	// syscall
# ifdef __sun__
#  define TRACE_GETTID SYS_lwp_self
# elif defined(__APPLE__)
#  define TRACE_GETTID SYS_thread_selfid
# else
#  define TRACE_GETTID __NR_gettid
# endif
static inline pid_t ex_gettid(void) { return syscall(TRACE_GETTID); }
#else
# include <windows.h>
static inline pid_t ex_gettid(void) { return GetCurrentThreadId(); }
#endif
#include <libgen.h>             // basename
${do_define:+#define TRACE_DEFINE // trace variables are defined in this module}
#ifndef NO_TRACE
# include "TRACE/trace.h"
#else
# define TRACE(...)
# define TLOG(...) if(0)std::cout /* allow compiler to compile-n-optimize-out */
#endif

#define USAGE "\
  usage: %s [options]\n\
example: %s -x1\n\
options:\n\
-n<TRACE_NAME>\n\
-f<TRACE_FILE>\n\
-l<loops>\n\
-t<num_threads>   # in addition to main program\n\
-p<num_processes> # in addition to main program\n\
-x<mask>  extra options - b0=count maps; b1=use sub-threads; b2=random delay in sub1 after 1st TRACE\n\
-s<seconds>       # sleep at end to allow /proc/<pid>/ examination from another terminal\n\
", basename(argv[0]), basename(argv[0])

$struct_args;

void sub1( struct args *aa );

void* thread_func(void *arg)
{
    unsigned char tos; /* top of stack */
    struct args *args_p=(struct args *)arg;
    long loops=args_p->loop; // initial "loops" from main
    struct args aa=*args_p;  // per thread copy -  initialize from main
    if (aa.tid != 1) aa.tid=ex_gettid();
    aa.tosp=&tos;
#   if TRACE_REVNUM <= 762
    TLOG(2)                                        << "hello from thread idx " << aa.thread_idx <<" "<<3.14 << TLOG_ENDL;
#   else
    TLOG(2,"thread"+std::to_string((long long)(aa.thread_idx))) << "hello from thread idx " << aa.thread_idx <<" "<<3.14;
#   endif
    for (unsigned ii=0; ii<loops; ++ii) {
        TRACE( 2, "tf tid=%d loop=%u calling sub1 tC_p=%p %u=tIL_hung_max",aa.tid,ii,traceControl_p,traceInitLck_hung_max);
        aa.loop=ii;
        if (aa.xtra_options & 2) {
            pthread_t thread;
            TRACE( 3, "tf tid=%d loop xtra_option b1 - before pthread_create of sub-thread",aa.tid );
            pthread_create(&thread,NULL,(void*(*)(void*))sub1,(void*)&aa);
            pthread_join(thread, NULL);
        }
        else {
            TRACE( 3, "tf tid=%d loop before call to sub1",aa.tid );
            sub1(&aa);
            TRACE( 3, "tf tid=%d loop after (returned from) call to sub1",aa.tid );
        }
    }
    if (aa.tid != 1) pthread_exit(NULL);
    else             return (NULL);                  // not a thread
}

int main( int argc, char *argv[] )
{
        pthread_t   * threads;
	unsigned      num_threads=4;
	unsigned      num_processes=0;
        pid_t       * pids;
extern  char        * optarg;        // for getopt
        int           opt;           // for how I use getopt
	unsigned long loops=500;
        unsigned      ii;
        struct args * args_p;
        int           xtra_options=1;
        int           opt_sleep=0;

    while ((opt=getopt(argc,argv,"?hn:f:l:p:t:x:s:")) != -1)
    {   switch (opt)
        { // '?' is also what you get w/ "invalid option -- -"
        case '?': case 'h': printf(USAGE);exit(0);    break;
	case 'n': setenv("TRACE_NAME",optarg,1);      break;
	case 'f': setenv("TRACE_FILE",optarg,1);      break;
	case 'l': loops=strtoul(optarg,NULL,0);       break;
	case 'p': num_processes=strtoul(optarg,NULL,0);break;
	case 't': num_threads=strtoul(optarg,NULL,0); break;
        case 'x': xtra_options=strtoul(optarg,NULL,0);break;
        case 's': opt_sleep=strtoul(optarg,NULL,0);   break;
        }
    }
    threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
    pids    =     (pid_t*)malloc(num_processes*sizeof(pid_t));
    args_p  = (struct args *)malloc(num_threads*sizeof(struct args));
    TRACE( 1, "b4 pthread_create - loops=%lu tC_p=%p %u=tIL_hung_max", loops, traceControl_p, traceInitLck_hung_max );
    printf("test-threads - before create loop - loops=%lu num_threads=%u xtra_threads=%d\n",
           loops,num_threads,!!(xtra_options&2));fflush(stdout); /* flush before possible fork */
    if (num_threads>1) for (ii=0; ii<num_threads; ii++) {
        args_p[ii].loop = loops; // the initial "loops"
        args_p[ii].xtra_options = xtra_options;
        args_p[ii].dly_us = 0;
        args_p[ii].thread_idx = ii;
        args_p[ii].tid = 0;
        int sts=pthread_create(&threads[ii],NULL,thread_func,(void*)&args_p[ii]);
        if(sts!=0){perror("pthread_create");exit(1);}
    }
    else {
        ii=0;
        args_p[ii].loop = loops; // the initial "loops"
        args_p[ii].xtra_options = xtra_options;
        args_p[ii].dly_us = 0;
        args_p[ii].thread_idx = ii;
        args_p[ii].tid = 1;
        thread_func( (void*)&args_p[ii] );
    }
    TLOG(1) << "Main - all 0x" << std::hex << num_threads << " threads created"
#   if TRACE_REVNUM <= 762
    << TLOG_ENDL
#   endif
    ;

    for (ii=0; ii<num_processes; ii++) {
        if((pids[ii]=fork()) == 0) {
            args_p[ii].loop = loops; // the initial "loops"
            args_p[ii].xtra_options = xtra_options;
            args_p[ii].dly_us = 0;
            args_p[ii].thread_idx = ii;
            args_p[ii].tid = 1;
            thread_func( (void*)&args_p[ii] );
            exit(0);
        }else if (pids[ii]==-1){perror("fork");exit(EXIT_FAILURE);}
    }
    for (ii=0; ii<num_processes; ii++) {
        int wstatus;
        printf("waiting for pid=%d\n",pids[ii]);
        if(waitpid(pids[ii],&wstatus,0)==-1){perror("waitpid");exit(EXIT_FAILURE);}
    }

    if (xtra_options & 1)
    {   char          cmd[200];
	sprintf( cmd, "echo trace_buffer mappings before join '(#1)' = \`cat /proc/%d/maps | grep $TRACE_FILE | wc -l\`", getpid() );
	system( cmd );
	sprintf( cmd, "echo trace_buffer mappings before join '(#2)' = \`cat /proc/%d/maps | grep $TRACE_FILE | wc -l\`", getpid() );
	system( cmd );
    }
    if (num_threads>1) for (ii=0; ii<num_threads; ii++) {
        pthread_join(threads[ii], NULL);
    }
    if (xtra_options & 1)
    {   char          cmd[200];
	sprintf( cmd, "echo trace_buffer mappings after join '(#1)' = \`cat /proc/%d/maps | grep $TRACE_FILE >big_ex_maps;wc -l big_ex_maps\`", getpid() );
	system( cmd );
	sprintf( cmd, "echo trace_buffer mappings after join '(#2)' = \`cat /proc/%d/maps | grep $TRACE_FILE | wc -l\`", getpid() );
	system( cmd );
    }
    printf("test-threads - after join loop\n");
    TRACE( 1, "after thread(s) (pthread_join) traceControl_p=%p", traceControl_p );
    free( args_p );
    free( threads );
    if (opt_sleep)
        sleep(opt_sleep); /* sleep at end to allow another terminal to cat /proc/<pid>/maps */
    return (0);
}   /* main */
EOF

# ^ ^ ^ ^ ^ ^ ^ ^ ^ Done making module source files ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^

opt_j=25
do_once=1
vprintf 0 'Compile subs\n'
nn=1
flags=$-
for ss in sub*.cc; do
   ofile=`basename $ss .cc`
   test -n "${do_shared-}" && out_opts="-fPIC -shared -o $ofile.so" || out_opts="-c -o $ofile.o"
   test -n "$do_once" && set -x
   g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_INC $opt_def_trace_revnum $out_opts $ss &
   test -n "$do_once" && { set +x; do_once=; }
   expr $nn % $opt_j >/dev/null || wait
   nn=`expr $nn + 1`
done
set -$flags
wait

fi # -z "${opt_rerun-}"

vprintf 0 'Compile main\n'
test $opt_v -gt 0 && set -x
test -n "${do_shared-}" \
 && { g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_INC $opt_def_trace_revnum -o big_ex_main big_ex_main.cc *.so -lpthread; sts=$?; export LD_LIBRARY_PATH=.${LD_LIBRARY_PATH+:$LD_LIBRARY_PATH}; } \
 || { g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_INC $opt_def_trace_revnum -o big_ex_main big_ex_main.cc *.o  -lpthread; sts=$?; }
test $opt_v -gt 0 && set +x
test $sts -eq 0 && echo big_ex_main built OK || { echo big_ex_main build FAILED; exit 1; }

test -n "${check_numents-}" \
 || check_numents=`expr \( \( $opt_depth - 1 \) \* $opt_tlogs_per + 35 \) \* \( $opt_threads + $opt_forks \) \* $opt_loops`

if [ "${do_mapcheck-0}" -gt 0 ];then
    export TRACE_PRINT
    TRACE_PRINT='%T %*n %*L %M'  # the old default (w/o __func__)
    if [ -z "$do_trace_active" ];then
        unset TRACE_NUMENTS TRACE_ARGSMAX TRACE_MSGMAX TRACE_NAMTBLENTS TRACE_NAMEMAX TRACE_NAME TRACE_FILE TRACE_LVLM
        opt_name=
    else
        export TRACE_NUMENTS TRACE_ARGSMAX TRACE_MSGMAX TRACE_NAMTBLENTS
        TRACE_ARGSMAX=4
        TRACE_MSGMAX=64
        TRACE_NUMENTS=`expr $check_numents + ${opt_extra_ents-0}`
        TRACE_NAMTBLENTS=`expr $opt_threads + 4 + $opt_depth / 10`   # extras: trace_cntl, jones, TRACE, _TRACE_ "sub10s"
        vprintf 1 'recreating trace buffer file with TRACE_ARGSMAX=4 TRACE_MSGMAX=64 TRACE_NUMENTS=%s TRACE_NAMTBLENTS=%s\n' "$TRACE_NUMENTS" "$TRACE_NAMTBLENTS"
        test "$TRACE_FILE" = /proc/trace/buffer && trace_cntl reset || { rm -f $TRACE_FILE; trace_cntl lvlset 0x2000000000000000 0 0; }    # master reset :) turn on atfork trace
        file_entries=`trace_cntl info | awk '/num_entries/{print$3;}'`
        test $file_entries -lt $TRACE_NUMENTS && { echo "file_entries=$file_entries -lt calculated_entries=$TRACE_NUMENTS"; exit; }
    fi
    echo check_opts=$check_opts
    uname=`uname`
    expect_static=`expr \( $opt_depth + 1 \) \* 2`
    #expect_declare=`expr 2 + $opt_depth / 10 \* 2` # take into accout the "do_TRACE_NAME" above
    expect_declare=2
    while true; do
        vprintf 0 "Testing... $do_mapcheck\n"

        test -f big_ex_main.out && mv -f big_ex_main.out big_ex_main.out~
        vprintf 1 "executing: ./big_ex_main ${opt_name:+-n$opt_name} -x1 $check_opts >big_ex_main.out 2>&1\n"
        trace_cntl reset; trace_cntl mode 3
        time ./big_ex_main ${opt_name:+-n$opt_name} -x1 $check_opts >big_ex_main.out 2>&1
        sts=$?; trace_cntl mode 0
        test $sts -ne 0 && { echo ./big_ex_main FAILED - exit status: $sts; exit 1; }

        parallel_threads=`cat big_ex_main.out | sed -n -e '/num_threads/{s/.*num_threads= *//;s/ .*//;p;}'`
        loops=`cat big_ex_main.out | sed -n -e '/before create/{s/.*loops= *//;s/ .*//;p;}'`
        num_maps=`cat big_ex_main.out | sed -n -e '/after join (#2) = /{s/.*= *//;p;}'`
        xtra_threads=`cat big_ex_main.out | sed -n -e '/xtra_threads/{s/.*xtra_threads= *//;s/ .*//;p;}'`
        test $xtra_threads -eq 0 && check_tids=`expr 1 + $parallel_threads + \( $opt_threads + $opt_forks \) \* $opt_loops` || check_tids=
        # Note; the OS can randomly recycle tids, so if the program is creating/joining threads,
        # the number of uniq tids that the program will experience is unknown.

        vprintf 0 "\
Analyzing trace_buffer... (n_maps=%d loops=%d pthreads=%d expect:STATIC=%d DECLARE=%d ?tids=%d)\n\
" $num_maps $loops $parallel_threads $expect_static $expect_declare $check_tids

        if [ -n "$do_trace_active" -a -f "${TRACE_FILE-}" ];then
            trace_cntl info | egrep 'used|full|num_entries' | sed 's/^/  /'
            uniq_addrs=`TRACE_SHOW='%H%x%i %I %C %L %R' trace_cntl show | sed -n -e '/_p=/{s/.*_p=//;s/ .*//;p;}' | sort -u | wc -l`
            vprintf 1 'Calculating sub10_trc_ids...\n'
            sub10_trc_id=`TRACE_SHOW='%H%x%i %I %C %L %R' trace_cntl show | awk '/sub10 tid=/{print$2;}' | sort -u`
            sub10_trc_ids=`echo "$sub10_trc_id" | wc -l`
            test $sub10_trc_ids -eq 1 || sub10_trc_id=-1
            vprintf 1 'Calculating uniq_thr_ids...\n'
            uniq_thr_ids=`TRACE_SHOW='%x%i %n' trace_cntl show | grep -v KERNEL | awk '{print$1;}' | sort -nu | wc -l`
            vprintf 1 'Calculating uniq_pids...\n'
            uniq_pids=`   TRACE_SHOW='%x%P %n' trace_cntl show | grep -v KERNEL | awk '{print$1;}' | sort -nu | wc -l`
            sub10_trc_id_=`trace_cntl tids | awk '/ sub10 /{print$1;}'`
            last_thr_idx=`expr $opt_threads - 1`
            _TRACE_=`TRACE_SHOW=%n trace_cntl show | grep "_TRACE_"` # should be none (i.e. namtbl should not be full)
            vprintf 1 'Calculating delta_min...\n'
            show_count=`trace_cntl info | awk '/entries used:/{print$6;}'`
            test $show_count -gt 500000 && show_count=500000
            start_idx=`expr $show_count - 1` # smaller buffers will wrap -- smallish number of unused entries have 0 timestamp which tdelta ignores
            if [ $trace_revnum -ge 1146 ];then # v3_14_02 (1129) or below does not have -c<cnt> and -s<start>; v3_15_00 (1146) does.
                delta_min=`TRACE_SHOW=%H%T trace_cntl show -c$show_count -s$start_idx|trace_delta -stats|awk '/^  *min /{print$2;}'`
            else
                #vprintf 1 "start_idx=$start_idx show_count=$show_count\n"
                delta_min=`TRACE_SHOW=%T trace_cntl show | head -$show_count | trace_delta -d 0 -stats|awk '/^  *min /{print$2;}'`
            fi
            if [ $delta_min -lt $def_min_delta -a $opt_v -ge 1 ];then
                TRACE_SHOW=%H%T trace_cntl show -c$show_count -s$start_idx | trace_delta | grep -C5 " $delta_min "
            fi
            vprintf 1 'done calculating\n'
        elif [ -z "$do_trace_active" ];then
            echo "stdout (big_ex_main.out) analysis only"
            expect_declare=0
            uniq_addrs=`expr $expect_declare / 2`   # the right answer
            sub10_trc_id_=0                         # the right answer
            sub10_trc_id=$sub10_trc_id_             # the right answer
            sub10_trc_ids=1                         # the right answer
            uniq_thr_ids=1                          # the right answer
            uniq_pids=0                             # the right answer
            vprintf 1 'Calculating delta_min...\n'
            d_min=`grep '[0-9][0-9]-[0-9]' big_ex_main.out | trace_delta -d 1 -i -stats | awk '/^  *min /{print$2;}'`
            delta_min=`perl -e "print $d_min * 1000000"`
            _TRACE_=`grep _TRACE_ big_ex_main.out`
        else
            echo "TRACE_FILE not found - FAIL will result"
            uniq_addrs=0 sub10_trc_ids=0 sub10_trc_id=-1 sub10_trc_id_=-2 uniq_thr_ids=0 uniq_pids=0
            _TRACE_=
            delta_min=-1000
        fi
        uniq2=`expr $uniq_addrs \* 2`
        echo "  uniq_addrs=$uniq_addrs uniq2=$uniq2 num_maps=$num_maps sub10_trc_ids="$sub10_trc_ids\
             "sub10_trc_id=$sub10_trc_id uniq_thr_ids=$uniq_thr_ids uniq_pids=$uniq_pids delta_min=$delta_min"
        fail=
        test \( $uniq2 -eq $expect_declare -o $uniq2 -eq $expect_static \) || fail="$fail uniq_addrs"
        test "$sub10_trc_id" -eq $sub10_trc_id_                            || fail="$fail sub10_trc_id"
        test -z "$_TRACE_"                                                 || fail="$fail _TRACE_tid"
        test $delta_min -ge $def_min_delta                                 || fail="$fail delta_min=$delta_min"
        if [ "$uname" = Linux ];then
            # additional checks
            test $num_maps -eq $uniq2                                      || fail="$fail num_maps($num_maps!=$uniq2)"
            test \( $uniq_thr_ids -eq 1 -o -z "$check_tids" -o $uniq_thr_ids -eq "${check_tids:-0}" \) \
                || fail="$fail uniq_thr_ids($uniq_thr_ids!=${check_tids:-0})"
        fi
        test -n "$fail" && { echo FAIL - $fail ; exit 1; } || echo "  SUCCESS"
        #trace_cntl tids
        test "$do_mapcheck" -le 1 && break
        do_mapcheck=`expr $do_mapcheck - 1`
    done
    ls -l ${TRACE_FILE-} big_ex_main.out*
fi
