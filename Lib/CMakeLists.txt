project(Easy-Factor-Graph-Core)

add_subdirectory(XML-parser/Lib)

if(THREAD_POOL_OPT)
add_subdirectory(Thread-Pool/Lib)
endif()

add_subdirectory(EFG)
