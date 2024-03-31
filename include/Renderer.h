#pragma once
#include <iostream>
#include <memory>
#include "Base/RenderStates.h"

class BufferAttribute;
class Geometry;
class Texture;
class Material;
class StandardMaterial;
class Camera;
class Uniform;
class UniformBlock;
class Object;
class Shader;
class FrameBuffer;


enum RendererType {
	Renderer_SOFT,
	Renderer_OPENGL,
	Renderer_Vulkan,
};

enum ShaderPass;

class Renderer {
public:
	Renderer(std::shared_ptr<Camera> camera);
	virtual void init() = 0;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) = 0;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") = 0;

	virtual void setupVertexAttribute(BufferAttribute& vertexAttribute) = 0;
	virtual void setupGeometry(Geometry& geometry) = 0;
	virtual void setupTexture(Texture& texture) = 0;
	virtual void loadShaders(Material& material) = 0;
	virtual void setupMaterial(Material& material) = 0;
	virtual void setupStandardMaterial(StandardMaterial& material) = 0;
	virtual void setupObject(Object& object) = 0;

	virtual void useMaterial(Material& material) = 0;
	virtual void useMaterial(Material& material, ShaderPass pass) = 0;
	virtual void drawObject(Object& object, ShaderPass pass) = 0;

	virtual void updateModelUniformBlock(Object& object, Camera& camera, bool shadowPass = false) = 0;

	// pipeline related
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer>& frameBuffer, const ClearStates& states) = 0;
	virtual void endRenderPass() = 0;
	virtual void waitIdle() = 0;

	virtual ~Renderer();
	virtual void clearTexture(Texture& texture) = 0;

	std::shared_ptr<Camera> getCamera();
protected:
	std::shared_ptr<Camera> camera_;
	std::shared_ptr<UniformBlock> modelUniformBlock_;
	std::shared_ptr<UniformBlock> pointLightUniformBlock_;
	std::shared_ptr<Texture> shadowPlaceholder_ = nullptr;
};