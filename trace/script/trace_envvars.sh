#! /bin/sh
 # This file (trace_envvars.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # May  1, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: trace_envvars.sh,v $
 # rev='$Revision: 1.2 $$Date: 2014-07-02 20:24:37 $'

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


tenv()  # $1=file
{   tcntlexe=$1
    envvars=`strings -a $tcntlexe | sed -n -e'/^TRACE_/p'`
    for ee in $envvars;do echo $ee=`printenv | sed -n -e"/^$ee=/{s/^[^=]*=//;p;}"`; done
}

test $# -gt 0 && file=$1 || file=trace_cntl

expr "$file" : '\.*/' >/dev/null || file=`which $file`
if [ ! -f "$file" ];then exit; fi

tenv $file
