#pragma once
#include <glm/gtc/quaternion.hpp>
#include "Base/Globals.h"
#include "Material/FMaterial.h"
#include <memory>
#include <vector>

#include "BufferAttribute.h"
#include "BufferAttribute.h"

//#include 

class ULight;
class FGeometry;
class Shader;
class Texture;
class FCamera;
class FMaterial;
class Renderer;
class Mesh;
enum EShadingModel;


// _Wptr is initialized in shared_ptr<>
class UObject: public std::enable_shared_from_this<UObject> {

public:

	virtual void updateFrame(Renderer& renderer);

	virtual ~UObject()= default;
	virtual void init();

	virtual std::unique_ptr<UObject> Clone() const { return std::unique_ptr<UObject>(new UObject(*this)); }

	// setters
	virtual void setPosition(const glm::vec3& position);
	void setScale(const glm::vec3& scale);
	void setScale(float x, float y, float z);

	void setWorldMatrix(const glm::mat4 &transform);
	void setParentWorldMatrix(const glm::mat4& transform);

	void setParent(const std::shared_ptr<UObject> &parent);
	void addChild(const std::shared_ptr<UObject>& child);
	std::vector<std::shared_ptr<UObject>>& getChildren();

	void enableCastShadow(bool enabled);
	void setShadingMode(EShadingModel shadingMode);

	// transforms
	void rotate(float angle, const glm::vec3& axis);
	void lookAt(const glm::vec3& focus, bool bIgnorePitch = true);
	void translate(float x, float y, float z);
	void scale(float x, float y, float z);

	// transform getters
	glm::vec3 getPosition() const;
	glm::mat4 getWorldMatrix() const;
	glm::mat4 getNormalToWorld() const;
	glm::vec3 getForwardVector() const { return quaternion_ * glm::vec3(0.f, 0.f, -1.f); }
	glm::vec3 getRightVector() const { return quaternion_ * glm::vec3(1.f, 0.f, 0.f); }
	glm::vec3 getUpVector() const { return quaternion_ * glm::vec3(0.f, 1.f, 0.f); }

	// define local forward 0 0 -1
	static glm::vec3 getLocalForward() { return {0.f, 0.f, -1.f}; }

	bool drawable() const { return bDrawable; }
	bool castShadow() const { return bCastShadow; }
	EShadingModel getShadingMode() const { return shadingMode_; }

	// std::shared_ptr<UObject> generateInstance() const;
	UObject& operator=(const UObject& other) = delete;

protected:
	UObject();
	UObject(const UObject& other);
	bool bDrawable;
	bool bCastShadow = false;
	EShadingModel shadingMode_ = Shading_BaseColor;

private:
	void updateWorldMatrix();
	// object
	// glm::mat4 rotation_;
	glm::quat quaternion_{};
	glm::mat4 normalToWorld_{};
	glm::vec3 position_{};
	glm::vec3 scale_{};

	glm::mat4 worldMatrix_{};
	glm::mat4 parentWorldMatrix_{};
	glm::mat4 localMatrix_{};

	// hierachy
	std::weak_ptr<UObject> parent_;
	std::vector<std::shared_ptr<UObject>> children_;

	// UUID
	Serika::UUID<UObject> uuid_;
};