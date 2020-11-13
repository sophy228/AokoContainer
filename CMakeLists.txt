cmake_minimum_required(VERSION 3.13)
project(AokoContainer)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/fuse_files
)

add_library(clrutils STATIC
        "lib/loopdev.cpp"
        "lib/privileges.cpp"
        "lib/ns_lib.cpp"
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

add_executable(clrc
        "clrc_main.cpp"
)

target_link_libraries(clrc clrutils)