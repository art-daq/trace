#!/bin/sh
#   This file (trace.sh.functions) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jul 15, 2003. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace.sh.functions,v $
#   $Revision: 1563 $
#   $Date: 2022-09-14 16:56:09 -0500 (Wed, 14 Sep 2022) $

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

# bitN_to_mask use to be a function and now it is a script

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

tenv()     { trace_envvars "$@"; }
tcolor()   { : $*=pids or other "token" which will have space before and after
  : build sed filter;: defaults: 1=red 2=green 3=orange 4=blue 5=magenta 6=cyan 7=while 226=yellow ... 
  test -n "${color_nums-}" || color_nums='1 2 3 4 5 6 7 226 93 87 189 195 177';: can put color_nums in env
  num_color_nums=`echo $color_nums | wc -w`
  test $# -eq 0 && { pid_list=`TRACE_SHOW="%x%P" trace_cntl show|awk '{print$1}'|head -300|sort -u | head -n$num_color_nums`; echo found `echo $pid_list | wc -w` unique PIDs in 1st 300 tshow lines >&2; set -- $pid_list; };: at most,could be less
  test $# -eq 0 && { echo need at least 1 tid; return; }
  tid=$1; esc=`printf "\033"`;: no shift to preserver num args to associate with arg with cnum
  filt="s/\( $tid .*$\)/${esc}[38;5;`echo $color_nums|cut -d' ' -f1`m\1${esc}[0m/"
  for num in `seq 2 $#`;do
    tid=$2; shift
    cnum=`echo $color_nums|cut -d' ' -f$num`
    filt="$filt;s/\( $tid .*$\)/${esc}[38;5;${cnum}m\1${esc}[0m/"
  done
  sed -e "$filt"
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
