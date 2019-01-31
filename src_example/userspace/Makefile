#  This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Sep  8, 2004. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: Makefile,v $
#  $Revision: 1030 $
#  $Date: 2019-01-02 17:05:09 -0600 (Wed, 02 Jan 2019) $

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
CFLAGS   = -g -Wall -DDO_THREADS -O2 $(XTRA_CFLAGS)
CXXFLAGS = -g -Wall -DDO_THREADS -O2 $(XTRA_CXXFLAGS)
CPPFLAGS = -I../../include
LDFLAGS  = -lpthread  # -g not needed

# in case someone (SoftRelTools) sets -r (or `--no-builtin-rules') in MAKEFLAGS
# ref.  10.2 Catalogue of Implicit Rules
#       http://www.delorie.com/gnu/docs/make/make_101.html
#   and 10.7 Old-Fashioned Suffix Rules
#       http://www.delorie.com/gnu/docs/make/make_112.html
% : %.c        # cancel the built-in rule (give no recipe)
${OUT}% : %.c  # cancel the built-in rule (give no recipe)
%.o : %.c        # cancel the built-in rule (give no recipe)
${OUT}%.o : %.c
	$(CC) $(CFLAGS) $(XTRA_CFLAGS) $(CPPFLAGS) -g -c -o $@ $<
${OUT}% : ${OUT}%.o
	@echo 'CXX="$(CXX)" LDFLAGS="$(LDFLAGS)" ^="$^"'
	@echo 'LOADLIBES="$(LOADLIBES)" LDLIBS="$(LDLIBS)" @="$@"'
	$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
% : %.cc        # cancel the built-in rule (give no recipe)
${OUT}% : %.cc
	$(CXX) $(CXXFLAGS) $(XTRA_CXXFLAGS) $(CPPFLAGS)    -o $@ $<
%.o : %.cc        # cancel the built-in rule (give no recipe)
${OUT}%.o : %.cc
	echo 'rule: ${OUT}%.o : %.cc'
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
${OUT}%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@; \
	sed 's,\($*\)\.o[ :]*,${OUT}\1.o $@ : ,g' < $@ > $@.$$$$; \
	mv -f $@.$$$$ $@
%.d : %.cc        # cancel the built-in rule (give no recipe)
${OUT}%.d: %.cc
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,${OUT}\1.o $@ : ,g' < $@.$$$$ > $@; \
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
out_deps        := $(patsubst %,${OUT}%.d,$(executables))


all: $(out_executables)
	echo done with ${OUT}just out_executables=$(out_executables)

${OUT}example_main: ${OUT}example_main.o ${OUT}example_sub1.o ${OUT}example_sub2.o ${OUT}example_sub3.o ${OUT}example_sub4.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< ${OUT}example_sub1.o ${OUT}example_sub2.o ${OUT}example_sub3.o ${OUT}example_sub4.o -lpthread

${OUT}just:         ${OUT}just.o         ${OUT}example_sub_.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< ${OUT}example_sub_.o

ifneq ($(executables),)
include ${out_deps}
endif
ifneq ($(objects),)
include $(objects:%.o=${OUT}%.d)
endif


# "$(executables): $(objects)" says that all executables depend on all objects.
# I probably just want the "% : %.o" rule above and the "include mk.dep" below
# to handle any further dependencies
# Note: "objects" can be controlled: make hello objects=hello.o
#$(executables): $(objects)
#$(patsubst %,${OUT}%,$(executables)): $(patsubst %,${OUT}%,$(objects))

.PHONY: tags
tags:
	@echo "Remember emacs esc-. (Find tag)"
	find . -name '*.[ch]' -o -name '*.cc' | ctags -e -L -
#	find . -name '*.[ch]' | etags --ignore-indentation --no-defines -

clean:
	rm -f $(executables) $(exeobjs) $(objects) *~ *.d mk.dep

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
