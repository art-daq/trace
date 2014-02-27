#  This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Sep  8, 2004. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: Makefile,v $
#  $Revision: 1.4 $
#  $Date: 2014-02-27 17:15:06 $

# ref. work/cswing/source/Makefile
#      work/d0unix/Makefile
#      work2/bdPrj/designs/ron_acnet/src/makefile

# To override CFLAGS, use, for example:
#    make CFLAGS="-g -Wall -DCOUNTING"
# or make ${CFLAGS+CFLAGS="$CFLAGS"}    # to pickup CFLAGS from env
# or, to make 32bit exe on 64bit machine:
#    rm -f hello.o;CXX="g++ -m32" CC="gcc -m32" make -e hello
#
# To specify an external library:
#     LDLIBS=-lpthread make pthread_poll_socketpair
#  OR make pthread_poll_socketpair LDLIBS=-lpthread
# Another example:
#     make boost_thread_poll_socketpair CPPFLAGS=-I$BOOST_INC LDLIBS="-lboost_thread -L$BOOST_LIB"

# THESE CAN BE OVERRIDDEN FROM THE MAKE CMDLINE
#ifeq ($(shell uname -s),SunOS)
#        LDFLAGS   = -g -Wall
#else
#        LDFLAGS   = -g -Wall
#endif
CC       = gcc
CXX      = g++   # a variable named "C++" is not compatible with bourne shell
CFLAGS   = -g -Wall -DDO_THREADS
CXXFLAGS = $(CFLAGS)
CPPFLAGS = -I../include
LDFLAGS  = -lpthread  # -g not needed

# in case someone (SoftRelTools) sets -r (or `--no-builtin-rules') in MAKEFLAGS
# ref.  10.2 Catalogue of Implicit Rules
#       http://www.delorie.com/gnu/docs/make/make_101.html
#   and 10.7 Old-Fashioned Suffix Rules
#       http://www.delorie.com/gnu/docs/make/make_112.html
%.o : %.c        # cancel the built-in rule (give no recipe)
${OUT}%.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -g -c -o $@ $<
# just use the C++ linker
% : %.o
	@echo 'CXX="$(CXX)" LDFLAGS="$(LDFLAGS)" ^="$^"'
	@echo 'LOADLIBES="$(LOADLIBES)" LDLIBS="$(LDLIBS)" @="$@"'
	$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

${OUT}%.o : %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
${OUT}%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@; \
	sed 's,\($*\)\.o[ :]*,${OUT}\1.o $@ : ,g' < $@ > $@.$$$$; \
	mv -f $@.$$$$ $@
${OUT}%.d: %.cc
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# ref. "The Function wildcard"
#      http://www.delorie.com/gnu/docs/make/make_29.html
# ref. "Functions for String Substitution and Analysis"
#      http://www.delorie.com/gnu/docs/make/make_81.html
mainre       = '(^| |	)main\ *\('
cexesrc     := $(shell egrep -l $(mainre) *.c 2>/dev/null)
c++exesrc   := $(shell egrep -l $(mainre) *.cc 2>/dev/null)
executables := $(patsubst %.c,%,  $(cexesrc)) $(patsubst %.cc,%,  $(c++exesrc))
exeobjs     := $(patsubst %.c,%.o,$(cexesrc)) $(patsubst %.cc,%.o,$(c++exesrc))
objects     := $(patsubst %.c,%.o,$(filter-out $(cexesrc),$(wildcard *.c))) \
               $(patsubst %.cc,%.o,$(filter-out $(c++exesrc),$(wildcard *.cc)))
headers     := $(wildcard *.h)

out_executables := $(patsubst %,${OUT}%,$(executables))

#all: $(executables)
#all: $(patsubst %,${OUT}%,$(executables))
all: ${OUT}trace_cntl
	echo done with ${OUT}trace_cntl out_executables=$(out_executables)

${OUT}trace_cntl: ${OUT}trace_cntl.o

include ${OUT}$(cexesrc:.c=.d)
ifneq ($(c++exesrc),)
include ${OUT}$(c++exesrc:.cc=.d)
endif


# "$(executables): $(objects)" says that all executables depend on all objects.
# I probably just want the "% : %.o" rule above and the "include mk.dep" below
# to handle any further dependencies
# Note: "objects" can be controlled: make hello objects=hello.o
#$(executables): $(objects)
$(patsubst %,${OUT}%,$(executables)): $(patsubst %,${OUT}%,$(objects))

.PHONY: tags
tags:
	@echo "Remember emacs esc-. (Find tag)"
	find . -name '*.[ch]' -o -name '*.cc' | ctags -e -L -
#	find . -name '*.[ch]' | etags --ignore-indentation --no-defines -

clean:
	rm -f $(executables) $(exeobjs) $(objects) *~ mk.dep

variables:
	@echo cexesrc=$(cexesrc)
	@echo executables=$(executables)
	@echo objects=$(objects)

MKDEPCMD= $(CC) -MM $(CPPFLAGS) $(wildcard *.cc) $(wildcard *.c) >mk.dep
dep:
	@echo hi from $@
	$(MKDEPCMD)

# The include mk.dep needs to be last.
# Uncomment to remake dependencies every time a .c or .h file changes or
# leave commented out and remake dependencies by hand when you feel it is
# needed.
#mk.dep: $(wildcard *.cc) $(wildcard *.c) $(headers)
#	$(MKDEPCMD)
#include mk.dep
