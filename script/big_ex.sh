#! /bin/sh
 # This file (big_ex.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Mar 10, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: big_ex.sh,v $
 # rev='$Revision: 1031 $$Date: 2019-01-30 17:09:16 -0600 (Wed, 30 Jan 2019) $'
set -u
opt_depth=30
opt_std=c++11
do_define=1
do_declare=1
do_mapcheck=2
do_shared=1
#check_opts='-l5000 -t7 -x3'
opt_tlogs_per=150   # default for subs besides last
def_threads=50
def_loops=50
def_stack=0x4000
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
Files in the dir will be overwritten.
-d, --depth=   defalut is $opt_depth, but a better test might be 500. MIN 10
-DTRACE_DECLARE  add -DTRACE_DECLARE to compile line
-DTRACE_STATIC   add -DTRACE_STATIC to compile line
--std=<c++std>,-std=<c++std)   both single and double - work default=$opt_std
--asan           Use AddressSanitizer (mutually exclusive with tsan)
--tsan           Use ThreadSanitizer (mutually exclusive with asan)
--ubsan          Use UndefinedBehaviorSanitizer
--no-define      remvoe #define TRACE_DEFINE from subs (could also do -DTRACE_STATIC)
--no-declare     remove #define TRACE_DECLARE from main
--no-shared      do not make subs into .so files
--mapcheck[=num] default=$do_mapcheck, the number of check LOOPS
-t<threads>      default $def_threads
-l<loops>        default $def_loops
--tlogs-per=[num] 
--check-opts=<opts> default=\"-t$def_threads -l$def_loops\"  See below. Note: thread and loops options added
--check-numents=<ents>  default=calculated: ( depth * tlogs_per + 2 ) * threads * loops
--stack=[num]    stack used in all but last subroutine - default $def_stack
-O<x>            compile optimization level. default: no -O

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
        d*|-depth)  eval $reqarg; opt_depth=$1; shift;;
        D*)         eval $reqarg; compile_opts="$compile_opts -D$1"; shift;;
        O*)         eval $reqarg; compile_opts="$compile_opts -O$1"; shift;;
        -asan)      compile_opts="$compile_opts -fsanitize=address"; shift;;
        -tsan)      compile_opts="$compile_opts -fsanitize=thread"; shift;;
        -ubsan)      compile_opts="$compile_opts -fsanitize=undefined"; shift;;
        -std)       eval $reqarg; opt_std=$1;                 shift;;
        std)                      opt_std=$1;                 shift;;
        t*)         eval $reqarg; opt_threads="$1";                  shift;;
        -stack)     eval $reqarg; opt_stack="$1";                  shift;;
        -tlogs-per) eval $reqarg; opt_tlogs_per="$1";                  shift;;
        -check-opts)eval $reqarg; check_opts=$1; shift;;
        -no-define) do_define= ;;
        -no-declare)do_declare= ;;
        -no-shared) do_shared= ;;
        -mapcheck)  test -n "$leq"&&do_mapcheck=$1&&shift||do_mapcheck=1;;
        -check-numents)eval $reqarg; check_numents=$1; shift;;
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
test -z "${check_opts-}" \
 && check_opts="-t$opt_threads -l$opt_loops" \
 || check_opts="-t$opt_threads -l$opt_loops $check_opts"

vprintf() { num=$1; shift; test $opt_v -ge $num && printf "`date`: ""$@"; }

odir=$1
test -d $odir || mkdir -p $odir

cd $odir
# Clean up
rm -f sub*.cc sub*.o sub*.so
rm -f big_ex_main.cc big_ex_main

# - - - - - - - - - - - Make all the sub modules (except for the last 1) - - -

struct_args='struct args { pid_t tid; unsigned loop; int xtra_options; useconds_t dly_us; int thread_idx; unsigned char *tosp; }'

echo opt_depth=$opt_depth opt_tlogs_per=$opt_tlogs_per check_opts=$check_opts
nn=1
while [ $nn -lt $opt_depth ];do
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
# include "trace.h"
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
            <<traceInitLck_hung_max<<"=tIL_hung_max";
    $POTENTIAL_DELAY
EOF
    xx=$opt_tlogs_per
    while xx=`expr $xx - 1`;do
        cat >>sub$nn.cc <<EOF
    TLOG(2) << "sub$nn additional $xx";
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

# - - - - - - - - - - - Make the last sub module - - - - - - - -

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
# include "trace.h"
#else
# define TRACE(...)
# define TLOG(...) if(0)std::cout /* allow compiler to compile-n-optimize-out */
#endif

$struct_args;

