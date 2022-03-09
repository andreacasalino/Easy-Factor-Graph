set(PROJECT_SHORTNAME "Sample-Utils")

MakeLibrary(${PROJECT_SHORTNAME} Header)

target_link_libraries(${PROJECT_SHORTNAME}
    PUBLIC
        EFG
)
