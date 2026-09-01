cmake_minimum_required(VERSION 3.16)

project(mdcat
    VERSION 1.0.0
    DESCRIPTION "A markdown-rendering cat replacement for the terminal"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

find_package(Boost REQUIRED COMPONENTS program_options regex)

add_executable(mdcat
    src/main.cpp
    src/markdown_renderer.cpp
    src/terminal.cpp
)

target_include_directories(mdcat PRIVATE src)

target_link_libraries(mdcat PRIVATE
    Boost::program_options
    Boost::regex
)

if(WIN32)
    target_compile_definitions(mdcat PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

install(TARGETS mdcat RUNTIME DESTINATION bin)
