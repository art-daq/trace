#!/usr/bin/env python
# This file (test_trace.py) was created by Ron Rechenmacher <ron@fnal.gov> on
# May  5, 2022. "TERMS AND CONDITIONS" governing this file are in the README
# or COPYING file. If you do not have such a file, one can be obtained by
# contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
# $RCSfile: test_trace.py,v $
# rev="$Revision: 1.8 $$Date: 2022/09/03 20:45:22 $";

# treset; ./test_trace.py 1;tshow|tac|tdelta -d 1 -ct 1 -i

import TRACE
import os      # os.path.basename
import sys

def one_level(lvl):
    TRACE.TRACE(lvl,"hello %s %d"%("ron",4))
    TRACE.TRACE(lvl,"hello %s %d"%("ron",5))
    TRACE.TRACE(lvl,"hello %s %d"%("ron",6))
    TRACE.TRACE_DEBUG(8,b"b --- no encode")
    TRACE.TRACE_DEBUG(8,b"b --- no encode")
    TRACE.TRACE_DEBUG(8,b"b --- no encode")
    TRACE.TRACE_DEBUG(8,b"b --- no encode")
    TRACE.TRACE_DEBUG(8,b"b --- no encode")
    TRACE.TRACE_DEBUG(8,b"b --- no encode")
    TRACE.TRACE_DEBUG(8, "str - do encode")
    TRACE.TRACE_DEBUG(8, "str - do encode")
    TRACE.TRACE_DEBUG(8, "str - do encode")
    TRACE.TRACE_DEBUG(8, "str - do encode")
    TRACE.TRACE_DEBUG(8, "str - do encode")
    TRACE.TRACE_DEBUG(8, "str - do encode")
    TRACE.TRACE_DEBUG(8, "str %%fmt %s %d"%("ron",5))
    TRACE.TRACE_DEBUG(8, "str %%fmt %s %d"%("ron",6))
    TRACE.TRACE_DEBUG(8, "str %%fmt %s %d"%("ron",7))
    TRACE.TRACE_DEBUG(8, "str %%fmt %s %d"%("ron",8))
    TRACE.TRACE_DEBUG(8, "str %%fnt %s %d"%("ron",9))
    TRACE.TRACE_DEBUG(8,b"b %%fmt b %s %d"%(b"ron",5))
    TRACE.TRACE_DEBUG(8,b"b %%fmt b %s %d"%(b"ron",6))
    TRACE.TRACE_DEBUG(8,b"b %%fmt b %s %d"%(b"ron",7))
    TRACE.TRACE_DEBUG(8,b"b %%fmt b %s %d"%(b"ron",8))
    TRACE.TRACE_DEBUG(8,b"b %%fmt b %s %d"%(b"ron",9))

def two_level(lvl):
    nam=os.path.basename(__file__)   # __file__ is not defined in the interactive interpreter; set by the import implementaion
    nam = nam[nam.rfind("/")+1:]
    msg="qmsg"
    TRACE.TRACE_INFO( msg, "SPECIAL_NAME" )
    TRACE.TRACE_DEBUG( lvl,msg )
    TRACE.TRACE_DEBUG( lvl,msg,nam )

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("run via: treset;./test_trace.py 1 | PAGER= tdelta -d 1 -i && TRACE_SHOW='%H%x%N %T %P %i %C %e %.3L %m' tshow | PAGER= tdelta")
        sys.exit(0)
    lvl=7
    one_level(lvl)
    two_level(lvl)
    nam=os.path.basename(__file__)   # __file__ is not defined in the interactive interpreter; set by the import implementaion
    nam = nam[nam.rfind("/")+1:]
    msg="qmsg".encode()
    TRACE.TRACE( TRACE.TLVL_DEBUG+lvl,msg,nam )
    TRACE.TRACE_INFO( msg,nam )
    TRACE.TRACE_ERROR( "This is an error." )
    TRACE.TRACE_WARN( "This is a warning.", "new NAME" )

    TRACE.TRACE_CNTL("printfd",5)
