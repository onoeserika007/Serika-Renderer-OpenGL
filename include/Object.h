#pragma once
#include "Base/GLMInc.h"
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

class Object {
	std::shared_ptr<Geometry> pgeometry_;
	std::shared_ptr<Material> pmaterial_;

	glm::mat4 modelMatrix_;
	glm::mat4 rotation_;
	glm::mat4 normalToWorld_;
	glm::vec3 position_;
	glm::vec3 scale_;
	unsigned VAO;
public:
	Object();

	Object(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);

	void init();

	virtual void updateFrame(Renderer& renderer);

	virtual ~Object(){}

	void draw(Renderer& renderer);

	void setupPipeline(Renderer& renderer);

	std::shared_ptr<Shader> getpShader();

	std::shared_ptr<Geometry> getpGeometry();

	std::shared_ptr<Material> getpMaterial();

	unsigned getVAO();

	void setVAO(unsigned id);

	// set model matrix
	void setWorldMatrix(glm::mat4 model);

	void updateWorldMatrix();

	void setPosition(const glm::vec3& position);

	void setScale(const glm::vec3& scale);
	
	void setScale(float x, float y, float z);

	void setLight(std::shared_ptr<Light> plight) const;

	void setLightArray(const std::vector<std::shared_ptr<Light>>& plightArray) const;

	void setpshader(std::shared_ptr<Shader> pshader);

	glm::vec3 getPosition() const;

	glm::mat4 getModelMatrix() const;

	glm::mat4 getNormalToWorld() const;
};