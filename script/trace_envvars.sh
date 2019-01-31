#! /bin/sh
 # This file (trace_envvars.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # May  1, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: trace_envvars.sh,v $
 # rev='$Revision: 682 $$Date: 2017-11-09 14:01:41 -0600 (Thu, 09 Nov 2017) $'

USAGE="\
 usage: `basename $0` [[path/]file]   # default file is trace_cntl
"

args=
while [ -n "${1+x}" ];do
    op=$1;shift
    case "$op" in
    -*) echo "$USAGE";exit;;
    *)  xx=`echo "$op" | sed -e "s/'/'\"'\"'/g"`; args="${args-} '$xx'";;
    esac
done
eval set -- ${args-} \"\$@\"; unset op args xx

test -f /bin/grep  &&  GREP=/bin/grep  ||  GREP=/usr/bin/grep
test -f /bin/egrep && EGREP=/bin/egrep || EGREP=/usr/bin/egrep

tenv()  # $1=file
{   tcntlexe=$1
    envvars=`strings -a $tcntlexe | sed -n -e '/^TRACE_/p' | sort -u`
    # according to trace.h, the following six "activate" TRACE
    list='NAMTBLENTS NUMENTS ARGSMAX MSGMAX NAME FILE LVLM'
    ere=`echo $list | sed 's/ /|/g'`
    envvars=`echo "$envvars" | $EGREP -v "^TRACE_($ere)"`
    for ee in $list;do
        printenv | $GREP "^TRACE_$ee=" || echo TRACE_$ee
    done
    # now the rest/others
    for ee in $envvars;do
        eval echo "$ee\${$ee+=\"'\$$ee'\"}"
    done
}

test $# -gt 0 && file=$1 || file=trace_cntl

expr "$file" : '\.*/' >/dev/null || file=`which $file`
if [ ! -f "$file" ];then exit; fi

tenv $file
