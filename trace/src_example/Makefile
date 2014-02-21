 # This file (Makefile<2>) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1.3 $$Date: 2014/02/21 15:38:28 $";


SUBDIRS=module userspace
unexport O          #

all: $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS) EXTRA_CFLAGS=-I${TRACE_INC} CFLAGS="-g -Wall -I${TRACE_INC}" CPPFLAGS=-I${TRACE_INC}

