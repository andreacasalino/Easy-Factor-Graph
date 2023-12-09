Fetch(https://github.com/catchorg/Catch2.git v3.4.0 catch2)

set(TEST_NAME Tests)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(${TEST_NAME} ${SOURCES})

target_link_libraries(${TEST_NAME} PUBLIC
    Catch2::Catch2
    Catch2::Catch2WithMain
    EFG
)

install(TARGETS ${TEST_NAME})

######################################################################################

option(BUILD_EFG_TESTS_FILTERED "" OFF)
if(BUILD_EFG_TESTS_FILTERED)

set(SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/tests.py)
set(TESTS_BIN_LOCATION ${CMAKE_CURRENT_BINARY_DIR})
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(TESTS_BIN_LOCATION ${TESTS_BIN_LOCATION}/$<CONFIG>)
endif()
set(TESTS_BIN_LOCATION ${TESTS_BIN_LOCATION}/Tests)
message(STATUS "===> python script detected at ${SCRIPT}")
find_package (Python3 COMPONENTS Interpreter)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${SCRIPT}"
    "-m" "terraform"
    "-o" "filtered"
    RESULT_VARIABLE RETURN_CODE
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_subdirectory(filtered)

endif()

