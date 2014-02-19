 # This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Oct  1, 2012. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1.2 $$Date: 2014-02-19 17:43:37 $";

# Ref. Documentation/kbuild/modules.txt
# Ref. http://www.tldp.org/LDP/lkmpg/2.6/html/  (2007-05-18 ver 2.6.4)
# Module files are specified in the Kbuild file.

# The default "KDIR" is based on the current running kernel (via uname -r)
# Specify another kernel via: make KDIR=/usr/local/src/linux-3.7.1

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
PARENT_DIR := $(shell dirname $(PWD))

# to allow: make trace_proc.lst    # not working?
#           make trace_proc.o
#           make TRACE.ko
# Ref. Documentation/kbuild/modules.txt
%.o %.lst %.ko:
	$(MAKE) -C $(KDIR) M=$(PWD) $@

all default:
	$(MAKE) -C $(KDIR) M=$(PWD) KBUILD_EXTRA_SYMBOLS=${OUT}/module/Module.symvers

.PHONY: /
/:
	$(MAKE) -C $(KDIR) M=$(PWD) $@

# After testing with just "insmod TRACE.ko". Of course, this needs to be done
# as user root.
install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

clean help modules modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) $@
