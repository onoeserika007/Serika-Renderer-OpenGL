#include "Renderer.h"

class RendererOpenGL : public Renderer {
	RendererOpenGL(std::shared_ptr<Camera> camera);
	virtual void init() override;

	virtual std::shared_ptr<UniformBlock> createUniformBlock(const std::string& name, int size) override;
	virtual std::shared_ptr<Shader> createShader(const std::string& vsPath = "", const std::string& fsPsth = "") override;

	virtual void setupVertexAttribute(BufferAttribute& vertexAttribute) override;
	virtual void setupGeometry(Geometry& geometry) override;
	virtual void setupTexture(Texture& texture) override;
	virtual void loadShaders(Material& material) override;
	virtual void setupMaterial(Material& material) override;
	virtual void setupStandardMaterial(StandardMaterial& material) override;
	virtual void setupObject(Object& object) override;

	virtual void useMaterial(Material& material) override;
	virtual void useMaterial(Material& material, ShaderPass pass) override;
	virtual void drawObject(Object& object, ShaderPass pass) override;

	virtual void updateModelUniformBlock(Object& object, Camera& camera, bool shadowPass = false) override;

	// pipeline related
	virtual void beginRenderPass(std::shared_ptr<FrameBuffer>& frameBuffer, const ClearStates& states) override;
	virtual void endRenderPass() override;
	virtual void waitIdle() override;

	virtual ~RendererOpenGL();
	virtual void clearTexture(Texture& texture) override;

};