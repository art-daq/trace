 # This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Oct  1, 2012. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1.5 $$Date: 2014/03/03 05:34:19 $";

#
# src_module/Makefile   -- NOTE: top level make file calls this make file
#                                with -e option to override some variables
#                                defined here.

# Ref. Documentation/kbuild/modules.txt
# Ref. http://www.tldp.org/LDP/lkmpg/2.6/html/  (2007-05-18 ver 2.6.4)
# Module files are specified in the Kbuild file.

# The default "KDIR" is based on the current running kernel (via uname -r)
# Specify another kernel via: make KDIR=/usr/local/src/linux-3.7.1

KDIR ?= /lib/modules/$(shell uname -r)/build
TRACE_INC = $$PWD/../include   # default _if_ make interactively called in this directory
EXTRA_CFLAGS=-I${TRACE_INC}

# to allow: make trace_proc.lst    # not working?
#           make trace_proc.o
#           make TRACE.ko
# Ref. Documentation/kbuild/modules.txt
%.o %.lst %.ko:
	$(MAKE) -C $(KDIR) M=$$PWD $@

all default:
	$(MAKE) -C $(KDIR) M=$$PWD EXTRA_CFLAGS=${EXTRA_CFLAGS}


# After testing with just "insmod TRACE.ko". Of course, this needs to be done
# as user root.
install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

clean help modules modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD $@
