#pragma once
#include <iostream>
#include <memory>
#include <RenderPass/RenderPass.h>

#include "OpenGL/ShaderGLSL.h"
#include "Base/RenderStates.h"
#include "Texture.h"
#include "Base/Config.h"

class RenderPass;
class BufferAttribute;
class Geometry;
class Texture;
class Material;
class StandardMaterial;
class Camera;
class Uniform;
class UniformBlock;
class UniformSampler;
class UObject;
class Shader;
class FrameBuffer;
class UMesh;

struct TextureInfo;
struct SamplerInfo;
struct TextureData;


enum RendererType {
	Renderer_SOFT,
	Renderer_OPENGL,
	Renderer_Vulkan,
};

enum class ShaderPass: uint8_t;
enum TextureTarget;
enum TextureFormat;

class Renderer {
public:
	explicit Renderer(const std::shared_ptr<Camera> &camera);
	virtual void init() = 0;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) const  = 0;
	virtual std::shared_ptr< UniformSampler> createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format) const = 0;
	virtual std::shared_ptr< UniformSampler> createUniformSampler(const TextureInfo& texInfo) const = 0;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") = 0;
	virtual std::shared_ptr<Texture> createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData = TextureData()) const = 0;
	virtual std::shared_ptr<FrameBuffer> createFrameBuffer(bool offScreen) = 0;
	virtual void setupColorBuffer(std::shared_ptr<Texture>& outBuffer, bool multiSample, bool force = false) const = 0;
	virtual void setupDepthBuffer(std::shared_ptr<Texture>& outBuffer, bool multiSample, bool force = false) const = 0;
	virtual void setupShadowMapBuffer(std::shared_ptr<Texture>& outBuffer, int width, int height, bool multiSample, bool force = false) const  = 0;
	virtual void setupGBuffer(std::shared_ptr<Texture>& outBuffer, bool multiSample, bool highp, bool force = false) const = 0;

	virtual void setupVertexAttribute(BufferAttribute& vertexAttribute) = 0;
	virtual void setupGeometry(Geometry& geometry) = 0;
	virtual void loadShaders(Material& material) = 0;
	virtual void setupMaterial(Material& material) = 0;
	//virtual void setupStandardMaterial(StandardMaterial& material) = 0;
	virtual void setupMesh(UMesh &mesh, ShaderPass shaderPass) = 0;

	virtual void draw(UMesh &mesh, ShaderPass pass, const std::shared_ptr<ULight> &shadowLight) = 0;

	void loadGlobalUniforms(UMesh& mesh);
	void loadUniformBlocks(Shader& program);

	void updateModelUniformBlock(UMesh & mesh, Camera& camera, const std::shared_ptr<ULight> &shadowLight = nullptr) const;
	void updateShadowCameraParamsToModelUniformBlock(const std::shared_ptr<Camera>& camera, const std::shared_ptr<ULight> &shadowLight) const;
	void updateLightUniformBlock(const std::shared_ptr<ULight>& light) const;

	// pipeline related
	virtual void updateRenderStates(RenderStates& renderStates) = 0;
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer> frameBuffer, const ClearStates& states) = 0;
	virtual void endRenderPass() = 0;

	virtual void setRenderViewPort(int x, int y, int width, int height) = 0;
	void setViewPort(int x, int y, int width, int height);
	void restoreViewPort();

	virtual void waitIdle() = 0;

	// renderpasses
	template <typename ...Args>
	void executeRenderPass(std::shared_ptr<RenderPass> renderPass, Args&&... args);
	virtual void dump(const std::shared_ptr<Texture> &srcTex, bool bFromColor, bool bBlend, std::shared_ptr<FrameBuffer> targetFrameBuffer, int dstColorBuffer, bool
	                  bDefferedShading, const std::vector<std::shared_ptr<Texture>> &defferedShadingPayloads) = 0;
	virtual std::shared_ptr<ShaderGLSL> getDefferedShadingProgram(const std::vector<std::shared_ptr<Texture>> & gBuffers) const = 0;

	virtual ~Renderer();
	virtual void clearTexture(Texture& texture) = 0;

	std::shared_ptr<Camera> getCamera() const;
	std::shared_ptr<Camera> getViewCamera() const;

	void setCamera(const std::shared_ptr<Camera>& camera);
	void setBackToViewCamera();
	int width() const;
	int height() const;

	ERenderMode render_mode;
	EShadingMode render_shading_mode;
	RenderStates renderStates;
protected:
	int width_ = 0;
	int height_ = 0;
	int viewer_x_ = 0;
	int viewer_y_ = 0;
	int viewer_width_ = 0;
	int viewer_height_ = 0;

	std::shared_ptr<Camera> mainCamera_;
	std::shared_ptr<Camera> viewCamera_;

	// global unniforms
	std::shared_ptr<UniformBlock> modelUniformBlock_;
	std::shared_ptr<UniformBlock> lightUniformBlock_;
	mutable std::weak_ptr<UniformSampler> shadowMapUniformSampler_;

	mutable std::unordered_map<int, std::shared_ptr<ShaderResources>> noObjectContextShaderResources;
};

template<typename ... Args>
void Renderer::executeRenderPass(std::shared_ptr<RenderPass> renderPass, Args&&... args) {
	renderPass->render(std::forward<Args>(args)...);
}
;
