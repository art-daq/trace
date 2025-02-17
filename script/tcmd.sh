#!/bin/sh
 # This file (tcmd.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Jan 17, 2018. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: .emacs.gnu,v $
 # rev='$Revision: 1.30 $$Date: 2016/03/01 14:27:27 $'

USAGE="\
usage: `basename $0` -<command> [opts]   # all commands begin with letter 't'
`basename $0 .sh|tr a-z A-Z`_OPTS='-f\"sdfsdf     \" -x' `basename $0` -tinfo
"
vecho() { lvl=$1; shift; test $opt_v -ge $lvl && echo "$@"; }
help() {
    test -z "${do_help-}" && return
    test $# -eq 0 && cmds=$cmd || cmds=$*
    longest=0
    for cc in $cmds;do len=`printf $cc | wc -c`; test $len -gt $longest && longest=$len;done
    for cc in $cmds;do
        printf "%*s - " $longest $cc
        case $cc in
        tcntl)            echo "access trace_cntl for commands such as TRACE or test*"
                          vecho 1 "`trace_cntl 2>&1 | pr --indent=16 --omit-header`"
                ;;
        tshow)            echo "show the memory buffer -- check environment via \"tenv\""
                ;;
        tinfo)            echo "trace config and buffer info"
                ;;
        ttids*|tlvls)     echo "names and masks for memory, \"slow\" and trigger"
                ;;
        tlvl*|tlvl[MST]g) echo "set the mask"
                ;;
        ton*|ton[MST]g)   echo "turn on (set) levels (bits)"
                ;;
        toff*|toff[MST]g) echo "turn off (clear) levels (bits)"
                ;;
        tmode*|tmode[MS]) echo "return or set mode bits; tmodeM [0|1] - memory bit (b0), tmodeS [0|1] - slow path bit (b1)"
                ;;
        tfreeze)          echo "disable the memory tracing"
                ;;
        treset)           echo "clear the memory buffer and trigger parameters"
                ;;
        tenv)             echo "list tbe TRACE environment variables and their values, if any"
                ;;
        tdelta)           echo "\"unix filter\" to display the numeric delta values between lines"
                ;;
        tlvlsSave)        echo "basically: tlvls >file"
                ;;
        tlvlsRestore)     echo "restore names/levels from previously save file"
                ;;
        tcmd.sh)
            echo "$USAGE"
            echo "commands:"
            ccc=`awk '/^case/,/^esac/' $0 |$GREP -v tcmd| $GREP -o '^ *t[][|a-zA-Z]*'`
            help $ccc
            ;;
        *)      echo "no help for $cmd"
        esac
    done
    exit
}

opt_v=0
eval env_opts=\${`basename $0 | sed 's/\.sh$//' | tr 'a-z-' 'A-Z_'`_OPTS-} # can be args too
eval "set -- $env_opts \"\$@\""
# Process script arguments and options
op1chr='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set -- "-$rest" "$@"'
op1arg='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set -- "$rest"  "$@"'
reqarg="$op1arg;"'test -z "${1+1}" &&echo opt -$op requires arg. &&echo "$USAGE" &&exit'
args= do_help=
while [ -n "${1-}" ];do
    if expr "x${1-}" : 'x-' >/dev/null;then
        op=`expr "x$1" : 'x-\(.*\)'`; shift   # done with $1
        leq=`expr "x$op" : 'x-[^=]*\(=\)'` lev=`expr "x$op" : 'x-[^=]*=\(.*\)'`
        test -n "$leq"&&eval "set -- \"\$lev\" \"\$@\""&&op=`expr "x$op" : 'x\([^=]*\)'`
        case "$op" in
        \?*|h*)    eval $op1chr; do_help=1;;
        v*)        eval $op1chr; opt_v=`expr $opt_v + 1`;;
        x*)        eval $op1chr; test $opt_v -ge 1 && set -xv || set -x;;
        t*)        eval $op1arg; opt_t=$1;      shift;;
        F*)        eval $op1chr; opt_F=-F;;               # intercept -F for tshow (tdelta)
