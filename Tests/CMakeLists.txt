add_subdirectory(GoogleTest/)
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)

add_subdirectory(Utils)

function(MakeTest NAME)
add_executable(${NAME} ${NAME}.cpp)

target_link_libraries(${NAME}
PUBLIC
	Test-Utils
)
endfunction()

MakeTest(Test01-Range)

MakeTest(Test02-Distribution)

MakeTest(Test03-DistributionFinder)

MakeTest(Test04-FactorSetting)

# MakeTest(Test05-FactorConstruction)

# MakeTest(Test06-FactorExponential)
