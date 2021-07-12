if (WIN32)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif (WIN32)
add_subdirectory(GoogleTest/)

add_subdirectory(Utils)

function(MakeTest NAME)
add_executable(${NAME} ${NAME}.cpp)

target_link_libraries(${NAME}
PUBLIC
	Test-Utils
)

install(TARGETS ${NAME})
endfunction()

MakeTest(Test01-Range)

MakeTest(Test02-Distribution)

MakeTest(Test03-DistributionFinder)

MakeTest(Test04-FactorSetting)

MakeTest(Test05-FactorConstruction)

MakeTest(Test06-Insertions)

MakeTest(Test07-Evidences)

MakeTest(Test08-BeliefPropagation)

MakeTest(Test09-Gibbs)

MakeTest(Test10-Learning)

MakeTest(Test11-ThreadPoolEfficiency)
