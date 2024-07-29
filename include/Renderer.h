#pragma once
#include <memory>
#include <RenderPass/RenderPass.h>

#include "OpenGL/ShaderGLSL.h"
#include "Base/RenderStates.h"
#include "Material/Texture.h"
#include "Base/Config.h"
#include "Geometry/BVHAccel.h"
#include "Geometry/Triangle.h"

class Triangle;
struct FLine;
class BoundingBox;
class ULight;
class RenderPass;
class BufferAttribute;
class FGeometry;
class Texture;
class FMaterial;
class StandardMaterial;
class FCamera;
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


enum class ShaderPass: uint8_t;
enum TextureTarget;
enum TextureFormat;

class Renderer: public std::enable_shared_from_this<Renderer> {
public:
	explicit Renderer(const std::shared_ptr<FCamera> &camera);
	virtual void init() = 0;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) const  = 0;
	virtual std::shared_ptr< UniformSampler> createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format) const = 0;
	virtual std::shared_ptr< UniformSampler> createUniformSampler(const TextureInfo& texInfo) const = 0;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") = 0;
	virtual std::shared_ptr<Texture> createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData = TextureData()) const = 0;
	virtual std::shared_ptr<FrameBuffer> createFrameBuffer(bool offScreen) = 0;
	virtual void setupColorBuffer(std::shared_ptr<Texture>& outBuffer, int width, int height, bool force = false, bool bCubeMap = false, TextureTarget
	                              texTarget = TextureTarget_TEXTURE_2D, TextureFormat texFormat = TextureFormat_RGBA8, TextureType texType =
			                              TEXTURE_TYPE_NONE) const = 0;
	virtual void setupDepthBuffer(std::shared_ptr<Texture>& outBuffer, bool multiSample, bool force = false) const = 0;
	virtual void setupShadowMapBuffer(std::shared_ptr<Texture>& outBuffer, int width, int height, bool multiSample, bool bCubeMap, bool force = false) const  = 0;

	virtual void setupVertexAttribute(const BufferAttribute &vertexAttribute) const = 0;
	virtual void setupGeometry(FGeometry& geometry) const = 0;
	virtual void loadShaders(FMaterial& material) const = 0;
	virtual void setupMaterial(FMaterial& material) const = 0;
	//virtual void setupStandardMaterial(StandardMaterial& material) = 0;
	virtual void setupMesh(const std::shared_ptr<UMesh> &mesh, ShaderPass shaderPass) const = 0;

	/** drawing begin*/
	virtual void drawMesh(
		const std::shared_ptr<UMesh> &mesh,
		ShaderPass pass,
		const std::shared_ptr<ULight> &shadowLight,
		const std::shared_ptr<Shader> &overrideShader = {}) = 0;
	virtual void drawDebugBBoxes(const std::shared_ptr<BVHNode> &node, int depth, const std::shared_ptr<UObject> &rootObject) = 0;
	virtual void drawDebugBBox(const BoundingBox& bbox, const std::shared_ptr<UObject> &holdingObject, int depth) = 0;
	virtual void drawWorldAxis(const std::shared_ptr<UMesh> &holdingMesh) = 0;
	void drawDebugLine(const glm::vec3& start, const glm::vec3& end, float persistTime = 0.f);
	void drawDebugTriangle(const Triangle& inTri, float persistTime = 0.f);
	void handleDebugs();
	void remove_triangle_debug_task_safe(int uuid);
	void remove_line_debug_task_safe(int uuid);
	/** drawing end*/

	void loadGlobalUniforms(const std::shared_ptr<UMesh> &mesh) const;
	void loadGlobalUniforms(const Shader& program) const;

	void updateModelUniformBlock(const std::shared_ptr<UMesh> &mesh, const std::shared_ptr<FCamera> &camera, const std::shared_ptr<ULight> &shadowLight = nullptr) const;
	void updateShadowCubeUniformBlock(const std::shared_ptr<ULight> &shadowLight) const;
	void updateLightUniformBlock(const std::shared_ptr<ULight>& light) const;

	// pipeline related
	virtual void setCullFaceEnabled(bool Enabled) = 0;
	virtual void updateRenderStates(const RenderStates &renderStates) = 0;
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer> frameBuffer, const ClearStates& states) = 0;
	virtual void endRenderPass() = 0;

	virtual void setRenderViewPort(int x, int y, int width, int height) = 0;
	void setViewPort(int x, int y, int width, int height);
	void restoreViewPort();

	virtual void waitIdle() = 0;

	// renderpasses
	template <typename ...Args>
	void executeRenderPass(std::shared_ptr<RenderPass> renderPass, Args&&... args);
	virtual void dump(const
	                  std::shared_ptr<Shader> &program, bool bBlend, std::shared_ptr<FrameBuffer> targetFrameBuffer, int dstColorBuffer) = 0;
	virtual std::shared_ptr<ShaderGLSL> getDefferedShadingProgram(const std::vector<std::shared_ptr<Texture>> & gBuffers) const = 0;
	virtual std::shared_ptr<ShaderGLSL> getToScreenColorProgram(const std::shared_ptr<Texture> &srcTex) const = 0;
	virtual std::shared_ptr<ShaderGLSL> getToScreenDepthProgram(const std::shared_ptr<Texture> &srcTex) const = 0;

	virtual ~Renderer();
	virtual void clearTexture(Texture& texture) = 0;

	std::shared_ptr<FCamera> getCamera() const;
	std::shared_ptr<FCamera> getViewCamera() const;

	void setCamera(const std::shared_ptr<FCamera>& camera);
	void setBackToViewCamera();
	void setRenderingScene(const std::shared_ptr<FScene> &scene);

	int width() const;
	int height() const;
	ERendererType rendererType() const { return renderer_type_; }

	ERenderMode render_mode_;
	RenderStates renderStates_;

	// place holder
	std::shared_ptr<Texture> envCubeMapPlaceholder_;
	std::shared_ptr<Texture> shadowMapPlaceholder_;
	std::shared_ptr<Texture> shadowMapCubePlaceholder_;

