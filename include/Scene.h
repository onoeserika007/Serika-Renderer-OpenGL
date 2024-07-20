#pragma once
#include <vector>
#include <memory>

class Model;
class ULight;

class Scene {
	std::vector<std::shared_ptr<Model>> models_;
	std::vector<std::shared_ptr<ULight>> lights_;
public:
	void addModel(std::shared_ptr<Model> model);
	std::vector<std::shared_ptr<Model>>& getModels();

	void addLight(std::shared_ptr<ULight> light);
	std::vector<std::shared_ptr<ULight>>& getLights();
};