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

# set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3 -fopenmp")
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE "Release")

# 将源代码添加到此项目的可执行文件。
add_executable (learnOpenGL-aTriangle "src/main.cpp" "src/glad.c" "include/primitives.h"  "src/ShaderGLSL.cpp" "include/Shader.h" "include/Utils/utils.h" "src/Utils/utils.cpp" "include/stb_image.h"  "include/Texture.h" "src/Texture.cpp" "include/Geometry.h" "include/Camera.h"  "include/Object.h" "include/BufferAttribute.h" "include/Material.h" "include/Light.h" "src/Object.cpp" "src/Materials/PhongMaterial.cpp" "src/Materials/Material.cpp" "src/Light.cpp" "src/Geometry.cpp"  "include/Model.h" "src/Model.cpp" "src/Materials/StandardMaterial.cpp" "include/Utils/Logger.h" "src/Utils/Logger.cpp" "include/OrbitController.h" "include/Buffer.h" "include/Utils/ImageUtils.h" "include/ResourceLoader.h" "src/Utils/ImageUtils.cpp"  "include/Utils/OpenGLUtils.h" "src/Utils/OpenGLUtils.cpp" "include/ShaderGLSL.h" "include/Uniform.h" "include/Utils/UUID.h" "include/UniformOpenGL.h" "src/UniformOpengl.cpp"  "include/Renderer.h"   "src/Renderer.cpp" "src/ResourceLoader.cpp" "src/Uniform.cpp" "include/FrameBuffer.h" "include/FrameBufferOpenGL.h" "include/OpenGL/EnumsOpenGL.h" "include/Base/RenderStates.h" "include/Viewer.h" "src/Viewer.cpp" "include/ViewerOpenGL.h" "include/Base/Config.h" "include/RenderPass/RenderPass.h" "src/RenderPass/RenderPass.cpp"  "src/RenderPassOpenGL.cpp" "include/RenderPass/RenderPassShadow.h"  "include/RendererOpenGL.h" "src/RendererOpenGL.cpp" "src/RenderPass/RenderPassShadow.cpp" "include/TextureOpenGL.h" "src/TextureOpenGL.cpp" "include/RenderPass/RenderPassGeometry.h" "src/RenderPass/RenderPassGeometry.cpp" "src/FrameBuffer.cpp" "src/FrameBufferOpenGL.cpp" "include/RenderPass/RenderPassToScreen.h" "src/RenderPass/RenderPassToScreen.cpp" "include/Base/TextureTypeEnum.def" "include/RenderPass/RenderPassPlain.h" "src/RenderPass/RenderPassPlain.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET learnOpenGL-aTriangle PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
# SET(CMAKE_PREFIX_PATH "C:/SRC/SDK/GLFW")
# MESSAGE(WARNING $ENV{CMAKE_PREFIX_PATH} )
# MESSAGE(WARNING $ENV{VULKAN_SDK})
# 总结，prefixpath需要写到具体包的位置
message(WARNING $ENV{CMAKE_PREFIX_PATH})
# SET(CMAKE_PREFIX_PATH "")
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
message(WARNING ${ASSIMP_LIBRARIES})
message(WARNING ${glfw})
# find_library(ASSIMP_LIBRARY assimp $ENV{CMAKE_PREFIX_PATH})
# find_library(GLFW3_LIBRARY glfw3 $ENV{CMAKE_PREFIX_PATH})
# message(WARNING ${ASSIMP_LIBRARY})
# message(WARNING ${GLFW3_LIBRARY})


target_link_libraries(learnOpenGL-aTriangle glfw3 ${ASSIMP_LIBRARIES})
message(WARNING ${PROJECT_NAME})