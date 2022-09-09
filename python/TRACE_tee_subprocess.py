#!/usr/bin/env python
# This file (TRACE_tee_subprocess.py) was created by Ron Rechenmacher <ron@fnal.gov> on
# Sep  3, 2022. "TERMS AND CONDITIONS" governing this file are in the README
# or COPYING file. If you do not have such a file, one can be obtained by
# contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
# $RCSfile: .emacs.gnu,v $
# rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

# rm -f test.log; ./TRACE_tee_subprocess.py; cat test.log

import TRACE
import subprocess

def sub1():
    TRACE.INFO('hello')

if __name__ == '__main__':
    p0= subprocess.Popen(['tee','-a','test.log'],stdin=subprocess.PIPE, text=True, bufsize=1, universal_newlines=True)
    TRACE.CNTL('printfd',p0.stdin.fileno())
    sub1()
