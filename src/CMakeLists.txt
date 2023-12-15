MakeLibrary(${EFG_LIB_NAME} header)

find_package(Threads REQUIRED)
target_link_libraries(${EFG_LIB_NAME} PRIVATE
  Threads::Threads
)
option(BUILD_EFG_COMPILER_CHECK "Enable extra compiler checks" OFF)
if(BUILD_EFG_COMPILER_CHECK)
	target_compile_options(${EFG_LIB_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

function(LinkAndFlag DEP_LIB_NAME FLAG_NAME)
  target_link_libraries(${EFG_LIB_NAME} PUBLIC ${DEP_LIB_NAME})
  target_compile_definitions(${EFG_LIB_NAME} PUBLIC
    ${FLAG_NAME}=1
  )  
endfunction()

option(BUILD_EFG_XML_CONVERTER "Compile features supporting conversion from and to xml files" ON)
if(BUILD_EFG_XML_CONVERTER)
  SET(BUILD_XML_Parser_SAMPLES OFF CACHE BOOL "Override xml parser sample option" FORCE)
  if (NOT TARGET XML-Parser)
    Fetch(https://github.com/andreacasalino/XML-parser.git d3f2810bd5b1e278d0fb2e8eadd2b24ff533f11e xml_parser)
  endif()
  LinkAndFlag(XML-Parser EFG_XML_IO)
endif()

option(BUILD_EFG_JSON_CONVERTER "Compile features supporting conversion from and to json files" ON)
if(BUILD_EFG_JSON_CONVERTER)
  if (NOT TARGET nlohmann_json::nlohmann_json)
    Fetch(https://github.com/nlohmann/json.git 7194245a314d142a5560f6906a87f7c67ebbcf2e nlohmann_json)
  endif()
  LinkAndFlag(nlohmann_json::nlohmann_json EFG_JSON_IO)
endif()

option(BUILD_EFG_TRAINER_TOOLS "Compile features supporting learning of tunable models" ON)
if(BUILD_EFG_TRAINER_TOOLS)
  if (NOT TARGET TrainingTools)
    Fetch(https://github.com/andreacasalino/TrainingTools.git 5acb40e087f3150c206b756d72d51e787e752414 trainers)
  endif()
  LinkAndFlag(TrainingTools EFG_LEARNING_ENABLED)
endif()
