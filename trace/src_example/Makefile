 # This file (Makefile<2>) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1.4 $$Date: 2014/03/03 05:34:19 $";


SUBDIRS=module userspace
unexport O          #
TRACE_INC = $$PWD/../include   # default _if_ make interactively called in this directory
EXTRA_SYMBOLS ?= KBUILD_EXTRA_SYMBOLS=$$PWD/../src_module/Module.symvers


all: $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS) EXTRA_CFLAGS=-I${TRACE_INC} \
	CFLAGS="-g -Wall -I${TRACE_INC}" CPPFLAGS=-I${TRACE_INC} \
	${EXTRA_SYMBOLS}

clean: $(SUBDIRS)
