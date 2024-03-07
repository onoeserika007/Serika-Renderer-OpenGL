# CMakeList.txt: learnOpenGL-aTriangle 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("learnOpenGL-aTriangle")

# 用于配置项目中源文件的查找目录
include_directories(${PROJECT_SOURCE_DIR}/include)
set(PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3 -fopenmp")
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE "Release")

# 将源代码添加到此项目的可执行文件。
add_executable (learnOpenGL-aTriangle "src/learnOpenGL-aTriangle.cpp" "src/glad.c" "include/primitives.h"  "src/Shader.cpp" "include/Shader.h" "include/Utils/utils.h" "src/Utils/utils.cpp" "include/stb_image.h" "src/stb_image.cpp" "include/Texture.h" "src/Texture.cpp" "include/Geometry.h" "include/Camera.h"  "include/Object.h" "include/BufferAttribute.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET learnOpenGL-aTriangle PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
# SET(CMAKE_PREFIX_PATH "C:/SRC/SDK/GLFW")
# MESSAGE(WARNING $ENV{CMAKE_PREFIX_PATH} )
# MESSAGE(WARNING $ENV{VULKAN_SDK})
# 总结，prefixpath需要写到具体包的位置
find_package(glfw3 REQUIRED)


target_link_libraries(learnOpenGL-aTriangle glfw)
message(WARNING ${PROJECT_NAME})