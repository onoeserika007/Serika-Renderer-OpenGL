#include "Scene.h"

void Scene::addModel(std::shared_ptr<UModel> model)
{
	models_.emplace_back(std::move(model));
}

std::vector<std::shared_ptr<UModel>>& Scene::getModels()
{
	// TODO: 在此处插入 return 语句
	return models_;
}

void Scene::addLight(std::shared_ptr<ULight> light) {
	lights_.emplace_back(std::move(light));
}

std::vector<std::shared_ptr<ULight>>& Scene::getLights() {
	return lights_;
}
