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
class Mesh;
enum EShadingMode;

// _Wptr is initialized in shared_ptr<>
class UObject: public std::enable_shared_from_this<UObject> {

public:

	virtual void updateFrame(Renderer& renderer);

	virtual ~UObject(){}

	// setters
	void setPosition(const glm::vec3& position);
	void setScale(const glm::vec3& scale);
	void setScale(float x, float y, float z);

	void setWorldMatrix(const glm::mat4 &transform);
	void setParentWorldMatrix(const glm::mat4& transform);

	void setParent(const std::shared_ptr<UObject> &parent);
	void addChild(const std::shared_ptr<UObject>& child);
	std::vector<std::shared_ptr<UObject>>& getChildren();

	void enableCastShadow(const bool enabled);
	void setShadingMode(EShadingMode shadingMode);

	// transform getters
	glm::vec3 getPosition() const;
	glm::mat4 getWorldMatrix() const;
	glm::mat4 getNormalToWorld() const;

	bool drawable() const { return bDrawable; }
	bool castShadow() const { return bCastShadow; }
	EShadingMode getShadingMode() { return shadingMode_; }

protected:
	UObject();
	bool bDrawable;
	bool bCastShadow = false;
	EShadingMode shadingMode_ = Shading_BaseColor;

private:
	void updateWorldMatrix();
	// object
	glm::mat4 rotation_;
	glm::mat4 normalToWorld_;
	glm::vec3 position_;
	glm::vec3 scale_;

	glm::mat4 worldMatrix_;
	glm::mat4 parentWorldMatrix_;
	glm::mat4 localMatrix_;

	// hierachy
	std::weak_ptr<UObject> parent_;
	std::vector<std::shared_ptr<UObject>> children_;
};