if (CMAKE_C_COMPILER_ID STREQUAL GNU AND
    CMAKE_C_COMPILER_VERSION VERSION_LESS 4.8.0)
  set_property(SOURCE trace_cntl.c APPEND
               PROPERTY COMPILE_OPTIONS -Wno-strict-aliasing)
endif()
if (${cetmodules_FOUND})
  cet_make_exec(NAME trace_cntl SOURCE trace_cntl.c LIBRARIES PRIVATE TRACE::TRACE Threads::Threads)
else (${cetmodules_FOUND})
  add_executable(trace_cntl trace_cntl.c)
  target_link_libraries(trace_cntl TRACE::TRACE Threads::Threads)
  install(TARGETS trace_cntl)
endif (${cetmodules_FOUND})

#install_source()
