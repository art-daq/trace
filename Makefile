 # This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb 18, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: Makefile,v $
 # rev="$Revision: 1585 $$Date: 2023-01-27 16:27:33 -0600 (Fri, 27 Jan 2023) $";

# TOP LEVEL Makefile

HELP=\
echo "OUT=<dir> must be specified and <dir> must exist.";\
echo "Examples:";\
echo "  make <rpm|srpm>         \# actually makes 2 rpms - TRACE-utils and kmod-TRACE - placed under ~/rpmbuild/{,S}RPMS";\
echo "  make OUT=\$$PWD -j4     \# the default is to build the 2 userspace programs";\
echo "  make OUT=\$$PWD -j4 all \# build userspace programs and modules";\
echo "  make OUT=\$$PWD src_utility src_example_user modules 2>&1 | egrep -iB4 'error|warn'";\
echo "  make OUT=\$$PWD KDIR=/home/ron/work/WireCapPrj/linux-3.16.1 src_utility src_example_user modules 2>&1 | egrep -iB4 'error|warn'";\
echo "  make OUT=\$$PWD 32ON64=1 XTRA_CXXFLAGS=-std=c++11 XTRA_CFLAGS=-std=c11";\
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

all: default modules
	@echo Done with $@

clean:                     # trying clean w/o OUT_check
	@${FLAVOR_SUBDIR};\
	test -n "${TRACE_FQ_DIR}"\
	 && out="${TRACE_FQ_DIR}"\
	 || out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}";\
	set -x; rm -fr $$out module/`uname -r` big_ex.d src_module/.tmp_versions
	rm -f src_example/userspace/*.d src_module/{.*.cmd,*.symvers,*.ko,*.mod.c,*.order}
	rm -f make.out
	rm -f rpm/TRACE.tar.bz2

rpm_source:
	@rm -f rpm/TRACE.tar.bz2
	@test -d .svn && echo "Warning: .svn dir present. Make sure files are checked in." || true
	@tar cf - --exclude=TRACE.tar.bz2 --exclude=.vs --exclude=*.json --exclude=.gdb_history --exclude=*~ \
	* | bzip2 > rpm/TRACE.tar.bz2
srpm: rpm_source
	@: TRACE_FQ_DIR is cleared in spec file;\
	rpmbuild -bs --define "_sourcedir ${PWD}/rpm" rpm/TRACE.spec
	@rm -f rpm/TRACE.tar.bz2
	@echo Output srpms should be under ~/rpmbuild/SRPMS
rpm: rpm_source
	@: TRACE_FQ_DIR is cleared in spec file;\
	rpmbuild -bb --quiet --define "_sourcedir ${PWD}/rpm" rpm/TRACE.spec
	@rm -f rpm/TRACE.tar.bz2
	@echo Output srpms should be under ~/rpmbuild/RPMS

modules: src_module src_example_module

src_module: OUT_check
	test -n "${KDIR}"\
	  && subdir=module/`cat ${KDIR}/include/config/kernel.release`\
	  || subdir=module/`uname -r`;\
	test -d "${OUT}/$$subdir" || mkdir -p "${OUT}/$$subdir";\
	cp -a $@/*.[ch] $@/[MK]* "${OUT}/$$subdir/.";\
	$(MAKE) -C "${OUT}/$$subdir" TRACE_INC=$${TRACE_INC-$$PWD/include};

src_example_module: OUT_check src_module # the example module depends on symbols from the TRACE module
	test -n "${KDIR}"\
	  && modsubdir=module/`cat ${KDIR}/include/config/kernel.release`\
	  || modsubdir=module/`uname -r`;\
	cp -a src_example/module*/*.c "${OUT}/$$modsubdir/.";\
	for mod in `grep -ho ^.*_mod-y src_example/module*/Kbuild | sed 's/-y//'`;do \
	  grep $${mod}ule.o "${OUT}/$$modsubdir/Kbuild" || \
	  sed -i -e '/TRACE.o/s/$$/ '$$mod'.o/;$$a\
	'$$mod'-y := '$$mod'ule.o' "${OUT}/$$modsubdir/Kbuild";\
	done;\
	$(MAKE) -C "${OUT}/$$modsubdir" TRACE_INC=$${TRACE_INC-$$PWD/include}

