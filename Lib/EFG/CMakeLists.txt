set(PROJECT_SHORTNAME "EFG")

MakeLibrary(${PROJECT_SHORTNAME} Header)

target_include_directories(${PROJECT_SHORTNAME}
	PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/train/handlers>
)

if(ADVANCED_TRAINERS_OPT)
target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC ADVANCED_TRAINERS_ENABLED)
endif()

if(THREAD_POOL_OPT)
target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC THREAD_POOL_ENABLED)
find_package(OpenMP)
target_link_libraries(${PROJECT_SHORTNAME}
    PRIVATE
        XML-Parser
		OpenMP::OpenMP_CXX
	PUBLIC
		Thread-Pool
)
else()
target_link_libraries(${PROJECT_SHORTNAME}
    PRIVATE
        XML-Parser
)
endif()

