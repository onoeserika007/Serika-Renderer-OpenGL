cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME LearnOpenGL-Personal)
project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_BUILD_TYPE "RelWithDebInfo")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

# 启用编译命令导出，以便使用代码分析工具
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 启用OpenMP支持
find_package(OpenMP REQUIRED)

if(OpenMP_CXX_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# 用于配置项目中源文件的查找目录
# include_directories(${PROJECT_SOURCE_DIR}/include)
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

# set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3 -fopenmp")


file(GLOB IMGUI_SOURCES
        ${THIRD_PARTY_DIR}/imgui/*.cpp
        ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp
)


# glslang
# set(SKIP_GLSLANG_INSTALL ON)
# set(ENABLE_OPT OFF)
# set(ENABLE_SPVREMAPPER OFF)
# set(ENABLE_GLSLANG_BINARIES OFF)
# set(ENABLE_CTEST OFF)
# set(ENABLE_PCH OFF)
# set(BUILD_TESTING OFF)
# set(USE_CCACHE ON)
# add_subdirectory(${THIRD_PARTY_DIR}/glslang)

# 启用AddressSanitizer
if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
else()
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
endif()

#target_compile_options(${TARGET_NAME} PRIVATE -Werror=return-type)


# TODO: 如有需要，请添加测试并安装目标。
# SET(CMAKE_PREFIX_PATH "C:/SRC/SDK/GLFW")
# MESSAGE(WARNING $ENV{CMAKE_PREFIX_PATH} )
# MESSAGE(WARNING $ENV{VULKAN_SDK})
# 总结，prefix path需要写到具体包的位置
# message(WARNING $ENV{CMAKE_PREFIX_PATH})
# SET(CMAKE_PREFIX_PATH "")
# find_package(glfw3 REQUIRED)
# find_package(assimp REQUIRED)
# message(WARNING ${ASSIMP_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Building in Debug mode")

# Add debug-specific settings here
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug)
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

########################################################
##############  Third-Party Libraries  #################
########################################################

# assimp
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_ZLIB ON)
# set(BUILD_SHARED_LIBS ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)
add_subdirectory(${THIRD_PARTY_DIR}/assimp)

# json11
add_subdirectory(${THIRD_PARTY_DIR}/json11)

# glfw
add_subdirectory(${THIRD_PARTY_DIR}/glfw)

# glad
add_subdirectory(${THIRD_PARTY_DIR}/glad)

# imgui
add_subdirectory(${THIRD_PARTY_DIR}/imgui)

########################################################
#################  Main Projects  ######################
########################################################
add_subdirectory(${PROJECT_SOURCE_DIR}/mods/Renderer)

########################################################
####################     Tests      ####################
########################################################

add_subdirectory(${PROJECT_SOURCE_DIR}/Test/Renderer)
