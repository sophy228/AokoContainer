include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

add_library(clrutils STATIC
        "lib/loopdev.cpp"
        "lib/privileges.cpp"
)

add_executable(clrd 
        "clrd_loop.cpp"
        "clrd_child_main.cpp"
        "clrd_namespace.cpp"
        "clrd.cpp"
        "clrd_linuxinit.cpp"
        "clrd_mount.cpp"
)
target_link_libraries(clrd clrutils)