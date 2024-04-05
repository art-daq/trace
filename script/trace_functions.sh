#!/bin/sh
#   This file (trace.sh.functions) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jul 15, 2003. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace.sh.functions,v $
#   $Revision: 1679 $
#   $Date: 2024-04-05 16:43:31 -0500 (Fri, 05 Apr 2024) $

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
tdelta()  { test -n "${PAGER-}" && { trace_delta "$@" | $PAGER;true;} || trace_delta "$@"; }  # ex. tshow | grep xxx | tdelta

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
tcolor()   { trace_color "$@"; }

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
