if (DEFINED ENV{KDIR})
  set(KDIR "$ENV{KDIR}")
  execute_process(COMMAND cat ${KDIR}/include/config/kernel.release OUTPUT_VARIABLE KVERS OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  execute_process(COMMAND uname -r OUTPUT_VARIABLE KVERS OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

message("KVERS is ${KVERS}")

if (EXISTS /lib/module/${KVERS}/build)
    add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/module/${KVERS}/other_mod.ko
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Makefile ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/other_module.c ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Kbuild ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND make CC=/usr/bin/gcc
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/module/${KVERS}
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/other_mod.ko ${CMAKE_BINARY_DIR}/module/${KVERS}/other_mod.ko
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/other_module.c
    )

    add_custom_target(module1 ALL DEPENDS ${CMAKE_BINARY_DIR}/module/${KVERS}/other_mod.ko src_module)
endif()
#install_source()
