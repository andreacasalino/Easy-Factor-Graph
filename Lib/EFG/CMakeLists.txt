set(PROJECT_SHORTNAME "EFG")

CollectSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)
GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

if(LIB_OPT)
	add_library(${PROJECT_SHORTNAME} SHARED ${SOURCES})
else()
	add_library(${PROJECT_SHORTNAME} STATIC ${SOURCES})
endif()

target_compile_features(${PROJECT_SHORTNAME}
    PUBLIC cxx_auto_type
    PRIVATE cxx_variadic_templates
)

target_include_directories(${PROJECT_SHORTNAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Header>
)

if(THREAD_POOL_OPT)
target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC THREAD_POOL_ENABLED)
target_link_libraries(${PROJECT_SHORTNAME}
    PUBLIC
        XMLPrs::XML-Parser
	thpl::Thread-Pool
)
else()
target_link_libraries(${PROJECT_SHORTNAME}
    PUBLIC
        XMLPrs::XML-Parser
)
endif()

