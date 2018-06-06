# ======================================================================
#  TRACE main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/trace/ups/setup_for_development <-d|-p>
#  buildtool
# ======================================================================

# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)

project(TRACE)
# cetbuildtools contains our cmake modules

if (IS_DIRECTORY $ENV{GCC_FQ_DIR} )

file(READ ${CMAKE_CURRENT_BINARY_DIR}/cetpkg_variable_report filedata)
#message("Read variable report: ${filedata}")
string(REGEX MATCHALL "(CETPKG_[^\n]*)" vars "${filedata}")
foreach(var ${vars})
    #message("Setting variables in line ${var}")
    string(REGEX REPLACE "(CETPKG_[^ \t]*)[ \t]*([^ ]*)" "\\1;\\2" thisvar "${var}")
    list(GET thisvar 0 varname)
    list(GET thisvar 1 varvalue)
    #message("set ${varname} to ${varvalue}")
    set(${varname} ${varvalue})
endforeach()

configure_file(ups/cetpkg_variable_report.in cetpkg_variable_report)
set(CMAKE_C_COMPILER "$ENV{GCC_FQ_DIR}/bin/gcc")
set(CMAKE_CXX_COMPILER "$ENV{GCC_FQ_DIR}/bin/g++")

endif()

set( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
if( NOT CETBUILDTOOLS_VERSION )
  message( FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules" )
endif()
set(CMAKE_MODULE_PATH $ENV{CETBUILDTOOLS_DIR}/Modules 
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT WERROR EXTRA_FLAGS -O2 -pedantic -Wno-variadic-macros
                       EXTRA_C_FLAGS -std=gnu11 -D_GNU_SOURCE -DDO_THREADS -pthread
                       EXTRA_CXX_FLAGS -std=c++0x)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDO_THREADS -pthread")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -lpthread")
set(CMAKE_CXX_FLAGS "-std=c++0x -O2 -Wno-variadic-macros")
cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( cetbuildtools v3_03_00 )

include_directories(include)

# We're going to install the source folders directly to make sure to have the correct Makefiles
# for building kernel modules after deployment.
FILE(COPY Makefile DESTINATION .)
INSTALL(FILES Makefile DESTINATION ${product}/${version})

# The trace utility
add_subdirectory(src_utility)
FILE(COPY src_utility DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_utility DESTINATION ${product}/${version} PATTERN ".svn" EXCLUDE)

# Example Programs
add_subdirectory(src_example)
FILE(COPY src_example DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_example DESTINATION ${product}/${version} PATTERN ".svn" EXCLUDE)

# Kernel Module
add_subdirectory(src_module)
FILE(COPY src_module DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_module DESTINATION ${product}/${version} PATTERN ".svn" EXCLUDE)

# The include directory (aka the core of TRACE)
FILE(COPY include DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY include DESTINATION ${product}/${version} PATTERN ".svn" EXCLUDE)

# scripts, including the trace_delta.pl script used for tdelta
add_subdirectory(script)

# ups - table and config files
add_subdirectory(ups)

# doc - Some documentation...There should be a cet_ function for this...
FILE(COPY README DESTINATION .)
INSTALL(FILES README DESTINATION ${product}/${version})
FILE(COPY doc DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY doc DESTINATION ${product}/${version} PATTERN ".svn" EXCLUDE)

# packaging utility - enable building a package tarball
include(UseCPack)
