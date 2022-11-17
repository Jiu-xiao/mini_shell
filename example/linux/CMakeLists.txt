cmake_minimum_required(VERSION 3.10)

add_compile_options(-g)

add_subdirectory(../.. ms.out)

target_include_directories(
    MiniShell
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

project(example)

add_executable(example main.c)

add_dependencies(example MiniShell)

target_link_libraries(
    example
    PUBLIC MiniShell
)

target_include_directories(
    example
    PRIVATE $<TARGET_PROPERTY:MiniShell,INTERFACE_INCLUDE_DIRECTORIES>
)
