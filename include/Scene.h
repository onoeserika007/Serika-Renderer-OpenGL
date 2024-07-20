#pragma once
#include <vector>
#include <memory>

class UModel;
class ULight;

class Scene {
	std::vector<std::shared_ptr<UModel>> models_;
	std::vector<std::shared_ptr<ULight>> lights_;
public:
	void addModel(std::shared_ptr<UModel> model);
	std::vector<std::shared_ptr<UModel>>& getModels();

	void addLight(std::shared_ptr<ULight> light);
	std::vector<std::shared_ptr<ULight>>& getLights();
};