#        n*)        eval $op1arg; opt_n=$1;      shift;;
#        N*)        eval $reqarg; opt_N=$1;      shift;;
#        f*)        eval $reqarg; opt_f=$1;      shift;;
        *)         # allow mix of opts we know, opts we don't and args
                    test -n "$leq" && op="$op=$lev" && shift
                    aa=`echo "x-$op" | sed -e "s/^x//;s/'/'\"'\"'/g"`
                    args="${args-} '$aa'";;
        esac
    else # allow mix of opts and arg (i.e. opts after args)
        aa=`echo "$1" | sed -e "s/'/'\"'\"'/g"` args="$args '$aa'"; shift
    fi
done
eval "set -- $args \"\$@\""; unset args aa
set -u

test -f /bin/grep  &&  GREP=/bin/grep  ||  GREP=/usr/bin/grep
test -f /bin/egrep && EGREP=/bin/egrep || EGREP=/usr/bin/egrep

bitN_to_mask() {
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
    echo $rr|perl -e '$_=<STDIN>;@l = split;foreach$w(@l){$rr|=1<<$w}printf"0x%x\n",$rr'
}

tenv() { # $1=file
    test $# -gt 0 && file=$1 || file=trace_cntl
    expr "$file" : '\.*/' >/dev/null || file=`which $file`
    if [ ! -f "$file" ];then return 1; fi
    tcntlexe=$file
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

tlvlsSave() {
  if [ $# -ne 1 ];then
    echo "\
   usage: tlvlsSave <id>   # where <id> is valid shell variable characters
          tlvlsRestore [id] # if [id] not given, read stdin
examples: export TRACE_MSGMAX=0; tlvlM 0xdeadbeef; tlvls; tlvlsSave test; rm -f /tmp/trace_buffer_$USER; tlvls; tlvlsRestore test; tlvls
          tlvlsRestore test | sed /TRACE/s/0x./0x7/ | tlvlsRestore; tlvls
"
  else
    trace_cntl tids -H >$1
  fi
}
tlvlsRestore() {
  if [ $# -gt 1 ];then
    tlvlsSave
  else
    test $# -eq 1 && { test -f $1 || { echo file $1 not found; return 1; } }
    test $# -eq 1 && x=`cat $1` || x=`cat`
    echo "$x" | while read id name mskM mskS mskT;do
      test -t 1 && trace_cntl lvlmsk -n$name $mskM $mskS $mskT || echo $id $name $mskM $mskS $mskT
    done
  fi
}

test -n "${opt_t-}" && cmd=t$opt_t || cmd=`basename $0`
case "$cmd" in
tcntl)                help; exec trace_cntl "$@";;
tshow)                help; test -t 1 -a -n "${PAGER-}" -a -z "${opt_F-}" \
                        && { trace_cntl show ${opt_F-} "$@" | $PAGER;true;} || exec trace_cntl show ${opt_F-} "$@";;
tinfo)                help; exec trace_cntl info "$@";;
ttids|tlvls)          help; exec trace_cntl tids "$@";;
tlvl[MST]|tlvl[MST]g) help; rest=`expr "$cmd" : '....\(.*\)'`; exec trace_cntl lvlmsk$rest "$@";;
ton[MST]|ton[MST]g)   help; rest=`expr "$cmd" : '...\(.*\)'`; trace_cntl lvlset$rest `bitN_to_mask "$@"` 0 0
                      exec trace_cntl modeM 1;;
toff[MST]|toff[MST]g) help; rest=`expr "$cmd" : '....\(.*\)'`; exec trace_cntl lvlclr$rest `bitN_to_mask "$@"` 0 0;;
tmode|tmode[MS])      help; exec trace_cntl mode "$@";;
tfreeze)              help; exec trace_cntl modeM 0 "$@";;
treset)               help; exec trace_cntl reset "$@";;
tenv)                 help; tenv "$@";;
tdelta)               help; test -t 1 -a -n "${PAGER-}" -a -z "${opt_F-}" \
                        && { trace_delta "$@" | $PAGER;true; } || exec trace_delta "$@";;
tlvlsSave)            help; tlvlsSave "$@";;
tlvlsRestore)         help; tlvlsRestore "$@";;
tcmd.sh)              do_help=1; help;;
esac
