#pragma once
#include "Renderer.h"

class ShaderGLSL;

class RendererOpenGL : public Renderer {
public:
	RendererOpenGL(const std::shared_ptr<Camera>& camera);
	virtual void init() override;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) override;
	virtual std::shared_ptr<UniformSampler> createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format) override;
	virtual std::shared_ptr<UniformSampler> createUniformSampler(const TextureInfo& texInfo) override;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") override;
	virtual std::shared_ptr<Texture> createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData) override;
	virtual std::shared_ptr<FrameBuffer> createFrameBuffer(bool offScreen) override;

	virtual void setupVertexAttribute(BufferAttribute& vertexAttribute) override;
	virtual void setupGeometry(Geometry& geometry) override;
	virtual void loadShaders(Material& material) override;
	virtual void setupMaterial(Material& material) override;
	//virtual void setupStandardMaterial(StandardMaterial& material) override;
	virtual void setupMesh(UMesh &mesh, ShaderPass shaderPass) override;

	virtual void useMaterial(Material& material) override;
	virtual void useMaterial(Material& material, ShaderPass pass) override;
	virtual void draw(UMesh &mesh, ShaderPass pass, const std::shared_ptr<Camera>&shadowCamera) override;

	// pipeline related
	virtual void updateRenderStates(RenderStates& renderStates) override;
	virtual RenderStates getRenderStates() override;
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer> frameBuffer, const ClearStates& states) override;
	virtual void endRenderPass() override;
	virtual void setRenderViewPort(int x, int y, int width, int height);
	virtual void waitIdle() override;

	// renderpasses
	virtual void renderToScreen(UniformSampler& outTex, int screen_width, int screen_height, bool bFromColor) override;

	virtual ~RendererOpenGL();
	virtual void clearTexture(Texture& texture) override;

private:
	std::shared_ptr<ShaderGLSL> getToScreenColorProgram(UniformSampler& outTex);
	std::shared_ptr<ShaderGLSL> getToScreenDepthProgram(UniformSampler& outTex);
};
