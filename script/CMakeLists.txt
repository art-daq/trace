cet_script(
		big_ex.sh
		bitN_to_mask
		trace_delta
		trace_envvars
		trace_feature_test.sh
		trace_functions.sh
)

include(CetTest)

cet_test_env("TRACE_DIR=${CMAKE_SOURCE_DIR}")

cet_test(big_ex_t HANDBUILT
  TEST_EXEC big_ex.sh
  TEST_ARGS test.d -t4 -l50
        )


