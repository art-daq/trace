# ======================================================================
#  TRACE main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/trace/ups/setup_for_development <-d|-p>
#  buildtool
# ======================================================================
cmake_minimum_required (VERSION 3.11)

if(LINUX AND (CMAKE_C_COMPILER OR CMAKE_CXX_COMPILER))
  message("WARNING: CMAKE_C_COMPILER and CMAKE_CXX_COMPILER must not be set: clearing!")
  unset(CMAKE_C_COMPILER)
  unset(CMAKE_CXX_COMPILER)
endif()

project(TRACE VERSION 3.14.00)
# cetbuildtools contains our cmake modules

find_package(cetmodules 1.00.00 REQUIRED)
include(CetCMakeEnv)
cet_cmake_env(PAD_MINOR ON NO_BUILDTYPE UPS_PRODUCT_NAME TRACE)

cet_set_compiler_flags(DIAGS VIGILANT WERROR EXTRA_FLAGS -O2 -pedantic -Wno-variadic-macros -Wno-implicit-fallthrough
                       EXTRA_C_FLAGS -std=gnu11 -D_GNU_SOURCE -DDO_THREADS
                       EXTRA_CXX_FLAGS -std=c++0x)
add_definitions(-DDO_THREADS)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

cet_report_compiler_flags()

if(DEFINED ENV{USE_ASAN})
	if($ENV{USE_ASAN} GREATER 0)
		message("!!! Address Sanitizer Activated!!!")
		cet_add_compiler_flags(-fsanitize=address -lasan -Wno-unused-command-line-argument)
	endif()
endif()
if(DEFINED ENV{USE_TSAN})
	if($ENV{USE_TSAN} GREATER 0)
		message("!!! Thread Sanitizer Activated!!!")
		cet_add_compiler_flags(-fsanitize=thread -ltsan -Wno-unused-command-line-argument)
	endif()
endif()
if(DEFINED ENV{USE_UBSAN})
	if($ENV{USE_UBSAN} GREATER 0)
		message("!!! Undefined Sanitizer Activated!!!")
		cet_add_compiler_flags(-fsanitize=undefined -lubsan -Wno-unused-command-line-argument)
	endif()
endif()

# these are minimum required versions, not the actual product versions

include_directories(include)

# We're going to install the source folders directly to make sure to have the correct Makefiles
# for building kernel modules after deployment.
FILE(COPY Makefile DESTINATION .)
INSTALL(FILES Makefile DESTINATION .)

# The trace utility
add_subdirectory(src_utility)
FILE(COPY src_utility DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_utility DESTINATION . PATTERN ".svn" EXCLUDE)

# Example Programs
add_subdirectory(src_example)
FILE(COPY src_example DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_example DESTINATION . PATTERN ".svn" EXCLUDE)

# Kernel Module
add_subdirectory(src_module)
FILE(COPY src_module DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_module DESTINATION . PATTERN ".svn" EXCLUDE)

# The include directory (aka the core of TRACE)
FILE(COPY include DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY include DESTINATION . PATTERN ".svn" EXCLUDE)

# scripts, including the trace_delta.pl script used for tdelta
add_subdirectory(script)

# ups - table and config files
add_subdirectory(ups)

# doc - Some documentation...There should be a cet_ function for this...
FILE(COPY doc DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ DESTINATION ${CMAKE_INSTALL_DOCDIR})

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/etc/compiler_info
  "Built TRACE utilities using ${CMAKE_C_COMPILER} and ${CMAKE_CXX_COMPILER}\n")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/compiler_info
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
