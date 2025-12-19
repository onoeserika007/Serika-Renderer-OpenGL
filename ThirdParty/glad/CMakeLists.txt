cmake_minimum_required(VERSION 3.10)

# 收集源文件
message(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB GLAD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(glad_lib ${GLAD_SOURCES})

target_include_directories(glad_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
