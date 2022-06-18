set(PROJECT_SHORTNAME "EFG")

MakeLibrary(${PROJECT_SHORTNAME} header)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_SHORTNAME} PRIVATE
  Threads::Threads
)
option(BUILD_EFG_COMPILER_CHECK "Enable extra compiler checks" OFF)
if(BUILD_EFG_COMPILER_CHECK)
	target_compile_options(${PROJECT_SHORTNAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

option(BUILD_EFG_XML_CONVERTER "Compile features supporting conversion from and to xml files" ON)
if(BUILD_EFG_XML_CONVERTER)
  SET(BUILD_XML_Parser_SAMPLES OFF CACHE BOOL "Override xml parser sample option" FORCE)
  message("fetching XML parser")
  include(FetchContent)
  FetchContent_Declare(
    xml_parser
    GIT_REPOSITORY https://github.com/andreacasalino/XML-parser.git
    GIT_TAG        d3f2810bd5b1e278d0fb2e8eadd2b24ff533f11e
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
  if (NOT TARGET nlohmann_json::nlohmann_json)
    message(STATUS "fetching nlohmann json")
    include(FetchContent)
    FetchContent_Declare(
      nlohmann_json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG        7194245a314d142a5560f6906a87f7c67ebbcf2e
    )
    FetchContent_MakeAvailable(nlohmann_json)
  endif()
  
  target_link_libraries(${PROJECT_SHORTNAME} PUBLIC
    nlohmann_json::nlohmann_json
  )
  target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC
    EFG_JSON_IO=1
  )
endif()

option(BUILD_EFG_TRAINER_TOOLS "Compile features supporting learning of tunable models" ON)
if(BUILD_EFG_TRAINER_TOOLS)
  message("fetching training utils")
  include(FetchContent)
  FetchContent_Declare(
    trainers
    GIT_REPOSITORY https://github.com/andreacasalino/TrainingTools.git
    GIT_TAG        BetterEigenSupport
  )
  FetchContent_MakeAvailable(trainers)
  target_link_libraries(${PROJECT_SHORTNAME} PUBLIC
    TrainingTools
  )
  target_compile_definitions(${PROJECT_SHORTNAME} PUBLIC
    EFG_LEARNING_ENABLED=1
  )
endif()
