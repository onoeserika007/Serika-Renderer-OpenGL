cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME LearnOpenGL-Personal)
project (${TARGET_NAME})

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_BUILD_TYPE "RelWithDebInfo")

# 启用OpenMP支持
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# 用于配置项目中源文件的查找目录
#include_directories(${PROJECT_SOURCE_DIR}/include)
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${THIRD_PARTY_DIR}
        ${THIRD_PARTY_DIR}/glad/include
        ${THIRD_PARTY_DIR}/glfw/include
        ${THIRD_PARTY_DIR}/glm
        ${THIRD_PARTY_DIR}/stb
        ${THIRD_PARTY_DIR}/assimp/include
        ${THIRD_PARTY_DIR}/assimp/contrib
        ${THIRD_PARTY_DIR}/nlohmann
        ${THIRD_PARTY_DIR}/json11
#        ${THIRD_PARTY_DIR}/glsl
)

# set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3 -fopenmp")

# 将源代码添加到此项目的可执行文件。
file(GLOB_RECURSE SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp")
# resources dependencies

file(GLOB_RECURSE RESOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/*.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json
)

# assimp
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_ZLIB ON)
#set(BUILD_SHARED_LIBS ON)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)
add_subdirectory(${THIRD_PARTY_DIR}/assimp)

# glfw
add_subdirectory(${THIRD_PARTY_DIR}/glfw)

# glslang
#set(SKIP_GLSLANG_INSTALL ON)
#set(ENABLE_OPT OFF)
#set(ENABLE_SPVREMAPPER OFF)
#set(ENABLE_GLSLANG_BINARIES OFF)
#set(ENABLE_CTEST OFF)
#set(ENABLE_PCH OFF)
#set(BUILD_TESTING OFF)
#set(USE_CCACHE ON)
#add_subdirectory(${THIRD_PARTY_DIR}/glslang)

#####################################
##          EXECUTABLES        ######
#####################################
add_executable (
        ${TARGET_NAME}
        ${SOURCES}
        ${THIRD_PARTY_DIR}/json11/json11.cpp
        ${THIRD_PARTY_DIR}/glad/src/glad.c
        ${RESOURCE_FILES}
)

add_executable(
        TestMain
        ${PROJECT_SOURCE_DIR}/Test/TestMain.cpp
        ${PROJECT_SOURCE_DIR}/src/Base/ThreadPool.cpp
)

add_executable(
        TestCubeShadow
        ${PROJECT_SOURCE_DIR}/Test/CubeShadowTest.cpp
        ${THIRD_PARTY_DIR}/glad/src/glad.c
)

add_executable(
        FeatureTestMain
        ${PROJECT_SOURCE_DIR}/Test/FeatureTestMain.cpp
)
#####################################
##          EXECUTABLES        ######
#####################################

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
# SET(CMAKE_PREFIX_PATH "C:/SRC/SDK/GLFW")
# MESSAGE(WARNING $ENV{CMAKE_PREFIX_PATH} )
# MESSAGE(WARNING $ENV{VULKAN_SDK})
# 总结，prefix path需要写到具体包的位置
#message(WARNING $ENV{CMAKE_PREFIX_PATH})
# SET(CMAKE_PREFIX_PATH "")
#find_package(glfw3 REQUIRED)
#find_package(assimp REQUIRED)
#message(WARNING ${ASSIMP_LIBRARIES})

set(LINK_LIBS
        assimp
        glfw)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Building in Debug mode")
  # Add debug-specific settings here
#  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Building in Release mode")
  # Add release-specific settings here
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  message(STATUS "Building in RelWithDebInfo mode")
  # Add RelWithDebInfo-specific settings here
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  message(STATUS "Building in MinSizeRel mode")
  # Add MinSizeRel-specific settings here
else()
  message(STATUS "Building in unknown mode")
endif()

# output dir
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

#message(WARNING ${CMAKE_BINARY_DIR})
#message(WARNING ${EXECUTABLE_OUTPUT_PATH})

#add_dependencies(${TARGET_NAME} ResourceTargets)

target_link_libraries(${TARGET_NAME} PRIVATE ${LINK_LIBS})
target_link_libraries(TestCubeShadow PRIVATE ${LINK_LIBS})
# message(WARNING ${PROJECT_NAME})

# copy assets
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${TARGET_NAME}>/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/configs $<TARGET_FILE_DIR:${TARGET_NAME}>/configs
)