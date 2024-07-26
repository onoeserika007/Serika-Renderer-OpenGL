#pragma once
#include <vector>
#include <memory>
#include <vec3.hpp>

#include "Base/Globals.h"

class UMesh;
class UModel;
class ULight;

class FScene {
public:
	static std::shared_ptr<FScene> generateDeaultScene();
	void addModel(std::shared_ptr<UModel> model);
	std::vector<std::shared_ptr<UModel>>& getModels();

	void addLight(std::shared_ptr<ULight> light);
	std::vector<std::shared_ptr<ULight>>& getLights();

	NO_DISCARD glm::vec3 getFocus() const { return focus_; }
	void setFocus(const glm::vec3& focus) { focus_ = focus; }

	std::shared_ptr<UMesh> skybox_;
private:
	std::vector<std::shared_ptr<UModel>> models_;
	std::vector<std::shared_ptr<ULight>> lights_;

	glm::vec3 focus_ {};
};