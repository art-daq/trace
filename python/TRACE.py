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
#for pp in sys.path:
#    if pp == '': pp="."
#    tracelibpath=pp+os.path.sep+"libtracelib.so"
#    if os.path.exists(tracelibpath): break
tracelibpath='libtracelib.so' # LD_LIBRARY_PATH should be searched
#print("tracelibpath=%s"%(tracelibpath,))
libtrace = ctypes.CDLL(tracelibpath); # what about ctypes.CDLL(ff,ctypes.RTLD_GLOBAL )
ctrace0 = libtrace.ctrace0

# Note: TRACE LEVELS mirror (mostly) linux kernel syslog levels. See syslog(2)
TLVL_EMERG,TLVL_ALERT,TLVL_CRIT,TLVL_ERROR,TLVL_WARN,TLVL_NOTICE,TLVL_INFO,TLVL_LOG,TLVL_DBG,TLVL_DEBUG_1=0,1,2,3,4,5,6,7,8,9
# aliases
TLVL_FATAL=TLVL_EMERG
TLVL_WARNING=TLVL_WARN
TLVL_DEBUG=TLVL_DBG
TLVL_TRACE=TLVL_DEBUG_1

Instance=b''  # must be bytes

# See https://docs.python.org/3/library/ctypes.html
# C type "char*" goes to Python type "bytes"
def TRACE( lvl, msg, name=None ):
    "TRACE( lvl, msg, name=None )"
    f=sys._getframe(1); co=f.f_code
    if name == None: name=co.co_filename; name=name[name.rfind("/")+1:].encode()
    if type(name) != type(b''): name = name.encode()
    if type(msg) != type(b''): msg = msg.encode()
    if Instance: name = Instance+b':'+name
    libtrace.TRACE( name, lvl, f.f_lineno, co.co_name.encode(), msg )

def DEBUG( dbg_lvl, msg, name=None ):
    "DEBUG( dbg_lvl, msg, name=None )"
    f=sys._getframe(1); co=f.f_code
    if name == None: name=co.co_filename; name=name[name.rfind("/")+1:].encode()
    if type(name) != type(b''): name = name.encode()
    if type(msg) != type(b''): msg = msg.encode()
    if Instance: name = Instance+b':'+name
    libtrace.TRACE( name,TLVL_DBG+dbg_lvl,f.f_lineno, co.co_name.encode(), msg )

def ERROR( msg, name=None ):
    "ERROR( msg, name=None )"
    f=sys._getframe(1); co=f.f_code
    if name == None: name=co.co_filename; name=name[name.rfind("/")+1:].encode()
    if type(name) != type(b''): name = name.encode()
    if type(msg) != type(b''): msg = msg.encode()
    if Instance: name = Instance+b':'+name
    libtrace.TRACE( name,TLVL_ERROR,f.f_lineno, co.co_name.encode(), msg )

def WARN( msg, name=None ):
    "WARN( msg, name=None )"
    f=sys._getframe(1); co=f.f_code
    if name == None: name=co.co_filename; name=name[name.rfind("/")+1:].encode()
    if type(name) != type(b''): name = name.encode()
    if type(msg) != type(b''): msg = msg.encode()
    if Instance: name = Instance+b':'+name
    libtrace.TRACE( name,TLVL_WARN,f.f_lineno, co.co_name.encode(), msg )

def INFO( msg, name=None ):
    "INFO( msg, name=None )"
    f=sys._getframe(1); co=f.f_code
    if name == None: name=co.co_filename; name=name[name.rfind("/")+1:].encode()
    if type(name) != type(b''): name = name.encode()
    if type(msg) != type(b''): msg = msg.encode()
    if Instance: name = Instance+b':'+name
    libtrace.TRACE( name,TLVL_INFO,f.f_lineno, co.co_name.encode(), msg )

def LOG( msg, name=None ):
    "LOG( msg, name=None )"
    f=sys._getframe(1); co=f.f_code
    if name == None: name=co.co_filename; name=name[name.rfind("/")+1:].encode()
    if type(name) != type(b''): name = name.encode()
    if type(msg) != type(b''): msg = msg.encode()
    if Instance: name = Instance+b':'+name
    libtrace.TRACE( name,TLVL_LOG,f.f_lineno, co.co_name.encode(), msg )

def CNTL( cmd, *args ):
    """CNTL( cmd, *args )
example: TRACE.CNTL("printfd",5)"""
    f=sys._getframe(1); co=f.f_code; name=co.co_filename; name=name[name.rfind("/")+1:].encode()
    nargs=len(args)
    if type(cmd) != type(b''): cmd = cmd.encode()
    if Instance: name = Instance+b':'+name
    libtrace.TRACE_CNTL(name,name,nargs,cmd,*args)
