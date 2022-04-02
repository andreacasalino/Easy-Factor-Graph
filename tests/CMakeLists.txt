if (WIN32)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif (WIN32)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        f2d71f0be2ff2230789cfaf37d4c4f38db5b1c7c
)
FetchContent_MakeAvailable(googletest)

function(MakeTest NAME)
add_executable(${NAME} ${NAME}.cpp)

target_link_libraries(${NAME}
PUBLIC
  gtest
  EFG
)

target_compile_definitions(${NAME} 
PUBLIC
	-D TEST_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/"
	-D SAMPLE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/../samples/"
)

install(TARGETS ${NAME})
endfunction()

MakeTest(Test01-Range)

MakeTest(Test02-Distribution)

MakeTest(Test03-DistributionFinder)

MakeTest(Test04-FactorSetting)

MakeTest(Test05-FactorConstruction)
MakeTest(Test05-SpecialFactorsConstruction)

MakeTest(Test06-Insertions)

# MakeTest(Test07-Evidences)

# MakeTest(Test08-BeliefPropagation)

# MakeTest(Test09-Gibbs)

# MakeTest(Test10-Learning)

# MakeTest(Test11-ThreadPoolEfficiency)
