#pragma once
#include <glad/glad.h> // 包含glad来获取所有的必须OpenGL头文件
#include <string>
#include <vector>
#include <memory>
#include "Base/GLMInc.h"
#include "Shader.h"

enum ShaderType{
    ShaderType_VertexShader,
    ShaderType_FragmentShader
};

class ShaderGLSL: public Shader
{
    struct PassKey {
        explicit PassKey() {}
    };
    //static std::unordered_map<std::string, std::shared_ptr<ShaderGLSL>> shader_map_;
    // 程序ID
    unsigned ID;
    unsigned samplerBinding_ = 0;
    unsigned uniformBlockBinding_ = 0;

    std::string headers_;
    std::string defines_;
    std::string vertexCode_;
    std::string vsPath_;
    std::string fragmentCode_;
    std::string fsPath_;
    // 构造器读取并构建着色器
    ShaderGLSL(const std::string& vertexPath, const std::string& fragmentPath);
    ShaderGLSL();
public:
    explicit ShaderGLSL(PassKey passkey, const std::string& vertexPath, const std::string& fragmentPath) : ShaderGLSL(vertexPath, fragmentPath) {}
    static std::shared_ptr<ShaderGLSL> loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    static std::shared_ptr<ShaderGLSL> loadDefaultShader();
    static std::shared_ptr<ShaderGLSL> loadBaseColorShader();
    static std::shared_ptr<ShaderGLSL> loadBlinnPhongShader();
    static std::shared_ptr<ShaderGLSL> loadGeometryShader();
    static std::shared_ptr<ShaderGLSL> loadPhongMaterialShader();
    static std::shared_ptr<ShaderGLSL> loadLightMapMaterialShader();
    static std::shared_ptr<ShaderGLSL> loadStandardMaterialShader();
    static std::shared_ptr<ShaderGLSL> loadShadowPassShader();
    static std::shared_ptr<ShaderGLSL> loadFromRawSource(const std::string& VS, const std::string& FS);

    // compile
    void addHeader(const std::string& header);
    void addDefines(const std::vector<std::string>& defines);
    void addDefine(const std::string& define);
    void compileAndLink();
    
    // uniform工具函数
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;

    GLint getAttributeLocation(const std::string& name) const override;
    virtual unsigned getSamplerBinding() override;
    virtual unsigned getUniformBlockBinding() override;
    // 使用/激活程序
    virtual void setupPipeline(Material& material) override;
    virtual void use() override;
    virtual int getId() const override;
    virtual std::shared_ptr<Shader> clone() const override;
};
