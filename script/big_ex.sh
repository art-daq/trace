#! /bin/sh
 # This file (big_ex.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Mar 10, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: big_ex.sh,v $
 # rev='$Revision: 490 $$Date: 2016-01-23 13:55:50 -0600 (Sat, 23 Jan 2016) $'
set -u
opt_depth=15
opt_std=c++11
do_define=1
do_declare=1
do_mapcheck=2
check_opts='-l5000 -t7'
check_numents=2000000  # maybe compute what this should be
USAGE="\
   usage: `basename $0` <dir>
examples: `basename $0` ./big_ex.d
          `basename $0` ./big_ex.d --std=
          `basename $0` ./big_ex.d -DTRACE_STATIC -d200 --mapcheck=2 --check-opts='-l1000 -t75' --check-numents=16000000
          `basename $0` ./big_ex.d -d200 --mapcheck=2 --check-opts='-l1000 -t75' --check-numents=16000000
          `basename $0` ./big_ex.d -DNO_TRACE -d200 --check-opts='-l1000 -t75' --check-numents=16000000
          `basename $0` ./big_ex.d --no-define --no-declare -d25 -std=
If directory does not exist, it will be created.
Files in the dir will be overwritten.
--depth=   defalut is $opt_depth, but a better test might be 500
-DTRACE_DECLARE  add -DTRACE_DECLARE to compile line
-DTRACE_STATIC   add -DTRACE_STATIC to compile line
--std=<c++std>  -std=   both single and double - work
--no-define      remvoe #define TRACE_DEFINE from subs (could also do -DTRACE_STATIC)
--no-declare     remove #define TRACE_DECLARE from main
--mapcheck[=num] default=$do_mapcheck, the number of check loops
--check-opts=<opts> default=$check_opts
----check-numents=<ents>

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
        -std)       eval $reqarg; opt_std=$1;                 shift;;
        std)                      opt_std=$1;                 shift;;
        -no-define) do_define= ;;
        -no-declare)do_declare= ;;
        -mapcheck)  test -n "$leq"&&do_mapcheck=$1&&shift||do_mapcheck=1;;
        -check-opts)eval $reqarg; check_opts=$1; shift;;
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
test -n "${do_help-}" && help && exit

test $opt_depth -ge 1 || { echo "depth option cannot be 0"; exit 1; }
hash trace_cntl || { echo "trace_cntl program must be in PATH"; exit 1; }

odir=$1
test -d $odir || mkdir -p $odir

cd $odir
# Clean up
rm -f sub*.cc sub*.o
rm -f big_ex_main.cc big_ex_main big_ex_main.out

# - - - - - - - - - - - Make all the sub modules (except for the last 1) - - -

echo opt_depth=$opt_depth
nn=1
while [ $nn -lt $opt_depth ];do
    next=`expr $nn + 1`
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
#endif

