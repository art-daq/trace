/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Mar 28, 2020. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: trace.c,v $
	rev="$Revision: 1.3 $$Date: 2020/03/30 17:38:21 $";

gcc -c trace.c

Use with trace-addr2line.
Example (with simple exe program "one_string_on_function_call.cc"):

g++ -g -O0 -fno-inline -Wall -I. -std=c++11 -Wextra -pedantic -D_GLIBCXX_DEBUG \
 -finstrument-functions\
 -finstrument-functions-exclude-file-list=move,ptr_traits,stl_iterator,basic_string.tcc \
 -c one_string_on_function_call.cc && \
strace -fot.strace g++ -g trace.o -o one_string_on_function_call{,.o}
treset;tmodeM 1;tonTg 0-63;tcntl trig 9990; ./one_string_on_function_call >/dev/null; tinfo|egrep 'entri|full|mode'

tshow|tac|trace-addr2line ./one_string_on_function_call -f '_[MS]_|_Alloc|::(length|copy|assign|__is_null)'|tdelta -d 1 -i

*/

#include "TRACE/trace.h"
#define TRACE_NAME "cyg_profile"

void __cyg_profile_func_enter (void *func,  void *caller) __attribute__((no_instrument_function));
void __cyg_profile_func_enter (void *func,  void *caller)
{
	TRACE(3, "e %p %p", func, caller);
}

void __cyg_profile_func_exit (void *func, void *caller) __attribute__((no_instrument_function));
void __cyg_profile_func_exit (void *func, void *caller)
{
	TRACE(3, "x %p %p", func, caller);
}
