#ifndef SERIKA_RENDERER_INCLUDE_GEOMETRY_OBJECT_H
#define SERIKA_RENDERER_INCLUDE_GEOMETRY_OBJECT_H
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

#include "Primitives.h"
#include "Geometry/BoundingBox.h"
#include "Geometry/BVHAccel.h"

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
class UObject: public Intersectable, public std::enable_shared_from_this<UObject> {

public:
	template<typename ...Args>
	static std::shared_ptr<UObject> makeObject(Args &&...args);

	virtual void updateFrame();

	virtual ~UObject()= default;
	virtual void init();

	virtual std::unique_ptr<UObject> Clone() const { return std::unique_ptr<UObject>(new UObject(*this)); }

	// setters
	virtual void setPosition(const glm::vec3& position);
	void setScale(const glm::vec3& scale);
	void setScale(float x, float y, float z);

	void setLocalMatrix(const glm::mat4 &local);
	void setParentWorldMatrix(const glm::mat4& transform);

	void setParent(const std::shared_ptr<UObject> &parent);
	void addChild(const std::shared_ptr<UObject>& child);
	std::vector<std::shared_ptr<UObject>>& getChildren();

	void setMesh(const std::shared_ptr<UMesh>& mesh);

	// getters
	virtual bool drawable() const { return false; }
	bool visible() const { return bVisible; }
	int getUUID() const { return uuid_.get(); }
	std::shared_ptr<UMesh> getMesh() const { return mesh_; }

	// transforms
	void rotate(float angle, const glm::vec3& axis);
	void lookAt(const glm::vec3& focus, bool bIgnorePitch = true);
	void translate(float x, float y, float z);
	void scale(float x, float y, float z);
	void applyTransform(const glm::mat4& transform);

	// transform getters
	glm::vec3 getWorldPosition() const;
	glm::mat4 getWorldMatrix() const;
	glm::mat4 getNormalToWorld() const;
	glm::vec3 getForwardVector() const { return worldOrientation_ * glm::vec3(0.f, 0.f, -1.f); }
	glm::vec3 getRightVector() const { return worldOrientation_ * glm::vec3(1.f, 0.f, 0.f); }
	glm::vec3 getUpVector() const { return worldOrientation_ * glm::vec3(0.f, 1.f, 0.f); }

	// define local forward 0 0 -1
	static glm::vec3 getLocalForward() { return {0.f, 0.f, -1.f}; }

	///
	/// Inherited from Primitive
	///
	virtual bool intersect(const Ray& ray) override { return true; }
	// virtual bool intersect(const Ray& ray, float &, uint32_t &) const = 0;
	virtual Intersection getIntersection(const Ray &ray) override;

	virtual BoundingBox getBounds() const override{ return bbox_; }
	virtual float getArea() const override { return area_; }
	virtual void Sample(Intersection &pos, float &pdf) override;
	virtual bool hasEmit() const override { return true; /* never to be called since mesh not directly intersect */ }
	virtual void transform(const glm::mat4 &trans) override {  }

	// call from parent explicitly
	// 并且从抽象层级来将，这个应该对应于loadMap的阶段
	void buildBVH();
	std::shared_ptr<BVHAccel> getBVH() const { return bvh_accel_; }
	const std::vector<std::shared_ptr<Intersectable>>& getIntersctables() const { return intersectables_; }


	// std::shared_ptr<UObject> generateInstance() const;
	UObject& operator=(const UObject& other) = delete;
public:
	bool bShowDebugBBox_ = false;

protected:
	UObject() = default;
	// explicit UObject(const std::shared_ptr<UMesh>& mesh);
	UObject(const UObject& other);
	UObject(UObject&& other) noexcept;
	bool bVisible = true;

private:
	void updateLocalMatrix();
	void setWorldMatrix(const glm::mat4 &transform); // local matrix should take the lead, only the parent matrix and local matrix can change
	// object
	// glm::mat4 rotation_;
	glm::mat4 normalToWorld_ {1.f};

	glm::vec3 worldPosition_ {0.f};
	glm::quat worldOrientation_ {1.f, 0.f, 0.f, 0.f};
	glm::vec3 worldScale_ {1.f};

	glm::vec3 localPosition_ {0.f};
	glm::quat localOrientation_ {1.f, 0.f, 0.f, 0.f};
	glm::vec3 localScale_ {1.f};

	glm::mat4 worldMatrix_ {1.f};
	glm::mat4 parentWorldMatrix_ {1.f};
	glm::mat4 localMatrix_ {1.f};

	// hierachy
	std::weak_ptr<UObject> parent_;
	std::vector<std::shared_ptr<UObject>> children_;
	std::shared_ptr<UMesh> mesh_; // trying to refactor!!!

	// intersectable
	float area_ = 0.f;
	BoundingBox bbox_;
	std::vector<std::shared_ptr<Intersectable>> intersectables_;
	std::shared_ptr<BVHAccel> bvh_accel_;

	// UUID
	Serika::UUID<UObject> uuid_;
};

template<typename ...Args>
std::shared_ptr<UObject> UObject::makeObject(Args &&...args) {
	return std::shared_ptr<UObject>(new UObject(std::forward<Args>(args)...));
}

#endif // SERIKA_RENDERER_INCLUDE_GEOMETRY_OBJECT_H
