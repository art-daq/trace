if ("cxx_lambdas" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  cet_make_exec(example_main SOURCE
		example_main.cc
		example_sub1.cc
		example_sub2.cc
		example_sub3.cc
		example_sub4.cc
		LIBRARIES
		pthread
    )

  cet_make_exec(no_std SOURCE no_std.cc)
  cet_make_exec(just SOURCE just.cc example_sub_.cc)
  cet_make_exec(just_user SOURCE just_user.cc)
else()
  message(WARNING "C++ userspace examples not built due to lack of compiler support for CXX lambdas.")
endif()
cet_make_exec(basic_c SOURCE basic_c.c)

#install_source()
