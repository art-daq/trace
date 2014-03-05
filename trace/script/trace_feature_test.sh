#! /bin/sh
 # This file (trace_feature_test.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb  1, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: trace_feature_test.sh,v $
 # rev='$Revision: 1.3 $$Date: 2014-03-05 23:05:15 $'

USAGE="\
`basename $0` <file>
"
if [ $# -ne 1 ];then echo "$USAGE"; exit 1;fi

file=$1

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

     