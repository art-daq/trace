 # This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1.3 $$Date: 2014/02/21 02:08:58 $";

# TOP LEVEL Makefile

TRACE_DIR := $(shell pwd)
export TRACE_INC=${TRACE_DIR}/include


all: src_example src_module src_utility

.PHONY: src_example src_module src_utility
src_example: src_module
src_example: OUT_check
	subdir=`expr "$@" : 'src_\(.*\)'`;\
	test -d "${OUT}/$$subdir" || mkdir "${OUT}/$$subdir";\
	cp -ar $@/. "${OUT}/$$subdir/.";\
	$(MAKE) -e -C "${OUT}/$$subdir" CFLAGS=-I${TRACE_INC} CPPFLAGS=-I${TRACE_INC}

# don't seem to be able to build 32bit module on 64bit machine
src_module: OUT_check
	subdir=`expr "$@" : 'src_\(.*\)'`;\
	test -d "${OUT}/$$subdir" || mkdir "${OUT}/$$subdir";\
	cp -ar $@/. "${OUT}/$$subdir/.";\
	$(MAKE) -e -C "${OUT}/$$subdir";

src_utility: OUT_check
	subdir=`expr "$@" : 'src_\(.*\)'`;\
	test -d "${OUT}/$$subdir" || mkdir "${OUT}/$$subdir";\
	$(MAKE) -C $@ OUT="${OUT}/$$subdir/" CFLAGS=-I${TRACE_INC} CPPFLAGS=-I${TRACE_INC};\
	if [ `uname -m` = x86_64 ];then\
	    test -d "${OUT}/$${subdir}32" || mkdir "${OUT}/$${subdir}32";\
	    $(MAKE) -C $@ OUT="${OUT}/$${subdir}32/" LDFLAGS=-m32 CFLAGS="-m32 -I${TRACE_INC}" CPPFLAGS=-I${TRACE_INC};\
	fi

.PHONY: OUT_check
OUT_check:
	@echo "checking OUT"
	@if [ -z "${OUT}" -o ! -d "${OUT}" ];then echo "must specify OUT=<dir> and <dir> must exist"; exit 1;fi
	@echo "\$$OUT=${OUT}"
