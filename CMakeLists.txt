# ======================================================================
#  TRACE main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/trace/ups/setup_for_development <-d|-p>
#  buildtool
#
# $Revision: 1530 $$Date: 2022-05-11 15:04:54 -0500 (Wed, 11 May 2022) $
# ======================================================================
cmake_minimum_required (VERSION 3.11)

if(LINUX AND (CMAKE_C_COMPILER OR CMAKE_CXX_COMPILER))
  message("WARNING: CMAKE_C_COMPILER and CMAKE_CXX_COMPILER must not be set: clearing!")
  unset(CMAKE_C_COMPILER)
  unset(CMAKE_CXX_COMPILER)
endif()

find_package(cetmodules 3.01.00 QUIET)
project(TRACE VERSION 3.17.06)
if (${cetmodules_FOUND})
  MESSAGE(STATUS "Found cetmodules.")
  set(CET_CETBUILDTOOLS_COMPAT TRUE)
  include(Compatibility)
  set(CET_WARN_DEPRECATED)
  set(CET_EXPORT EXPORT)
  include(CetCMakeEnv)
  cet_cmake_env(PAD_MINOR ON NO_BUILDTYPE UPS_PRODUCT_NAME TRACE)
else (${cetmodules_FOUND})
  MESSAGE(STATUS "Could not locate cetmodules.")
endif (${cetmodules_FOUND})

# Language standard requests.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11) # Optional only.

if ((CMAKE_CXX_COMPILER_ID STREQUAL Clang AND
     CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.0.0) OR
    (CMAKE_CXX_COMPILER_ID STREQUAL AppleClang AND
     CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0.0) OR
    (CMAKE_CXX_COMPILER_ID STREQUAL GNU AND
     CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.1.0))
  set(IF_FLAG -Wno-implicit-fallthrough)
else()
  set(IF_FLAG)
endif()

if (${cetmodules_FOUND})
  cet_set_compiler_flags(DIAGS VIGILANT WERROR EXTRA_FLAGS -O2 -pedantic -Wno-variadic-macros ${IF_FLAG}
                         EXTRA_C_FLAGS -D_GNU_SOURCE -DDO_THREADS)
endif (${cetmodules_FOUND})

add_definitions(-DDO_THREADS)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED ${CET_EXPORT})

include(GNUInstallDirs)     # Note comments below

if (${cetmodules_FOUND})
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

  find_package(messagefacility ${CET_EXPORT})
  set(mflib "")
  if(messagefacility_FOUND)
    set(mflib messagefacility::MF_MessageLogger)
  endif()

  cet_make_library(USE_PROJECT_NAME INTERFACE 
     SOURCE
    include/TRACE/trace.h
  )

  cet_make_library(LIBRARY_NAME MF INTERFACE
    SOURCE
    include/TRACE/tracemf.h
    include/TRACE/tracemf2.h
    include/TRACE/tracemf3.h
    LIBRARIES INTERFACE
    messagefacility::MF_MessageLogger
    TRACE::TRACE
  )
else (${cetmodules_FOUND})
  add_library(${PROJECT_NAME} INTERFACE )
  # add alias so the project can be used with add_subdirectory
  add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
endif (${cetmodules_FOUND})
# Adding the install interface generator expression makes sure that the include
# files are installed to the proper location (provided by GNUInstallDirs)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# We're going to install the source folders directly to make sure to have the correct Makefiles
# for building kernel modules after deployment.
FILE(COPY Makefile DESTINATION .)
INSTALL(FILES Makefile DESTINATION .)

# The trace utility
add_subdirectory(src_utility)
FILE(COPY src_utility DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_utility DESTINATION . PATTERN ".svn" EXCLUDE)

if (${cetmodules_FOUND})
  # Example Programs
  add_subdirectory(src_example)
  FILE(COPY src_example DESTINATION . PATTERN ".svn" EXCLUDE)
  INSTALL(DIRECTORY src_example DESTINATION . PATTERN ".svn" EXCLUDE)
endif (${cetmodules_FOUND})

# Kernel Module
add_subdirectory(src_module)
FILE(COPY src_module DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY src_module DESTINATION . PATTERN ".svn" EXCLUDE)

# The include directory (aka the core of TRACE)
FILE(COPY include DESTINATION . PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY include DESTINATION . PATTERN ".svn" EXCLUDE)

# scripts, including the trace_delta.pl script used for tdelta
add_subdirectory(script)

if (${cetmodules_FOUND})

  # doc - Some documentation...There should be a cet_ function for this...
  FILE(COPY doc DESTINATION . PATTERN ".svn" EXCLUDE)
  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ DESTINATION ${CMAKE_INSTALL_DOCDIR})

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/etc/compiler_info
    "Built TRACE utilities using ${CMAKE_C_COMPILER} and ${CMAKE_CXX_COMPILER}\n")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/compiler_info
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

  cet_cmake_config(CONFIG_FRAGMENTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TRACEConfig.cmake.in)

else  (${cetmodules_FOUND})

  # locations are provided by GNUInstallDirs
  install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)



  #---------------------------------------------------------------------------------------------

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

  configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

  install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

  install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} DESTINATION include)
endif (${cetmodules_FOUND})
