
if (${cetmodules_FOUND})
  cet_make_library(LIBRARY_NAME tracelib SOURCE tracelib.c LIBRARIES PRIVATE TRACE::TRACE)
else (${cetmodules_FOUND})
  add_library(tracelib SOURCE tracelib.c)
endif (${cetmodules_FOUND})
