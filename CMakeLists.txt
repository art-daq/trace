# ======================================================================
#  TRACE main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/trace/ups/setup_for_development <-d|-p>
#  buildtool
#
# ======================================================================
cmake_minimum_required (VERSION 3.11)

find_package(cetmodules 3.01.00)
project(TRACE VERSION 3.25.00)    # See https://github.com/art-daq/trace/wiki/Making-a-TRACE-release-package
if (${cetmodules_FOUND})
  MESSAGE(STATUS "Found cetmodules.")
  set(CET_EXPORT EXPORT)
  include(CetCMakeEnv)
  include(SetCompilerFlags)
  cet_cmake_env()
else (${cetmodules_FOUND})
  MESSAGE(STATUS "Could not locate cetmodules.")
endif (${cetmodules_FOUND})

# Language standard requests.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11) # Optional only.

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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
                         $<$<COMPILE_LANGUAGE:C>:-D_GNU_SOURCE -DDO_THREADS>)
endif (${cetmodules_FOUND})

add_definitions(-DDO_THREADS)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED ${CET_EXPORT})

include(GNUInstallDirs)     # Note comments below

# If a build tree previously used CMAKE_INSTALL_PREFIX=/, GNUInstallDirs cache
# entries can persist as usr/... and later become /usr/usr/... when the prefix
# is switched to /usr. Normalize those cached defaults for system installs.
if (CMAKE_INSTALL_PREFIX STREQUAL "/usr")
  foreach(_trace_dir_var
      BINDIR SBINDIR LIBEXECDIR SYSCONFDIR SHAREDSTATEDIR LOCALSTATEDIR
      RUNSTATEDIR LIBDIR INCLUDEDIR OLDINCLUDEDIR DATAROOTDIR DATADIR
      INFODIR LOCALEDIR MANDIR DOCDIR)
    if (DEFINED CMAKE_INSTALL_${_trace_dir_var}
        AND CMAKE_INSTALL_${_trace_dir_var} MATCHES "^usr/")
      string(REGEX REPLACE "^usr/" "" _trace_dir_norm
             "${CMAKE_INSTALL_${_trace_dir_var}}")
      set(CMAKE_INSTALL_${_trace_dir_var} "${_trace_dir_norm}" CACHE PATH
          "Install directory (${_trace_dir_var})" FORCE)
    endif()
  endforeach()
endif()

