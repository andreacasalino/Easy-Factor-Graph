set(PROJECT_SHORTNAME "Test-Utils")

MakeLibrary(${PROJECT_SHORTNAME} Header)

target_link_libraries(${PROJECT_SHORTNAME}
PUBLIC
	gtest
	EFG
)

target_compile_definitions(${PROJECT_SHORTNAME} 
PUBLIC
	-D TEST_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/../"
)
