#! /bin/sh
 # This file (trace_feature_test.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb  1, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: trace_feature_test.sh,v $
 # rev='$Revision: 1.6 $$Date: 2014-04-16 17:50:59 $'

USAGE="\
  usage: `basename $0` <\"check\">...
example: `basename $0` -f \`which trace_cntl\`
checks:
-f | --file=<file> check for features in a specific executable
"
VUSAGE=

# Process script arguments and options
eval env_opts=\${$env_opts_var-} # can be args too
eval "set -- $env_opts \"\$@\""
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
        f*)         eval $op1arg; file=$1;    have_check=1; shift;;
        -file)      eval $reqarg; file=$1;    have_check=1; shift;;
        c*)         eval $op1chr; compiler=1; have_check=1;;
        -compiler)                compiler=1; have_check=1;;
        -std)       eval $reqarg; standard=$1;              shift;;
        std)                      standard=$1;              shift;;
        *)          echo "Unknown option -$op"; do_help=1;;
        esac
    else
        aa=`echo "$1" | sed -e"s/'/'\"'\"'/g"` args="$args '$aa'"; shift
    fi
done
eval "set -- $args \"\$@\""; unset args aa
help() { echo "$USAGE";test $opt_v -ge 1 && echo "$VUSAGE" || true for pipeline; }
test -n "${do_help-}" && help && exit


if [ -z "${have_check-}" ];then echo "$USAGE"; exit 1;fi


if [ -n "${file-}" ];then
    if objdump --disassemble $file | grep -m1 lock;then
        echo "This file seems to have the basic multi-process locked memory
access functionality."
    else
        echo "This file does not seem to have the basic multi-process locked
memory access functionality."
    fi
    if objdump --disassemble $file | grep -m1 '%[fg]s:0xf';then
        echo "This file seems to have basic thread local storage functionality"
    else
        echo "This file does not seem to have basic thread local storage functionality"
    fi
fi

if [ -n "${compiler-}" ];then
    if [ -z "${TRACE_INC-}" ];then
        echo "env var TRACE_INC must be set"
    else
        if [ -n "${standard-}" ];then
            expr "$standard" : 'c++' >/dev/null && xc=-xc++ ||  xc=-xc
            standard=-std=$standard
        fi
        tmpfile=/tmp/trace_feature.$$.c
        cat >$tmpfile <<EOF
#include <stdio.h>              /* printf */
#include "trace.h"
int main( int argc, char *argv[] )
{   return (0);
}   /* main */
EOF
        gcc -pedantic -Wall -Winline $xc $standard -I$TRACE_INC -o/dev/null $tmpfile
        rm -f $tmpfile
    fi
fi

exit

build directory:

on 32b with build from 64b -- have 32b exe in build...32 dir
on 32b with build from 32b -- have 32b exe in build dir
on 64b with build from 32b -- have 32b exe in build dir
on 64b with build from 64b -- have 32b exe in build...32 dir and 64b in build


compile "just" (just include and single TRACE; no NAME) no warnings
compile "just" -DNO_TRACE no warnings

run
  no existing file - defaults (no NAME) - no env
  no existing file - NAME               - no env
  no existing file - NAME               - env override
  problem creating file
  problem accessing existing file

  file processing:
      check env.
      check /proc/trace/buffer
      try $HOME/trace_buffer



  "new name" entry sets specified "default" lvl masks
    --- methods to set level masks (regardless of name existing or not)  (Trace_cntl utility should use this)
        methods to set level masks only if name does not exist.   (programs should use this)


lvlmsks and name
  name override from env var
  lvlmsks override from env var


nameclr  (erase)



trigger feature
    set trigger mask(s)
    arm (set arm/active/post)
  armPost==0 ==> nothing
  armPost==-1 postComplete
  armPost>0  

     