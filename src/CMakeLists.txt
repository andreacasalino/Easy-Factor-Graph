set(PROJECT_SHORTNAME "EFG")

MakeLibrary(${PROJECT_SHORTNAME} header)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_SHORTNAME} PRIVATE
  Threads::Threads
)
target_compile_options(${PROJECT_SHORTNAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

option(BUILD_EFG_XML_CONVERTER "Compile features supporting conversion from and to xml files" ON)
if(BUILD_EFG_XML_CONVERTER)
  SET(BUILD_XML_Parser_SAMPLES OFF CACHE BOOL "Override xml parser sample option" FORCE)
  include(FetchContent)
  FetchContent_Declare(
    xml_parser
    GIT_REPOSITORY https://github.com/andreacasalino/XML-parser.git
    GIT_TAG        5006262a76192b1af4988f1530be12cec4e8b74a
  )
  FetchContent_MakeAvailable(xml_parser)
  target_link_libraries(${PROJECT_SHORTNAME} PRIVATE
    XML-Parser
  )
  target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC
    EFG_XML_IO=1
  )
endif()

option(BUILD_EFG_JSON_CONVERTER "Compile features supporting conversion from and to json files" ON)
if(BUILD_EFG_JSON_CONVERTER)
  message(STATUS "fetching nlohmann json")
  include(FetchContent)
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        7194245a314d142a5560f6906a87f7c67ebbcf2e
  )
  FetchContent_MakeAvailable(nlohmann_json)
  target_link_libraries(${PROJECT_SHORTNAME} PUBLIC
    nlohmann_json::nlohmann_json
  )
  target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC
    EFG_JSON_IO=1
  )
endif()

option(BUILD_EFG_TRAINER_TOOLS "Compile features supporting learning of tunable models" ON)
if(BUILD_EFG_TRAINER_TOOLS)
  FetchContent_Declare(
    trainers
    GIT_REPOSITORY https://github.com/andreacasalino/TrainingTools.git
    GIT_TAG        4d81b25919e7f37dba98481a914279bcac5f17fc
  )
  FetchContent_MakeAvailable(trainers)
  target_link_libraries(${PROJECT_SHORTNAME} PUBLIC
    TrainingTools
  )
  target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC
    EFG_LEARNING_ENABLED=1
  )
endif()
