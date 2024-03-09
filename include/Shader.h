#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>; // 包含glad来获取所有的必须OpenGL头文件

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Shader
{
    struct PassKey {
        explicit PassKey() {}
    };
    static std::unordered_map<std::string, std::shared_ptr<Shader>> shader_map_;
    // 程序ID
    unsigned int ID;
    // 构造器读取并构建着色器
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
public:
    explicit Shader(PassKey passkey, const std::string& vertexPath, const std::string& fragmentPath): Shader(vertexPath, fragmentPath) {}
    static std::shared_ptr<Shader> loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    static std::shared_ptr<Shader> loadDefaultShader();
    static std::shared_ptr<Shader> loadPhongMaterialShader();
    // 使用/激活程序
    void use();
    // uniform工具函数
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;

    GLint getAttributeLocation(const std::string& name) const;
};

#endif