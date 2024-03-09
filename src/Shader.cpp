#include "Shader.h"
#include "Utils/utils.h"
#include <vector>

std::unordered_map<std::string, std::shared_ptr<Shader>> Shader::shader_map_;

std::shared_ptr<Shader> Shader::loadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    const auto& key = vertexPath + fragmentPath;
    if (shader_map_.count(key)) {
        return shader_map_[key];
    }
    else {
        // 更高的异常安全级别，防止构造 shared_ptr 之前就调用 new ，抛出异常;
        // 仅分配1次内存来保存引用对象和控制块(两者内存分布是连续的);
        // 原因是 make_shared 函数模板并非 Widget 类的友元函数，其访问了私有构造函数。而静态成员函数可以访问类的私有成员(比如这里的私有构造函数)，因此可以在 create 内部调用 new (两步：分配内存、调用构造函数)。
        // 不知道是不是 C++ 在制定 make_shared 的标准时疏忽的一点，但是在保持可移植性的情况下，最简单的方法就是用 new 替代 make_shared ，而且仔细来看， make_shared 的性能优势可能并没那么重要，至于异常安全，大多数时候程序处理 new 抛出的异常就是任其终止。
        //return shader_map_[key] = std::make_shared<Shader>(vertexPath, fragmentPath); 
        //return shader_map_[key] = std::shared_ptr<Shader>(new Shader(vertexPath, fragmentPath));

        // 或者采用PassKey方法
        return shader_map_[key] = std::make_shared<Shader>(PassKey(), vertexPath, fragmentPath);
    }
}

std::shared_ptr<Shader> Shader::loadDefaultShader() {
    return loadShader("./assets/shader/default.vert", "./assets/shader/default.frag");
}

std::shared_ptr<Shader> Shader::loadPhongMaterialShader() {
    return loadShader("./assets/shader/PhongMaterial.vert", "./assets/shader/PhongMaterial.frag");
}

// 构造器读取并构建着色器
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    // 1. 从文件路径中获取顶点/片段着色器
    const auto& vertexCode = readFile(vertexPath);
    const auto& fragmentCode = readFile(fragmentPath);
    const char* vShaderCode = vertexCode.data();
    const char* fShaderCode = fragmentCode.data();

    // 2. 编译着色器
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // 打印编译错误（如果有的话）
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        std::cout << "VertexShader Path: " + vertexPath << std::endl;
    };

    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // 打印编译错误（如果有的话）
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        std::cout << "FragmentShader Path: " + fragmentPath << std::endl;
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
// 使用/激活程序
void Shader::use() {
    glUseProgram(ID);
}
// uniform工具函数
void Shader::setBool(const std::string& name, bool value) const {
    // 注意，查询uniform地址不要求你之前使用过着色器程序，但是更新一个uniform之前你必须先使用程序（调用glUseProgram)，因为它是在当前激活的着色器程序中设置uniform的。
    glUseProgram(ID);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const {
    glUseProgram(ID);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const {
    glUseProgram(ID);
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setMat4(const std::string& name, glm::mat4 value) const {
    glUseProgram(ID);
    auto mat4loc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(mat4loc, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setVec3(const std::string& name, glm::vec3 value) const {
    glUseProgram(ID);
    auto vec3Loc = glGetUniformLocation(ID, name.c_str());
    glUniform3f(vec3Loc, value.x, value.y, value.z);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    setVec3(name, glm::vec3(x, y, z));
}
GLint Shader::getAttributeLocation(const std::string& name) const {
    return glGetAttribLocation(ID, name.data());
}
