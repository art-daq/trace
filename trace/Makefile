 # This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1.20 $$Date: 2015/04/27 03:26:19 $";

# TOP LEVEL Makefile

HELP=\
echo "OUT=<dir> must be specified and <dir> must exist.";\
echo "Examples:";\
echo "  make OUT=\$$PWD -j4     \# the default is to build the 2 userspace programs";\
echo "  make OUT=\$$PWD -j4 all \# build userspace programs and modules";\
echo "  make OUT=\$$PWD src_module";\
echo "  make OUT=\$$PWD modules";\
echo "  make OUT=\$$PWD modules KDIR=<path_to_built_kernel>";\
echo "  make OUT=\$$PWD XTRA_CXXFLAGS=-std=c++11 src_example_user";\
echo "If \"ups\" is available and OUT is set to \$$PWD, you should be able to:";\
echo "  setup -r\$$PWD -z\$$PWD TRACE";\
echo "to get env.var. TRACE_INC set and env.var. PATH adjusted."

TRACE_DIR := $(shell pwd)
export TRACE_INC=${TRACE_DIR}/include
FLAVOR_SUBDIR = os=`uname -s`;\
        mach=`uname -m`;\
	b64=`sh -c "case $$mach in x86_64|sun*) echo 64bit;;*) :;;esac"`;\
	mach=`sh -c "case $$mach in *86*|sun*) :;; *) echo $mach;;esac"`;\
	os_rev_fields=`sh -c "case $$os in Darwin) echo 1;; *) echo 2;;esac"`;\
	os_rev1=`uname -r |cut -d. -f1-$$os_rev_fields`;\
	libc1=`/bin/ls /lib*/libc-* /lib/*/libc-* 2>/dev/null|sed '{s/.*libc-//;s/[^0-9]*$$//;q;}'`;\
	echo os=$$os mach=$$mach b64=$$b64 os_rev1=$$os_rev1 libc1=$$libc1

CPPFLAGS += $(shell test -f /lib/libc-2.5.so -o -f /lib64/libc-2.5.so && echo -DNO_SCHED_GETCPU)


default: src_utility src_example_user script ups
	@${FLAVOR_SUBDIR};\
	echo add to PATH: PATH=${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}/bin:${OUT}/script:\$$PATH
	@echo Possibly: setup -r\$$PWD -z\$$PWD TRACE

.PHONY: src_utility src_module src_example_user src_example_module script ups module default

modules: src_module src_example_module
all: default modules
	@echo Done with $@



src_example_module: src_module      # the example module depends on symbols from the TRACE module

src_module: OUT_check
	test -n "${KDIR}"\
	  && subdir=module/`cat ${KDIR}/include/config/kernel.release`\
	  || subdir=module/`uname -r`;\
	test -d "${OUT}/$$subdir" || mkdir -p "${OUT}/$$subdir";\
	cp -ar $@/*.[ch] $@/[MK]* "${OUT}/$$subdir/.";\
	$(MAKE) -e -C "${OUT}/$$subdir";

# have to do this in two parts:
#    1) userspace - src can remain
#    2) module (which needs src copied)
src_example_user: OUT_check src_lib
	${FLAVOR_SUBDIR};\
	out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}";\
	test -d "$$out/bin/" || mkdir -p "$$out/bin/";\
	$(MAKE) -C src_example userspace OUT=$$out/bin/ CPPFLAGS="${CPPFLAGS}" TRACE_INC=$$PWD/include LIBDIR=$$out/lib/;\
	if [ `uname -m` = x86_64 -a -z '${NO32}' ];then\
	    out="${OUT}/$$os$$mach+$$os_rev1$${libc1:+-$$libc1}";\
	    test -d "$$out/bin/" || mkdir -p "$$out/bin/";\
	    $(MAKE) -C src_example userspace OUT=$$out/bin/ CPPFLAGS="-m32 ${CPPFLAGS}" TRACE_INC=$$PWD/include LDFLAGS=-m32 LIBDIR=$$out/lib/;\
	fi
src_example_module: OUT_check
	test -n "${KDIR}"\
	  && modsubdir=module/`cat ${KDIR}/include/config/kernel.release`\
	  || modsubdir=module/`uname -r`;\
	cp -a src_example/module*/*.c "${OUT}/$$modsubdir/.";\
	for mod in `grep -ho ^.*_mod-y src_example/module*/Kbuild | sed 's/-y//'`;do \
	  grep $${mod}ule.o "${OUT}/$$modsubdir/Kbuild" || \
	  sed -i -e '/TRACE.o/s/$$/ '$$mod'.o/;$$a\
	'$$mod'-y := '$$mod'ule.o' "${OUT}/$$modsubdir/Kbuild";\
	done;\
	$(MAKE) -e -C "${OUT}/$$modsubdir" TRACE_INC=$$PWD/include

src_utility: OUT_check
	${FLAVOR_SUBDIR};\
	out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}/bin/";\
	test -d "$$out" || mkdir -p "$$out";\
	$(MAKE) -C $@ OUT="$$out" CPPFLAGS="-I${TRACE_INC} ${CPPFLAGS}";\
	if [ `uname -m` = x86_64 -a -z '${NO32}' ];then\
	    out="${OUT}/$$os$$mach+$$os_rev1$${libc1:+-$$libc1}/bin/";\
	    test -d "$$out" || mkdir -p "$$out";\
	    $(MAKE) -C $@ OUT="$$out" LDFLAGS="-m32 -lpthread" CPPFLAGS="-m32 -I${TRACE_INC} ${CPPFLAGS}";\
	fi

src_lib: OUT_check
	${FLAVOR_SUBDIR};\
	out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}/lib/";\
	test -d "$$out" || mkdir -p "$$out";\
	$(MAKE) -C $@ OUT="$$out" CPPFLAGS="-I${TRACE_INC} ${CPPFLAGS}";\
	if [ `uname -m` = x86_64 -a -z '${NO32}' ];then\
	    out="${OUT}/$$os$$mach+$$os_rev1$${libc1:+-$$libc1}/lib/";\
	    test -d "$$out" || mkdir -p "$$out";\
	    $(MAKE) -C $@ OUT="$$out" LDFLAGS="-m32 -lpthread" CPPFLAGS="-m32 -I${TRACE_INC} ${CPPFLAGS}";\
	fi

script ups: OUT_check
	test -d "${OUT}/$@" || mkdir "${OUT}/$@"
	-cp -au $@/[a-zABD-Z]*[^~] "${OUT}/$@"   # filter out CVS dir and backups files

help:
	@$(HELP)

.PHONY: OUT_check
OUT_check:
	@echo "checking OUT..."
	@if [ -z "${OUT}" -o ! -d "${OUT}" ];then $(HELP); exit 1;fi
	@echo "\$$OUT=${OUT}"
