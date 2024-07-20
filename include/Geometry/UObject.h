#pragma once
#include "../Base/GLMInc.h"
#include "../Material.h"
#include <memory>
#include <vector>

//#include 

class ULight;
class Geometry;
class Shader;
class Texture;
class Camera;
class Material;
class Renderer;
enum ShadingMode;

class UObject {

	// object
	glm::mat4 modelMatrix_;
	glm::mat4 rotation_;
	glm::mat4 normalToWorld_;
	glm::vec3 position_;
	glm::vec3 scale_;

	bool bPipelineSetup = false;
public:
	UObject();

	void init();

	virtual void updateFrame(Renderer& renderer);

	virtual ~UObject(){}

	// setters
	void setWorldMatrix(glm::mat4 model);
	void updateWorldMatrix();
	void setPosition(const glm::vec3& position);
	void setScale(const glm::vec3& scale);
	void setScale(float x, float y, float z);
	void setLight(std::shared_ptr<ULight> plight) const;
	void setLightArray(const std::vector<std::shared_ptr<ULight>>& plightArray) const;

	// transform getters
	glm::vec3 getPosition() const;
	glm::mat4 getModelMatrix() const;
	glm::mat4 getNormalToWorld() const;

	///
	/// pipeline
	///

	void setPipelineReady(bool ready) {
		bPipelineSetup = ready;
	}

	bool isPipelineReady() const {
		return bPipelineSetup;
	}
};