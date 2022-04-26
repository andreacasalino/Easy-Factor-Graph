set(PROJECT_SHORTNAME "Samples-Helpers")

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_SHORTNAME} ${SRC_FILES})

target_link_libraries(${PROJECT_SHORTNAME} PUBLIC EFG)

target_include_directories(${PROJECT_SHORTNAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
