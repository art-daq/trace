 # This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1.8 $$Date: 2014/03/04 15:55:45 $";

# TOP LEVEL Makefile

TRACE_DIR := $(shell pwd)
export TRACE_INC=${TRACE_DIR}/include


all: src_utility src_module src_example

.PHONY: src_utility src_module src_example
src_example: src_module

# don't seem to be able to build 32bit module on 64bit machine
src_module: OUT_check
	subdir=module/`uname -r`;\
	test -d "${OUT}/$$subdir" || mkdir -p "${OUT}/$$subdir";\
	cp -ar $@/. "${OUT}/$$subdir/.";\
	$(MAKE) -e -C "${OUT}/$$subdir";

FLAVOR_SUBDIR = os=`uname -s`;\
        mach=`uname -m`;\
	b64=`sh -c "case $$mach in x86_64|sun*) echo 64bit;;*) :;;esac"`;\
	mach=`sh -c "case $$mach in *86*|sun*) :;; *) echo $mach;;esac"`;\
	os_rev1=`uname -r |cut -d. -f1-2`;\
	libc1=-`/bin/ls /lib*/libc-* /lib/*/libc-* 2>/dev/null|sed '{s/.*libc-//;s/[^0-9]*$$//;q}'`;\
	echo os=$$os mach=$$mach b64=$$b64 os_rev1=$$os_rev1 libc1=$$libc1

# have to do this in two parts:
#    1) module (which needs src copied)
#    2) userspace - src can remain
src_example: OUT_check
	${FLAVOR_SUBDIR};\
	out="${OUT}/$$os$$mach$$b64+$$os_rev1$$libc1/bin/";\
	test -d "$$out" || mkdir -p "$$out";\
	$(MAKE) -C src_example userspace OUT=$$out TRACE_INC=$$PWD/include;\
	if [ `uname -m` = x86_64 ];then\
	    out="${OUT}/$$os$$mach+$$os_rev1$$libc1/bin/";\
	    test -d "$$out" || mkdir -p "$$out";\
	    $(MAKE) -C src_example userspace OUT=$$out CPPFLAGS="-m32 -I$$PWD/include" LDFLAGS=-m32;\
	fi
	modsubdir=module/`uname -r`;\
	cp -ar $@/module/*.c "${OUT}/$$modsubdir/.";\
	grep some_module.o "${OUT}/$$modsubdir/Kbuild" || \
	sed -i -e '/TRACE.o$$/s/$$/ some_mod.o/;$$a\
	some_mod-y := some_module.o' "${OUT}/$$modsubdir/Kbuild";\
	$(MAKE) -e -C "${OUT}/$$modsubdir" TRACE_INC=$$PWD/include

src_utility: OUT_check
	${FLAVOR_SUBDIR};\
	out="${OUT}/$$os$$mach$$b64+$$os_rev1$$libc1/bin/";\
	test -d "$$out" || mkdir -p "$$out";\
	$(MAKE) -C $@ OUT="$$out" CPPFLAGS=-I${TRACE_INC};\
	if [ `uname -m` = x86_64 ];then\
	    out="${OUT}/$$os$$mach+$$os_rev1$$libc1/bin/";\
	    test -d "$$out" || mkdir -p "$$out";\
	    $(MAKE) -C $@ OUT="$$out" LDFLAGS="-m32 -lpthread" CPPFLAGS="-m32 -I${TRACE_INC}";\
	fi


HELP=\
must specify OUT=<dir> and <dir> must exist.\n\
Examples: make OUT=\$$PWD


.PHONY: OUT_check
OUT_check:
	@echo "checking OUT"
	@if [ -z "${OUT}" -o ! -d "${OUT}" ];then echo -e "$(HELP)"; exit 1;fi
	@echo "\$$OUT=${OUT}"
