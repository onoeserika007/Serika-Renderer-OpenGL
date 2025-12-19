cmake_minimum_required(VERSION 3.10)

# 收集源文件
message(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB RENDERER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(serika_renderer "${RENDERER_SOURCES}")

target_include_directories(serika_renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(serika_renderer renderer_lib glad_lib glfw)