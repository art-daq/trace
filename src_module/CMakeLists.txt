if (DEFINED ENV{KDIR})
  set(KDIR "$ENV{KDIR}")
  execute_process(COMMAND cat ${KDIR}/include/config/kernel.release OUTPUT_VARIABLE KVERS OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  execute_process(COMMAND uname -r OUTPUT_VARIABLE KVERS OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

message("KVERS is ${KVERS}")

add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../module/${KVERS}/TRACE.ko
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Makefile ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/trace_.c ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Kbuild ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND make CC=/usr/bin/gcc
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/../module/${KVERS}
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/TRACE.ko ${CMAKE_CURRENT_BINARY_DIR}/../module/${KVERS}/TRACE.ko
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/Module.symvers ${CMAKE_CURRENT_BINARY_DIR}/../module/${KVERS}/Module.symvers
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/trace_.c
)

add_custom_target(src_module ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../module/${KVERS}/TRACE.ko)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../module/ DESTINATION ${${product}_inc_dir}/../module)

#install_source()
