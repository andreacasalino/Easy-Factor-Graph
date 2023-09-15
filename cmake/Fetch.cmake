function(Fetch URL TAG NAME)
    message("fetching ${URL} may take a while ...")
    include(FetchContent)
    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY ${URL}
        GIT_TAG        ${TAG}
    )
    FetchContent_MakeAvailable(${NAME})
endfunction()