# have to do this in two parts:
#    1) userspace - src can remain
#    2) module (which needs src copied)
src_example_user: OUT_check
	@${FLAVOR_SUBDIR};\
	test -n "${TRACE_FQ_DIR}"\
	 && out="${TRACE_FQ_DIR}"\
	 || out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}";\
	test -d "$$out/bin/" || mkdir -p "$$out/bin/";\
	$(MAKE) -C src_example userspace OUT="$$out/bin/" CPPFLAGS="${CPPFLAGS}" TRACE_INC=$$PWD/include;\
	sts=$$?;\
	if [ $$sts -eq 0 -a `uname -m` = x86_64 -a '${32ON64}' = 1 ];then\
	    out=`echo "$$out" | sed 's/64bit//'`;\
	    test -d "$$out/bin/" || mkdir -p "$$out/bin/";\
	    $(MAKE) -C src_example userspace OUT="$$out/bin/" CPPFLAGS="-m32 ${CPPFLAGS}" TRACE_INC=$$PWD/include LDFLAGS="-m32  -lpthread";\
	else\
	    exit $$sts;\
	fi

src_utility: OUT_check
	@${FLAVOR_SUBDIR};\
	test -n "${TRACE_FQ_DIR}"\
	 && out="${TRACE_FQ_DIR}/bin/"\
	 || out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}/bin/";\
	test -d "$$out" || mkdir -p "$$out";\
	$(MAKE) -C $@ OUT="$$out" CPPFLAGS="-I${TRACE_INC} ${CPPFLAGS}";\
	if [ `uname -m` = x86_64 -a '${32ON64}' = 1 ];then\
	    out=`echo "$$out" | sed 's/64bit//'`;\
	    test -d "$$out" || mkdir -p "$$out";\
	    $(MAKE) -C $@ OUT="$$out" LDFLAGS="-m32 -lpthread" CPPFLAGS="-m32 -I${TRACE_INC} ${CPPFLAGS}";\
	fi

cmake: OUT_check
	@${FLAVOR_SUBDIR};\
	test -n "${TRACE_FQ_DIR}"\
	 && out="${TRACE_FQ_DIR}/lib/TRACE/cmake/"\
	 || out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}/lib/TRACE/cmake";\
	test -d "$$out" || mkdir -p "$$out";\
	sed -e '/^[^#]/{s|@PACKAGE_INIT@|get_filename_component(PACKAGE_PREFIX_DIR "$${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)\
	macro(check_required_components _NAME)\
	  foreach(comp $${$${_NAME}_FIND_COMPONENTS})\
	    if(NOT $${_NAME}_$${comp}_FOUND)\
	      if($${_NAME}_FIND_REQUIRED_$${comp})\
	        set($${_NAME}_FOUND FALSE)\
	      endif()\
	    endif()\
	  endforeach()\
	endmacro()\
	|;s/@PROJECT_NAME@/TRACE/;}' cmake/TRACEConfig.cmake.in >"$$out/TRACEConfig.cmake";\
	_ver=`sed -n '/TRACE *VERSION/{s/.*VERSION *//;s/)//;p}' CMakeLists.txt`;\
	sed -e "s/@PACKAGE_VERSION@/$$_ver/" cmake/TRACEConfigVersion.cmake.in >"$$out/TRACEConfigVersion.cmake"

script: OUT_check
	@${FLAVOR_SUBDIR};\
	test -n "${TRACE_FQ_DIR}"\
	 && out="${TRACE_FQ_DIR}/bin/"\
	 || out="${OUT}/$$os$$mach$$b64+$$os_rev1$${libc1:+-$$libc1}/bin/";\
	test -d "$$out" || mkdir -p "$$out";\
	$(MAKE) -C $@ OUT="$$out"

${OUT}/ups/TRACE.table: ${CURDIR}/ups/TRACE.table.in
	test -d "${OUT}/ups" || mkdir "${OUT}/ups"
	@${FLAVOR_SUBDIR};\
	ups_prod_ver=`sed -n '/TRACE *VERSION/{s/.*VERSION */v/;s/).*//;s/\./_/g;p}' CMakeLists.txt`;\
	sed -e "s/@UPS_PRODUCT_NAME@/TRACE/;\
	s/@UPS_PROUDCT_VERSION@/$$ups_prod_ver/;\
	s/@UPS_QUALIFIER_STRING@//;\
	s|@CMAKE_INSTALL_DOCDIR@|doc|;\
	s/@UPS_PRODUCT_FQ@/\`ups flavor -4\`/" $< >"$@" || sh -c "echo problem makeing \"$@\" - removing; rm -f \"$@\""

ups: OUT_check ${OUT}/ups/TRACE.table
	@echo done with ups

help:
	@$(HELP)

.PHONY: OUT_check
OUT_check:
	@echo "checking OUT..."
	@if [ -z "${OUT}" -o ! -d "${OUT}" ];then $(HELP); exit 1;fi
	@echo "\$$OUT=${OUT}"
