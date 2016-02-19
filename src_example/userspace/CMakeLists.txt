cet_make_exec(example_main SOURCE 
						   example_main.cc
						   example_sub1.cc
						   example_sub2.cc
						   example_sub3.cc
						   example_sub4.cc
						   LIBRARIES
						   trace
						   pthread
)

cet_make_exec(no_std SOURCE no_std.cc)
cet_make_exec(just SOURCE just.cc)
cet_make_exec(just_user SOURCE just_user.cc)
cet_make_exec(basic_c SOURCE basic_c.c)

#install_source()
