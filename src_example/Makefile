 # This file (Makefile<2>) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
<<<<<<< HEAD
 # rev="$Revision$$Date$";
=======
 # rev="$Revision: 1.6 $$Date: 2014/03/05 23:05:00 $";
>>>>>>> b06df97e1064bf6d8b0e7c0cfd42e2343a2521a5


SUBDIRS=module userspace
unexport O          # if I am/were to use "O=..." -- it would conflict with Kbuild
TRACE_INC = $$PWD/../include   # default _if_ make interactively called in this directory
EXTRA_SYMBOLS ?= KBUILD_EXTRA_SYMBOLS=$$PWD/../src_module/Module.symvers

all: $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	echo 'OUT=${OUT}'
	$(MAKE) -C $@ EXTRA_CFLAGS=-I${TRACE_INC} \
	CFLAGS="-g -Wall -I${TRACE_INC}" CPPFLAGS="-I${TRACE_INC} ${CPPFLAGS}" \
	${EXTRA_SYMBOLS}

clean: $(SUBDIRS)
