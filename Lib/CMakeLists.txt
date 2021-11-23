include(FetchContent)
FetchContent_Declare(
  xml_parser
  GIT_REPOSITORY https://github.com/andreacasalino/XML-parser.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(xml_parser)
set(BUILD_XML_Parser_SAMPLES OFF)

if(THREAD_POOL_OPT)
	include(FetchContent)
	FetchContent_Declare(
	  thread_pool
	  GIT_REPOSITORY https://github.com/andreacasalino/Cpp-Thread-Pool.git
	  GIT_TAG        master
	)
	FetchContent_MakeAvailable(thread_pool)
	set(BUILD_Thread_Pool_SAMPLES  OFF)
endif()

set(PROJECT_SHORTNAME "EFG")

MakeLibrary(${PROJECT_SHORTNAME} Header)

target_include_directories(${PROJECT_SHORTNAME}
	PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/train/handlers>
)

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

