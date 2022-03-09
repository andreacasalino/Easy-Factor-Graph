SET(BUILD_XML_Parser_SAMPLES OFF CACHE BOOL "Override xml parser sample option" FORCE)
include(FetchContent)
FetchContent_Declare(
  xml_parser
  GIT_REPOSITORY https://github.com/andreacasalino/XML-parser.git
  GIT_TAG        5006262a76192b1af4988f1530be12cec4e8b74a
)
FetchContent_MakeAvailable(xml_parser)

FetchContent_Declare(
  trainers
  GIT_REPOSITORY https://github.com/andreacasalino/TrainingTools.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(trainers)

set(PROJECT_SHORTNAME "EFG")

MakeLibrary(${PROJECT_SHORTNAME} header)

target_include_directories(${PROJECT_SHORTNAME}
	PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/train/handlers>
)

find_package(OpenMP)
target_link_libraries(${PROJECT_SHORTNAME}
    PRIVATE
        XML-Parser
	OpenMP::OpenMP_CXX
    PUBLIC
	TrainingTools
)

