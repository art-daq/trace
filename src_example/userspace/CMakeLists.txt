
if ("cxx_lambdas" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  cet_make_exec(example_main SOURCE
		example_main.cc
		example_sub1.cc
		example_sub2.cc
		example_sub3.cc
		example_sub4.cc
		LIBRARIES
    PRIVATE
    TRACE::TRACE
    Threads::Threads
    )

  cet_make_exec(no_std SOURCE no_std.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(just SOURCE just.cc example_sub_.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(just_user SOURCE just_user.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(just_recursive SOURCE just_recursive.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(TLOG_manipulators SOURCE TLOG_manipulators.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(ex_traceln SOURCE ex_traceln.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(ptr_address_string SOURCE ptr_address_string.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(trace_lvl SOURCE trace_lvl.cc LIBRARIES PRIVATE TRACE::TRACE)
  cet_make_exec(tlog-compare SOURCE tlog-compare.cc LIBRARIES PRIVATE TRACE::TRACE)
  #cet_make_exec(setting_lclTime SOURCE setting_lclTime.cc LIBRARIES PRIVATE TRACE::TRACE) # this one is problematic for some compilers
  cet_make_exec(TLOG SOURCE TLOG.cc LIBRARIES PRIVATE TRACE::TRACE)
else()
  message(WARNING "C++ userspace examples not built due to lack of compiler support for CXX lambdas.")
endif()
cet_make_exec(basic_c SOURCE basic_c.c LIBRARIES PRIVATE TRACE::TRACE)
cet_make_exec(no_trace SOURCE no_trace.cc)

#install_source() # handled elsewhere
