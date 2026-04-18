Fetch(https://github.com/catchorg/Catch2.git v3.4.0 catch2)

set(TEST_NAME Tests)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${TEST_NAME} ${SOURCES} ${TEST_SOURCES})

target_link_libraries(${TEST_NAME} PUBLIC
    Catch2::Catch2
    Catch2::Catch2WithMain
    ${EFG_LIB_NAME}
)

target_include_directories(${TEST_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
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

