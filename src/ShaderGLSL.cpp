#include "ShaderGLSL.h"
#include "Base/GLMInc.h"
#include "Utils/utils.h"
#include "Utils/OpenGLUtils.h"
#include <vector>
#include "../include/Base/ResourceLoader.h"
#include "Material.h"

//std::unordered_map<std::string, std::shared_ptr<ShaderGLSL>> ShaderGLSL::shader_map_;
const std::string OPENGL_VERSION_HEADER = "#version 430 core";

// 构造器读取并构建着色器
ShaderGLSL::ShaderGLSL(const std::string& vertexPath, const std::string& fragmentPath) {
    // 1. 从文件路径中获取顶点/片段着色器
    vertexCode_ = ResourceLoader::loadShader(vertexPath);
    fragmentCode_ = ResourceLoader::loadShader(fragmentPath);

    vsPath_ = vertexPath;
    fsPath_ = fragmentPath;

    addHeader(OPENGL_VERSION_HEADER);
}

ShaderGLSL::ShaderGLSL()
{
    addHeader(OPENGL_VERSION_HEADER);
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    // 更高的异常安全级别，防止构造 shared_ptr 之前就调用 new ，抛出异常;
    // 仅分配1次内存来保存引用对象和控制块(两者内存分布是连续的);
    // 原因是 make_shared 函数模板并非 Widget 类的友元函数，其访问了私有构造函数。而静态成员函数可以访问类的私有成员(比如这里的私有构造函数)，因此可以在 create 内部调用 new (两步：分配内存、调用构造函数)。
    // 不知道是不是 C++ 在制定 make_shared 的标准时疏忽的一点，但是在保持可移植性的情况下，最简单的方法就是用 new 替代 make_shared ，而且仔细来看， make_shared 的性能优势可能并没那么重要，至于异常安全，大多数时候程序处理 new 抛出的异常就是任其终止。
    // return shader_map_[key] = std::make_shared<ShaderGLSL>(vertexPath, fragmentPath); 
    // return shader_map_[key] = std::shared_ptr<ShaderGLSL>(new ShaderGLSL(vertexPath, fragmentPath));

    // 或者采用PassKey方法
    return std::make_shared<ShaderGLSL>(PassKey(), vertexPath, fragmentPath);
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadDefaultShader() {
    return loadShader("./assets/shader/default.vert", "./assets/shader/default.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadBaseColorShader() {
    return loadShader("assets/shader/Passes/PlainPass/BaseColor.vert", "assets/shader/Passes/PlainPass/BaseColor.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadBlinnPhongShader()
{
    return loadShader("./assets/shader/Passes/PlainPass/Blinn-Phong.vert", "./assets/shader/Passes/PlainPass/Blinn-Phong.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadGeometryShader() {
    return loadShader("./assets/shader/Passes/DefferedRendering/Geometry.vert", "./assets/shader/Passes/DefferedRendering/Geometry.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadPhongMaterialShader() {
    return loadShader("./assets/shader/PhongMaterial.vert", "./assets/shader/PhongMaterial.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadLightMapMaterialShader()
{
    return loadShader("./assets/shader/LightMap/lightMap.vert", "./assets/shader/LightMap/lightMap.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadStandardMaterialShader()
{
    return loadShader("./assets/shader/Standard.vert", "./assets/shader/Standard.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadShadowPassShader()
{
    return loadShader("./assets/shader/Passes/ShadowPass/ShadowPass.vert", "./assets/shader/Passes/ShadowPass/ShadowPass.frag");
}

std::shared_ptr<ShaderGLSL> ShaderGLSL::loadFromRawSource(const std::string& VS, const std::string& FS)
{
    auto ret = std::shared_ptr<ShaderGLSL>(new ShaderGLSL());
    ret->vertexCode_ = VS;
    ret->fragmentCode_ = FS;

    ret->vsPath_ = "";
    ret->fsPath_ = "";
    return ret;
}

void ShaderGLSL::addHeader(const std::string& header)
{
    headers_ += header + "\n";
}

void ShaderGLSL::addDefines(const std::vector<std::string>& defines)
{
    for (const auto& def : defines) {
        addDefine(def);
    }
}

void ShaderGLSL::addDefine(const std::string& define)
{
    defines_ += "#define " + define + "\n";
}

void ShaderGLSL::compileAndLink()
{
    auto vertexCode = headers_ + defines_ + vertexCode_;
    const char* vShaderSource = vertexCode.data();

    auto fragmentCode = headers_ + defines_ + fragmentCode_;
    const char* fShaderSource = fragmentCode.data();

    // 2. 编译着色器
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // 顶点着色器
    GL_CHECK(vertex = glCreateShader(GL_VERTEX_SHADER));
    GL_CHECK(glShaderSource(vertex, 1, &vShaderSource, NULL));
    GL_CHECK(glCompileShader(vertex));
    // 打印编译错误（如果有的话）
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        std::cout << "VertexShader Path: " + vsPath_ << std::endl;
        std::cout << vertexCode << std::endl;
        exit(-1);
    };

    // 片段着色器
    GL_CHECK(fragment = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CHECK(glShaderSource(fragment, 1, &fShaderSource, NULL));
    GL_CHECK(glCompileShader(fragment));
    // 打印编译错误（如果有的话）
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        std::cout << "FragmentShader Path: " + fsPath_ << std::endl;
        std::cout << fragmentCode << std::endl;
        exit(-1);
    };

    // 着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    // 打印连接错误（如果有的话）
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // 删除着色器，它们已经链接到我们的程序中了，已经不再需要了
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void ShaderGLSL::setupPipeline(Material& material)
{
    if (!ready()) {
        addDefines(material.getDefines());
        compileAndLink();
        setReady(true);
    }
    
}

// 使用/激活程序
void ShaderGLSL::use() {
    GL_CHECK(glUseProgram(ID));
    samplerBinding_ = 0;
    uniformBlockBinding_ = 0;
}

// uniform工具函数
void ShaderGLSL::setBool(const std::string& name, bool value) const {
    // 注意，查询uniform地址不要求你之前使用过着色器程序，但是更新一个uniform之前你必须先使用程序（调用glUseProgram)，因为它是在当前激活的着色器程序中设置uniform的。
    glUseProgram(ID);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void ShaderGLSL::setInt(const std::string& name, int value) const {
    glUseProgram(ID);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void ShaderGLSL::setFloat(const std::string& name, float value) const {
    glUseProgram(ID);
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void ShaderGLSL::setMat4(const std::string& name, glm::mat4 value) const {
    glUseProgram(ID);
    auto mat4loc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(mat4loc, 1, GL_FALSE, glm::value_ptr(value));
}
void ShaderGLSL::setVec3(const std::string& name, glm::vec3 value) const {
    glUseProgram(ID);
    auto vec3Loc = glGetUniformLocation(ID, name.c_str());
    glUniform3f(vec3Loc, value.x, value.y, value.z);
}
void ShaderGLSL::setVec3(const std::string& name, float x, float y, float z) const {
    setVec3(name, glm::vec3(x, y, z));
}
GLint ShaderGLSL::getAttributeLocation(const std::string& name) const {
    return glGetAttribLocation(ID, name.data());
}

unsigned ShaderGLSL::getSamplerBinding()
{
    return samplerBinding_++;
}

unsigned ShaderGLSL::getUniformBlockBinding()
{
    return uniformBlockBinding_++;
}

int ShaderGLSL::getId() const 
{
    return ID;
}

std::shared_ptr<Shader> ShaderGLSL::clone() const
{
    return std::make_shared<ShaderGLSL>(*this);
}
