#pragma once
#include <glad/glad.h>
#include <string>
#include <memory>
#include "Base/Globals.h"
#include "Material/Shader.h"

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
    mutable unsigned samplerBinding_ = 0;
    mutable unsigned uniformBlockBinding_ = 0;

    std::string headers_;
    std::string defines_;
    // vert
    std::string vertexCode_;
    std::string vsPath_;
    // frag
    std::string fragmentCode_;
    std::string fsPath_;
    // geom
    std::string geometryCode_;
    std::string gsPath_;

    static bool checkSourceVersionHeader(const std::string& code);

    // 构造器读取并构建着色器
    ShaderGLSL(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = {});
    ShaderGLSL();
public:
    explicit ShaderGLSL(PassKey passkey, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = {}) : ShaderGLSL(vertexPath, fragmentPath, geometryPath) {}
    static std::shared_ptr<ShaderGLSL> loadShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = {});

    static std::shared_ptr<ShaderGLSL> loadDefaultShader();
    static std::shared_ptr<ShaderGLSL> loadBaseColorShader();
    static std::shared_ptr<ShaderGLSL> loadBlinnPhongShader();
    static std::shared_ptr<ShaderGLSL> loadGeometryShader();

    static std::shared_ptr<ShaderGLSL> loadShadowPassShader();
    static std::shared_ptr<ShaderGLSL> loadShadowCubePassShader();
    static std::shared_ptr<ShaderGLSL> loadDefferedBlinnPhongShader();
    static std::shared_ptr<ShaderGLSL> loadSkyBoxShader();
    static std::shared_ptr<ShaderGLSL> loadFromRawSource(const std::string& VS, const std::string& FS, const std::string &GS = {});

    // compile
    virtual void addDefine(const std::string& define) override;
    void addHeader(const std::string& header);
    void compileAndLink();
    
    // uniform工具函数
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;

    GLint getAttributeLocation(const std::string& name) const override;
    virtual unsigned getSamplerBinding() const override;
    virtual unsigned getUniformBlockBinding() const override;
    // 使用/激活程序
    virtual void setupPipeline(FMaterial& material) override;
    virtual void use() const override;
    virtual GLuint getId() const override;
    virtual std::shared_ptr<Shader> clone() const override;
};