protected:
	/** Inner functions */
	virtual void drawDebugLine_Impl(FLine &line) = 0;
	virtual void drawDebugTriangle_Impl(Triangle &triangle) = 0;
protected:

	ERendererType renderer_type_;

	int width_ = 0;
	int height_ = 0;
	int viewer_x_ = 0;
	int viewer_y_ = 0;
	int viewer_width_ = 0;
	int viewer_height_ = 0;

	std::shared_ptr<FCamera> mainCamera_;
	std::shared_ptr<FCamera> viewCamera_;

	std::weak_ptr<FScene> renderingScene_;

	// global unniforms
	std::shared_ptr<UniformBlock> modelUniformBlock_;
	std::shared_ptr<UniformBlock> shadowUniformBlock_;
	std::shared_ptr<UniformBlock> lightUniformBlock_;
	mutable std::weak_ptr<UniformSampler> shadowMapUniformSampler_;
	mutable std::weak_ptr<UniformSampler> shadowMapCubeUniformSampler_;
	mutable std::weak_ptr<UniformSampler> envCubeMapUniformSampler_;

	// cache
	mutable std::unordered_map<int, std::shared_ptr<ShaderResources>> noObjectContextShaderResources;
	mutable std::unordered_map<int, unsigned> bbox_draw_cache_;
	// mutable std::vector<std::shared_ptr<UMesh>> meshes_to_render_cache;
	mutable std::unordered_map<int, std::shared_ptr<FLine>> debug_line_tasks_;
	mutable std::unordered_map<int, std::shared_ptr<Triangle>> debug_triangle_tasks_;

	// mutex
	std::mutex debug_line_task_lock_;
	std::mutex debug_triangle_task_lock_;
};

template<typename ... Args>
void Renderer::executeRenderPass(std::shared_ptr<RenderPass> renderPass, Args&&... args) {
	renderPass->render(std::forward<Args>(args)...);
}
;
