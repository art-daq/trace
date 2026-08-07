
if ("cxx_lambdas" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    add_executable(ptr_address_string ptr_address_string.cc)
    target_link_libraries(ptr_address_string PRIVATE TRACE::TRACE)
else()
    message(WARNING "Some C++ userspace examples not built due to lack of compiler support for CXX lambdas.")
endif()

add_executable(example_main
		example_main.cc
		example_sub1.cc
		example_sub2.cc
		example_sub3.cc
		example_sub4.cc
		)
target_link_libraries(example_main PRIVATE TRACE::TRACE Threads::Threads)

add_executable(no_std no_std.cc)
target_link_libraries(no_std PRIVATE TRACE::TRACE)

add_executable(just just.cc example_sub_.cc)
target_link_libraries(just PRIVATE TRACE::TRACE)

add_executable(just_user just_user.cc)
target_link_libraries(just_user PRIVATE TRACE::TRACE)

add_executable(just_recursive just_recursive.cc)
target_link_libraries(just_recursive PRIVATE TRACE::TRACE)

add_executable(TLOG_manipulators TLOG_manipulators.cc)
target_link_libraries(TLOG_manipulators PRIVATE TRACE::TRACE)

add_executable(TLOG_ENTEX TLOG_ENTEX.cc)
target_link_libraries(TLOG_ENTEX PRIVATE TRACE::TRACE)

add_executable(ex_traceln ex_traceln.cc)
target_link_libraries(ex_traceln PRIVATE TRACE::TRACE)

add_executable(trace_lvl trace_lvl.cc)
target_link_libraries(trace_lvl PRIVATE TRACE::TRACE)

add_executable(tlog-compare tlog-compare.cc)
target_link_libraries(tlog-compare PRIVATE TRACE::TRACE)
install(TARGETS tlog-compare)

#add_executable(setting_lclTime setting_lclTime.cc) # this one is problematic for some compilers
#target_link_libraries(setting_lclTime PRIVATE TRACE::TRACE)

add_executable(TLOG TLOG.cc)
target_link_libraries(TLOG PRIVATE TRACE::TRACE)

add_executable(basic_c basic_c.c)
target_link_libraries(basic_c PRIVATE TRACE::TRACE)

add_executable(no_trace no_trace.cc)

#install_source() # handled elsewhere
