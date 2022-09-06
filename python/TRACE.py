 # This file (trace.py) was created by Ron Rechenmacher <ron@fnal.gov> on
 # May  4, 2022. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: TRACE.py,v $
 # rev="$Revision: 1.7 $$Date: 2022/09/03 21:14:09 $";


import ctypes
import os
import sys

tracelibpath=None
for pp in sys.path:
    if pp == '': pp="."
    tracelibpath=pp+os.path.sep+"libtracelib.so"
    if os.path.exists(tracelibpath): break
#print("tracelibpath=%s"%(tracelibpath,))
libtrace = ctypes.CDLL(tracelibpath); # what about ctypes.CDLL(ff,ctypes.RTLD_GLOBAL )
ctrace0 = libtrace.ctrace0

TLVL_EMERG,TLVL_ALERT,TLVL_CRIT,TLVL_ERROR,TLVL_WARN,TLVL_NOTICE,TLVL_INFO,TLVL_LOG,TLVL_DBG,TLVL_DEBUG_1=0,1,2,3,4,5,6,7,8,9
# aliases
TLVL_FATAL=TLVL_EMERG
TLVL_WARNING=TLVL_WARN
TLVL_DEBUG=TLVL_DBG
TLVL_TRACE=TLVL_DEBUG_1

# See https://docs.python.org/3/library/ctypes.html
# C type "char*" goes to Python type "bytes"
def TRACE( lvl, msg, nam=None ):
    f=sys._getframe(1); co=f.f_code
    if nam == None: nam=co.co_filename; nam=nam[nam.rfind("/")+1:].encode()
    if type(nam) != type(b''): nam = nam.encode()
    if type(msg) != type(b''): msg = msg.encode()
    libtrace.TRACE( nam, lvl, f.f_lineno, co.co_name.encode(), msg )

def TRACE_DEBUG( dbg_lvl, msg, nam=None ):
    f=sys._getframe(1); co=f.f_code
    if nam == None: nam=co.co_filename; nam=nam[nam.rfind("/")+1:].encode()
    if type(nam) != type(b''): nam = nam.encode()
    if type(msg) != type(b''): msg = msg.encode()
    libtrace.TRACE( nam,TLVL_DBG+dbg_lvl,f.f_lineno, co.co_name.encode(), msg )

def TRACE_ERROR( msg, nam=None ):
    f=sys._getframe(1); co=f.f_code
    if nam == None: nam=co.co_filename; nam=nam[nam.rfind("/")+1:].encode()
    if type(nam) != type(b''): nam = nam.encode()
    if type(msg) != type(b''): msg = msg.encode()
    libtrace.TRACE( nam,TLVL_ERROR,f.f_lineno, co.co_name.encode(), msg )

def TRACE_WARN( msg, nam=None ):
    f=sys._getframe(1); co=f.f_code
    if nam == None: nam=co.co_filename; nam=nam[nam.rfind("/")+1:].encode()
    if type(nam) != type(b''): nam = nam.encode()
    if type(msg) != type(b''): msg = msg.encode()
    libtrace.TRACE( nam,TLVL_WARN,f.f_lineno, co.co_name.encode(), msg )

def TRACE_INFO( msg, nam=None ):
    f=sys._getframe(1); co=f.f_code
    if nam == None: nam=co.co_filename; nam=nam[nam.rfind("/")+1:].encode()
    if type(nam) != type(b''): nam = nam.encode()
    if type(msg) != type(b''): msg = msg.encode()
    libtrace.TRACE( nam,TLVL_INFO,f.f_lineno, co.co_name.encode(), msg )

def TRACE_LOG( msg, nam=None ):
    f=sys._getframe(1); co=f.f_code
    if nam == None: nam=co.co_filename; nam=nam[nam.rfind("/")+1:].encode()
    if type(nam) != type(b''): nam = nam.encode()
    if type(msg) != type(b''): msg = msg.encode()
    libtrace.TRACE( nam,TLVL_LOG,f.f_lineno, co.co_name.encode(), msg )

def TRACE_CNTL( cmd, *args ):
    f=sys._getframe(1); co=f.f_code; nam=co.co_filename; nam=nam[nam.rfind("/")+1:].encode()
    nargs=len(args)
    if type(cmd) != type(b''): cmd = cmd.encode()
    libtrace.TRACE_CNTL(nam,nam,nargs,cmd,*args)
