#ifndef SERIKA_RENDERER_INCLUDE_FSCENE_H
#define SERIKA_RENDERER_INCLUDE_FSCENE_H
#include <vector>
#include <memory>

#include "Base/Globals.h"
#include "Geometry/BoundingBox.h"
#include "Geometry/BVHAccel.h"

class UObject;
class FCamera;
struct Intersection;
class Intersectable;
class BVHAccel;
class UMesh;
class ULight;

class FScene {
public:
	static std::shared_ptr<FScene> generateDeaultScene(const std::shared_ptr<FCamera> &camera);
	static std::shared_ptr<FScene> generateRaytracingStanfordBunnyScene(const std::shared_ptr<FCamera> &camera);
	static std::shared_ptr<FScene> generatePBRScene(const std::shared_ptr<FCamera> &camera);
	static std::shared_ptr<FScene> generateRaytracingCornellBoxScene(const std::shared_ptr<FCamera> &camera);

	void addObject(std::shared_ptr<UObject> obj);
	void addLight(std::shared_ptr<ULight> light);
	void addBBox(const BoundingBox& bbox);

	void addIntersectable(const std::shared_ptr<Intersectable>& inters);

	void setFocus(const glm::vec3& focus);

	std::vector<std::shared_ptr<UObject>> &getObjects();
	std::vector<std::shared_ptr<ULight>>& getLights();
	std::vector<BoundingBox>& getBBoxes();

	NO_DISCARD const std::vector<std::shared_ptr<UMesh>>& getPackedMeshes() const;
	NO_DISCARD const std::vector<std::shared_ptr<UMesh>>& getPackedLightMeshes() const;
	NO_DISCARD glm::vec3 getFocus() const;

	void buildBVH();
	NO_DISCARD std::shared_ptr<BVHAccel> getBVH();

	void setupScene();

	Intersection intersect(const Ray& ray) const;
	void sampleLight(Intersection& outIsct, float &pdf) const ;
	glm::vec3 castRay(const Ray& ray, int depth, int SobolIndex, float RussianRoulette = 0.6) const;

	// rendering
	void packMeshesFromScene();
	void traverseToPackMesh(const std::shared_ptr<UObject>& obj, std::vector<std::shared_ptr<UMesh>> &cache_to_pack);

	std::shared_ptr<UObject> skybox_;
	ESceneType sceneType_;
private:
	std::vector<std::shared_ptr<UObject>> objects_;
	std::vector<std::shared_ptr<ULight>> lights_;

	std::vector<std::shared_ptr<UMesh>> packed_meshes_cache_;
	std::vector<std::shared_ptr<UMesh>> packed_lights_meshes_cache_;
	bool bMeshCacheDirty = true;

	std::vector<BoundingBox> bboxes_;

	glm::vec3 focus_ {};

	std::vector<std::shared_ptr<Intersectable>> intersectables_;
	std::weak_ptr<FCamera> camera_;

	std::shared_ptr<BVHAccel> bvh_accel_;
};

#endif // SERIKA_RENDERER_INCLUDE_FSCENE_H