option(WANT_KMOD "Build TRACE kernel module" OFF)
option(TRACE_INSTALL_SOURCE_TREE "Install TRACE source/support trees alongside binaries" ON)
set(TRACE_SOURCE_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/TRACE" CACHE STRING "Destination for TRACE source/support trees when TRACE_INSTALL_SOURCE_TREE is ON")
if (IS_ABSOLUTE "${TRACE_SOURCE_INSTALL_DIR}")
  set(TRACE_SOURCE_INSTALL_FULL_DIR "${TRACE_SOURCE_INSTALL_DIR}")
elseif (CMAKE_INSTALL_PREFIX STREQUAL "/")
  set(TRACE_SOURCE_INSTALL_FULL_DIR "/${TRACE_SOURCE_INSTALL_DIR}")
else()
  set(TRACE_SOURCE_INSTALL_FULL_DIR "${CMAKE_INSTALL_PREFIX}/${TRACE_SOURCE_INSTALL_DIR}")
endif()
message(STATUS "TRACE install dirs (relative): prefix=${CMAKE_INSTALL_PREFIX}, bindir=${CMAKE_INSTALL_BINDIR}, includedir=${CMAKE_INSTALL_INCLUDEDIR}, datarootdir=${CMAKE_INSTALL_DATAROOTDIR}, sysconfdir=${CMAKE_INSTALL_SYSCONFDIR}")
message(STATUS "TRACE install dirs (effective): bindir=${CMAKE_INSTALL_FULL_BINDIR}, includedir=${CMAKE_INSTALL_FULL_INCLUDEDIR}, datarootdir=${CMAKE_INSTALL_FULL_DATAROOTDIR}, sysconfdir=${CMAKE_INSTALL_FULL_SYSCONFDIR}")
message(STATUS "TRACE source tree: install=${TRACE_INSTALL_SOURCE_TREE}, dir=${TRACE_SOURCE_INSTALL_DIR}, effective=${TRACE_SOURCE_INSTALL_FULL_DIR}")

if (${cetmodules_FOUND})
  #cet_report_compiler_flags()

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
	if(DEFINED ENV{USE_GCOV})
		add_compile_options(-fprofile-arcs -ftest-coverage --coverage)
		add_link_options(--coverage)
	endif()

  cet_make_library(USE_PROJECT_NAME INTERFACE
	 SOURCE
	include/TRACE/trace.h
  )

  find_package(messagefacility ${CET_EXPORT} QUIET)
  set(mflib "")
  if(messagefacility_FOUND)
	  cet_make_library(LIBRARY_NAME MF INTERFACE
	    SOURCE
	    include/TRACE/tracemf.h
	    include/TRACE/tracemf2.h
	    include/TRACE/tracemf3.h
	    LIBRARIES INTERFACE
	    messagefacility::MF_MessageLogger
	    TRACE::TRACE
	  )
  else()
    if(${WANT_MF})
        message(FATAL_ERROR "MessageFacility not found and MF build requested! Aborting...")
    endif()
  endif()

else (${cetmodules_FOUND})

  if(${WANT_MF})
      message(FATAL_ERROR "MessageFacility build requested, requires CETModules! Aborting...")
  endif()
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
if (TRACE_INSTALL_SOURCE_TREE)
  install(FILES Makefile DESTINATION ${TRACE_SOURCE_INSTALL_DIR})
endif()

# The trace utility
add_subdirectory(src_utility)
FILE(COPY src_utility DESTINATION . PATTERN ".svn" EXCLUDE)
if (TRACE_INSTALL_SOURCE_TREE)
  install(DIRECTORY src_utility DESTINATION ${TRACE_SOURCE_INSTALL_DIR} PATTERN ".svn" EXCLUDE)
endif()

# Example Programs
add_subdirectory(src_example)
FILE(COPY src_example DESTINATION . PATTERN ".svn" EXCLUDE)
if (TRACE_INSTALL_SOURCE_TREE)
  install(DIRECTORY src_example DESTINATION ${TRACE_SOURCE_INSTALL_DIR} PATTERN ".svn" EXCLUDE)
endif()

# Kernel Module
add_subdirectory(src_module)
FILE(COPY src_module DESTINATION . PATTERN ".svn" EXCLUDE)
if (TRACE_INSTALL_SOURCE_TREE)
  install(DIRECTORY src_module DESTINATION ${TRACE_SOURCE_INSTALL_DIR} PATTERN ".svn" EXCLUDE)
endif()

# The include directory (aka the core of TRACE)
FILE(COPY include DESTINATION . PATTERN ".svn" EXCLUDE)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} PATTERN ".svn" EXCLUDE)

# The python directory (aka the core of TRACE)
add_subdirectory(python)
FILE(COPY python DESTINATION . PATTERN ".svn" EXCLUDE)
if (TRACE_INSTALL_SOURCE_TREE)
  install(DIRECTORY python DESTINATION ${TRACE_SOURCE_INSTALL_DIR} PATTERN ".svn" EXCLUDE)
endif()

# Backward compatibility: for non-system prefixes, expose <prefix>/python in
# addition to the canonical source-tree location.
if (TRACE_INSTALL_SOURCE_TREE
    AND NOT CMAKE_INSTALL_PREFIX STREQUAL "/"
    AND NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
  set(_trace_python_compat_target "${TRACE_SOURCE_INSTALL_DIR}/python")
  install(CODE
"
set(_trace_py_compat \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/python\")
set(_trace_py_target \"${_trace_python_compat_target}\")
if (EXISTS \"\${_trace_py_compat}\" AND NOT IS_SYMLINK \"\${_trace_py_compat}\")
  message(STATUS \"TRACE: leaving existing non-symlink python compat path in place: \${_trace_py_compat}\")
else()
  if (IS_SYMLINK \"\${_trace_py_compat}\")
    file(REMOVE \"\${_trace_py_compat}\")
  endif()
  file(MAKE_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\")
  execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"\${_trace_py_target}\" \"\${_trace_py_compat}\" RESULT_VARIABLE _trace_py_link_rc)
  if (NOT _trace_py_link_rc EQUAL 0)
    message(WARNING \"TRACE: failed to create python compat symlink: \${_trace_py_compat} (rc=\${_trace_py_link_rc})\")
  endif()
endif()
")
endif()

# scripts, including the trace_delta script used for tdelta
add_subdirectory(script)

# doc - Some documentation...There should be a cet_ function for this...
add_subdirectory(doc)

if (${cetmodules_FOUND})

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/etc/compiler_info
    "Built TRACE utilities using ${CMAKE_C_COMPILER} and ${CMAKE_CXX_COMPILER}\n")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/compiler_info
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR})

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
