#pragma once
#include "../Renderer.h"

class ShaderGLSL;

class RendererOpenGL : public Renderer {
public:
	RendererOpenGL(const std::shared_ptr<Camera>& camera);
	virtual void init() override;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) const override;
	virtual std::shared_ptr<UniformSampler> createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format) const override;
	virtual std::shared_ptr<UniformSampler> createUniformSampler(const TextureInfo& texInfo) const override;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") override;
	virtual std::shared_ptr<Texture> createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData) const override;
	virtual std::shared_ptr<FrameBuffer> createFrameBuffer(bool offScreen) override;
	virtual void setupColorBuffer(std::shared_ptr<Texture>& outBuffer, bool multiSample, bool force = false) const override;
	virtual void setupDepthBuffer(std::shared_ptr<Texture>& outBuffer, bool multiSample, bool force = false) const override;
	virtual void setupShadowMapBuffer(std::shared_ptr<Texture>& outBuffer, int width, int height, bool multiSample, bool force = false) const override;
	virtual void setupGBuffer(std::shared_ptr<Texture>& outBuffer, bool multiSample, bool highp, bool force = false) const;

	virtual void setupVertexAttribute(BufferAttribute& vertexAttribute) override;
	virtual void setupGeometry(Geometry& geometry) override;
	virtual void loadShaders(Material& material) override;
	virtual void setupMaterial(Material& material) override;
	//virtual void setupStandardMaterial(StandardMaterial& material) override;
	virtual void setupMesh(UMesh &mesh, ShaderPass shaderPass) override;

	virtual void draw(UMesh &mesh, ShaderPass pass, const std::shared_ptr<ULight> &shadowLight) override;

	// pipeline related
	virtual void updateRenderStates(RenderStates& renderStates) override;
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer> frameBuffer, const ClearStates& states) override;
	virtual void endRenderPass() override;
	virtual void setRenderViewPort(int x, int y, int width, int height);
	virtual void waitIdle() override;

	// renderpasses
	virtual void dump(const std::shared_ptr<Texture> &srcTex, bool bFromColor, bool bBlend, std::shared_ptr<FrameBuffer> targetFrameBuffer, int dstColorBuffer, bool
	                  bDefferedShading, const std::vector<std::shared_ptr<Texture>> &defferedShadingPayloads) override;
	virtual std::shared_ptr<ShaderGLSL> getDefferedShadingProgram(const std::vector<std::shared_ptr<Texture>> & gBuffers) const override;

	virtual ~RendererOpenGL();
	virtual void clearTexture(Texture& texture) override;

private:
	std::shared_ptr<ShaderGLSL> getToScreenColorProgram(const std::shared_ptr<Texture> &srcTex) const;
	std::shared_ptr<ShaderGLSL> getToScreenDepthProgram(const std::shared_ptr<Texture> &srcTex) const;
};