struct args { pid_t tid; unsigned loop; };
void sub$next( struct args *aa );
void sub$nn( struct args *aa )
{   TRACE( 0,"sub$nn loop=%u tid=%d calling sub$next tC_p=%p %u=tIL_hung_max",aa->loop,aa->tid,traceControl_p,traceInitLck_hung_max);
    sub$next(aa);
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
#endif

struct args { pid_t tid; unsigned loop; };

void sub$last( struct args *aa )
{   TRACE( 0, "sub$last loop=%u tid=%d returning tC_p=%p %u=tIL_hung_max",aa->loop,aa->tid,traceControl_p,traceInitLck_hung_max);
}
EOF


# - - - - - - - - - - - Make the main module - - - - - -

cat >big_ex_main.cc <<EOF
#include <stdio.h>              // printf
#include <stdlib.h>             // exit
#include <pthread.h>		// pthread_self
#include <unistd.h>		// getopt
#include <sys/types.h>          // pid_t
#include <sys/syscall.h>	// syscall
#include <libgen.h>             // basename
${do_define:+#define TRACE_DEFINE // trace variables are defined in this module}
#ifndef NO_TRACE
# include "trace.h"
#else
# define TRACE(...)
#endif

#define USAGE "\
  usage: %s [options]\n\
example: %s -x1\n\
options:\n\
-n<TRACE_NAME>\n\
-f<TRACE_FILE>\n\
-l<loops>\n\
-t<num_threads>\n\
-x<mask>  extra options - b0=count maps\n\
", basename(argv[0]), basename(argv[0])

static int xtra_options=1;

struct args { pid_t tid; unsigned loop; };
void sub1( struct args *aa );

void* thread_func(void *arg)
{
    long loops=(long)arg;
    struct args aa;
    aa.tid=syscall(TRACE_GETTID);
    for (unsigned ii=0; ii<loops; ++ii)
    {   TRACE( 0, "tf loop=%u tid=%d calling sub1 tC_p=%p %u=tIL_hung_max",ii,aa.tid,traceControl_p,traceInitLck_hung_max);
        aa.loop=ii;
        if (xtra_options & 2)
        {   pthread_t thread;
            pthread_create(&thread,NULL,(void*(*)(void*))sub1,(void*)&aa);
            pthread_join(thread, NULL);
        }
        else
            sub1(&aa);
    }
    pthread_exit(NULL);
}

int main( int argc, char *argv[] )
{
        pthread_t   * threads;
	unsigned      num_threads=4;
extern  char        * optarg;        // for getopt
        int           opt;           // for how I use getopt
	unsigned long loops=500;
        unsigned      ii;

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
    TRACE( 0, "b4 pthread_create - loops=%lu tC_p=%p %u=tIL_hung_max", loops, traceControl_p, traceInitLck_hung_max );
    printf("test-threads - before create loop - loops=%lu num_threads=%u xtra_threads=%d\n",
           loops,num_threads,!!(xtra_options&2));
    for (ii=0; ii<num_threads; ii++)
    {   pthread_create(&threads[ii],NULL,thread_func,(void*)loops);
    }
    if (xtra_options & 1)
    {   char          cmd[200];
	sprintf( cmd, "echo trace_buffer mappings before join '(#1)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
	sprintf( cmd, "echo trace_buffer mappings before join '(#2)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
    }
    for (ii=0; ii<num_threads; ii++)
    {   pthread_join(threads[ii], NULL);
    }
    if (xtra_options & 1)
    {   char          cmd[200];
	sprintf( cmd, "echo trace_buffer mappings after join '(#1)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
	sprintf( cmd, "echo trace_buffer mappings after join '(#2)' = \`cat /proc/%d/maps | grep trace_buffer | wc -l\`", getpid() );
	system( cmd );
    }
    printf("test-threads - after join loop\n");
    TRACE( 0, "after pthread_join traceControl_p=%p", traceControl_p );
    free( threads );
    return (0);
}   /* main */
EOF

# ^ ^ ^ ^ ^ ^ ^ ^ ^ Done making module source files ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^

opt_j=25
do_once=1
echo Compile subs
nn=1
for ss in sub*.cc; do
   ofile=`basename $ss .c`.o
   test -n "$do_once" && set -x
   g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_DIR/include -c -o $ofile $ss &
   test -n "$do_once" && { set +x; do_once=; }
   expr $nn % $opt_j >/dev/null || wait
   nn=`expr $nn + 1`
done
wait

echo Compile main
g++ ${opt_std:+-std=$opt_std} $compile_opts -g -Wall -I$TRACE_DIR/include -o big_ex_main big_ex_main.cc *.o -lpthread
sts=$?

test $sts -eq 0 && echo big_ex_main built OK || { echo big_ex_main build FAILED; exit 1; }

if [ "${do_mapcheck-0}" -gt 0 ];then
    export TRACE_FILE TRACE_NUMENTS TRACE_ARGSMAX TRACE_MSGMAX
    TRACE_ARGSMAX=4
    TRACE_MSGMAX=48
    TRACE_NUMENTS=$check_numents
    TRACE_FILE=/tmp/trace_buffer_`whoami`  # make sure
    rm -f $TRACE_FILE
    echo check_opts=$check_opts
    uname=`uname`
    expect_static=`expr \( $opt_depth + 1 \) \* 2`
    #expect_declare=`expr 2 + $opt_depth / 10 \* 2` # take into accout the "do_TRACE_NAME" above
    expect_declare=2
    while true; do
        echo Testing... $do_mapcheck

        time ./big_ex_main -njones -x3 $check_opts >big_ex_main.out 2>&1

        parallel_threads=`cat big_ex_main.out | sed -n -e '/num_threads/{s/.*num_threads= *//;s/ .*//;p;}'`
        loops=`cat big_ex_main.out | sed -n -e '/loops/{s/.*loops= *//;s/ .*//;p;}'`
        num_maps=`cat big_ex_main.out | sed -n -e '/after join (#2) = /{s/.*= *//;p;}'`
        xtra_threads=`cat big_ex_main.out | sed -n -e '/xtra_threads/{s/.*xtra_threads= *//;s/ .*//;p;}'`
        test $xtra_threads -eq 0 && check_tids=`expr 1 + $parallel_threads` || check_tids=
        # Note; the OS can randomly recycle tids, so if the program is creating/joining threads,
        # the number of uniq tids that the program will experience is unknown.

        printf "\
Analyzing trace_buffer... (n_maps=%d loops=%d pthreads=%d expect:STATIC=%d DECLARE=%d ?tids=%d)\n\
" $num_maps $loops $parallel_threads $expect_static $expect_declare $check_tids

        uniq_addrs=`TRACE_SHOW=HxiICLR trace_cntl show | sed -n -e '/_p=/{s/.*_p=//;s/ .*//;p;}' | sort -u | wc -l`
        sub10_tid=`TRACE_SHOW=HxiICLR trace_cntl show | awk '/sub10 /{print$2;exit;}'`
        uniq_tids=`TRACE_SHOW=xi trace_cntl show | awk '{print$1;}' | sort -u | wc -l`
        uniq2=`expr $uniq_addrs \* 2`
        trace_cntl info | egrep 'used|full|num_entries' | sed 's/^/  /'
        echo "  uniq_addrs=$uniq_addrs uniq2=$uniq2 num_maps=$num_maps sub10_tid=$sub10_tid uniq_tids=$uniq_tids"
        if [ "$uname" = Linux ];then
            test \( $uniq2 -eq $expect_declare -o $uniq2 -eq $expect_static \) -a "$sub10_tid" -eq 1\
                 -a $num_maps -eq $uniq2 -a \( $uniq_tids -eq 1 -o -z "$check_tids" -o $uniq_tids -eq "${check_tids:-0}" \)\
              && echo "  SUCCESS" || { echo FAIL; exit 1; }
        else
            test \( $uniq2 -eq $expect_declare -o $uniq2 -eq $expect_static \) -a "$sub10_tid" -eq 1\
              && echo "  SUCCESS" || { echo FAIL; exit 1; }
        fi
        test "$do_mapcheck" -le 1 && break
        trace_cntl reset
        do_mapcheck=`expr $do_mapcheck - 1`
    done
    ls -l $TRACE_FILE
fi
