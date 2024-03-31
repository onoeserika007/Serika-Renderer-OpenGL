#include <memory>

class Renderer;
class Camera;
class Texture;
class FrameBuffer;
class Config;
class Object;
enum class CameraType;

class Viewer {
private:
	void init(int width, int height, int outTexId);
	void cleanup();

	std::shared_ptr<Camera> createCamera(CameraType type);
	virtual std::shared_ptr<Renderer> createRenderer() = 0;
	void drawObject(Object& obj);
protected:
	Config& config_;
	Camera& cameraMain_;
	std::shared_ptr<Camera> cameraDepth_ = nullptr;

	// scene

	int width_ = 0;
	int height_ = 0;
	int outTexId_ = 0;

	std::shared_ptr<Renderer> renderer_ = nullptr;

	// 现在暂定不对renderpass进行循环渲染，而是手动装配管线，即手动接入和输出renderpass的结果

	// main fbo
	std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
	std::shared_ptr<Texture> texColorMain_ = nullptr;
	std::shared_ptr<Texture> texDepthMain_ = nullptr;

	// shadow map
	std::shared_ptr<FrameBuffer> fboShadow_ = nullptr;
	std::shared_ptr<Texture> texDepthShadow_ = nullptr;
	std::shared_ptr<Texture> shadowPlaceholder_ = nullptr;
public:
	Viewer(Camera& camera, Config& config);
};