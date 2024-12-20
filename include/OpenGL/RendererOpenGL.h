#pragma once
#include "Renderer.h"

class ShaderGLSL;

class RendererOpenGL final : public Renderer {
public:
	RendererOpenGL(const std::shared_ptr<FCamera>& camera);
	virtual void init() override;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) const override;
	virtual std::shared_ptr<UniformSampler> createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format) const override;
	virtual std::shared_ptr<UniformSampler> createUniformSampler(const TextureInfo& texInfo) const override;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") override;
	virtual std::shared_ptr<Texture> createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData) const override;
	virtual std::shared_ptr<FrameBuffer> createFrameBuffer(bool offScreen) override;

	/** Setup Functions */
	virtual void setupVertexAttribute(BufferAttribute &vertexAttribute) const override;
	virtual void setupGeometry(FGeometry& geometry) const override;
	virtual void loadShaders(FMaterial& material) const override;
	virtual void setupMaterial(FMaterial& material) const override;
	//virtual void setupStandardMaterial(StandardMaterial& material) override;
	virtual void setupMesh(const std::shared_ptr<UMesh> &mesh, ShaderPass shaderPass) override;

	/** Draing Begin*/
	virtual void drawMesh(
		const std::shared_ptr<UMesh> &mesh,
		ShaderPass pass,
		const std::shared_ptr<ULight> &shadowLight,
		const std::shared_ptr<Shader> &overrideShader = {}) override;
	virtual void drawDebugBBoxes(const std::shared_ptr<BVHNode> &node, int depth, const std::shared_ptr<UObject> &rootObject) override;
	virtual void drawDebugBBox(const BoundingBox& bbox, const std::shared_ptr<UObject> &holdingObject, int depth) override;
	virtual void drawWorldAxis(const std::shared_ptr<UMesh> &holdingMesh) override;
	/** Draing End*/

	// pipeline related
	virtual void updateRenderStates(const RenderStates &inRenderStates) override;
	virtual void setCullFaceEnabled(bool bEnabled) override;
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer> frameBuffer, const ClearStates& states) override;
	virtual void endRenderPass() override;
	virtual void setRenderViewPort(int x, int y, int width, int height);
	virtual void waitIdle() override;

	// renderpasses
	virtual void dump(const
	                  std::shared_ptr<Shader> &program, bool bBlend, std::shared_ptr<FrameBuffer> targetFrameBuffer, int dstColorBuffer) override;
	virtual std::shared_ptr<ShaderGLSL> getDefferedShadingProgram(const std::vector<std::shared_ptr<Texture>> & gBuffers, EShadingModel shadingModel) const override;
	virtual std::shared_ptr<ShaderGLSL> getSSAOProgram(const std::vector<std::shared_ptr<Texture>> & gBuffers) const override;
	virtual std::shared_ptr<ShaderGLSL> getSSAOBlurProgram(const std::shared_ptr<Texture> &ssaoInput) const override;
	virtual std::shared_ptr<ShaderGLSL> getToScreenColorProgram(const std::shared_ptr<Texture> &srcTex) const override;
	virtual std::shared_ptr<ShaderGLSL> getToScreenDepthProgram(const std::shared_ptr<Texture> &srcTex) const override;

	std::shared_ptr<ShaderGLSL> getDebugBBoxProgram() const;
	std::shared_ptr<ShaderGLSL> getDrawDebuglineProgram() const;

	virtual ~RendererOpenGL();
	virtual void clearTexture(Texture& texture) override;
protected:
	/** Inner Functions */
	virtual void drawDebugPoint_Impl(const std::shared_ptr<FPoint> &point) override;
	virtual void drawDebugLine_Impl(const std::shared_ptr<FLine> &line) override;
	virtual void drawDebugTriangle_Impl(const std::shared_ptr<Triangle> &triangle) override;
private:
};
