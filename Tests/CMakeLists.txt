add_subdirectory(GoogleTest/)
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)

function(MakeTest NAME)
add_executable(${NAME} ${NAME}.cpp)

target_link_libraries(${NAME}
PUBLIC
	gtest
	EFG
)
endfunction()

MakeTest(Test01-Range)
