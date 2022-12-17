
if (${cetmodules_FOUND})
  cet_make_library(LIBRARY_NAME tracelib SOURCE tracelib.c LIBRARIES PRIVATE TRACE::TRACE)
else (${cetmodules_FOUND})
  add_library(tracelib SHARED tracelib.c)
  target_include_directories(tracelib PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>)
  install (TARGETS tracelib) # LIBRARY DESTINATION lib)
endif (${cetmodules_FOUND})
