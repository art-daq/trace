if (${cetmodules_FOUND})
  cet_script( ALWAYS_COPY
		big_ex.sh
		bitN_to_mask
		trace_delta
		trace_maxents
		trace_envvars
		trace_feature_test.sh
		trace_functions.sh
		trace_color
		trace-cmd-merge
  )

  if( DEFINED $ENV{DO_BIGEX_TEXT} )

    include(CetTest)

    cet_test_env("TRACE_DIR=${CMAKE_SOURCE_DIR}")

    cet_test(big_ex_t HANDBUILT
      TEST_EXEC big_ex.sh
      TEST_ARGS test.d -t4 -l50
        )

  endif()
else (${cetmodules_FOUND})
  install(PROGRAMS bitN_to_mask trace_envvars trace_delta trace-cmd-merge DESTINATION bin)
  install(PROGRAMS trace_functions.sh DESTINATION etc/profile.d)
  install(PROGRAMS trace_functions.sh DESTINATION bin)  # OK of DUNE patch (to this file) causes double bin install
endif (${cetmodules_FOUND})
