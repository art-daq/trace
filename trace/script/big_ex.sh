#! /bin/sh
 # This file (big_ex.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Mar 10, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: big_ex.sh,v $
 # rev='$Revision: 1.1 $$Date: 2014/03/11 12:58:53 $'

USAGE="\
  usage: `basename $0` <dir>
example: `basename $0` $PWD/big
If directory does not exist, it will be created.
Files in the dir will be overwritten.
"
opt_depth=5
op1chr='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set -- "-$rest" "$@"'
op1arg='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set --  "$rest" "$@"'
reqarg="$op1arg;"'test -z "${1+1}" &&echo opt -$op requires arg. &&echo "$USAGE" &&exit'
args= do_help= opt_v=0
while [ -n "${1-}" ];do
    if expr "x${1-}" : 'x-' >/dev/null;then
        op=`expr "x$1" : 'x-\(.*\)'`; shift   # done with $1
        leq=`expr "x$op" : 'x[^=]*\(=\)'` lev=`expr "x$op" : 'x[^=]*=\(.*\)'`
        test "x$leq" != x &&eval "set -- \"\$lev\" \"\$@\""&&op=`expr "x$op" : 'x\([^=]*\)'`
        case "$op" in
        \?*|h*)     eval $op1chr; do_help=1;;
        v*)         eval $op1chr; opt_v=`expr $opt_v + 1`;;
        x*)         eval $op1chr; test $opt_v -ge 1 && set -xv || set -x;;
        -depth)     eval $reqarg; opt_depth=$1; shift;;
        *)          echo "Unknown option -$op"; do_help=1;;
        esac
    else
        aa=`echo "$1" | sed -e"s/'/'\"'\"'/g"` args="$args '$aa'"; shift
    fi
done
eval "set -- $args \"\$@\""; unset args aa
test $# -ne 1 && echo 'Missing required "directory" argument.' && do_help=1
help() { echo "$USAGE";test $opt_v -ge 1 && echo "$VUSAGE" || true for pipeline; }
test -n "${do_help-}" && help && exit

odir=$1
test -d $odir || mkdir -p $odir

cd $odir

for nn in `seq $opt_depth`;do
    next=`expr $nn + 1`
    cat >sub$nn.cc <<EOF
#include "trace.h"
void sub$next( void );
void sub$nn()
{   TRACE( 0, "sub$nn calling sub$next" );
    sub$next();
}
EOF
done

echo nn=$nn
last=`expr $nn + 1`
cat >sub$last.cc <<EOF
#include "trace.h"

void sub$last()
{   TRACE( 0, "sub$last returning" );
}
EOF


cat >big_ex_main.cc <<EOF
#include <pthread.h>		/* pthread_self */
#include <unistd.h>		/* getopt */
#include "trace.h"

#define USAGE "\
  usage:%s\n\
", basename(argv[0])

void sub1(void);

void* thread_func(void *arg)
{
    long loops=(long)arg;
    for (unsigned ii=0; ii<loops; ++ii)
    {   TRACE( 0, "loop=%u calling sub1",ii );
        sub1();
    }
    pthread_exit(NULL);
}

int main( int argc, char *argv[] )
{
        pthread_t   * threads;
	unsigned      num_threads=4;
extern  char        * optarg;        /* for getopt */
        int           opt;            /* for how I use getopt */
	unsigned long loops=10000;
        unsigned      ii;

    while ((opt=getopt(argc,argv,"?hn:f:l:t:")) != -1)
    {   switch (opt)
        { /* '?' is also what you get w/ "invalid option -- -" */
        case '?': case 'h': printf(USAGE);exit(0);    break;
	case 'n': setenv("TRACE_NAME",optarg,1);      break;
	case 'f': setenv("TRACE_FILE",optarg,1);      break;
	case 'l': loops=strtoul(optarg,NULL,0);       break;
	case 't': num_threads=strtoul(optarg,NULL,0); break;
        }
    }
    threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
    TRACE( 0, "before pthread_create - loops=%lu (must be multiple of 4)", loops );
    printf("test-threads - before create loop - traceControl_p=%p\n",(void*)traceControl_p);
    for (ii=0; ii<num_threads; ii++)
    {   pthread_create(&threads[ii],NULL,thread_func,(void*)loops);
    }
    for (ii=0; ii<num_threads; ii++)
    {   pthread_join(threads[ii], NULL);
    }
    printf("test-threads - after create loop - traceControl_p=%p\n",(void*)traceControl_p);
    TRACE( 0, "after pthread_join" );
    free( threads );
    return (0);
}   /* main */
EOF

echo compile subs
for ss in sub*.cc; do
   ofile=`basename $ss .c`.o
   g++ -std=c++11 -g -Wall -I$TRACE_DIR/include -c -o $ofile $ss
done

echo compile main
g++ -std=c++11 -g -Wall -I$TRACE_DIR/include -o big_ex_main big_ex_main.cc *.o -lpthread