void* simple_thread_func(void *arg)
{
    struct args aa=*(struct args *)arg;
    TLOG(2) << "hello from simple_thread idx " << aa.thread_idx;
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
# include "trace.h"
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
-x<mask>  extra options - b0=count maps; b1=use sub-threads; b2=random delay in sub1 after 1st TRACE\n\
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
    TLOG(2,"thread"+std::to_string(aa.thread_idx)) << "hello from thread idx " << aa.thread_idx <<" "<<3.14;
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
extern  char        * optarg;        // for getopt
        int           opt;           // for how I use getopt
	unsigned long loops=500;
        unsigned      ii;
        struct args * args_p;
        int           xtra_options=1;

    while ((opt=getopt(argc,argv,"?hn:f:l:t:x:")) != -1)
    {   switch (opt)
        { // '?' is also what you get w/ "invalid option -- -"
        case '?': case 'h': printf(USAGE);exit(0);    break;
	case 'n': setenv("TRACE_NAME",optarg,1);      break;
	case 'f': setenv("TRACE_FILE",optarg,1);      break;
	case 'l': loops=strtoul(optarg,NULL,0);       break;
	case 't': num_threads=strtoul(optarg,NULL,0); break;
        case 'x': xtra_options=strtoul(optarg,NULL,0);break;
        }
    }
    threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
    args_p  = (struct args *)malloc(num_threads*sizeof(struct args));
    TRACE( 1, "b4 pthread_create - loops=%lu tC_p=%p %u=tIL_hung_max", loops, traceControl_p, traceInitLck_hung_max );
    printf("test-threads - before create loop - loops=%lu num_threads=%u xtra_threads=%d\n",
           loops,num_threads,!!(xtra_options&2));
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
    TLOG(1) << "Main - all 0x" << std::hex << num_threads << " threads created";
    if (xtra_options & 1)
    {   char          cmd[200];
	sprintf( cmd, "echo trace_buffer mappings before join '(#1)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
	sprintf( cmd, "echo trace_buffer mappings before join '(#2)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
    }
    if (num_threads>1) for (ii=0; ii<num_threads; ii++) {
        pthread_join(threads[ii], NULL);
    }
    if (xtra_options & 1)
    {   char          cmd[200];
	sprintf( cmd, "echo trace_buffer mappings after join '(#1)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
	sprintf( cmd, "echo trace_buffer mappings after join '(#2)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
    }
    printf("test-threads - after join loop\n");
    TRACE( 1, "after thread(s) (pthread_join) traceControl_p=%p", traceControl_p );
    free( args_p );
    free( threads );
    return (0);
}   /* main */
EOF

# ^ ^ ^ ^ ^ ^ ^ ^ ^ Done making module source files ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^

opt_j=25
do_once=1
vprintf 0 'Compile subs\n'
nn=1
for ss in sub*.cc; do
   ofile=`basename $ss .cc`
   test -n "${do_shared-}" && out_opts="-fPIC -shared -o $ofile.so" || out_opts="-c -o $ofile.o"
   test -n "$do_once" && { flags=$-; set -x; }
   g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_DIR/include $out_opts $ss &
   test -n "$do_once" && { set +x; set -$flags; do_once=; }
   expr $nn % $opt_j >/dev/null || wait
   nn=`expr $nn + 1`
done
wait

vprintf 0 'Compile main\n'
test -n "${do_shared-}" \
 && { g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_DIR/include -o big_ex_main big_ex_main.cc *.so -lpthread; sts=$?; export LD_LIBRARY_PATH=.${LD_LIBRARY_PATH+:$LD_LIBRARY_PATH}; } \
 || { g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_DIR/include -o big_ex_main big_ex_main.cc *.o  -lpthread; sts=$?; }

test $sts -eq 0 && echo big_ex_main built OK || { echo big_ex_main build FAILED; exit 1; }

test -n "${check_numents-}" \
 || check_numents=`expr \( \( $opt_depth - 1 \) \* $opt_tlogs_per + 35 \) \* $opt_threads \* $opt_loops`

if [ "${do_mapcheck-0}" -gt 0 ];then
    export TRACE_FILE TRACE_NUMENTS TRACE_ARGSMAX TRACE_MSGMAX TRACE_NAMTBLENTS
    TRACE_ARGSMAX=4
    TRACE_MSGMAX=64
    TRACE_NUMENTS=$check_numents
    TRACE_NAMTBLENTS=`expr $opt_threads + 3 + $opt_depth / 10`   # extras: jones, TRACE, _TRACE_ "sub10s"
    TRACE_FILE=/tmp/trace_buffer_`whoami`  # make sure
    rm -f $TRACE_FILE   # master reset :)
    echo check_opts=$check_opts
    uname=`uname`
    expect_static=`expr \( $opt_depth + 1 \) \* 2`
    #expect_declare=`expr 2 + $opt_depth / 10 \* 2` # take into accout the "do_TRACE_NAME" above
    expect_declare=2
    while true; do
        vprintf 0 "Testing... $do_mapcheck\n"

        test -f big_ex_main.out && mv -f big_ex_main.out big_ex_main.out~
        vprintf 1 "executing: ./big_ex_main -njones -x1 $check_opts >big_ex_main.out 2>&1\n"
        time ./big_ex_main -njones -x1 $check_opts >big_ex_main.out 2>&1
        sts=$?
        test $sts -ne 0 && { echo ./big_ex_main FAILED - exit status: $sts; exit 1; }

        parallel_threads=`cat big_ex_main.out | sed -n -e '/num_threads/{s/.*num_threads= *//;s/ .*//;p;}'`
        loops=`cat big_ex_main.out | sed -n -e '/before create/{s/.*loops= *//;s/ .*//;p;}'`
        num_maps=`cat big_ex_main.out | sed -n -e '/after join (#2) = /{s/.*= *//;p;}'`
        xtra_threads=`cat big_ex_main.out | sed -n -e '/xtra_threads/{s/.*xtra_threads= *//;s/ .*//;p;}'`
        test $xtra_threads -eq 0 && check_tids=`expr 1 + $parallel_threads + $opt_threads \* $opt_loops` || check_tids=
        # Note; the OS can randomly recycle tids, so if the program is creating/joining threads,
        # the number of uniq tids that the program will experience is unknown.

        vprintf 0 "\
Analyzing trace_buffer... (n_maps=%d loops=%d pthreads=%d expect:STATIC=%d DECLARE=%d ?tids=%d)\n\
" $num_maps $loops $parallel_threads $expect_static $expect_declare $check_tids

        if [ -f $TRACE_FILE ];then
            trace_cntl info | egrep 'used|full|num_entries' | sed 's/^/  /'
            uniq_addrs=`TRACE_SHOW=HxiICLR trace_cntl show | sed -n -e '/_p=/{s/.*_p=//;s/ .*//;p;}' | sort -u | wc -l`
            vprintf 1 'Calculating sub10_trc_ids...\n'
            sub10_trc_id=`TRACE_SHOW=HxiICLR trace_cntl show | awk '/sub10 tid=/{print$2;}' | sort -u`
            sub10_trc_ids=`echo "$sub10_trc_id" | wc -l`
            test $sub10_trc_ids -eq 1 || sub10_trc_id=-1
            vprintf 1 'Calculating uniq_thr_ids...\n'
            uniq_thr_ids=`TRACE_SHOW=xi trace_cntl show | awk '{print$1;}' | sort -nu | wc -l`
            sub10_trc_id_=`trace_cntl tids | awk '/ sub10 /{print$1;}'`
            last_thr_idx=`expr $opt_threads - 1`
            _TRACE_=`TRACE_SHOW=n trace_cntl show | grep "_TRACE_"`
            vprintf 1 'Calculating delta_min...\n'
            num_entries=`trace_cntl info | awk '/num_entries/{print$3;}'`
            test $num_entries -gt 500000 && num_entries=500000
            start_idx=`expr $num_entries - 1` # smaller buffers will wrap -- smallish number of unused entries have 0 timestamp which tdelta ignores
            delta_min=`TRACE_SHOW=HTm trace_cntl show $num_entries $start_idx | trace_delta.pl -stats | awk '/^  *min /{print$2;}'`
            vprintf 1 'done calculating\n'
        else
            echo "TRACE_FILE not found - FAIL will result"
            uniq_addrs=0 sub10_trc_ids=0 sub10_trc_id=-1 sub10_trc_id_=-2 uniq_thr_ids=0
            _TRACE_=
            delta_min=-1000
        fi
        uniq2=`expr $uniq_addrs \* 2`
        echo "  uniq_addrs=$uniq_addrs uniq2=$uniq2 num_maps=$num_maps sub10_trc_ids="$sub10_trc_ids "sub10_trc_id=$sub10_trc_id uniq_thr_ids=$uniq_thr_ids"
        fail=
        test \( $uniq2 -eq $expect_declare -o $uniq2 -eq $expect_static \) || fail="$fail uniq_addrs"
        test "$sub10_trc_id" -eq $sub10_trc_id_                            || fail="$fail sub10_trc_id"
        test -z "$_TRACE_"                                                 || fail="$fail _TRACE_tid"
        test $delta_min -gt -100                                           || fail="$fail delta_min=$delta_min"
        if [ "$uname" = Linux ];then
            # additional checks
            test $num_maps -eq $uniq2                                      || fail="$fail num_maps"
            test \( $uniq_thr_ids -eq 1 -o -z "$check_tids" -o $uniq_thr_ids -eq "${check_tids:-0}" \) || fail="$fail uniq_thr_ids($uniq_thr_ids!=${check_tids:-0})"
        fi
        test -n "$fail" && { echo FAIL - $fail ; exit 1; } || echo "  SUCCESS"
        #trace_cntl tids
        test "$do_mapcheck" -le 1 && break
        trace_cntl reset
        do_mapcheck=`expr $do_mapcheck - 1`
    done
    ls -l $TRACE_FILE big_ex_main.out*
fi
