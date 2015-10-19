execute_process(COMMAND uname -r OUTPUT_VARIABLE KVERS OUTPUT_STRIP_TRAILING_WHITESPACE)
message("KVERS is ${KVERS}")

add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../../module/${KVERS}/some_mod.ko
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Makefile ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/steve_module.c ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/some_module.c ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Kbuild ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND make CC=/usr/bin/gcc
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/../module/${KVERS}
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/some_mod.ko ${CMAKE_CURRENT_BINARY_DIR}/../../module/${KVERS}/some_mod.ko
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/steve_mod.ko ${CMAKE_CURRENT_BINARY_DIR}/../../module/${KVERS}/steve_mod.ko
		COMMAND make clean
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/some_module.c ${CMAKE_CURRENT_SOURCE_DIR}/steve_module.c
)
add_custom_target(some_mod ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../../module/${KVERS}/some_mod.ko src_module)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../../module/${KVERS}/some_mod.ko ${CMAKE_CURRENT_BINARY_DIR}/../../module/${KVERS}/steve_mod.ko 
	    DESTINATION ${${product}_inc_dir}/../module/${KVERS})
