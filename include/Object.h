#pragma once
#include "Base/GLMInc.h"
#include "Material.h"
#include <memory>
#include <vector>

//#include 

class Light;
class Geometry;
class Shader;
class Texture;
class Camera;
class Material;
class Renderer;
enum ShadingMode;

class Object {
	// mesh
	std::shared_ptr<Geometry> pgeometry_;
	std::shared_ptr<Material> pmaterial_;
	ShadingMode shadingMode_ = Shading_BaseColor;


	// object
	glm::mat4 modelMatrix_;
	glm::mat4 rotation_;
	glm::mat4 normalToWorld_;
	glm::vec3 position_;
	glm::vec3 scale_;
	unsigned VAO;

	bool bPipelineSetup = false;
public:
	Object();

	Object(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);

	void init();

	virtual void updateFrame(Renderer& renderer);

	virtual ~Object(){}

	void draw(Renderer& renderer);

	// getters
	std::shared_ptr<Shader> getpShader();
	std::shared_ptr<Geometry> getpGeometry();
	std::shared_ptr<Material> getpMaterial();
	unsigned getVAO();
	ShadingMode getShadingMode();

	// setters
	void setWorldMatrix(glm::mat4 model);
	void updateWorldMatrix();
	void setPosition(const glm::vec3& position);
	void setScale(const glm::vec3& scale);
	void setScale(float x, float y, float z);
	void setLight(std::shared_ptr<Light> plight) const;
	void setLightArray(const std::vector<std::shared_ptr<Light>>& plightArray) const;
	void setpshader(std::shared_ptr<Shader> pshader);
	void setVAO(unsigned id);
	void setShadingMode(ShadingMode shadingMode);

	// transform getters
	glm::vec3 getPosition() const;
	glm::mat4 getModelMatrix() const;
	glm::mat4 getNormalToWorld() const;

	///
	/// pipeline
	/// 
	//void setupPipeline(Renderer& renderer);

	inline void setPipelineReady(bool ready) {
		bPipelineSetup = ready;
	}

	bool isPipelineReady() const {
		return bPipelineSetup;
	}
};