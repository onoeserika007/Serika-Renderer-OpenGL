#pragma once
#include <vector>
#include <memory>

class Model;

class Scene {
	std::vector<std::shared_ptr<Model>> models_;
public:
	void addModel(std::shared_ptr<Model> model);
	std::vector<std::shared_ptr<Model>>& getModels();
};