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
set( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
if( NOT CETBUILDTOOLS_VERSION )
  message( FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules" )
endif()
set(CMAKE_MODULE_PATH $ENV{CETBUILDTOOLS_DIR}/Modules 
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT WERROR EXTRA_FLAGS -O2 -pedantic -Wno-variadic-macros
                       EXTRA_C_FLAGS -std=gnu11)
cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( cetbuildtools v3_03_00 )

include_directories(include)

# The trace library
add_subdirectory(src_lib)

# The trace utility
add_subdirectory(src_utility)

# Example Programs
add_subdirectory(src_example)

# Kernel Module
add_subdirectory(src_module)

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
