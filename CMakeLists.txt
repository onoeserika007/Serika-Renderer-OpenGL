# CMakeList.txt: learnOpenGL-aTriangle 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# for mingw
# add_compile_options(-msse4.1)

set(TARGET_NAME LearnOpenGL-Personal)
project (${TARGET_NAME})

# 用于配置项目中源文件的查找目录
#include_directories(${PROJECT_SOURCE_DIR}/include)
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${THIRD_PARTY_DIR}
        ${THIRD_PARTY_DIR}/glad
        ${THIRD_PARTY_DIR}/glfw/include
        ${THIRD_PARTY_DIR}/glm
        ${THIRD_PARTY_DIR}/KHR
        ${THIRD_PARTY_DIR}/stb
        ${THIRD_PARTY_DIR}/assimp/include
        ${THIRD_PARTY_DIR}/assimp/contrib
)

# assimp
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_ZLIB ON)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)
add_subdirectory(${THIRD_PARTY_DIR}/assimp)

# glfw
add_subdirectory(${THIRD_PARTY_DIR}/glfw)


# set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3 -fopenmp")
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE "Release")

# 将源代码添加到此项目的可执行文件。
add_executable (${TARGET_NAME} "src/main.cpp" "src/glad.c" "include/primitives.h"  "src/ShaderGLSL.cpp" "include/Shader.h" "include/Utils/utils.h" "src/Utils/utils.cpp" "include/Texture.h" "src/Texture.cpp" "include/Geometry.h" "include/Camera.h"  "include/Object.h" "include/BufferAttribute.h" "include/Material.h" "include/Light.h" "src/Object.cpp" "src/Materials/PhongMaterial.cpp" "src/Materials/Material.cpp" "src/Light.cpp" "src/Geometry.cpp"  "include/Model.h" "src/Model.cpp" "src/Materials/StandardMaterial.cpp" "include/Utils/Logger.h" "src/Utils/Logger.cpp" "include/OrbitController.h" "include/Buffer.h" "include/Utils/ImageUtils.h" "include/ResourceLoader.h" "src/Utils/ImageUtils.cpp"  "include/Utils/OpenGLUtils.h" "src/Utils/OpenGLUtils.cpp" "include/ShaderGLSL.h" "include/Uniform.h" "include/Utils/UUID.h" "include/UniformOpenGL.h" "src/UniformOpengl.cpp"  "include/Renderer.h"   "src/Renderer.cpp" "src/ResourceLoader.cpp" "src/Uniform.cpp" "include/FrameBuffer.h" "include/FrameBufferOpenGL.h" "include/OpenGL/EnumsOpenGL.h" "include/Base/RenderStates.h" "include/Viewer.h" "src/Viewer.cpp" "include/ViewerOpenGL.h" "include/Base/Config.h" "include/RenderPass/RenderPass.h" "src/RenderPass/RenderPass.cpp"   "include/RenderPass/RenderPassShadow.h"  "include/RendererOpenGL.h" "src/RendererOpenGL.cpp" "src/RenderPass/RenderPassShadow.cpp" "include/TextureOpenGL.h" "src/TextureOpenGL.cpp" "include/RenderPass/RenderPassGeometry.h" "src/RenderPass/RenderPassGeometry.cpp" "src/FrameBuffer.cpp" "src/FrameBufferOpenGL.cpp" "include/RenderPass/RenderPassToScreen.h" "src/RenderPass/RenderPassToScreen.cpp" "include/Base/TextureTypeEnum.def" "include/RenderPass/RenderPassPlain.h" "src/RenderPass/RenderPassPlain.cpp" "include/Scene.h" "src/Scene.cpp" "src/ViewerOpenGL.cpp")

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

# resources dependencies
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Assets)
file(GLOB_RECURSE DEPENDENCY_RESOURCES "${ASSETS_DIR}/*")
set_property(TARGET ${TARGET_NAME} PROPERTY OBJEC_DEPENDS ${DEPENDENCY_RESOURCES})

# output dir
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)


target_link_libraries(${TARGET_NAME} ${LINK_LIBS})
# message(WARNING ${PROJECT_NAME})

# copy assets
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${TARGET_NAME}>/assets
)