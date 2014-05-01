#! /bin/sh
 # This file (trace_envvars.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # May  1, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: trace_envvars.sh,v $
 # rev='$Revision: 1.1 $$Date: 2014-05-01 14:27:14 $'

tenv()
{   tcntlexe=`which trace_cntl`
    envvars=`strings -a $tcntlexe | sed -n -e'/^TRACE_/p'`
    for ee in $envvars;do echo $ee=`printenv | sed -n -e"/^$ee=/{s/^[^=]*=//;p;}"`; done
}

tenv
