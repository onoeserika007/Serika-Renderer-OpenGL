cmake_minimum_required(VERSION 3.10)

# 收集源文件
message(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE RENDERER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(renderer_lib ${RENDERER_SOURCES})

target_link_libraries(renderer_lib PUBLIC assimp::assimp nlohmann_json Boost::pfr imgui)

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/ThirdParty)

target_include_directories(renderer_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${THIRD_PARTY_DIR}/glm/include
    ${THIRD_PARTY_DIR}/glad/include
    ${THIRD_PARTY_DIR}/glfw/include
    ${THIRD_PARTY_DIR}/assimp/include
    ${THIRD_PARTY_DIR}/stb
    ${THIRD_PARTY_DIR}/assimp/include
    ${THIRD_PARTY_DIR}/assimp/contrib
    ${THIRD_PARTY_DIR}/imgui
)
