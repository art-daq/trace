#!/bin/sh
#   This file (trace.sh.functions) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jul 15, 2003. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace.sh.functions,v $
#   $Revision: 1095 $
#   $Date: 2019-04-05 10:17:42 -0500 (Fri, 05 Apr 2019) $

tcntl()   { trace_cntl "$@"; }
tshow()   { test -n "${PAGER-}" && trace_cntl show "$@" | $PAGER || trace_cntl show "$@"; }
tinfo()   { trace_cntl info "$@"; }   # the linux getopt by default will process all
ttids()   { trace_cntl tids "$@"; }   #    args so -f<file> can be used. (Darwin's getopt
tlvls()   { trace_cntl tids "$@"; }   #    args so -f<file> can be used. (Darwin's getopt
tlvlM()   { trace_cntl lvlmskM "$@"; }  #  does not :(
tlvlS()   { trace_cntl lvlmskS "$@"; }
tlvlT()   { trace_cntl lvlmskT "$@"; }
tlvlMg()  { trace_cntl lvlmskMg "$@"; }
tlvlSg()  { trace_cntl lvlmskSg "$@"; }
tlvlTg()  { trace_cntl lvlmskTg "$@"; }
tmode()   { trace_cntl mode  "$@"; }
tmodeM()  { trace_cntl modeM "$@"; }
tmodeS()  { trace_cntl modeS "$@"; }
tfreeze() { trace_cntl modeM 0; }
treset()  { trace_cntl reset; }
tdelta()  { test -n "${PAGER-}" && trace_delta "$@" | $PAGER || trace_delta "$@"; }  # ex. tshow | grep xxx | tdelta

bitN_to_mask()
{
    opts= args=
    op1arg='xx=`expr "x$op" : "x-.\(.*\)"`; test -n "$xx" && set -- "$xx" "$@"'
    while [ -n "${1+x}" ];do
        op=$1;shift
        case "$op" in
        -f*) eval $op1arg; opts="${opts:+$opts }-f$1"; shift;;
        -n*) eval $op1arg; opts="${opts:+$opts }-n$1"; shift;;
        -N*) eval $op1arg; opts="${opts:+$opts }-N$1"; shift;;
        *)  xx=`echo "$op" | sed -e "s/'/'\"'\"'/g"`; args="${args-} '$xx'";;
        esac
    done
    eval set -- ${args-} \"\$@\"; unset op args xx

    spec=$*
    prev=
    while [ "x$spec" != "x$prev" ];do
        prev=$spec
        spec=`echo $spec | sed 's/\([0-9]\) \([0-9]\)/\1,\2/g'` # put , between spaced nums
    done
    spec=`echo $spec | sed 's/ //g'`  # git rid of spaces (ie. "4- 7" -> "4-7"
    rr=
    for xx in `echo $spec | sed 's/,/ /g'`;do
        start=`expr "$xx" : '\([^-]*\)'`
        end=`expr "$xx" : '[^-]*-\(.*\)'`; test -z "$end" && end=$start
        test $start -le $end && inc=1 || inc=-1
        for ii in `seq $start $inc $end`; do rr="${rr:+$rr }$ii"; done
    done
    test -n "$opts" && printf -- "$opts "
    #echo $rr|awk '{for(i=1;i<NF+1;++i)m=or(m,lshift(1,$i));printf"0x%x\n",m;}'
    echo $rr|perl -e '$_=<STDIN>;@l = split;foreach$w(@l){$rr|=1<<$w}printf"0x%x\n",$rr'
}

# could implement the "begins with 0x -> mask, otherwise list of bits???
tonM()     { trace_cntl lvlset  `bitN_to_mask "$@"` 0 0; trace_cntl modeM 1; }
tonS()     { trace_cntl lvlset  0 `bitN_to_mask "$@"` 0; trace_cntl modeS 1; }
tonT()     { trace_cntl lvlset  0 0 `bitN_to_mask "$@"`; }
toffM()    { trace_cntl lvlclr  `bitN_to_mask "$@"` 0 0; }
toffS()    { trace_cntl lvlclr  0 `bitN_to_mask "$@"` 0; }
toffT()    { trace_cntl lvlclr  0 0 `bitN_to_mask "$@"`; }
tonMg()    { trace_cntl lvlsetg `bitN_to_mask "$@"` 0 0; trace_cntl modeM 1; }
tonSg()    { trace_cntl lvlsetg 0 `bitN_to_mask "$@"` 0; trace_cntl modeS 1; }
tonTg()    { trace_cntl lvlsetg 0 0 `bitN_to_mask "$@"`; }
toffMg()   { trace_cntl lvlclrg `bitN_to_mask "$@"` 0 0; }
toffSg()   { trace_cntl lvlclrg 0 `bitN_to_mask "$@"` 0; }
toffTg()   { trace_cntl lvlclrg 0 0 `bitN_to_mask "$@"`; }

tenv()
{   tcntlexe=`which trace_cntl`
    envvars=`strings -a $tcntlexe | sed -n -e '/^TRACE_/p' | sort -u`
    list='NAMTBLENTS NUMENTS ARGSMAX MSGMAX NAME FILE LVLM'
    ere=`echo $list | sed 's/ /|/g'`
    envvars=`echo "$envvars" | egrep -v "^TRACE_($ere)"`
    for ee in $list;do
        printenv | grep "^TRACE_$ee=" || echo TRACE_$ee
    done
    for ee in $envvars;do
        eval echo "$ee\${$ee+=\"'\$$ee'\"}"
    done
}

tlvlsSave() {
  if [ $# -ne 1 ];then
    echo "\
   usage: tlvlsSave <id>   # where <id> is valid shell variable characters
          tlvlsRestore [id] # if [id] not given, read stdin
examples: export TRACE_MSGMAX=0; tlvlM 0xdeadbeef; tlvls; tlvlsSave test; rm -f /tmp/trace_buffer_$USER; tlvls; tlvlsRestore test; tlvls
          tlvlsRestore test | sed /TRACE/s/0x./0x7/ | tlvlsRestore; tlvls
"
  else
    x=`tlvls -H`
    eval tlvls_save_$1=\$x
  fi
}
tlvlsRestore() {
  if [ $# -gt 1 ];then
    tlvlsSave
  else
    test $# -eq 1 && eval x=\$tlvls_save_$1 || x=`cat`
    echo "$x" | while read id name mskM mskS mskT;do
      test -t 1 && trace_cntl lvlmsk -n$name $mskM $mskS $mskT || echo $id $name $mskM $mskS $mskT
    done
  fi
}
