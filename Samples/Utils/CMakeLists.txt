set(PROJECT_SHORTNAME "Sample-Utils")

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

target_link_libraries(${PROJECT_SHORTNAME}
    PUBLIC
        EFG
)
