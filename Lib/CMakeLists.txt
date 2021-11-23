SET(BUILD_XML_Parser_SAMPLES OFF CACHE BOOL "Override xml parser sample option" FORCE)
include(FetchContent)
FetchContent_Declare(
  xml_parser
  GIT_REPOSITORY https://github.com/andreacasalino/XML-parser.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(xml_parser)

if(THREAD_POOL_OPT)
	SET(BUILD_Thread_Pool_SAMPLES OFF CACHE BOOL "Override thread pool sample option" FORCE)
	include(FetchContent)
	FetchContent_Declare(
	  thread_pool
	  GIT_REPOSITORY https://github.com/andreacasalino/Cpp-Thread-Pool.git
	  GIT_TAG        master
	)
	FetchContent_MakeAvailable(thread_pool)
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

