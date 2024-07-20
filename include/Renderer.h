#pragma once
#include <iostream>
#include <memory>
#include <RenderPass/RenderPass.h>

#include "ShaderGLSL.h"
#include "Base/RenderStates.h"
#include "Texture.h"

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
	Renderer(Camera& camera);
	virtual void init() = 0;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) = 0;
	virtual std::shared_ptr< UniformSampler> createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format) = 0;
	virtual std::shared_ptr< UniformSampler> createUniformSampler(const TextureInfo& texInfo) = 0;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") = 0;
	virtual std::shared_ptr<Texture> createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData = TextureData()) = 0;
	virtual std::shared_ptr<FrameBuffer> createFrameBuffer(bool offScreen) = 0;

	virtual void setupVertexAttribute(BufferAttribute& vertexAttribute) = 0;
	virtual void setupGeometry(Geometry& geometry) = 0;
	virtual void loadShaders(Material& material) = 0;
	virtual void setupMaterial(Material& material) = 0;
	//virtual void setupStandardMaterial(StandardMaterial& material) = 0;
	virtual void setupMesh(UMesh &mesh, ShaderPass shaderPass) = 0;

	virtual void useMaterial(Material& material) = 0;
	virtual void useMaterial(Material& material, ShaderPass pass) = 0;
	virtual void draw(UMesh &mesh, ShaderPass pass, const std::shared_ptr<Camera>&shadowCamera) = 0;

	void updateModelUniformBlock(UMesh & mesh, Camera& camera, const std::shared_ptr<Camera> &shadowCamera = nullptr) const;
	void updateLightUniformBlock(Shader& shader, const std::shared_ptr<ULight>& light) const;
	void updateLightUniformBlock(UMesh &mesh, const std::shared_ptr<ULight>& light) const;

	// pipeline related
	virtual void updateRenderStates(RenderStates& renderStates) = 0;
	virtual RenderStates getRenderStates() = 0;
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer> frameBuffer, const ClearStates& states) = 0;
	virtual void endRenderPass() = 0;

	virtual void setRenderViewPort(int x, int y, int width, int height) = 0;
	void setViewPort(int x, int y, int width, int height);
	void restoreViewPort();

	virtual void waitIdle() = 0;

	// renderpasses
	template <typename ...Args>
	void executeRenderPass(std::shared_ptr<RenderPass> renderPass, Args&&... args);
	virtual void renderToScreen(UniformSampler& outTex, int screen_width, int screen_height, bool bFromColor) = 0;

	virtual ~Renderer();
	virtual void clearTexture(Texture& texture) = 0;

	Camera& getCamera();
	int width();
	int height();
protected:
	int width_ = 0;
	int height_ = 0;
	int viewer_x_ = 0;
	int viewer_y_ = 0;
	int viewer_width_ = 0;
	int viewer_height_ = 0;
	Camera& camera_;
	std::shared_ptr<UniformBlock> modelUniformBlock_;
	std::shared_ptr<UniformBlock> lightUniformBlock_;
	std::shared_ptr<Texture> shadowPlaceholder_ = nullptr;
	std::shared_ptr<RenderStates> renderStates_ = nullptr;
};

template<typename ... Args>
void Renderer::executeRenderPass(std::shared_ptr<RenderPass> renderPass, Args&&... args) {
	renderPass->render(std::forward<Args>(args)...);
}
;
