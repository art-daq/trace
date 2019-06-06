if (CMAKE_C_COMPILER_ID STREQUAL GNU AND
    CMAKE_C_COMPILER_VERSION VERSION_LESS 4.8.0)
  set_property(SOURCE trace_cntl.c APPEND
               PROPERTY COMPILE_OPTIONS -Wno-strict-aliasing)
endif()
cet_make_exec(trace_cntl SOURCE trace_cntl.c LIBRARIES Threads::Threads)

#install_source()
