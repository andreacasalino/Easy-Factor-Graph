log("building lib XML-parser" ${CMAKE_CURRENT_LIST_LINE})
add_subdirectory(XML-parser/Lib)

if(THREAD_POOL_OPT)
log("building lib Thread-Pool" ${CMAKE_CURRENT_LIST_LINE})
add_subdirectory(Thread-Pool/Lib)
else()
log("Thread-Pool disabled" ${CMAKE_CURRENT_LIST_LINE})
endif()

log("building lib EFG" ${CMAKE_CURRENT_LIST_LINE})
add_subdirectory(EFG)
